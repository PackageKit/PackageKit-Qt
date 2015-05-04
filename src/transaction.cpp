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

#include <QDBusError>

Q_LOGGING_CATEGORY(PACKAGEKITQT_TRANSACTION, "packagekitqt.transaction")

using namespace PackageKit;

Transaction::Transaction()
    : d_ptr(new TransactionPrivate(this))
{
    connect(Daemon::global(), SIGNAL(daemonQuit()), SLOT(daemonQuit()));

    QDBusPendingReply<QDBusObjectPath> reply = Daemon::global()->createTransaction();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(createTransactionFinished(QDBusPendingCallWatcher*)));
}

Transaction::Transaction(const QDBusObjectPath &tid)
    : d_ptr(new TransactionPrivate(this))
{
    Q_D(Transaction);

    connect(Daemon::global(), SIGNAL(daemonQuit()), SLOT(daemonQuit()));
    d->setup(tid);
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

Transaction::Transaction(TransactionPrivate *d)
    : d_ptr(d)
{
}

void TransactionPrivate::setupSignal(const QString &signal, bool connect)
{
    Q_Q(Transaction);

    const char *signalToConnect = 0;
    const char *memberToConnect = 0;

    if (signal == SIGNAL(category(QString,QString,QString,QString,QString))) {
        signalToConnect = SIGNAL(Category(QString,QString,QString,QString,QString));
        memberToConnect = SIGNAL(category(QString,QString,QString,QString,QString));
    } else if (signal == SIGNAL(details(PackageKit::Details))) {
        signalToConnect = SIGNAL(Details(QVariantMap));
        memberToConnect = SLOT(details(QVariantMap));
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

Transaction::~Transaction()
{
//    qDebug() << "Destroying transaction with tid" << d_ptr->tid.path();
    delete d_ptr;
}

QDBusObjectPath Transaction::tid() const
{
    Q_D(const Transaction);
    return d->tid;
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

QDBusPendingReply<> Transaction::cancel()
{
    Q_D(const Transaction);
    if (d->p) {
        return d->p->Cancel();
    }
    return QDBusPendingReply<>();
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
    return QString();
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

QDBusPendingReply<> Transaction::setHints(const QStringList &hints)
{
    Q_D(Transaction);
    if (d->p) {
        return d->p->SetHints(hints);
    }
    return QDBusPendingReply<>();
}

QDBusPendingReply<> Transaction::setHints(const QString &hints)
{
    return setHints(QStringList() << hints);
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

    qCWarning(PACKAGEKITQT_TRANSACTION) << "Transaction::parseError: unknown error" << errorName;
    return Transaction::InternalErrorFailed;
}

#include "transaction.moc"

