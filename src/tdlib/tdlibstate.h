#ifndef TDLIBSTATE_H
#define TDLIBSTATE_H

#include "tdlibwrapper.h"

class TDLibState : public QObject {
    Q_OBJECT

    Q_PROPERTY(ConnectionState connectionState MEMBER connectionState NOTIFY connectionStateChanged)
    Q_PROPERTY(QString connectionStateText READ connectionStateText NOTIFY connectionStateChanged)

public:
    explicit TDLibState(TDLibReceiver *tdLibReceiver, TDLibWrapper *tdLibWrapper, QObject *parent = nullptr);

    enum ConnectionState {
        Connecting,
        ConnectingToProxy,
        ConnectionReady,
        Updating,
        WaitingForNetwork
    };
    Q_ENUM(ConnectionState)

    QString connectionStateText();

signals:
    void connectionStateChanged();

private slots:
    void reset();

    void handleConnectionStateChanged(const QString &connectionState);

    void handleNetworkConfigurationChanged(const QNetworkConfiguration &config);

private:
    TDLibWrapper *tdLibWrapper;
    QNetworkConfigurationManager *networkConfigurationManager;
    ConnectionState connectionState;
};

#endif // TDLIBSTATE_H
