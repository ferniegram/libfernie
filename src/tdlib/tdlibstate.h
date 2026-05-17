#ifndef TDLIBSTATE_H
#define TDLIBSTATE_H

#include "tdlibwrapper.h"

class TDLibState : public QObject {
    Q_OBJECT

public:
    explicit TDLibState(TDLibWrapper *tdLibWrapper, QObject *parent = nullptr);

private slots:
    void reset();

private:
    TDLibWrapper *tdLibWrapper;
};

#endif // TDLIBSTATE_H
