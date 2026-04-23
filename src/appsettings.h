#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QObject>
#include <QSettings>
#include <QStandardPaths>

#define SETTING_DEFINE(TYPE, F) \
public: \
Q_PROPERTY(TYPE F READ F WRITE F NOTIFY F##Changed) \
TYPE F() const; \
void F(TYPE value);

#define BOOL_SETTING_DEFINE(F) SETTING_DEFINE(bool, F)

class AppSettings : public QObject {
    Q_OBJECT

public:
    enum SponsoredMess {
        SponsoredMessHandle,
        SponsoredMessHandleCustomMessagesBetween,
        SponsoredMessAutoView,
        SponsoredMessIgnore
    };
    Q_ENUM(SponsoredMess)

    enum NotificationFeedback {
        NotificationFeedbackNone,
        NotificationFeedbackNew,
        NotificationFeedbackAll
    };
    Q_ENUM(NotificationFeedback)

public:
    AppSettings(QObject *parent = Q_NULLPTR);

    BOOL_SETTING_DEFINE(sendByEnter)
    BOOL_SETTING_DEFINE(focusTextAreaAfterSend)
    BOOL_SETTING_DEFINE(showStickersAsEmojis)
    BOOL_SETTING_DEFINE(showStickersAsImages)
    BOOL_SETTING_DEFINE(animateStickers)
    BOOL_SETTING_DEFINE(videoStickers)
    BOOL_SETTING_DEFINE(notificationTurnsDisplayOn)
    BOOL_SETTING_DEFINE(notificationSoundsEnabled)
    BOOL_SETTING_DEFINE(notificationSuppressContent)

    SETTING_DEFINE(NotificationFeedback, notificationFeedback)

    BOOL_SETTING_DEFINE(notificationAlwaysShowPreview)
    BOOL_SETTING_DEFINE(goToQuotedMessage)
    BOOL_SETTING_DEFINE(storageOptimizer)
    BOOL_SETTING_DEFINE(allowInlineBotLocationAccess)

    SETTING_DEFINE(int, remainingInteractionHints)
    SETTING_DEFINE(int, remainingDoubleTapHints)

    BOOL_SETTING_DEFINE(onlineOnlyMode)
    BOOL_SETTING_DEFINE(delayMessageRead)
    BOOL_SETTING_DEFINE(highlightUnreadConversations)
    BOOL_SETTING_DEFINE(focusTextAreaOnChatOpen)

    SETTING_DEFINE(SponsoredMess, sponsoredMess)
    SETTING_DEFINE(int, sponsoredMessagesMessagesBetween)

    BOOL_SETTING_DEFINE(sendAttachmentByEnter)

    SETTING_DEFINE(qreal, voiceNoteVolume)

    BOOL_SETTING_DEFINE(showTranslateOption)
    BOOL_SETTING_DEFINE(formattedTranslate)
    BOOL_SETTING_DEFINE(sendMarkdown)

    BOOL_SETTING_DEFINE(unreadCountIncludeMuted)
    BOOL_SETTING_DEFINE(showFolderUnreadCount)
    BOOL_SETTING_DEFINE(foldersUnreadCountIncludeMuted)
    BOOL_SETTING_DEFINE(archiveChatListHintCompleted)
    BOOL_SETTING_DEFINE(chatFoldersTabBarOnBottom)
    BOOL_SETTING_DEFINE(chatFoldersTabBarShowIcons)
    BOOL_SETTING_DEFINE(forceQtAudioRecorder)
    BOOL_SETTING_DEFINE(downscaleAnimatedStickers)
    BOOL_SETTING_DEFINE(forceAllowAISummary)

// FIXME: macros should handle signals too
signals:
    void sendByEnterChanged();
    void focusTextAreaAfterSendChanged();
    void showStickersAsEmojisChanged();
    void showStickersAsImagesChanged();
    void animateStickersChanged();
    void notificationTurnsDisplayOnChanged();
    void notificationSoundsEnabledChanged();
    void notificationSuppressContentChanged();
    void notificationFeedbackChanged();
    void notificationAlwaysShowPreviewChanged();
    void goToQuotedMessageChanged();
    void storageOptimizerChanged();
    void allowInlineBotLocationAccessChanged();
    void remainingInteractionHintsChanged();
    void remainingDoubleTapHintsChanged();
    void onlineOnlyModeChanged();
    void delayMessageReadChanged();
    void focusTextAreaOnChatOpenChanged();
    void sponsoredMessChanged();
    void sponsoredMessagesMessagesBetweenChanged();
    void highlightUnreadConversationsChanged();
    void sendAttachmentByEnterChanged();
    void voiceNoteVolumeChanged();
    void showTranslateOptionChanged();
    void videoStickersChanged();
    void formattedTranslateChanged();
    void sendMarkdownChanged();
    void unreadCountIncludeMutedChanged();
    void showFolderUnreadCountChanged();
    void foldersUnreadCountIncludeMutedChanged();
    void archiveChatListHintCompletedChanged();
    void chatFoldersTabBarOnBottomChanged();
    void chatFoldersTabBarShowIconsChanged();
    void forceQtAudioRecorderChanged();
    void downscaleAnimatedStickersChanged();
    void forceAllowAISummaryChanged();

private:
    QSettings settings;
};

#undef BOOL_SETTING_DEFINE
#undef SETTING_DEFINE

#endif // APPSETTINGS_H
