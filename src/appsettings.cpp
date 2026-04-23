#include "appsettings.h"

#include <QGuiApplication>

#define DEBUG_MODULE AppSettings
#include "debuglog.h"


#define SETTING_GETTER(GET, TYPE, KEY, GET_FUNCTION) TYPE AppSettings::GET() const { \
    return settings.value(KEY).GET_FUNCTION(); \
}
#define SETTING_GETTER2(GET, TYPE, KEY, GET_FUNCTION, DEFAULT) TYPE AppSettings::GET() const { \
    return settings.value(KEY, DEFAULT).GET_FUNCTION(); \
}

#define SETTING_SETTER_(SET, TYPE, GET, KEY, CHANGED_SIGNAL) void AppSettings::SET(TYPE value) { \
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

#define ENUM_SETTING(F, KEY, TYPE, DEFAULT) AppSettings::TYPE AppSettings::F() const { \
    return (TYPE) settings.value(KEY, (int) AppSettings::DEFAULT).toInt(); \
} \
SETTING_SETTER(F, KEY, TYPE)

namespace {
    const QString SEND_BY_ENTER("sendByEnter");
    const QString FOCUS_TEXTAREA_AFTER_SEND("focusTextAreaAfterSend");
    const QString SHOW_STICKERS_AS_EMOJIS("showStickersAsEmojis");
    const QString SHOW_STICKERS_AS_IMAGES("showStickersAsImages");
    const QString ANIMATE_STICKERS("animateStickers");
    const QString VIDEO_STICKERS("videoStickers");
    const QString NOTIFICATION_TURNS_DISPLAY_ON("notificationTurnsDisplayOn");
    const QString NOTIFICATION_SOUNDS_ENABLED("notificationSoundsEnabled");
    const QString NOTIFICATION_SUPPRESS_ENABLED("notificationSuppressContent");
    const QString NOTIFICATION_FEEDBACK("notificationFeedback");
    const QString NOTIFICATION_ALWAYS_SHOW_PREVIEW("notificationAlwaysShowPreview");
    const QString GO_TO_QUOTED_MESSAGE("goToQuotedMessage");
    const QString STORAGE_OPTIMIZER("useStorageOptimizer");
    const QString INLINEBOT_LOCATION_ACCESS("allowInlineBotLocationAccess");
    const QString REMAINING_INTERACTION_HINTS("remainingInteractionHints");
    const QString REMAINING_DOUBLE_TAP_HINTS("remainingDoubleTapHints");
    const QString ONLINE_ONLY_MODE("onlineOnlyMode");
    const QString DELAY_MESSAGE_READ("delayMessageRead");
    const QString FOCUS_TEXTAREA_ON_CHAT_OPEN("focusTextAreaOnChatOpen");
    const QString SPONSORED_MESS("sponsoredMess");
    const QString SPONSORED_MESSAGES_MESSAGES_BETWEEN("sponsoredMessagesMessagesBetween");
    const QString HIGHLIGHT_UNREADCONVS("highlightUnreadConversations");
    const QString SEND_ATTACHMENT_BY_ENTER("sendAttachmentByEnter");
    const QString VOICE_NOTE_VOLUME("voiceNoteVolumne");
    const QString SHOW_TRANSLATE_OPTION("showTranslateOption");
    const QString FORMATTED_TRANSLATE("formattedTranslate");
    const QString SEND_MARKDOWN("sendMarkdown");
    const QString UNREAD_COUNT_INCLUDE_MUTED("unreadCountIncludeMuted");
    const QString SHOW_FOLDER_UNREAD_COUNT("showFolderUnreadCount");
    const QString FOLDERS_UNREAD_COUNT_INCLUDE_MUTED("foldersUnreadCountIncludeMuted");
    const QString ARCHIVE_CHAT_LIST_HINT_COMPLETED("archiveChatListHintCompleted");
    const QString CHAT_FOLDERS_TAB_BAR_ON_BOTTOM("chatFoldersTabBarOnBottom");
    const QString CHAT_FOLDERS_TAB_BAR_SHOW_ICONS("chatFoldersTabBarShowIcons");
    const QString FORCE_QT_AUDIO_RECORDER("forceQtAudioRecorder");
    const QString DOWNSCALE_ANIMATED_STICKERS("downscaleAnimatedStickers");
    const QString FORCE_ALLOW_AI_SUMMARY("forceAllowAISummary");
}

AppSettings::AppSettings(QObject *parent) :
    QObject(parent),
    settings(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/" + QGuiApplication::organizationName() + "/" + QGuiApplication::applicationName() + "/libfernie-settings.conf", QSettings::NativeFormat)
{
    if (sponsoredMess() != SponsoredMessHandle) {
        settings.remove(SPONSORED_MESS);
        sponsoredMessChanged();
    }
}


BOOL_SETTING(sendByEnter, SEND_BY_ENTER)
BOOL_SETTING(focusTextAreaAfterSend, FOCUS_TEXTAREA_AFTER_SEND)
BOOL_SETTING(showStickersAsEmojis, SHOW_STICKERS_AS_EMOJIS)
BOOL_SETTING(showStickersAsImages, SHOW_STICKERS_AS_IMAGES)
BOOL_SETTING2(animateStickers, ANIMATE_STICKERS, true)
BOOL_SETTING2(videoStickers, VIDEO_STICKERS, true)
BOOL_SETTING(notificationTurnsDisplayOn, NOTIFICATION_TURNS_DISPLAY_ON)
BOOL_SETTING2(notificationSoundsEnabled, NOTIFICATION_SOUNDS_ENABLED, true)
BOOL_SETTING(notificationSuppressContent, NOTIFICATION_SUPPRESS_ENABLED)

ENUM_SETTING(notificationFeedback, NOTIFICATION_FEEDBACK, NotificationFeedback, NotificationFeedbackAll)

BOOL_SETTING(notificationAlwaysShowPreview, NOTIFICATION_ALWAYS_SHOW_PREVIEW)
BOOL_SETTING(goToQuotedMessage, GO_TO_QUOTED_MESSAGE)
BOOL_SETTING2(storageOptimizer, STORAGE_OPTIMIZER, true)
BOOL_SETTING(allowInlineBotLocationAccess, INLINEBOT_LOCATION_ACCESS)

INT_SETTING2(remainingInteractionHints, REMAINING_INTERACTION_HINTS, 3)
INT_SETTING2(remainingDoubleTapHints, REMAINING_DOUBLE_TAP_HINTS, 3)

BOOL_SETTING(onlineOnlyMode, ONLINE_ONLY_MODE)
BOOL_SETTING2(delayMessageRead, DELAY_MESSAGE_READ, true)
BOOL_SETTING(highlightUnreadConversations, HIGHLIGHT_UNREADCONVS)
BOOL_SETTING(focusTextAreaOnChatOpen, FOCUS_TEXTAREA_ON_CHAT_OPEN)

ENUM_SETTING(sponsoredMess, SPONSORED_MESS, SponsoredMess, SponsoredMessHandle)
SETTING(sponsoredMessagesMessagesBetween, SPONSORED_MESSAGES_MESSAGES_BETWEEN, int, toInt)

BOOL_SETTING(sendAttachmentByEnter, SEND_ATTACHMENT_BY_ENTER)

SETTING2(voiceNoteVolume, VOICE_NOTE_VOLUME, qreal, toReal, 1)

BOOL_SETTING(showTranslateOption, SHOW_TRANSLATE_OPTION)
BOOL_SETTING(formattedTranslate, FORMATTED_TRANSLATE)
BOOL_SETTING2(sendMarkdown, SEND_MARKDOWN, true)

BOOL_SETTING(unreadCountIncludeMuted, UNREAD_COUNT_INCLUDE_MUTED)
BOOL_SETTING2(showFolderUnreadCount, SHOW_FOLDER_UNREAD_COUNT, true)
BOOL_SETTING2(foldersUnreadCountIncludeMuted, FOLDERS_UNREAD_COUNT_INCLUDE_MUTED, true)
BOOL_SETTING(archiveChatListHintCompleted, ARCHIVE_CHAT_LIST_HINT_COMPLETED)
BOOL_SETTING(chatFoldersTabBarOnBottom, CHAT_FOLDERS_TAB_BAR_ON_BOTTOM)
BOOL_SETTING(chatFoldersTabBarShowIcons, CHAT_FOLDERS_TAB_BAR_SHOW_ICONS)
BOOL_SETTING(forceQtAudioRecorder, FORCE_QT_AUDIO_RECORDER)
BOOL_SETTING(downscaleAnimatedStickers, DOWNSCALE_ANIMATED_STICKERS)
BOOL_SETTING(forceAllowAISummary, FORCE_ALLOW_AI_SUMMARY)
