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
    Q_D(Transaction);

    connect(Daemon::global(), SIGNAL(daemonQuit()), SLOT(daemonQuit()));

    QDBusPendingReply<QDBusObjectPath> reply = Daemon::global()->createTransaction();
    auto watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, [this, d] (QDBusPendingCallWatcher *call)
    {
        QDBusPendingReply<QDBusObjectPath> reply = *call;
        if (reply.isError()) {
            QDBusError error = reply.error();
            Transaction::Error transactionError = error.type() == QDBusError::AccessDenied ? Transaction::ErrorNotAuthorized
                                                                                           : Transaction::ErrorInternalError;
            errorCode(transactionError, error.message());
            d->finished(Transaction::ExitFailed, 0);
            d->destroy();
        } else {
            // Setup our new Transaction ID
            d->setup(reply.argumentAt<0>());
        }
        call->deleteLater();
    });
}

Transaction::Transaction(const QDBusObjectPath &tid)
    : d_ptr(new TransactionPrivate(this))
{
    Q_D(Transaction);

    connect(Daemon::global(), SIGNAL(daemonQuit()), SLOT(daemonQuit()));
    d->setup(tid);
}

void Transaction::connectNotify(const QMetaMethod &signal)
{
    Q_D(Transaction);
    if (!d->connectedSignals.contains(signal)) {
        d->connectedSignals << signal;

        if (d->p) {
            d->setupSignal(signal);
        }
    }
}

void Transaction::disconnectNotify(const QMetaMethod &signal)
{
    QObject::disconnectNotify(signal);
}

Transaction::Transaction(TransactionPrivate *d)
    : d_ptr(d)
{
}

void TransactionPrivate::setupSignal(const QMetaMethod &signal)
{
    Q_Q(Transaction);

    const char *signalToConnect = 0;
    const char *memberToConnect = 0;

    if (signal == QMetaMethod::fromSignal(&Transaction::category)) {
        signalToConnect = SIGNAL(Category(QString,QString,QString,QString,QString));
        memberToConnect = SIGNAL(category(QString,QString,QString,QString,QString));
    } else if (signal == QMetaMethod::fromSignal(&Transaction::details)) {
        signalToConnect = SIGNAL(Details(QVariantMap));
        memberToConnect = SLOT(details(QVariantMap));
    } else if (signal == QMetaMethod::fromSignal(&Transaction::distroUpgrade)) {
        signalToConnect = SIGNAL(DistroUpgrade(uint,QString,QString));
        memberToConnect = SLOT(distroUpgrade(uint,QString,QString));
    } else if (signal == QMetaMethod::fromSignal(&Transaction::errorCode)) {
        signalToConnect = SIGNAL(ErrorCode(uint,QString));
        memberToConnect = SLOT(errorCode(uint,QString));
    } else if (signal == QMetaMethod::fromSignal(&Transaction::files)) {
        signalToConnect = SIGNAL(Files(QString,QStringList));
        memberToConnect = SIGNAL(files(QString,QStringList));
    } else if (signal == QMetaMethod::fromSignal(&Transaction::finished)) {
        signalToConnect = SIGNAL(Finished(uint,uint));
        memberToConnect = SLOT(finished(uint,uint));
    } else if (signal == QMetaMethod::fromSignal(&Transaction::package)) {
        signalToConnect = SIGNAL(Package(uint,QString,QString));
        memberToConnect = SLOT(Package(uint,QString,QString));
    } else if (signal == QMetaMethod::fromSignal(&Transaction::repoDetail)) {
        signalToConnect = SIGNAL(RepoDetail(QString,QString,bool));
        memberToConnect = SIGNAL(repoDetail(QString,QString,bool));
    } else if (signal == QMetaMethod::fromSignal(&Transaction::repoSignatureRequired)) {
        signalToConnect = SIGNAL(RepoSignatureRequired(QString,QString,QString,QString,QString,QString,QString,uint));
        memberToConnect = SLOT(RepoSignatureRequired(QString,QString,QString,QString,QString,QString,QString,uint));
    } else if (signal == QMetaMethod::fromSignal(&Transaction::eulaRequired)) {
        signalToConnect = SIGNAL(EulaRequired(QString,QString,QString,QString));
        memberToConnect = SIGNAL(eulaRequired(QString,QString,QString,QString));
    } else if (signal == QMetaMethod::fromSignal(&Transaction::mediaChangeRequired)) {
        signalToConnect = SIGNAL(MediaChangeRequired(uint,QString,QString));
        memberToConnect = SLOT(mediaChangeRequired(uint,QString,QString));
    } else if (signal == QMetaMethod::fromSignal(&Transaction::itemProgress)) {
        signalToConnect = SIGNAL(ItemProgress(QString,uint,uint));
        memberToConnect = SLOT(ItemProgress(QString,uint,uint));
    } else if (signal == QMetaMethod::fromSignal(&Transaction::requireRestart)) {
        signalToConnect = SIGNAL(RequireRestart(uint,QString));
        memberToConnect = SLOT(requireRestart(uint,QString));
    } else if (signal == QMetaMethod::fromSignal(&Transaction::transaction)) {
        signalToConnect = SIGNAL(Transaction(QDBusObjectPath,QString,bool,uint,uint,QString,uint,QString));
        memberToConnect = SLOT(transaction(QDBusObjectPath,QString,bool,uint,uint,QString,uint,QString));
    } else if (signal == QMetaMethod::fromSignal(&Transaction::updateDetail)) {
        signalToConnect = SIGNAL(UpdateDetail(QString,QStringList,QStringList,QStringList,QStringList,QStringList,uint,QString,QString,uint,QString,QString));
        memberToConnect = SLOT(UpdateDetail(QString,QStringList,QStringList,QStringList,QStringList,QStringList,uint,QString,QString,uint,QString,QString));
    }

    if (signalToConnect && memberToConnect) {
        QObject::connect(p, signalToConnect, q, memberToConnect);
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
    QString ret;
    ret = packageID.left(packageID.indexOf(QLatin1Char(';')));
    return ret;
}

QString Transaction::packageVersion(const QString &packageID)
{
    QString ret;
    int start = packageID.indexOf(QLatin1Char(';'));
    if (start == -1) {
        return ret;
    }
    int end = packageID.indexOf(QLatin1Char(';'), ++start);
    if (Q_UNLIKELY(end == -1)) {
        ret = packageID.mid(start);
    } else {
        ret = packageID.mid(start, end - start);
    }
    return ret;
}

QString Transaction::packageArch(const QString &packageID)
{
    QString ret;
    int start = packageID.indexOf(QLatin1Char(';'));
    if (start == -1) {
        return ret;
    }
    start = packageID.indexOf(QLatin1Char(';'), ++start);
    if (start == -1) {
        return ret;
    }
    int end = packageID.indexOf(QLatin1Char(';'), ++start);
    if (Q_UNLIKELY(end == -1)) {
        ret = packageID.mid(start);
    } else {
        ret = packageID.mid(start, end - start);
    }
    return ret;
}

QString Transaction::packageData(const QString &packageID)
{
    QString ret;
    int start = packageID.indexOf(QLatin1Char(';'));
    if (start == -1) {
        return ret;
    }
    start = packageID.indexOf(QLatin1Char(';'), ++start);
    if (start == -1) {
        return ret;
    }
    start = packageID.indexOf(QLatin1Char(';'), ++start);
    if (start == -1) {
        return ret;
    }

    ret = packageID.mid(++start);
    return ret;
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
    return setHints(QStringList{ hints });
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

#include "moc_transaction.cpp"
