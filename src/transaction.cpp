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
        d->setup(tid);
    } else {
        QDBusPendingReply<QDBusObjectPath> reply = Daemon::global()->createTransaction();
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
        QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                         this, SLOT(createTransactionFinished(QDBusPendingCallWatcher*)));
    }

    return false;
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
    return d->allowCancel;
}

bool Transaction::isCallerActive() const
{
    Q_D(const Transaction);
    return d->callerActive;
}

void Transaction::cancel()
{
    Q_D(const Transaction);
    if (d->p) {
        d->p->Cancel();
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
    return d->lastPackage;
}

uint Transaction::percentage() const
{
    Q_D(const Transaction);
    return d->percentage;
}

uint Transaction::elapsedTime() const
{
    Q_D(const Transaction);
    return d->elapsedTime;
}

uint Transaction::remainingTime() const
{
    Q_D(const Transaction);
    return d->remainingTime;
}

uint Transaction::speed() const
{
    Q_D(const Transaction);
    return d->speed;
}

qulonglong Transaction::downloadSizeRemaining() const
{
    Q_D(const Transaction);
    return d->downloadSizeRemaining;
}    

Transaction::Role Transaction::role() const
{
    Q_D(const Transaction);
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
    return d->status;
}

Transaction::TransactionFlags Transaction::transactionFlags() const
{
    Q_D(const Transaction);
    return d->transactionFlags;
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
    return d->uid;
}

QString Transaction::cmdline() const
{
    Q_D(const Transaction);
    return d->cmdline;
}

void Transaction::acceptEula(const QString &eulaId)
{
    Q_D(Transaction);
    d->role = Transaction::RoleAcceptEula;
    d->eulaId = eulaId;
    if (init()) {
        d->runQueuedTransaction();
    }
}

void Transaction::downloadPackages(const QStringList &packageIDs, bool storeInCache)
{
    Q_D(Transaction);
    d->role = Transaction::RoleDownloadPackages;
    d->search = packageIDs;
    d->storeInCache = storeInCache;
    if (init()) {
        d->runQueuedTransaction();
    }
}

void Transaction::downloadPackage(const QString &packageID, bool storeInCache)
{
    downloadPackages(QStringList() << packageID, storeInCache);
}

void Transaction::getCategories()
{
    Q_D(Transaction);
    d->role = Transaction::RoleGetCategories;
    if (init()) {
        d->runQueuedTransaction();
    }
}

void Transaction::getDepends(const QStringList &packageIDs, Transaction::Filters filters, bool recursive)
{
    Q_D(Transaction);
    d->role = Transaction::RoleGetDepends;
    d->search = packageIDs;
    d->filters = filters;
    d->recursive = recursive;
    if (init()) {
        d->runQueuedTransaction();
    }
}

void Transaction::getDepends(const QString &packageID, Transaction::Filters filters, bool recursive)
{
    getDepends(QStringList() << packageID, filters, recursive);
}

void Transaction::getDetails(const QStringList &packageIDs)
{
    Q_D(Transaction);
    d->role = Transaction::RoleGetDetails;
    d->search = packageIDs;
    if (init()) {
        d->runQueuedTransaction();
    }
}

void Transaction::getDetails(const QString &packageID)
{
    getDetails(QStringList() << packageID);
}

void Transaction::getFiles(const QStringList &packageIDs)
{
    Q_D(Transaction);
    d->role = Transaction::RoleGetFiles;
    d->search = packageIDs;
    if (init()) {
        d->runQueuedTransaction();
    }
}

void Transaction::getFiles(const QString &packageID)
{
    getFiles(QStringList() << packageID);
}

void Transaction::getOldTransactions(uint number)
{
    Q_D(Transaction);
    d->role = Transaction::RoleGetOldTransactions;
    d->numberOfOldTransactions = number;
    if (init()) {
        d->runQueuedTransaction();
    }
}

void Transaction::getPackages(Transaction::Filters filters)
{
    Q_D(Transaction);
    d->role = Transaction::RoleGetPackages;
    d->filters = filters;
    if (init()) {
        d->runQueuedTransaction();
    }
}

void Transaction::getRepoList(Transaction::Filters filters)
{
    Q_D(Transaction);
    d->role = Transaction::RoleGetRepoList;
    d->filters = filters;
    if (init()) {
        d->runQueuedTransaction();
    }
}

void Transaction::getRequires(const QStringList &packageIDs, Transaction::Filters filters, bool recursive)
{
    Q_D(Transaction);
    d->role = Transaction::RoleGetRequires;
    d->search = packageIDs;
    d->filters = filters;
    d->recursive = recursive;
    if (init()) {
        d->runQueuedTransaction();
    }
}

void Transaction::getRequires(const QString &packageID, Transaction::Filters filters, bool recursive)
{
    getRequires(QStringList() << packageID, filters, recursive);
}

void Transaction::getUpdatesDetails(const QStringList &packageIDs)
{
    Q_D(Transaction);
    d->role = Transaction::RoleGetUpdateDetail;
    d->search = packageIDs;
    if (init()) {
        d->runQueuedTransaction();
    }
}

void Transaction::getUpdateDetail(const QString &packageID)
{
    getUpdatesDetails(QStringList() << packageID);
}

void Transaction::getUpdates(Transaction::Filters filters)
{
    Q_D(Transaction);
    d->role = Transaction::RoleGetUpdates;
    d->filters = filters;
    if (init()) {
        d->runQueuedTransaction();
    }
}

void Transaction::getDistroUpgrades()
{
    Q_D(Transaction);
    d->role = Transaction::RoleGetDistroUpgrades;
    if (init()) {
        d->runQueuedTransaction();
    }
}

void Transaction::installFiles(const QStringList &files, TransactionFlags flags)
{
    Q_D(Transaction);
    d->role = Transaction::RoleInstallFiles;
    d->search = files;
    d->flags = flags;
    if (init()) {
        d->runQueuedTransaction();
    }
}

void Transaction::installFile(const QString &file, TransactionFlags flags)
{
    installFiles(QStringList() << file, flags);
}

void Transaction::installPackages(const QStringList &packageIDs, TransactionFlags flags)
{
    Q_D(Transaction);
    d->role = Transaction::RoleInstallPackages;
    d->search = packageIDs;
    d->flags = flags;
    if (init()) {
        d->runQueuedTransaction();
    }
}

void Transaction::installPackage(const QString &packageID, TransactionFlags flags)
{
    installPackages(QStringList() << packageID, flags);
}

void Transaction::installSignature(SigType type, const QString &keyID, const QString &packageID)
{
    Q_D(Transaction);
    d->role = Transaction::RoleInstallSignature;
    d->signatureType = type;
    d->signatureKey = keyID;
    d->signaturePackage = packageID;
    if (init()) {
        d->runQueuedTransaction();
    }
}

void Transaction::refreshCache(bool force)
{
    Q_D(Transaction);
    d->role = Transaction::RoleRefreshCache;
    d->refreshCacheForce = force;
    if (init()) {
        d->runQueuedTransaction();
    }
}

void Transaction::removePackages(const QStringList &packageIDs, bool allowDeps, bool autoremove, TransactionFlags flags)
{
    Q_D(Transaction);
    d->role = Transaction::RoleRemovePackages;
    d->search = packageIDs;
    d->allowDeps = allowDeps;
    d->autoremove = autoremove;
    d->flags = flags;
    if (init()) {
        d->runQueuedTransaction();
    }
}

void Transaction::removePackage(const QString &packageID, bool allowDeps, bool autoremove, TransactionFlags flags)
{
    removePackages(QStringList() << packageID, allowDeps, autoremove, flags);
}

void Transaction::repairSystem(TransactionFlags flags)
{
    Q_D(Transaction);
    d->role = Transaction::RoleRepairSystem;
    d->flags = flags;
    if (init()) {
        d->runQueuedTransaction();
    }
}

void Transaction::repoEnable(const QString &repoId, bool enable)
{
    Q_D(Transaction);
    d->role = Transaction::RoleRepoEnable;
    d->repoId = repoId;
    d->repoEnable = enable;
    if (init()) {
        d->runQueuedTransaction();
    }
}

void Transaction::repoSetData(const QString &repoId, const QString &parameter, const QString &value)
{
    Q_D(Transaction);
    d->role = Transaction::RoleRepoSetData;
    d->repoId = repoId;
    d->repoParameter = parameter;
    d->repoValue = value;
    if (init()) {
        d->runQueuedTransaction();
    }
}

void Transaction::resolve(const QStringList &packageNames, Transaction::Filters filters)
{
    Q_D(Transaction);
    d->role = Transaction::RoleResolve;
    d->search = packageNames;
    d->filters = filters;
    if (init()) {
        d->runQueuedTransaction();
    }
}

void Transaction::resolve(const QString &packageName, Transaction::Filters filters)
{
    resolve(QStringList() << packageName, filters);
}

void Transaction::searchFiles(const QStringList &search, Transaction::Filters filters)
{
    Q_D(Transaction);
    d->role = Transaction::RoleSearchFile;
    d->search = search;
    d->filters = filters;
    if (init()) {
        d->runQueuedTransaction();
    }
}

void Transaction::searchFiles(const QString &search, Transaction::Filters filters)
{
    searchFiles(QStringList() << search, filters);
}

void Transaction::searchDetails(const QStringList &search, Transaction::Filters filters)
{
    Q_D(Transaction);
    d->role = Transaction::RoleSearchDetails;
    d->search = search;
    d->filters = filters;
    if (init()) {
        d->runQueuedTransaction();
    }
}

void Transaction::searchDetails(const QString &search, Transaction::Filters filters)
{
    searchDetails(QStringList() << search, filters);
}

void Transaction::searchGroups(const QStringList &groups, Transaction::Filters filters)
{
    Q_D(Transaction);
    d->role = Transaction::RoleSearchGroup;
    d->search = groups;
    d->filters = filters;
    if (init()) {
        d->runQueuedTransaction();
    }
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
    Q_D(Transaction);
    d->role = Transaction::RoleSearchName;
    d->search = search;
    d->filters = filters;
    if (init()) {
        d->runQueuedTransaction();
    }
}

void Transaction::searchNames(const QString &search, Transaction::Filters filters)
{
    searchNames(QStringList() << search, filters);
}

void Transaction::updatePackages(const QStringList &packageIDs, TransactionFlags flags)
{
    Q_D(Transaction);
    d->role = Transaction::RoleUpdatePackages;
    d->search = packageIDs;
    d->flags = flags;
    if (init()) {
        d->runQueuedTransaction();
    }
}

void Transaction::updatePackage(const QString &packageID, TransactionFlags flags)
{
    updatePackages(QStringList() << packageID, flags);
}

void Transaction::whatProvides(const QStringList &search, Transaction::Filters filters)
{
    Q_D(Transaction);
    d->role = Transaction::RoleWhatProvides;
    d->search = search;
    d->filters = filters;
    if (init()) {
        d->runQueuedTransaction();
    }
}

void Transaction::whatProvides(const QString &search, Transaction::Filters filters)
{
    whatProvides(QStringList() << search, filters);
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

