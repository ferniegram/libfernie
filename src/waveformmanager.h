#ifndef WAVEFORMMANAGER_H
#define WAVEFORMMANAGER_H

#include <QObject>
#include <QVariant>

class WaveformManager : public QObject {
    Q_OBJECT

public:
    explicit WaveformManager(QObject *parent = nullptr);

    Q_INVOKABLE static QString encodeWaveform(const QVariantList &waveform);
    Q_INVOKABLE static QVariantList decodeWaveform(const QString &encodedData);
    Q_INVOKABLE static QVariantList getWaveformData(const QVariantList &waveform, int count);
};

#endif // WAVEFORMMANAGER_H
