#include "settings.h"

#include <QGuiApplication>

#define DEBUG_MODULE Settings
#include "debuglog.h"


#define SETTING_GETTER(GET, TYPE, KEY, GET_FUNCTION) TYPE Settings::GET() const { \
    return settings.value(KEY).GET_FUNCTION(); \
}
#define SETTING_GETTER2(GET, TYPE, KEY, GET_FUNCTION, DEFAULT) TYPE Settings::GET() const { \
    return settings.value(KEY, DEFAULT).GET_FUNCTION(); \
}

#define SETTING_SETTER_(SET, TYPE, GET, KEY, CHANGED_SIGNAL) void Settings::SET(TYPE value) { \
    if (GET() != value) { \
        LOG(KEY << value); \
        settings.setValue(KEY, value); \
        emit CHANGED_SIGNAL(); \
    } \
}

#define SETTING_SETTER(F, KEY, TYPE) SETTING_SETTER_(F, TYPE, F, KEY, F##Changed)

#define SETTING_(GET, SET, KEY, CHANGED_SIGNAL, TYPE, GET_FUNCTION) \
SETTING_GETTER(GET, TYPE, KEY, GET_FUNCTION) \
SETTING_SETTER_(SET, TYPE, GET, KEY, CHANGED_SIGNAL)

#define SETTING2_(GET, SET, KEY, CHANGED_SIGNAL, TYPE, GET_FUNCTION, DEFAULT) \
SETTING_GETTER2(GET, TYPE, KEY, GET_FUNCTION, DEFAULT) \
SETTING_SETTER_(SET, TYPE, GET, KEY, CHANGED_SIGNAL)

#define SETTING(F, KEY, TYPE, GET_FUNCTION) SETTING_(F, F, KEY, F##Changed, TYPE, GET_FUNCTION)
#define SETTING2(F, KEY, TYPE, GET_FUNCTION, DEFAULT) SETTING2_(F, F, KEY, F##Changed, TYPE, GET_FUNCTION, DEFAULT)


#define BOOL_SETTING(F, KEY) SETTING(F, KEY, bool, toBool)
#define BOOL_SETTING2(F, KEY, DEFAULT) SETTING2(F, KEY, bool, toBool, DEFAULT)

#define INT_SETTING(F, KEY) SETTING(F, KEY, int, toInt)
#define INT_SETTING2(F, KEY, DEFAULT) SETTING2(F, KEY, int, toInt, DEFAULT)

#define ENUM_SETTING(F, KEY, TYPE, DEFAULT) Settings::TYPE Settings::F() const { \
    return (TYPE) settings.value(KEY, (int) Settings::DEFAULT).toInt(); \
} \
SETTING_SETTER(F, KEY, TYPE)

namespace {
    const QString NOTIFICATION_TURNS_DISPLAY_ON("notificationTurnsDisplayOn");
    const QString NOTIFICATION_SOUNDS_ENABLED("notificationSoundsEnabled");
    const QString NOTIFICATION_SUPPRESS_ENABLED("notificationSuppressContent");
    const QString NOTIFICATION_FEEDBACK("notificationFeedback");
    const QString STORAGE_OPTIMIZER("useStorageOptimizer");
    const QString ONLINE_ONLY_MODE("onlineOnlyMode");
    const QString SPONSORED_MESS("sponsoredMess");
    const QString SPONSORED_MESSAGES_MESSAGES_BETWEEN("sponsoredMessagesMessagesBetween");
    const QString VOICE_NOTE_VOLUME("voiceNoteVolumne");
    const QString SEND_MARKDOWN("sendMarkdown");
    const QString UNREAD_COUNT_INCLUDE_MUTED("unreadCountIncludeMuted");
    const QString FOLDERS_UNREAD_COUNT_INCLUDE_MUTED("foldersUnreadCountIncludeMuted");
}

Settings::Settings(QObject *parent) :
    QObject(parent),
    settings(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/" + QGuiApplication::organizationName() + "/" + QGuiApplication::applicationName() + "/libfernie-settings.conf", QSettings::NativeFormat)
{
    if (sponsoredMess() != SponsoredMessHandle) {
        settings.remove(SPONSORED_MESS);
        sponsoredMessChanged();
    }
}


BOOL_SETTING(notificationTurnsDisplayOn, NOTIFICATION_TURNS_DISPLAY_ON)
BOOL_SETTING2(notificationSoundsEnabled, NOTIFICATION_SOUNDS_ENABLED, true)
BOOL_SETTING(notificationSuppressContent, NOTIFICATION_SUPPRESS_ENABLED)

ENUM_SETTING(notificationFeedback, NOTIFICATION_FEEDBACK, NotificationFeedback, NotificationFeedbackAll)

BOOL_SETTING2(storageOptimizer, STORAGE_OPTIMIZER, true)

BOOL_SETTING(onlineOnlyMode, ONLINE_ONLY_MODE)

ENUM_SETTING(sponsoredMess, SPONSORED_MESS, SponsoredMess, SponsoredMessHandle)
SETTING(sponsoredMessagesMessagesBetween, SPONSORED_MESSAGES_MESSAGES_BETWEEN, int, toInt)

BOOL_SETTING2(sendMarkdown, SEND_MARKDOWN, true)

BOOL_SETTING(unreadCountIncludeMuted, UNREAD_COUNT_INCLUDE_MUTED)
BOOL_SETTING2(foldersUnreadCountIncludeMuted, FOLDERS_UNREAD_COUNT_INCLUDE_MUTED, true)
