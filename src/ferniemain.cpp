/*
    Copyright (C) 2020 roundedrectangle, Sebastian J. Wolf and other contributors

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

#include "ferniemain.h"

#include "tdlib/tdlibfile.h"
#include "tdlib/tdlibresponse.h"
#include "chatpermissionfiltermodel.h"
#include "chatlistmodel.h"
#include "chat/chatmanager.h"
#include "dbusadaptor.h"
#include "textfiltermodel.h"
#include "boolfiltermodel.h"
#include "tgsplugin.h"
#include "lottieitem.h"
#include "chat/forumtopicmessagesmodel.h"
#include "chat/mediamessagesmodel.h"
#include "chat/invertedmediamessagesmodel.h"
#include "userprofilepicturesmodel.h"
#include "chat/chatphotosmodel.h"
#include "tdlib/tdlibstate.h"

#include <QGuiApplication>
#include <QLoggingCategory>

#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

Q_IMPORT_PLUGIN(TgsIOPlugin)

FernieMain::AppContext::AppContext(const char *uri, QSharedPointer<QQuickView> view,
                                   TDLibWrapper *tdLibWrapper, DBusAdaptor *dbusAdaptor, Settings *settings, Utilities *utilities,
                                   const QString &appName, const QUrl &appIconPath, const QString &dbusPath,
                                   const QString &dbusServiceName, const QString &dbusInterface) :
    uri(uri),
    settings(settings),
    tdLibWrapper(tdLibWrapper),
    dbusAdaptor(dbusAdaptor),
    waveformManager(view.data()),
    chatFoldersModel(tdLibWrapper, settings, utilities, view.data()),
    notificationManager(tdLibWrapper, settings, utilities, appName, appIconPath, dbusPath, dbusServiceName, dbusInterface),
    stickerManager(tdLibWrapper),
    knownUsersModel(tdLibWrapper, view.data()),
    knownUsersProxyModel(view.data()),
    contactsModel(tdLibWrapper, view.data()),
    suggestedActionsManager(tdLibWrapper, view.data())
{}

FernieMain::AppContext* FernieMain::registerTypes(int argc, char *argv[], QSharedPointer<QQuickView> view,
                                                  const QString &appName = QGuiApplication::applicationName(), const QUrl &appIconPath,
                                                  const QString &dbusPath, const QString &dbusServiceName, const QString &dbusInterface) {
    QQmlContext *context = view->rootContext();

    const char *uri = "App.Logic";
    qmlRegisterType<TDLibFile>(uri, 1, 0, "TDLibFile");
    qmlRegisterType<TextFilterModel>(uri, 1, 0, "TextFilterModel");
    qmlRegisterType<BoolFilterModel>(uri, 1, 0, "BoolFilterModel");
    qmlRegisterType<ChatPermissionFilterModel>(uri, 1, 0, "ChatPermissionFilterModel");
    qmlRegisterType<ChatManager>(uri, 1, 0, "ChatManager");
    qmlRegisterType<LottieItem>(uri, 1, 0, "LottieItem");
    qmlRegisterType<ForumTopicMessagesModel>(uri, 1, 0, "ForumTopicMessagesModel");
    qmlRegisterType<MediaMessagesModel>(uri, 1, 0, "MediaMessagesModel");
    qmlRegisterType<InvertedMediaMessagesModel>(uri, 1, 0, "InvertedMediaMessagesModel");
    qmlRegisterType<UserProfilePicturesModel>(uri, 1, 0, "UserProfilePicturesModel");
    qmlRegisterType<ChatPhotosModel>(uri, 1, 0, "ChatPhotosModel");

    Settings *settings = new Settings(view.data());
    context->setContextProperty("fernieSettings", settings);
    qmlRegisterUncreatableType<Settings>(uri, 1, 0, "FernieSettings", QString());

    TDLibWrapper *tdLibWrapper = new TDLibWrapper(settings, view.data());
    context->setContextProperty("tdLibWrapper", tdLibWrapper);
    qmlRegisterUncreatableType<TDLibWrapper>(uri, 1, 0, "TDLibAPI", QString());

    qmlRegisterUncreatableType<TDLibResponse>(uri, 1, 0, "TDLibResponse", QString());

    Utilities *utilities = tdLibWrapper->getUtilities();
    context->setContextProperty("utilities", utilities);
    qmlRegisterUncreatableType<Utilities>(uri, 1, 0, "Utilities", QString());

    TDLibState *tdLibState = tdLibWrapper->getState();
    context->setContextProperty("tdLibState", tdLibState);
    qmlRegisterUncreatableType<TDLibState>(uri, 1, 0, "TDLibState", QString());

    DBusAdaptor *dbusAdaptor = new DBusAdaptor(tdLibWrapper, view.data());
    context->setContextProperty("dBusAdaptor", dbusAdaptor);

    AppContext *appContext = new AppContext(uri, view, tdLibWrapper, dbusAdaptor, settings, utilities,
                                            appName, appIconPath, dbusPath, dbusServiceName, dbusInterface);

    context->setContextProperty("chatFoldersModel", &appContext->chatFoldersModel);
    qmlRegisterUncreatableType<ChatFoldersModel>(uri, 1, 0, "ChatFoldersModel", QString());

    ChatListModel* chatListModel = appContext->chatFoldersModel.getMainChatListModel();
    context->setContextProperty("chatListModel", chatListModel);
    ChatListModel* archiveChatListModel = appContext->chatFoldersModel.getArchiveChatListModel();
    context->setContextProperty("archiveChatListModel", archiveChatListModel);

    context->setContextProperty("knownUsersModel", &appContext->knownUsersModel);
    appContext->knownUsersProxyModel.setSourceModel(&appContext->knownUsersModel);
    appContext->knownUsersProxyModel.setFilterRole(KnownUsersModel::RoleFilter);
    appContext->knownUsersProxyModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
    context->setContextProperty("knownUsersProxyModel", &appContext->knownUsersProxyModel);

    context->setContextProperty("waveformManager", &appContext->waveformManager);
    context->setContextProperty("notificationManager", &appContext->notificationManager);
    context->setContextProperty("stickerManager", &appContext->stickerManager);
    context->setContextProperty("contactsModel", &appContext->contactsModel);
    context->setContextProperty("suggestedActionsManager", &appContext->suggestedActionsManager);

    return appContext;
}

void FernieMain::registerDBusService(QSharedPointer<QGuiApplication> app, QSharedPointer<QQuickView> view, const QString &path, const QString &serviceName) {
    LOG("Initializing DBus connectivity");
    QDBusConnection sessionBusConnection = QDBusConnection::sessionBus();

    if (!sessionBusConnection.isConnected()) {
        WARN("Error connecting to DBus");
        return;
    }

    if (!sessionBusConnection.registerObject(path, view.data())) {
        WARN("Error registering DBus root object" << sessionBusConnection.lastError().message());
        return;
    }

    if (!sessionBusConnection.registerService(serviceName)) {
        WARN("Error registering DBus interface" << sessionBusConnection.lastError().message());
        return;
    }

    LOG("DBus service registered successfully");

    QObject::connect(app.data(), &QGuiApplication::aboutToQuit, [app, path, serviceName]() {
        LOG("Cleaning up DBus connectivity");
        QDBusConnection sessionBusConnection = QDBusConnection::sessionBus();

        if (!sessionBusConnection.isConnected()) {
            LOG("Error connecting to DBus");
            return;
        }

        if (!sessionBusConnection.unregisterService(serviceName)) {
            LOG("Couldn't unregister DBus interface" << sessionBusConnection.lastError().message());
            return;
        }

        sessionBusConnection.unregisterObject(path);

        LOG("DBus service unregistered successfully");
    });
}
