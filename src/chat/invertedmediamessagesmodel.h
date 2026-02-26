#ifndef INVERTEDMEDIAMESSAGESMODEL_H
#define INVERTEDMEDIAMESSAGESMODEL_H

#include "mediamessagesmodel.h"

class InvertedMediaMessagesModel : public MediaMessagesModel {
    Q_OBJECT
public:
    explicit InvertedMediaMessagesModel(QObject *parent = nullptr);

protected:
    inline virtual void appendMessages(const QList<MessageData*> newMessages, bool updateIsLastInSequence = true) override {
        MessagesModel::prependMessages(newMessages, updateIsLastInSequence);
    }
    inline virtual void prependMessages(const QList<MessageData*> newMessages, bool updateIsFirstInSequence = true) override {
        MessagesModel::appendMessages(newMessages, updateIsFirstInSequence);
    }
    inline virtual bool handleInsertMessages(const QVariantList &messages, QList<MessageData*> &newMessagesList, bool setAlbum = true, bool reverseOrder = true) override {
        return MessagesModel::handleInsertMessages(messages, newMessagesList, setAlbum, true);
    }
    inline virtual void removeRange(int firstDeleted, int lastDeleted, bool updateAlbums = true, bool updateIsFirstLastInSequence = true, bool invertIsFirstLastInSequence = true) override {
        return MessagesModel::removeRange(firstDeleted, lastDeleted, updateAlbums, updateIsFirstLastInSequence, true);
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

#endif // INVERTEDMEDIAMESSAGESMODEL_H
