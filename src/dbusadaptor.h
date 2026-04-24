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

#ifndef DBUSADAPTOR_H
#define DBUSADAPTOR_H

#include <QDBusAbstractAdaptor>

#include "tdlib/tdlibwrapper.h"

class DBusAdaptor : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "io.libfernie.default")

public:
    DBusAdaptor(TDLibWrapper *tdLibWrapper, QObject *parent = nullptr);

signals:
    void doOpenMessage(qlonglong chatId, qlonglong messageId);

public slots:
    virtual void openUrl(const QStringList &arguments);
    void openMessage(const QString &chatId, const QString &messageId);
    virtual void markMessageAsRead(const QString &chatId, const QString &messageId);
    virtual void replyToMessage(const QString &chatId, const QString &messageId, const QString &messageContent);

private:
    TDLibWrapper *tdLibWrapper;
    bool defaultOpenUrl;
};

#endif // DBUSADAPTOR_H
