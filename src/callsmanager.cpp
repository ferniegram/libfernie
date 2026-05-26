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

CallsManager::~CallsManager() {
    LOG("Destroying");
    if (instance) {
        LOG("Stopping tgcalls instance");
        instance->stop([](tgcalls::FinalState) {});
    }
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

    emit callStarted();
}

CallsManager::TdCallState CallsManager::getTdCallState(const QString &type) {
    if (type == "callStatePending")
        return TdCallState::Pending;
    if (type == "callStateExchangingKeys")
        return TdCallState::ExchangingKeys;
    if (type == "callStateReady")
        return TdCallState::Ready;
    if (type == "callStateHangingUp")
        return TdCallState::HangingUp;
    if (type == "callStateDiscarded")
        return TdCallState::Discarded;
    if (type == "callStateError")
        return TdCallState::Error;

    return TdCallState::Discarded;
}

void CallsManager::handleCallUpdated(int id, qlonglong uniqueId, qlonglong userId, bool outgoing, bool video, const QVariantMap &state) {
    if (id != this->currentCallId)
        return; // TODO: cancel current call or handle this somehow otherwise

    if (currentCallUserId != userId) {
        currentCallUserId = userId;
        emit currentCallUserIdChanged();
    }

    this->currentCallState = getTdCallState(state.value(_TYPE).toString());
    this->currentCallStateData = state;
    LOG("Call updated" << id << static_cast<int>(currentCallState) << getCurrentCallState() << state);

    switch (currentCallState) {
    case TdCallState::Pending:
    case TdCallState::ExchangingKeys:
    case TdCallState::HangingUp:
        break;
    case TdCallState::Discarded:
        handleCallDiscarded();
        break;
    case TdCallState::Error:
    {
        const QVariantMap error = state.value("error").toMap();
        WARN("Error" << error.value("code").toInt() << error.value("message").toString());
        break;
    }
    case TdCallState::Ready:
        handleCallReady(outgoing, state);
        break;
    default:
        break;
    }

    emit currentCallStateChanged();
    emit currentCallEmojisChanged();
}

void CallsManager::handleCallReady(bool outgoing, const QVariantMap &state) {
    LOG("Call is ready");
    const QVariantMap protocol = state.value(PROTOCOL).toMap();

    tgcalls::Config config{
        .initializationTimeout = tdLibWrapper->getOption("call_packet_timeout_ms").toDouble() / 1000,
        .receiveTimeout = tdLibWrapper->getOption("call_connect_timeout_ms").toDouble() / 1000,
        .enableP2P = protocol.value(UDP_P2P).toBool() && protocol.value("allow_p2p").toBool(),
        .maxApiLayer = protocol.value(MAX_LAYER).toInt(),
        .customParameters = state.value("custom_parameters").toString().toStdString()
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

    descriptor.stateUpdated = [this](tgcalls::State state) {
        if (currentCallState != TdCallState::Ready)
            return;

        currentCallReadyState = static_cast<CallReadyState>(state);

        LOG("State updated" << static_cast<int>(state) << static_cast<int>(currentCallState) << getCurrentCallState());
        emit currentCallStateChanged();
    };
    descriptor.signalBarsUpdated = [this](int bars) {
        LOG("Signal bars updated" << bars);
        const bool wasZero = signalBars == 0;
        this->signalBars = bars;
        emit signalBarsChanged();

        if (wasZero || bars == 0)
            emit currentCallStateChanged();
    };
    descriptor.audioLevelUpdated = [](float level) {
        LOG("Audio level updated" << level);
    };
    descriptor.remoteBatteryLevelIsLowUpdated = [this](bool value) {
        LOG("Remote battery level is low updated" << value);
        this->remoteBatteryLevelIsLow = value;
        emit remoteBatteryLevelIsLowChanged();
    };
    descriptor.remoteMediaStateUpdated = [this](tgcalls::AudioState audioState, tgcalls::VideoState videoState) {
        LOG("Remote media state updated audio:" << static_cast<int>(audioState) << "video:" << static_cast<int>(videoState));

        bool muted = audioState == tgcalls::AudioState::Muted;
        if (this->remoteAudioMuted != muted) {
            LOG("Remote audio muted changed" << muted);
            this->remoteAudioMuted = muted;
            emit remoteAudioMutedChanged();
        }
    };
    descriptor.remotePrefferedAspectRatioUpdated = [](float ratio) {
        LOG("Remote preferred aspect ratio updated" << ratio);
    };

    descriptor.signalingDataEmitted = [this](const std::vector<uint8_t> &data) {
        QByteArray bytes(reinterpret_cast<const char*>(data.data()), data.size());
        tdLibWrapper->sendCallSignalingData(currentCallId, bytes);
    };

    // TODO: setting to save call logs

    instance = tgcalls::Meta::Create(protocol.value("library_versions").toStringList().first().toStdString(), std::move(descriptor));
}

CallsManager::CallState CallsManager::getCurrentCallState() {
    switch (currentCallState) {
    case TdCallState::Ready:
        if (signalBars == 0)
            return CallState::Connecting;

        switch (currentCallReadyState) {
        case CallReadyState::WaitInit:
        case CallReadyState::WaitInitAck:
            return CallState::Connecting;
        case CallReadyState::Established:
            return CallState::Connected;
        case CallReadyState::Failed:
            return CallState::UnknownError;
        case CallReadyState::Reconnecting:
            return CallState::Connecting;
        default:
            return CallState::Connecting;
        }
    case TdCallState::Pending:
        return currentCallStateData.value("is_received").toBool() ? CallState::Ringing : CallState::Pending;
    case TdCallState::ExchangingKeys:
        return CallState::ExchangingKeys;
    case TdCallState::HangingUp:
        return CallState::HangingUp;
    case TdCallState::Discarded:
    {
        const QString discardReason = currentCallStateData.value("discard_reason").toMap().value(_TYPE).toString();
        if (discardReason == "callDiscardReasonDeclined")
            return CallState::Declined;
        if (discardReason == "callDiscardReasonDisconnected")
            return CallState::Disconnected;
        if (discardReason == "callDiscardReasonHungUp")
            return CallState::HungUp;
        return CallState::Discarded;
    }
    case TdCallState::Error:
        return CallState::Error;
    }

    return static_cast<CallState>(currentCallState);
}

QStringList CallsManager::currentCallEmojis() {
    if (currentCallState != TdCallState::Ready)
        return {};

    return currentCallStateData.value("emojis").toStringList();
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

    currentCallReadyState = CallReadyState::Reconnecting;

    emit callDiscarded();

    if (signalBars != 0) {
        signalBars = 0;
        emit signalBarsChanged();
    }
    if (remoteBatteryLevelIsLow) {
        remoteBatteryLevelIsLow = false;
        emit remoteBatteryLevelIsLowChanged();
    }
    if (remoteAudioMuted) {
        remoteAudioMuted = false;
        emit remoteAudioMutedChanged();
    }
}

void CallsManager::toggleSpeakerphone(bool enabled) {
    CallAudio::toggleSpeakerphone(enabled);
}
