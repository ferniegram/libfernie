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

#include "dbusadaptor.h"

#include <QQuickView>

#define DEBUG_MODULE DBusAdaptor
#include "debuglog.h"

DBusAdaptor::DBusAdaptor(QObject *parent): QDBusAbstractAdaptor(parent) {}

void DBusAdaptor::openMessage(const QString &chatId, const QString &messageId) {
    LOG("Opening message" << chatId << messageId);
    emit doOpenMessage(chatId, messageId);
}

void DBusAdaptor::openUrl(const QStringList &arguments) {
    LOG("Opening URL" << arguments);
    if (arguments.isEmpty())
        return;

    const QString url = arguments.first();
    if (!url.isEmpty())
        emit doOpenUrl(url);
}
