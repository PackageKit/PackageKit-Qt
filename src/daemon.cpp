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

#include <QtSql>
#include <QDebug>

#include "daemon.h"
#include "daemonprivate.h"
#include "transactionprivate.h"
#include "daemonproxy.h"

#include "common.h"

using namespace PackageKit;

Daemon* Daemon::m_global = 0;

Daemon* Daemon::global()
{
    if(!m_global) {
        m_global = new Daemon(qApp);
    }

    return m_global;
}

Daemon::Daemon(QObject *parent) :
    QObject(parent),
    d_ptr(new DaemonPrivate(this))
{
    Q_D(Daemon);
    d->daemon = new ::DaemonProxy(QLatin1String(PK_NAME),
                                  QLatin1String(PK_PATH),
                                  QDBusConnection::systemBus(),
                                  this);

    QDBusConnection::systemBus().connect(QLatin1String(PK_NAME),
                                         QLatin1String(PK_PATH),
                                         QLatin1String(DBUS_PROPERTIES),
                                         QLatin1String("PropertiesChanged"),
                                         this,
                                         SLOT(propertiesChanged(QString,QVariantMap,QStringList)));

    // Set up database for desktop files
    QSqlDatabase db;
    db = QSqlDatabase::addDatabase("QSQLITE", PK_DESKTOP_DEFAULT_DATABASE);
    db.setDatabaseName(PK_DESKTOP_DEFAULT_DATABASE);
    if (!db.open()) {
        qDebug() << "Failed to initialize the desktop files database";
    }

    qRegisterMetaType<PackageKit::Daemon::Network>("PackageKit::Daemon::Network");
    qRegisterMetaType<PackageKit::Daemon::Authorize>("PackageKit::Daemon::Authorize");
    qRegisterMetaType<PackageKit::Transaction::InternalError>("PackageKit::Transaction::InternalError");
    qRegisterMetaType<PackageKit::Transaction::Role>("PackageKit::Transaction::Role");
    qRegisterMetaType<PackageKit::Transaction::Error>("PackageKit::Transaction::Error");
    qRegisterMetaType<PackageKit::Transaction::Exit>("PackageKit::Transaction::Exit");
    qRegisterMetaType<PackageKit::Transaction::Filter>("PackageKit::Transaction::Filter");
    qRegisterMetaType<PackageKit::Transaction::Message>("PackageKit::Transaction::Message");
    qRegisterMetaType<PackageKit::Transaction::Status>("PackageKit::Transaction::Status");
    qRegisterMetaType<PackageKit::Transaction::MediaType>("PackageKit::Transaction::MediaType");
    qRegisterMetaType<PackageKit::Transaction::Provides>("PackageKit::Transaction::Provides");
    qRegisterMetaType<PackageKit::Transaction::ProvidesFlag>("PackageKit::Transaction::ProvidesFlag");
    qRegisterMetaType<PackageKit::Transaction::DistroUpgrade>("PackageKit::Transaction::DistroUpgrade");
    qRegisterMetaType<PackageKit::Transaction::TransactionFlag>("PackageKit::Transaction::TransactionFlag");
    qRegisterMetaType<PackageKit::Transaction::TransactionFlags>("PackageKit::Transaction::TransactionFlags");
    qRegisterMetaType<PackageKit::Transaction::Restart>("PackageKit::Transaction::Restart");
    qRegisterMetaType<PackageKit::Transaction::UpdateState>("PackageKit::Transaction::UpdateState");
    qRegisterMetaType<PackageKit::Transaction::Group>("PackageKit::Transaction::Group");
    qRegisterMetaType<PackageKit::Transaction::Info>("PackageKit::Transaction::Info");
    qRegisterMetaType<PackageKit::Transaction::SigType>("PackageKit::Transaction::SigType");
}

void DaemonPrivate::setupSignal(const QString &signal, bool connect)
{
    Q_Q(Daemon);

    const char *signalToConnect = 0;
    const char *memberToConnect = 0;

    if (signal == SIGNAL(changed())) {
        signalToConnect = SIGNAL(Changed());
        memberToConnect = SIGNAL(changed());
    } else if (signal == SIGNAL(repoListChanged())) {
        signalToConnect = SIGNAL(RepoListChanged());
        memberToConnect = SIGNAL(repoListChanged());
    } else if (signal == SIGNAL(restartScheduled())) {
        signalToConnect = SIGNAL(RestartSchedule());
        memberToConnect = SIGNAL(restartScheduled());
    } else if (signal == SIGNAL(transactionListChanged(QStringList))) {
        signalToConnect = SIGNAL(TransactionListChanged(QStringList));
        memberToConnect = SIGNAL(transactionListChanged(QStringList));
    } else if (signal == SIGNAL(updatesChanged())) {
        signalToConnect = SIGNAL(UpdatesChanged());
        memberToConnect = SIGNAL(updatesChanged());
    }

    if (signalToConnect && memberToConnect) {
        if (connect) {
            q->connect(daemon, signalToConnect, memberToConnect);
        } else {
            daemon->disconnect(signalToConnect, q, memberToConnect);
        }
    }
}

void Daemon::connectNotify(const char *signal)
{
    Q_D(Daemon);
    if (!d->connectedSignals.contains(signal) && d->daemon) {
        d->setupSignal(signal, true);
    }
    d->connectedSignals << signal;
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
void Daemon::connectNotify(const QMetaMethod &signal)
{
    // ugly but recommended way to convert a methodSignature to a SIGNAL
    connectNotify(QString("2%1")
                  .arg(QLatin1String(signal.methodSignature())).toLatin1());
}
#endif

void Daemon::disconnectNotify(const char *signal)
{
    Q_D(Daemon);
    if (d->connectedSignals.contains(signal)) {
        d->connectedSignals.removeOne(signal);
        if (d->daemon && !d->connectedSignals.contains(signal)) {
            d->setupSignal(signal, false);
        }
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
void Daemon::disconnectNotify(const QMetaMethod &signal)
{
    // ugly but recommended way to convert a methodSignature to a SIGNAL
    disconnectNotify(QString("2%1")
                     .arg(QLatin1String(signal.methodSignature())).toLatin1());
}
#endif

Daemon::~Daemon()
{
    delete d_ptr;
}

bool Daemon::isRunning() const
{
    Q_D(const Daemon);
    return d->running;
}

Transaction::Roles Daemon::actions() const
{
    Q_D(const Daemon);
    return d->roles;
}

Transaction::ProvidesFlag Daemon::provides() const
{
    Q_D(const Daemon);
    return d->provides;
}

QString Daemon::backendName() const
{
    Q_D(const Daemon);
    return d->backendName;
}

QString Daemon::backendDescription() const
{
    Q_D(const Daemon);
    return d->backendDescription;
}

QString Daemon::backendAuthor() const
{
    Q_D(const Daemon);
    return d->backendAuthor;
}

Transaction::Filters Daemon::filters() const
{
    Q_D(const Daemon);
    return d->filters;
}

Transaction::Groups Daemon::groups() const
{
    Q_D(const Daemon);
    return d->groups;
}

bool Daemon::locked() const
{
    Q_D(const Daemon);
    return d->locked;
}

QStringList Daemon::mimeTypes() const
{
    Q_D(const Daemon);
    return d->mimeTypes;
}

Daemon::Network Daemon::networkState() const
{
    Q_D(const Daemon);
    return d->networkState;
}

QString Daemon::distroID() const
{
    Q_D(const Daemon);
    return d->distroId;
}

QDBusPendingReply<Daemon::Authorize> Daemon::canAuthorize(const QString &actionId)
{
    Q_D(Daemon);
    return d->daemon->CanAuthorize(actionId);
}

QDBusPendingReply<QDBusObjectPath> Daemon::createTransaction() const
{
    Q_D(const Daemon);
    return d->daemon->CreateTransaction();
}

QDBusPendingReply<uint> Daemon::getTimeSinceAction(Transaction::Role role)
{
    Q_D(Daemon);
    return d->daemon->GetTimeSinceAction(role);
}

QDBusPendingReply<QList<QDBusObjectPath> > Daemon::getTransactionList()
{
    Q_D(Daemon);
    return d->daemon->GetTransactionList();
}

QList<Transaction*> Daemon::getTransactionObjects(QObject *parent)
{
    Q_D(Daemon);
    return d->transactions(getTransactionList(), parent);
}

void Daemon::setHints(const QStringList &hints)
{
    Q_D(Daemon);
    d->hints = hints;
}

void Daemon::setHints(const QString &hints)
{
    Q_D(Daemon);
    d->hints = QStringList() << hints;
}

QStringList Daemon::hints() const
{
    Q_D(const Daemon);
    return d->hints;
}

QDBusPendingReply<> Daemon::setProxy(const QString& http_proxy, const QString& https_proxy, const QString& ftp_proxy, const QString& socks_proxy, const QString& no_proxy, const QString& pac)
{
    Q_D(Daemon);
    return d->daemon->SetProxy(http_proxy, https_proxy, ftp_proxy, socks_proxy, no_proxy, pac);
}

QDBusPendingReply<> Daemon::stateHasChanged(const QString& reason)
{
    Q_D(Daemon);
    return d->daemon->StateHasChanged(reason);
}

QDBusPendingReply<> Daemon::suggestDaemonQuit()
{
    Q_D(Daemon);
    return d->daemon->SuggestDaemonQuit();
}

QDBusError Daemon::lastError() const
{
    Q_D(const Daemon);
    return d->lastError;
}

uint Daemon::versionMajor() const
{
    Q_D(const Daemon);
    return d->versionMajor;
}

uint Daemon::versionMinor() const
{
    Q_D(const Daemon);
    return d->versionMinor;
}

uint Daemon::versionMicro() const
{
    Q_D(const Daemon);
    return d->versionMicro;
}

QString Daemon::packageName(const QString &packageID)
{
    return packageID.section(QLatin1Char(';'), 0, 0);
}

QString Daemon::packageVersion(const QString &packageID)
{
    return packageID.section(QLatin1Char(';'), 1, 1);
}

QString Daemon::packageArch(const QString &packageID)
{
    return packageID.section(QLatin1Char(';'), 2, 2);
}

QString Daemon::packageData(const QString &packageID)
{
    return packageID.section(QLatin1Char(';'), 3, 3);
}

QString Daemon::packageIcon(const QString &packageID)
{
    return Transaction::packageIcon(packageID);
}

Transaction *Daemon::acceptEula(const QString &eulaId)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleAcceptEula;
    ret->d_ptr->eulaId = eulaId;
    return ret;
}

Transaction *Daemon::downloadPackages(const QStringList &packageIDs, bool storeInCache)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleDownloadPackages;
    ret->d_ptr->search = packageIDs;
    ret->d_ptr->storeInCache = storeInCache;
    return ret;
}

Transaction *Daemon::downloadPackage(const QString &packageID, bool storeInCache)
{
    return downloadPackages(QStringList() << packageID, storeInCache);
}

Transaction *Daemon::getCategories()
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleGetCategories;
    return ret;
}

Transaction *Daemon::getDepends(const QStringList &packageIDs, Transaction::Filters filters, bool recursive)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleGetDepends;
    ret->d_ptr->search = packageIDs;
    ret->d_ptr->filters = filters;
    ret->d_ptr->recursive = recursive;
    return ret;
}

Transaction *Daemon::getDepends(const QString &packageID, Transaction::Filters filters, bool recursive)
{
    return getDepends(QStringList() << packageID, filters, recursive);
}

Transaction *Daemon::getDetails(const QStringList &packageIDs)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleGetDetails;
    ret->d_ptr->search = packageIDs;
    return ret;
}

Transaction *Daemon::getDetails(const QString &packageID)
{
    return getDetails(QStringList() << packageID);
}

Transaction *Daemon::getDetailsLocal(const QStringList &files)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleGetDetailsLocal;
    ret->d_ptr->search = files;
    return ret;
}

Transaction *Daemon::getDetailsLocal(const QString &file)
{
    return getDetailsLocal(QStringList() << file);
}

Transaction *Daemon::getFiles(const QStringList &packageIDs)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleGetFiles;
    ret->d_ptr->search = packageIDs;
    return ret;
}

Transaction *Daemon::getFiles(const QString &packageID)
{
    return getFiles(QStringList() << packageID);
}

Transaction *Daemon::getFilesLocal(const QStringList &files)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleGetFilesLocal;
    ret->d_ptr->search = files;
    return ret;
}

Transaction *Daemon::getFilesLocal(const QString &file)
{
    return getFilesLocal(QStringList() << file);
}

Transaction *Daemon::getOldTransactions(uint number)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleGetOldTransactions;
    ret->d_ptr->numberOfOldTransactions = number;
    return ret;
}

Transaction *Daemon::getPackages(Transaction::Filters filters)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleGetPackages;
    ret->d_ptr->filters = filters;
    return ret;
}

Transaction *Daemon::getRepoList(Transaction::Filters filters)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleGetRepoList;
    ret->d_ptr->filters = filters;
    return ret;
}

Transaction *Daemon::getRequires(const QStringList &packageIDs, Transaction::Filters filters, bool recursive)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleGetRequires;
    ret->d_ptr->search = packageIDs;
    ret->d_ptr->filters = filters;
    ret->d_ptr->recursive = recursive;
    return ret;
}

Transaction *Daemon::getRequires(const QString &packageID, Transaction::Filters filters, bool recursive)
{
    return getRequires(QStringList() << packageID, filters, recursive);
}

Transaction *Daemon::getUpdatesDetails(const QStringList &packageIDs)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleGetUpdateDetail;
    ret->d_ptr->search = packageIDs;
    return ret;
}

Transaction *Daemon::getUpdateDetail(const QString &packageID)
{
    return getUpdatesDetails(QStringList() << packageID);
}

Transaction *Daemon::getUpdates(Transaction::Filters filters)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleGetUpdates;
    ret->d_ptr->filters = filters;
    return ret;
}

Transaction *Daemon::getDistroUpgrades()
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleGetDistroUpgrades;
    return ret;
}

Transaction *Daemon::installFiles(const QStringList &files, Transaction::TransactionFlags flags)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleInstallFiles;
    ret->d_ptr->search = files;
    ret->d_ptr->flags = flags;
    return ret;
}

Transaction *Daemon::installFile(const QString &file, Transaction::TransactionFlags flags)
{
    return installFiles(QStringList() << file, flags);
}

Transaction *Daemon::installPackages(const QStringList &packageIDs, Transaction::TransactionFlags flags)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleInstallPackages;
    ret->d_ptr->search = packageIDs;
    ret->d_ptr->flags = flags;
    return ret;
}

Transaction *Daemon::installPackage(const QString &packageID, Transaction::TransactionFlags flags)
{
    return installPackages(QStringList() << packageID, flags);
}

Transaction *Daemon::installSignature(Transaction::SigType type, const QString &keyID, const QString &packageID)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleInstallSignature;
    ret->d_ptr->signatureType = type;
    ret->d_ptr->signatureKey = keyID;
    ret->d_ptr->signaturePackage = packageID;
    return ret;
}

Transaction *Daemon::refreshCache(bool force)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleRefreshCache;
    ret->d_ptr->refreshCacheForce = force;
    return ret;
}

Transaction *Daemon::removePackages(const QStringList &packageIDs, bool allowDeps, bool autoremove, Transaction::TransactionFlags flags)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleRemovePackages;
    ret->d_ptr->search = packageIDs;
    ret->d_ptr->allowDeps = allowDeps;
    ret->d_ptr->autoremove = autoremove;
    ret->d_ptr->flags = flags;
    return ret;
}

Transaction *Daemon::removePackage(const QString &packageID, bool allowDeps, bool autoremove, Transaction::TransactionFlags flags)
{
    return removePackages(QStringList() << packageID, allowDeps, autoremove, flags);
}

Transaction *Daemon::repairSystem(Transaction::TransactionFlags flags)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleRepairSystem;
    ret->d_ptr->flags = flags;
    return ret;
}

Transaction *Daemon::repoEnable(const QString &repoId, bool enable)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleRepoEnable;
    ret->d_ptr->repoId = repoId;
    ret->d_ptr->repoEnable = enable;
    return ret;
}

Transaction *Daemon::repoRemove(const QString &repoId, bool autoremove, Transaction::TransactionFlags flags)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleRepoRemove;
    ret->d_ptr->repoId = repoId;
    ret->d_ptr->autoremove = autoremove;
    ret->d_ptr->flags = flags;
    return ret;
}

Transaction *Daemon::repoSetData(const QString &repoId, const QString &parameter, const QString &value)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleRepoSetData;
    ret->d_ptr->repoId = repoId;
    ret->d_ptr->repoParameter = parameter;
    ret->d_ptr->repoValue = value;
    return ret;
}

Transaction *Daemon::resolve(const QStringList &packageNames, Transaction::Filters filters)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleResolve;
    ret->d_ptr->search = packageNames;
    ret->d_ptr->filters = filters;
    return ret;
}

Transaction *Daemon::resolve(const QString &packageName, Transaction::Filters filters)
{
    return resolve(QStringList() << packageName, filters);
}

Transaction *Daemon::searchFiles(const QStringList &search, Transaction::Filters filters)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleSearchFile;
    ret->d_ptr->search = search;
    ret->d_ptr->filters = filters;
    return ret;
}

Transaction *Daemon::searchFiles(const QString &search, Transaction::Filters filters)
{
    return searchFiles(QStringList() << search, filters);
}

Transaction *Daemon::searchDetails(const QStringList &search, Transaction::Filters filters)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleSearchDetails;
    ret->d_ptr->search = search;
    ret->d_ptr->filters = filters;
    return ret;
}

Transaction *Daemon::searchDetails(const QString &search, Transaction::Filters filters)
{
    return searchDetails(QStringList() << search, filters);
}

Transaction *Daemon::searchGroups(const QStringList &groups, Transaction::Filters filters)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleSearchGroup;
    ret->d_ptr->search = groups;
    ret->d_ptr->filters = filters;
    return ret;
}

Transaction *Daemon::searchGroup(const QString &group, Transaction::Filters filters)
{
    return searchGroups(QStringList() << group, filters);
}

Transaction *Daemon::searchGroup(Transaction::Group group, Transaction::Filters filters)
{
    QString groupString = Daemon::enumToString<Transaction>(group, "Group");
    return searchGroup(groupString, filters);
}

Transaction *Daemon::searchGroups(Transaction::Groups groups, Transaction::Filters filters)
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
    return searchGroups(groupsStringList, filters);
}

Transaction *Daemon::searchNames(const QStringList &search, Transaction::Filters filters)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleSearchName;
    ret->d_ptr->search = search;
    ret->d_ptr->filters = filters;
    return ret;
}

Transaction *Daemon::searchNames(const QString &search, Transaction::Filters filters)
{
    return searchNames(QStringList() << search, filters);
}

Transaction *Daemon::updatePackages(const QStringList &packageIDs, Transaction::TransactionFlags flags)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleUpdatePackages;
    ret->d_ptr->search = packageIDs;
    ret->d_ptr->flags = flags;
    return ret;
}

Transaction *Daemon::updatePackage(const QString &packageID, Transaction::TransactionFlags flags)
{
    return updatePackages(QStringList() << packageID, flags);
}

Transaction *Daemon::whatProvides(const QStringList &search, Transaction::Filters filters)
{
    Transaction *ret = new Transaction(Daemon::global());
    ret->d_ptr->role = Transaction::RoleWhatProvides;
    ret->d_ptr->search = search;
    ret->d_ptr->filters = filters;
    return ret;
}

Transaction *Daemon::whatProvides(const QString &search, Transaction::Filters filters)
{
    return whatProvides(QStringList() << search, filters);
}

#include "daemon.moc"

