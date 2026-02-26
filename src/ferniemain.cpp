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

#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <QLoggingCategory>

// The default filter can be overridden by QT_LOGGING_RULES envinronment variable, e.g.
// QT_LOGGING_RULES="fernschreiber2.*=true" harbour-fernschreiber2
#if defined (QT_DEBUG) || defined(DEBUG)
#  define DEFAULT_LOG_FILTER "fernschreiber2.*=true"
#else
#  define DEFAULT_LOG_FILTER "fernschreiber2.*=false"
#endif

Q_IMPORT_PLUGIN(TgsIOPlugin)

FernieMain::AppContext::AppContext(const char *uri, QSharedPointer<QQuickView> view, TDLibWrapper *tdLibWrapper, AppSettings *appSettings, Utilities *utilities, MceInterface *mceInterface) :
    uri(uri),
    appSettings(appSettings),
    tdLibWrapper(tdLibWrapper),
    waveformManager(view.data()),
    chatFoldersModel(tdLibWrapper, appSettings, utilities, view.data()),
    notificationManager(tdLibWrapper, appSettings, mceInterface, utilities),
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

FernieMain::AppContext* FernieMain::registerTypes(int argc, char *argv[], QSharedPointer<QQuickView> view) {
    QQmlContext *context = view.data()->rootContext();

    const char *uri = "App.Logic";
    qmlRegisterType<TDLibFile>(uri, 1, 0, "TDLibFile");
    qmlRegisterType<TextFilterModel>(uri, 1, 0, "TextFilterModel");
    qmlRegisterType<BoolFilterModel>(uri, 1, 0, "BoolFilterModel");
    qmlRegisterType<InvertedProxyModel>(uri, 1, 0, "InvertedProxyModel");
    qmlRegisterType<ChatPermissionFilterModel>(uri, 1, 0, "ChatPermissionFilterModel");
    qmlRegisterType<ChatManager>(uri, 1, 0, "ChatManager");
    qmlRegisterType<LottieItem>(uri, 1, 0, "LottieItem");
    qmlRegisterType<ForumTopicMessagesModel>(uri, 1, 0, "ForumTopicMessagesModel");
    qmlRegisterType<MediaMessagesModel>(uri, 1, 0, "MediaMessagesModel");
    qmlRegisterType<InvertedMediaMessagesModel>(uri, 1, 0, "InvertedMediaMessagesModel");
    qmlRegisterSingletonType<DebugLogJS>(uri, 1, 0, "DebugLog", DebugLogJS::createSingleton);

    AppSettings *appSettings = new AppSettings(view.data());
    context->setContextProperty("appSettings", appSettings);
    qmlRegisterUncreatableType<AppSettings>(uri, 1, 0, "AppSettings", QString());

    MceInterface *mceInterface = new MceInterface(view.data());
    TDLibWrapper *tdLibWrapper = new TDLibWrapper(appSettings, mceInterface, view.data());
    context->setContextProperty("tdLibWrapper", tdLibWrapper);
    qmlRegisterUncreatableType<TDLibWrapper>(uri, 1, 0, "TDLibAPI", QString());

    qmlRegisterUncreatableType<TDLibResponse>(uri, 1, 0, "TDLibResponse", QString());

    Utilities *utilities = tdLibWrapper->getUtilities();
    context->setContextProperty("utilities", utilities);
    qmlRegisterUncreatableType<Utilities>(uri, 1, 0, "Utilities", QString());

    DBusAdaptor *dBusAdaptor = tdLibWrapper->getDBusAdaptor();
    context->setContextProperty("dBusAdaptor", dBusAdaptor);

    AppContext *appContext = new AppContext(uri, view, tdLibWrapper, appSettings, utilities, mceInterface);

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
