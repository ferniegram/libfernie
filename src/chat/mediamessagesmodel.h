#pragma once

#include "jumpablemessagesmodel.h"

class MediaMessagesModel : public JumpableMessagesModel {
    Q_OBJECT
    Q_PROPERTY(QObject* tdlib MEMBER tdLibWrapper WRITE setTDLibWrapper NOTIFY tdlibChanged)
    Q_PROPERTY(TDLibWrapper::SearchMessagesFilter filter MEMBER searchMessagesFilter WRITE setSearchMessagesFilter NOTIFY searchMessagesFilterChanged)
public:
    MediaMessagesModel(QObject *parent = nullptr);

    void setTDLibWrapper(QObject* obj);
    void setSearchMessagesFilter(TDLibWrapper::SearchMessagesFilter filter);

    Q_INVOKABLE virtual bool clear() override;
    Q_INVOKABLE void init(qlonglong chatId, qlonglong fromMessageId = 0);

signals:
    void tdlibChanged();
    void searchMessagesFilterChanged();
    void alreadyLoaded();
    void notEmptyDetected();

private slots:
    void handleChatMessageCountReceived(int count, qlonglong chatId, TDLibWrapper::SearchMessagesFilter filter, bool onlyLocal);
    void handleMessagesReceived(qlonglong chatId, TDLibWrapper::SearchMessagesFilter filter, int extra, const QVariantList &messages, int totalCount, qlonglong nextFromMessageId);
    void handleNewMessageReceived(qlonglong chatId, const QVariantMap &message);

protected:
    virtual void setupTDLibWrapper() override;

    virtual void loadMoreHistoryImpl() override;
    virtual void loadMoreFutureImpl() override;
    virtual void loadHistoryForMessageImpl(qlonglong messageId) override;


    inline virtual void loadMessages(int extra = 0, qlonglong fromMessageId = 0, int offset = 0) override { loadMessagesWithLimit(extra, fromMessageId, offset); }
    void loadMessagesWithLimit(int extra = 0, qlonglong fromMessageId = 0, int offset = 0, int limit = 100);


    TDLibWrapper::SearchMessagesFilter searchMessagesFilter;

    qlonglong nextFromMessageId;
};
