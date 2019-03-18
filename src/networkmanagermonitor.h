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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef NETWORKMANAGERMONITOR_H
#define NETWORKMANAGERMONITOR_H

#include <QObject>

namespace PackageKit {

class NetworkManagerMonitor : public QObject
{
    Q_OBJECT
public:
    enum NMState {
        NM_STATE_UNKNOWN = 0,
        NM_STATE_ASLEEP = 10,
        NM_STATE_DISCONNECTED = 20,
        NM_STATE_DISCONNECTING = 30,
        NM_STATE_CONNECTING = 40,
        NM_STATE_CONNECTED_LOCAL = 50,
        NM_STATE_CONNECTED_SITE = 60,
        NM_STATE_CONNECTED_GLOBAL = 70
    };

    NetworkManagerMonitor(QObject *parent = nullptr);
    ~NetworkManagerMonitor();

    NMState state();

Q_SIGNALS:
    void networkStateChanged(uint state);
};

};

#endif
