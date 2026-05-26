#ifndef CALLSMANAGER_H
#define CALLSMANAGER_H

#include <QObject>

#include <tgcalls/Instance.h>

#include "tdlib/tdlibwrapper.h"

class CallsManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(qlonglong currentCallUserId MEMBER currentCallUserId NOTIFY currentCallUserIdChanged)
    Q_PROPERTY(CallState currentCallState READ getCurrentCallState NOTIFY currentCallStateChanged)
    Q_PROPERTY(int signalBars MEMBER signalBars NOTIFY signalBarsChanged) // 0-4
    Q_PROPERTY(bool remoteBatteryLevelIsLow MEMBER remoteBatteryLevelIsLow NOTIFY remoteBatteryLevelIsLowChanged)
    Q_PROPERTY(bool remoteAudioMuted MEMBER remoteAudioMuted NOTIFY remoteAudioMutedChanged)

    Q_PROPERTY(QStringList currentCallEmojis READ currentCallEmojis NOTIFY currentCallEmojisChanged)

public:
    enum class CallState {
        Pending,
        Ringing,
        ExchangingKeys,
        HangingUp,
        Declined,
        Disconnected,
        HungUp,
        Discarded,
        Error,

        Connecting,
        Connected,
        UnknownError
    };
    Q_ENUM(CallState)

    explicit CallsManager(TDLibWrapper *tdLibWrapper, QObject *parent = nullptr);
    ~CallsManager();

    Q_INVOKABLE void createCall(qlonglong userId);
    Q_INVOKABLE void discardCurrentCall();

    CallState getCurrentCallState();
    Q_INVOKABLE QStringList currentCallEmojis();

    Q_INVOKABLE void toggleSpeakerphone(bool enabled);

private:
    enum class TdCallState {
        Pending,
        ExchangingKeys,
        HangingUp,
        Discarded,
        Error,
        Ready
    };

    enum class CallReadyState {
        WaitInit,
        WaitInitAck,
        Established,
        Failed,
        Reconnecting
    };

signals:
    void currentCallUserIdChanged();
    void currentCallStateChanged();
    void callStarted();
    void callDiscarded();
    void signalBarsChanged();
    void remoteBatteryLevelIsLowChanged();
    void remoteAudioMutedChanged();
    void currentCallEmojisChanged();

private slots:
    void handleCallIdReceived(int id);
    void handleCallUpdated(int id, qlonglong uniqueId, qlonglong userId, bool outgoing, bool video, const QVariantMap &state);
    void handleNewCallSignalingDataReceived(int callId, const QByteArray &data);

private:
    static QVariantMap protocol();
    static TdCallState getTdCallState(const QString &type);
    void handleCallReady(bool outgoing, const QVariantMap &state);
    void handleCallDiscarded();

private:
    TDLibWrapper *tdLibWrapper;

    qlonglong currentCallId = 0;
    qlonglong currentCallUserId = 0;
    TdCallState currentCallState = TdCallState::Discarded;
    QVariantMap currentCallStateData;
    CallReadyState currentCallReadyState = CallReadyState::Reconnecting;
    int signalBars = 0;
    bool remoteBatteryLevelIsLow = false;
    bool remoteAudioMuted = false;

    std::unique_ptr<tgcalls::Instance> instance;
};

#endif // CALLSMANAGER_H
