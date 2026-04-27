#include "basemessagabledata.h"

#include "utilities.h"

#define DEBUG_MODULE BaseMessagableData
#include "debuglog.h"

namespace {
    const QString _TYPE("@type");
    const QString ID("id");
    const QString CHAT_ID("chat_id");
    const QString SENDER_ID("sender_id");
    const QString USER_ID("user_id");
    const QString DATE("date");
    const QString CONTENT("content");
    const QString SENDING_STATE("sending_state");
    const QString TEXT("text");
}

BaseMessagableData::BaseMessagableData(TDLibWrapper *tdLibWrapper, Utilities *utilities) :
    tdLibWrapper(tdLibWrapper),
    utilities(utilities)
{}

const QVariant BaseMessagableData::lastMessage(const QString &key) const {
    return lastMessage().value(key);
}

qlonglong BaseMessagableData::lastMessageSenderUserId() const {
    return lastMessage(SENDER_ID).toMap().value(USER_ID).toLongLong();
}

qlonglong BaseMessagableData::lastMessageSenderChatId() const {
    return lastMessage(SENDER_ID).toMap().value(CHAT_ID).toLongLong();
}

bool BaseMessagableData::lastMessageSenderIsChat() const {
    return lastMessage(SENDER_ID).toMap().value(_TYPE).toString() == "messageSenderChat";
}

qlonglong BaseMessagableData::lastMessageDate() const {
    return lastMessage(DATE).toLongLong();
}

QString BaseMessagableData::lastMessageText() const {
    return utilities->getMessageText(lastMessage(), Utilities::MessageTextSimpleWithThumbnails);
}

QVariant BaseMessagableData::lastMessageMinithumbnail() const {
    return utilities->getMessageMinithumbnail(lastMessage(CONTENT).toMap());
}

bool BaseMessagableData::lastMessageIsService() const {
    return Utilities::messageContentIsService(lastMessage(CONTENT).toMap().value(_TYPE).toString());
}


QString BaseMessagableData::lastMessageStatus() const {
    if (tdLibWrapper->myUserId() != lastMessageSenderUserId())
        return "";

    if (lastReadOutboxMessageId() >= lastMessage(ID).toLongLong())
        return "&nbsp;&nbsp;✅";
    else {
        QVariantMap message = lastMessage();
        if (message.contains(SENDING_STATE)) {
            QVariantMap sendingState = message.value(SENDING_STATE).toMap();
            if (sendingState.value(_TYPE).toString() == "messageSendingStatePending") {
                return "&nbsp;&nbsp;🕙";
            } else {
                return "&nbsp;&nbsp;❌";
            }
        } else {
            return "&nbsp;&nbsp;☑️";
        }
    }
}


qlonglong BaseMessagableData::draftMessageDate() const {
    QVariantMap draft = draftMessage();
    if(draft.isEmpty())
        return qlonglong(0);

    return draft.value(DATE).toLongLong();
}

QString BaseMessagableData::draftMessageText() const {
    QVariantMap draft = draftMessage();
    if(draft.isEmpty())
        return QString();

    return draft.value("input_message_text").toMap().value(TEXT).toMap().value(TEXT).toString();
}
