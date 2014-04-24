/*
 * This file is part of the QPackageKit project
 * Copyright (C) 2008 Adrien Bustany <madcat@mymadcat.com>
 * Copyright (C) 2010-2011 Daniel Nicoletti <dantti85-pk@yahoo.com.br>
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

#ifndef PACKAGEKIT_TRANSACTION_PRIVATE_H
#define PACKAGEKIT_TRANSACTION_PRIVATE_H

#include <QString>
#include <QHash>
#include <QStringList>
#include <QDBusPendingCallWatcher>

#include "transaction.h"

class TransactionProxy;

namespace PackageKit {

class TransactionPrivate
{
    Q_DECLARE_PUBLIC(Transaction)
    friend class Daemon;
protected:
    TransactionPrivate(Transaction *parent);
    virtual ~TransactionPrivate();

    void setup(const QDBusObjectPath &transactionId);
    void runQueuedTransaction();

    QDBusObjectPath tid;
    ::TransactionProxy* p = 0;
    Transaction *q_ptr;
    QStringList connectedSignals;

    bool sentFinished = false;
    bool allowCancel = false;
    bool callerActive = false;
    qulonglong downloadSizeRemaining = 0;
    uint elapsedTime = 0;
    QString lastPackage;
    uint percentage = 0;
    uint remainingTime = 0;
    Transaction::Role role = Transaction::RoleUnknown;
    uint speed = 0;
    Transaction::Status status = Transaction::StatusUnknown;
    uint uid = 0;

    // Queue params
    QString eulaId;
    bool storeInCache;
    Transaction::Filters filters;
    bool recursive = false;
    uint numberOfOldTransactions = 0;
    Transaction::TransactionFlags transactionFlags = Transaction::TransactionFlagNone;
    Transaction::SigType signatureType = Transaction::SigTypeUnknown;
    QString signatureKey;
    QString signaturePackage;
    bool refreshCacheForce;
    bool allowDeps;
    bool autoremove;
    QString repoId;
    QString repoParameter;
    QString repoValue;
    bool repoEnable;
    QStringList search;

    // Only used for old transactions
    QDateTime timespec;
    bool succeeded;
    uint duration;
    QString data;
    QString cmdline;

    void setupSignal(const QString &signal, bool connect);

protected Q_SLOTS:
    void createTransactionFinished(QDBusPendingCallWatcher *call);
    void methodCallFinished(QDBusPendingCallWatcher *call);
    void details(const QVariantMap &values);
    void distroUpgrade(uint type, const QString &name, const QString &description);
    void errorCode(uint error, const QString &details);
    void mediaChangeRequired(uint mediaType, const QString &mediaId, const QString &mediaText);
    void finished(uint exitCode, uint runtime);
    void message(uint type, const QString &message);
    void Package(uint info, const QString &pid, const QString &summary);
    void ItemProgress(const QString &itemID, uint status, uint percentage);
    void RepoSignatureRequired(const QString &pid,
                               const QString &repoName,
                               const QString &keyUrl,
                               const QString &keyUserid,
                               const QString &keyId,
                               const QString &keyFingerprint,
                               const QString &keyTimestamp,
                               uint type);
    void requireRestart(uint type, const QString &pid);
    void transaction(const QDBusObjectPath &oldTid, const QString &timespec, bool succeeded, uint role, uint duration, const QString &data, uint uid, const QString &cmdline);
    void UpdateDetail(const QString &package_id, const QStringList &updates, const QStringList &obsoletes, const QStringList &vendor_urls, const QStringList &bugzilla_urls, const QStringList &cve_urls, uint restart, const QString &update_text, const QString &changelog, uint state, const QString &issued, const QString &updated);
    void destroy();
    void daemonQuit();
    void propertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidatedProperties);
    void updateProperties(const QVariantMap &properties);
};

} // End namespace PackageKit

#endif
