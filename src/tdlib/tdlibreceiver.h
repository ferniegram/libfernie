/*
    Copyright (C) 2020 Sebastian J. Wolf and other contributors

    This file is part of Fernschreiber.

    Fernschreiber is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Fernschreiber is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Fernschreiber. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef TDLIBRECEIVER_H
#define TDLIBRECEIVER_H

#include <QHash>
#include <QVariantMap>
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include <td/telegram/td_json_client.h>
#include "waveformmanager.h"

class TDLibReceiver : public QThread {
    Q_OBJECT
    void run() Q_DECL_OVERRIDE {
        receiverLoop();
    }
public:
    explicit TDLibReceiver(int tdLibClientId, QObject *parent = nullptr);
    void setActive(bool active);

signals:
    void responseForRequestIdReceived(qlonglong requestId, const QVariantMap &response);
    void authorizationStateChanged(const QString &authorizationState, const QVariantMap &authorizationStateData);
    void optionUpdated(const QString &optionName, const QVariant &optionValue);
    void connectionStateChanged(const QString &connectionState);
    void userUpdated(const QVariantMap &userInformation);
    void userStatusUpdated(qlonglong userId, const QVariantMap &userStatusInformation);
    void fileUpdated(const QVariantMap &fileInformation);
    void newChatDiscovered(const QVariantMap &chatInformation);
    void chatAddedToList(const QVariantMap &chatList, qlonglong chatId);
    void chatRemovedFromList(const QVariantMap &chatList, qlonglong chatId);
    void unreadMessageCountUpdated(const QVariantMap &messageCountInformation);
    void unreadChatCountUpdated(const QVariantMap &chatCountInformation);
    void chatLastMessageUpdated(qlonglong chatId, const QVariantMap &lastMessage, const QVariantList &positions);
    void chatPositionUpdated(qlonglong chatId, const QVariantMap &position);
    void chatReadInboxUpdated(const QString &chatId, const QString &lastReadInboxMessageId, int unreadCount);
    void chatReadOutboxUpdated(const QString &chatId, const QString &lastReadOutboxMessageId);
    void chatAvailableReactionsUpdated(qlonglong chatId, const QVariantMap &availableReactions);
    void basicGroupUpdated(qlonglong groupId, const QVariantMap &groupInformation);
    void supergroupUpdated(qlonglong groupId, const QVariantMap &groupInformation);
    void chatOnlineMemberCountUpdated(const QString &chatId, int onlineMemberCount);
    void messagesReceived(qlonglong chatId, int extra, const QVariantList &messages, int totalCount);
    void foundChatMessagesReceived(qlonglong chatId, int extra, int extra2, const QVariantList &messages, int totalCount, qlonglong nextFromMessageId);
    void messageLinkInfoReceived(qlonglong chatId, qlonglong messageId);
    void sponsoredMessagesReceived(qlonglong chatId, const QVariantList &messages, int messagesBetween);
    void newMessageReceived(qlonglong chatId, const QVariantMap &message);
    void messageInformation(qlonglong chatId, qlonglong messageId, const QVariantMap &message);
    void messageSendSucceeded(qlonglong chatId, qlonglong oldMessageId, qlonglong messageId, const QVariantMap &message);
    void activeNotificationsUpdated(const QVariantList notificationGroups);
    void notificationGroupUpdated(const QVariantMap notificationGroupUpdate);
    void notificationUpdated(const QVariantMap updatedNotification);
    void chatNotificationSettingsUpdated(const QString &chatId, const QVariantMap updatedChatNotificationSettings);
    void messageContentUpdated(qlonglong chatId, qlonglong messageId, const QVariantMap &newContent);
    void messageEditedUpdated(qlonglong chatId, qlonglong messageId, int editDate, const QVariantMap &replyMarkup);
    void messagesDeleted(qlonglong chatId, const QList<qlonglong> &messageIds);
    void chats(const QString &extra, const QVariantList &chatIds, const int totalCount);
    void sponsoredChatsReceived(const QVariantList &chats);
    void chat(const QVariantMap &chats);
    void recentStickersUpdated(bool isAttached, const QList<int> &stickerIds);
    void favoriteStickersUpdated(const QList<int> &stickerIds);
    void stickers(const QVariantList &stickers, const QString &extra);
    void installedStickerSetsUpdated(const QString &stickerType, const QVariantList &stickerSetIds);
    void stickerSets(const QVariantList &stickerSets, int totalCount, const QString &extra);
    void stickerSet(const QString &stickerSetId, const QVariantMap &stickerSet);
    void chatMembers(qlonglong chatId, const QVariantList &members, int totalMembers);
    void userFullInfo(qlonglong userId, const QVariantMap &userFullInfo);
    void userFullInfoUpdated(qlonglong userId, const QVariantMap &userFullInfo);
    void basicGroupFullInfo(qlonglong groupId, const QVariantMap &groupFullInfo);
    void basicGroupFullInfoUpdated(qlonglong groupId, const QVariantMap &groupFullInfo);
    void supergroupFullInfo(qlonglong groupId, const QVariantMap &groupFullInfo);
    void supergroupFullInfoUpdated(qlonglong groupId, const QVariantMap &groupFullInfo);
    void chatPhotos(qlonglong chatId, const QVariantList &photos, int totalCount);
    void chatPermissionsUpdated(qlonglong chatId, const QVariantMap &chatPermissions);
    void chatPhotoUpdated(qlonglong chatId, const QVariantMap &photo);
    void chatTitleUpdated(qlonglong chatId, const QString &title);
    void messageIsPinnedUpdated(qlonglong chatId, qlonglong messageId, bool isPinned);
    void usersReceived(const QString &extra, const QVariantList &senders, int totalCount);
    void messageSendersReceived(const QString &extra, const QVariantList &messageSenders, int totalCount);
    void errorReceived(const int code, const QString &message, const QVariant &extra);
    void serviceNotificationReceived(const QString &type, const QVariantMap &content);
    void secretChat(qlonglong secretChatId, const QVariantMap &secretChat);
    void secretChatUpdated(qlonglong secretChatId, const QVariantMap &secretChat);
    void contactsImported(const QVariantList &importerCount, const QVariantList &userIds, bool single);
    void chatIsMarkedAsUnreadUpdated(qlonglong chatId, bool chatIsMarkedAsUnread);
    void chatDraftMessageUpdated(qlonglong chatId, const QVariantMap &draftMessage, const QVariantList &positions);
    void inlineQueryResults(const QString &inlineQueryId, const QString &nextOffset, const QVariantList &results, const QString &switchPmText, const QString &switchPmParameter, const QString &extra);
    void callbackQueryAnswer(const QString &text, bool alert, const QString &url);
    void userPrivacySettingRules(const QVariantMap &rules);
    void userPrivacySettingRulesUpdated(const QVariantMap &updatedRules);
    void messageInteractionInfoUpdated(qlonglong chatId, qlonglong messageId, const QVariantMap &updatedInfo);
    void okReceived(const QString &extra);
    void okMapReceived(const QString &type, const QVariantMap &extra);
    void sessionsReceived(int inactive_session_ttl_days, const QVariantList &sessions);
    void availableReactionsReceived(qlonglong messageId, const QStringList &reactions);
    void chatUnreadMentionCountUpdated(qlonglong chatId, int unreadMentionCount);
    void messageMentionRead(qlonglong chatId, qlonglong messageId);
    void chatUnreadReactionCountUpdated(qlonglong chatId, int unreadReactionCount);
    void activeEmojiReactionsUpdated(const QStringList &emojis);
    void messagePropertiesReceived(qlonglong chatId, qlonglong messageId, const QVariantMap &messageProperties);
    void storageStatisticsFastReceived(const QVariantMap &statistics);
    void storageStatisticsReceived(const QVariantMap &statistics);
    void formattedTextReceived(const QVariantMap &formattedText, const QString &extra);
    void chatActionUpdated(qlonglong chatId, const QVariantMap &sender, const QVariantMap &action, qlonglong messageThreadId);
    void emojiKeywordsReceived(const QString &text, const QVariantList &emojis);
    void diceEmojisUpdated(const QStringList &emojis);
    void suggestedActionsUpdated(const QVariantList &added, const QVariantList &removed);
    void countReceived(int count, const QString &extra);
    void chatListsReceived(qlonglong chatId, const QVariantList &chatLists);
    void archiveChatListSettingsReceived(bool archiveAndMuteNewChatsFromUnknownUsers, bool keepUnmutedChatsArchived, bool keepChatsFromFoldersArchived);
    void chatFoldersUpdated(const QVariantList &chatFolders, int mainChatListPosition, bool tagsEnabled);
    void forumTopicsReceived(qlonglong chatId, int totalCount, QVariantList topics, int nextOffsetDate, qlonglong nextOffsetMessageId, int nextOffsetForumTopicId);
    void forumTopicUpdated(qlonglong chatId, int forumTopicId, const QVariantMap &update);
    void forumTopicInfoUpdated(qlonglong chatId, int forumTopicId, const QVariantMap &info);
    void chatPendingJoinRequestsUpdated(qlonglong chatId, const QVariantMap &pendingJoinRequests);
    void chatJoinRequestsReceived(qlonglong chatId, int totalCount, const QVariantList &requests);
    void internalLinkTypeReceived(const QVariantMap &internalLinkType);
    void deepLinkInfoReceived(const QVariantMap &text, bool needUpdateApplication);
    void userReceived(const QVariantMap &user, bool doOpenOnFound);
    void chatInviteLinkInfoReceived(const QString &link, const QVariantMap &info);
    void chatViewAsTopicsUpdated(qlonglong chatId, bool viewAsTopics);
    void threadMessagesReceived(qlonglong chatId, qlonglong messageId, int extra, const QVariantList &messages, int totalCount);
    void forumTopicMessagesReceived(qlonglong chatId, int forumTopicId, int extra, const QVariantList &messages, int totalCount);
    void forumTopicReceived(qlonglong chatId, int forumTopicId, const QVariantMap &topic);
    void messageSuggestedPostInfoUpdated(qlonglong chatId, qlonglong messageId, const QVariantMap &suggestedPostInfo);
    void messageContentOpened(qlonglong chatId, qlonglong messageId);
    void messageFactCheckUpdated(qlonglong chatId, qlonglong messageId, const QVariantMap &factCheck);
    void stickerSetUpdated(const QString &stickerSetId, const QVariantMap &stickerSet);
    void pollVotersReceived(const QString &extra, const QVariantList &voters, int totalCount);

private:
    typedef void (TDLibReceiver::*Handler)(const QVariantMap &);

    QHash<QString, Handler> handlers;
    int tdLibClientId;
    bool isActive;

private:
    static const QVariantList cleanupList(const QVariantList& list, bool *updated = Q_NULLPTR);
    static const QVariantMap cleanupMap(const QVariantMap& data, bool *updated = Q_NULLPTR);
    void receiverLoop();
    void ok(const QVariantMap &receivedInformation);
    void processReceivedDocument(const QJsonDocument &receivedJsonDocument);
    void processUpdateOption(const QVariantMap &receivedInformation);
    void processUpdateAuthorizationState(const QVariantMap &receivedInformation);
    void processUpdateConnectionState(const QVariantMap &receivedInformation);
    void processUpdateUser(const QVariantMap &receivedInformation);
    void processUpdateUserStatus(const QVariantMap &receivedInformation);
    void processUpdateFile(const QVariantMap &receivedInformation);
    void processFile(const QVariantMap &receivedInformation);
    void processUpdateNewChat(const QVariantMap &receivedInformation);
    void processUpdateChatAddedToList(const QVariantMap &receivedInformation);
    void processUpdateChatRemovedFromList(const QVariantMap &receivedInformation);
    void processUpdateUnreadMessageCount(const QVariantMap &receivedInformation);
    void processUpdateUnreadChatCount(const QVariantMap &receivedInformation);
    void processUpdateChatLastMessage(const QVariantMap &receivedInformation);
    void processUpdateChatPosition(const QVariantMap &receivedInformation);
    void processUpdateChatReadInbox(const QVariantMap &receivedInformation);
    void processUpdateChatReadOutbox(const QVariantMap &receivedInformation);
    void processUpdateChatAvailableReactions(const QVariantMap &receivedInformation);
    void processUpdateBasicGroup(const QVariantMap &receivedInformation);
    void processUpdateSuperGroup(const QVariantMap &receivedInformation);
    void processChatOnlineMemberCountUpdated(const QVariantMap &receivedInformation);
    void processMessages(const QVariantMap &receivedInformation);
    void processFoundChatMessages(const QVariantMap &receivedInformation);
    void processSponsoredMessages(const QVariantMap &receivedInformation);
    void processUpdateNewMessage(const QVariantMap &receivedInformation);
    void processMessage(const QVariantMap &receivedInformation);
    void processMessageLinkInfo(const QVariantMap &receivedInformation);
    void processMessageSendSucceeded(const QVariantMap &receivedInformation);
    void processUpdateActiveNotifications(const QVariantMap &receivedInformation);
    void processUpdateNotificationGroup(const QVariantMap &receivedInformation);
    void processUpdateNotification(const QVariantMap &receivedInformation);
    void processUpdateChatNotificationSettings(const QVariantMap &receivedInformation);
    void processUpdateMessageContent(const QVariantMap &receivedInformation);
    void processUpdateDeleteMessages(const QVariantMap &receivedInformation);
    void processChats(const QVariantMap &receivedInformation);
    void processSponsoredChats(const QVariantMap &receivedInformation);
    void processChat(const QVariantMap &receivedInformation);
    void processUpdateRecentStickers(const QVariantMap &receivedInformation);
    void processUpdateFavoriteStickers(const QVariantMap &receivedInformation);
    void processStickers(const QVariantMap &receivedInformation);
    void processUpdateInstalledStickerSets(const QVariantMap &receivedInformation);
    void processStickerSets(const QVariantMap &receivedInformation);
    void processStickerSet(const QVariantMap &receivedInformation);
    void processChatMembers(const QVariantMap &receivedInformation);
    void processUserFullInfo(const QVariantMap &receivedInformation);
    void processUpdateUserFullInfo(const QVariantMap &receivedInformation);
    void processBasicGroupFullInfo(const QVariantMap &receivedInformation);
    void processUpdateBasicGroupFullInfo(const QVariantMap &receivedInformation);
    void processSupergroupFullInfo(const QVariantMap &receivedInformation);
    void processUpdateSupergroupFullInfo(const QVariantMap &receivedInformation);
    void processChatPhotos(const QVariantMap &receivedInformation);
    void processUpdateChatPermissions(const QVariantMap &receivedInformation);
    void processUpdateChatPhoto(const QVariantMap &receivedInformation);
    void processUpdateChatTitle(const QVariantMap &receivedInformation);
    void processUpdateChatPinnedMessage(const QVariantMap &receivedInformation);
    void processUpdateMessageIsPinned(const QVariantMap &receivedInformation);
    void processUsers(const QVariantMap &receivedInformation);
    void processMessageSenders(const QVariantMap &receivedInformation);
    void processError(const QVariantMap &receivedInformation);
    void processUpdateServiceNotification(const QVariantMap &receivedInformation);
    void processSecretChat(const QVariantMap &receivedInformation);
    void processUpdateSecretChat(const QVariantMap &receivedInformation);
    void processUpdateMessageEdited(const QVariantMap &receivedInformation);
    void processImportedContacts(const QVariantMap &receivedInformation);
    void processUpdateChatIsMarkedAsUnread(const QVariantMap &receivedInformation);
    void processUpdateChatDraftMessage(const QVariantMap &receivedInformation);
    void processInlineQueryResults(const QVariantMap &receivedInformation);
    void processCallbackQueryAnswer(const QVariantMap &receivedInformation);
    void processUserPrivacySettingRules(const QVariantMap &receivedInformation);
    void processUpdateUserPrivacySettingRules(const QVariantMap &receivedInformation);
    void processUpdateMessageInteractionInfo(const QVariantMap &receivedInformation);
    void processSessions(const QVariantMap &receivedInformation);
    void processAvailableReactions(const QVariantMap &receivedInformation);
    void processUpdateChatUnreadMentionCount(const QVariantMap &receivedInformation);
    void processUpdateMessageMentionRead(const QVariantMap &receivedInformation);
    void processUpdateChatUnreadReactionCount(const QVariantMap &receivedInformation);
    void processUpdateActiveEmojiReactions(const QVariantMap &receivedInformation);
    void processMessageProperties(const QVariantMap &receivedInformation);
    void processStorageStatisticsFast(const QVariantMap &receivedInformation);
    void processStorageStatistics(const QVariantMap &receivedInformation);
    void processFormattedText(const QVariantMap &receivedInformation);
    void processUpdateChatAction(const QVariantMap &receivedInformation);
    void processEmojiKeywords(const QVariantMap &receivedInformation);
    void processUpdateDiceEmojis(const QVariantMap &receivedInformation);
    void processUpdateSuggestedActions(const QVariantMap &receivedInformation);
    void processCount(const QVariantMap &receivedInformation);
    void processChatLists(const QVariantMap &receivedInformation);
    void processArchiveChatListSettings(const QVariantMap &receivedInformation);
    void processUpdateChatFolders(const QVariantMap &receivedInformation);
    void processForumTopics(const QVariantMap &receivedInformation);
    void processUpdateForumTopic(const QVariantMap &receivedInformation);
    void processUpdateForumTopicInfo(const QVariantMap &receivedInformation);
    void processUpdateChatPendingJoinRequests(const QVariantMap &receivedInformation);
    void processChatJoinRequests(const QVariantMap &receivedInformation);
    void processInternalLinkType(const QVariantMap &receivedInformation);
    void processDeepLinkInfo(const QVariantMap &receivedInformation);
    void processUser(const QVariantMap &receivedInformation);
    void processChatInviteLinkInfo(const QVariantMap &receivedInformation);
    void processUpdateChatViewAsTopics(const QVariantMap &receivedInformation);
    void processForumTopic(const QVariantMap &receivedInformation);
    void processUpdateMessageSuggestedPostInfo(const QVariantMap &receivedInformation);
    void processUpdateMessageContentOpened(const QVariantMap &receivedInformation);
    void processUpdateMessageFactCheck(const QVariantMap &receivedInformation);
    void processUpdateStickerSet(const QVariantMap &receivedInformation);
    void processPollVoters(const QVariantMap &receivedInformation);
};

#endif // TDLIBRECEIVER_H
