#pragma once

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
    Q_INVOKABLE virtual int calculateScrollPosition() const override;

signals:
    void newMessageReceived(const QVariantMap &message);
    void unreadCountUpdated(int unreadCount, const QString &lastReadInboxMessageId);

    void lastReadSentMessageUpdated();
    void lastReadMessageIndexChanged();

protected slots:
    void handleNewMessageReceived(const QVariantMap &message);

protected:
    int calculateLastReadMessageIndexInBounds() const;

    int getLastReadMessageIndex() const;
    int calculateLastReadSentMessageIndex() const;

    virtual void loadMoreHistoryImpl() override;
    virtual void loadMoreFutureImpl() override;
    virtual void loadHistoryForMessageImpl(qlonglong messageId) override;

    virtual qlonglong lastReadInboxMessageId() const = 0;
    virtual qlonglong lastReadOutboxMessageId() const = 0;
    virtual qlonglong lastMessageId() const = 0;

    virtual void processMessageData(MessageData *message) override;


    // Proper isFirst/LastInSequence handling
    virtual void removeRange(int firstDeleted, int lastDeleted, bool updateAlbums = true) override;
    virtual void insertMessagesAt(int index, const QList<MessageData*> newMessages) override;
    virtual void appendMessages(const QList<MessageData*> newMessages) override;
    virtual void prependMessages(const QList<MessageData*> newMessages) override;

    virtual bool messageIsFirstInSequence(const int index, const MessageData *message) const override;
    virtual bool messageIsLastInSequence(const int index, const MessageData *message) const override;

protected slots:
    virtual void handlePrepareMessagesReceived(int totalCount, UpdateType fromUpdate) override;

protected:
    bool loadingFullEnd;
};
