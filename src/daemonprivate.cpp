/*
 * This file is part of the QPackageKit project
 * Copyright (C) 2008 Adrien Bustany <madcat@mymadcat.com>
 * Copyright (C) 2010-2018 Daniel Nicoletti <dantti12@gmail.com>
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

#include "daemonprivate.h"
#include "transaction.h"
#include "common.h"

#include "offline_p.h"

#include <QDBusServiceWatcher>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusArgument>
#include <QDBusReply>

using namespace PackageKit;

DaemonPrivate::DaemonPrivate(Daemon* parent)
    : q_ptr(parent)
    , offline(new Offline(parent))
{
    Q_Q(Daemon);

    auto watcher = new QDBusServiceWatcher(PK_NAME,
                                           QDBusConnection::systemBus(),
                                           QDBusServiceWatcher::WatchForOwnerChange,
                                           q_ptr);
    q->connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged,
                   q, [this, q] (const QString &service, const QString &oldOwner, const QString &newOwner) {
        Q_UNUSED(service)
        if (newOwner.isEmpty() || !oldOwner.isEmpty()) {
            // TODO check if we don't emit this twice when
            // the daemon exits cleanly
            q->daemonQuit();
        }

        // There is a new PackageKit running get it's props
        if (!newOwner.isEmpty()) {
            // We don't have more transactions running
            q->transactionListChanged(QStringList());

            getAllProperties();

            if (!running) {
                running = true;
                q->isRunningChanged();
            }
        } else if (running) {
            running = false;
            q->isRunningChanged();
        }
    });

    getAllProperties();
}

void DaemonPrivate::getAllProperties()
{
    Q_Q(Daemon);

    QDBusMessage message = QDBusMessage::createMethodCall(PK_NAME,
                                                          PK_PATH,
                                                          DBUS_PROPERTIES,
                                                          QLatin1String("GetAll"));
    message << PK_NAME;
    QDBusConnection::systemBus().callWithCallback(message,
                                                  q,
                                                  SLOT(updateProperties(QVariantMap)));

    message = QDBusMessage::createMethodCall(PK_NAME,
                                             PK_PATH,
                                             DBUS_PROPERTIES,
                                             QLatin1String("GetAll"));
    message << PK_OFFLINE_INTERFACE;
    QDBusConnection::systemBus().callWithCallback(message,
                                                  offline,
                                                  SLOT(updateProperties(QVariantMap)));
}

void DaemonPrivate::propertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidatedProperties)
{
    Q_UNUSED(invalidatedProperties)

    if (interface == PK_NAME) {
        updateProperties(properties);
    } else if (interface == PK_OFFLINE_INTERFACE) {
        offline->d_ptr->updateProperties(properties);
    } else {
        qCWarning(PACKAGEKITQT_DAEMON) << "Unknown PackageKit interface:" << interface;
    }
}

void DaemonPrivate::updateProperties(const QVariantMap &properties)
{
    Q_Q(Daemon);

    if (!running) {
        running = true;
        q->isRunningChanged();
    }

    QVariantMap::ConstIterator it = properties.constBegin();
    while (it != properties.constEnd()) {
        const QString &property = it.key();
        const QVariant &value = it.value();
        if (property == QLatin1String("BackendAuthor")) {
            backendAuthor = value.toString();
        } else if (property == QLatin1String("BackendDescription")) {
            backendDescription = value.toString();
        } else if (property == QLatin1String("BackendName")) {
            backendName = value.toString();
        } else if (property == QLatin1String("DistroId")) {
            distroId = value.toString();
        } else if (property == QLatin1String("Filters")) {
            filters = static_cast<Transaction::Filters>(value.toUInt());
        } else if (property == QLatin1String("Groups")) {
            groups =  static_cast<Transaction::Groups>(value.toULongLong());
        } else if (property == QLatin1String("Locked")) {
            locked = value.toBool();
        } else if (property == QLatin1String("MimeTypes")) {
            mimeTypes = value.toStringList();
        } else if (property == QLatin1String("NetworkState")) {
            networkState = static_cast<Daemon::Network>(value.toUInt());
            q->networkStateChanged();
        } else if (property == QLatin1String("Roles")) {
            roles = value.toULongLong();
        } else if (property == QLatin1String("VersionMajor")) {
            versionMajor = value.toUInt();
        } else if (property == QLatin1String("VersionMicro")) {
            versionMicro = value.toUInt();
        } else if (property == QLatin1String("VersionMinor")) {
            versionMinor = value.toUInt();
        } else {
            qCWarning(PACKAGEKITQT_DAEMON) << "Unknown Daemon property:" << property << value;
        }

        ++it;
    }

    if (!properties.isEmpty()) {
        q->changed();
    }
}
