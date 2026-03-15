#include <QSharedPointer>
#include <QQuickView>
#include <QQmlContext>

#include "appsettings.h"
#include "debuglog.h"
#include "tdlib/tdlibwrapper.h"
#include "notificationmanager.h"
#include "mceinterface.h"
#include "processlauncher.h"
#include "stickermanager.h"
#include "utilities.h"
#include "knownusersmodel.h"
#include "contactsmodel.h"
#include "chatfoldersmodel.h"
#include "waveformmanager.h"
#include "suggestedactionsmanager.h"

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
