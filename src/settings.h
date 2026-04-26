#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>
#include <QStandardPaths>

#define SETTING_DEFINE(TYPE, F) \
public: \
Q_PROPERTY(TYPE F READ F WRITE F NOTIFY F##Changed) \
TYPE F() const; \
void F(TYPE value);

#define BOOL_SETTING_DEFINE(F) SETTING_DEFINE(bool, F)

class Settings : public QObject {
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
    Settings(QObject *parent = Q_NULLPTR);

    BOOL_SETTING_DEFINE(notificationTurnsDisplayOn)
    BOOL_SETTING_DEFINE(notificationSoundsEnabled)
    BOOL_SETTING_DEFINE(notificationSuppressContent)

    SETTING_DEFINE(NotificationFeedback, notificationFeedback)

    BOOL_SETTING_DEFINE(storageOptimizer)

    BOOL_SETTING_DEFINE(onlineOnlyMode)

    SETTING_DEFINE(SponsoredMess, sponsoredMess)
    SETTING_DEFINE(int, sponsoredMessagesMessagesBetween)

    BOOL_SETTING_DEFINE(sendMarkdown)

    BOOL_SETTING_DEFINE(unreadCountIncludeMuted)
    BOOL_SETTING_DEFINE(foldersUnreadCountIncludeMuted)

signals:
    void notificationTurnsDisplayOnChanged();
    void notificationSoundsEnabledChanged();
    void notificationSuppressContentChanged();
    void notificationFeedbackChanged();
    void storageOptimizerChanged();
    void onlineOnlyModeChanged();
    void sponsoredMessChanged();
    void sponsoredMessagesMessagesBetweenChanged();
    void sendMarkdownChanged();
    void unreadCountIncludeMutedChanged();
    void foldersUnreadCountIncludeMutedChanged();

private:
    QSettings settings;
};

#undef BOOL_SETTING_DEFINE
#undef SETTING_DEFINE

#endif // SETTINGS_H
