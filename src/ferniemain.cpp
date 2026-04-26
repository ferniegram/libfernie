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

#include "debuglogjs.h"
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

#include <QGuiApplication>
#include <QLoggingCategory>

#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

// The default filter can be overridden by QT_LOGGING_RULES envinronment variable, e.g.
// QT_LOGGING_RULES="ferniegram.*=true" harbour-ferniegram
#if defined (QT_DEBUG) || defined(DEBUG)
#  define DEFAULT_LOG_FILTER "ferniegram.*=true"
#else
#  define DEFAULT_LOG_FILTER "ferniegram.*=false"
#endif

Q_IMPORT_PLUGIN(TgsIOPlugin)

FernieMain::AppContext::AppContext(const char *uri, QSharedPointer<QQuickView> view, TDLibWrapper *tdLibWrapper, DBusAdaptor *dbusAdaptor, Settings *settings, Utilities *utilities, MceInterface *mceInterface, const QString &appName, const QString &dbusPath, const QString &dbusServiceName, const QString &dbusInterface) :
    uri(uri),
    settings(settings),
    tdLibWrapper(tdLibWrapper),
    dbusAdaptor(dbusAdaptor),
    waveformManager(view.data()),
    chatFoldersModel(tdLibWrapper, settings, utilities, view.data()),
    notificationManager(tdLibWrapper, settings, mceInterface, utilities, appName, dbusPath, dbusServiceName, dbusInterface),
    processLauncher(),
    stickerManager(tdLibWrapper),
    knownUsersModel(tdLibWrapper, view.data()),
    knownUsersProxyModel(view.data()),
    contactsModel(tdLibWrapper, view.data()),
    suggestedActionsManager(tdLibWrapper, view.data())
{}

void FernieMain::setupLogging() {
    QLoggingCategory::setFilterRules(DEFAULT_LOG_FILTER);
}

FernieMain::AppContext* FernieMain::registerTypes(int argc, char *argv[], QSharedPointer<QQuickView> view, const QString &appName = QGuiApplication::applicationName(), const QString &dbusPath, const QString &dbusServiceName, const QString &dbusInterface) {
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
    qmlRegisterSingletonType<DebugLogJS>(uri, 1, 0, "DebugLog", DebugLogJS::createSingleton);

    Settings *settings = new Settings(view.data());
    context->setContextProperty("fernieSettings", settings);
    qmlRegisterUncreatableType<Settings>(uri, 1, 0, "FernieSettings", QString());

    MceInterface *mceInterface = new MceInterface(view.data());
    TDLibWrapper *tdLibWrapper = new TDLibWrapper(settings, mceInterface, view.data());
    context->setContextProperty("tdLibWrapper", tdLibWrapper);
    qmlRegisterUncreatableType<TDLibWrapper>(uri, 1, 0, "TDLibAPI", QString());

    qmlRegisterUncreatableType<TDLibResponse>(uri, 1, 0, "TDLibResponse", QString());

    Utilities *utilities = tdLibWrapper->getUtilities();
    context->setContextProperty("utilities", utilities);
    qmlRegisterUncreatableType<Utilities>(uri, 1, 0, "Utilities", QString());

    DBusAdaptor *dbusAdaptor = new DBusAdaptor(tdLibWrapper, view.data());
    view->rootContext()->setContextProperty("dBusAdaptor", dbusAdaptor);

    AppContext *appContext = new AppContext(uri, view, tdLibWrapper, dbusAdaptor, settings, utilities, mceInterface, appName, dbusPath, dbusServiceName, dbusInterface);

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
    context->setContextProperty("processLauncher", &appContext->processLauncher);
    context->setContextProperty("stickerManager", &appContext->stickerManager);
    context->setContextProperty("contactsModel", &appContext->contactsModel);
    context->setContextProperty("suggestedActionsManager", &appContext->suggestedActionsManager);

    return appContext;
}

void FernieMain::registerDBusService(QSharedPointer<QQuickView> view, const QString &path, const QString &serviceName) {
    LOG("Initializing DBus connectivity");
    QDBusConnection sessionBusConnection = QDBusConnection::sessionBus();

    if (!sessionBusConnection.isConnected()) {
        WARN("Error connecting to D-BUS");
        return;
    }

    if (!sessionBusConnection.registerObject(path, view.data())) {
        WARN("Error registering root object to D-BUS" << sessionBusConnection.lastError().message());
        return;
    }

    if (!sessionBusConnection.registerService(serviceName)) {
        WARN("Error registering interface to D-BUS" << sessionBusConnection.lastError().message());
        return;
    }

    LOG("DBus service registered successfully");
}
