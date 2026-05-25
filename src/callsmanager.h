#ifndef CALLSMANAGER_H
#define CALLSMANAGER_H

#include <QObject>

#include <tgcalls/Instance.h>

#include "tdlib/tdlibwrapper.h"

class CallsManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(CallState currentCallState MEMBER currentCallState NOTIFY currentCallStateChanged)

public:
    // FIXME: is this actually needed?
    enum class CallState {
        Pending,
        ExchangingKeys,
        Ready,
        HangingUp,
        Discarded,
        Error
    };
    Q_ENUM(CallState)

    explicit CallsManager(TDLibWrapper *tdLibWrapper, QObject *parent = nullptr);

    Q_INVOKABLE void createCall(qlonglong userId);
    Q_INVOKABLE void discardCurrentCall();

signals:
    void currentCallStateChanged();
    void callStarted();
    void callDiscarded();

private slots:
    void handleCallIdReceived(int id);
    void handleCallUpdated(int id, qlonglong uniqueId, qlonglong userId, bool outgoing, bool video, const QVariantMap &state);
    void handleNewCallSignalingDataReceived(int callId, const QByteArray &data);

private:
    static QVariantMap protocol();
    static CallState getCallState(const QString &type);
    void handleCallReady(bool outgoing, const QVariantMap &state);
    void handleCallDiscarded();

private:
    TDLibWrapper *tdLibWrapper;

    qlonglong currentCallId = 0;
    CallState currentCallState = CallState::Discarded;

    std::unique_ptr<tgcalls::Instance> instance;
};

#endif // CALLSMANAGER_H
