#pragma once

#include <QObject>

namespace CallAudio {
    QString getOutputDeviceName();
    QString getInputDeviceName();

    void toggleSpeakerphone(const QString &deviceName, bool enabled);
};
