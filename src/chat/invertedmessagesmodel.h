#ifndef INVERTEDMESSAGESMODEL_H
#define INVERTEDMESSAGESMODEL_H

#include "messagesmodel.h"

class InvertedMessagesModel : public MessagesModel {
    Q_OBJECT
public:
    explicit InvertedMessagesModel(TDLibWrapper *tdLibWrapper, QObject *parent = nullptr);

protected:
    inline virtual void appendMessages(const QList<MessageData*> newMessages, bool updateIsLastInSequence = true) override {
        MessagesModel::prependMessages(newMessages, updateIsLastInSequence);
    }
    inline virtual void prependMessages(const QList<MessageData*> newMessages, bool updateIsFirstInSequence = true) override {
        MessagesModel::appendMessages(newMessages, updateIsFirstInSequence);
    }
    inline virtual bool handleInsertMessages(const QVariantList &messages, QList<MessageData*> &newMessagesList, bool setAlbum = true, bool reverseOrder = true) override {
        return MessagesModel::handleInsertMessages(messages, newMessagesList, setAlbum, reverseOrder);
    }
    inline virtual void removeRange(int firstDeleted, int lastDeleted, bool updateAlbums = true, bool updateIsFirstLastInSequence = true, bool invertIsFirstLastInSequence = true) override {
        return MessagesModel::removeRange(firstDeleted, lastDeleted, updateAlbums, updateIsFirstLastInSequence, invertIsFirstLastInSequence);
    }
    inline virtual bool messageIsFirstInSequence(const int index, const MessageData *message) const override {
        return MessagesModel::messageIsLastInSequence(index, message);
    }
    inline virtual bool messageIsLastInSequence(const int index, const MessageData *message) const override {
        return MessagesModel::messageIsFirstInSequence(index, message);
    }

protected slots:
    virtual void handleMessagesDeleted(qlonglong chatId, const QList<qlonglong> &messageIds) override;
};

#endif // INVERTEDMESSAGESMODEL_H
