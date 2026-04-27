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

#include "notificationmanager.h"
#include "chatdata.h"
#include "tdlib/tdlibfile.h"
#include <QListIterator>
#include <QDateTime>
#include <QDBusConnection>
#include <QGuiApplication>
#include <QImage>
#include <QPainter>

#define DEBUG_MODULE NotificationManager
#include "debuglog.h"

namespace {
    const QString _TYPE("@type");
    const QString TYPE("type");
    const QString ID("id");
    const QString CHAT_ID("chat_id");
    const QString IS_CHANNEL("is_channel");
    const QString TOTAL_COUNT("total_count");
    const QString DATE("date");
    const QString TITLE("title");
    const QString CONTENT("content");
    const QString MESSAGE("message");
    const QString FIRST_NAME("first_name");
    const QString LAST_NAME("last_name");
    const QString SENDER_ID("sender_id");
    const QString USER_ID("user_id");
    const QString NOTIFICATIONS("notifications");
    const QString NOTIFICATION_GROUP_ID("notification_group_id");
    const QString ADDED_NOTIFICATIONS("added_notifications");
    const QString REMOVED_NOTIFICATION_IDS("removed_notification_ids");
    const QString NOTIFICATION("notification");

    const QString CHAT_TYPE_BASIC_GROUP("chatTypeBasicGroup");
    const QString CHAT_TYPE_SUPERGROUP("chatTypeSupergroup");

    // Notification hints
    const QString HINT_GROUP_TYPE("x-libfernie.group_type");
    const QString HINT_GROUP_ID("x-libfernie.group_id");        // int
    const QString HINT_CHAT_ID("x-libfernie.chat_id");          // qlonglong
    const QString HINT_TOTAL_COUNT("x-libfernie.total_count");  // int

    const QString HINT_VIBRA("x-nemo-vibrate");                     // bool
    const QString HINT_SUPPRESS_SOUND("suppress-sound");            // bool
    const QString HINT_DISPLAY_ON("x-nemo-display-on");             // bool
    const QString HINT_VISIBILITY("x-nemo-visibility");             // QString
    const QString VISIBILITY_PUBLIC("public");
}

NotificationManager::NotificationGroup::NotificationGroup(NotificationGroupType type, int group, qlonglong chat, int count, Notification *notification) :
    type(type),
    notificationGroupId(group),
    chatId(chat),
    totalCount(count),
    nemoNotification(notification)
{}

NotificationManager::NotificationGroup::~NotificationGroup() {
    delete nemoNotification;
}

QVariantMap NotificationManager::NotificationGroup::lastNotification() const {
    if (notificationOrder.isEmpty())
        return QVariantMap();

    return activeNotifications.value(notificationOrder.last());
}

NotificationManager::NotificationManager(TDLibWrapper *tdLibWrapper, Settings *settings, MceInterface *mceInterface, Utilities *utilities,
                                         const QString &appName = QGuiApplication::applicationName(), const QUrl &appIconPath,
                                         const QString &dbusPath, const QString &dbusServiceName, const QString &dbusInterface) :
    tdLibWrapper(tdLibWrapper),
    settings(settings),
    mceInterface(mceInterface),
    utilities(utilities),
    appName(appName),
    dbusPath(dbusPath),
    dbusServiceName(dbusServiceName),
    dbusInterface(dbusInterface),
    appIconFile(appIconPath.toLocalFile()),
    activeChatId(0)
{
    LOG("Initializing...");

    connect(this->tdLibWrapper, &TDLibWrapper::activeNotificationsUpdated, this, &NotificationManager::handleUpdateActiveNotifications);
    connect(this->tdLibWrapper, &TDLibWrapper::notificationGroupUpdated, this, &NotificationManager::handleUpdateNotificationGroup);
    connect(this->tdLibWrapper, &TDLibWrapper::notificationUpdated, this, &NotificationManager::handleUpdateNotification);
    connect(this->tdLibWrapper, &TDLibWrapper::chatRolesUpdated, this, &NotificationManager::handleChatRolesUpdated);

    this->controlLedNotification(false);

    // Restore notifications
    QList<QObject*> notifications = Notification::notifications();
    const int n = notifications.count();
    LOG("Found" << n << "existing notifications");
    for (int i = 0; i < n; i++) {
        QObject *notificationObject = notifications.at(i);
        Notification *notification = qobject_cast<Notification *>(notificationObject);
        if (notification) {
            bool typeOk, groupOk, chatOk, countOk;
            const int type = notification->hintValue(HINT_GROUP_TYPE).toInt(&typeOk);
            const int groupId = notification->hintValue(HINT_GROUP_ID).toInt(&groupOk);
            const qlonglong chatId = notification->hintValue(HINT_CHAT_ID).toLongLong(&chatOk);
            const int totalCount = notification->hintValue(HINT_TOTAL_COUNT).toInt(&countOk);
            if (typeOk && groupOk && chatOk && countOk && !notificationGroups.contains(groupId)) {
                LOG("Restoring notification group" << groupId << "chatId" << chatId << "count" << totalCount);
                notificationGroups.insert(groupId, new NotificationGroup(NotificationGroupType(type), groupId, chatId, totalCount, notification));
                continue;
            }
        }
        delete notificationObject;
    }
}

NotificationManager::~NotificationManager() {
    LOG("Destroying");
    qDeleteAll(notificationGroups.values());
}

NotificationManager::NotificationGroupType NotificationManager::getGroupType(const QVariantMap &groupType) {
    const QString type = groupType.value(_TYPE).toString();

    if (type == "notificationGroupTypeMessages")
        return NotificationGroupTypeMessages;
    else if (type == "notificationGroupTypeMentions")
        return NotificationGroupTypeMentions;
    else if (type == "notificationGroupTypeSecretChat")
        return NotificationGroupTypeSecretChat;
    else if (type == "notificationGroupTypeCalls")
        return NotificationGroupTypeCalls;

    // Should never reach here
    return NotificationGroupTypeMessages;
}

void NotificationManager::setActiveChatId(qlonglong chatId) {
    LOG("Set active chat ID to" << chatId);
    this->activeChatId = chatId;
}

bool NotificationManager::acceptNotificationGroupType(const QVariantMap &groupType) {
    const QString type = groupType.value(_TYPE).toString();
    return type != "notificationGroupTypeCalls";
}

void NotificationManager::handleUpdateActiveNotifications(const QVariantList &notificationGroups) {
    LOG("Received active notifications" << notificationGroups.size());
    for (const QVariant &groupVariant : notificationGroups) {
        const QVariantMap group = groupVariant.toMap();

        updateNotificationGroup(group.value(TYPE).toMap(), group.value(ID).toInt(),
            group.value(CHAT_ID).toLongLong(),
            group.value(TOTAL_COUNT).toInt(),
            group.value(NOTIFICATIONS).toList());
    }
}

void NotificationManager::handleUpdateNotificationGroup(const QVariantMap &update) {
    const int groupId = update.value(NOTIFICATION_GROUP_ID).toInt();
    const int totalCount = update.value(TOTAL_COUNT).toInt();
    LOG("Received notification group update" << groupId << "total count" << totalCount);

    updateNotificationGroup(update.value(TYPE).toMap(), groupId, update.value(CHAT_ID).toLongLong(), totalCount,
        update.value(ADDED_NOTIFICATIONS).toList(),
        update.value(REMOVED_NOTIFICATION_IDS).toList(),
        settings->notificationFeedback());
}

void NotificationManager::updateNotificationGroup(const QVariantMap &type, int groupId, qlonglong chatId, int totalCount,
    const QVariantList &addedNotifications, const QVariantList &removedNotificationIds,
    Settings::NotificationFeedback feedback)
{
    bool needFeedback = false;
    NotificationGroup* notificationGroup = notificationGroups.value(groupId);

    LOG("Received notification group update, group ID:" << groupId << "total count" << totalCount);
    if (totalCount) {
        if (notificationGroup) {
            notificationGroup->type = getGroupType(type);
            notificationGroup->totalCount = totalCount;
        } else {
            Notification *notification = new Notification(this);
            notification->setCategory("x-nemo.messaging.im");
            notification->setAppName(this->appName);
            notification->setAppIcon(appIconFile);
            notification->setHintValue(HINT_GROUP_ID, groupId);
            notification->setHintValue(HINT_CHAT_ID, chatId);
            notification->setHintValue(HINT_TOTAL_COUNT, totalCount);
            notificationGroups.insert(groupId, notificationGroup =
                new NotificationGroup(getGroupType(type), groupId, chatId, totalCount, notification));
        }

        for (const QVariant &notificationVariant : addedNotifications) {
            const QVariantMap addedNotification = notificationVariant.toMap();
            const int addedId = addedNotification.value(ID).toInt();
            notificationGroup->activeNotifications.insert(addedId, addedNotification);
            notificationGroup->notificationOrder.append(addedId);
        }

        for (const QVariant &removedVariant : removedNotificationIds) {
            const int removedId = removedVariant.toInt();
            notificationGroup->activeNotifications.remove(removedId);
            notificationGroup->notificationOrder.removeOne(removedId);
        }

        switch (feedback) {
        case Settings::NotificationFeedbackNone:
            break;
        case Settings::NotificationFeedbackNew:
            needFeedback = !notificationGroup->nemoNotification->replacesId();
            break;
        case Settings::NotificationFeedbackAll:
            // don't alert the user just about removals
            needFeedback = !addedNotifications.isEmpty();
            break;
        }
        needFeedback = needFeedback && !notificationGroup->lastNotification().value("is_silent").toBool();

        // Publish new or update the existing notification
        LOG("Feedback" << needFeedback);
        publishNotification(notificationGroup, needFeedback);
    } else if (notificationGroup) {
        // No active notifications left in this group
        notificationGroup->nemoNotification->close();
        notificationGroups.remove(groupId);
        delete notificationGroup;
    }

    if (notificationGroups.isEmpty())
        // No notifications left
        controlLedNotification(false);
    else if (needFeedback)
        controlLedNotification(true);
}

void NotificationManager::handleUpdateNotification(int groupId, const QVariantMap &notification) {
    int notificationId = notification.value(ID).toInt();
    LOG("Received notification update group ID" << groupId << "notification ID" << notificationId);

    NotificationGroup *group = notificationGroups.value(groupId);
    if (group && group->activeNotifications.contains(notificationId)) {
        LOG("Updating notification" << notificationId << "group" << groupId);
        group->activeNotifications.insert(notificationId, notification);

        // Silently update notification
        publishNotification(group, false);
    }
}

void NotificationManager::handleChatRolesUpdated(qlonglong chatId, const QVector<int> changedRoles) {
    if (changedRoles.contains(ChatData::RoleTitle) || changedRoles.contains(ChatData::RolePhoto)) {
        LOG("Chat" << chatId << "title or photo changed");

        // Silently update notifications
        for (NotificationGroup *group : notificationGroups)
            if (group->chatId == chatId) {
                LOG("Updating notification for group ID" << group->notificationGroupId);
                publishNotification(group, false);
                break;
            }
    }
}

void NotificationManager::publishNotification(const NotificationGroup *notificationGroup, bool needFeedback) {
    const QVariantMap lastNotification = notificationGroup->lastNotification();
    const QVariantMap notificationType = lastNotification.value(TYPE).toMap();
    const ChatData *chat = tdLibWrapper->getChatData(notificationGroup->chatId);

    Notification *nemoNotification = notificationGroup->nemoNotification;

    nemoNotification->setSummary(utilities->getChatTitle(chat));
    nemoNotification->setTimestamp(QDateTime::fromMSecsSinceEpoch(lastNotification.value(DATE).toLongLong() * 1000));
    nemoNotification->setItemCount(notificationGroup->totalCount);

    const QVariantMap photoSmall = chat->photoSmall();
    if (!photoSmall.isEmpty()) {
        TDLibFile file(tdLibWrapper, photoSmall);

        if (file.isDownloadingCompleted()) {
            QImage image(file.getPath());

            QImage result(image.size(), QImage::Format_ARGB32_Premultiplied);
            result.fill(Qt::transparent);

            QPainter p(&result);
            p.setRenderHint(QPainter::Antialiasing, true);
            p.setRenderHint(QPainter::SmoothPixmapTransform, true);

            QPainterPath clipPath;
            clipPath.addEllipse(image.rect());

            p.setClipPath(clipPath);
            p.drawImage(0, 0, image);
            p.end();

            nemoNotification->setIconData(result);
        } else if (file.canBeDownloaded())
            file.load();
    }


    QVariantList remoteActionArguments{QString::number(notificationGroup->chatId), ""};
    QVariantList remoteActions;

    switch (notificationGroup->type) {
    case NotificationGroupTypeSecretChat:
        nemoNotification->setBody(tr("This secret chat was created", "Notification"));

        remoteActions.append(Notification::remoteAction(
                                 "", tr("Close", "Notification button for closing a newly created secret chat"),
                                 dbusServiceName, dbusPath, dbusInterface,
                                 "closeSecretChat", remoteActionArguments
                                 ));
        break;
    case NotificationGroupTypeMessages:
    case NotificationGroupTypeMentions:
    {
        const bool showPreview = !settings->notificationSuppressContent() && notificationType.value("show_preview").toBool() && chat && chat->chatType != TDLibWrapper::ChatTypeSecret;
        const QVariantMap message = notificationType.value(MESSAGE).toMap();

        if (showPreview) {
            QString body;

            if (chat->chatType == TDLibWrapper::ChatTypeBasicGroup || (chat->chatType == TDLibWrapper::ChatTypeSupergroup && !chat->isChannel()))
                // Add author
                body = utilities->formatMessageSender(message.value(SENDER_ID).toMap()) + ": ";

            body += utilities->getMessageText(message, Utilities::MessageTextSimple, true, false);
            nemoNotification->setBody(body);
        } else
            nemoNotification->setBody(tr("You have a new message", "Notification"));


        remoteActionArguments.removeLast();
        remoteActionArguments.append(message.value(ID).toString());

        remoteActions.append(Notification::remoteAction(
                                 "", tr("Mark as read", "Notification button"),
                                 dbusServiceName, dbusPath, dbusInterface,
                                 "markMessageAsRead", remoteActionArguments
                                 ));

        if (showPreview && !chat->isChannel()) {
            QVariantMap replyAction = Notification::remoteAction("", tr("Reply", "Reply to a message in a notification"),
                                                                dbusServiceName, dbusPath, dbusInterface,
                                                                "replyToMessage", remoteActionArguments).toMap();
            // See https://github.com/sailfishos/nemo-qml-plugin-notifications/blob/d4d0a0ce8257b90293b8df469830f0e288faeeae/src/notification.cpp#L213
            replyAction.insert(TYPE, "input");

            remoteActions.append(replyAction);
        }

        break;
    }
    case NotificationGroupTypeCalls:
        // Should never reach here
        return;
    }

    if (notificationGroup->type == NotificationGroupTypeMentions)
        nemoNotification->setSummary(tr("%1 (mentions)",
                                        "Title for a notification containing messages with mentions from a chat. Mention count is displayed separately",
                                        notificationGroup->totalCount
                                        ).arg(nemoNotification->summary()));

    remoteActions.append(Notification::remoteAction(
                             "default", "",
                             dbusServiceName, dbusPath, dbusInterface,
                             "openMessage", remoteActionArguments
                             ));
    nemoNotification->setRemoteActions(remoteActions);

    nemoNotification->setHintValue(HINT_VIBRA, needFeedback);

    // Don't show popup for currently open chat
    if (!needFeedback || (activeChatId == notificationGroup->chatId && QGuiApplication::applicationState() == Qt::ApplicationActive)) {
        nemoNotification->setHintValue(HINT_SUPPRESS_SOUND, true);
        nemoNotification->setHintValue(HINT_DISPLAY_ON, false);
        nemoNotification->setHintValue(HINT_VISIBILITY, QString());
        nemoNotification->setUrgency(Notification::Low);
    } else {
        nemoNotification->setHintValue(HINT_SUPPRESS_SOUND, !settings->notificationSoundsEnabled());
        nemoNotification->setHintValue(HINT_DISPLAY_ON, settings->notificationTurnsDisplayOn());
        nemoNotification->setHintValue(HINT_VISIBILITY, VISIBILITY_PUBLIC);
        nemoNotification->setUrgency(Notification::Normal);
    }

    nemoNotification->publish();
}

void NotificationManager::controlLedNotification(bool enabled) {
    static const QString PATTERN("PatternCommunicationIM");
    mceInterface->setLedPattern(PATTERN, enabled);
}
