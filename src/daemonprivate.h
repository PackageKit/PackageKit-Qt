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

#include <QtCore/QStringList>
#include <QtDBus/QDBusServiceWatcher>

#include "daemon.h"

class OrgFreedesktopPackageKitInterface;

namespace PackageKit {

class DaemonPrivate
{
    Q_DECLARE_PUBLIC(Daemon)
protected:
    DaemonPrivate(Daemon *parent);
    virtual ~DaemonPrivate() {}

    Daemon *q_ptr;
    ::OrgFreedesktopPackageKitInterface *daemon;
    QStringList hints;
    QList<QByteArray> connectedSignals;

    void setupSignal(const QByteArray &signal, bool connect);
    void getAllProperties(bool sync);

    QString backendAuthor;
    QString backendDescription;
    QString backendName;
    QString distroId;
    Transaction::Filters filters = Transaction::FilterNone;
    Transaction::Groups groups = Transaction::GroupUnknown;
    bool locked = false;
    QStringList mimeTypes;
    Daemon::Network networkState = Daemon::NetworkUnknown;
    Transaction::Roles roles = Transaction::RoleUnknown;
    uint versionMajor = 0;
    uint versionMicro = 0;
    uint versionMinor = 0;

    bool running = false;

protected Q_SLOTS:
    void serviceOwnerChanged(const QString &service, const QString &oldOwner, const QString &newOwner);
    void propertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidatedProperties);
    void updateProperties(const QVariantMap &properties);

private:
    QDBusServiceWatcher *m_watcher;
};

} // End namespace PackageKit

#endif
