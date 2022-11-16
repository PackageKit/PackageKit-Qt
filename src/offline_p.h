/*
 * This file is part of the PackageKitQt project
 * Copyright (C) 2018 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef OFFLINE_P_H
#define OFFLINE_P_H

#include "offline.h"
#include "daemonproxy.h"
#include "daemonprivate.h"

namespace PackageKit {
class OfflinePrivate
{
    Q_DECLARE_PUBLIC(Offline)
public:
    OfflinePrivate(Offline *q) : q_ptr(q), iface(PK_NAME, PK_PATH, QDBusConnection::systemBus())
    {
    }

    void initializeProperties(const QVariantMap &properties);
    void updateProperties(const QString &interface, const QVariantMap &properties, const QStringList &invalidate);

    Offline *q_ptr;
    OrgFreedesktopPackageKitOfflineInterface iface;
    QVariantMap preparedUpgrade;
    Offline::Action triggerAction = Offline::ActionUnset;
    bool updatePrepared = false;
    bool updateTriggered = false;
    bool upgradePrepared = false;
    bool upgradeTriggered = false;
};
}

#endif // OFFLINE_P_H
