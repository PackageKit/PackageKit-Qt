/*
 * This file is part of the QPackageKit project
 * Copyright (C) 2008 Adrien Bustany <madcat@mymadcat.com>
 * Copyright (C) 2010-2016 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef DAEMON_PRIVATE_H
#define DAEMON_PRIVATE_H

#include <QStringList>
#include <QLoggingCategory>

#include "daemon.h"
#include "offline.h"

Q_DECLARE_LOGGING_CATEGORY(PACKAGEKITQT_DAEMON)
Q_DECLARE_LOGGING_CATEGORY(PACKAGEKITQT_OFFLINE)

class OrgFreedesktopPackageKitInterface;

namespace PackageKit {

static QString PK_NAME = QStringLiteral("org.freedesktop.PackageKit");
static QString PK_OFFLINE_INTERFACE = QStringLiteral("org.freedesktop.PackageKit.Offline");
static QString PK_PATH = QStringLiteral("/org/freedesktop/PackageKit");
static QString PK_TRANSACTION_INTERFACE = QStringLiteral("org.freedesktop.PackageKit.Transaction");

static QString DBUS_PROPERTIES = QStringLiteral("org.freedesktop.DBus.Properties");

class DaemonPrivate
{
    Q_DECLARE_PUBLIC(Daemon)
protected:
    DaemonPrivate(Daemon *parent);
    virtual ~DaemonPrivate() {}

    Daemon *q_ptr;
    ::OrgFreedesktopPackageKitInterface *daemon;
    QStringList hints;
    QList<QMetaMethod> connectedSignals;

    void setupSignal(const QMetaMethod &signal);
    void getAllProperties();

    QString backendAuthor;
    QString backendDescription;
    QString backendName;
    QString distroId;
    Transaction::Filters filters = Transaction::FilterNone;
    Transaction::Groups groups = Transaction::GroupUnknown;
    QStringList mimeTypes;
    Daemon::Network networkState = Daemon::NetworkUnknown;
    Transaction::Roles roles = Transaction::RoleUnknown;
    Offline *offline;
    uint versionMajor = 0;
    uint versionMicro = 0;
    uint versionMinor = 0;
    bool locked = false;
    bool running = false;

protected Q_SLOTS:
    void propertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidatedProperties);
    void updateProperties(const QVariantMap &properties);
};

} // End namespace PackageKit

#endif
