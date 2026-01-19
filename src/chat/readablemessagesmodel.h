#ifndef READABLEMESSAGESMODEL_H
#define READABLEMESSAGESMODEL_H

#include "jumpablemessagesmodel.h"

class ReadableMessagesModel : public JumpableMessagesModel {
    Q_OBJECT
    Q_PROPERTY(int lastReadMessageIndexInBounds READ calculateLastReadMessageIndexInBounds NOTIFY lastReadMessageIndexChanged)
    Q_PROPERTY(int lastReadIncomingMessageIndex READ getLastReadMessageIndex NOTIFY lastReadMessageIndexChanged)

    Q_PROPERTY(int lastReadSentMessageIndex READ calculateLastReadSentMessageIndex NOTIFY lastReadSentMessageUpdated)

public:
    ReadableMessagesModel(QObject *parent = nullptr);
    ReadableMessagesModel(TDLibWrapper *tdLibWrapper, QObject *parent = nullptr);

    Q_INVOKABLE virtual bool clear() override;
    Q_INVOKABLE void loadEnd(bool markAllAsRead = false);
    Q_INVOKABLE virtual int calculateScrollPosition() override;

signals:
    void newMessageReceived(const QVariantMap &message);
    void unreadCountUpdated(int unreadCount, const QString &lastReadInboxMessageId);

    void lastReadSentMessageUpdated();
    void lastReadMessageIndexChanged();

protected slots:
    void handleNewMessageReceived(const QVariantMap &message);

protected:
    int calculateLastReadMessageIndexInBounds();

    int getLastReadMessageIndex();
    int calculateLastReadSentMessageIndex();

    virtual void loadMoreHistoryImpl() override;
    virtual void loadMoreFutureImpl() override;
    virtual void loadHistoryForMessageImpl(qlonglong messageId) override;

    virtual qlonglong lastReadInboxMessageId() const = 0;
    virtual qlonglong lastReadOutboxMessageId() const = 0;
    virtual qlonglong lastMessageId() const = 0;

    virtual void processMessageData(MessageData *message) override;

protected slots:
    virtual void handlePrepareMessagesReceived(int totalCount, UpdateType fromUpdate) override;

protected:
    bool loadingFullEnd;
};

#endif // READABLEMESSAGESMODEL_H
