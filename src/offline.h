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
#ifndef OFFLINE_H
#define OFFLINE_H

#include <QObject>
#include <QDBusPendingReply>
#include <QVariantMap>

#include <packagekitqt_global.h>

namespace PackageKit {

class OfflinePrivate;
class PACKAGEKITQT_LIBRARY Offline : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Offline)
public:
    /**
     * Actions to trigger
     *
     * \sa offlineTrigger()
     */
    enum Action {
        ActionUnset,
        ActionPowerOff, /** < powers off the computer after applying offline updates */
        ActionReboot    /** < reboots the computer after applying offline updates */
    };
    Q_ENUM(Action)

    ~Offline();

    Q_PROPERTY(QVariantMap preparedUpgrade READ preparedUpgrade NOTIFY changed)
    /**
     * Details about a prepared system upgrade.
     * Currently recognized keys are "name" and "version".
     */
    QVariantMap preparedUpgrade() const;

    Q_PROPERTY(Action triggerAction READ triggerAction NOTIFY changed)
    /**
     * The action to take when finished applying updates, known values
     */
    Action triggerAction() const;

    Q_PROPERTY(bool updatePrepared READ updatePrepared NOTIFY changed)
    /**
     * If an update has been prepared and is ready to be triggered.
     */
    bool updatePrepared() const;

    Q_PROPERTY(bool updateTriggered READ updateTriggered NOTIFY changed)
    /**
     * If an update has been triggered.
     */
    bool updateTriggered() const;

    Q_PROPERTY(bool upgradePrepared READ upgradePrepared NOTIFY changed)
    /**
     * If a system upgrade has been prepared and is ready to be triggered.
     */
    bool upgradePrepared() const;

    Q_PROPERTY(bool upgradeTriggered READ upgradeTriggered NOTIFY changed)
    /**
     * If a system upgrade has been triggered.
     */
    bool upgradeTriggered() const;

    /**
     * Triggers the offline update for the next boot
     *
     * @p action is the action to take when finished applying updates
     */
    QDBusPendingReply<> trigger(Action action);

    /**
     * Triggers the offline system upgrade for next boot.
     *
     * @p action is the action to take when finished installing the system upgrade
     */
    QDBusPendingReply<> triggerUpgrade(Action action);

    /**
     * Cancels the offline update so the next boot procceeds as normal.
     */
    QDBusPendingReply<> cancel();

    /**
     *  Clears the offline update results store.
     */
    QDBusPendingReply<> clearResults();

    /**
     * Asks for the list of prepared packages for update.
     * \sa preparedUpdates() signal
     */
    void getPrepared();

Q_SIGNALS:
    /**
     * A list of package-ids ready to be installed
     */
    void preparedUpdates(const QStringList &updates);

    /**
     * Emitted when any of the properties changes
     */
    void changed();

protected:
    friend class DaemonPrivate;

    explicit Offline(QObject *parent = nullptr);

private:
    Q_PRIVATE_SLOT(d_func(), void updateProperties(QVariantMap))

    OfflinePrivate *d_ptr;
};

}

#endif // OFFLINE_H
