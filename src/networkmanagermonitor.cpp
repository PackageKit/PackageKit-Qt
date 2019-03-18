/*
 * This file is part of the QPackageKit project
 * Copyright (C) 2019 Daniel Nicoletti <dantti12@gmail.com>
 * Copyright (C) 2019 Antonio Larrosa <alarrosa@suse.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "networkmanagermonitor.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QList>
#include <QVariant>
#include <QString>

static QString NM_DBUS_SERVICE   = QStringLiteral("org.freedesktop.NetworkManager");
static QString NM_DBUS_PATH      = QStringLiteral("/org/freedesktop/NetworkManager");
static QString NM_DBUS_INTERFACE = QStringLiteral("org.freedesktop.NetworkManager");

using namespace PackageKit;

NetworkManagerMonitor::NetworkManagerMonitor(QObject *parent)
    : QObject(parent)
{
    QDBusConnection::systemBus().connect(NM_DBUS_SERVICE,
                              NM_DBUS_PATH,
                              NM_DBUS_INTERFACE,
                              QLatin1String("StateChanged"),
                              this, SIGNAL(networkStateChanged(uint)));
}

NetworkManagerMonitor::~NetworkManagerMonitor()
{
    QDBusConnection::systemBus().disconnect(NM_DBUS_SERVICE,
                              NM_DBUS_PATH,
                              NM_DBUS_INTERFACE,
                              QLatin1String("StateChanged"),
                              this, SIGNAL(networkStateChanged(uint)));
}

NetworkManagerMonitor::NMState NetworkManagerMonitor::state()
{
    QDBusMessage message = QDBusMessage::createMethodCall(NM_DBUS_SERVICE,
                                             NM_DBUS_PATH,
                                             NM_DBUS_INTERFACE,
                                             QLatin1String("state"));

    QDBusMessage reply = QDBusConnection::systemBus().call(message);
    if (reply.arguments().isEmpty()) return NM_STATE_UNKNOWN;

    return static_cast<NMState>(reply.arguments()[0].toUInt());
}
