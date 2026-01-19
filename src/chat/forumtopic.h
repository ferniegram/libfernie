#ifndef FORUMTOPIC_H
#define FORUMTOPIC_H

#include "tdlib/tdlibwrapper.h"
#include "basemessagabledata.h"

struct ForumTopic : public BaseMessagableData {
    enum Role {
        RoleDisplay = Qt::DisplayRole,
        RoleId,
        RoleName,
        RoleIconColor,
        RoleIconCustomEmojiId,
        RoleCreationDate,
        RoleCreatorIsChat,
        RoleCreatorUserId,
        RoleCreatorChatId,
        RoleIsGeneral,
        RoleIsOutgoing,
        RoleIsClosed,
        RoleIsHidden,
        RoleIsNameImplicit,
        RoleLastMessageId,
        RoleLastMessageSenderId,
        RoleLastMessageDate,
        RoleLastMessageText,
        RoleLastMessageMinithumbnail,
        RoleLastMessageIsService,
        RoleLastMessageStatus,
        RoleIsPinned,
        RoleUnreadCount,
        RoleLastReadInboxMessageId,
        RoleUnreadMentionCount,
        RoleUnreadReactionCount,
        RoleNotificationSettings,
        RoleDraftMessageText,
        RoleDraftMessageDate,
    };

    ForumTopic(TDLibWrapper *tdLibWrapper, Utilities *utilities, const QVariantMap &forumTopic);

    static bool lessThan(const ForumTopic *topic1, const ForumTopic *topic2);

    QVariantMap info() const;
    inline QVariant info(const QString &key) const {
        return info().value(key);
    }

    bool isPinned() const;
    int unreadCount() const;
    int unreadMentionCount() const;
    int unreadReactionCount() const;
    virtual qlonglong lastReadInboxMessageId() const override;
    virtual qlonglong lastReadOutboxMessageId() const override;
    const QVariantMap notificationSettings() const;

    virtual const QVariantMap lastMessage() const override;
    qlonglong lastMessageId() const;
    virtual const QVariantMap draftMessage() const override;

    const QVector<int> updateIsPinned(bool value);
    const QVector<int> updateLastReadInboxMessageId(qlonglong value);
    const QVector<int> updateLastReadOutboxMessageId(qlonglong value);
    const QVector<int> updateLastMessage(const QVariantMap &message);
    const QVector<int> updateLastMessageContent(const QVariantMap &content);
    const QVector<int> updateUnreadCount(int value);
    const QVector<int> updateUnreadMentionCount(int value);
    const QVector<int> updateUnreadReactionCount(int value);
    const QVector<int> updateNotificationSettings(const QVariantMap &value);
    const QVector<int> updateDraftMessage(const QVariantMap &value);

    const QVector<int> updateFromForumTopicUpdate(const QVariantMap &update);
    const QVector<int> updateForumTopicInfo(const QVariantMap &newInfo);
    const QVector<int> updateForumTopicData(const QVariantMap &topic);

    QVariantMap data;
    int id;
};

#endif // FORUMTOPIC_H
