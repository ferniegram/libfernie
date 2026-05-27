#ifndef CALLSMANAGER_H
#define CALLSMANAGER_H

#include <QObject>

#include <tgcalls/Instance.h>

#include "tdlib/tdlibwrapper.h"
#include "settings.h"

class CallsManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(qlonglong currentCallUserId READ currentCallUserId NOTIFY currentCallUserIdChanged)
    Q_PROPERTY(CurrentCallState currentCallState READ getCurrentCallState NOTIFY currentCallStateChanged)
    Q_PROPERTY(int signalBars MEMBER signalBars NOTIFY signalBarsChanged) // 0-4
    Q_PROPERTY(bool remoteBatteryLevelIsLow MEMBER remoteBatteryLevelIsLow NOTIFY remoteBatteryLevelIsLowChanged)
    Q_PROPERTY(bool remoteAudioMuted MEMBER remoteAudioMuted NOTIFY remoteAudioMutedChanged)

    Q_PROPERTY(QStringList currentCallEmojis READ currentCallEmojis NOTIFY currentCallEmojisChanged)

public:
    enum class CallState {
        Pending,
        ExchangingKeys,
        HangingUp,
        Discarded,
        Error,
        Ready
    };

    enum class CurrentCallState {
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
    Q_ENUM(CurrentCallState)

    struct Call {
        qlonglong uniqueId;
        qlonglong userId;
        bool outgoing;
        bool video;
        CallState state;
        QVariantMap stateData;

        Call() {}
        Call(qlonglong uniqueId, qlonglong userId, bool outgoing, bool video, const QVariantMap &state);
        void update(qlonglong uniqueId, qlonglong userId, bool outgoing, bool video, const QVariantMap &state);
    };

    explicit CallsManager(TDLibWrapper *tdLibWrapper, Settings *settings, QObject *parent = nullptr);
    ~CallsManager();

    Q_INVOKABLE void createCall(qlonglong userId);
    Q_INVOKABLE void discardCurrentCall();
    Q_INVOKABLE void acceptCall(int callId);
    Q_INVOKABLE void discardCall(int callId);
    const Call *getCall(int callId);

    qlonglong currentCallUserId() const;
    CurrentCallState getCurrentCallState() const;
    Q_INVOKABLE QStringList currentCallEmojis() const;

    Q_INVOKABLE void toggleSpeakerphone(bool enabled);

private:
    enum class CallReadyState {
        WaitInit,
        WaitInitAck,
        Established,
        Failed,
        Reconnecting
    };

signals:
    void pendingIncomingCall(int callId);
    void incomingCallNotPending(int callId);

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
    static CallState getTdCallState(const QString &type);
    void resetInstance();
    void setCurrentCallId(int id);
    void handleCallReady();
    void handleCallDiscarded();

private:
    TDLibWrapper *tdLibWrapper;
    Settings *settings;

    QHash<int, Call*> activeCalls;
    qlonglong currentCallId = 0;
    CallReadyState currentCallReadyState = CallReadyState::Reconnecting;
    int signalBars = 0;
    bool remoteBatteryLevelIsLow = false;
    bool remoteAudioMuted = false;

    std::unique_ptr<tgcalls::Instance> instance;
};

#endif // CALLSMANAGER_H
