#include "tdlibstate.h"

#define DEBUG_MODULE TDLibState
#include "debuglog.h"

TDLibState::TDLibState(TDLibWrapper *tdLibWrapper, QObject *parent) :
    QObject(parent), tdLibWrapper(tdLibWrapper)
{
    connect(tdLibWrapper, &TDLibWrapper::clearContent, this, &TDLibState::reset);
}

void TDLibState::reset() {
    LOG("Resetting");
}
