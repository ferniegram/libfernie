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
#include "chatdata.h"

#include <QQuickView>

#define DEBUG_MODULE DBusAdaptor
#include "debuglog.h"

namespace {
    const QString LAST_MESSAGE("last_message");
    const QString ID("id");
}

DBusAdaptor::DBusAdaptor(TDLibWrapper *tdLibWrapper,
#ifdef USE_CALLS
                         CallsManager *callsManager,
#endif
                         QObject *parent)
    : QDBusAbstractAdaptor(parent),
      tdLibWrapper(tdLibWrapper)
#ifdef USE_CALLS
      , callsManager(callsManager)
#endif
{}

void DBusAdaptor::openUrl(const QStringList &arguments) {
    LOG("Opening URL requested" << arguments);
    if (arguments.isEmpty())
        return;

    const QString url = arguments.first();
    if (!url.isEmpty()) {
        LOG("Opening URL" << url);
        emit activateWindow();
        tdLibWrapper->getInternalLinkType(url);
    }
}

void DBusAdaptor::openMessage(const QString &chatId, const QString &messageId) {
    LOG("Opening message" << chatId << messageId);
    emit activateWindow();
    emit doOpenMessage(chatId.toLongLong(), messageId.toLongLong());
}

void DBusAdaptor::markMessageAsRead(const QString &chatIdString, const QString &messageId) {
    LOG("Requested to mark message as read" << chatIdString << messageId);

    qlonglong chatId = chatIdString.toLongLong();
    qlonglong lastMessageId = tdLibWrapper->getChat(chatId).value(LAST_MESSAGE).toMap().value(ID).toLongLong();
    if (lastMessageId) {
        LOG("Marking message as read" << chatId << messageId << lastMessageId);
        tdLibWrapper->viewMessage(chatId, lastMessageId, true, TDLibWrapper::MessageSourceNotification);
    }
}

void DBusAdaptor::replyToMessage(const QString &chatIdString, const QString &messageId, const QString &messageContent) {
    LOG("Replying to message" << chatIdString << messageId);
    qlonglong chatId = chatIdString.toLongLong();

    qlonglong lastMessageId = tdLibWrapper->getChat(chatId).value(LAST_MESSAGE).toMap().value(ID).toLongLong();
    if (lastMessageId)
        tdLibWrapper->viewMessage(chatId, lastMessageId, true, TDLibWrapper::MessageSourceNotification);

    tdLibWrapper->sendTextMessage(chatId, messageContent, messageId.toLongLong(), QVariantMap(), TDLibWrapper::getMessageSendOptions(true));
}

void DBusAdaptor::reactToMessage(const QString &chatId, const QString &messageId) {
    LOG("Reacting to message" << chatId << messageId);
    tdLibWrapper->addMessageReaction(chatId.toLongLong(), messageId.toLongLong(), tdLibWrapper->getDefaultReactionType());
}

void DBusAdaptor::closeSecretChat(const QString &chatId) {
    LOG("Closing secret chat" << chatId);

    ChatData *chat = tdLibWrapper->getChatData(chatId.toLongLong());
    if (chat && chat->chatType == TDLibWrapper::ChatTypeSecret) {
        int secretChatId = chat->chatData.value("type").toMap().value("secret_chat_id").toInt();
        tdLibWrapper->closeSecretChat(secretChatId);
    }
}

#ifdef USE_CALLS
void DBusAdaptor::acceptCall(int callId) {
    LOG("Accepting a call" << callId);
    callsManager->acceptCall(callId);
}

void DBusAdaptor::discardCall(int callId) {
    LOG("Discarding a call" << callId);
    callsManager->discardCall(callId);
}
#endif
