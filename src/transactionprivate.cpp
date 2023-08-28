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

#include "transactionprivate.h"
#include "daemonprivate.h"

#include "daemon.h"
#include "common.h"
#include "details.h"

#include <QStringList>

using namespace PackageKit;

TransactionPrivate::TransactionPrivate(Transaction* parent)
    : q_ptr(parent)
{
}

TransactionPrivate::~TransactionPrivate()
{
    delete p;
}

void TransactionPrivate::setup(const QDBusObjectPath &transactionId)
{
    Q_Q(Transaction);

    tid = transactionId;
    p = new OrgFreedesktopPackageKitTransactionInterface(PK_NAME,
                                                         tid.path(),
                                                         QDBusConnection::systemBus(),
                                                         q);
    QStringList hints = this->hints ? *this->hints : Daemon::global()->hints();
    hints << QStringLiteral("supports-plural-signals=true");
    q->setHints(hints);

    q->connect(p, SIGNAL(Destroy()), SLOT(destroy()));

    // Get current properties
    QDBusMessage message = QDBusMessage::createMethodCall(PK_NAME,
                                                          tid.path(),
                                                          DBUS_PROPERTIES,
                                                          QLatin1String("GetAll"));
    message << PK_TRANSACTION_INTERFACE;
    QDBusConnection::systemBus().callWithCallback(message,
                                                  q,
                                                  SLOT(updateProperties(QVariantMap)));

    // Watch for properties updates
    QDBusConnection::systemBus().connect(PK_NAME,
                                         tid.path(),
                                         DBUS_PROPERTIES,
                                         QLatin1String("PropertiesChanged"),
                                         q,
                                         SLOT(propertiesChanged(QString,QVariantMap,QStringList)));

    const QVector<QMetaMethod> signals = connectedSignals;
    for (const QMetaMethod &signal : signals) {
        setupSignal(signal);
    }

    // Execute pending call
    runQueuedTransaction();
}

void TransactionPrivate::runQueuedTransaction()
{
    Q_Q(Transaction);

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
    case Transaction::RoleDependsOn:
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
    case Transaction::RoleRequiredBy:
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
        reply = p->InstallFiles(transactionFlags, search);
        break;
    case Transaction::RoleInstallPackages:
        reply = p->InstallPackages(transactionFlags, search);
        break;
    case Transaction::RoleInstallSignature:
        reply = p->InstallSignature(signatureType, signatureKey, signaturePackage);
        break;
    case Transaction::RoleRefreshCache:
        reply = p->RefreshCache(refreshCacheForce);
        break;
    case Transaction::RoleRemovePackages:
        reply = p->RemovePackages(transactionFlags, search, allowDeps, autoremove);
        break;
    case Transaction::RoleRepairSystem:
        reply = p->RepairSystem(transactionFlags);
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
        reply = p->UpdatePackages(transactionFlags, search);
        break;
    case Transaction::RoleWhatProvides:
        reply = p->WhatProvides(filters, search);
        break;
    case Transaction::RoleGetDetailsLocal:
        reply = p->GetDetailsLocal(search);
        break;
    case Transaction::RoleGetFilesLocal:
        reply = p->GetFilesLocal(search);
        break;
    case Transaction::RoleRepoRemove:
        reply = p->RepoRemove(transactionFlags, repoId, autoremove);
        break;
    case Transaction::RoleUpgradeSystem:
        reply = p->UpgradeSystem(transactionFlags, upgradeDistroId, upgradeKind);
        break;
    default:
        return;
    }

    if (reply.isFinished() && reply.isError()) {
        q->errorCode(Transaction::ErrorInternalError, reply.error().message());
        finished(Transaction::ExitFailed, 0);
        return;
    }
    auto watcher = new QDBusPendingCallWatcher(reply, q);
    q->connect(watcher, &QDBusPendingCallWatcher::finished,
               q, [this, q] (QDBusPendingCallWatcher *call) {
        QDBusPendingReply<> reply = *call;
        if (reply.isError()) {
            QDBusError error = reply.error();
            Transaction::Error transactionError = error.type() == QDBusError::AccessDenied ? Transaction::ErrorNotAuthorized
                                                                                           : Transaction::ErrorInternalError;
            q->errorCode(transactionError, error.message());
            finished(Transaction::ExitFailed, 0);
            destroy();
        }
        call->deleteLater();
    });
}

void TransactionPrivate::details(const QVariantMap &values)
{
    Q_Q(Transaction);
    q->details(PackageKit::Details(values));
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
    sentFinished = true;
    q->deleteLater();
}

void TransactionPrivate::destroy()
{
    Q_Q(Transaction);
    if (p) {
       delete p;
       p = nullptr;
    }

    if (!sentFinished) {
       // If after we connect to a transaction we happend
       // to only receive destroyed signal send a finished
       // to the client
       q->finished(Transaction::ExitUnknown, 0);
    }

    q->deleteLater();
}

void TransactionPrivate::daemonQuit()
{
    Q_Q(Transaction);
    if (p) {
        q->errorCode(Transaction::ErrorProcessKill, QObject::tr("The PackageKit daemon has crashed"));
        finished(Transaction::ExitKilled, 0);
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
            QMetaObject::invokeMethod(q, &Transaction::allowCancelChanged, Qt::QueuedConnection);
        } else if (property == QLatin1String("CallerActive")) {
            callerActive = value.toBool();
            QMetaObject::invokeMethod(q, &Transaction::isCallerActiveChanged, Qt::QueuedConnection);
        } else if (property == QLatin1String("DownloadSizeRemaining")) {
            downloadSizeRemaining = value.toLongLong();
            QMetaObject::invokeMethod(q, &Transaction::downloadSizeRemainingChanged, Qt::QueuedConnection);
        } else if (property == QLatin1String("ElapsedTime")) {
            elapsedTime = value.toUInt();
            QMetaObject::invokeMethod(q, &Transaction::elapsedTimeChanged, Qt::QueuedConnection);
        } else if (property == QLatin1String("LastPackage")) {
            lastPackage = value.toString();
            QMetaObject::invokeMethod(q, &Transaction::lastPackageChanged, Qt::QueuedConnection);
        } else if (property == QLatin1String("Percentage")) {
            percentage = value.toUInt();
            QMetaObject::invokeMethod(q, &Transaction::percentageChanged, Qt::QueuedConnection);
        } else if (property == QLatin1String("RemainingTime")) {
            remainingTime = value.toUInt();
            q->remainingTimeChanged();
        } else if (property == QLatin1String("Role")) {
            role = static_cast<Transaction::Role>(value.toUInt());
            QMetaObject::invokeMethod(q, &Transaction::roleChanged, Qt::QueuedConnection);
        } else if (property == QLatin1String("Speed")) {
            speed = value.toUInt();
            QMetaObject::invokeMethod(q, &Transaction::speedChanged, Qt::QueuedConnection);
        } else if (property == QLatin1String("Status")) {
            status = static_cast<Transaction::Status>(value.toUInt());
            QMetaObject::invokeMethod(q, &Transaction::statusChanged, Qt::QueuedConnection);
        } else if (property == QLatin1String("TransactionFlags")) {
            transactionFlags = static_cast<Transaction::TransactionFlags>(value.toUInt());
            QMetaObject::invokeMethod(q, &Transaction::transactionFlagsChanged, Qt::QueuedConnection);
        } else if (property == QLatin1String("Uid")) {
            uid = value.toUInt();
            QMetaObject::invokeMethod(q, &Transaction::uidChanged, Qt::QueuedConnection);
        } else if (property == QLatin1String("Sender")) {
            senderName = value.toString();
            QMetaObject::invokeMethod(q, &Transaction::senderNameChanged, Qt::QueuedConnection);
        } else {
            qCWarning(PACKAGEKITQT_TRANSACTION) << "Unknown Transaction property:" << property << value;
        }

        ++it;
    }
}

void TransactionPrivate::Package(uint info, const QString &pid, const QString &summary)
{
    Q_Q(Transaction);
    q->package(static_cast<Transaction::Info>(info),
               pid,
               summary);
}

void TransactionPrivate::Packages(const QList<PackageKit::PkPackage> &pkgs)
{
    Q_Q(Transaction);
    for (PkPackage const &pkg : pkgs) {
        q->package(static_cast<Transaction::Info>(pkg.info), pkg.pid, pkg.summary);
    }
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
                                     const QString &senderName,
                                     const QString &cmdline)
{
    Q_Q(Transaction);

    auto priv = new TransactionPrivate(q);
    priv->tid = tid;
    priv->timespec = QDateTime::fromString(timespec, Qt::ISODate);
    priv->succeeded = succeeded;
    priv->role = static_cast<Transaction::Role>(role);
    priv->duration = duration;
    priv->data = data;
    priv->uid = uid;
    priv->senderName = senderName;
    priv->cmdline = cmdline;

    auto transaction = new Transaction(priv);
    priv->q_ptr = transaction;

    q->transaction(transaction);
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

void TransactionPrivate::UpdateDetails(const QList<PkDetail> &details)
{
    Q_Q(Transaction);
    for (const PkDetail &detail : details) {
        q->updateDetail(detail.package_id,
                        detail.updates,
                        detail.obsoletes,
                        detail.vendor_urls,
                        detail.bugzilla_urls,
                        detail.cve_urls,
                        static_cast<PackageKit::Transaction::Restart>(detail.restart),
                        detail.update_text,
                        detail.changelog,
                        static_cast<PackageKit::Transaction::UpdateState>(detail.state),
                        QDateTime::fromString(detail.issued, Qt::ISODate),
                        QDateTime::fromString(detail.updated, Qt::ISODate));
    }
}
