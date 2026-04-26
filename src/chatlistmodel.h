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

#ifndef CHATLISTMODEL_H
#define CHATLISTMODEL_H

#include <QAbstractListModel>
#include <QTimer>

#include "tdlib/tdlibwrapper.h"
#include "appsettings.h"
#include "utilities.h"
#include "chatdata.h"

class ChatListModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(int unreadChatCount READ getUnreadChatCount NOTIFY unreadChatCountChanged)
    Q_PROPERTY(int unreadMessageCount READ getUnreadMessageCount NOTIFY unreadMessageCountChanged)
public:
    ChatListModel(TDLibWrapper *tdLibWrapper, AppSettings *appSettings, Utilities *utilities, bool archive = false, bool doNotConnectChatListSignals = false);
    ~ChatListModel() override;

    QHash<int,QByteArray> roleNames() const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &index = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;

    Q_INVOKABLE void redrawModel();
    Q_INVOKABLE QVariantMap get(int row);

    virtual int getUnreadChatCount(bool asFolder = false) const;
    virtual int getUnreadMessageCount(bool asFolder = false) const;

public slots:
    Q_INVOKABLE void reset();

    Q_INVOKABLE void calculateUnreadState();

    void handleChatAddedToList(ChatData *chatData, qlonglong order, bool isPinned);

signals:
    void unreadChatCountChanged();
    void unreadMessageCountChanged();

protected slots:
    void handleUnreadChatCountUpdated(const QVariantMap &chatCountInformation);
    void handleUnreadMessageCountUpdated(const QVariantMap &messageCountInformation);

    void handleChatRemovedFromList(qlonglong chatId);
    void handleChatPositionUpdated(qlonglong chatId, qlonglong order, bool isPinned);

private slots:
    void handleChatRolesChanged(qlonglong chatId, const QVector<int> changedRoles);
    void handleChatPinnedMessageUpdated(qlonglong chatId, qlonglong pinnedMessageId);
    void handleMessageSendSucceeded(qlonglong chatId, qlonglong oldMessageId, qlonglong messageId, const QVariantMap &message);
    void handleRelativeTimeRefreshTimer();

signals:
    void countChanged();
    void chatJoined(const qlonglong &chatId, const QString &chatTitle);
    void unreadStateChanged(int unreadMessagesCount, int unreadChatsCount);

private:
    class ListChatData {
    public:
        ListChatData(ChatData *data, qlonglong order, bool isPinned);
        ChatData *data;
        qlonglong order;
        bool isPinned;
        bool setOrder(const QVariant &order);
        int compareTo(const ListChatData *other) const;
    };

    int updateChatOrder(const int chatIndex);
    void updateChatIsPinned(const int chatIndex, const bool isPinned);
    void enableRefreshTimer();

private:
    TDLibWrapper *tdLibWrapper;
    Utilities *utilities;
    QTimer *relativeTimeRefreshTimer;
    QList<ListChatData*> chatList; // should we use a list of pointers to ListChatData or of plain ListChatData?
    QHash<qlonglong, int> chatIndexMap;
    bool archive;

protected:
    AppSettings *appSettings;

    int unreadChatCount;
    int unreadUnmutedChatCount;
    int unreadMessageCount;
    int unreadUnmutedMessageCount;
};

#endif // CHATLISTMODEL_H
