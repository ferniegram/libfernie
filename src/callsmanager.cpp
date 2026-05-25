#include "callsmanager.h"
#include "callaudio.h"

#include <tgcalls/InstanceImpl.h>
#include <tgcalls/v2/InstanceV2Impl.h>
#include <tgcalls/v2/InstanceV2ReferenceImpl.h>

#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_device/linux/audio_device_pulse_linux.h"

#define DEBUG_MODULE CallsManager
#include "debuglog.h"

#include <QAudioDeviceInfo>

namespace {
    const QString _TYPE("@type");
    const QString PROTOCOL("protocol");
    const QString UDP_P2P("udp_p2p");
    const QString MAX_LAYER("max_layer");
}


CallsManager::CallsManager(TDLibWrapper *tdLibWrapper, QObject *parent) :
    QObject(parent),
    tdLibWrapper(tdLibWrapper)
{
    connect(tdLibWrapper, &TDLibWrapper::callIdReceived, this, &CallsManager::handleCallIdReceived);
    connect(tdLibWrapper, &TDLibWrapper::callUpdated, this, &CallsManager::handleCallUpdated);
    connect(tdLibWrapper, &TDLibWrapper::newCallSignalingDataReceived, this, &CallsManager::handleNewCallSignalingDataReceived);

    tgcalls::Register<tgcalls::InstanceImpl>();
    tgcalls::Register<tgcalls::InstanceV2Impl>();
    tgcalls::Register<tgcalls::InstanceV2ReferenceImpl>();
}

QVariantMap CallsManager::protocol() {
    QStringList versions;
    for (const auto &version : tgcalls::Meta::Versions())
        versions << QString::fromStdString(version);
    std::sort(versions.begin(), versions.end(), [](const QString &a, const QString &b) {
        return a > b;
    }); // Server processes them newer to older

    return {
        {_TYPE, "callProtocol"},
        {UDP_P2P, true},
        {"udp_reflector", true},
        {"min_layer", 65},
        {MAX_LAYER, tgcalls::Meta::MaxLayer()},
        {"library_versions", versions}
    };
}

void CallsManager::createCall(qlonglong userId) {
    tdLibWrapper->createCall(userId, protocol());
}

void CallsManager::handleCallIdReceived(int id) {
    LOG("Call ID received" << id);
    currentCallId = id;
}

CallsManager::CallState CallsManager::getCallState(const QString &type) {
    if (type == "callStatePending")
        return CallState::Pending;
    if (type == "callStateExchangingKeys")
        return CallState::ExchangingKeys;
    if (type == "callStateReady")
        return CallState::Ready;
    if (type == "callStateHangingUp")
        return CallState::HangingUp;
    if (type == "callStateDiscarded")
        return CallState::Discarded;
    if (type == "callStateError")
        return CallState::Error;

    return CallState::Discarded;
}

void CallsManager::handleCallUpdated(int id, qlonglong uniqueId, qlonglong userId, bool outgoing, bool video, const QVariantMap &state) {
    if (id != this->currentCallId)
        return; // TODO: cancel current call or handle this somehow otherwise

    this->currentCallState = getCallState(state.value(_TYPE).toString());
    LOG("Call updated" << id << currentCallState << state);

    switch (currentCallState) { // TODO
    case CallState::Pending:
    case CallState::ExchangingKeys:
    case CallState::HangingUp:
        break;
    case CallState::Discarded:
        handleCallDiscarded();
        break;
    case CallState::Error:
    {
        const QVariantMap error = state.value("error").toMap();
        WARN("Error" << error.value("code").toInt() << error.value("message").toString());
        break;
    }
    case CallState::Ready:
        handleCallReady(outgoing, state);
        break;
    }

    emit currentCallStateChanged();
}

void CallsManager::handleCallReady(bool outgoing, const QVariantMap &state) {
    LOG("Call is ready");
    const QVariantMap protocol = state.value(PROTOCOL).toMap();

    tgcalls::Config config{
        .initializationTimeout = tdLibWrapper->getOption("call_packet_timeout_ms").toDouble() / 1000,
        .receiveTimeout = tdLibWrapper->getOption("call_connect_timeout_ms").toDouble() / 1000,
        .enableP2P = protocol.value(UDP_P2P).toBool() && protocol.value("allow_p2p").toBool(),
        .maxApiLayer = protocol.value(MAX_LAYER).toInt()
    };

    QByteArray key = QByteArray::fromBase64(state.value("encryption_key").toString().toUtf8());
    auto encryptionKey = std::make_shared<std::array<uint8_t, 256>>();
    memcpy(encryptionKey->data(), key.constData(), 256);

    const QString outputDevice = CallAudio::getOutputDeviceName(),
            inputDevice = CallAudio::getInputDeviceName();
    LOG("Using output device" << outputDevice << "input device" << inputDevice);

    tgcalls::MediaDevicesConfig mediaConfig{
        .audioInputId = inputDevice.toStdString(),
        .audioOutputId = outputDevice.toStdString(),
        .inputVolume = 1.f,
        .outputVolume = 1.f
    };

    tgcalls::Descriptor descriptor{
        .config = config,
        .encryptionKey = tgcalls::EncryptionKey(encryptionKey, outgoing),
        .mediaDevicesConfig = mediaConfig
    };

    for (const QVariant &serverVariant : state.value("servers").toList()) {
        const QVariantMap server = serverVariant.toMap();

        QString ip = server.value("ip_address").toString();
        QString ipv6 = server.value("ipv6_address").toString();
        uint16_t port = server.value("port").toUInt();

        const QVariantMap type = server.value("type").toMap();
        if (type.value(_TYPE).toString() == "callServerTypeWebrtc") {
            tgcalls::RtcServer rtcServer{
                .host = ip.toStdString(),
                .port = port,
                .login = type.value("username").toString().toStdString(),
                .password = type.value("password").toString().toStdString(),
                .isTurn = type.value("supports_turn").toBool()
            };
            descriptor.rtcServers.push_back(rtcServer);

            if (!ipv6.isEmpty()) {
                tgcalls::RtcServer ipv6RtcServer(rtcServer);
                ipv6RtcServer.host = ipv6.toStdString();
                descriptor.rtcServers.push_back(ipv6RtcServer);
            }
        } else { // callServerTypeTelegramReflector
            tgcalls::Endpoint endpoint{
                .endpointId = server.value("id").toLongLong(),
                .host = tgcalls::EndpointHost{
                    .ipv4 = ip.toStdString(),
                    .ipv6 = ipv6.toStdString()
                },
                .port = port,
                .type = type.value("is_tcp").toBool() ? tgcalls::EndpointType::TcpRelay : tgcalls::EndpointType::UdpRelay
            };

            QByteArray peerTag = QByteArray::fromBase64(type.value("peer_tag").toString().toUtf8());
            if (peerTag.size() == 16)
                memcpy(endpoint.peerTag, peerTag.constData(), 16);
            else {
                WARN("Invalid reflector peerTag size" << peerTag.size());
                continue;
            }

            descriptor.endpoints.push_back(endpoint);
        }
    }

    descriptor.stateUpdated = [](tgcalls::State state) {
        LOG("State updated" << static_cast<int>(state));
    };
    descriptor.signalBarsUpdated = [](int bars) {
        LOG("Signal bars updated" << bars);
    };
    descriptor.audioLevelUpdated = [](float level) {
        LOG("Audio level updated" << level);
    };
    descriptor.remoteBatteryLevelIsLowUpdated = [](bool value) {
        LOG("Remote battery level is low updated" << value);
    };
    descriptor.remoteMediaStateUpdated = [](tgcalls::AudioState audioState, tgcalls::VideoState videoState) {
        LOG("Remote media state updated audio:" << static_cast<int>(audioState) << "video:" << static_cast<int>(videoState));
    };
    descriptor.remotePrefferedAspectRatioUpdated = [](float ratio) {
        LOG("Remote preferred aspect ratio updated" << ratio);
    };

    descriptor.signalingDataEmitted = [this](const std::vector<uint8_t> &data) {
        QByteArray bytes(reinterpret_cast<const char*>(data.data()), data.size());
        tdLibWrapper->sendCallSignalingData(currentCallId, bytes);
    };

    descriptor.config.logPath.data = "/home/defaultuser/Downloads/call-log.txt";
    descriptor.config.statsLogPath.data = "/home/defaultuser/Downloads/stats-call-log.txt";

    instance = tgcalls::Meta::Create(protocol.value("library_versions").toStringList().first().toStdString(), std::move(descriptor));

    emit callStarted();
}

void CallsManager::handleNewCallSignalingDataReceived(int callId, const QByteArray &data) {
    if (currentCallId == callId) {
        LOG("New call signaling data received");
        if (instance) {
            const auto *ptr = reinterpret_cast<const uint8_t*>(data.constData());
            instance->receiveSignalingData(std::vector<uint8_t>(ptr, ptr + data.size()));
        }
    }
}

void CallsManager::discardCurrentCall() {
    tdLibWrapper->discardCall(currentCallId);
}

void CallsManager::handleCallDiscarded() {
    LOG("Call discarded");
    currentCallId = 0;
    if (instance) {
        instance->stop([](tgcalls::FinalState) {
            LOG("tgcalls instance stopped");
        });
        instance.reset();
    }

    emit callDiscarded();
}
