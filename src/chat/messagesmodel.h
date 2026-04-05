/*
    Copyright (C) 2020 Sebastian J. Wolf and other contributors

    This file is part of Fernschreiber.

    Fernschreiber is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Fernschreiber is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Fernschreiber. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MESSAGESMODEL_H
#define MESSAGESMODEL_H

#include <QAbstractListModel>
#include "../tdlib/tdlibwrapper.h"
#include "messagedata.h"

// MessagesModel's main job is to take care of the messages updates (content, interaction info, etc.)
// It also contains utility unctions used by subclasses and handles messages deletion and some other stuff

class MessagesModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(qlonglong chatId READ getChatId NOTIFY chatIdChanged)

public:
    MessagesModel(QObject *parent = nullptr);
    MessagesModel(TDLibWrapper *tdLibWrapper, QObject *parent = nullptr);
    ~MessagesModel() override;

    virtual QHash<int,QByteArray> roleNames() const override;
    virtual int rowCount(const QModelIndex&) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;

    Q_INVOKABLE virtual bool clear();
    Q_INVOKABLE virtual void reset();
    Q_INVOKABLE QVariantMap getMessage(int index) const;
    Q_INVOKABLE QVariantList getMessages(const QVariantList &messageIds) const;
    Q_INVOKABLE QVariantList getMessageIdsForAlbum(qlonglong albumId) const;
    Q_INVOKABLE QVariantList getMessagesForAlbum(qlonglong albumId, int startAt = 0) const;

    Q_INVOKABLE int getMessageIndex(qlonglong messageId);
    inline qlonglong getChatId() const { return chatId; }

    Q_INVOKABLE void markGeneratedContentAsRead(int index);

signals:
    void chatIdChanged();
    void messageUpdated(int modelIndex);

private slots:
    void handleMessageReceived(qlonglong chatId, qlonglong messageId, const QVariantMap &message);
    void handleMessageSendSucceeded(qlonglong chatId, qlonglong oldMessageId, qlonglong messageId, const QVariantMap &message);
    void handleMessageContentUpdated(qlonglong chatId, qlonglong messageId, const QVariantMap &newContent);
    void handleMessageEditedUpdated(qlonglong chatId, qlonglong messageId, int editDate, const QVariantMap &replyMarkup);
    void handleMessageInteractionInfoUpdated(qlonglong chatId, qlonglong messageId, const QVariantMap &updatedInfo);
    void handleMessageSuggestedPostInfoUpdated(qlonglong chatId, qlonglong messageId, const QVariantMap &suggestedPostInfo);
    void handleMessageMentionRead(qlonglong chatId, qlonglong messageId);
    void handleMessageContentOpened(qlonglong chatId, qlonglong messageId);
    void handleMessageFactCheckUpdated(qlonglong chatId, qlonglong messageId, const QVariantMap &factCheck);

private:
    void updateAlbumMessages(qlonglong albumId, bool checkDeleted);
    void handleAlbumMessageUpdated(qlonglong albumId);
    void updateAlbumMessages(QList<qlonglong> albumIds, bool checkDeleted);
    void setMessagesAlbum(MessageData *message);

protected:
    virtual void setupTDLibWrapper();
    virtual void removeRange(int firstDeleted, int lastDeleted, bool updateAlbums = true, bool updateIsFirstLastInSequence = true, bool invertIsFirstLastInSequence = false);
    virtual void insertMessages(const QList<MessageData*> newMessages);
    virtual void insertMessagesAt(int index, const QList<MessageData*> newMessages, bool updateIsFirstLastFirstInSequence = true);
    virtual void appendMessages(const QList<MessageData*> newMessages, bool updateIsLastInSequence = true);
    virtual void prependMessages(const QList<MessageData*> newMessages, bool updateIsFirstInSequence = true);
    void setMessagesAlbum(const QList<MessageData*> newMessages);
    int findLastSentMessageIndex();
    virtual bool handleInsertMessages(const QVariantList &messages, QList<MessageData*> &newMessagesList, bool setAlbum = true, bool reverseOrder = false);
    inline virtual void processMessageData(MessageData* message) {}
    virtual bool messageIsFirstInSequence(const int index, const MessageData *message) const;
    virtual bool messageIsLastInSequence(const int index, const MessageData *message) const;

protected slots:
    virtual void handleMessagesDeleted(qlonglong chatId, const QList<qlonglong> &messageIds);

protected:
signals:
    void messageSendSucceeded();

protected:
    TDLibWrapper *tdLibWrapper;
    qlonglong chatId;
    QList<MessageData*> messages;
    QHash<qlonglong,int> messageIndexMap;
    QHash<qlonglong, QVariantList> albumMessageMap;
};

#endif // MESSAGESMODEL_H
