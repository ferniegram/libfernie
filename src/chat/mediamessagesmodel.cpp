#include "mediamessagesmodel.h"

#include "utilities.h"

#define DEBUG_MODULE MediaMessagesModel
#include "debuglog.h"

namespace {
    const QString ID("id");
    const QString CONTENT("content");
    const QString _TYPE("@type");
}

MediaMessagesModel::MediaMessagesModel(QObject *parent)
    : JumpableMessagesModel(parent),
      searchMessagesFilter(TDLibWrapper::SearchMessagesFilterEmpty)
{}

void MediaMessagesModel::setTDLibWrapper(QObject *obj) {
    TDLibWrapper *wrapper = qobject_cast<TDLibWrapper*>(obj);
    if (tdLibWrapper != wrapper) {
        tdLibWrapper = wrapper;
        emit tdlibChanged();
        LOG("Set TDLibWrapper" << tdLibWrapper);

        if (tdLibWrapper)
            setupTDLibWrapper();
    }
}

void MediaMessagesModel::setupTDLibWrapper() {
    JumpableMessagesModel::setupTDLibWrapper();

    connect(this->tdLibWrapper, &TDLibWrapper::chatMessageCountReceived, this, &MediaMessagesModel::handleChatMessageCountReceived);
    connect(this->tdLibWrapper, &TDLibWrapper::foundChatMessagesReceived, this, &MediaMessagesModel::handleMessagesReceived);
    connect(this->tdLibWrapper, &TDLibWrapper::newMessageReceived, this, &MediaMessagesModel::handleNewMessageReceived);
}

void MediaMessagesModel::setSearchMessagesFilter(TDLibWrapper::SearchMessagesFilter filter) {
    if (this->searchMessagesFilter != filter) {
        this->searchMessagesFilter = filter;
        LOG("Filter set" << filter);
        emit searchMessagesFilterChanged();
    }
}

bool MediaMessagesModel::clear() {
    LOG("Clearing media messages model");
    this->nextFromMessageId = 0;
    return JumpableMessagesModel::clear();
}

void MediaMessagesModel::loadMessagesWithLimit(int extra, qlonglong fromMessageId, int offset, int limit) {
    if (!tdLibWrapper) {
        LOG("Can't load messages, tdLibWrapper is not set" << extra << fromMessageId << offset << limit);
        return;
    }
    LOG("Loading messages" << extra << fromMessageId << offset << limit);
    this->tdLibWrapper->searchChatMessages(this->chatId, QString(), extra, fromMessageId, this->searchMessagesFilter, limit, offset);
}

void MediaMessagesModel::init(qlonglong chatId, qlonglong fromMessageId) {
    if (!tdLibWrapper) {
        LOG("Can't initialize, tdLibWrapper is not set" << chatId << fromMessageId);
        return;
    }
    LOG("Initializing" << searchMessagesFilter << chatId << fromMessageId);

    // TODO: (maybe) add this to JumpableMessagesModel too
    if (this->chatId == chatId) {
        LOG("Model already initialized for this chat ID, checking if other required stuff is already loaded");

        if (fromMessageId == 0) {
            if (endReached) {
                LOG("Message history end already loaded, skipping initialization");
                emit alreadyLoaded();
                return;
            }
        } else {
            if (!messages.isEmpty() && messageIndexMap.contains(fromMessageId)) {
                LOG("Message is already loaded, skipping initialization");
                this->highlightedMessageId = fromMessageId;
                emit alreadyLoaded();
                return;
            }
        }
    }

    clear();
    this->chatId = chatId;
    this->highlightedMessageId = fromMessageId;

    if (fromMessageId != 0)
        loadMessagesWithLimit(UpdateInitial, fromMessageId, -16, 32);
    else
        tdLibWrapper->getChatMessageCount(chatId, this->searchMessagesFilter);
}

void MediaMessagesModel::loadMoreHistoryImpl() {
    this->loadMessages(UpdatePreviousSlice, nextFromMessageId);
}
void MediaMessagesModel::loadMoreFutureImpl() {
    this->loadMessagesWithLimit(UpdateNextSlice, messages.last()->messageId, -16, 32);
}
void MediaMessagesModel::loadHistoryForMessageImpl(qlonglong messageId) {
    this->loadMessagesWithLimit(UpdateInitial, messageId, -26, 51);
}

void MediaMessagesModel::handleChatMessageCountReceived(int count, qlonglong chatId, TDLibWrapper::SearchMessagesFilter filter, bool onlyLocal) {
    Q_UNUSED(onlyLocal) // TODO: we should first try to get count with onlyLocal = true,
    // then if we have an error (or -1 as result) try again with onlyLocal = false
    // see tgx implementatinon as well as tdlib docs for more info

    if (this->chatId == chatId && this->searchMessagesFilter == filter) {
        endReached = true;
        emit endReachedChanged();
        if (count == 0) {
            LOG("No messages in chat" << chatId << "for filter" << TDLibWrapper::getSearchMessagesFilterType(filter));
            startReached = true;
        } else {
            LOG("Found" << count << "messages in chat" << chatId << "for filter" << TDLibWrapper::getSearchMessagesFilterType(filter) << ", loading messages");
            emit notEmptyDetected();
            loadMessages(UpdateInitial);
        }
    }
}

void MediaMessagesModel::handleMessagesReceived(qlonglong chatId, TDLibWrapper::SearchMessagesFilter filter, int extra, const QVariantList &messages, int totalCount, qlonglong nextFromMessageId) {
    if (this->chatId == chatId && filter == this->searchMessagesFilter) {
        LOG("Messages received next id:" << nextFromMessageId);
        JumpableMessagesModel::handleMessagesReceived(extra, messages, totalCount);
        this->nextFromMessageId = nextFromMessageId;
    }
}

void MediaMessagesModel::handleNewMessageReceived(qlonglong chatId, const QVariantMap &message) {
    if (!endReached) return;

    const qlonglong messageId = message.value(ID).toLongLong();
    if (chatId == this->chatId && !messageIndexMap.contains(messageId)) {
        if (Utilities::messageMatchesSearchFilter(message, this->searchMessagesFilter)) {
            LOG("New media message received for this chat");
            insertMessages(QList<MessageData*>{new MessageData(message, messageId)});
            emit notEmptyDetected();
        }
    }
}
