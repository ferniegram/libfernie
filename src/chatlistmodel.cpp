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

#include "chatlistmodel.h"
#include <QListIterator>

#define DEBUG_MODULE ChatListModel
#include "debuglog.h"

namespace {
    const QString ID("id");
    const QString DATE("date");
    const QString TEXT("text");
    const QString TYPE("type");
    const QString TITLE("title");
    const QString PHOTO("photo");
    const QString SMALL("small");
    const QString ORDER("order");
    const QString CHAT_ID("chat_id");
    const QString CONTENT("content");
    const QString LAST_MESSAGE("last_message");
    const QString DRAFT_MESSAGE("draft_message");
    const QString SENDER_ID("sender_id");
    const QString USER_ID("user_id");
    const QString BASIC_GROUP_ID("basic_group_id");
    const QString SUPERGROUP_ID("supergroup_id");
    const QString UNREAD_COUNT("unread_count");
    const QString UNREAD_MENTION_COUNT("unread_mention_count");
    const QString UNREAD_REACTION_COUNT("unread_reaction_count");
    const QString AVAILABLE_REACTIONS("available_reactions");
    const QString NOTIFICATION_SETTINGS("notification_settings");
    const QString LAST_READ_INBOX_MESSAGE_ID("last_read_inbox_message_id");
    const QString LAST_READ_OUTBOX_MESSAGE_ID("last_read_outbox_message_id");
    const QString SENDING_STATE("sending_state");
    const QString IS_CHANNEL("is_channel");
    const QString VERIFICATION_STATUS("verification_status");
    const QString IS_MARKED_AS_UNREAD("is_marked_as_unread");
    const QString PINNED_MESSAGE_ID("pinned_message_id");
    const QString _TYPE("@type");
    const QString SECRET_CHAT_ID("secret_chat_id");
    const QString UNREAD_UNMUTED_COUNT("unread_unmuted_count");
}

ChatListModel::ListChatData::ListChatData(ChatData *data, qlonglong order, bool isPinned) : data(data), order(order), isPinned(isPinned) {

}

bool ChatListModel::ListChatData::setOrder(const QVariant &newOrder) {
    if (newOrder.isValid()) {
        //chatData.insert(ORDER, newOrder); // is this really needed?
        order = newOrder.toLongLong();
        return true;
    }
    return false;
}

int ChatListModel::ListChatData::compareTo(const ListChatData *other) const {
    if (order == other->order)
        return (data->chatId < other->data->chatId) ? 1 : -1;
    else
        // This puts most recent ones to the top of the list
        return (order < other->order) ? 1 : -1;
}

ChatListModel::ChatListModel(TDLibWrapper *tdLibWrapper, Settings *settings, Utilities *utilities, bool archive, bool doNotConnectChatListSignals) :
    tdLibWrapper(tdLibWrapper),
    utilities(utilities),
    archive(archive),
    settings(settings),
    unreadChatCount(0),
    unreadUnmutedChatCount(0),
    unreadMessageCount(0),
    unreadUnmutedMessageCount(0)
{

    if (!doNotConnectChatListSignals) {
        if (!archive) {
            connect(tdLibWrapper, &TDLibWrapper::chatAddedToMainList, this, &ChatListModel::handleChatAddedToList);
            connect(tdLibWrapper, &TDLibWrapper::chatRemovedFromMainList, this, &ChatListModel::handleChatRemovedFromList);
            connect(tdLibWrapper, &TDLibWrapper::mainChatListChatPositionUpdated, this, &ChatListModel::handleChatPositionUpdated);
            connect(tdLibWrapper, &TDLibWrapper::mainChatListUnreadChatCountUpdated, this, &ChatListModel::handleUnreadChatCountUpdated);
            connect(tdLibWrapper, &TDLibWrapper::mainChatListUnreadMessageCountUpdated, this, &ChatListModel::handleUnreadMessageCountUpdated);
        } else {
            connect(tdLibWrapper, &TDLibWrapper::chatAddedToArchiveList, this, &ChatListModel::handleChatAddedToList);
            connect(tdLibWrapper, &TDLibWrapper::chatRemovedFromArchiveList, this, &ChatListModel::handleChatRemovedFromList);
            connect(tdLibWrapper, &TDLibWrapper::archiveChatListChatPositionUpdated, this, &ChatListModel::handleChatPositionUpdated);
            connect(tdLibWrapper, &TDLibWrapper::archiveChatListUnreadChatCountUpdated, this, &ChatListModel::handleUnreadChatCountUpdated);
            connect(tdLibWrapper, &TDLibWrapper::archiveChatListUnreadMessageCountUpdated, this, &ChatListModel::handleUnreadMessageCountUpdated);
        }
    }

    connect(tdLibWrapper, &TDLibWrapper::chatRolesUpdated, this, &ChatListModel::handleChatRolesChanged);
    //connect(tdLibWrapper, &TDLibWrapper::chatPinnedMessageUpdated, this, &ChatListModel::handleChatPinnedMessageUpdated); // also disabled for now
    //connect(tdLibWrapper, &TDLibWrapper::messageSendSucceeded, this, &ChatListModel::handleMessageSendSucceeded); // disabled for now, let's see if it will fix (or break) anything

    connect(tdLibWrapper, &TDLibWrapper::chatListsCalculateUnreadState, this, &ChatListModel::calculateUnreadState);

    connect(settings, &Settings::unreadCountIncludeMutedChanged, this, &ChatListModel::unreadChatCountChanged);
    connect(settings, &Settings::unreadCountIncludeMutedChanged, this, &ChatListModel::unreadMessageCountChanged);

    // Don't start the timer until we have at least one chat
    relativeTimeRefreshTimer = new QTimer(this);
    relativeTimeRefreshTimer->setSingleShot(false);
    relativeTimeRefreshTimer->setInterval(30000);
    connect(relativeTimeRefreshTimer, &QTimer::timeout, this, &ChatListModel::handleRelativeTimeRefreshTimer);
    connect(this, &ChatListModel::rowsInserted, this, &ChatListModel::countChanged);
    connect(this, &ChatListModel::rowsRemoved, this, &ChatListModel::countChanged);
    connect(this, &ChatListModel::modelReset, this, &ChatListModel::countChanged);
}

ChatListModel::~ChatListModel()
{
    LOG("Destroying myself...");
    qDeleteAll(chatList);
}

void ChatListModel::reset() {
    if (!chatList.isEmpty()) {
        beginResetModel();
        qDeleteAll(chatList);
        chatList.clear();
        endResetModel();
    }
}

QHash<int,QByteArray> ChatListModel::roleNames() const {
    return {
        {ChatData::RoleDisplay, "display"},
        {ChatData::RoleChatId, "chat_id"},
        {ChatData::RoleChatType, "chat_type"},
        {ChatData::RoleGroupId, "group_id"},
        {ChatData::RoleTitle, "title"},
        {ChatData::RolePhoto, "photo_data"},
        {ChatData::RoleUnreadCount, "unread_count"},
        {ChatData::RoleUnreadMentionCount, "unread_mention_count"},
        {ChatData::RoleUnreadReactionCount, "unread_reaction_count"},
        {ChatData::RoleAvailableReactions, "available_reactions"},
        {ChatData::RoleLastReadInboxMessageId, "last_read_inbox_message_id"},
        {ChatData::RoleLastMessageSenderId, "last_message_sender_id"},
        {ChatData::RoleLastMessageDate, "last_message_date"},
        {ChatData::RoleLastMessageText, "last_message_text"},
        {ChatData::RoleLastMessageMinithumbnail, "last_message_minithumbnail"},
        {ChatData::RoleLastMessageIsService, "last_message_is_service"},
        {ChatData::RoleLastMessageStatus, "last_message_status"},
        {ChatData::RoleChatMemberStatus, "chat_member_status"},
        {ChatData::RoleSecretChatState, "secret_chat_state"},
        {ChatData::RoleVerificationStatus, "verification_status"},
        {ChatData::RoleIsChannel, "is_channel"},
        {ChatData::RoleIsMarkedAsUnread, "is_marked_as_unread"},
        {ChatData::RoleIsPinned, "is_pinned"},
        {ChatData::RoleDraftMessageDate, "draft_message_date"},
        {ChatData::RoleDraftMessageText, "draft_message_text"},
        {ChatData::RoleNotificationSettings, "notification_settings"},
        {ChatData::RolePermissions, "permissions"}
    };
}

int ChatListModel::rowCount(const QModelIndex &) const {
    return chatList.size();
}

QVariant ChatListModel::data(const QModelIndex &index, int role) const {
    const int row = index.row();
    if (row >= 0 && row < chatList.size()) {
        const ListChatData *data = chatList.at(row);
        switch ((ChatData::Role)role) {
        case ChatData::RoleDisplay: return data->data->chatData;
        case ChatData::RoleChatId: return data->data->chatId;
        case ChatData::RoleChatType: return data->data->chatType;
        case ChatData::RoleGroupId: return data->data->groupId;
        case ChatData::RoleTitle: return data->data->title();
        case ChatData::RolePhoto: return data->data->photo();
        case ChatData::RoleUnreadCount: return data->data->unreadCount();
        case ChatData::RoleUnreadMentionCount: return data->data->unreadMentionCount();
        case ChatData::RoleAvailableReactions: return data->data->availableReactions();
        case ChatData::RoleUnreadReactionCount: return data->data->unreadReactionCount();
        case ChatData::RoleLastReadInboxMessageId: return data->data->lastReadInboxMessageId();
        case ChatData::RoleLastMessageSenderId: return data->data->lastMessageSenderUserId();
        case ChatData::RoleLastMessageText: return data->data->lastMessageText();
        case ChatData::RoleLastMessageMinithumbnail: return data->data->lastMessageMinithumbnail();
        case ChatData::RoleLastMessageIsService: return data->data->lastMessageIsService();
        case ChatData::RoleLastMessageDate: return data->data->lastMessageDate();
        case ChatData::RoleLastMessageStatus: return data->data->lastMessageStatus();
        case ChatData::RoleChatMemberStatus: return data->data->memberStatus;
        case ChatData::RoleSecretChatState: return data->data->secretChatState;
        case ChatData::RoleVerificationStatus: return data->data->verificationStatus;
        case ChatData::RoleIsChannel: return data->data->isChannel();
        case ChatData::RoleIsMarkedAsUnread: return data->data->isMarkedAsUnread();
        case ChatData::RoleIsPinned: return data->isPinned;
        case ChatData::RoleDraftMessageText: return data->data->draftMessageText();
        case ChatData::RoleDraftMessageDate: return data->data->draftMessageDate();
        case ChatData::RoleNotificationSettings: return data->data->notificationSettings();
        case ChatData::RolePermissions: return data->data->permissions();
        }
    }
    return QVariant();
}

void ChatListModel::redrawModel() {
    LOG("Enforcing UI redraw...");
    layoutChanged();
}

QVariantMap ChatListModel::get(int row) {
    QHash<int,QByteArray> names = roleNames();
    QHashIterator<int, QByteArray> i(names);
    QVariantMap res;
    QModelIndex idx = index(row, 0);
    while (i.hasNext()) {
        i.next();
        QVariant data = idx.data(i.key());
        res[i.value()] = data;
    }
    return res;
}

int ChatListModel::updateChatOrder(const int chatIndex) {
    ListChatData *chat = chatList.at(chatIndex);

    const int n = chatList.size();
    int newIndex = chatIndex;
    while (newIndex > 0 && chat->compareTo(chatList.at(newIndex-1)) < 0) {
        newIndex--;
    }
    if (newIndex == chatIndex) {
        while (newIndex < n-1 && chat->compareTo(chatList.at(newIndex+1)) > 0) {
            newIndex++;
        }
    }
    if (newIndex != chatIndex) {
        LOG("Moving chat" << chat->data->chatId << "from position" << chatIndex << "to" << newIndex);
        beginMoveRows(QModelIndex(), chatIndex, chatIndex, QModelIndex(), (newIndex < chatIndex) ? newIndex : (newIndex+1));
        chatList.move(chatIndex, newIndex);
        chatIndexMap.insert(chat->data->chatId, newIndex);
        // Update damaged part of the map
        const int last = qMax(chatIndex, newIndex);
        if (newIndex < chatIndex) {
            // First index is already correct
            for (int i = newIndex + 1; i <= last; i++) {
                chatIndexMap.insert(chatList.at(i)->data->chatId, i);
            }
        } else {
            // Last index is already correct
            for (int i = chatIndex; i < last; i++) {
                chatIndexMap.insert(chatList.at(i)->data->chatId, i);
            }
        }
        endMoveRows();
    } else
        LOG("Chat" << chat->data->chatId << "stays at position" << chatIndex);

    return newIndex;
}

void ChatListModel::updateChatIsPinned(const int chatIndex, const bool isPinned) {
    LOG("Updating chat is pinned at" << chatIndex << isPinned);
    chatList.at(chatIndex)->isPinned = isPinned;

    const QVector<int> changedRoles{ChatData::RoleIsPinned};
    const QModelIndex modelIndex(index(chatIndex));
    emit dataChanged(modelIndex, modelIndex, changedRoles);
}

void ChatListModel::handleChatRolesChanged(qlonglong chatId, const QVector<int> changedRoles) {
    if (chatIndexMap.contains(chatId)) {
        LOG("Chat roles changed for" << chatId);
        const QModelIndex modelIndex = index(chatIndexMap.value(chatId));
        emit dataChanged(modelIndex, modelIndex, changedRoles);
    }
}

void ChatListModel::enableRefreshTimer() {
    // Start timestamp refresh timer if not yet active (usually when the first visible chat is discovered)
    if (!relativeTimeRefreshTimer->isActive()) {
        LOG("Enabling refresh timer");
        relativeTimeRefreshTimer->start();
    }
}

void ChatListModel::calculateUnreadState() {
    if (this->settings->onlineOnlyMode()) {
        LOG("Online-only mode: Calculating unread state on my own...");
        int unreadMessages = 0;
        int unreadChats = 0;
        QListIterator<ListChatData*> chatIterator(this->chatList);
        while (chatIterator.hasNext()) {
            ChatData *currentChat = chatIterator.next()->data;
            int unreadCount = currentChat->unreadCount();
            if (unreadCount > 0) {
                unreadChats++;
                unreadMessages += unreadCount;
            }
        }
        LOG("Online-only mode: New unread state:" << unreadMessages << unreadChats);
        emit unreadStateChanged(unreadMessages, unreadChats);
    }
}

void ChatListModel::handleChatAddedToList(ChatData *chatData, qlonglong order, bool isPinned) {
    LOG("Chat added to list");
    ListChatData* chat = new ListChatData(chatData, order, isPinned);

    // Actually add the chat to list
    const int n = chatList.size();
    int pos;
    for (pos = 0; pos < n && chat->compareTo(chatList.at(pos)) >= 0; pos++);
    LOG("Adding chat" << chat->data->chatId << "at" << pos);
    beginInsertRows(QModelIndex(), pos, pos);
    chatList.insert(pos, chat);
    chatIndexMap.insert(chat->data->chatId, pos);
    // Update damaged part of the map
    for (int i = pos + 1; i <= n; i++) {
        chatIndexMap.insert(chatList.at(i)->data->chatId, i);
    }
    endInsertRows();
    if (this->tdLibWrapper->getJoinChatRequested()) {
        this->tdLibWrapper->registerJoinChat();
        emit chatJoined(chat->data->chatId, chat->data->chatData.value("title").toString());
    }
    enableRefreshTimer();
}

void ChatListModel::handleChatRemovedFromList(qlonglong chatId) {
    LOG("Chat removed from list" << chatId);
    if (chatIndexMap.contains(chatId)) {
        const int i = chatIndexMap.value(chatId);
        LOG("Removing chat at" << i);

        beginRemoveRows(QModelIndex(), i, i);
        delete chatList.takeAt(i);
        chatIndexMap.remove(chatId);
        // Update damaged part of the map
        const int n = chatList.size();
        for (int pos = i; pos < n; pos++)
            chatIndexMap.insert(chatList.at(pos)->data->chatId, pos);
        endRemoveRows();
    }
}

void ChatListModel::handleChatPositionUpdated(qlonglong chatId, qlonglong order, bool isPinned) {
    if (chatIndexMap.contains(chatId)) {
        LOG("Updating chat order of" << chatId << "to" << order);
        int chatIndex = chatIndexMap.value(chatId);

        chatList.at(chatIndex)->order = order;
        chatIndex = updateChatOrder(chatIndex);
        updateChatIsPinned(chatIndex, isPinned);
    }
}

void ChatListModel::handleChatPinnedMessageUpdated(qlonglong chatId, qlonglong pinnedMessageId)
{
    if (chatIndexMap.contains(chatId)) {
        LOG("Updating pinned message for" << chatId);
        const int chatIndex = chatIndexMap.value(chatId);
        ChatData *chat = chatList.at(chatIndex)->data;
        chat->chatData.insert(PINNED_MESSAGE_ID, pinnedMessageId);
    }
}

void ChatListModel::handleMessageSendSucceeded(qlonglong chatId, qlonglong oldMessageId, qlonglong messageId, const QVariantMap &message) {
    // is this really needed? and doesn't it break some stuff
    if (chatIndexMap.contains(chatId)) {
        const int chatIndex = chatIndexMap.value(chatId);
        LOG("Updating last message for chat" << chatId << "at index" << chatIndex << ", as message was sent, old ID:" << oldMessageId << ", new ID:" << messageId);
        const QModelIndex modelIndex(index(chatIndex));
        emit dataChanged(modelIndex, modelIndex, chatList.at(chatIndex)->data->updateLastMessage(message));
    }
}

void ChatListModel::handleRelativeTimeRefreshTimer() {
    LOG("Refreshing timestamps");
    emit dataChanged(index(0), index(chatList.size() - 1), {
                         ChatData::RoleLastMessageDate,
                         ChatData::RoleLastMessageStatus // FIXME: why was this added here?
                     });
}


void ChatListModel::handleUnreadChatCountUpdated(const QVariantMap &chatCountInformation) {
    this->unreadChatCount = chatCountInformation.value(UNREAD_COUNT).toInt();
    this->unreadUnmutedChatCount = chatCountInformation.value(UNREAD_UNMUTED_COUNT).toInt();
    unreadChatCountChanged();
}

void ChatListModel::handleUnreadMessageCountUpdated(const QVariantMap &messageCountInformation) {
    this->unreadMessageCount = messageCountInformation.value(UNREAD_COUNT).toInt();
    this->unreadUnmutedMessageCount = messageCountInformation.value(UNREAD_UNMUTED_COUNT).toInt();
    unreadMessageCountChanged();
}

int ChatListModel::getUnreadChatCount(bool asFolder) const {
    return archive || (asFolder ? settings->foldersUnreadCountIncludeMuted() : settings->unreadCountIncludeMuted())
            ? unreadChatCount : unreadUnmutedChatCount;
}

int ChatListModel::getUnreadMessageCount(bool asFolder) const {
    return archive || (asFolder ? settings->foldersUnreadCountIncludeMuted() : settings->unreadCountIncludeMuted())
            ? unreadMessageCount : unreadUnmutedMessageCount;
}
