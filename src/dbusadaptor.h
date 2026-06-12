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

#pragma once

#include <QDBusAbstractAdaptor>

#include "tdlib/tdlibwrapper.h"
#ifdef USE_CALLS
#include "callsmanager.h"
#endif

class DBusAdaptor : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "io.yaqtlib.default")

public:
    DBusAdaptor(TDLibWrapper *tdLibWrapper,
#ifdef USE_CALLS
                CallsManager *callsManager,
#endif
                QObject *parent = nullptr);

signals:
    void doOpenMessage(qlonglong chatId, qlonglong messageId, const QVariantMap &topicId);
    void activateWindow();

public slots:
    virtual void openUrl(const QStringList &arguments);
    void openMessage(const QString &chatId, const QString &messageId, const QVariantMap &topicId);
    virtual void markMessageAsRead(const QString &chatId, const QString &messageId, const QVariantMap &topicId);
    virtual void replyToMessage(const QString &chatId, const QString &messageId, const QVariantMap &topicId, const QString &messageContent);
    virtual void reactToMessage(const QString &chatId, const QString &messageId, const QVariantMap &topicId);
    virtual void closeSecretChat(const QString &chatId);
#ifdef USE_CALLS
    virtual void acceptCall(int callId);
    virtual void discardCall(int callId);
#endif

private:
    TDLibWrapper *tdLibWrapper;
#ifdef USE_CALLS
    CallsManager *callsManager;
#endif
};
