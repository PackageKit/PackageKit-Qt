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
#include "offline_p.h"

Q_DECLARE_LOGGING_CATEGORY(PACKAGEKITQT_OFFLINE)

using namespace PackageKit;

Offline::Offline(QObject *parent) : QObject(parent)
  , d_ptr(new OfflinePrivate(this))
{
    QDBusConnection::systemBus().connect(PK_NAME,
                                         PK_PATH,
                                         DBUS_PROPERTIES,
                                         QLatin1String("PropertiesChanged"),
                                         this,
                                         SLOT(updateProperties(QString,QVariantMap,QStringList)));
}

Offline::~Offline()
{
    delete d_ptr;
}

QVariantMap Offline::preparedUpgrade() const
{
    Q_D(const Offline);
    return d->preparedUpgrade;
}

Offline::Action Offline::triggerAction() const
{
    Q_D(const Offline);
    return d->triggerAction;
}

bool Offline::updatePrepared() const
{
    Q_D(const Offline);
    return d->updatePrepared;
}

bool Offline::updateTriggered() const
{
    Q_D(const Offline);
    return d->updateTriggered;
}

bool Offline::upgradePrepared() const
{
    Q_D(const Offline);
    return d->upgradePrepared;
}

bool Offline::upgradeTriggered() const
{
    Q_D(const Offline);
    return d->upgradeTriggered;
}

QDBusPendingReply<> Offline::trigger(Action action)
{
    Q_D(Offline);

    QString actionStr;
    switch(action) {
    case ActionPowerOff:
        actionStr = QStringLiteral("power-off");
        break;
    case ActionReboot:
        actionStr = QStringLiteral("reboot");
        break;
    case ActionUnset:
        break;
    };
    Q_ASSERT(!actionStr.isEmpty());

    // Manually invoke dbus because the qdbusxml2cpp does not allow
    // setting the ALLOW_INTERACTIVE_AUTHORIZATION flag
    auto msg = QDBusMessage::createMethodCall(PK_NAME,
                                              PK_PATH,
                                              PK_OFFLINE_INTERFACE,
                                              QStringLiteral("Trigger"));
    msg << actionStr;
    msg.setInteractiveAuthorizationAllowed(true);
    return QDBusConnection::systemBus().asyncCall(msg);
}

QDBusPendingReply<> Offline::triggerUpgrade(Action action)
{
    Q_D(Offline);

    QString actionStr;
    switch(action) {
    case ActionPowerOff:
        actionStr = QStringLiteral("power-off");
        break;
    case ActionReboot:
        actionStr = QStringLiteral("reboot");
        break;
    case ActionUnset:
        break;
    };
    Q_ASSERT(!actionStr.isEmpty());

    // Manually invoke dbus because the qdbusxml2cpp does not allow
    // setting the ALLOW_INTERACTIVE_AUTHORIZATION flag
    auto msg = QDBusMessage::createMethodCall(PK_NAME,
                                              PK_PATH,
                                              PK_OFFLINE_INTERFACE,
                                              QStringLiteral("TriggerUpgrade"));
    msg << actionStr;
    msg.setInteractiveAuthorizationAllowed(true);
    return QDBusConnection::systemBus().asyncCall(msg, 24 * 60 * 1000 * 1000);
}

QDBusPendingReply<> Offline::cancel()
{
    // Manually invoke dbus because the qdbusxml2cpp does not allow
    // setting the ALLOW_INTERACTIVE_AUTHORIZATION flag
    auto msg = QDBusMessage::createMethodCall(PK_NAME,
                                              PK_PATH,
                                              PK_OFFLINE_INTERFACE,
                                              QStringLiteral("Cancel"));
    msg.setInteractiveAuthorizationAllowed(true);
    return QDBusConnection::systemBus().asyncCall(msg);
}

QDBusPendingReply<> Offline::clearResults()
{
    // Manually invoke dbus because the qdbusxml2cpp does not allow
    // setting the ALLOW_INTERACTIVE_AUTHORIZATION flag
    auto msg = QDBusMessage::createMethodCall(PK_NAME,
                                              PK_PATH,
                                              PK_OFFLINE_INTERFACE,
                                              QStringLiteral("ClearResults"));
    msg.setInteractiveAuthorizationAllowed(true);
    return QDBusConnection::systemBus().asyncCall(msg);
}

void Offline::getPrepared()
{
    Q_D(Offline);
    QDBusPendingReply<QStringList> reply = d->iface.GetPrepared();
    auto watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [=] (QDBusPendingCallWatcher *call) {
        QDBusPendingReply<QStringList> reply = *call;
        QStringList pkgids;
        if (!reply.isError()) {
            pkgids = reply.argumentAt<0>();
        } else {
            qCWarning(PACKAGEKITQT_OFFLINE) << "Failed to GetPrepared" << reply.error();
        }
        Q_EMIT preparedUpdates(pkgids);
        call->deleteLater();
    });
}

void OfflinePrivate::initializeProperties(const QVariantMap &properties)
{
    Q_Q(Offline);

    QVariantMap::ConstIterator it = properties.constBegin();
    while (it != properties.constEnd()) {
        const QString &property = it.key();
        const QVariant &value = it.value();
        if (property == QLatin1String("PreparedUpgrade")) {
            preparedUpgrade = value.toMap();;
        } else if (property == QLatin1String("TriggerAction")) {
            const QString actionStr = value.toString();
            if (actionStr == QLatin1String("power-off")) {
                triggerAction = Offline::ActionPowerOff;
            } else if (actionStr == QLatin1String("reboot")) {
                triggerAction = Offline::ActionReboot;
            } else {
                triggerAction = Offline::ActionUnset;
            }
        } else if (property == QLatin1String("UpdatePrepared")) {
            updatePrepared = value.toBool();
        } else if (property == QLatin1String("UpdateTriggered")) {
            updateTriggered = value.toBool();
        } else if (property == QLatin1String("UpgradePrepared")) {
            upgradePrepared = value.toBool();
        } else if (property == QLatin1String("UpgradeTriggered")) {
            upgradeTriggered = value.toBool();
        } else {
            qCWarning(PACKAGEKITQT_OFFLINE) << "Unknown property:" << property << value;
        }

        ++it;
    }

    if (!properties.isEmpty()) {
        q->changed();
    }
}

void OfflinePrivate::updateProperties(const QString &interface, const QVariantMap &properties, const QStringList &invalidate)
{
    if(interface != PK_OFFLINE_INTERFACE) {
        qCWarning(PACKAGEKITQT_OFFLINE) << "Cannot process" << interface << "as" << PK_OFFLINE_INTERFACE;
        return;
    }

    if (!invalidate.isEmpty()) {
        qCWarning(PACKAGEKITQT_OFFLINE) << "Properties could not be invalidated" << interface << invalidate;
    }

    initializeProperties(properties);
}

#include "moc_offline.cpp"
