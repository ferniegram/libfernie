#include "tdlibstate.h"

#define DEBUG_MODULE TDLibState
#include "debuglog.h"

TDLibState::TDLibState(TDLibReceiver *tdLibReceiver, TDLibWrapper *tdLibWrapper, QObject *parent) :
    QObject(parent),
    tdLibWrapper(tdLibWrapper),
    networkConfigurationManager(new QNetworkConfigurationManager(this))
{
    connect(tdLibWrapper, &TDLibWrapper::clearContent, this, &TDLibState::reset);

    connect(tdLibReceiver, &TDLibReceiver::connectionStateChanged, this, &TDLibState::handleConnectionStateChanged);

    connect(networkConfigurationManager, &QNetworkConfigurationManager::configurationChanged, this, &TDLibState::handleNetworkConfigurationChanged);
}

void TDLibState::reset() {
    LOG("Resetting");
}

QString TDLibState::connectionStateText() {
    switch (connectionState) {
    case WaitingForNetwork:
        return tr("Waiting for network...");
    case Connecting:
        return tr("Connecting to network...");
    case ConnectingToProxy:
        return tr("Connecting to proxy...");
    case Updating:
        return tr("Updating content...");
    default:
        return QString();
    }
}

void TDLibState::handleConnectionStateChanged(const QString &connectionState) {
    if (connectionState == "connectionStateConnecting")
        this->connectionState = ConnectionState::Connecting;
    else if (connectionState == "connectionStateConnectingToProxy")
        this->connectionState = ConnectionState::ConnectingToProxy;
    else if (connectionState == "connectionStateReady")
        this->connectionState = ConnectionState::ConnectionReady;
    else if (connectionState == "connectionStateUpdating")
        this->connectionState = ConnectionState::Updating;
    else if (connectionState == "connectionStateWaitingForNetwork")
        this->connectionState = ConnectionState::WaitingForNetwork;

    emit connectionStateChanged();
}

void TDLibState::handleNetworkConfigurationChanged(const QNetworkConfiguration &config) {
    LOG("A network configuration changed, updating network type" << config.bearerTypeName() << config.state());

    QList<QNetworkConfiguration> activeConfigurations = networkConfigurationManager->allConfigurations(QNetworkConfiguration::Active);
    for (const QNetworkConfiguration &configuration : activeConfigurations) {
        switch (configuration.bearerTypeFamily()) {
        case QNetworkConfiguration::BearerWLAN:
            tdLibWrapper->setNetworkType(TDLibWrapper::NetworkType::WiFi);
            return;
        case QNetworkConfiguration::Bearer2G:
        case QNetworkConfiguration::Bearer3G:
        case QNetworkConfiguration::Bearer4G:
            tdLibWrapper->setNetworkType(TDLibWrapper::NetworkType::Mobile);
            return;
        case QNetworkConfiguration::BearerEthernet:
            tdLibWrapper->setNetworkType(TDLibWrapper::NetworkType::Other);
            return;
        default:
            break;
        }
    }

    tdLibWrapper->setNetworkType(TDLibWrapper::NetworkType::None);
}
