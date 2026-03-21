/*
    Copyright (C) 2020-22 Sebastian J. Wolf and other contributors

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

#include "tdlibwrapper.h"
#include "tdlibsecrets.h"
#include "utilities.h"
#include "chatdata.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QProcess>
#include <QSysInfo>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QDesktopServices>

#define DEBUG_MODULE TDLibWrapper
#include "debuglog.h"

#define VERSION_NUMBER(x,y,z) \
    ((((x) & 0x3ff) << 20) | (((y) & 0x3ff) << 10) | ((z) & 0x3ff))

namespace {
    const QString STATUS("status");
    const QString ID("id");
    const QString CHAT_ID("chat_id");
    const QString USER_ID("user_id");
    const QString MESSAGE_ID("message_id");
    const QString MESSAGE_IDS("message_ids");
    const QString TYPE("type");
    const QString CAPTION("caption");
    const QString LAST_NAME("last_name");
    const QString FIRST_NAME("first_name");
    const QString USERNAME("username");
    const QString USERNAMES("usernames");
    const QString EDITABLE_USERNAME("editable_username");
    const QString THREAD_ID("thread_id");
    const QString VALUE("value");
    const QString REPLY_TO_MESSAGE_ID("reply_to_message_id");
    const QString REPLY_TO("reply_to");
    const QString _TYPE("@type");
    const QString _EXTRA("@extra");
    const QString TYPE_CHAT_POSITION("chatPosition");
    const QString TYPE_CHAT_LIST_MAIN("chatListMain");
    const QString TYPE_CHAT_LIST_ARCHIVE("chatListArchive");
    const QString TYPE_CHAT_LIST_FOLDER("chatListFolder");
    const QString CHAT_FOLDER_ID("chat_folder_id");
    const QString CHAT_AVAILABLE_REACTIONS("available_reactions");
    const QString CHAT_AVAILABLE_REACTIONS_ALL("chatAvailableReactionsAll");
    const QString CHAT_AVAILABLE_REACTIONS_SOME("chatAvailableReactionsSome");
    const QString REACTIONS("reactions");
    const QString REACTION_TYPE("reaction_type");
    const QString REACTION_TYPE_EMOJI("reactionTypeEmoji");
    const QString EMOJI("emoji");
    const QString TYPE_MESSAGE_REPLY_TO_MESSAGE("messageReplyToMessage");
    const QString TYPE_INPUT_MESSAGE_REPLY_TO_MESSAGE("inputMessageReplyToMessage");
    const QString TEXT("text");
    const QString PHOTO("photo");
    const QString TYPE_INPUT_FILE_LOCAL("inputFileLocal");
    const QString TYPE_INPUT_FILE_ID("inputFileId");
    const QString PATH("path");
    const QString CONTACT("contact");
    const QString PHONE_NUMBER("phone_number");
    const QString REMOVE_CONTACTS("removeContacts");
    const QString INPUT_MESSAGE_CONTENT("input_message_content");
    const QString LOCATION("location");
    const QString LIMIT("limit");
    const QString OFFSET("offset");
    const QString QUERY("query");
    const QString FILTER("filter");
    const QString EXTRA_RECENTLY_FOUND("recentlyFound");
    const QString POSITIONS("positions");
    const QString CHAT_LISTS("chat_lists");
    const QString CHAT_LIST("chat_list");
    const QString LIST("list");
    const QString ORDER("order");
    const QString IS_PINNED("is_pinned");
    const QString LAST_MESSAGE("last_message");
    const QString DRAFT_MESSAGE("draft_message");
    const QString LAST_READ_INBOX_MESSAGE_ID("last_read_inbox_message_id");
    const QString LAST_READ_OUTBOX_MESSAGE_ID("last_read_outbox_message_id");
    const QString UNREAD_COUNT("unread_count");
    const QString TITLE("title");
    const QString NOTIFICATION_SETTINGS("notification_settings");
    const QString UNREAD_MENTION_COUNT("unread_mention_count");
    const QString UNREAD_REACTION_COUNT("unread_reaction_count");
    const QString AVAILABLE_REACTIONS("available_reactions");
    const QString IS_MARKED_AS_UNREAD("is_marked_as_unread");
    const QString SECRET_CHAT_ID("secret_chat_id");
    const QString TYPE_READ_CHAT_LIST("readChatList");
    const QString RETURN_LOCAL("return_local");
    const QString ACTION("action");
    const QString TYPE_SET_BIRTHDATE("setBirthdate");
    const QString BIRTHDATE("birthdate");
    const QString PENDING_JOIN_REQUESTS("pending_join_requests");
    const QString APPROVE("approve");
    const QString INVITE_LINK("invite_link");
    const QString LINK("link");
    const QString EXTRA_OPEN_DIRECTLY("openDirectly");
    const QString URL("url");
    const QString BASIC_GROUP_ID("basic_group_id");
    const QString SUPERGROUP_ID("supergroup_id");
    const QString ACTIVE_USERNAMES("active_usernames");
    const QString VIEW_AS_TOPICS("view_as_topics");
    const QString FROM_MESSAGE_ID("from_message_id");
    const QString MY_ID("my_id");
    const QString TOPIC_ID("topic_id");
    const QString SOURCE("source");
    const QString FORUM_TOPIC_ID("forum_topic_id");
    const QString TYPE_GET_FORUM_TOPIC("getForumTopic");
    const QString NAME("name");
    const QString TYPE_SET_OPTION("setOption");
    const QString STICKER_TYPE("sticker_type");
    const QString STICKER("sticker");
    const QString BOT_USER_ID("bot_user_id");
    const QString PROXY("proxy");
    const QString PROXY_ID("proxy_id");
    const QString ENABLE("enable");
    const QString TYPE_DISABLE_PROXY("disableProxy");
    const QString SERVER("server");
    const QString PORT("port");

    const QStringList ALL_FILE_TYPES(QStringList()
                                     << "fileTypeAnimation"
                                     << "fileTypeAudio"
                                     << "fileTypeDocument"
                                     << "fileTypeNone"
                                     << "fileTypeNotificationSound"
                                     << "fileTypePhoto"
                                     << "fileTypePhotoStory"
                                     << "fileTypeProfilePhoto"
                                     << "fileTypeSecret"
                                     << "fileTypeSecretThumbnail"
                                     << "fileTypeSecure"
                                     << "fileTypeSelfDestructingPhoto"
                                     << "fileTypeSelfDestructingVideo"
                                     << "fileTypeSelfDestructingVideoNote"
                                     << "fileTypeSelfDestructingVoiceNote"
                                     << "fileTypeSticker"
                                     << "fileTypeThumbnail"
                                     << "fileTypeUnknown"
                                     << "fileTypeVideo"
                                     << "fileTypeVideoNote"
                                     << "fileTypeVideoStory"
                                     << "fileTypeVoiceNote"
                                     << "fileTypeWallpaper"
    );

    const QRegularExpression RE_EXTRA_CHAT_MESSAGE_COUNT("^(searchMessagesFilter[a-zA-Z]+)(!?):(-?[0-9]*)$");
}

QVariantMap findChatPosition(const QVariantList &positions, bool archive = false) {
    for (const QVariant &positionVariant : positions) {
        const QVariantMap position = positionVariant.toMap();
        if (position.value(_TYPE).toString() == TYPE_CHAT_POSITION &&
                position.value(LIST).toMap().value(_TYPE).toString() == (archive ? TYPE_CHAT_LIST_ARCHIVE : TYPE_CHAT_LIST_MAIN))
            return position;
    }
    return QVariantMap();
}

QVariantMap findChatPositionForFolder(const QVariantList &positions, int folderId) {
    for (const QVariant &positionVariant : positions) {
        const QVariantMap position = positionVariant.toMap();
        if (position.value(_TYPE).toString() == TYPE_CHAT_POSITION) {
            const QVariantMap chatList = position.value(LIST).toMap();
            if (chatList.value(_TYPE).toString() == TYPE_CHAT_LIST_FOLDER && chatList.value(CHAT_FOLDER_ID).toInt() == folderId)
                return position;
        }
    }
    return QVariantMap();
}

TDLibWrapper::TDLibWrapper(AppSettings *settings, MceInterface *mce, QObject *parent)
    : QObject(parent)
    , tdLibClientId(td_create_client_id())
    , manager(new QNetworkAccessManager(this))
    , networkConfigurationManager(new QNetworkConfigurationManager(this))
    , appSettings(settings)
    , mceInterface(mce)
    , utilities(new Utilities(this))
    , authorizationState(AuthorizationState::Closed)
    , options(new QQmlPropertyMap(this))
    , diceEmojis()
    , versionNumber(0)
    , joinChatRequested(false)
    , isLoggingOut(false)
    , nextRequestId(1)
{
    LOG("Initializing TD Lib...");

    initializeTDLibReceiver();

    QString tdLibDatabaseDirectoryPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/tdlib";
    QDir tdLibDatabaseDirectory(tdLibDatabaseDirectoryPath);
    if (!tdLibDatabaseDirectory.exists()) {
        tdLibDatabaseDirectory.mkpath(tdLibDatabaseDirectoryPath);
    }

    this->dbusInterface = new DBusInterface(this);
    if (appSettings->useOpenWith()) {
        initializeOpenWith();
    } else {
        removeOpenWith();
    }

    connect(appSettings, &AppSettings::useOpenWithChanged, this, &TDLibWrapper::handleOpenWithChanged);
    connect(appSettings, &AppSettings::storageOptimizerChanged, this, &TDLibWrapper::handleStorageOptimizerChanged);
    connect(appSettings, &AppSettings::sendMarkdownChanged, this, &TDLibWrapper::handleSendMarkdownChanged);

    connect(networkConfigurationManager, &QNetworkConfigurationManager::configurationChanged, this, &TDLibWrapper::handleNetworkConfigurationChanged);

    connect(options, &QQmlPropertyMap::valueChanged, this, &TDLibWrapper::handleOptionsValueChanged);

    this->setLogVerbosityLevel();
    this->setOptionInteger("notification_group_count_max", 5);
    // set initial option states
    this->handleStorageOptimizerChanged();
    this->handleSendMarkdownChanged();
    this->setOptionBoolean("online", true);
}

TDLibWrapper::~TDLibWrapper() {
    LOG("Closing TDLib instance...");
    this->close();
    while (this->authorizationState != AuthorizationState::Closed) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
    }
    this->tdLibReceiver->setActive(false);
    while (this->tdLibReceiver->isRunning()) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
    }
    qDeleteAll(basicGroups.values());
    qDeleteAll(superGroups.values());
    qDeleteAll(chats.values());
}

void TDLibWrapper::initializeTDLibReceiver() {
    this->tdLibReceiver = new TDLibReceiver(this->tdLibClientId, this);
    connect(this->tdLibReceiver, &TDLibReceiver::authorizationStateChanged, this, &TDLibWrapper::handleAuthorizationStateChanged);
    connect(this->tdLibReceiver, &TDLibReceiver::optionUpdated, this, &TDLibWrapper::handleOptionUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::connectionStateChanged, this, &TDLibWrapper::handleConnectionStateChanged);
    connect(this->tdLibReceiver, &TDLibReceiver::userUpdated, this, &TDLibWrapper::handleUserUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::userStatusUpdated, this, &TDLibWrapper::handleUserStatusUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::fileUpdated, this, &TDLibWrapper::handleFileUpdated);

    connect(this->tdLibReceiver, &TDLibReceiver::newChatDiscovered, this, &TDLibWrapper::handleNewChatDiscovered);
    connect(this->tdLibReceiver, &TDLibReceiver::chatAddedToList, this, &TDLibWrapper::handleChatAddedToList);
    connect(this->tdLibReceiver, &TDLibReceiver::chatRemovedFromList, this, &TDLibWrapper::handleChatRemovedFromList);
    connect(this->tdLibReceiver, &TDLibReceiver::chatPositionUpdated, this, &TDLibWrapper::handleChatPositionUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::chatLastMessageUpdated, this, &TDLibWrapper::handleChatLastMessageUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::chatDraftMessageUpdated, this, &TDLibWrapper::handleChatDraftMessageUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::chatReadInboxUpdated, this, &TDLibWrapper::handleChatReadInboxUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::chatReadOutboxUpdated, this, &TDLibWrapper::handleChatReadOutboxUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::chatTitleUpdated, this, &TDLibWrapper::handleChatTitleUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::chatPhotoUpdated, this, &TDLibWrapper::handleChatPhotoUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::chatNotificationSettingsUpdated, this, &TDLibWrapper::handleChatNotificationSettingsUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::chatIsMarkedAsUnreadUpdated, this, &TDLibWrapper::handleChatIsMarkedAsUnreadUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::chatUnreadMentionCountUpdated, this, &TDLibWrapper::handleChatUnreadMentionCountUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::messageMentionRead, this, &TDLibWrapper::messageMentionRead);
    connect(this->tdLibReceiver, &TDLibReceiver::chatUnreadReactionCountUpdated, this, &TDLibWrapper::handleChatUnreadReactionCountUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::chatAvailableReactionsUpdated, this, &TDLibWrapper::handleChatAvailableReactionsUpdated);

    connect(this->tdLibReceiver, &TDLibReceiver::unreadMessageCountUpdated, this, &TDLibWrapper::handleUnreadMessageCountUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::unreadChatCountUpdated, this, &TDLibWrapper::handleUnreadChatCountUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::basicGroupUpdated, this, &TDLibWrapper::handleBasicGroupUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::supergroupUpdated, this, &TDLibWrapper::handleSupergroupUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::chatOnlineMemberCountUpdated, this, &TDLibWrapper::chatOnlineMemberCountUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::messagesReceived, this, &TDLibWrapper::messagesReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::foundChatMessagesReceived, this, &TDLibWrapper::handleFoundChatMessagesReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::sponsoredMessagesReceived, this, &TDLibWrapper::handleSponsoredMessagesReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::messageLinkInfoReceived, this, &TDLibWrapper::messageLinkInfoReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::newMessageReceived, this, &TDLibWrapper::newMessageReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::messageInformation, this, &TDLibWrapper::handleMessageInformation);
    connect(this->tdLibReceiver, &TDLibReceiver::messageSendSucceeded, this, &TDLibWrapper::messageSendSucceeded);
    connect(this->tdLibReceiver, &TDLibReceiver::activeNotificationsUpdated, this, &TDLibWrapper::activeNotificationsUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::notificationGroupUpdated, this, &TDLibWrapper::notificationGroupUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::notificationUpdated, this, &TDLibWrapper::notificationUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::messageContentUpdated, this, &TDLibWrapper::messageContentUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::messagesDeleted, this, &TDLibWrapper::messagesDeleted);
    connect(this->tdLibReceiver, &TDLibReceiver::chats, this, &TDLibWrapper::chatsReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::sponsoredChatsReceived, this, &TDLibWrapper::sponsoredChatsReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::chat, this, &TDLibWrapper::chatReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::secretChat, this, &TDLibWrapper::handleSecretChatReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::secretChatUpdated, this, &TDLibWrapper::handleSecretChatUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::recentStickersUpdated, this, &TDLibWrapper::recentStickersUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::favoriteStickersUpdated, this, &TDLibWrapper::favoriteStickersUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::stickers, this, &TDLibWrapper::handleStickersReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::installedStickerSetsUpdated, this, &TDLibWrapper::installedStickerSetsUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::stickerSets, this, &TDLibWrapper::handleStickerSets);
    connect(this->tdLibReceiver, &TDLibReceiver::stickerSet, this, &TDLibWrapper::stickerSetReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::chatMembers, this, &TDLibWrapper::chatMembersReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::userFullInfo, this, &TDLibWrapper::userFullInfoReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::userFullInfoUpdated, this, &TDLibWrapper::userFullInfoUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::basicGroupFullInfo, this, &TDLibWrapper::basicGroupFullInfoReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::basicGroupFullInfoUpdated, this, &TDLibWrapper::basicGroupFullInfoUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::supergroupFullInfo, this, &TDLibWrapper::supergroupFullInfoReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::supergroupFullInfoUpdated, this, &TDLibWrapper::supergroupFullInfoUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::chatPhotos, this, &TDLibWrapper::chatPhotosReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::chatPermissionsUpdated, this, &TDLibWrapper::chatPermissionsUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::messageIsPinnedUpdated, this, &TDLibWrapper::handleMessageIsPinnedUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::usersReceived, this, &TDLibWrapper::usersReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::messageSendersReceived, this, &TDLibWrapper::messageSendersReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::errorReceived, this, &TDLibWrapper::handleErrorReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::serviceNotificationReceived, this, &TDLibWrapper::serviceNotificationReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::contactsImported, this, &TDLibWrapper::contactsImported);
    connect(this->tdLibReceiver, &TDLibReceiver::messageEditedUpdated, this, &TDLibWrapper::messageEditedUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::inlineQueryResults, this, &TDLibWrapper::inlineQueryResults);
    connect(this->tdLibReceiver, &TDLibReceiver::callbackQueryAnswer, this, &TDLibWrapper::callbackQueryAnswer);
    connect(this->tdLibReceiver, &TDLibReceiver::userPrivacySettingRules, this, &TDLibWrapper::handleUserPrivacySettingRules);
    connect(this->tdLibReceiver, &TDLibReceiver::userPrivacySettingRulesUpdated, this, &TDLibWrapper::handleUpdatedUserPrivacySettingRules);
    connect(this->tdLibReceiver, &TDLibReceiver::messageInteractionInfoUpdated, this, &TDLibWrapper::messageInteractionInfoUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::okReceived, this, &TDLibWrapper::okReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::sessionsReceived, this, &TDLibWrapper::sessionsReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::availableReactionsReceived, this, &TDLibWrapper::availableReactionsReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::activeEmojiReactionsUpdated, this, &TDLibWrapper::handleActiveEmojiReactionsUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::messagePropertiesReceived, this, &TDLibWrapper::messagePropertiesReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::storageStatisticsFastReceived, this, &TDLibWrapper::storageStatisticsFastReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::storageStatisticsReceived, this, &TDLibWrapper::storageStatisticsReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::formattedTextReceived, this, &TDLibWrapper::formattedTextReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::chatActionUpdated, this, &TDLibWrapper::chatActionUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::emojiKeywordsReceived, this, &TDLibWrapper::emojiKeywordsReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::diceEmojisUpdated, this, &TDLibWrapper::handleDiceEmojisUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::suggestedActionsUpdated, this, &TDLibWrapper::suggestedActionsUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::countReceived, this, &TDLibWrapper::handleCountReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::chatListsReceived, this, &TDLibWrapper::chatListsReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::archiveChatListSettingsReceived, this, &TDLibWrapper::archiveChatListSettingsReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::chatFoldersUpdated, this, &TDLibWrapper::chatFoldersUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::responseForRequestIdReceived, this, &TDLibWrapper::responseForRequestIdReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::forumTopicsReceived, this, &TDLibWrapper::forumTopicsReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::chatPendingJoinRequestsUpdated, this, &TDLibWrapper::handleChatPendingJoinRequestsUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::chatJoinRequestsReceived, this, &TDLibWrapper::chatJoinRequestsReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::internalLinkTypeReceived, this, &TDLibWrapper::handleInternalLinkTypeReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::deepLinkInfoReceived, this, &TDLibWrapper::deepLinkInfoReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::userReceived, this, &TDLibWrapper::handleUserReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::chatInviteLinkInfoReceived, this, &TDLibWrapper::chatInviteLinkInfoReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::chatViewAsTopicsUpdated, this, &TDLibWrapper::handleChatViewAsTopicsUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::threadMessagesReceived, this, &TDLibWrapper::threadMessagesReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::forumTopicMessagesReceived, this, &TDLibWrapper::forumTopicMessagesReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::forumTopicUpdated, this, &TDLibWrapper::forumTopicUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::forumTopicInfoUpdated, this, &TDLibWrapper::forumTopicInfoUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::forumTopicReceived, this, &TDLibWrapper::forumTopicReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::messageSuggestedPostInfoUpdated, this, &TDLibWrapper::messageSuggestedPostInfoUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::messageContentOpened, this, &TDLibWrapper::messageContentOpened);
    connect(this->tdLibReceiver, &TDLibReceiver::messageFactCheckUpdated, this, &TDLibWrapper::messageFactCheckUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::stickerSetUpdated, this, &TDLibWrapper::stickerSetUpdated);
    connect(this->tdLibReceiver, &TDLibReceiver::pollVotersReceived, this, &TDLibWrapper::pollVotersReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::addedProxiesReceived, this, &TDLibWrapper::addedProxiesReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::addedProxyReceived, this, &TDLibWrapper::addedProxyReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::pingReceived, this, &TDLibWrapper::pingReceived);
    connect(this->tdLibReceiver, &TDLibReceiver::proxyPingReceived, this, &TDLibWrapper::proxyPingReceived);

    this->tdLibReceiver->start();
}

void TDLibWrapper::sendRequest(const QVariantMap &requestObject) {
    if (this->isLoggingOut) {
        LOG("Sending request to TD Lib skipped as logging out is in progress, object type name:" << requestObject.value(_TYPE).toString());
        return;
    }
    LOG("Sending request to TD Lib, object type name:" << requestObject.value(_TYPE).toString());
    QJsonDocument requestDocument = QJsonDocument::fromVariant(requestObject);
    VERBOSE(requestDocument.toJson().constData());
    td_send(this->tdLibClientId, requestDocument.toJson().constData());
}

TDLibResponse *TDLibWrapper::sendRequestWithId(const QVariantMap &requestObject) {
    LOG("Sending request to TDLib with request ID" << nextRequestId);
    QVariantMap newRequestObject = requestObject;
    newRequestObject.insert(_EXTRA, "R"+QString::number(nextRequestId));
    this->sendRequest(newRequestObject);
    nextRequestId++;
    return new TDLibResponse(nextRequestId - 1, this);
}

void TDLibWrapper::setAuthenticationPhoneNumber(const QString &phoneNumber) {
    LOG("Set authentication phone number " << phoneNumber);
    this->sendRequest(QVariantMap{
                          {_TYPE, "setAuthenticationPhoneNumber"},
                          {PHONE_NUMBER, phoneNumber},
                          {"settings", QVariantMap{
                               {"allow_flash_call", false},
                               {"is_current_phone_number", true}
                           }}
                      });
}

void TDLibWrapper::setAuthenticationCode(const QString &authenticationCode) {
    LOG("Set authentication code " << authenticationCode);
    this->sendRequest(QVariantMap{{_TYPE, "checkAuthenticationCode"}, {"code", authenticationCode}});
}

void TDLibWrapper::setAuthenticationPassword(const QString &authenticationPassword) {
    LOG("Set authentication password " << authenticationPassword);
    this->sendRequest(QVariantMap{{_TYPE, "checkAuthenticationPassword"}, {"password", authenticationPassword}});
}

void TDLibWrapper::registerUser(const QString &firstName, const QString &lastName) {
    LOG("Register User " << firstName << lastName);
    this->sendRequest(QVariantMap{
        {_TYPE, "registerUser"},
        {FIRST_NAME, firstName},
        {LAST_NAME, lastName}
    });
}

void TDLibWrapper::logout() {
    LOG("Logging out");
    this->sendRequest(QVariantMap{{_TYPE, "logOut"}});
    this->isLoggingOut = true;

}

void TDLibWrapper::loadChats(bool archive) {
    LOG("Loading chats archive:" << archive);
    this->sendRequest(QVariantMap{
                          {_TYPE, "loadChats"},
                          {LIMIT, 5},
                          {CHAT_LIST, QVariantMap{{_TYPE, (archive ? TYPE_CHAT_LIST_ARCHIVE : TYPE_CHAT_LIST_MAIN)}}}
                      });
}

void TDLibWrapper::loadChatsForFolder(int folderId) {
    LOG("Loading chats for folder" << folderId);
    this->sendRequest(QVariantMap{
                          {_TYPE, "loadChats"},
                          {LIMIT, 5},
                          {CHAT_LIST, QVariantMap{{_TYPE, TYPE_CHAT_LIST_FOLDER}, {CHAT_FOLDER_ID, folderId}}}
                      });
}

void TDLibWrapper::downloadFile(int fileId) {
    LOG("Downloading file " << fileId);
    this->sendRequest(QVariantMap{
        {_TYPE, "downloadFile"},
        {"file_id", fileId},
        {"synchronous", false},
        {OFFSET, 0},
        {LIMIT, 0},
        {"priority", 1}
    });
}

void TDLibWrapper::openChat(qlonglong chatId) {
    LOG("Opening chat " << chatId);
    this->sendRequest(QVariantMap{{_TYPE, "openChat"}, {CHAT_ID, chatId}});
}

void TDLibWrapper::closeChat(qlonglong chatId) {
    LOG("Closing chat " << chatId);
    this->sendRequest(QVariantMap{{_TYPE, "closeChat"}, {CHAT_ID, chatId}});
}

void TDLibWrapper::joinChat(const QString &chatId) {
    LOG("Joining chat " << chatId);
    this->joinChatRequested = true;
    this->sendRequest(QVariantMap{{_TYPE, "joinChat"}, {CHAT_ID, chatId}});
}

void TDLibWrapper::leaveChat(const QString &chatId) {
    LOG("Leaving chat " << chatId);
    this->sendRequest(QVariantMap{{_TYPE, "leaveChat"}, {CHAT_ID, chatId}});
}

void TDLibWrapper::deleteChat(qlonglong chatId) {
    LOG("Deleting chat" << chatId);
    this->sendRequest(QVariantMap{{_TYPE, "deleteChat"}, {CHAT_ID, chatId}});
}

void TDLibWrapper::getChatHistory(qlonglong chatId, int extra, qlonglong fromMessageId, int offset, int limit, bool onlyLocal) {
    LOG("Retrieving chat history" << chatId << extra << fromMessageId << offset << limit << onlyLocal);
    this->sendRequest(QVariantMap{
        {_TYPE, "getChatHistory"},
        {CHAT_ID, chatId},
        {FROM_MESSAGE_ID, fromMessageId},
        {OFFSET, offset},
        {LIMIT, limit},
        {"only_local", onlyLocal},
        {_EXTRA, QString::number(chatId)+":"+QString::number(extra)}
    });
}

void TDLibWrapper::viewMessage(qlonglong chatId, qlonglong messageId, bool force, MessageSource source) {
    QVariantMap request{
        {_TYPE, "viewMessages"},
        {CHAT_ID, chatId},
        {"force_read", force},
        {MESSAGE_IDS, QVariantList{messageId}}
    };

    if (source != MessageSourceAuto)
        request.insert(SOURCE, QVariantMap{{_TYPE, getMessageSourceType(source)}});

    this->sendRequest(request);
}

void TDLibWrapper::pinMessage(const QString &chatId, const QString &messageId, bool disableNotification) {
    LOG("Pin message to chat" << chatId << messageId << disableNotification);
    this->sendRequest(QVariantMap{
        {_TYPE, "pinChatMessage"},
        {CHAT_ID, chatId},
        {MESSAGE_ID, messageId},
        {"disable_notification", disableNotification}
    });
}

void TDLibWrapper::unpinMessage(const QString &chatId, const QString &messageId) {
    LOG("Unpin message from chat" << chatId);
    this->sendRequest(QVariantMap{
        {_TYPE, "unpinChatMessage"},
        {CHAT_ID, chatId},
        {MESSAGE_ID, messageId},
        {_EXTRA, "unpinChatMessage:" + chatId}
    });
}

void TDLibWrapper::sendMessage(qlonglong chatId, qlonglong replyToMessageId, const QVariantMap &topicId, const QVariantMap &content) {
    QVariantMap request{{_TYPE, "sendMessage"}, {CHAT_ID, chatId}, {INPUT_MESSAGE_CONTENT, content}};
    if (replyToMessageId != 0)
        request.insert(REPLY_TO, QVariantMap{
            {_TYPE, TYPE_INPUT_MESSAGE_REPLY_TO_MESSAGE},
            {MESSAGE_ID, replyToMessageId}
        });
    if (!topicId.isEmpty())
        request.insert(TOPIC_ID, topicId);

    this->sendRequest(request);
}

void TDLibWrapper::sendTextMessage(qlonglong chatId, const QString &message, qlonglong replyToMessageId, const QVariantMap &topicId) {
    LOG("Sending text message" << chatId << message << replyToMessageId);
    sendMessage(chatId, replyToMessageId, topicId, {{_TYPE, "inputMessageText"}, {TEXT, Utilities::enhanceInputText(message)}});
}

void TDLibWrapper::sendFileMessage(qlonglong chatId, const QString &messageType, const QString &fileType, const QString &filePath, const QString &caption, qlonglong replyToMessageId, const QVariantMap &topicId, const QVariantMap &additionalOptions) {
    QVariantMap content = additionalOptions;
    content.insert(_TYPE, messageType);
    content.insert(CAPTION, Utilities::enhanceInputText(caption));
    content.insert(fileType, QVariantMap{{_TYPE, TYPE_INPUT_FILE_LOCAL}, {PATH, filePath}});

    sendMessage(chatId, replyToMessageId, topicId, content);
}

void TDLibWrapper::sendLocationMessage(qlonglong chatId, double latitude, double longitude, double horizontalAccuracy, qlonglong replyToMessageId, const QVariantMap &topicId) {
    LOG("Sending location message" << chatId << latitude << longitude << horizontalAccuracy << replyToMessageId);

    sendMessage(chatId, replyToMessageId, topicId, {
                             {_TYPE, "inputMessageLocation"},
                             {LOCATION, QVariantMap{
                                     {_TYPE, LOCATION},
                                     {"latitude", latitude},
                                     {"longitude", longitude},
                                     {"horizontal_accuracy", horizontalAccuracy}
                                 }},
                             {"live_period", 0},
                             {"heading", 0},
                             {"proximity_alert_radius", 0}
                         });
}

void TDLibWrapper::sendStickerMessage(qlonglong chatId, const QString &fileId, qlonglong replyToMessageId, const QVariantMap &topicId) {
    LOG("Sending sticker message" << chatId << fileId << replyToMessageId);
    sendMessage(chatId, replyToMessageId, topicId, {
                    {_TYPE, "inputMessageSticker"},
                    {STICKER, QVariantMap{{_TYPE, "inputFileRemote"}, {ID, fileId}}}
                });
}

void TDLibWrapper::sendPollMessage(qlonglong chatId, const QString &question, const QStringList &options, bool anonymous, int correctOption, bool multiple, const QString &explanation, qlonglong replyToMessageId, const QVariantMap &topicId) {
    LOG("Sending poll message" << chatId << question << replyToMessageId);
    QVariantMap inputMessageContent{
        {_TYPE, "inputMessagePoll"},
        {"question", Utilities::newFormattedText(question)},
        {"is_anonymous", anonymous}
    };

    QVariantList formattedOptions;
    for (QString option : options)
        formattedOptions.append(Utilities::newFormattedText(option));
    inputMessageContent.insert("options", formattedOptions);

    QVariantMap pollType;
    if(correctOption > -1) {
        pollType.insert(_TYPE, "pollTypeQuiz");
        pollType.insert("correct_option_id", correctOption);
        if(!explanation.isEmpty())
            pollType.insert("explanation", Utilities::newFormattedText(explanation));
    } else {
        pollType.insert(_TYPE, "pollTypeRegular");
        pollType.insert("allow_multiple_answers", multiple);
    }
    inputMessageContent.insert(TYPE, pollType);

    sendMessage(chatId, replyToMessageId, topicId, inputMessageContent);
}

void TDLibWrapper::sendDiceMessage(qlonglong chatId, const QString &emoji, qlonglong replyToMessageId, const QVariantMap &topicId) {
    LOG("Sending dice message" << chatId << emoji << replyToMessageId);
    sendMessage(chatId, replyToMessageId, topicId, QVariantMap{{_TYPE, "inputMessageDice"}, {EMOJI, emoji}});
}

void TDLibWrapper::forwardMessages(const QString &chatId, const QString &fromChatId, const QVariantList &messageIds, bool sendCopy, bool removeCaption) {
    LOG("Forwarding messages" << chatId << fromChatId << messageIds);
    this->sendRequest(QVariantMap{
        {_TYPE, "forwardMessages"},
        {CHAT_ID, chatId},
        {"from_chat_id", fromChatId},
        {"message_ids", messageIds},
        {"send_copy", sendCopy},
        {"remove_caption", removeCaption}
    });
}

void TDLibWrapper::getMessage(qlonglong chatId, qlonglong messageId) {
    LOG("Retrieving message" << chatId << messageId);
    this->sendRequest(QVariantMap{
        {_TYPE, "getMessage"},
        {CHAT_ID, chatId},
        {MESSAGE_ID, messageId},
        {_EXTRA, QString("getMessage:%1:%2").arg(chatId).arg(messageId)}
    });
}

void TDLibWrapper::getMessageLinkInfo(const QString &url) {
    LOG("Retrieving message link info" << url);
    this->sendRequest(QVariantMap{
        {_TYPE, "getMessageLinkInfo"},
        {URL, url}
    });
}

void TDLibWrapper::getExternalLinkInfo(const QString &url, const QString &extra) {
    LOG("Retrieving external link info" << url << extra);
    this->sendRequest(QVariantMap{
        {_TYPE, "getExternalLinkInfo"},
        {URL, url},
        {_EXTRA, extra == "" ? url : (url + "|" + extra)}
    });
}

void TDLibWrapper::getCallbackQueryAnswer(const QString &chatId, const QString &messageId, const QVariantMap &payload) {
    LOG("Getting Callback Query Answer" << chatId << messageId);
    this->sendRequest(QVariantMap{
        {_TYPE, "getCallbackQueryAnswer"},
        {CHAT_ID, chatId},
        {MESSAGE_ID, messageId},
        {"payload", payload}
    });
}

void TDLibWrapper::getChatPinnedMessage(qlonglong chatId) {
    LOG("Retrieving pinned message" << chatId);
    this->sendRequest(QVariantMap{
        {_TYPE, "getChatPinnedMessage"},
        {CHAT_ID, chatId},
        {_EXTRA, "getChatPinnedMessage:" + QString::number(chatId)}
    });
}

void TDLibWrapper::getChatSponsoredMessages(qlonglong chatId) {
    LOG("Retrieving sponsored message" << chatId);
    this->sendRequest(QVariantMap{
        {_TYPE, "getChatSponsoredMessages"},
        {CHAT_ID, chatId},
        {_EXTRA, chatId} // see TDLibReceiver::processSponsoredMessage
    });
}

void TDLibWrapper::setOption(const QString &name, const QString &type, const QVariant &value) {
    sendRequest({
        {_TYPE, TYPE_SET_OPTION},
        {NAME, name},
        {VALUE, QVariantMap{{_TYPE, type}, {VALUE, value}}}
    });
}

void TDLibWrapper::setOptionInteger(const QString &optionName, qlonglong optionValue) {
    LOG("Setting integer option" << optionName << optionValue);
    setOption(optionName, "optionValueInteger", optionValue);
}

void TDLibWrapper::setOptionBoolean(const QString &optionName, bool optionValue) {
    LOG("Setting boolean option" << optionName << optionValue);
    setOption(optionName, "optionValueBoolean", optionValue);
}

void TDLibWrapper::setOptionString(const QString &optionName, const QString &optionValue) {
    LOG("Setting string option" << optionName << optionValue);
    setOption(optionName, "optionValueString", optionValue);
}

void TDLibWrapper::resetOption(const QString &optionName) {
    LOG("Resetting option" << optionName);
    sendRequest({
        {_TYPE, TYPE_SET_OPTION},
        {NAME, optionName},
        {VALUE, QVariantMap{{_TYPE, "optionValueEmpty"}}}
    });
}

void TDLibWrapper::handleOptionsValueChanged(const QString &name, const QVariant &value) {
    switch (value.userType()) {
    case QMetaType::Bool:
        setOptionBoolean(name, value.toBool());
        break;
    case QMetaType::Int:
    case QMetaType::Long:
    case QMetaType::LongLong:
        setOptionInteger(name, value.toLongLong());
        break;
    case QMetaType::UnknownType:
        resetOption(name);
        break;
    default:
        setOptionString(name, value.toString());
        break;
    }
}

void TDLibWrapper::setChatNotificationSettings(const QString &chatId, const QVariantMap &notificationSettings) {
    LOG("Notification settings for chat " << chatId << notificationSettings);
    this->sendRequest(QVariantMap{
        {_TYPE, "setChatNotificationSettings"},
        {CHAT_ID, chatId},
        {"notification_settings", notificationSettings}
    });
}

void TDLibWrapper::editMessageText(const QString &chatId, const QString &messageId, const QString &message) {
    LOG("Editing message text" << chatId << messageId);
    this->sendRequest(QVariantMap{
        {_TYPE, "editMessageText"},
        {CHAT_ID, chatId},
        {MESSAGE_ID, messageId},
        {INPUT_MESSAGE_CONTENT, QVariantMap{
            {_TYPE, "inputMessageText"},
            {TEXT, Utilities::enhanceInputText(message)}
        }}
    });
}

void TDLibWrapper::editMessageCaption(const QString &chatId, const QString &messageId, const QString &caption) {
    LOG("Editing message caption" << chatId << messageId);
    this->sendRequest(QVariantMap{
        {_TYPE, "editMessageCaption"},
        {CHAT_ID, chatId},
        {MESSAGE_ID, messageId},
        {CAPTION, Utilities::enhanceInputText(caption)}
    });
}

void TDLibWrapper::deleteMessages(const QString &chatId, const QVariantList messageIds, bool revoke) {
    LOG("Deleting some messages" << chatId << messageIds);
    this->sendRequest(QVariantMap{
        {_TYPE, "deleteMessages"},
        {CHAT_ID, chatId},
        {"message_ids", messageIds},
        {"revoke", revoke}
    });
}

void TDLibWrapper::getMapThumbnailFile(const QString &chatId, double latitude, double longitude, int width, int height, const QString &extra) {
    LOG("Getting Map Thumbnail File" << chatId);
    this->sendRequest(QVariantMap{
        {_TYPE, "getMapThumbnailFile"},
        {"location", QVariantMap{
            {"latitude", latitude},
            {"longitude", longitude}
        }},
        {"zoom", 17}, //13-20

        // ensure dimensions are in bounds (16 - 1024)
        {"width", std::min(std::max(width, 16), 1024)},
        {"height", std::min(std::max(height, 16), 1024)},

        {"scale", 1}, // 1-3
        {CHAT_ID, chatId},
        {_EXTRA, extra},
    });
}

void TDLibWrapper::getRecentStickers() {
    LOG("Retrieving recent stickers");
    this->sendRequest({{_TYPE, "getRecentStickers"}, {_EXTRA, "recent"}});
}

void TDLibWrapper::getFavoriteStickers() {
    LOG("Retrieving favorite stickers");
    this->sendRequest({{_TYPE, "getFavoriteStickers"}, {_EXTRA, "favorite"}});
}

TDLibWrapper::StickerType TDLibWrapper::getStickerTypeForType(const QString &type) {
    if (type == "stickerTypeRegular")
        return StickerTypeRegular;
    if (type == "stickerTypeMask")
        return StickerTypeMask;
    if (type == "stickerTypeCustomEmoji")
        return StickerTypeCustomEmoji;

    return StickerTypeRegular;
}

QString TDLibWrapper::getStickerTypeType(StickerType stickerType) {
    switch (stickerType) {
    case StickerTypeRegular:
        return "stickerTypeRegular";
    case StickerTypeMask:
        return "stickerTypeMask";
    case StickerTypeCustomEmoji:
        return "stickerTypeCustomEmoji";
    }

    return QString();
}

void TDLibWrapper::getInstalledStickerSets(StickerType stickerType) {
    LOG("Retrieving installed sticker sets");
    this->sendRequest({
        {_TYPE, "getInstalledStickerSets"},
        {STICKER_TYPE, QVariantMap{{_TYPE, getStickerTypeType(stickerType)}}},
        {_EXTRA, "installed"+QString::number(stickerType)}
    });
}

void TDLibWrapper::getStickerSet(const QString &setId) {
    LOG("Retrieving sticker set" << setId);
    this->sendRequest({{_TYPE, "getStickerSet"}, {"set_id", setId}});
}

void TDLibWrapper::getSupergroupMembers(qlonglong groupId, int limit, int offset) {
    LOG("Retrieving supergroup members");
    this->sendRequest({
        {_TYPE, "getSupergroupMembers"},
        {_EXTRA, groupId},
        {SUPERGROUP_ID, groupId},
        {OFFSET, offset},
        {LIMIT, limit}
    });
}

void TDLibWrapper::getGroupFullInfo(const QString &groupId, bool isSupergroup) {
    QVariantMap requestObject{{_EXTRA, groupId}};
    if (isSupergroup) {
        LOG("Retrieving supergroup full info" << groupId);
        requestObject.insert(_TYPE, "getSupergroupFullInfo");
        requestObject.insert(SUPERGROUP_ID, groupId);
    } else {
        LOG("Retrieving basic group full info" << groupId);
        requestObject.insert(_TYPE, "getBasicGroupFullInfo");
        requestObject.insert(BASIC_GROUP_ID, groupId);
    }
    this->sendRequest(requestObject);
}

void TDLibWrapper::getUserFullInfo(qlonglong userId) {
    LOG("Retrieving UserFullInfo" << userId);
    this->sendRequest(QVariantMap{
        {_TYPE, "getUserFullInfo"},
        {_EXTRA, userId},
        {USER_ID, userId}
    });
}

void TDLibWrapper::createPrivateChat(const QString &userId, const QString &extra) {
    LOG("Creating Private Chat");
    this->sendRequest(QVariantMap{
        {_TYPE, "createPrivateChat"},
        {USER_ID, userId},
        {_EXTRA, extra} //"openDirectly"/"openAndSendStartToBot:[optional parameter]" gets matched in qml
    });
}

void TDLibWrapper::createNewSecretChat(const QString &userId, const QString &extra) {
    LOG("Creating new secret chat");
    this->sendRequest(QVariantMap{
        {_TYPE, "createNewSecretChat"},
        {USER_ID, userId},
        {_EXTRA, extra} //"openDirectly" gets matched in qml
    });
}

void TDLibWrapper::createSupergroupChat(const QString &supergroupId, const QString &extra) {
    LOG("Creating Supergroup Chat");
    this->sendRequest(QVariantMap{
        {_TYPE, "createSupergroupChat"},
        {SUPERGROUP_ID, supergroupId},
        {_EXTRA, extra} //"openDirectly" gets matched in qml
    });
}

void TDLibWrapper::createBasicGroupChat(const QString &basicGroupId, const QString &extra) {
    LOG("Creating Basic Group Chat");
    this->sendRequest(QVariantMap{
        {_TYPE, "createBasicGroupChat"},
        {BASIC_GROUP_ID, basicGroupId},
        {_EXTRA, extra} //"openDirectly"/"openAndSend:*" gets matched in qml
    });
}

void TDLibWrapper::getGroupsInCommon(qlonglong userId, int limit, int offset) {
    LOG("Retrieving Groups in Common");
    this->sendRequest(QVariantMap{
        {_TYPE, "getGroupsInCommon"},
        {_EXTRA, "getGroupsInCommon:" + QString::number(userId)},
        {USER_ID, userId},
        {OFFSET, offset},
        {LIMIT, limit}
    });
}

void TDLibWrapper::getUserProfilePhotos(qlonglong userId, int limit, int offset) {
    LOG("Retrieving User Profile Photos");
    this->sendRequest(QVariantMap{
        {_TYPE, "getUserProfilePhotos"},
        {_EXTRA, userId},
        {USER_ID, userId},
        {OFFSET, offset},
        {LIMIT, limit}
    });
}

void TDLibWrapper::setChatPermissions(const QString &chatId, const QVariantMap &chatPermissions) {
    LOG("Setting Chat Permissions");
    this->sendRequest(QVariantMap{
        {_TYPE, "setChatPermissions"},
    {_EXTRA, chatId},
    {CHAT_ID, chatId},
    {"permissions", chatPermissions}
    });
}

void TDLibWrapper::setChatSlowModeDelay(const QString &chatId, int delay) {
    LOG("Setting Chat Slow Mode Delay");
    this->sendRequest(QVariantMap{
        {_TYPE, "setChatSlowModeDelay"},
        {CHAT_ID, chatId},
        {"slow_mode_delay", delay}
    });
}

void TDLibWrapper::setChatDescription(const QString &chatId, const QString &description) {
    LOG("Setting Chat Description");
    this->sendRequest(QVariantMap{
        {_TYPE, "setChatDescription"},
        {CHAT_ID, chatId},
        {"description", description}
    });
}

void TDLibWrapper::setChatTitle(const QString &chatId, const QString &title) {
    LOG("Setting Chat Title");
    this->sendRequest(QVariantMap{
        {_TYPE, "setChatTitle"},
        {CHAT_ID, chatId},
        {"title", title}
    });
}

void TDLibWrapper::setBio(const QString &bio) {
    LOG("Setting Bio");
    this->sendRequest(QVariantMap{
        {_TYPE, "setBio"},
        {"bio", bio}
    });
}

void TDLibWrapper::toggleSupergroupIsAllHistoryAvailable(const QString &groupId, bool isAllHistoryAvailable) {
    LOG("Toggling SupergroupIsAllHistoryAvailable");
    this->sendRequest(QVariantMap{
        {_TYPE, "toggleSupergroupIsAllHistoryAvailable"},
        {SUPERGROUP_ID, groupId},
        {"is_all_history_available", isAllHistoryAvailable}
    });
}

void TDLibWrapper::setPollAnswer(const QString &chatId, qlonglong messageId, QVariantList optionIds) {
    LOG("Setting Poll Answer");
    this->sendRequest(QVariantMap{
        {_TYPE, "setPollAnswer"},
        {CHAT_ID, chatId},
        {MESSAGE_ID, messageId},
        {"option_ids", optionIds}
    });
}

void TDLibWrapper::stopPoll(const QString &chatId, qlonglong messageId) {
    LOG("Stopping poll");
    this->sendRequest(QVariantMap{
        {_TYPE, "stopPoll"},
        {CHAT_ID, chatId},
        {MESSAGE_ID, messageId}
    });
}

void TDLibWrapper::getPollVoters(qlonglong chatId, qlonglong messageId, int optionId, const QString &extra, int offset, int limit) {
    LOG("Getting poll voters");
    this->sendRequest(QVariantMap{
        {_TYPE, "getPollVoters"},
        {_EXTRA, extra},
        {CHAT_ID, chatId},
        {MESSAGE_ID, messageId},
        {"option_id", optionId},
        {OFFSET, offset},
        {LIMIT, limit} //max 50
    });
}

void TDLibWrapper::searchPublicChat(const QString &userName, bool doOpenOnFound) {
    LOG("Search public chat" << userName);

    this->sendRequest(QVariantMap{
        {_TYPE, "searchPublicChat"},
        {USERNAME, userName},
        {_EXTRA, QVariantMap{
            {TYPE, "searchPublicChat:"+userName},
            {EXTRA_OPEN_DIRECTLY, doOpenOnFound},
        }}
    });
}

void TDLibWrapper::searchUserByPhoneNumber(const QString &phoneNumber, bool doOpenOnFound) {
    LOG("Search user by phone number" << phoneNumber);

    this->sendRequest({
                          {_TYPE, "searchUserByPhoneNumber"},
                          {PHONE_NUMBER, phoneNumber},
                          {_EXTRA, doOpenOnFound}
                      });
}

void TDLibWrapper::joinChatByInviteLink(const QString &inviteLink, bool isChannel) {
    LOG("Join chat by invite link" << inviteLink);
    this->joinChatRequested = true;
    this->sendRequest({
                          {_TYPE, "joinChatByInviteLink"},
                          {INVITE_LINK, inviteLink},
                          {_EXTRA, QVariantMap{
                               {"isChannel", isChannel},
                               {EXTRA_OPEN_DIRECTLY, true}
                           }}
                      });
}

void TDLibWrapper::getDeepLinkInfo(const QString &link) {
    LOG("Resolving unknown deep link info" << link);
    this->sendRequest(QVariantMap{{_TYPE, "getDeepLinkInfo"}, {LINK, link}});
}

void TDLibWrapper::getContacts() {
    LOG("Retrieving contacts");
    this->sendRequest(QVariantMap{{_TYPE, "getContacts"}, {_EXTRA, "contactsRequested"}});
}

void TDLibWrapper::getSecretChat(qlonglong secretChatId) {
    LOG("Getting detailed information about secret chat" << secretChatId);
    this->sendRequest(QVariantMap{{_TYPE, "getSecretChat"}, {SECRET_CHAT_ID, secretChatId}});
}

void TDLibWrapper::closeSecretChat(qlonglong secretChatId) {
    LOG("Closing secret chat" << secretChatId);
    this->sendRequest(QVariantMap{{_TYPE, "closeSecretChat"}, {SECRET_CHAT_ID, secretChatId}});
}

void TDLibWrapper::importContacts(const QVariantList &contacts, bool single) {
    LOG("Importing contacts");
    this->sendRequest(QVariantMap{{_TYPE, "importContacts"}, {"contacts", contacts}, {_EXTRA, single}});
}

void TDLibWrapper::addContact(qlonglong userId, const QString &firstName, const QString &lastName, const QString &phone, bool sharePhoneNumber) {
    LOG("Adding contact" << userId << firstName);
    sendRequest(QVariantMap{
                          {_TYPE, "addContact"},
                          {CONTACT, QVariantMap{
                               {_TYPE, CONTACT},
                               {PHONE_NUMBER, phone},
                               {FIRST_NAME, firstName},
                               {LAST_NAME, lastName},
                               {USER_ID, userId}
                           }},
                          {"share_phone_number", sharePhoneNumber}
                      });
}

void TDLibWrapper::removeContacts(QStringList userIds) {
    LOG("Removing" << userIds.size() << "contacts");
    const QVariantMap extra{{_TYPE, REMOVE_CONTACTS}, {"user_ids", userIds}};
    QVariantMap requestObject = extra;
    requestObject.insert(_EXTRA, extra);
    sendRequest(requestObject);
}

void TDLibWrapper::removeContact(QString userId) {
    LOG("Removing contact" << userId);
    removeContacts(QStringList{userId});
}

void TDLibWrapper::searchChatMessages(qlonglong chatId, const QString &query, int extra, qlonglong fromMessageId, SearchMessagesFilter filter, int limit, int offset) {
    const QString filterType = getSearchMessagesFilterType(filter);

    LOG("Searching for messages" << chatId << query << fromMessageId << filterType << limit << extra);
    this->sendRequest(QVariantMap{
        {_TYPE, "searchChatMessages"},
        {CHAT_ID, chatId},
        {QUERY, query},
        {FROM_MESSAGE_ID, fromMessageId},
        {OFFSET, offset},
        {LIMIT, limit},
        {FILTER, QVariantMap{{_TYPE, filterType}}},
        {_EXTRA, QString::number(chatId)+":"+QString::number((int)filter)+":"+QString::number(extra)}
    });
}

void TDLibWrapper::searchChats(const QString &query) {
    LOG("Searching local chats" << query);
    this->sendRequest(QVariantMap{
        {_TYPE, "searchChats"},
        {QUERY, query},
        {LIMIT, 50},
        {_EXTRA, "searchChats"}
    });
}

void TDLibWrapper::searchPublicChats(const QString &query) {
    LOG("Searching public chats" << query);
    this->sendRequest(QVariantMap{
        {_TYPE, "searchPublicChats"},
        {QUERY, query},
        {_EXTRA, "searchPublicChats"}
    });
}

void TDLibWrapper::getSearchSponsoredChats(const QString &query) {
    LOG("Getting sponsored public chats for search" << query);
    this->sendRequest(QVariantMap{
        {_TYPE, "getSearchSponsoredChats"},
        {QUERY, query}
    });
}

void TDLibWrapper::readAllChatMentions(qlonglong chatId) {
    LOG("Read all chat mentions" << chatId);
    this->sendRequest(QVariantMap{{_TYPE, "readAllChatMentions"}, {CHAT_ID, chatId}});
}

void TDLibWrapper::readAllChatReactions(qlonglong chatId) {
    LOG("Read all chat reactions" << chatId);
    this->sendRequest(QVariantMap{{_TYPE, "readAllChatReactions"}, {CHAT_ID, chatId}});
}

void TDLibWrapper::toggleChatIsMarkedAsUnread(qlonglong chatId, bool isMarkedAsUnread) {
    LOG("Toggle chat is marked as unread" << chatId << isMarkedAsUnread);
    this->sendRequest(QVariantMap{
        {_TYPE, "toggleChatIsMarkedAsUnread"},
        {CHAT_ID, chatId},
        {"is_marked_as_unread", isMarkedAsUnread}
    });
}

void TDLibWrapper::toggleChatIsPinned(qlonglong chatId, bool isPinned, bool archive) {
    LOG("Toggle chat is pinned archive:" << archive << chatId << isPinned);
    this->sendRequest(QVariantMap{
        {_TYPE, "toggleChatIsPinned"},
        {CHAT_LIST, QVariantMap{{_TYPE, archive ? TYPE_CHAT_LIST_ARCHIVE : TYPE_CHAT_LIST_MAIN}}},
        {CHAT_ID, chatId},
        {"is_pinned", isPinned},
        {"is_marked_as_unread", isPinned}
    });
}

void TDLibWrapper::toggleChatIsPinnedForFolder(qlonglong chatId, bool isPinned, int folderId) {
    LOG("Toggle chat is pinned in folder" << folderId << chatId << isPinned);
    this->sendRequest(QVariantMap{
        {_TYPE, "toggleChatIsPinned"},
        {CHAT_LIST, QVariantMap{{_TYPE, TYPE_CHAT_LIST_FOLDER}, {CHAT_FOLDER_ID, folderId}}},
        {CHAT_ID, chatId},
        {"is_pinned", isPinned},
        {"is_marked_as_unread", isPinned}
    });
}

void TDLibWrapper::setChatDraftMessage(qlonglong chatId, qlonglong replyToMessageId, const QString &draft, const QVariantMap &topicId) {
    LOG("Set Draft Message" << chatId);
    QVariantMap requestObject{
        {_TYPE, "setChatDraftMessage"},
        {CHAT_ID, chatId}
    };
    if (!topicId.isEmpty())
        requestObject.insert(TOPIC_ID, topicId);

    QVariantMap draftMessage{
        {_TYPE, "draftMessage"},
        {"input_message_text", QVariantMap{
            {_TYPE, "inputMessageText"},
            {TEXT, Utilities::newFormattedText(draft)},
            {"clear_draft", draft.isEmpty()},
        }}
    };

    if (replyToMessageId != 0)
        draftMessage.insert(REPLY_TO, QVariantMap{
            {_TYPE, TYPE_INPUT_MESSAGE_REPLY_TO_MESSAGE},
            {MESSAGE_ID, replyToMessageId}
        });

    requestObject.insert(DRAFT_MESSAGE, draftMessage);
    this->sendRequest(requestObject);
}

void TDLibWrapper::getInlineQueryResults(qlonglong botUserId, qlonglong chatId, const QVariantMap &userLocation, const QString &query, const QString &offset, const QString &extra) {
    LOG("Get Inline Query Results" << chatId << query);
    QVariantMap requestObject{
        {_TYPE, "getInlineQueryResults"},
        {CHAT_ID, chatId},
        {BOT_USER_ID, botUserId},
        {QUERY, query},
        {OFFSET, offset},
        {_EXTRA, extra}
    };
    if(!userLocation.isEmpty())
        requestObject.insert("user_location", userLocation);

    this->sendRequest(requestObject);
}

void TDLibWrapper::sendInlineQueryResultMessage(qlonglong chatId, qlonglong threadId, qlonglong replyToMessageId, const QString &queryId, const QString &resultId) {
    LOG("Send Inline Query Result Message" << chatId);
    this->sendRequest(QVariantMap{
        {_TYPE, "sendInlineQueryResultMessage"},
        {CHAT_ID, chatId},
        {"message_thread_id", threadId},
        {"reply_to_message_id", replyToMessageId},
        {"query_id", queryId},
        {"result_id", resultId}
    });
}

void TDLibWrapper::sendBotStartMessage(qlonglong botUserId, qlonglong chatId, const QString &parameter, const QString &extra)
{

    LOG("Send Bot Start Message" << botUserId << chatId << parameter << extra);
    this->sendRequest(QVariantMap{
        {_TYPE, "sendBotStartMessage"},
        {BOT_USER_ID, botUserId},
        {CHAT_ID, chatId},
        {"parameter", parameter},
        {_EXTRA, extra}
    });
}

void TDLibWrapper::cancelDownloadFile(int fileId) {
    LOG("Cancel Download File" << fileId);
    this->sendRequest(QVariantMap{
        {_TYPE, "cancelDownloadFile"},
        {"file_id", fileId},
        {"only_if_pending", false}
    });
}

void TDLibWrapper::cancelUploadFile(int fileId) {
    LOG("Cancel Upload File" << fileId);
    this->sendRequest(QVariantMap{{_TYPE, "cancelUploadFile"}, {"file_id", fileId}});
}

void TDLibWrapper::deleteFile(int fileId) {
    LOG("Delete cached File" << fileId);
    this->sendRequest(QVariantMap{{_TYPE, "deleteFile"}, {"file_id", fileId}});
}

void TDLibWrapper::setName(const QString &firstName, const QString &lastName) {
    LOG("Set name of current user" << firstName << lastName);
    this->sendRequest(QVariantMap{{_TYPE, "setName"}, {FIRST_NAME, firstName}, {LAST_NAME, lastName}});
}

void TDLibWrapper::setUsername(const QString &username) {
    LOG("Set username of current user" << username);
    this->sendRequest(QVariantMap{{_TYPE, "setUsername"}, {USERNAME, username}});
}

void TDLibWrapper::setUserPrivacySettingRule(TDLibWrapper::UserPrivacySetting setting, TDLibWrapper::UserPrivacySettingRule rule) {
    LOG("Set user privacy setting rule of current user" << setting << rule);
    QVariantMap requestObject{{_TYPE, "setUserPrivacySettingRules"}};

    QVariantMap settingMap;
    switch (setting) {
    case SettingShowStatus:
        settingMap.insert(_TYPE, "userPrivacySettingShowStatus");
        break;
    case SettingShowPhoneNumber:
        settingMap.insert(_TYPE, "userPrivacySettingShowPhoneNumber");
        break;
    case SettingAllowChatInvites:
        settingMap.insert(_TYPE, "userPrivacySettingAllowChatInvites");
        break;
    case SettingShowProfilePhoto:
        settingMap.insert(_TYPE, "userPrivacySettingShowProfilePhoto");
        break;
    case SettingAllowFindingByPhoneNumber:
        settingMap.insert(_TYPE, "userPrivacySettingAllowFindingByPhoneNumber");
        break;
    case SettingShowLinkInForwardedMessages:
        settingMap.insert(_TYPE, "userPrivacySettingShowLinkInForwardedMessages");
        break;
    case SettingUnknown:
        return;
    }
    requestObject.insert("setting", settingMap);


    QVariantMap ruleMap;
    switch (rule) {
    case RuleAllowAll:
        ruleMap.insert(_TYPE, "userPrivacySettingRuleAllowAll");
        break;
    case RuleAllowContacts:
        ruleMap.insert(_TYPE, "userPrivacySettingRuleAllowContacts");
        break;
    case RuleRestrictAll:
        ruleMap.insert(_TYPE, "userPrivacySettingRuleRestrictAll");
        break;
    }
    requestObject.insert("rules", QVariantMap{{_TYPE, "userPrivacySettingRules"}, {"rules", QVariantList{ruleMap}}});

    this->sendRequest(requestObject);
}

void TDLibWrapper::getUserPrivacySettingRules(TDLibWrapper::UserPrivacySetting setting) {
    LOG("Getting user privacy setting rules of current user" << setting);
    QVariantMap requestObject{{_TYPE, "getUserPrivacySettingRules"}, {_EXTRA, setting}};

    QVariantMap settingMap;
    switch (setting) {
    case SettingShowStatus:
        settingMap.insert(_TYPE, "userPrivacySettingShowStatus");
        break;
    case SettingShowPhoneNumber:
        settingMap.insert(_TYPE, "userPrivacySettingShowPhoneNumber");
        break;
    case SettingAllowChatInvites:
        settingMap.insert(_TYPE, "userPrivacySettingAllowChatInvites");
        break;
    case SettingShowProfilePhoto:
        settingMap.insert(_TYPE, "userPrivacySettingShowProfilePhoto");
        break;
    case SettingAllowFindingByPhoneNumber:
        settingMap.insert(_TYPE, "userPrivacySettingAllowFindingByPhoneNumber");
        break;
    case SettingShowLinkInForwardedMessages:
        settingMap.insert(_TYPE, "userPrivacySettingShowLinkInForwardedMessages");
        break;
    case SettingUnknown:
        return;
    }
    requestObject.insert("setting", settingMap);

    this->sendRequest(requestObject);
}

void TDLibWrapper::setProfilePhoto(const QString &filePath) {
    LOG("Set a profile photo" << filePath);
    this->sendRequest(QVariantMap{
        {_TYPE, "setProfilePhoto"},
        {_EXTRA, "setProfilePhoto"},
        {PHOTO, QVariantMap{
            {_TYPE, "inputChatPhotoStatic"},
            {PHOTO, QVariantMap{
                {_TYPE, TYPE_INPUT_FILE_LOCAL},
                {PATH, filePath}
            }}
        }}
    });
}

void TDLibWrapper::deleteProfilePhoto(const QString &profilePhotoId) {
    LOG("Delete a profile photo" << profilePhotoId);
    this->sendRequest(QVariantMap{
        {_TYPE, "deleteProfilePhoto"},
        {_EXTRA, "deleteProfilePhoto"},
        {"profile_photo_id", profilePhotoId}
    });
}

void TDLibWrapper::changeStickerSet(const QString &stickerSetId, bool isInstalled) {
    LOG("Change sticker set" << stickerSetId << isInstalled);
    this->sendRequest(QVariantMap{
        {_TYPE, "changeStickerSet"},
        {_EXTRA, isInstalled ? "installStickerSet" : "removeStickerSet"},
        {"set_id", stickerSetId},
        {"is_installed", isInstalled}
    });
}

void TDLibWrapper::getActiveSessions() {
    LOG("Get active sessions");
    this->sendRequest(QVariantMap{{_TYPE, "getActiveSessions"}});
}

void TDLibWrapper::terminateSession(const QString &sessionId) {
    LOG("Terminate session" << sessionId);
    this->sendRequest(QVariantMap{
        {_TYPE, "terminateSession"},
        {_EXTRA, "terminateSession"},
        {"session_id", sessionId}
    });
}

void TDLibWrapper::getMessageAvailableReactions(qlonglong chatId, qlonglong messageId) {
    LOG("Get available reactions for message" << chatId << messageId);
    this->sendRequest(QVariantMap{
        {_TYPE, "getMessageAvailableReactions"},
        {_EXTRA, QString::number(messageId)},
        {CHAT_ID, chatId},
        {MESSAGE_ID, messageId}
    });
}

void TDLibWrapper::addMessageReaction(qlonglong chatId, qlonglong messageId, const QString &reaction) {
    LOG("Add message reaction" << chatId << messageId << reaction);
    this->sendRequest(QVariantMap{
                          {_TYPE, "addMessageReaction"},
                          {CHAT_ID, chatId},
                          {MESSAGE_ID, messageId},
                          {"is_big", false},
                          {REACTION_TYPE, QVariantMap{{_TYPE, REACTION_TYPE_EMOJI}, {EMOJI, reaction}}},
                      });
}

void TDLibWrapper::removeMessageReaction(qlonglong chatId, qlonglong messageId, const QString &reaction) {
    LOG("Remove message reaction" << chatId << messageId << reaction);
    this->sendRequest(QVariantMap{
                          {_TYPE, "removeMessageReaction"},
                          {CHAT_ID, chatId},
                          {MESSAGE_ID, messageId},
                          {REACTION_TYPE, QVariantMap{{_TYPE, REACTION_TYPE_EMOJI}, {EMOJI, reaction}}}
                      });
}

void TDLibWrapper::setNetworkType(NetworkType networkType) {
    LOG("Set network type" << networkType);

    QVariantMap requestObject{{_TYPE, "setNetworkType"}, {_EXTRA, "setNetworkType"}};
    QVariantMap networkTypeObject;
    switch (networkType) {
    case Mobile:
        networkTypeObject.insert(_TYPE, "networkTypeMobile");
        break;
    case MobileRoaming:
        networkTypeObject.insert(_TYPE, "networkTypeMobileRoaming");
        break;
    case None:
        networkTypeObject.insert(_TYPE, "networkTypeNone");
        break;
    case Other:
        networkTypeObject.insert(_TYPE, "networkTypeOther");
        break;
    case WiFi:
        networkTypeObject.insert(_TYPE, "networkTypeWiFi");
        break;
    default:
        networkTypeObject.insert(_TYPE, "networkTypeOther");
        break;
    }
    requestObject.insert(TYPE, networkTypeObject);

    this->sendRequest(requestObject);
}

void TDLibWrapper::setInactiveSessionTtl(int days) {
    QVariantMap requestObject;
    this->sendRequest(QVariantMap{{_TYPE, "setInactiveSessionTtl"}, {"inactive_session_ttl_days", days}});
}

QVariantMap TDLibWrapper::getUserInformation() {
    return this->userInformation;
}

QVariantMap TDLibWrapper::getUserInformation(const QString &userId) {
    // LOG("Returning user information for ID" << userId);
    return this->usersById.value(userId.toLongLong());
}

bool TDLibWrapper::hasUserInformation(const QString &userId) {
    return this->usersById.contains(userId.toLongLong());
}

bool TDLibWrapper::hasUserNameInformation(const QString &userName) {
    return this->usersByName.contains(userName);
}

QVariantMap TDLibWrapper::getUserInformationByName(const QString &userName) {
    return this->usersById.value(this->usersByName.value(userName.toLower()));
}

bool TDLibWrapper::hasSuperGroupNameInformation(const QString &name) {
    return this->superGroupsByName.contains(name);
}

QVariantMap TDLibWrapper::getSupergroupInformationByName(const QString &name) {
    return this->superGroupsByName.value(name.toLower()).toMap();
}

TDLibWrapper::UserPrivacySettingRule TDLibWrapper::getUserPrivacySettingRule(TDLibWrapper::UserPrivacySetting userPrivacySetting) {
    return this->userPrivacySettingRules.value(userPrivacySetting, UserPrivacySettingRule::RuleAllowAll);
}

QVariantMap TDLibWrapper::getBasicGroup(qlonglong groupId) const {
    const Group* group = basicGroups.value(groupId);
    if (group) {
        LOG("Returning basic group information for ID" << groupId);
        return group->groupInfo;
    } else {
        LOG("No super group information for ID" << groupId);
        return QVariantMap();
    }
}

QVariantMap TDLibWrapper::getSuperGroup(qlonglong groupId) const {
    const Group* group = superGroups.value(groupId);
    if (group) {
        LOG("Returning super group information for ID" << groupId);
        return group->groupInfo;
    } else {
        LOG("No super group information for ID" << groupId);
        return QVariantMap();
    }
}

QVariantMap TDLibWrapper::getChat(qlonglong chatId) {
    LOG("Returning chat information for ID" << chatId);
    if (this->chats.contains(chatId))
        return this->chats.value(chatId)->chatData;
    return QVariantMap();
}

bool TDLibWrapper::hasChatData(qlonglong chatId) {
    LOG("Checking if have chat data for ID" << chatId);
    return this->chats.contains(chatId);
}

ChatData* TDLibWrapper::getChatData(qlonglong chatId) {
    LOG("Returning chat data for ID" << chatId);
    if (this->chats.contains(chatId))
        return this->chats.value(chatId);
    return nullptr;
}

ChatData* TDLibWrapper::getChatDataForce(qlonglong chatId) {
    LOG("Forcefully returning chat data for ID" << chatId);
    if (!this->chats.contains(chatId))
        this->chats.insert(chatId, new ChatData(this, this->utilities, chatId));

    return this->chats.value(chatId);
}

QStringList TDLibWrapper::getChatReactions(qlonglong chatId) {
    LOG("Obtaining chat reactions for chat" << chatId);
    const QVariant available_reactions(getChat(chatId).value(CHAT_AVAILABLE_REACTIONS));
    const QVariantMap map(available_reactions.toMap());
    const QString reactions_type(map.value(_TYPE).toString());
    if (reactions_type == CHAT_AVAILABLE_REACTIONS_ALL) {
        LOG("Chat uses all available reactions, currently available number" << activeEmojiReactions.size());
        return activeEmojiReactions;
    } else if (reactions_type == CHAT_AVAILABLE_REACTIONS_SOME) {
        LOG("Chat uses reduced set of reactions");
        const QVariantList reactions(map.value(REACTIONS).toList());
        const int n = reactions.count();
        QStringList emojis;

        // "available_reactions": {
        //     "@type": "chatAvailableReactionsSome",
        //     "reactions": [
        //         {
        //             "@type": "reactionTypeEmoji",
        //             "emoji": "..."
        //         },
        emojis.reserve(n);
        for (int i = 0; i < n; i++) {
            const QVariantMap reaction(reactions.at(i).toMap());
            if (reaction.value(_TYPE).toString() == REACTION_TYPE_EMOJI) {
                const QString emoji(reaction.value(EMOJI).toString());
                if (!emoji.isEmpty()) {
                    emojis.append(emoji);
                }
            }
        }
        LOG("Found emojis for this chat" << emojis.size());
        return emojis;
    } else if (reactions_type.isEmpty()) {
        LOG("No chat reaction type specified, using all reactions");
        return available_reactions.toStringList();
    } else {
        LOG("Unknown chat reaction type" << reactions_type);
        return QStringList();
    }
}

QVariantMap TDLibWrapper::getSecretChatFromCache(qlonglong secretChatId) {
    return this->secretChats.value(secretChatId);
}

QVariant TDLibWrapper::getOption(const QString &optionName) {
    return this->options->value(optionName);
}

void TDLibWrapper::copyFileToDownloads(const QString &filePath, bool openAfterCopy) {
    LOG("Copy file to downloads" << filePath << openAfterCopy);
    QFileInfo fileInfo(filePath);
    if (fileInfo.exists()) {
        QString downloadFilePath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/" + fileInfo.fileName();
        if (QFile::exists(downloadFilePath)) {
            if (openAfterCopy) {
                QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
            } else {
                emit copyToDownloadsSuccessful(fileInfo.fileName(), downloadFilePath);
            }
        } else {
            if (QFile::copy(filePath, downloadFilePath)) {
                if (openAfterCopy) {
                    QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
                } else {
                    emit copyToDownloadsSuccessful(fileInfo.fileName(), downloadFilePath);
                }
            } else {
                emit copyToDownloadsError(fileInfo.fileName(), downloadFilePath);
            }
        }
    } else {
        emit copyToDownloadsError(fileInfo.fileName(), filePath);
    }
}

bool TDLibWrapper::getJoinChatRequested() {
    return this->joinChatRequested;
}

void TDLibWrapper::registerJoinChat() {
    this->joinChatRequested = false;
}

DBusAdaptor *TDLibWrapper::getDBusAdaptor() {
    return this->dbusInterface->getDBusAdaptor();
}

void TDLibWrapper::handleAuthorizationStateChanged(const QString &authorizationState, const QVariantMap authorizationStateData) {
    if (authorizationState == "authorizationStateClosed") {
        this->authorizationState = AuthorizationState::Closed;
    }

    if (authorizationState == "authorizationStateClosing") {
        this->authorizationState = AuthorizationState::Closing;
    }

    if (authorizationState == "authorizationStateLoggingOut") {
        this->authorizationState = AuthorizationState::LoggingOut;
    }

    if (authorizationState == "authorizationStateReady") {
        this->authorizationState = AuthorizationState::AuthorizationReady;
    }

    if (authorizationState == "authorizationStateWaitCode") {
        this->authorizationState = AuthorizationState::WaitCode;
    }

    if (authorizationState == "authorizationStateWaitEncryptionKey") {
        this->setEncryptionKey();
        this->authorizationState = AuthorizationState::WaitEncryptionKey;
    }

    if (authorizationState == "authorizationStateWaitOtherDeviceConfirmation") {
        this->authorizationState = AuthorizationState::WaitOtherDeviceConfirmation;
    }

    if (authorizationState == "authorizationStateWaitPassword") {
        this->authorizationState = AuthorizationState::WaitPassword;
    }

    if (authorizationState == "authorizationStateWaitPhoneNumber") {
        this->authorizationState = AuthorizationState::WaitPhoneNumber;
    }

    if (authorizationState == "authorizationStateWaitRegistration") {
        this->authorizationState = AuthorizationState::WaitRegistration;
    }

    if (authorizationState == "authorizationStateWaitTdlibParameters") {
        this->setInitialParameters();
        this->authorizationState = AuthorizationState::WaitTdlibParameters;
    }
    this->authorizationStateData = authorizationStateData;
    emit authorizationStateChanged(this->authorizationState, this->authorizationStateData);
}

void TDLibWrapper::handleOptionUpdated(const QString &optionName, const QVariant &optionValue) {
    this->options->insert(optionName, optionValue);
    emit optionUpdated(optionName, optionValue);
    if (optionName == "version") {
        const QString version = optionValue.toString();
        const QStringList parts(version.split('.'));
        uint major, minor, release;
        bool ok;
        if (parts.count() >= 3 &&
           (major = parts.at(0).toInt(&ok), ok) &&
           (minor = parts.at(1).toInt(&ok), ok) &&
           (release = parts.at(2).toInt(&ok), ok)) {
            versionNumber = VERSION_NUMBER(major, minor, release);
        }
    } else if (optionName == MY_ID) {
        QString ownUserId = optionValue.toString();
        this->userInformation = this->getUserInformation(ownUserId);
        emit ownUserIdFound(ownUserId);
    }
}

qlonglong TDLibWrapper::myUserId() const {
    return options->value(MY_ID).toLongLong();
}

void TDLibWrapper::handleConnectionStateChanged(const QString &connectionState) {
    if (connectionState == "connectionStateConnecting") {
        this->connectionState = ConnectionState::Connecting;
    }
    if (connectionState == "connectionStateConnectingToProxy") {
        this->connectionState = ConnectionState::ConnectingToProxy;
    }
    if (connectionState == "connectionStateReady") {
        this->connectionState = ConnectionState::ConnectionReady;
    }
    if (connectionState == "connectionStateUpdating") {
        this->connectionState = ConnectionState::Updating;
    }
    if (connectionState == "connectionStateWaitingForNetwork") {
        this->connectionState = ConnectionState::WaitingForNetwork;
    }

    emit connectionStateChanged(this->connectionState);
}

void TDLibWrapper::handleUserUpdated(const QVariantMap &updatedUserInformation) {
    qlonglong updatedUserId = updatedUserInformation.value(ID).toLongLong();
    if (updatedUserId == this->options->value(MY_ID).toLongLong()) {
        LOG("Current user information updated");
        this->userInformation = updatedUserInformation;
        emit ownUserUpdated(updatedUserInformation);
    }
    LOG("User information updated:" << updatedUserInformation.value(USERNAMES).toMap().value(EDITABLE_USERNAME).toString() << updatedUserInformation.value(FIRST_NAME).toString() << updatedUserInformation.value(LAST_NAME).toString());
    updateUserInformation(updatedUserId, updatedUserInformation);
}

void TDLibWrapper::handleUserStatusUpdated(qlonglong userId, const QVariantMap &userStatusInformation) {
    if (userId == this->options->value(MY_ID).toLongLong()) {
        LOG("Current user status information updated");
        this->userInformation.insert(STATUS, userStatusInformation);
    }
    QVariantMap updatedUserInformation = this->usersById.value(userId);
    if(updatedUserInformation.value(STATUS) == userStatusInformation) {
        return;
    }
    LOG("User status information updated:" << userId << userStatusInformation.value(_TYPE).toString());
    updatedUserInformation.insert(STATUS, userStatusInformation);
    updateUserInformation(userId, updatedUserInformation);
}

void TDLibWrapper::updateUserInformation(qlonglong userId, const QVariantMap &userInformation) {
    const QString username = userInformation.value(USERNAMES).toMap().value(EDITABLE_USERNAME).toString().toLower();
    if (hasUserInformation(QString::number(userId))) {
        const QString prevUsername = getUserInformation(QString::number(userId)).value(USERNAMES).toMap().value(EDITABLE_USERNAME).toString().toLower();
        if (prevUsername != username)
            this->usersByName.remove(prevUsername);
    }
    this->usersById.insert(userId, userInformation);
    this->usersByName.insert(username, userId);
    emit userUpdated(userId, userInformation);
}

void TDLibWrapper::handleFileUpdated(const QVariantMap &fileInformation) {
    emit fileUpdated(fileInformation.value(ID).toInt(), fileInformation);
}

void TDLibWrapper::handleNewChatDiscovered(const QVariantMap &chatInformation) {
    qlonglong chatId = chatInformation.value(ID).toLongLong();
    ChatData *chat;
    if (this->chats.contains(chatId)) {
        // Chat can be forcefully added when other updates on it are received before updateNewChat (see getChatDataForce)
        LOG("Chat information discovered for previously forcefully added chat");
        chat = this->chats.value(chatId);
        chat->updateChatData(chatInformation);
        emit chatRolesUpdated(chatId);
    } else {
        LOG("New chat discovered" << chatId);
        chat = new ChatData(this, this->utilities, chatInformation);
        this->chats.insert(chatId, chat);
        emit newChatDiscovered(chatId, chatInformation);
    }

    for (const QVariant &chatList : chatInformation.value(CHAT_LISTS).toList()) {
        const QString chatListType = chatList.toMap().value(_TYPE).toString();
        const QVariantList positions = chatInformation.value(POSITIONS).toList();
        if (chatListType == TYPE_CHAT_LIST_MAIN) {
            LOG("Newly discovered chat added to main list" << chatId);
            const QVariantMap position = findChatPosition(positions);
            emit chatAddedToMainList(chat, position.value(ORDER).toLongLong(), position.value(IS_PINNED).toBool());
        } else if (chatListType == TYPE_CHAT_LIST_ARCHIVE) {
            LOG("Newly discovered chat added to archive list" << chatId);
            const QVariantMap position = findChatPosition(positions, true);
            emit chatAddedToArchiveList(chat, position.value(ORDER).toLongLong(), position.value(IS_PINNED).toBool());
        } else if (chatListType == TYPE_CHAT_LIST_FOLDER) {
            const int folderId = chatList.toMap().value(CHAT_FOLDER_ID).toInt();
            LOG("Newly discovered chat added to a folder list" << folderId);
            const QVariantMap position = findChatPositionForFolder(positions, folderId);
            emit chatAddedToFolderList(folderId, chat, position.value(ORDER).toLongLong(), position.value(IS_PINNED).toBool());
        }
    }
}

void TDLibWrapper::handleChatAddedToList(const QVariantMap &chatList, qlonglong chatId) {
    if (this->chats.contains(chatId)) {
        ChatData *chat = this->chats.value(chatId);
        const QString chatListType = chatList.value(_TYPE).toString();
        const QVariantList positions = chat->chatData.value(POSITIONS).toList();

        if (chatListType == TYPE_CHAT_LIST_MAIN) {
            LOG("Chat added to main list" << chatId);
            // TODO: update positions field when needed (maybe, but probably not needed)
            const QVariantMap position = findChatPosition(positions);
            emit chatAddedToMainList(chat, position.value(ORDER).toLongLong(), position.value(IS_PINNED).toBool());
        } else if (chatListType == TYPE_CHAT_LIST_ARCHIVE) {
            LOG("Chat added to archive list" << chatId);
            const QVariantMap position = findChatPosition(positions, true);
            emit chatAddedToArchiveList(chat, position.value(ORDER).toLongLong(), position.value(IS_PINNED).toBool());
        } else if (chatListType == TYPE_CHAT_LIST_FOLDER) {
            const int folderId = chatList.value(CHAT_FOLDER_ID).toInt();
            LOG("Chat added to a folder list" << folderId);
            const QVariantMap position = findChatPositionForFolder(positions, folderId);
            emit chatAddedToFolderList(folderId, chat, position.value(ORDER).toLongLong(), position.value(IS_PINNED).toBool());
        }
    }
}

void TDLibWrapper::handleChatRemovedFromList(const QVariantMap &chatList, qlonglong chatId) {
    const QString chatListType = chatList.value(_TYPE).toString();
    if (chatListType == TYPE_CHAT_LIST_MAIN) {
        LOG("Chat removed from main list" << chatId);
        emit chatRemovedFromMainList(chatId);
    } else if (chatListType == TYPE_CHAT_LIST_ARCHIVE) {
        LOG("Chat removed from archive list" << chatId);
        emit chatRemovedFromArchiveList(chatId);
    } else if (chatListType == TYPE_CHAT_LIST_FOLDER) {
        const int folderId = chatList.value(CHAT_FOLDER_ID).toInt();
        LOG("Chat removed from a folder list" << folderId);
        emit chatRemovedFromFolderList(folderId, chatId);
    }
}

void TDLibWrapper::handleChatPositionUpdated(qlonglong chatId, const QVariantMap &position) {
    const QVariantMap chatList = position.value(LIST).toMap();
    const QString chatListType = chatList.value(_TYPE).toString();
    const qlonglong order = position.value(ORDER).toLongLong();
    const bool isPinned = position.value(IS_PINNED).toBool();

    if (chatListType == TYPE_CHAT_LIST_MAIN) {
        LOG("Chat position updated in main list for ID" << chatId << "new order" << order << "is pinned" << isPinned);
        emit mainChatListChatPositionUpdated(chatId, order, isPinned);
    } else if (chatListType == TYPE_CHAT_LIST_ARCHIVE) {
        LOG("Chat position updated in archive list for ID" << chatId << "new order" << order << "is pinned" << isPinned);
        emit archiveChatListChatPositionUpdated(chatId, order, isPinned);
    } else if (chatListType == TYPE_CHAT_LIST_FOLDER) {
        const int folderId = chatList.value(CHAT_FOLDER_ID).toInt();
        LOG("Chat position updated in a folder list" << folderId << "for ID" << chatId << "new order" << order << "is pinned" << isPinned);
        emit folderChatListChatPositionUpdated(folderId, chatId, order, isPinned);
    }

    emit someChatListUpdated();
}

void TDLibWrapper::updateChatPositions(qlonglong chatId, const QVariantList &positions) {
    for (const QVariant &position : positions)
        handleChatPositionUpdated(chatId, position.toMap());
}

void TDLibWrapper::handleChatLastMessageUpdated(qlonglong chatId, const QVariantMap &lastMessage, const QVariantList &positions) {
    LOG("Chat last message updated" << chatId);
    emit chatRolesUpdated(chatId, this->getChatDataForce(chatId)->updateLastMessage(lastMessage));

    emit someChatListUpdated();
    emit chatLastMessageUpdated(chatId, lastMessage);
    updateChatPositions(chatId, positions); // FIXME: this might affect performance
}

void TDLibWrapper::handleChatDraftMessageUpdated(qlonglong chatId, const QVariantMap &draftMessage, const QVariantList &positions) {
    LOG("Chat draft message updated" << chatId);
    this->getChatDataForce(chatId)->chatData.insert(DRAFT_MESSAGE, draftMessage);
    emit chatRolesUpdated(chatId, QVector<int>{ChatData::RoleDraftMessageDate, ChatData::RoleDraftMessageText});

    emit someChatListUpdated();
    updateChatPositions(chatId, positions); // FIXME: this might affect performance
}

void TDLibWrapper::handleChatReadInboxUpdated(const QString &chatId, const QString &lastReadInboxMessageId, int unreadCount) {
    bool ok;
    qlonglong id = chatId.toLongLong(&ok);
    if (ok) {
        ChatData *chatData = this->getChatDataForce(id);

        QVector<int> changedRoles;
        changedRoles.append(ChatData::RoleDisplay);
        if (chatData->updateUnreadCount(unreadCount))
            changedRoles.append(ChatData::RoleUnreadCount);
        if (chatData->updateLastReadInboxMessageId(lastReadInboxMessageId.toLongLong()))
            changedRoles.append(ChatData::RoleLastReadInboxMessageId);
        emit chatRolesUpdated(id, changedRoles);
    }
    emit chatReadInboxUpdated(chatId, lastReadInboxMessageId, unreadCount);
}

void TDLibWrapper::handleChatReadOutboxUpdated(const QString &chatId, const QString &lastReadOutboxMessageId) {
    bool ok;
    qlonglong id = chatId.toLongLong(&ok);
    if (ok) {
        if (this->getChatDataForce(id)->updateLastReadOutboxMessageId(lastReadOutboxMessageId.toLongLong()))
            emit chatRolesUpdated(id, QVector<int>{ChatData::RoleLastMessageStatus});

        emit chatReadOutboxUpdated(chatId, lastReadOutboxMessageId);
    }
}

void TDLibWrapper::handleChatTitleUpdated(qlonglong chatId, const QString &title) {
    this->getChatDataForce(chatId)->chatData.insert(TITLE, title);
    emit chatRolesUpdated(chatId, QVector<int>{ChatData::RoleTitle});
    emit chatTitleUpdated(chatId, title);
}

void TDLibWrapper::handleChatPhotoUpdated(qlonglong chatId, const QVariantMap &photo) {
    this->getChatDataForce(chatId)->chatData.insert(PHOTO, photo);
    emit chatRolesUpdated(chatId, QVector<int>{ChatData::RolePhotoSmall});
    emit chatPhotoUpdated(chatId, photo);
}

void TDLibWrapper::handleChatNotificationSettingsUpdated(const QString &chatId, const QVariantMap chatNotificationSettings) {
    bool ok;
    qlonglong id = chatId.toLongLong(&ok);
    if (ok) {
        this->getChatDataForce(id)->chatData.insert(NOTIFICATION_SETTINGS, chatNotificationSettings);
        emit chatRolesUpdated(id);
    }
    emit chatNotificationSettingsUpdated(chatId, chatNotificationSettings);
}

void TDLibWrapper::handleChatIsMarkedAsUnreadUpdated(qlonglong chatId, bool chatIsMarkedAsUnread) {
    this->getChatDataForce(chatId)->chatData.insert(IS_MARKED_AS_UNREAD, chatIsMarkedAsUnread);
    emit chatRolesUpdated(chatId, QVector<int>{ChatData::RoleIsMarkedAsUnread});
    emit chatIsMarkedAsUnreadUpdated(chatId, chatIsMarkedAsUnread);
}

void TDLibWrapper::handleChatUnreadMentionCountUpdated(qlonglong chatId, int unreadMentionCount) {
    this->getChatDataForce(chatId)->chatData.insert(UNREAD_MENTION_COUNT, unreadMentionCount);
    emit chatRolesUpdated(chatId, QVector<int>{ChatData::RoleUnreadMentionCount});
    emit chatUnreadMentionCountUpdated(chatId, unreadMentionCount);
}

void TDLibWrapper::handleChatUnreadReactionCountUpdated(qlonglong chatId, int unreadReactionCount) {
    this->getChatDataForce(chatId)->chatData.insert(UNREAD_REACTION_COUNT, unreadReactionCount);
    emit chatRolesUpdated(chatId, QVector<int>{ChatData::RoleUnreadReactionCount});
    emit chatUnreadReactionCountUpdated(chatId, unreadReactionCount);
}

void TDLibWrapper::handleUnreadMessageCountUpdated(const QVariantMap &messageCountInformation) {
    const QVariantMap chatList = messageCountInformation.value(CHAT_LIST).toMap();
    const QString chatListType = chatList.value(_TYPE).toString();
    if (chatListType == TYPE_CHAT_LIST_MAIN) {
        LOG("Received unread message count update for main chat list");
        emit mainChatListUnreadMessageCountUpdated(messageCountInformation);
    } else if (chatListType == TYPE_CHAT_LIST_ARCHIVE) {
        LOG("Received unread message count update for archive chat list");
        emit archiveChatListUnreadMessageCountUpdated(messageCountInformation);
    } else if (chatListType == TYPE_CHAT_LIST_FOLDER) {
        const int folderId = chatList.value(CHAT_FOLDER_ID).toInt();
        LOG("Received unread message count update for a folder chat list" << folderId);
        emit folderChatListUnreadMessageCountUpdated(folderId, messageCountInformation);
    }
}

void TDLibWrapper::handleUnreadChatCountUpdated(const QVariantMap &chatCountInformation) {
    const QVariantMap chatList = chatCountInformation.value(CHAT_LIST).toMap();
    const QString chatListType = chatList.value(_TYPE).toString();
    if (chatListType == TYPE_CHAT_LIST_MAIN) {
        LOG("Received unread chat count update for main chat list");
        emit mainChatListUnreadChatCountUpdated(chatCountInformation);
    } else if (chatListType == TYPE_CHAT_LIST_ARCHIVE) {
        LOG("Received unread chat count update for archive chat list");
        emit archiveChatListUnreadChatCountUpdated(chatCountInformation);
    } else if (chatListType == TYPE_CHAT_LIST_FOLDER) {
        const int folderId = chatList.value(CHAT_FOLDER_ID).toInt();
        LOG("Received unread chat count update for a folder chat list" << folderId);
        emit folderChatListUnreadChatCountUpdated(folderId, chatCountInformation);
    }
}

void TDLibWrapper::handleChatAvailableReactionsUpdated(qlonglong chatId, const QVariantMap &availableReactions) {
    LOG("Updating available reactions for chat" << chatId << availableReactions);
    this->getChatDataForce(chatId)->chatData.insert(CHAT_AVAILABLE_REACTIONS, availableReactions);
    emit chatRolesUpdated(chatId, QVector<int>{ChatData::RoleAvailableReactions});
    emit chatAvailableReactionsUpdated(chatId, availableReactions);
}

void TDLibWrapper::handleBasicGroupUpdated(qlonglong groupId, const QVariantMap &groupInformation) {
    emit basicGroupUpdated(updateGroup(groupId, groupInformation, &basicGroups)->groupId);
}

void TDLibWrapper::handleSupergroupUpdated(qlonglong groupId, const QVariantMap &groupInformation) {
    superGroupsByName.insert(groupInformation.value(USERNAMES).toMap().value(EDITABLE_USERNAME).toString().toLower(), groupInformation);
    emit supergroupUpdated(updateGroup(groupId, groupInformation, &superGroups)->groupId);
}

void TDLibWrapper::handleStickerSets(const QVariantList &stickerSets, int totalCount, const QString &extra) {
    if (extra.startsWith("installed")) {
        StickerType type = (StickerType)extra.mid(9).toInt();
        LOG("Installed sticker sets received" << type << totalCount);
        emit installedStickerSetsReceived(type, stickerSets);
    } else {
        LOG("Unknown sticker sets received" << totalCount);
        emit stickerSetsReceived(stickerSets);
    }
}

void TDLibWrapper::handleOpenWithChanged() {
    if (this->appSettings->useOpenWith()) {
        this->initializeOpenWith();
    } else {
        this->removeOpenWith();
    }
}

void TDLibWrapper::handleSecretChatReceived(qlonglong secretChatId, const QVariantMap &secretChat) {
    this->secretChats.insert(secretChatId, secretChat);
    emit secretChatReceived(secretChatId, secretChat);
}

void TDLibWrapper::handleSecretChatUpdated(qlonglong secretChatId, const QVariantMap &secretChat) {
    this->secretChats.insert(secretChatId, secretChat);

    for (ChatData *chat : this->chats) {
        if (chat->chatType != TDLibWrapper::ChatTypeSecret) continue;
        if (chat->chatData.value(TYPE).toMap().value(SECRET_CHAT_ID).toLongLong() != secretChatId) continue;

        const QVector<int> changedRoles = chat->updateSecretChat(secretChat);
        if (!changedRoles.isEmpty())
            emit chatRolesUpdated(chat->chatId, changedRoles);
    }

    emit secretChatUpdated(secretChatId, secretChat);
}

void TDLibWrapper::handleStorageOptimizerChanged() {
    setOptionBoolean("use_storage_optimizer", appSettings->storageOptimizer());
}
void TDLibWrapper::handleSendMarkdownChanged() {
    setOptionBoolean("always_parse_markdown", appSettings->sendMarkdown());
}

void TDLibWrapper::handleErrorReceived(int code, const QString &message, const QVariant &extra) {
    const QString extraString = extra.toString();
    if (extra.userType() == QMetaType::QString && !extraString.isEmpty()) {
        if (extraString == "ping") {
            LOG("Ping error");
            emit pingErrorReceived();
            return;
        }

        QStringList parts(extraString.split(':'));
        if (parts.size() == 3 && parts.at(0) == QStringLiteral("getMessage")) {
            emit messageNotFound(parts.at(1).toLongLong(), parts.at(2).toLongLong());
        } else if (extraString.startsWith("getInternalLinkType:") && code == 404) {
            LOG("Opening non-internal URL externally");
            QString url = extraString.mid(20);
            if (!url.contains("://"))
                url.prepend("https://");

            QDesktopServices::openUrl(url);
            return;
        } else if (parts.size() == 3 && parts.at(0) == TYPE_GET_FORUM_TOPIC) {
            qlonglong chatId = parts.at(1).toLongLong();
            int forumTopicId = parts.at(2).toInt();
            LOG("Forum topic not found" << chatId << forumTopicId);
            emit forumTopicNotFound(chatId, forumTopicId);
            return;
        } else {
            QRegularExpressionMatch match = RE_EXTRA_CHAT_MESSAGE_COUNT.match(extraString);
            if (match.hasMatch()) {
                const QString filterType = match.captured(1);
                const bool local = !match.captured(2).isEmpty();
                const qlonglong chatId = match.captured(3).toLongLong();
                LOG("Received chat message count error" << chatId << filterType << local);

                emit chatMessageCountErrorReceived(chatId, getSearchMessagesFilterForType(filterType), local);
            }
        }
    } else if (extra.userType() == QMetaType::QVariantMap) {
        const QVariantMap map = extra.toMap();
        const QString type = map.value(_TYPE).toString();
        if (type == PROXY) {
            LOG("Proxy ping error");
            emit proxyPingErrorReceived(map.value(SERVER).toString(), map.value(PORT).toInt(), map.value(TYPE).toMap());
            return;
        }
    }

    emit errorReceived(code, message, extra);
}

void TDLibWrapper::handleMessageInformation(qlonglong chatId, qlonglong messageId, const QVariantMap &receivedInformation) {
    QString extraInformation = receivedInformation.value(_EXTRA).toString();
    if (extraInformation.startsWith("getChatPinnedMessage:")) {
        emit chatPinnedMessageUpdated(chatId, messageId);
    }
    emit receivedMessage(chatId, messageId, receivedInformation);
}

void TDLibWrapper::handleMessageIsPinnedUpdated(qlonglong chatId, qlonglong messageId, bool isPinned) {
    if (isPinned) {
        emit chatPinnedMessageUpdated(chatId, messageId);
    } else {
        emit chatPinnedMessageUpdated(chatId, 0);
        this->getChatPinnedMessage(chatId);
    }
}

void TDLibWrapper::handleUserPrivacySettingRules(const QVariantMap &rules) {
    QVariantList newGivenRules = rules.value("rules").toList();
    // If nothing (or something unsupported is sent out) it is considered to be restricted completely
    UserPrivacySettingRule newAppliedRule = UserPrivacySettingRule::RuleRestrictAll;
    QListIterator<QVariant> givenRulesIterator(newGivenRules);
    while (givenRulesIterator.hasNext()) {
        QString givenRule = givenRulesIterator.next().toMap().value(_TYPE).toString();
        if (givenRule == "userPrivacySettingRuleAllowContacts") {
            newAppliedRule = UserPrivacySettingRule::RuleAllowContacts;
        }
        if (givenRule == "userPrivacySettingRuleAllowAll") {
            newAppliedRule = UserPrivacySettingRule::RuleAllowAll;
        }
    }
    UserPrivacySetting usedSetting = static_cast<UserPrivacySetting>(rules.value(_EXTRA).toInt());
    this->userPrivacySettingRules.insert(usedSetting, newAppliedRule);
    emit userPrivacySettingUpdated(usedSetting, newAppliedRule);
}

void TDLibWrapper::handleUpdatedUserPrivacySettingRules(const QVariantMap &updatedRules) {
    QString rawSetting = updatedRules.value("setting").toMap().value(_TYPE).toString();
    UserPrivacySetting usedSetting = UserPrivacySetting::SettingUnknown;
    if (rawSetting == "userPrivacySettingAllowChatInvites") {
        usedSetting = UserPrivacySetting::SettingAllowChatInvites;
    }
    if (rawSetting == "userPrivacySettingAllowFindingByPhoneNumber") {
        usedSetting = UserPrivacySetting::SettingAllowFindingByPhoneNumber;
    }
    if (rawSetting == "userPrivacySettingShowLinkInForwardedMessages") {
        usedSetting = UserPrivacySetting::SettingShowLinkInForwardedMessages;
    }
    if (rawSetting == "userPrivacySettingShowPhoneNumber") {
        usedSetting = UserPrivacySetting::SettingShowPhoneNumber;
    }
    if (rawSetting == "userPrivacySettingShowProfilePhoto") {
        usedSetting = UserPrivacySetting::SettingShowProfilePhoto;
    }
    if (rawSetting == "userPrivacySettingShowStatus") {
        usedSetting = UserPrivacySetting::SettingShowStatus;
    }
    if (usedSetting != UserPrivacySetting::SettingUnknown) {
        QVariantMap rawRules = updatedRules.value("rules").toMap();
        rawRules.insert(_EXTRA, usedSetting);
        this->handleUserPrivacySettingRules(rawRules);
    }
}

void TDLibWrapper::handleSponsoredMessagesReceived(qlonglong chatId, const QVariantList &messages, int messagesBetween) {
    switch (appSettings->sponsoredMess()) {
    case AppSettings::SponsoredMessHandle:
        emit sponsoredMessagesReceived(chatId, messages, messagesBetween);
        break;
    case AppSettings::SponsoredMessHandleCustomMessagesBetween:
        LOG("Handling sponsored messages with custom messagesBetween" << messagesBetween);
        emit sponsoredMessagesReceived(chatId, messages, appSettings->sponsoredMessagesMessagesBetween());
    case AppSettings::SponsoredMessAutoView:
        LOG("Auto-viewing sponsored messages");
        for (const QVariant &message : messages)
            viewMessage(chatId, message.toMap().value(MESSAGE_ID).toULongLong(), false);
        break;
    case AppSettings::SponsoredMessIgnore:
        LOG("Ignoring chat sponsored messages");
        break;
    }
}

void TDLibWrapper::handleActiveEmojiReactionsUpdated(const QStringList& emojis) {
    if (activeEmojiReactions != emojis) {
        activeEmojiReactions = emojis;
        LOG(emojis.count() << "reaction(s) available");
        emit reactionsUpdated();
    }
}

void TDLibWrapper::handleNetworkConfigurationChanged(const QNetworkConfiguration &config) {
    LOG("A network configuration changed: " << config.bearerTypeName() << config.state());
    LOG("Checking overall network state...");

    bool wifiFound = false;
    bool mobileFound = false;

    QList<QNetworkConfiguration> activeConfigurations = networkConfigurationManager->allConfigurations(QNetworkConfiguration::Active);
    QListIterator<QNetworkConfiguration> configurationIterator(activeConfigurations);
    while (configurationIterator.hasNext()) {
        QNetworkConfiguration activeConfiguration = configurationIterator.next();
        if (activeConfiguration.bearerType() == QNetworkConfiguration::BearerWLAN
                || activeConfiguration.bearerType() == QNetworkConfiguration::BearerEthernet) {
            LOG("Active WiFi found...");
            wifiFound = true;
        }
        if (activeConfiguration.bearerType() == QNetworkConfiguration::Bearer2G
                || activeConfiguration.bearerType() == QNetworkConfiguration::Bearer3G
                || activeConfiguration.bearerType() == QNetworkConfiguration::Bearer4G
                || activeConfiguration.bearerType() == QNetworkConfiguration::BearerCDMA2000
                || activeConfiguration.bearerType() == QNetworkConfiguration::BearerEVDO
                || activeConfiguration.bearerType() == QNetworkConfiguration::BearerHSPA
                || activeConfiguration.bearerType() == QNetworkConfiguration::BearerLTE
                || activeConfiguration.bearerType() == QNetworkConfiguration::BearerWCDMA) {
            LOG("Active mobile connection found...");
            mobileFound = true;
        }
    }
    if (wifiFound) {
        this->setNetworkType(NetworkType::WiFi);
    } else if (mobileFound) {
        this->setNetworkType(NetworkType::Mobile);
    } else {
        this->setNetworkType(NetworkType::None);
    }
}

void TDLibWrapper::setInitialParameters() {
    LOG("Setting TDLib initial parameters");
    QVariantMap parameters;
    parameters.insert(_TYPE, "setTdlibParameters");

    parameters.insert("api_id", TDLIB_API_ID);
    parameters.insert("api_hash", TDLIB_API_HASH);
    parameters.insert("database_directory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/tdlib");
    parameters.insert("files_directory", QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/tdlib");
    bool onlineOnlyMode = this->appSettings->onlineOnlyMode();
    parameters.insert("use_file_database", !onlineOnlyMode);
    parameters.insert("use_chat_info_database", !onlineOnlyMode);
    parameters.insert("use_message_database", !onlineOnlyMode);
    parameters.insert("use_secret_chats", true);
    parameters.insert("system_language_code", QLocale::system().name());
    QSettings hardwareSettings("/etc/hw-release", QSettings::NativeFormat);
    parameters.insert("device_model", hardwareSettings.value("NAME", "Unknown Mobile Device").toString());
    parameters.insert("system_version", QSysInfo::prettyProductName());
    parameters.insert("application_version", "0.17");
    // parameters.insert("use_test_dc", true);

    this->sendRequest(parameters);
}

void TDLibWrapper::setEncryptionKey() {
    LOG("Setting database encryption key");
    QVariantMap requestObject;
    requestObject.insert(_TYPE, "checkDatabaseEncryptionKey");
    // see https://github.com/tdlib/td/issues/188#issuecomment-379536139
    requestObject.insert("encryption_key", "");
    this->sendRequest(requestObject);
}

void TDLibWrapper::setLogVerbosityLevel() {
    LOG("Setting log verbosity level to something less chatty");
    this->sendRequest({{_TYPE, "setLogVerbosityLevel"}, {"new_verbosity_level", 2}});
}

void TDLibWrapper::initializeOpenWith() {
    LOG("Initialize open-with");LOG("Checking standard open URL file...");

    const QStringList sailfishOSVersion = QSysInfo::productVersion().split(".");
    int sailfishOSMajorVersion = sailfishOSVersion.value(0).toInt();
    int sailfishOSMinorVersion = sailfishOSVersion.value(1).toInt();

    const QString applicationsLocation(QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation));
    const QString openUrlFilePath(applicationsLocation + "/open-url.desktop");
    if (sailfishOSMajorVersion < 4 || ( sailfishOSMajorVersion == 4 && sailfishOSMinorVersion < 2 )) {
        if (QFile::exists(openUrlFilePath)) {
            LOG("Standard open URL file exists, good!");
        } else {
            LOG("Copying standard open URL file to " << openUrlFilePath);
            QFile::copy("/usr/share/applications/open-url.desktop", openUrlFilePath);
            QProcess::startDetached("update-desktop-database " + applicationsLocation);
        }
    } else {
        const QString sailfishBrowserFilePath(applicationsLocation + "/sailfish-browser.desktop");
        if (QFile::exists(sailfishBrowserFilePath)) {
            LOG("Removing existing local Sailfish browser file, that was not working as expected in 0.10...!");
            QFile::remove(sailfishBrowserFilePath);
            QProcess::startDetached("update-desktop-database " + applicationsLocation);
        }
        if (QFile::exists(openUrlFilePath)) {
            LOG("Old open URL file exists, that needs to go away...!");
            QFile::remove(openUrlFilePath);
            QProcess::startDetached("update-desktop-database " + applicationsLocation);
        }
        // Something special for Verla...
        if (sailfishOSMajorVersion == 4 && sailfishOSMinorVersion == 2) {
            LOG("Creating open URL file at " << openUrlFilePath);
            QFile openUrlFile(openUrlFilePath);
            if (openUrlFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream fileOut(&openUrlFile);
                fileOut.setCodec("UTF-8");
                fileOut << QString("[Desktop Entry]").toUtf8() << "\n";
                fileOut << QString("Type=Application").toUtf8() << "\n";
                fileOut << QString("Name=Browser").toUtf8() << "\n";
                fileOut << QString("Icon=icon-launcher-browser").toUtf8() << "\n";
                fileOut << QString("NoDisplay=true").toUtf8() << "\n";
                fileOut << QString("X-MeeGo-Logical-Id=sailfish-browser-ap-name").toUtf8() << "\n";
                fileOut << QString("X-MeeGo-Translation-Catalog=sailfish-browser").toUtf8() << "\n";
                fileOut << QString("MimeType=text/html;x-scheme-handler/http;x-scheme-handler/https;").toUtf8() << "\n";
                fileOut << QString("X-Maemo-Service=org.sailfishos.browser.ui").toUtf8() << "\n";
                fileOut << QString("X-Maemo-Object-Path=/ui").toUtf8() << "\n";
                fileOut << QString("X-Maemo-Method=org.sailfishos.browser.ui.openUrl").toUtf8() << "\n";
                fileOut.flush();
                openUrlFile.close();
                QProcess::startDetached("update-desktop-database " + applicationsLocation);
            }
        }
    }

    const QString desktopFilePath(applicationsLocation + "/harbour-fernschreiber2-open-url.desktop");
    QFile desktopFile(desktopFilePath);
    if (desktopFile.exists()) {
        LOG("Fernschreiber open-with file existing, removing...");
        desktopFile.remove();
        QProcess::startDetached("update-desktop-database " + applicationsLocation);
    }
    LOG("Creating Fernschreiber open-with file at " << desktopFile.fileName());
    if (desktopFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream fileOut(&desktopFile);
        fileOut.setCodec("UTF-8");
        fileOut << QString("[Desktop Entry]").toUtf8() << "\n";
        fileOut << QString("Type=Application").toUtf8() << "\n";
        fileOut << QString("Name=Ferniegram").toUtf8() << "\n";
        fileOut << QString("Icon=harbour-fernschreiber2").toUtf8() << "\n";
        fileOut << QString("NotShowIn=X-MeeGo;").toUtf8() << "\n";
        if (sailfishOSMajorVersion < 4 || ( sailfishOSMajorVersion == 4 && sailfishOSMinorVersion < 1 )) {
            fileOut << QString("MimeType=text/html;x-scheme-handler/http;x-scheme-handler/https;x-scheme-handler/tg;").toUtf8() << "\n";
        } else {
            fileOut << QString("MimeType=x-url-handler/t.me;x-scheme-handler/tg;").toUtf8() << "\n";
        }
        fileOut << QString("X-Maemo-Service=io.github.roundedrectangle.fernschreiber2").toUtf8() << "\n";
        fileOut << QString("X-Maemo-Object-Path=/io/github/roundedrectangle/fernschreiber2").toUtf8() << "\n";
        fileOut << QString("X-Maemo-Method=io.github.roundedrectangle.fernschreiber2.openUrl").toUtf8() << "\n";
        fileOut << QString("Hidden=true;").toUtf8() << "\n";
        fileOut.flush();
        desktopFile.close();
        QProcess::startDetached("update-desktop-database " + applicationsLocation);
    }

    QString dbusPathName = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/dbus-1/services";
    QDir dbusPath(dbusPathName);
    if (!dbusPath.exists()) {
        LOG("Creating D-Bus directory" << dbusPathName);
        dbusPath.mkpath(dbusPathName);
    }
    QString dbusServiceFileName = dbusPathName + "/io.github.roundedrectangle.fernschreiber2.service";
    QFile dbusServiceFile(dbusServiceFileName);
    if (dbusServiceFile.exists()) {
        LOG("D-BUS service file existing, removing to ensure proper re-creation...");
        dbusServiceFile.remove();
    }
    LOG("Creating D-Bus service file at" << dbusServiceFile.fileName());
    if (dbusServiceFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream fileOut(&dbusServiceFile);
        fileOut.setCodec("UTF-8");
        fileOut << QString("[D-BUS Service]").toUtf8() << "\n";
        fileOut << QString("Name=io.github.roundedrectangle.fernschreiber2").toUtf8() << "\n";
        fileOut << QString("Exec=/usr/bin/sailjail -- /usr/bin/harbour-fernschreiber2").toUtf8() << "\n";
        fileOut.flush();
        dbusServiceFile.close();
    }
}

void TDLibWrapper::removeOpenWith() {
    LOG("Remove open-with");
    QFile::remove(QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + "/harbour-fernschreiber2-open-url.desktop");
    QProcess::startDetached("update-desktop-database " + QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation));
}

const TDLibWrapper::Group *TDLibWrapper::updateGroup(qlonglong groupId, const QVariantMap &groupInfo, QHash<qlonglong,Group*> *groups) {
    Group* group = groups->value(groupId);
    if (!group) {
        group = new Group(groupId);
        groups->insert(groupId, group);
    }
    group->groupInfo = groupInfo;

    for (ChatData *chat : this->chats) {
        const QVector<int> changedRoles = chat->updateGroup(group);
        if (!changedRoles.isEmpty())
            emit chatRolesUpdated(chat->chatId, changedRoles);
    }

    return group;
}

const TDLibWrapper::Group* TDLibWrapper::getGroup(qlonglong groupId) const {
    if (groupId) {
        const Group* group = superGroups.value(groupId);
        return group ? group : basicGroups.value(groupId);
    }
    return Q_NULLPTR;
}

TDLibWrapper::ChatType TDLibWrapper::chatTypeFromString(const QString &type) {
    return (type == QStringLiteral("chatTypePrivate")) ? ChatTypePrivate :
        (type == QStringLiteral("chatTypeBasicGroup")) ? ChatTypeBasicGroup :
        (type == QStringLiteral("chatTypeSupergroup")) ? ChatTypeSupergroup :
        (type == QStringLiteral("chatTypeSecret")) ?  ChatTypeSecret :
        ChatTypeUnknown;
}

TDLibWrapper::ChatMemberStatus TDLibWrapper::chatMemberStatusFromString(const QString &status) {
    // Most common ones first
    return (status == QStringLiteral("chatMemberStatusMember")) ? ChatMemberStatusMember :
        (status == QStringLiteral("chatMemberStatusLeft")) ? ChatMemberStatusLeft :
        (status == QStringLiteral("chatMemberStatusCreator")) ? ChatMemberStatusCreator :
        (status == QStringLiteral("chatMemberStatusAdministrator")) ?  ChatMemberStatusAdministrator :
        (status == QStringLiteral("chatMemberStatusRestricted")) ? ChatMemberStatusRestricted :
        (status == QStringLiteral("chatMemberStatusBanned")) ?  ChatMemberStatusBanned :
                                                                ChatMemberStatusUnknown;
}

TDLibWrapper::SecretChatState TDLibWrapper::secretChatStateFromString(const QString &state) {
    return (state == QStringLiteral("secretChatStateClosed")) ? SecretChatStateClosed :
        (state == QStringLiteral("secretChatStatePending")) ? SecretChatStatePending :
        (state == QStringLiteral("secretChatStateReady")) ? SecretChatStateReady :
        SecretChatStateUnknown;
}

TDLibWrapper::ChatMemberStatus TDLibWrapper::Group::chatMemberStatus() const {
    const QString statusType(groupInfo.value(STATUS).toMap().value(_TYPE).toString());
    return statusType.isEmpty() ? ChatMemberStatusUnknown : chatMemberStatusFromString(statusType);
}

bool TDLibWrapper::Group::isPublic() const {
    return !this->groupInfo.value(USERNAMES).toMap().value(ACTIVE_USERNAMES).toList().isEmpty()
            || this->groupInfo.value("has_linked_chat").toBool()
            || this->groupInfo.value("has_location").toBool()
            || this->groupInfo.value("is_direct_messages_group").toBool(); // last one is questionable
}

void TDLibWrapper::getMessageProperties(qlonglong chatId, qlonglong messageId) {
    LOG("Retrieving message properties" << chatId << messageId);
    QVariantMap requestObject{{CHAT_ID, chatId}, {MESSAGE_ID, messageId}};
    QVariantMap extra(requestObject);
    requestObject.insert(_TYPE, "getMessageProperties");
    requestObject.insert(_EXTRA, extra);
    this->sendRequest(requestObject);
}

void TDLibWrapper::getCustomEmojiStickers(QStringList ids) {
    LOG("Receiving stickers for custom emojis" << ids);
    this->sendRequest(QVariantMap{{_TYPE, "getCustomEmojiStickers"}, {"custom_emoji_ids", ids}});
}

void TDLibWrapper::getCustomEmojiStickers(QString id) {
    LOG("Receiving sticker for custom emoji" << id);
    getCustomEmojiStickers(QStringList{id});
}

void TDLibWrapper::getStorageStatisticsFast() {
    this->sendRequest(QVariantMap{{_TYPE, "getStorageStatisticsFast"}});
}

void TDLibWrapper::optimizeStorage(bool entire) {
    QVariantMap requestObject{{_TYPE, "optimizeStorage"}};
    if (entire) {
        requestObject.insert("size", 0);
        QVariantList fileTypes;
        for (QString type : ALL_FILE_TYPES)
            fileTypes.append(QVariantMap{{_TYPE, type}});
        requestObject.insert("file_types", fileTypes);
    }
    this->sendRequest(requestObject);
}

void TDLibWrapper::translateText(const QVariantMap &text, const QString &languageCode, const QString &extra) {
    LOG("Translating text" << languageCode << extra);
    this->sendRequest(QVariantMap{
        {_TYPE, "translateText"},
    {TEXT, text},
    {"to_language_code", languageCode},
    {_EXTRA, extra}
    });
}

void TDLibWrapper::translateMessageText(qlonglong chatId, qlonglong messageId, const QString &languageCode) {
    LOG("Translating message text" << chatId << messageId << languageCode);
    this->sendRequest(QVariantMap{
        {_TYPE, "translateMessageText"},
        {CHAT_ID, chatId},
        {MESSAGE_ID, messageId},
        {"to_language_code", languageCode},
        {_EXTRA, "msgtr" + QString::number(chatId) + ":" + QString::number(messageId) + languageCode}
    });
}

void TDLibWrapper::summarizeMessage(qlonglong chatId, qlonglong messageId, const QString &translateToLanguageCode) {
    LOG("Summarizing message text" << chatId << messageId << translateToLanguageCode);
    this->sendRequest(QVariantMap{
        {_TYPE, "summarizeMessage"},
        {CHAT_ID, chatId},
        {MESSAGE_ID, messageId},
        {"translate_to_language_code", translateToLanguageCode},
        {_EXTRA, "summarizeMessage" + QString::number(chatId) + ":" + QString::number(messageId) + translateToLanguageCode}
    });
}

void TDLibWrapper::sendChatAction(qlonglong chatId, const QString &chatActionType, const QVariantMap &topicId) {
    LOG("Sending chat action" << chatId << chatActionType);
    QVariantMap request{
        {_TYPE, "sendChatAction"},
        {CHAT_ID, chatId},
        {"action", QVariantMap{{_TYPE, chatActionType}}}
    };

    if (!topicId.isEmpty())
        request.insert(TOPIC_ID, topicId);

    this->sendRequest(request);
}

void TDLibWrapper::searchEmojis(const QString &text) {
    LOG("Searching emojis" << text);
    this->sendRequest(QVariantMap{
                          {_TYPE, "searchEmojis"},
                          {TEXT, text},
                          {_EXTRA, text},
                          {"input_language_codes", QVariantList{{QLocale::system().name()}}}
                      });
}

void TDLibWrapper::close() {
    sendRequest(QVariantMap{{_TYPE, "close"}});
}

void TDLibWrapper::toggleSupergroupIsForum(bool isForum) {
    sendRequest(QVariantMap{{_TYPE, "toggleSupergroupIsForum"}, {"is_forum", isForum}});
}

void TDLibWrapper::handleDiceEmojisUpdated(const QStringList &emojis) {
    if (diceEmojis != emojis) {
        LOG("Dice emojis updated" << emojis);
        diceEmojis = emojis;
    }
}

bool TDLibWrapper::isDiceEmoji(const QString &text) {
    LOG("Checking if text is a dice emoji" << text);
    return diceEmojis.contains(QString(text).trimmed());
}

void TDLibWrapper::getChatListsToAddChat(qlonglong chatId) {
    LOG("Getting chat lists the chat can be added to" << chatId);
    sendRequest(QVariantMap{{_TYPE, "getChatListsToAddChat"}, {CHAT_ID, chatId}, {_EXTRA, chatId}});
}

void TDLibWrapper::addChatToList(qlonglong chatId, bool archive) {
    LOG("Adding chat to a list" << chatId << "archive" << archive);
    sendRequest(QVariantMap{{_TYPE, "addChatToList"}, {CHAT_ID, chatId}, {CHAT_LIST, QVariantMap{{_TYPE, archive ? TYPE_CHAT_LIST_ARCHIVE : TYPE_CHAT_LIST_MAIN}}}});
}

void TDLibWrapper::getArchiveChatListSettings() {
    LOG("Retrieving archive chat list settings");
    sendRequest(QVariantMap{{_TYPE, "getArchiveChatListSettings"}});
}

void TDLibWrapper::setArchiveChatListSettings(bool archiveAndMuteNewChatsFromUnknownUsers, bool keepUnmutedChatsArchived, bool keepChatsFromFoldersArchived) {
    // If this value is true while we can't set it, AUTOARCHIVE_NOT_AVAILABLE error will show up, so we double-check
    if (!this->options->value("can_archive_and_mute_new_chats_from_unknown_users").toBool())
        archiveAndMuteNewChatsFromUnknownUsers = false;

    LOG("Setting archive chat list settings");
    sendRequest(QVariantMap{{_TYPE, "setArchiveChatListSettings"}, {"settings", QVariantMap{
                                                                        {"archive_and_mute_new_chats_from_unknown_users", archiveAndMuteNewChatsFromUnknownUsers},
                                                                        {"keep_unmuted_chats_archived", keepUnmutedChatsArchived},
                                                                        {"keep_chats_from_folders_archived", keepChatsFromFoldersArchived}
                                                                    }}});
}

void TDLibWrapper::readChatList(bool archive) {
    LOG("Reading chat list archive:" << archive);
    this->sendRequest(QVariantMap{{_TYPE, TYPE_READ_CHAT_LIST}, {CHAT_LIST, QVariantMap{{_TYPE, (archive ? TYPE_CHAT_LIST_ARCHIVE : TYPE_CHAT_LIST_MAIN)}}}});
}

void TDLibWrapper::readFolderChatList(int folderId) {
    LOG("Reading folder chat list" << folderId);
    this->sendRequest(QVariantMap{{_TYPE, TYPE_READ_CHAT_LIST}, {CHAT_LIST, QVariantMap{{_TYPE, TYPE_CHAT_LIST_FOLDER}, {CHAT_FOLDER_ID, folderId}}}});
}

QString TDLibWrapper::getTopChatCategoryType(TopChatCategory category) {
    switch (category) {
    case TopChatCategoryUsers:
        return "topChatCategoryUsers";
    case TopChatCategoryBots:
        return "topChatCategoryBots";
    case TopChatCategoryCalls:
        return "topChatCategoryCalls";
    case TopChatCategoryChannels:
        return "topChatCategoryChannels";
    case TopChatCategoryForwardChats:
        return "topChatCategoryForwardChats";
    case TopChatCategoryGroups:
        return "topChatCategoryGroups";
    case TopChatCategoryInlineBots:
        return "topChatCategoryInlineBots";
    case TopChatCategoryWebAppBots:
        return "topChatCategoryWebAppBots";
    }

    return QString();
}

void TDLibWrapper::getTopChats(TopChatCategory category, int limit) {
    const QString categoryType = getTopChatCategoryType(category);
    LOG("Getting top chats for category" << categoryType);

    this->sendRequest(QVariantMap{
                          {_TYPE, "getTopChats"},
                          {"category", QVariantMap{{_TYPE, categoryType}}},
                          {LIMIT, limit},
                          {_EXTRA, categoryType}
                      });
}

void TDLibWrapper::removeTopChat(TopChatCategory category, qlonglong chatId) {
    const QString categoryType = getTopChatCategoryType(category);
    LOG("Removing top chat" << chatId << "from category" << categoryType);

    this->sendRequest(QVariantMap{
                          {_TYPE, "removeTopChat"},
                          {"category", QVariantMap{{_TYPE, categoryType}}},
                          {CHAT_ID, chatId},
                          {_EXTRA, categoryType}
                      });
}

void TDLibWrapper::searchRecentlyFoundChats(const QString &query) {
    LOG("Searching for recently found chats" << query);
    this->sendRequest(QVariantMap{{_TYPE, "searchRecentlyFoundChats"}, {QUERY, query}, {LIMIT, 50}, {_EXTRA, "searchRecentlyFoundChats"}});
}

void TDLibWrapper::clearRecentlyFoundChats() {
    LOG("Clearing recently found chats");
    this->sendRequest(QVariantMap{{_TYPE, "clearRecentlyFoundChats"}});
}

void TDLibWrapper::addRecentlyFoundChat(qlonglong chatId) {
    LOG("Adding chat to recently found chats list" << chatId);
    this->sendRequest(QVariantMap{{_TYPE, "addRecentlyFoundChat"}, {CHAT_ID, chatId}, {_EXTRA, EXTRA_RECENTLY_FOUND}});
}

void TDLibWrapper::removeRecentlyFoundChat(qlonglong chatId) {
    LOG("Removing chat from recently found chats list" << chatId);
    this->sendRequest(QVariantMap{{_TYPE, "removeRecentlyFoundChat"}, {CHAT_ID, chatId}, {_EXTRA, EXTRA_RECENTLY_FOUND}});
}


void TDLibWrapper::handleFoundChatMessagesReceived(qlonglong chatId, int extra, int extra2, const QVariantList &messages, int totalCount, qlonglong nextFromMessageId) {
    emit foundChatMessagesReceived(chatId, (SearchMessagesFilter)extra, extra2, messages, totalCount, nextFromMessageId);
}

QString TDLibWrapper::getSearchMessagesFilterType(SearchMessagesFilter filter) {
    switch (filter) {
    case SearchMessagesFilterEmpty:
        return "searchMessagesFilterEmpty";
    case SearchMessagesFilterPhotoAndVideo:
        return "searchMessagesFilterPhotoAndVideo";
    case SearchMessagesFilterAnimation:
        return "searchMessagesFilterAnimation";
    case SearchMessagesFilterAudio:
        return "searchMessagesFilterAudio";
    case SearchMessagesFilterChatPhoto:
        return "searchMessagesFilterChatPhoto";
    case SearchMessagesFilterDocument:
        return "searchMessagesFilterDocument";
    case SearchMessagesFilterFailedToSend:
        return "searchMessagesFilterFailedToSend";
    case SearchMessagesFilterMention:
        return "searchMessagesFilterMention";
    case SearchMessagesFilterPhoto:
        return "searchMessagesFilterPhoto";
    case SearchMessagesFilterPinned:
        return "searchMessagesFilterPinned";
    case SearchMessagesFilterUnreadMention:
        return "searchMessagesFilterUnreadMention";
    case SearchMessagesFilterUnreadReaction:
        return "searchMessagesFilterUnreadReaction";
    case SearchMessagesFilterUrl:
        return "searchMessagesFilterUrl";
    case SearchMessagesFilterVideo:
        return "searchMessagesFilterVideo";
    case SearchMessagesFilterVideoNote:
        return "searchMessagesFilterVideoNote";
    case SearchMessagesFilterVoiceAndVideoNote:
        return "searchMessagesFilterVoiceAndVideoNote";
    case SearchMessagesFilterVoiceNote:
        return "searchMessagesFilterVoiceNote";
    }

    return "searchMessagesFilterEmpty";
}

TDLibWrapper::SearchMessagesFilter TDLibWrapper::getSearchMessagesFilterForType(const QString &type) {
    if (type == "searchMessagesFilterAnimation")
        return SearchMessagesFilterAnimation;
    if (type == "searchMessagesFilterAudio")
        return SearchMessagesFilterAudio;
    if (type == "searchMessagesFilterChatPhoto")
        return SearchMessagesFilterChatPhoto;
    if (type == "searchMessagesFilterDocument")
        return SearchMessagesFilterDocument;
    if (type == "searchMessagesFilterEmpty")
        return SearchMessagesFilterEmpty;
    if (type == "searchMessagesFilterFailedToSend")
        return SearchMessagesFilterFailedToSend;
    if (type == "searchMessagesFilterMention")
        return SearchMessagesFilterMention;
    if (type == "searchMessagesFilterPhoto")
        return SearchMessagesFilterPhoto;
    if (type == "searchMessagesFilterPhotoAndVideo")
        return SearchMessagesFilterPhotoAndVideo;
    if (type == "searchMessagesFilterPinned")
        return SearchMessagesFilterPinned;
    if (type == "searchMessagesFilterUnreadMention")
        return SearchMessagesFilterUnreadMention;
    if (type == "searchMessagesFilterUnreadReaction")
        return SearchMessagesFilterUnreadReaction;
    if (type == "searchMessagesFilterUrl")
        return SearchMessagesFilterUrl;
    if (type == "searchMessagesFilterVideo")
        return SearchMessagesFilterVideo;
    if (type == "searchMessagesFilterVideoNote")
        return SearchMessagesFilterVideoNote;
    if (type == "searchMessagesFilterVoiceAndVideoNote")
        return SearchMessagesFilterVoiceAndVideoNote;
    if (type == "searchMessagesFilterVoiceNote")
        return SearchMessagesFilterVoiceNote;

    return SearchMessagesFilterEmpty;
}

void TDLibWrapper::getChatMessageCount(qlonglong chatId, SearchMessagesFilter filter, bool returnLocal) {
    const QString filterType = getSearchMessagesFilterType(filter);
    LOG("Receiving chat message count" << chatId << filterType);
    this->sendRequest(QVariantMap{
                          {_TYPE, "getChatMessageCount"},
                          {CHAT_ID, chatId},
                          {FILTER, QVariantMap{{_TYPE, filterType}}},
                          {RETURN_LOCAL, returnLocal},
                          {_EXTRA, filterType+(returnLocal?"!":"")+":"+QString::number(chatId)}
                      });
}

void TDLibWrapper::handleCountReceived(int count, const QString &extra) {
    QRegularExpressionMatch match = RE_EXTRA_CHAT_MESSAGE_COUNT.match(extra);
    if (match.hasMatch()) {
        const QString filterType = match.captured(1);
        const bool local = !match.captured(2).isEmpty();
        const qlonglong chatId = match.captured(3).toLongLong();
        LOG("Received chat message count" << chatId << filterType << local << count);

        emit chatMessageCountReceived(count, chatId, getSearchMessagesFilterForType(filterType), local);
    } else {
        LOG("Unknown count received" << count << extra);
        emit countReceived(count, extra);
    }
}

void TDLibWrapper::getForumTopics(qlonglong chatId, qint32 offsetDate, qlonglong offsetMessageId, int offsetForumTopicId, const QString &query, int limit) {
    LOG("Retreiving forum topics" << chatId);
    this->sendRequest(QVariantMap{
                          {_TYPE, "getForumTopics"},
                          {CHAT_ID, chatId},
                          {QUERY, query},
                          {"offset_date", offsetDate},
                          {"offset_message_id", offsetMessageId},
                          {"offset_forum_topic_id", offsetForumTopicId},
                          {LIMIT, limit},
                          {_EXTRA, chatId}
                      });
}

void TDLibWrapper::hideSuggestedAction(const QVariantMap &action) {
    LOG("Removing suggested action" << action.value(_TYPE).toString());
    this->sendRequest(QVariantMap{{_TYPE, "hideSuggestedAction"}, {ACTION, action}});
}

void TDLibWrapper::hideSuggestedAction(const QString &type) {
    this->hideSuggestedAction(QVariantMap{{_TYPE, type}});
}

void TDLibWrapper::setBirthdate(int day, int month, int year) {
    LOG("Setting birthdate" << day << month << year);
    this->sendRequest(QVariantMap{
                          {_TYPE, TYPE_SET_BIRTHDATE},
                          {BIRTHDATE, QVariantMap{{_TYPE, BIRTHDATE}, {"day", day}, {"month", month}, {"year", year}}}
                      });
}

void TDLibWrapper::setBirthdate() {
    LOG("Removing birthdate");
    this->sendRequest(QVariantMap{{_TYPE, TYPE_SET_BIRTHDATE}});
}

void TDLibWrapper::handleChatPendingJoinRequestsUpdated(qlonglong chatId, const QVariantMap &pendingJoinRequests) {
    LOG("Chat pending join requests updated" << chatId);
    this->getChatDataForce(chatId)->chatData.insert(PENDING_JOIN_REQUESTS, pendingJoinRequests);
    emit chatPendingJoinRequestsUpdated(chatId);
}

void TDLibWrapper::getChatJoinRequests(qlonglong chatId, const QVariantMap &offsetRequest, const QString &query, int limit) {
    LOG("Getting chat join requests for" << chatId);
    this->sendRequest({
                          {_TYPE, "getChatJoinRequests"},
                          {CHAT_ID, chatId},
                          {_EXTRA, chatId},
                          {"offset_request", offsetRequest},
                          {LIMIT, limit},
                          {QUERY, query}
                      });
}

void TDLibWrapper::processChatJoinRequest(qlonglong chatId, qlonglong userId, bool approve) {
    LOG("Processing chat join request" << chatId << userId << approve);
    this->sendRequest({
                          {_TYPE, "processChatJoinRequest"},
                          {CHAT_ID, chatId},
                          {USER_ID, userId},
                          {APPROVE, approve}
                      });
}

void TDLibWrapper::processChatJoinRequests(qlonglong chatId, bool approve, const QString &inviteLink) {
    LOG("Processing chat join requests" << chatId << approve << "with link:" << !inviteLink.isEmpty());
    this->sendRequest({
                          {_TYPE, "processChatJoinRequests"},
                          {CHAT_ID, chatId},
                          {APPROVE, approve},
                          {INVITE_LINK, inviteLink}
                      });
}

QString TDLibWrapper::connectionStateText() {
    switch (connectionState) {
    case WaitingForNetwork:
        return tr("Waiting for network...");
    case Connecting:
        return tr("Connecting to network...");
    case ConnectingToProxy:
        return tr("Connecting to proxy...");
    case Updating:
        return tr("Updating content...");
    default:
        return QString();
    }
}

void TDLibWrapper::getInternalLinkType(const QString &link) {
    LOG("Getting internal link type for" << link);
    this->sendRequest({
                          {_TYPE, "getInternalLinkType"},
                          {LINK, link},
                          {_EXTRA, "getInternalLinkType:"+link} // only for errors
                      });
}

void TDLibWrapper::handleInternalLinkTypeReceived(const QVariantMap &linkType) {
    const QString type = linkType.value(_TYPE).toString();
    LOG("Internal link type received" << type);

    if (type == "internalLinkTypePublicChat")
        // TODO: handle draft_text and open_profile
        this->searchPublicChat(linkType.value("chat_username").toString(), true);
    else if (type == "internalLinkTypeUserPhoneNumber")
        // TODO: handle draft_text and open_profile
        this->searchUserByPhoneNumber(linkType.value(PHONE_NUMBER).toString(), true);
    else if (type == "internalLinkTypeMessage")
        // TODO: handle topic, media timestamp, for album, etc.
        this->getMessageLinkInfo(linkType.value(URL).toString());
    else if (type == "internalLinkTypeChatInvite")
        this->checkChatInviteLink(linkType.value(INVITE_LINK).toString());
    else if (type == "internalLinkTypeUnknownDeepLink")
        this->getDeepLinkInfo(linkType.value(LINK).toString());
    else if (type == "internalLinkTypeProxy") {
        const QVariantMap &proxy = linkType.value(PROXY).toMap();
        emit internalLinkTypeProxyReceived(proxy.value(SERVER).toString(), proxy.value(PORT).toInt(), proxy.value(TYPE).toMap());
    } else
        emit linkUnsupportedByApp(type.mid(16));
}

void TDLibWrapper::handleUserReceived(const QVariantMap &user, bool doOpenOnFound) {
    const QString id = user.value(ID).toString();
    LOG("User received" << id << doOpenOnFound);

    if (doOpenOnFound) {
        LOG("Opening private chat for user" << id);
        this->createPrivateChat(id, EXTRA_OPEN_DIRECTLY);
    } else
        emit userReceived(user);
}

void TDLibWrapper::checkChatInviteLink(const QString &link) {
    LOG("Checking chat invite link info" << link);
    this->sendRequest({{_TYPE, "checkChatInviteLink"}, {INVITE_LINK, link}, {_EXTRA, link}});
}

bool TDLibWrapper::canSkipChatJoinDialog(qlonglong chatId) {
    const QVariantMap chat = getChat(chatId);
    if (chat.isEmpty())
        return false;

    const QVariantMap chatType = chat.value(TYPE).toMap();
    if (chatTypeFromString(chatType.value(_TYPE).toString()) == ChatTypeSupergroup) {
        const Group *supergroup = superGroups.value(chatType.value(SUPERGROUP_ID).toLongLong());

        return supergroup && (supergroup->chatMemberStatus() != ChatMemberStatusLeft || supergroup->isPublic());
    }

    return true;
}

void TDLibWrapper::clickChatSponsoredMessage(qlonglong chatId, qlonglong messageId, bool isMediaClick, bool fromFullscreen) {
    LOG("Clicking chat sponsored message" << chatId << messageId << isMediaClick << fromFullscreen);
    this->sendRequest({
                          {_TYPE, "clickChatSponsoredMessage"},
                          {CHAT_ID, chatId},
                          {MESSAGE_ID, messageId},
                          {"is_media_click", isMediaClick},
                          {"from_fullscreen", fromFullscreen}
                      });
}

void TDLibWrapper::toggleChatViewAsTopics(qlonglong chatId, bool viewAsTopics) {
    LOG("Setting chat view as topics value" << chatId << viewAsTopics);
    this->sendRequest({{_TYPE, "toggleChatViewAsTopics"}, {CHAT_ID, chatId}, {VIEW_AS_TOPICS, viewAsTopics}});
}

void TDLibWrapper::handleChatViewAsTopicsUpdated(qlonglong chatId, bool viewAsTopics) {
    this->getChatDataForce(chatId)->chatData.insert(VIEW_AS_TOPICS, viewAsTopics);
    emit chatViewAsTopicsUpdated(chatId);
}

void TDLibWrapper::getMessageThreadHistory(qlonglong chatId, qlonglong messageId, int extra, qlonglong fromMessageId, int offset, int limit) {
    LOG("Getting message thread history" << chatId << messageId << fromMessageId << offset << limit);
    this->sendRequest({
                          {_TYPE, "getMessageThreadHistory"},
                          {CHAT_ID, chatId},
                          {MESSAGE_ID, messageId},
                          {FROM_MESSAGE_ID, fromMessageId},
                          {OFFSET, offset},
                          {LIMIT, limit},
                          {_EXTRA, "thread:"+QString::number(chatId)+":"+QString::number(messageId)+":"+QString::number(extra)}
                      });
}

void TDLibWrapper::getForumTopicHistory(qlonglong chatId, int forumTopicId, int extra, qlonglong fromMessageId, int offset, int limit) {
    LOG("Getting forum topic history" << chatId << forumTopicId << fromMessageId << offset << limit);
    this->sendRequest({
                          {_TYPE, "getForumTopicHistory"},
                          {CHAT_ID, chatId},
                          {FORUM_TOPIC_ID, forumTopicId},
                          {FROM_MESSAGE_ID, fromMessageId},
                          {OFFSET, offset},
                          {LIMIT, limit},
                          {_EXTRA, "forumTopic:"+QString::number(chatId)+":"+QString::number(forumTopicId)+":"+QString::number(extra)}
                      });
}

QString TDLibWrapper::getMessageSourceType(MessageSource source) {
    switch (source) {
    case MessageSourceChatEventLog:
        return "messageSourceChatEventLog";
    case MessageSourceChatHistory:
        return "messageSourceChatHistory";
    case MessageSourceChatList:
        return "messageSourceChatList";
    case MessageSourceDirectMessagesChatTopicHistory:
        return "messageSourceDirectMessagesChatTopicHistory";
    case MessageSourceForumTopicHistory:
        return "messageSourceForumTopicHistory";
    case MessageSourceHistoryPreview:
        return "messageSourceHistoryPreview";
    case MessageSourceMessageThreadHistory:
        return "messageSourceMessageThreadHistory";
    case MessageSourceNotification:
        return "messageSourceNotification";
    case MessageSourceOther:
        return "messageSourceOther";
    case MessageSourceScreenshot:
        return "messageSourceScreenshot";
    case MessageSourceSearch:
        return "messageSourceSearch";
    default:
        return QString();
    }
}

void TDLibWrapper::getForumTopic(qlonglong chatId, int forumTopicId) {
    LOG("Getting forum topic" << chatId << forumTopicId);
    this->sendRequest({
                          {_TYPE, TYPE_GET_FORUM_TOPIC},
                          {CHAT_ID, chatId},
                          {FORUM_TOPIC_ID, forumTopicId},
                          {_EXTRA, "getForumTopic:"+QString::number(chatId)+":"+QString::number(forumTopicId)}
                      });
}

void TDLibWrapper::handleStickersReceived(const QVariantList &stickers, const QString &extra) {
    if (extra == "recent") {
        LOG("Recent stickers received" << stickers.length());
        emit recentStickersReceived(stickers);
    } else if (extra == "favorite") {
        LOG("Favorite stickers received" << stickers.length());
        emit favoriteStickersReceived(stickers);
    } else {
        LOG("Unknown stickers received" << extra << stickers.length());
        emit stickersReceived(stickers);
    }
}

void TDLibWrapper::addFavoriteSticker(int fileId) {
    LOG("Adding sticker to favorites" << fileId);
    this->sendRequest({{_TYPE, "addFavoriteSticker"}, {STICKER, QVariantMap{{_TYPE, TYPE_INPUT_FILE_ID}, {ID, fileId}}}});
}

void TDLibWrapper::removeFavoriteSticker(int fileId) {
    LOG("Removing sticker to favorites" << fileId);
    this->sendRequest({{_TYPE, "removeFavoriteSticker"}, {STICKER, QVariantMap{{_TYPE, TYPE_INPUT_FILE_ID}, {ID, fileId}}}});
}

void TDLibWrapper::getChatSimilarChats(qlonglong chatId) {
    LOG("Getting similar chats" << chatId);
    this->sendRequest({{_TYPE, "getChatSimilarChats"}, {CHAT_ID, chatId}, {_EXTRA, "getChatSimilarChats:"+QString::number(chatId)}});
}

void TDLibWrapper::getBotSimilarBots(qlonglong botUserId) {
    LOG("Getting similar bots" << botUserId);
    this->sendRequest({{_TYPE, "getBotSimilarBots"}, {BOT_USER_ID, botUserId}, {_EXTRA, "getBotSimilarBots:"+QString::number(botUserId)}});
}

QVariantMap TDLibWrapper::getProxyObject(const QString &server, int port, const QVariantMap &type) {
    return {
        {_TYPE, PROXY},
        {SERVER, server},
        {PORT, port},
        {TYPE, type}
    };
}

void TDLibWrapper::addProxy(const QVariantMap &proxy, const QString &extra, bool enable) {
    LOG("Adding proxy");
    sendRequest({{_TYPE, "addProxy"}, {PROXY, proxy}, {ENABLE, enable}, {_EXTRA, extra}});
}

void TDLibWrapper::editProxy(int proxyId, const QString &server, int port, const QVariantMap &type, bool enable) {
    LOG("Editing proxy" << proxyId);
    sendRequest({
                          {_TYPE, "editProxy"},
                          {PROXY_ID, proxyId},
                          {PROXY, getProxyObject(server, port, type)},
                          {ENABLE, enable}
                      });
}

void TDLibWrapper::enableProxy(int proxyId) {
    LOG("Enabling proxy" << proxyId);
    sendRequest({{_TYPE, "enableProxy"}, {PROXY_ID, proxyId}, {_EXTRA, "enableProxy:" + QString::number(proxyId)}});
}

void TDLibWrapper::disableProxy() {
    LOG("Disabling proxy");
    sendRequest({{_TYPE, TYPE_DISABLE_PROXY}, {_EXTRA, TYPE_DISABLE_PROXY}});
}

void TDLibWrapper::removeProxy(int proxyId) {
    LOG("Removing proxy" << proxyId);
    sendRequest({{_TYPE, "removeProxy"}, {PROXY_ID, proxyId}, {_EXTRA, "removeProxy:" + QString::number(proxyId)}});
}

void TDLibWrapper::getProxies() {
    LOG("Getting proxies");
    sendRequest({{_TYPE, "getProxies"}});
}

void TDLibWrapper::pingProxy() {
    LOG("Pinging telegram server");
    sendRequest({{_TYPE, "pingProxy"}, {_EXTRA, "ping"}});
}

void TDLibWrapper::pingProxy(const QVariantMap &proxy) {
    LOG("Pinging proxy");
    sendRequest({{_TYPE, "pingProxy"}, {PROXY, proxy}, {_EXTRA, proxy}});
}

void TDLibWrapper::getInternalLink(const QVariantMap &type, bool isHttp) {
    LOG("Getting internal link HTTP:" << isHttp);
    sendRequest({{_TYPE, "getInternalLink"}, {TYPE, type}, {"is_http", isHttp}});
}
