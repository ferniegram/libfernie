#ifndef MEDIAMESSAGESMODEL_H
#define MEDIAMESSAGESMODEL_H

#include "jumpablemessagesmodel.h"

class MediaMessagesModel : public JumpableMessagesModel {
    Q_OBJECT
public:
    MediaMessagesModel(TDLibWrapper *tdLibWrapper, TDLibWrapper::SearchMessagesFilter searchMessagesFilter, QObject *parent = nullptr);

    Q_INVOKABLE virtual bool clear() override;
    Q_INVOKABLE void init(qlonglong chatId, qlonglong fromMessageId = 0);

signals:
    void alreadyLoaded();
    void notEmptyDetected();

private slots:
    void handleChatMessageCountReceived(int count, qlonglong chatId, TDLibWrapper::SearchMessagesFilter filter, bool onlyLocal);
    void handleMessagesReceived(qlonglong chatId, TDLibWrapper::SearchMessagesFilter filter, int extra, const QVariantList &messages, int totalCount, qlonglong nextFromMessageId);
    void handleNewMessageReceived(qlonglong chatId, const QVariantMap &message);

private:
    inline virtual void loadMessages(int extra = 0, qlonglong fromMessageId = 0, int offset = 0) override { loadMessagesWithLimit(extra, fromMessageId, offset); }
    void loadMessagesWithLimit(int extra = 0, qlonglong fromMessageId = 0, int offset = 0, int limit = 100);

protected:
    virtual void loadMoreHistoryImpl() override;
    virtual void loadMoreFutureImpl() override;
    virtual void loadHistoryForMessageImpl(qlonglong messageId) override;

private:
    const TDLibWrapper::SearchMessagesFilter searchMessagesFilter;
    const QStringList allowedMessageContentTypes;

    qlonglong nextFromMessageId;
};

#endif // MEDIAMESSAGESMODEL_H
