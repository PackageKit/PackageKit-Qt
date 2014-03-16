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

#include "transaction.h"
#include "transactionprivate.h"
#include "transactionproxy.h"

#include "daemon.h"
#include "common.h"

#include <QSqlQuery>
#include <QDBusError>

#define RUN_TRANSACTION(blurb)                      \
        Q_D(Transaction);                           \
        if (init()) {                               \
            QDBusPendingReply<> r = d->p->blurb;    \
            r.waitForFinished();                    \
            if (r.isError()) {                      \
                d->error = Transaction::parseError(r.error().name()); \
                d->errorMessage = r.error().message(); \
            }                                                         \
        }                                           \

using namespace PackageKit;

Transaction::Transaction(QObject *parent) :
    QObject(parent),
    d_ptr(new TransactionPrivate(this))
{
    connect(Daemon::global(), SIGNAL(daemonQuit()), SLOT(daemonQuit()));
}

Transaction::Transaction(const QDBusObjectPath &tid, QObject *parent) :
    QObject(parent),
    d_ptr(new TransactionPrivate(this))
{
    connect(Daemon::global(), SIGNAL(daemonQuit()), SLOT(daemonQuit()));
    init(tid);
}

void Transaction::connectNotify(const char *signal)
{
    Q_D(Transaction);
    if (!d->connectedSignals.contains(signal) && d->p) {
        d->setupSignal(signal, true);
    }
    d->connectedSignals << signal;
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
void Transaction::connectNotify(const QMetaMethod &signal)
{
    // ugly but recommended way to convert a methodSignature to a SIGNAL
    connectNotify(QString("2%1")
                  .arg(QLatin1String(signal.methodSignature())).toLatin1());
}
#endif

void Transaction::disconnectNotify(const char *signal)
{
    Q_D(Transaction);
    if (d->connectedSignals.contains(signal)) {
        d->connectedSignals.removeOne(signal);
        if (d->p && !d->connectedSignals.contains(signal)) {
            d->setupSignal(signal, false);
        }
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
void Transaction::disconnectNotify(const QMetaMethod &signal)
{
    // ugly but recommended way to convert a methodSignature to a SIGNAL
    disconnectNotify(QString("2%1")
                     .arg(QLatin1String(signal.methodSignature())).toLatin1());
}
#endif

void TransactionPrivate::setupSignal(const QString &signal, bool connect)
{
    Q_Q(Transaction);

    const char *signalToConnect = 0;
    const char *memberToConnect = 0;

    if (signal == SIGNAL(changed())) {
        signalToConnect = SIGNAL(Changed());
        memberToConnect = SIGNAL(changed());
    } else if (signal == SIGNAL(category(QString,QString,QString,QString,QString))) {
        signalToConnect = SIGNAL(Category(QString,QString,QString,QString,QString));
        memberToConnect = SIGNAL(category(QString,QString,QString,QString,QString));
    } else if (signal == SIGNAL(details(QString,QString,PackageKit::Transaction::Group,QString,QString,qulonglong))) {
        signalToConnect = SIGNAL(Details(QString,QString,uint,QString,QString,qulonglong));
        memberToConnect = SLOT(Details(QString,QString,uint,QString,QString,qulonglong));
    } else if (signal == SIGNAL(distroUpgrade(PackageKit::Transaction::DistroUpgrade,QString,QString))) {
        signalToConnect = SIGNAL(DistroUpgrade(uint,QString,QString));
        memberToConnect = SLOT(distroUpgrade(uint,QString,QString));
    } else if (signal == SIGNAL(errorCode(PackageKit::Transaction::Error,QString))) {
        signalToConnect = SIGNAL(ErrorCode(uint,QString));
        memberToConnect = SLOT(errorCode(uint,QString));
    } else if (signal == SIGNAL(files(QString,QStringList))) {
        signalToConnect = SIGNAL(Files(QString,QStringList));
        memberToConnect = SIGNAL(files(QString,QStringList));
    } else if (signal == SIGNAL(finished(PackageKit::Transaction::Exit,uint))) {
        signalToConnect = SIGNAL(Finished(uint,uint));
        memberToConnect = SLOT(finished(uint,uint));
    } else if (signal == SIGNAL(message(PackageKit::Transaction::Message,QString))) {
        signalToConnect = SIGNAL(Message(uint,QString));
        memberToConnect = SLOT(message(uint,QString));
    } else if (signal == SIGNAL(package(PackageKit::Transaction::Info,QString,QString))) {
        signalToConnect = SIGNAL(Package(uint,QString,QString));
        memberToConnect = SLOT(Package(uint,QString,QString));
    } else if (signal == SIGNAL(repoDetail(QString,QString,bool))) {
        signalToConnect = SIGNAL(RepoDetail(QString,QString,bool));
        memberToConnect = SIGNAL(repoDetail(QString,QString,bool));
    } else if (signal == SIGNAL(repoSignatureRequired(QString,QString,QString,QString,QString,QString,QString,PackageKit::Transaction::SigType))) {
        signalToConnect = SIGNAL(RepoSignatureRequired(QString,QString,QString,QString,QString,QString,QString,uint));
        memberToConnect = SLOT(RepoSignatureRequired(QString,QString,QString,QString,QString,QString,QString,uint));
    } else if (signal == SIGNAL(eulaRequired(QString,QString,QString,QString))) {
        signalToConnect = SIGNAL(EulaRequired(QString,QString,QString,QString));
        memberToConnect = SIGNAL(eulaRequired(QString,QString,QString,QString));
    } else if (signal == SIGNAL(mediaChangeRequired(PackageKit::Transaction::MediaType,QString,QString))) {
        signalToConnect = SIGNAL(MediaChangeRequired(uint,QString,QString));
        memberToConnect = SLOT(mediaChangeRequired(uint,QString,QString));
    } else if (signal == SIGNAL(itemProgress(QString,PackageKit::Transaction::Status,uint))) {
        signalToConnect = SIGNAL(ItemProgress(QString,uint,uint));
        memberToConnect = SLOT(ItemProgress(QString,uint,uint));
    } else if (signal == SIGNAL(requireRestart(PackageKit::Transaction::Restart,QString))) {
        signalToConnect = SIGNAL(RequireRestart(uint,QString));
        memberToConnect = SLOT(requireRestart(uint,QString));
    } else if (signal == SIGNAL(transaction(PackageKit::Transaction*))) {
        signalToConnect = SIGNAL(Transaction(QDBusObjectPath,QString,bool,uint,uint,QString,uint,QString));
        memberToConnect = SLOT(transaction(QDBusObjectPath,QString,bool,uint,uint,QString,uint,QString));
    } else if (signal == SIGNAL(updateDetail(QString,QStringList,QStringList,QStringList,QStringList,QStringList,PackageKit::Transaction::Restart,QString,QString,PackageKit::Transaction::UpdateState,QDateTime,QDateTime))) {
        signalToConnect = SIGNAL(UpdateDetail(QString,QStringList,QStringList,QStringList,QStringList,QStringList,uint,QString,QString,uint,QString,QString));
        memberToConnect = SLOT(UpdateDetail(QString,QStringList,QStringList,QStringList,QStringList,QStringList,uint,QString,QString,uint,QString,QString));
    }

    if (signalToConnect && memberToConnect) {
        if (connect) {
            q->connect(p, signalToConnect, memberToConnect);
        } else {
            p->disconnect(signalToConnect, q, memberToConnect);
        }
    }
}

bool Transaction::init(const QDBusObjectPath &tid)
{
    Q_D(Transaction);

    if (d->p) {
        return true;
    }

    if (!tid.path().isNull()) {
        d->tid = tid;
    } else {
        d->tid = Daemon::global()->getTid();
        if (d->tid.path().isNull()) {
            d->error = Transaction::InternalErrorFailed;
            if (Daemon::global()->lastError().isValid()) {
                d->errorMessage = Daemon::global()->lastError().message();
            }
            return false;
        }
    }

    d->p = new TransactionProxy(QLatin1String(PK_NAME),
                                d->tid.path(),
                                QDBusConnection::systemBus(),
                                this);
    d->error = Transaction::InternalErrorNone;
    d->errorMessage.clear();
    if (!Daemon::global()->hints().isEmpty()) {
        setHints(Daemon::global()->hints());
    }

    connect(d->p, SIGNAL(Destroy()),
            SLOT(destroy()));

    QStringList currentSignals = d->connectedSignals;
    currentSignals.removeDuplicates();
    foreach (const QString &signal, currentSignals) {
        d->setupSignal(signal, true);
    }
    return true;
}

Transaction::Transaction(const QDBusObjectPath &tid,
                         const QString &timespec,
                         bool succeeded,
                         Role role,
                         uint duration,
                         const QString &data,
                         uint uid,
                         const QString &cmdline,
                         QObject *parent) :
    QObject(parent),
    d_ptr(new TransactionPrivate(this))
{
    Q_D(Transaction);
    d->tid = tid;
    d->timespec = QDateTime::fromString(timespec, Qt::ISODate);
    d->succeeded = succeeded;
    d->role = role;
    d->duration = duration;
    d->data = data;
    d->uid = uid;
    d->cmdline = cmdline;
    d->error = InternalErrorNone;
}

Transaction::~Transaction()
{
    Q_D(Transaction);
//     qDebug() << "Destroying transaction with tid" << d->tid;
    delete d;
}

void Transaction::reset()
{
    Q_D(Transaction);
    d->destroy();
}

QDBusObjectPath Transaction::tid() const
{
    Q_D(const Transaction);
    return d->tid;
}

Transaction::InternalError Transaction::error() const
{
    Q_D(const Transaction);
    return d->error;
}

Transaction::InternalError Transaction::internalError() const
{
    Q_D(const Transaction);
    return d->error;
}

QString Transaction::internalErrorMessage() const
{
    Q_D(const Transaction);
    return d->errorMessage;
}

bool Transaction::allowCancel() const
{
    Q_D(const Transaction);
    if (d->p) {
        return d->p->allowCancel();
    }
    return false;
}

bool Transaction::isCallerActive() const
{
    Q_D(const Transaction);
    if (d->p) {
        return d->p->callerActive();
    }
    return false;
}

void Transaction::cancel()
{
    Q_D(const Transaction);
    if (d->p) {
        RUN_TRANSACTION(Cancel())
    }
}

QString Transaction::packageName(const QString &packageID)
{
    return packageID.section(QLatin1Char(';'), 0, 0);
}

QString Transaction::packageVersion(const QString &packageID)
{
    return packageID.section(QLatin1Char(';'), 1, 1);
}

QString Transaction::packageArch(const QString &packageID)
{
    return packageID.section(QLatin1Char(';'), 2, 2);
}

QString Transaction::packageData(const QString &packageID)
{
    return packageID.section(QLatin1Char(';'), 3, 3);
}

QString Transaction::packageIcon(const QString &packageID)
{
    QString path;
    QSqlDatabase db = QSqlDatabase::database(PK_DESKTOP_DEFAULT_DATABASE);
    if (!db.isOpen()) {
        qDebug() << "Desktop files database is not open";
        return path;
    }

    QSqlQuery q(db);
    q.prepare("SELECT filename FROM cache WHERE package = :name");
    q.bindValue(":name", Transaction::packageName(packageID));
    if (q.exec()) {
        if (q.next()) {
            QFile desktopFile(q.value(0).toString());
            if (desktopFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                while (!desktopFile.atEnd()) {
                    QByteArray line = desktopFile.readLine().trimmed();
                    if (line.startsWith("Icon=")) {
                        path = line.mid(5);
                        break;
                    }
                }
                desktopFile.close();
            } else {
                qDebug() << "Cannot open desktop file " << q.value(0).toString();
            }
        }
    } else {
        qDebug() << "Error while running query " << q.executedQuery();
    }

    return path;
}

QString Transaction::lastPackage() const
{
    Q_D(const Transaction);
    if (d->p) {
        return d->p->lastPackage();
    }
    return QString();
}

uint Transaction::percentage() const
{
    Q_D(const Transaction);
    if (d->p) {
        return d->p->percentage();
    }
    return 0;
}

uint Transaction::elapsedTime() const
{
    Q_D(const Transaction);
    if (d->p) {
        return d->p->elapsedTime();
    }
    return 0;
}

uint Transaction::remainingTime() const
{
    Q_D(const Transaction);
    if (d->p) {
        return d->p->remainingTime();
    }
    return 0;
}

uint Transaction::speed() const
{
    Q_D(const Transaction);
    if (d->p) {
        return d->p->speed();
    }
    return 0;
}

qulonglong Transaction::downloadSizeRemaining() const
{
    Q_D(const Transaction);
    if (d->p) {
        return d->p->downloadSizeRemaining();
    }
    return 0;
}    

Transaction::Role Transaction::role() const
{
    Q_D(const Transaction);
    if (d->p) {
        return static_cast<Transaction::Role>(d->p->role());
    }
    return d->role;
}

void Transaction::setHints(const QStringList &hints)
{
    Q_D(Transaction);
    if (d->p) {
        d->p->SetHints(hints);
    }
}

void Transaction::setHints(const QString &hints)
{
    setHints(QStringList() << hints);
}

Transaction::Status Transaction::status() const
{
    Q_D(const Transaction);
    if (d->p) {
        return static_cast<Transaction::Status>(d->p->status());
    }
    return Transaction::StatusUnknown;
}

Transaction::TransactionFlags Transaction::transactionFlags() const
{
    Q_D(const Transaction);
    if (d->p) {
        return static_cast<Transaction::TransactionFlags>(d->p->transactionFlags());
    }
    return Transaction::TransactionFlagNone;
}

QDateTime Transaction::timespec() const
{
    Q_D(const Transaction);
    return d->timespec;
}

bool Transaction::succeeded() const
{
    Q_D(const Transaction);
    return d->succeeded;
}

uint Transaction::duration() const
{
    Q_D(const Transaction);
    return d->duration;
}

QString Transaction::data() const
{
    Q_D(const Transaction);
    return d->data;
}

uint Transaction::uid() const
{
    Q_D(const Transaction);
    if(d->p) {
        return d->p->uid();
    }
    return d->uid;
}

QString Transaction::cmdline() const
{
    Q_D(const Transaction);
    return d->cmdline;
}

void Transaction::acceptEula(const QString &eulaId)
{
    RUN_TRANSACTION(AcceptEula(eulaId))
}

void Transaction::downloadPackages(const QStringList &packageIDs, bool storeInCache)
{
    RUN_TRANSACTION(DownloadPackages(storeInCache, packageIDs))
}

void Transaction::downloadPackage(const QString &packageID, bool storeInCache)
{
    downloadPackages(QStringList() << packageID, storeInCache);
}

void Transaction::getCategories()
{
    RUN_TRANSACTION(GetCategories())
}

void Transaction::getDepends(const QStringList &packageIDs, Transaction::Filters filters, bool recursive)
{
    RUN_TRANSACTION(GetDepends(filters, packageIDs, recursive))
}

void Transaction::getDepends(const QString &packageID, Transaction::Filters filters, bool recursive)
{
    getDepends(QStringList() << packageID, filters, recursive);
}

void Transaction::getDetails(const QStringList &packageIDs)
{
    RUN_TRANSACTION(GetDetails(packageIDs))
}

void Transaction::getDetails(const QString &packageID)
{
    getDetails(QStringList() << packageID);
}

void Transaction::getFiles(const QStringList &packageIDs)
{
    RUN_TRANSACTION(GetFiles(packageIDs))
}

void Transaction::getFiles(const QString &packageID)
{
    getFiles(QStringList() << packageID);
}

void Transaction::getOldTransactions(uint number)
{
    RUN_TRANSACTION(GetOldTransactions(number))
}

void Transaction::getPackages(Transaction::Filters filters)
{
    RUN_TRANSACTION(GetPackages(filters))
}

void Transaction::getRepoList(Transaction::Filters filters)
{
    RUN_TRANSACTION(GetRepoList(filters))
}

void Transaction::getRequires(const QStringList &packageIDs, Transaction::Filters filters, bool recursive)
{
    RUN_TRANSACTION(GetRequires(filters, packageIDs, recursive))
}

void Transaction::getRequires(const QString &packageID, Transaction::Filters filters, bool recursive)
{
    getRequires(QStringList() << packageID, filters, recursive);
}

void Transaction::getUpdatesDetails(const QStringList &packageIDs)
{
    RUN_TRANSACTION(GetUpdateDetail(packageIDs))
}

void Transaction::getUpdateDetail(const QString &packageID)
{
    getUpdatesDetails(QStringList() << packageID);
}

void Transaction::getUpdates(Transaction::Filters filters)
{
    RUN_TRANSACTION(GetUpdates(filters))
}

void Transaction::getDistroUpgrades()
{
    RUN_TRANSACTION(GetDistroUpgrades())
}

void Transaction::installFiles(const QStringList &files, TransactionFlags flags)
{
    RUN_TRANSACTION(InstallFiles(flags, files))
}

void Transaction::installFile(const QString &file, TransactionFlags flags)
{
    installFiles(QStringList() << file, flags);
}

void Transaction::installPackages(const QStringList &packageIDs, TransactionFlags flags)
{
    RUN_TRANSACTION(InstallPackages(flags, packageIDs))
}

void Transaction::installPackage(const QString &packageID, TransactionFlags flags)
{
    installPackages(QStringList() << packageID, flags);
}

void Transaction::installSignature(SigType type, const QString &keyID, const QString &packageID)
{
    RUN_TRANSACTION(InstallSignature(type, keyID, packageID))
}

void Transaction::refreshCache(bool force)
{
    RUN_TRANSACTION(RefreshCache(force))
}

void Transaction::removePackages(const QStringList &packageIDs, bool allowDeps, bool autoremove, TransactionFlags flags)
{
    RUN_TRANSACTION(RemovePackages(flags, packageIDs, allowDeps, autoremove))
}

void Transaction::removePackage(const QString &packageID, bool allowDeps, bool autoremove, TransactionFlags flags)
{
    removePackages(QStringList() << packageID, allowDeps, autoremove, flags);
}

void Transaction::repairSystem(TransactionFlags flags)
{
    RUN_TRANSACTION(RepairSystem(flags))
}

void Transaction::repoEnable(const QString &repoId, bool enable)
{
    RUN_TRANSACTION(RepoEnable(repoId, enable))
}

void Transaction::repoSetData(const QString &repoId, const QString &parameter, const QString &value)
{
    RUN_TRANSACTION(RepoSetData(repoId, parameter, value))
}

void Transaction::resolve(const QStringList &packageNames, Transaction::Filters filters)
{
    RUN_TRANSACTION(Resolve(filters, packageNames))
}

void Transaction::resolve(const QString &packageName, Transaction::Filters filters)
{
    resolve(QStringList() << packageName, filters);
}

void Transaction::searchFiles(const QStringList &search, Transaction::Filters filters)
{
    RUN_TRANSACTION(SearchFiles(filters, search))
}

void Transaction::searchFiles(const QString &search, Transaction::Filters filters)
{
    searchFiles(QStringList() << search, filters);
}

void Transaction::searchDetails(const QStringList &search, Transaction::Filters filters)
{
    RUN_TRANSACTION(SearchDetails(filters, search))
}

void Transaction::searchDetails(const QString &search, Transaction::Filters filters)
{
    searchDetails(QStringList() << search, filters);
}

void Transaction::searchGroups(const QStringList &groups, Transaction::Filters filters)
{
    RUN_TRANSACTION(SearchGroups(filters, groups))
}

void Transaction::searchGroup(const QString &group, Transaction::Filters filters)
{
    searchGroups(QStringList() << group, filters);
}

void Transaction::searchGroup(Group group, Filters filters)
{
    QString groupString = Daemon::enumToString<Transaction>(group, "Group");
    searchGroup(groupString, filters);
}

void Transaction::searchGroups(Groups groups, Transaction::Filters filters)
{
    QStringList groupsStringList;
    for (int i = 1; i < 64; ++i) {
        if (groups & i) {
            Transaction::Group group = static_cast<Transaction::Group>(i);
            if (group != Transaction::GroupUnknown) {
                groupsStringList << Daemon::enumToString<Transaction>(group, "Group");
            }
        }
    }
    searchGroups(groupsStringList, filters);
}

void Transaction::searchNames(const QStringList &search, Transaction::Filters filters)
{
    RUN_TRANSACTION(SearchNames(filters, search))
}

void Transaction::searchNames(const QString &search, Transaction::Filters filters)
{
    searchNames(QStringList() << search, filters);
}

void Transaction::updatePackages(const QStringList &packageIDs, TransactionFlags flags)
{
    RUN_TRANSACTION(UpdatePackages(flags, packageIDs))
}

void Transaction::updatePackage(const QString &packageID, TransactionFlags flags)
{
    updatePackages(QStringList() << packageID, flags);
}

void Transaction::upgradeSystem(const QString &distroId, UpgradeKind kind)
{
    RUN_TRANSACTION(UpgradeSystem(distroId, kind))
}

void Transaction::whatProvides(Transaction::Provides type, const QStringList &search, Transaction::Filters filters)
{
    RUN_TRANSACTION(WhatProvides(filters, type, search))
}

void Transaction::whatProvides(Transaction::Provides type, const QString &search, Transaction::Filters filters)
{
    whatProvides(type, QStringList() << search, filters);
}

Transaction::InternalError Transaction::parseError(const QString &errorName)
{
    QString error = errorName;
    if (error.startsWith(QLatin1String("org.freedesktop.packagekit."))) {
        return Transaction::InternalErrorFailedAuth;
    }
    
    error.remove(QLatin1String("org.freedesktop.PackageKit.Transaction."));
    
    if (error.startsWith(QLatin1String("PermissionDenied")) ||
        error.startsWith(QLatin1String("RefusedByPolicy"))) {
        return Transaction::InternalErrorFailedAuth;
    }

    if (error.startsWith(QLatin1String("PackageIdInvalid")) ||
        error.startsWith(QLatin1String("SearchInvalid")) ||
        error.startsWith(QLatin1String("FilterInvalid")) ||
        error.startsWith(QLatin1String("InvalidProvide")) ||
        error.startsWith(QLatin1String("InputInvalid"))) {
        return Transaction::InternalErrorInvalidInput;
    }

    if (error.startsWith(QLatin1String("PackInvalid")) ||
        error.startsWith(QLatin1String("NoSuchFile")) ||
        error.startsWith(QLatin1String("NoSuchDirectory"))) {
        return Transaction::InternalErrorInvalidFile;
    }

    if (error.startsWith(QLatin1String("NotSupported"))) {
        return Transaction::InternalErrorFunctionNotSupported;
    }

    qWarning() << "Transaction::parseError: unknown error" << errorName;
    return Transaction::InternalErrorFailed;
}

#include "transaction.moc"

