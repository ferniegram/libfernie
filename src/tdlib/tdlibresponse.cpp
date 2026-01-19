#include "tdlibresponse.h"

#include "tdlibwrapper.h"

#define DEBUG_MODULE TDLibResponse
#include "debuglog.h"

#define LOG_(x) LOG(id << x)

TDLibResponse::TDLibResponse(qlonglong id, TDLibWrapper *tdLibWrapper) : QObject(tdLibWrapper), id(id) {
    this->connection = connect(tdLibWrapper, &TDLibWrapper::responseForRequestIdReceived, this, &TDLibResponse::handleResponseForRequestIdReceived);
    LOG_("Created");
}

void TDLibResponse::handleResponseForRequestIdReceived(qlonglong requestId, const QVariantMap &response) {
    if (this->id == requestId) {
        LOG_("Finished" << response.value("@type").toString());
        emit this->finished(response);
        disconnect(this->connection); // FIXME: is this really needed?
        this->deleteLater();
    }
}
