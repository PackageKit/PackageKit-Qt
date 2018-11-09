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

#ifndef PACKAGEKIT_DAEMON_H
#define PACKAGEKIT_DAEMON_H

#include <QtCore/QObject>
#include <QtCore/QMetaEnum>
#include <QtDBus/QDBusError>
#include <QtDBus/QDBusPendingReply>

#include <packagekitqt_global.h>

#include "transaction.h"

namespace PackageKit {

class Offline;

/**
 * \class Daemon daemon.h Daemon
 * \author Adrien Bustany \e <madcat@mymadcat.com>
 * \author Daniel Nicoletti \e <dantti12@gmail.com>
 *
 * \brief Base class used to interact with the PackageKit daemon
 *
 * This class holds all the functions enabling the user to interact with the PackageKit daemon.
 *
 * Most methods are static so that you can just call Daemon::backendName() to get the name of the backend.
 * 
 * This class is a singleton, its constructor is private. Call Daemon::global() to get
 * an instance of the Daemon object, you only need Daemon::global() when connecting to the signals
 * of this class.
 */
class DaemonPrivate;
class PACKAGEKITQT_LIBRARY Daemon : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningChanged)
    Q_PROPERTY(Transaction::Roles roles READ roles NOTIFY changed)
    Q_PROPERTY(QString backendName READ backendName NOTIFY changed)
    Q_PROPERTY(QString backendDescription READ backendDescription NOTIFY changed)
    Q_PROPERTY(QString backendAuthor READ backendAuthor NOTIFY changed)
    Q_PROPERTY(Transaction::Filters filters READ filters NOTIFY changed)
    Q_PROPERTY(Transaction::Groups groups READ groups NOTIFY changed)
    Q_PROPERTY(bool locked READ locked NOTIFY changed)
    Q_PROPERTY(QStringList mimeTypes READ mimeTypes NOTIFY changed)
    Q_PROPERTY(Daemon::Network networkState READ networkState NOTIFY networkStateChanged)
    Q_PROPERTY(QString distroID READ distroID NOTIFY changed)
    Q_PROPERTY(uint versionMajor READ versionMajor NOTIFY changed)
    Q_PROPERTY(uint versionMinor READ versionMinor NOTIFY changed)
    Q_PROPERTY(uint versionMicro READ versionMicro NOTIFY changed)
public:
    /**
     * Describes the current network state
     */
    enum Network {
        NetworkUnknown,
        NetworkOffline,
        NetworkOnline,
        NetworkWired,
        NetworkWifi,
        NetworkMobile
    };
    Q_ENUM(Network)

    /**
     * Describes the authorization result
     * \sa canAuthorize()
     */
    enum Authorize {
        AuthorizeUnknown,
        AuthorizeYes,
        AuthorizeNo,
        AuthorizeInteractive
    };
    Q_ENUM(Authorize)

    /**
     * \brief Returns an instance of the Daemon
     *
     * The Daemon class is a singleton, you can call this method several times,
     * a single Daemon object will exist.
     * Use this only when connecting to this class signals
     */
    static Daemon* global();

    /**
     * Destructor
     */
    ~Daemon();

    /**
     * Returns true if the daemon is running (ie registered to DBus)
     */
    static bool isRunning();

    /**
     * Returns all the roles supported by the current backend
     */
    static Transaction::Roles roles();

    /**
     * The backend name, e.g. "yum".
     */
    static QString backendName();

    /**
     * The backend description, e.g. "Yellow Dog Update Modifier".
     */
    static QString backendDescription();

    /**
     * The backend author, e.g. "Joe Bloggs <joe@blogs.com>"
     */
    static QString backendAuthor();

    /**
     * Returns the package filters supported by the current backend
     */
    static Transaction::Filters filters();

    /**
     * Returns the package groups supported by the current backend
     */
    static Transaction::Groups groups();

    /**
     * Set when the backend is locked and native tools would fail.
     */
    static bool locked();

    /**
     * Returns a list containing the MIME types supported by the current backend
     */
    static QStringList mimeTypes();

    /**
     * Returns the current network state
     */
    static Daemon::Network networkState();

    /**
     * The distribution identifier in the
     * distro;version;arch form,
     * e.g. "debian;squeeze/sid;x86_64".
     */
    static QString distroID();

    /**
     * Returns the major version number.
     */
    static uint versionMajor();

    /**
     * The minor version number.
     */
    static uint versionMinor();

    /**
     * The micro version number.
     */
    static uint versionMicro();

    /**
     * Allows a client to find out if it would be allowed to authorize an action.
     * The action ID, e.g. org.freedesktop.packagekit.system-network-proxy-configure
     * specified in \p actionId
     * Return might be either yes, no or interactive \sa Authorize.
     */
    static QDBusPendingReply<Authorize> canAuthorize(const QString &actionId);

    /**
     * Returns the time (in seconds) since the specified \p action
     */
    static QDBusPendingReply<uint> getTimeSinceAction(PackageKit::Transaction::Role action);

    /**
     * \brief creates a new transaction path
     *
     * This function register a new DBus path on PackageKit
     * allowing a \c Transaction object to be created.
     *
     * \note Unless you want to know the transaction id
     * before creating the \c Transaction object this function
     * is not useful as simply creating a \c Transaction object will
     * automatically create this path.
     */
    static QDBusPendingReply<QDBusObjectPath> createTransaction();

    /**
     * Returns the list of current transactions
     */
    static QDBusPendingReply<QList<QDBusObjectPath> > getTransactionList();

    /**
     * \brief Sets a global hints for all the transactions to be created
     *
     * This method allows the calling session to set transaction \p hints for
     * the package manager which can change as the transaction runs.
     *
     * This method can be sent before the transaction has been run
     * (by using Daemon::setHints) or whilst it is running
     * (by using Transaction::setHints).
     * There is no limit to the number of times this
     * method can be sent, although some backends may only use the values
     * that were set before the transaction was started.
     *
     * The \p hints can be filled with entries like these
     * ('locale=en_GB.utf8','idle=true','interactive=false').
     *
     * \sa Transaction::setHints
     */
    static void setHints(const QStringList &hints);

    /**
     * Convenience function to set global hints
     * \sa setHints(const QStringList &hints)
     */
    static void setHints(const QString &hints);

    /**
     * This method returns the current hints
     */
    static QStringList hints();

    /**
     * Sets a proxy to be used for all the network operations
     */
    static QDBusPendingReply<> setProxy(const QString &http_proxy, const QString &https_proxy, const QString &ftp_proxy, const QString &socks_proxy, const QString &no_proxy, const QString &pac);

    /**
     * \brief Tells the daemon that the system state has changed, to make it reload its cache
     *
     * \p reason can be resume or posttrans
     */
    static QDBusPendingReply<> stateHasChanged(const QString &reason);

    /**
     * Asks PackageKit to quit, for example to let a native package manager operate
     */
    static QDBusPendingReply<> suggestDaemonQuit();

    /**
     * Returns a class representing PackageKit offline interface, as with the Daemon
     * class this will only have valid properties if isRunning() is true
     */
    Offline *offline() const;

    /**
     * Returns the package name from the \p packageID
     */
    Q_INVOKABLE static QString packageName(const QString &packageID);

    /**
     * Returns the package version from the \p packageID
     */
    Q_INVOKABLE static QString packageVersion(const QString &packageID);

    /**
     * Returns the package arch from the \p packageID
     */
    Q_INVOKABLE static QString packageArch(const QString &packageID);

    /**
     * Returns the package data from the \p packageID
     */
    Q_INVOKABLE static QString packageData(const QString &packageID);
    
    static QString enumToString(const QMetaObject &metaObject, int value, const char *enumName);

    /**
     * Returns the string representing the enum
     * Useful for PackageDetails::Group
     */
    template<class T> static QString enumToString(int value, const char *enumName)
    {
        return enumToString(T::staticMetaObject, value, enumName);
    }

    static int enumFromString(const QMetaObject &metaObject, const QString &str, const char *enumName);
    
    template<class T> static int enumFromString(const QString &str, const char *enumName)
    {
        return enumFromString(T::staticMetaObject, str, enumName);
    }

    /**
     * \brief Accepts an EULA
     *
     * The EULA is identified by the \sa Eula structure \p info
     *
     * \note You need to manually restart the transaction which triggered the EULA.
     * \sa eulaRequired()
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *acceptEula(const QString &eulaID);

    /**
     * Download the given \p packages to a temp dir, if \p storeInCache is true
     * the download will be stored in the package manager cache
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *downloadPackages(const QStringList &packageIDs, bool storeInCache = false);

    /**
     * This is a convenience function to download this \p package
     * \sa downloadPackages(const QStringList &packageIDs, bool storeInCache = false)
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *downloadPackage(const QString &packageID, bool storeInCache = false);

    /**
     * Returns the collection categories
     *
     * \sa category
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *getCategories();

    /**
     * \brief Gets the list of dependencies for the given \p packages
     *
     * You can use the \p filters to limit the results to certain packages.
     * The \p recursive flag indicates if the package manager should also
     * fetch the dependencies's dependencies.
     *
     * \note This method emits \sa package()
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *dependsOn(const QStringList &packageIDs, Transaction::Filters filters = Transaction::FilterNone, bool recursive = false);

    /**
     * Convenience function to get the dependencies of this \p package
     * \sa dependsOn(const QStringList &packageIDs, Filters filters, bool recursive = false)
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *dependsOn(const QString &packageID, Transaction::Filters filters = Transaction::FilterNone, bool recursive = false);

    /**
     * Gets more details about the given \p packages
     *
     * \sa Transaction::details
     * \note This method emits \sa package()
     * with details set
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *getDetails(const QStringList &packageIDs);

    /**
     * Convenience function to get the details about this \p package
     * \sa getDetails(const QStringList &packageIDs)
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *getDetails(const QString &packageID);

    /**
     * Gets more details about the given \p files
     *
     * \sa Transaction::details
     * \note this method emits:
     * \li details()
     * \li status()
     * \li progress()
     * \li error()
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *getDetailsLocal(const QStringList &files);

    /**
     * Gets more details about the given \p file
     *
     * \sa Transaction::details
     *
     * \note this method emits:
     * \li details()
     * \li status()
     * \li progress()
     * \li error()
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *getDetailsLocal(const QString &file);

    /**
     * Gets the files contained in the given \p packages
     *
     * \note This method emits \sa files()
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *getFiles(const QStringList &packageIDs);

    /**
     * Convenience function to get the files contained in this \p package
     * \sa getFiles(const QStringList &packageIDs)
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *getFiles(const QString &packageIDs);

    /**
     * Gets the files contained in the given \p files
     *
     * \note This method emits \sa files()
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *getFilesLocal(const QStringList &files);

    /**
     * Gets the files contained in the given \p file
     *
     * \note This method emits \sa files()
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *getFilesLocal(const QString &file);

    /**
     * \brief Gets the last \p number finished transactions
     *
     * \note You must delete these transactions yourself
     * \note This method emits \sa transaction()
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *getOldTransactions(uint number);

    /**
     * Gets all the packages matching the given \p filters
     *
     * \note This method emits \sa package()
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *getPackages(Transaction::Filters filters = Transaction::FilterNone);

    /**
     * Gets the list of software repositories matching the given \p filters
     *
     * \note This method emits \sa repository()
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *getRepoList(Transaction::Filters filters = Transaction::FilterNone);

    /**
     * \brief Searches for the packages requiring the given \p packages
     *
     * The search can be limited using the \p filters parameter.
     * The \p recursive flag is used to tell if the package manager should
     * also search for the package requiring the resulting packages.
     *
     * \note This method emits \sa package()
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *requiredBy(const QStringList &packageIDs, Transaction::Filters filters = Transaction::FilterNone, bool recursive = false);

    /**
     * Convenience function to get packages requiring this package
     * \sa requiredBy(const QStringList &packageIDs, Filters filters, bool recursive = false)
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *requiredBy(const QString &packageID, Transaction::Filters filters = Transaction::FilterNone, bool recursive = false);

    /**
     * Retrieves more details about the update for the given \p packageIDs
     *
     * \note This method emits \sa updateDetail()
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *getUpdatesDetails(const QStringList &packageIDs);

    /**
     * Convenience function to get update details
     * \sa getUpdateDetail(const QStringList &packageIDs)
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *getUpdateDetail(const QString &packageID);

    /**
     * \p Gets the available updates
     *
     * The \p filters parameters can be used to restrict the updates returned
     *
     * \note This method emits \sa package()
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *getUpdates(Transaction::Filters filters = Transaction::FilterNone);

    /**
     * Retrieves the available distribution upgrades
     *
     * \note This method emits \sa distroUpgrade()
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *getDistroUpgrades();

    /**
     * Updates the whole system
     *
     * This method perfoms a distribution upgrade to the
     * specified version.
     *
     * The \p type of upgrade, e.g. minimal, default or complete.
     * Minimal upgrades will download the smallest amount of data
     * before launching a installer.
     * The default is to download enough data to launch a full
     * graphical installer, but a complete upgrade will be
     * required if there is no internet access during install time.
     *
     * \note This method typically emits
     * \li progress()
     * \li status()
     * \li error()
     * \li package()
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *upgradeSystem(const QString &distroId, Transaction::UpgradeKind kind,
                                      Transaction::TransactionFlags flags = Transaction::TransactionFlagOnlyTrusted);

    /**
     * \brief Installs the local packages \p files
     *
     * \p onlyTrusted indicate if the packages are signed by a trusted authority
     *
     * \note This method emits \sa package() and \sa changed()
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *installFiles(const QStringList &files, Transaction::TransactionFlags flags = Transaction::TransactionFlagOnlyTrusted);

    /**
     * Convenience function to install a file
     * \sa installFiles(const QStringList &files, TransactionFlags flags)
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *installFile(const QString &file, Transaction::TransactionFlags flags = Transaction::TransactionFlagOnlyTrusted);

    /**
     * Install the given \p packages
     *
     * \p only_trusted indicates if we should allow installation of untrusted packages (requires a different authorization)
     *
     * \note This method emits \sa package() and \sa changed()
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *installPackages(const QStringList &packageIDs, Transaction::TransactionFlags flags = Transaction::TransactionFlagOnlyTrusted);

    /**
     * Convenience function to install a package
     * \sa installPackages(const QStringList &packageIDs, TransactionFlags flags)
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *installPackage(const QString &packageID, Transaction::TransactionFlags flags = Transaction::TransactionFlagOnlyTrusted);

    /**
     * \brief Installs a signature
     *
     * \p type, \p keyId and \p package generally come from the Transaction::repoSignatureRequired
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *installSignature(Transaction::SigType type, const QString &keyID, const QString &packageID);

    /**
     * Refreshes the package manager's cache
     *
     * \note This method emits \sa changed()
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *refreshCache(bool force);

    /**
     * \brief Removes the given \p packages
     *
     * \p allowDeps if the package manager has the right to remove other packages which depend on the
     * packages to be removed. \p autoRemove tells the package manager to remove all the package which
     * won't be needed anymore after the packages are uninstalled.
     *
     * \note This method emits \sa package() and \sa changed()
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *removePackages(const QStringList &packageIDs, bool allowDeps = false, bool autoRemove = false, Transaction::TransactionFlags flags = Transaction::TransactionFlagOnlyTrusted);

    /**
     * Convenience function to remove a package
     *
     * \sa removePackages(const PackageList  &packages, bool allowDeps = false, bool autoRemove = false, TransactionFlags flags)
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *removePackage(const QString &packageID, bool allowDeps = false, bool autoRemove = false, Transaction::TransactionFlags flags = Transaction::TransactionFlagOnlyTrusted);

    /**
     * Repairs a broken system
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *repairSystem(Transaction::TransactionFlags flags = Transaction::TransactionFlagOnlyTrusted);

    /**
     * Activates or disables a repository
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *repoEnable(const QString &repoId, bool enable = true);

    /**
     * Removes a repository
     *
     * \p autoremove packages from this repository
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *repoRemove(const QString &repoId, bool autoremove, Transaction::TransactionFlags flags = Transaction::TransactionFlagNone);

    /**
     * Sets a repository's parameter
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *repoSetData(const QString &repoId, const QString &parameter, const QString &value);

    /**
     * \brief Tries to create a Package object from the package's name
     *
     * The \p filters can be used to restrict the search
     *
     * \note This method emits \sa package()
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *resolve(const QStringList &packageNames, Transaction::Filters filters = Transaction::FilterNone);

    /**
     * Convenience function to remove a package name
     * \sa resolve(const QStringList &packageNames, Transaction::Filters filters = Transaction::FilterNone)
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *resolve(const QString &packageName, Transaction::Filters filters = Transaction::FilterNone);

    /**
     * \brief Search in the packages files
     *
     * \p filters can be used to restrict the returned packages
     *
     * \note This method emits \sa package()
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *searchFiles(const QStringList &search, Transaction::Filters filters = Transaction::FilterNone);

    /**
     * Convenience function to search for a file
     * \sa searchFiles(const QStringList &search, Transaction::Filters filters = Transaction::FilterNone)
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *searchFiles(const QString &search, Transaction::Filters filters = Transaction::FilterNone);

    /**
     * \brief Search in the packages details
     *
     * \p filters can be used to restrict the returned packages
     *
     * \note This method emits \sa package()
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *searchDetails(const QStringList &search, Transaction::Filters filters = Transaction::FilterNone);

    /**
     * Convenience function to search by details
     * \sa searchDetails(const QStringList &search, Transaction::Filters filters = Transaction::FilterNone)
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *searchDetails(const QString &search, Transaction::Filters filters = Transaction::FilterNone);

    /**
     * \brief Lists all the packages in the given \p group
     *
     * \p groups is the name of the group that you want, when searching for
     * categories prefix it with '@'
     * \p filters can be used to restrict the returned packages
     *
     * \note This method emits \sa package()
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *searchGroups(const QStringList &groups, Transaction::Filters filters = Transaction::FilterNone);

    /**
     * Convenience function to search by group string
     * \sa searchGroups(const QStringList &groups, Transaction::Filters filters = Transaction::FilterNone)
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *searchGroup(const QString &group, Transaction::Filters filters = Transaction::FilterNone);

    /**
     * Convenience function to search by group enum
     * \sa searchGroups(const QStringList &groups, Transaction::Filters filters = Transaction::FilterNone)
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *searchGroup(Transaction::Group group, Transaction::Filters filters = Transaction::FilterNone);

    /**
     * \brief Lists all the packages in the given \p group
     *
     * \p filters can be used to restrict the returned packages
     *
     * \note This method emits \sa package()
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *searchGroups(Transaction::Groups group, Transaction::Filters filters = Transaction::FilterNone);

    /**
     * \brief Search in the packages names
     *
     * \p filters can be used to restrict the returned packages
     *
     * \note This method emits \sa package()
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *searchNames(const QStringList &search, Transaction::Filters filters = Transaction::FilterNone);

    /**
     * Convenience function to search by names
     * \sa searchNames(const QStringList &search, Filters filters)
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *searchNames(const QString &search, Transaction::Filters filters = Transaction::FilterNone);

    /**
     * Update the given \p packages
     *
     * \p onlyTrusted indicates if this transaction is only allowed to install trusted packages
     * \note This method emits \sa package() and \sa changed()
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *updatePackages(const QStringList &packageIDs, Transaction::TransactionFlags flags = Transaction::TransactionFlagOnlyTrusted);

    /**
     * Convenience function to update a package
     * \sa updatePackages(const QStringList &packageIDs, TransactionFlags flags)
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *updatePackage(const QString &packageID, Transaction::TransactionFlags flags = Transaction::TransactionFlagOnlyTrusted);

    /**
     * Searchs for a package providing a file/a mimetype
     *
     * \note This method emits \sa package()
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *whatProvides(const QStringList &search, Transaction::Filters filters = Transaction::FilterNone);

    /**
     * Convenience function to search for what provides
     * \sa whatProvides(Provides type, const QStringList &search, Transaction::Filters filters = Transaction::FilterNone)
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    static Transaction *whatProvides(const QString &search, Transaction::Filters filters = Transaction::FilterNone);

Q_SIGNALS:
    void isRunningChanged();

    void networkStateChanged();

    /**
     * This signal is emitted when a property on the interface changes.
     */
    void changed();

    /**
     * Emitted when the list of repositories changes
     */
    void repoListChanged();

    /**
     * Emmitted when a restart is scheduled
     */
    void restartScheduled();

    /**
     * \brief Emitted when the current transactions list changes.
     *
     * \note This is mostly useful for monitoring the daemon's state.
     */
    void transactionListChanged(const QStringList &tids);

    /**
     * Emitted when new updates are available
     */
    void updatesChanged();

    /**
     * Emitted when the daemon quits
     */
    void daemonQuit();

protected:
    /**
     * This method connects to DBus signals
     * \attention Make sure to call this method in inherited classes
     * otherwise no signals will be emitted
     */
    void connectNotify(const QMetaMethod &signal) override;

    /**
     * \attention Make sure to call this method in inherited classes
     */
    void disconnectNotify(const QMetaMethod &signal) override;

    DaemonPrivate * const d_ptr;

private:
    Q_DECLARE_PRIVATE(Daemon)
    Q_PRIVATE_SLOT(d_func(), void propertiesChanged(QString,QVariantMap,QStringList))
    Q_PRIVATE_SLOT(d_func(), void updateProperties(QVariantMap))
    Daemon(QObject *parent = nullptr);
    static Daemon *m_global;
};

} // End namespace PackageKit

Q_DECLARE_METATYPE(PackageKit::Daemon::Network)
Q_DECLARE_METATYPE(PackageKit::Daemon::Authorize)

#endif
