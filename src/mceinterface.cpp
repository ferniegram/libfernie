/*
    Copyright (C) 2020 Slava Monich et al.

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

#include "mceinterface.h"
#include <QDBusConnection>
#include <QDBusReply>

#define DEBUG_MODULE MceInterface
#include "debuglog.h"

namespace {
    const char *MCE_DBUS_SERVICE = "com.nokia.mce";

    const char *MCE_DBUS_PATH_SIGNAL = "/com/nokia/mce/signal";
    const char *MCE_DBUS_INTERFACE_SIGNAL = "com.nokia.mce.signal";
}

MceInterface::MceInterface(QObject *parent) :
    QDBusInterface(MCE_DBUS_SERVICE, "/com/nokia/mce/request", "com.nokia.mce.request",
    QDBusConnection::systemBus(), parent)
{
    // Get initial state
    updatePowerSaveMode();

    QDBusConnection::systemBus().connect(MCE_DBUS_SERVICE, MCE_DBUS_PATH_SIGNAL, MCE_DBUS_INTERFACE_SIGNAL,
                                         "psm_state_ind", this, SLOT(handlePowerSaveModeChanged(bool)));
}

void MceInterface::setLedPattern(const QString &pattern, bool activate) {
    LOG("Setting pattern" << pattern << activate);
    call(activate ? QStringLiteral("req_led_pattern_activate") : QStringLiteral("req_led_pattern_deactivate"), pattern);
}

void MceInterface::updatePowerSaveMode() {
    QDBusReply<bool> reply = call(QStringLiteral("get_psm_state"));
    if (reply.isValid()) {
        powerSaveMode = reply.value();
        LOG("Initial power save mode" << powerSaveMode);
    } else
        WARN("Failed to get power save mode" << reply.error().message());
}

void MceInterface::handlePowerSaveModeChanged(bool active) {
    if (powerSaveMode != active) {
        LOG("Power save mode changed" << active);
        powerSaveMode = active;
        emit powerSaveModeChanged(powerSaveMode);
    }
}
