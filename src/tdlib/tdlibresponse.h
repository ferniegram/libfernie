#ifndef TDLIBRESPONSE_H
#define TDLIBRESPONSE_H

#include <QObject>

class TDLibWrapper;

class TDLibResponse : public QObject {
    Q_OBJECT
public:
    explicit TDLibResponse(qlonglong id, TDLibWrapper *tdLibWrapper);

signals:
    void finished(const QString &type, const QVariantMap &response);

private slots:
    void handleResponseForRequestIdReceived(qlonglong requestId, const QVariantMap &response);

private:
    QMetaObject::Connection connection;
    qlonglong id;
};

#endif // TDLIBRESPONSE_H
