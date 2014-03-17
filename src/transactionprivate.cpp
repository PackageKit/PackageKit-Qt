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

#include "transactionprivate.h"

#include "transactionproxy.h"
#include "daemon.h"
#include "common.h"

#include <QStringList>
#include <QDebug>

using namespace PackageKit;

TransactionPrivate::TransactionPrivate(Transaction* parent) :
    q_ptr(parent),
    p(0),
    role(Transaction::RoleUnknown),
    status(Transaction::StatusUnknown),
    transactionFlags(Transaction::TransactionFlagNone)
{
}

void TransactionPrivate::setup(const QDBusObjectPath &transactionId)
{
    Q_Q(Transaction);

    tid = transactionId;
    p = new TransactionProxy(QLatin1String(PK_NAME),
                             tid.path(),
                             QDBusConnection::systemBus(),
                             q);
    error = Transaction::InternalErrorNone;
    errorMessage.clear();
    if (!Daemon::global()->hints().isEmpty()) {
        q->setHints(Daemon::global()->hints());
    }

    q->connect(p, SIGNAL(Destroy()),
               SLOT(destroy()));

    // Get current properties
    QDBusMessage message = QDBusMessage::createMethodCall(QLatin1String(PK_NAME),
                                                          tid.path(),
                                                          QLatin1String(DBUS_PROPERTIES),
                                                          QLatin1String("GetAll"));
    message << PK_TRANSACTION_INTERFACE;
    QDBusConnection::systemBus().callWithCallback(message,
                                                  q,
                                                  SLOT(updateProperties(QVariantMap)));

    // Watch for properties updates
    QDBusConnection::systemBus().connect(QLatin1String(PK_NAME),
                                         tid.path(),
                                         QLatin1String(DBUS_PROPERTIES),
                                         QLatin1String("PropertiesChanged"),
                                         q,
                                         SLOT(propertiesChanged(QString,QVariantMap,QStringList)));

    QStringList currentSignals = connectedSignals;
    currentSignals.removeDuplicates();
    foreach (const QString &signal, currentSignals) {
        setupSignal(signal, true);
    }
}

void TransactionPrivate::runQueuedTransaction()
{
    QDBusPendingReply<> reply;
    switch (role) {
    case Transaction::RoleAcceptEula:
        reply = p->AcceptEula(eulaId);
        break;
    case Transaction::RoleDownloadPackages:
        reply = p->DownloadPackages(storeInCache, search);
        break;
    case Transaction::RoleGetCategories:
        reply = p->GetCategories();
        break;
    case Transaction::RoleGetDepends:
        reply = p->DependsOn(filters, search, recursive);
        break;
    case Transaction::RoleGetDetails:
        reply = p->GetDetails(search);
        break;
    case Transaction::RoleGetFiles:
        reply = p->GetFiles(search);
        break;
    case Transaction::RoleGetOldTransactions:
        reply = p->GetOldTransactions(numberOfOldTransactions);
        break;
    case Transaction::RoleGetPackages:
        reply = p->GetPackages(filters);
        break;
    case Transaction::RoleGetRepoList:
        reply = p->GetRepoList(filters);
        break;
    case Transaction::RoleGetRequires:
        reply = p->RequiredBy(filters, search, recursive);
        break;
    case Transaction::RoleGetUpdateDetail:
        reply = p->GetUpdateDetail(search);
        break;
    case Transaction::RoleGetUpdates:
        reply = p->GetUpdates(filters);
        break;
    case Transaction::RoleGetDistroUpgrades:
        reply = p->GetDistroUpgrades();
        break;
    case Transaction::RoleInstallFiles:
        reply = p->InstallFiles(flags, search);
        break;
    case Transaction::RoleInstallPackages:
        reply = p->InstallPackages(flags, search);
        break;
    case Transaction::RoleInstallSignature:
        reply = p->InstallSignature(signatureType, signatureKey, signaturePackage);
        break;
    case Transaction::RoleRefreshCache:
        reply = p->RefreshCache(refreshCacheForce);
        break;
    case Transaction::RoleRemovePackages:
        reply = p->RemovePackages(flags, search, allowDeps, autoremove);
        break;
    case Transaction::RoleRepairSystem:
        reply = p->RepairSystem(flags);
        break;
    case Transaction::RoleRepoEnable:
        reply = p->RepoEnable(repoId, repoEnable);
        break;
    case Transaction::RoleRepoSetData:
        reply = p->RepoSetData(repoId, repoParameter, repoValue);
        break;
    case Transaction::RoleResolve:
        reply = p->Resolve(filters, search);
        break;
    case Transaction::RoleSearchFile:
        reply = p->SearchFiles(filters, search);
        break;
    case Transaction::RoleSearchDetails:
        reply = p->SearchDetails(filters, search);
        break;
    case Transaction::RoleSearchGroup:
        reply = p->SearchGroups(filters, search);
        break;
    case Transaction::RoleSearchName:
        reply = p->SearchNames(filters, search);
        break;
    case Transaction::RoleUpdatePackages:
        reply = p->UpdatePackages(flags, search);
        break;
    case Transaction::RoleWhatProvides:
        reply = p->WhatProvides(filters, search);
        break;
    default:
        break;
    }


}

void TransactionPrivate::createTransactionFinished(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<QDBusObjectPath> reply = *call;
    if (reply.isError()) {
        error = Transaction::InternalErrorFailed;
        errorMessage = reply.error().message();
    } else {
        QDBusObjectPath tid = reply.argumentAt<0>();
        setup(tid);
    }
    call->deleteLater();
}

void TransactionPrivate::Details(const QString &pid,
                                 const QString &license,
                                 uint group,
                                 const QString &detail,
                                 const QString &url,
                                 qulonglong size)
{
    Q_Q(Transaction);
    q->details(pid,
               license,
               static_cast<Transaction::Group>(group),
               detail,
               url,
               size);
}

void TransactionPrivate::distroUpgrade(uint type, const QString &name, const QString &description)
{
    Q_Q(Transaction);
    q->distroUpgrade(static_cast<Transaction::DistroUpgrade>(type),
                     name,
                     description);
}

void TransactionPrivate::errorCode(uint error, const QString &details)
{
    Q_Q(Transaction);
    q->errorCode(static_cast<Transaction::Error>(error), details);
}

void TransactionPrivate::mediaChangeRequired(uint mediaType, const QString &mediaId, const QString &mediaText)
{
    Q_Q(Transaction);
    q->mediaChangeRequired(static_cast<Transaction::MediaType>(mediaType),
                           mediaId,
                           mediaText);
}

void TransactionPrivate::finished(uint exitCode, uint runtime)
{
    Q_Q(Transaction);
    q->finished(static_cast<Transaction::Exit>(exitCode), runtime);
}

void TransactionPrivate::destroy()
{
    Q_Q(Transaction);
    if (p) {
       delete p;
       p = 0;
    }
    q->destroy();
}

void TransactionPrivate::daemonQuit()
{
    Q_Q(Transaction);
    if (p) {
        q->finished(Transaction::ExitFailed, 0);
        destroy();
    }
}

void TransactionPrivate::propertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidatedProperties)
{
    Q_UNUSED(interface)
    Q_UNUSED(invalidatedProperties)

    updateProperties(properties);
}

void TransactionPrivate::updateProperties(const QVariantMap &properties)
{
    Q_Q(Transaction);

    QVariantMap::ConstIterator it = properties.constBegin();
    while (it != properties.constEnd()) {
        const QString &property = it.key();
        const QVariant &value = it.value();
        if (property == QLatin1String("AllowCancel")) {
            allowCancel = value.toBool();
            q->allowCancelChanged();
        } else if (property == QLatin1String("CallerActive")) {
            callerActive = value.toBool();
            q->isCallerActiveChanged();
        } else if (property == QLatin1String("DownloadSizeRemaining")) {
            downloadSizeRemaining = value.toLongLong();
            q->downloadSizeRemainingChanged();
        } else if (property == QLatin1String("ElapsedTime")) {
            elapsedTime = value.toUInt();
            q->elapsedTimeChanged();
        } else if (property == QLatin1String("LastPackage")) {
            lastPackage = value.toString();
            q->lastPackageChanged();
        } else if (property == QLatin1String("Percentage")) {
            percentage = value.toUInt();
            q->percentageChanged();
        } else if (property == QLatin1String("RemainingTime")) {
            remainingTime = value.toUInt();
            q->remainingTimeChanged();
        } else if (property == QLatin1String("Role")) {
            role = static_cast<Transaction::Role>(value.toUInt());
            q->roleChanged();
        } else if (property == QLatin1String("Speed")) {
            speed = value.toUInt();
            q->speedChanged();
        } else if (property == QLatin1String("Status")) {
            status = static_cast<Transaction::Status>(value.toUInt());
            q->statusChanged();
        } else if (property == QLatin1String("TransactionFlags")) {
            transactionFlags = static_cast<Transaction::TransactionFlags>(value.toULongLong());
            q->transactionFlagsChanged();
        } else if (property == QLatin1String("Uid")) {
            uid = value.toUInt();
            q->uidChanged();
        } else {
            qWarning() << "Unknown Transaction property:" << property << value;
        }

        ++it;
    }

    if (!properties.isEmpty()) {
        q->changed();
    }
}

void TransactionPrivate::message(uint type, const QString &message)
{
    Q_Q(Transaction);
    q->message(static_cast<Transaction::Message>(type), message);
}

void TransactionPrivate::Package(uint info, const QString &pid, const QString &summary)
{
    Q_Q(Transaction);
    q->package(static_cast<Transaction::Info>(info),
               pid,
               summary);
}

void TransactionPrivate::ItemProgress(const QString &itemID, uint status, uint percentage)
{
    Q_Q(Transaction);
    q->itemProgress(itemID,
                    static_cast<PackageKit::Transaction::Status>(status),
                    percentage);
}

void TransactionPrivate::RepoSignatureRequired(const QString &pid,
                                               const QString &repoName,
                                               const QString &keyUrl,
                                               const QString &keyUserid,
                                               const QString &keyId,
                                               const QString &keyFingerprint,
                                               const QString &keyTimestamp,
                                               uint type)
{
    Q_Q(Transaction);
    q->repoSignatureRequired(pid,
                             repoName,
                             keyUrl,
                             keyUserid,
                             keyId,
                             keyFingerprint,
                             keyTimestamp,
                             static_cast<Transaction::SigType>(type));
}

void TransactionPrivate::requireRestart(uint type, const QString &pid)
{
    Q_Q(Transaction);
    q->requireRestart(static_cast<PackageKit::Transaction::Restart>(type), pid);
}

void TransactionPrivate::transaction(const QDBusObjectPath &oldTid,
                                     const QString &timespec,
                                     bool succeeded,
                                     uint role,
                                     uint duration,
                                     const QString &data,
                                     uint uid,
                                     const QString &cmdline)
{
    Q_Q(Transaction);
    q->transaction(new Transaction(oldTid, timespec, succeeded, static_cast<Transaction::Role>(role), duration, data, uid, cmdline, q->parent()));
}

void TransactionPrivate::UpdateDetail(const QString &package_id,
                                      const QStringList &updates,
                                      const QStringList &obsoletes,
                                      const QStringList &vendor_urls,
                                      const QStringList &bugzilla_urls,
                                      const QStringList &cve_urls,
                                      uint restart,
                                      const QString &update_text,
                                      const QString &changelog,
                                      uint state,
                                      const QString &issued,
                                      const QString &updated)
{
    Q_Q(Transaction);
    q->updateDetail(package_id,
                    updates,
                    obsoletes,
                    vendor_urls,
                    bugzilla_urls,
                    cve_urls,
                    static_cast<PackageKit::Transaction::Restart>(restart),
                    update_text,
                    changelog,
                    static_cast<PackageKit::Transaction::UpdateState>(state),
                    QDateTime::fromString(issued, Qt::ISODate),
                    QDateTime::fromString(updated, Qt::ISODate));
}
