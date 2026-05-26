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

#pragma once

#include <QObject>
#include <nemonotifications-qt5/notification.h>
#include "tdlib/tdlibwrapper.h"
#include "tdlib/tdlibfile.h"
#include "settings.h"
#include "mceinterface.h"
#include "utilities.h"

class NotificationManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(qlonglong activeChatId MEMBER activeChatId WRITE setActiveChatId)

public:
    NotificationManager(TDLibWrapper *tdLibWrapper, Settings *settings, Utilities *utilities,
                        const QString &appName, const QUrl &appIconPath = QUrl(),
                        const QString &dbusPath = QString(), const QString &dbusServiceName = QString(), const QString &dbusInterface = "io.libfernie.default");
    ~NotificationManager() override;

    void setActiveChatId(qlonglong chatId);

private slots:
    void handleUpdateActiveNotifications(const QVariantList &notificationGroups);
    void handleUpdateNotificationGroup(const QVariantMap &update);
    void handleUpdateNotification(int groupId, const QVariantMap &notification);
    void handleChatRolesUpdated(qlonglong chatId, const QVector<int> changedRoles);
    void handleChatPhotoDownloadingCompletedChanged();
    void updateAllNotifications();
    void handleDefaultReactionTypeChanged();
    void updateNotificationForChat(qlonglong chatId, TDLibFile *chatPhotoFile = nullptr);

private:
    enum NotificationGroupType {
        NotificationGroupTypeMessages,
        NotificationGroupTypeMentions,
        NotificationGroupTypeSecretChat,
        NotificationGroupTypeCalls
    };
    static NotificationGroupType getGroupType(const QVariantMap &groupType);

    struct NotificationGroup {
        NotificationGroup(NotificationGroupType type, int groupId, qlonglong chatId, int count, Notification *notification);
        NotificationGroup(Notification *notification);
        ~NotificationGroup();

        QVariantMap lastNotification() const;

        NotificationGroupType type;
        int notificationGroupId;
        qlonglong chatId;
        int totalCount;
        Notification *nemoNotification;
        QMap<int, QVariantMap> activeNotifications;
        QList<int> notificationOrder;
    };

    void publishNotification(const NotificationGroup *notificationGroup, bool needFeedback, bool suppressSound = false, const QString &soundFilePath = QString(), TDLibFile *chatPhotoFile = nullptr);
    void controlLedNotification(bool enabled);
    void updateNotificationGroup(const QVariantMap &type, int groupId, qlonglong chatId, int totalCount,
        const QVariantList &addedNotifications, const QVariantList &removedNotificationIds = QVariantList(),
        Settings::NotificationFeedback feedback = Settings::NotificationFeedbackNone,
        qlonglong notificationSoundId = 0);

private:
    TDLibWrapper *tdLibWrapper;
    Settings *settings;
    MceInterface *mceInterface;
    Utilities *utilities;
    QString appName;
    QString dbusPath;
    QString dbusServiceName;
    QString dbusInterface;
    QMap<int, NotificationGroup*> notificationGroups;
    QString appIconFile;
    qlonglong activeChatId;
    QMap<int, qlonglong> pendingChatPhotoChats;
};
