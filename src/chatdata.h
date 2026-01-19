#ifndef CHATDATA_H
#define CHATDATA_H

#include <QObject>
#include "tdlib/tdlibwrapper.h"
#include "basemessagabledata.h"

class ChatData : public BaseMessagableData {
public:
    enum Role {
        RoleDisplay = Qt::DisplayRole,
        RoleChatId,
        RoleChatType,
        RoleGroupId,
        RoleTitle,
        RolePhotoSmall,
        RoleUnreadCount,
        RoleUnreadMentionCount,
        RoleUnreadReactionCount,
        RoleAvailableReactions,
        RoleLastReadInboxMessageId,
        RoleLastMessageSenderId,
        RoleLastMessageDate,
        RoleLastMessageText,
        RoleLastMessageMinithumbnail,
        RoleLastMessageIsService,
        RoleLastMessageStatus,
        RoleChatMemberStatus,
        RoleSecretChatState,
        RoleVerificationStatus,
        RoleIsChannel,
        RoleIsMarkedAsUnread,
        RoleIsPinned,
        RoleDraftMessageText,
        RoleDraftMessageDate
    };

    ChatData(TDLibWrapper *tdLibWrapper, Utilities *utilities, const QVariantMap &data);
    ChatData(TDLibWrapper *tdLibWrapper, Utilities *utilities, qlonglong chatId);

    void updateChatData(const QVariantMap &data);
    virtual const QVariantMap lastMessage() const override;
    virtual QString lastMessageStatus() const override;
    virtual const QVariantMap draftMessage() const override;
    QString title() const;
    int unreadCount() const;
    int unreadMentionCount() const;
    int unreadReactionCount() const;
    QVariant availableReactions() const;
    QVariant photoSmall() const;
    virtual qlonglong lastReadInboxMessageId() const override;
    virtual qlonglong lastReadOutboxMessageId() const override;

    bool isChannel() const;
    bool isMarkedAsUnread() const;
    bool updateUnreadCount(int unreadCount);
    bool updateLastReadInboxMessageId(qlonglong messageId);
    bool updateLastReadOutboxMessageId(qlonglong messageId);
    QVector<int> updateLastMessage(const QVariantMap &message);
    QVector<int> updateGroup(const TDLibWrapper::Group *group);
    QVector<int> updateSecretChat(const QVariantMap &secretChatDetails);

public:
    QVariantMap chatData;
    qlonglong chatId;
    qlonglong groupId;
    QVariantMap verificationStatus;
    TDLibWrapper::ChatType chatType;
    TDLibWrapper::ChatMemberStatus memberStatus;
    TDLibWrapper::SecretChatState secretChatState;
};

#endif // CHATDATA_H
