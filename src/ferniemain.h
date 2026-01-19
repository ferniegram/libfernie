#include <QSharedPointer>
#include <QQuickView>
#include <QQmlContext>

#include "appsettings.h"
#include "debuglog.h"
#include "debuglogjs.h"
#include "tdlib/tdlibfile.h"
#include "tdlib/tdlibwrapper.h"
#include "tdlib/tdlibresponse.h"
#include "chatpermissionfiltermodel.h"
#include "chatlistmodel.h"
#include "chat/chatmanager.h"
#include "notificationmanager.h"
#include "mceinterface.h"
#include "dbusadaptor.h"
#include "processlauncher.h"
#include "stickermanager.h"
#include "textfiltermodel.h"
#include "boolfiltermodel.h"
#include "tgsplugin.h"
#include "utilities.h"
#include "knownusersmodel.h"
#include "contactsmodel.h"
#include "chatfoldersmodel.h"
#include "invertedproxymodel.h"
#include "waveformmanager.h"
#include "suggestedactionsmanager.h"
#include "lottieitem.h"
#include "chat/forumtopicmessagesmodel.h"

namespace FernieMain {
    void setupLogging();

    struct AppContext {
        const char *uri;
        AppSettings *appSettings;
        TDLibWrapper *tdLibWrapper;
        WaveformManager waveformManager;
        ChatFoldersModel chatFoldersModel;
        NotificationManager notificationManager;
        ProcessLauncher processLauncher;
        StickerManager stickerManager;
        KnownUsersModel knownUsersModel;
        QSortFilterProxyModel knownUsersProxyModel;
        ContactsModel contactsModel;
        SuggestedActionsManager suggestedActionsManager;

        AppContext(const char *uri, QSharedPointer<QQuickView> view, TDLibWrapper *tdLibWrapper, AppSettings *appSettings, Utilities *utilities, MceInterface *mceInterface);
    };
    AppContext* registerTypes(int argc, char *argv[], QSharedPointer<QQuickView> view);
}