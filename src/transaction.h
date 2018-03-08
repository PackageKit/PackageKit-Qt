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

#ifndef PACKAGEKIT_TRANSACTION_H
#define PACKAGEKIT_TRANSACTION_H

#include <QtCore/QObject>
#include <QtCore/QDateTime>
#include <QtDBus/QDBusObjectPath>
#include <QtDBus/QDBusPendingReply>

#include <packagekitqt_global.h>

#include "bitfield.h"

namespace PackageKit {

class Details;

/**
* \class Transaction transaction.h Transaction
* \author Adrien Bustany \e <madcat@mymadcat.com>
* \author Daniel Nicoletti \e <dantti12@gmail.com>
*
* \brief A transaction represents an occurring action in PackageKit
*
* A Transaction is created whenever you do an asynchronous action (for example a Search, Install...).
* This class allows you to monitor and control the flow of the action.
*
* Transaction will be automatically deleted after finished() is emitted
*
* \sa Daemon
*/
class TransactionPrivate;
class PACKAGEKITQT_LIBRARY Transaction : public QObject
{
    Q_OBJECT
    Q_FLAGS(TransactionFlag TransactionFlags)
    Q_FLAGS(Filter Filters)
    Q_PROPERTY(QDBusObjectPath tid READ tid)
    Q_PROPERTY(bool allowCancel READ allowCancel NOTIFY allowCancelChanged)
    Q_PROPERTY(bool isCallerActive READ isCallerActive NOTIFY isCallerActiveChanged)
    Q_PROPERTY(QString lastPackage READ lastPackage NOTIFY lastPackageChanged)
    Q_PROPERTY(uint percentage READ percentage NOTIFY percentageChanged)
    Q_PROPERTY(uint elapsedTime READ elapsedTime NOTIFY elapsedTimeChanged)
    Q_PROPERTY(uint remainingTime READ remainingTime NOTIFY remainingTimeChanged)
    Q_PROPERTY(uint speed READ speed NOTIFY speedChanged)
    Q_PROPERTY(qulonglong downloadSizeRemaining READ downloadSizeRemaining NOTIFY downloadSizeRemainingChanged)
    Q_PROPERTY(Role role READ role NOTIFY roleChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(TransactionFlags transactionFlags READ transactionFlags NOTIFY transactionFlagsChanged)
    Q_PROPERTY(QDateTime timespec READ timespec)
    Q_PROPERTY(bool succeeded READ succeeded)
    Q_PROPERTY(uint duration READ duration)
    Q_PROPERTY(QString data READ data)
    Q_PROPERTY(uint uid READ uid NOTIFY uidChanged)
    Q_PROPERTY(QString cmdline READ cmdline)
public:
    /**
     * Describes an error at the daemon level (for example, PackageKit crashes or is unreachable)
     *
     * \sa Transaction::error
     */
    enum InternalError {
        InternalErrorNone = 0,
        InternalErrorUnkown,
        InternalErrorFailed,
        InternalErrorFailedAuth,
        InternalErrorNoTid,
        InternalErrorAlreadyTid,
        InternalErrorRoleUnkown,
        InternalErrorCannotStartDaemon,
        InternalErrorInvalidInput,
        InternalErrorInvalidFile,
        InternalErrorFunctionNotSupported,
        InternalErrorDaemonUnreachable
    };

    /**
     * Describes the role of the transaction
     */
    enum Role {
        RoleUnknown,
        RoleCancel,
        RoleDependsOn,
        RoleGetDetails,
        RoleGetFiles,
        RoleGetPackages,
        RoleGetRepoList,
        RoleRequiredBy,
        RoleGetUpdateDetail,
        RoleGetUpdates,
        RoleInstallFiles,
        RoleInstallPackages,
        RoleInstallSignature,
        RoleRefreshCache,
        RoleRemovePackages,
        RoleRepoEnable,
        RoleRepoSetData,
        RoleResolve,
        RoleSearchDetails,
        RoleSearchFile,
        RoleSearchGroup,
        RoleSearchName,
        RoleUpdatePackages,
        RoleWhatProvides,
        RoleAcceptEula,
        RoleDownloadPackages,
        RoleGetDistroUpgrades,
        RoleGetCategories,
        RoleGetOldTransactions,
        RoleRepairSystem,       // Since 0.7.2
        RoleGetDetailsLocal,    // Since 0.8.17
        RoleGetFilesLocal,      // Since 0.9.1
        RoleRepoRemove,         // Since 0.9.1
        RoleUpgradeSystem       // since 1.0.11
    };
    Q_ENUM(Role)
    typedef Bitfield Roles;

    /**
     * Describes the different types of error
     */
    enum Error {
        ErrorUnknown,
        ErrorOom,
        ErrorNoNetwork,
        ErrorNotSupported,
        ErrorInternalError,
        ErrorGpgFailure,
        ErrorPackageIdInvalid,
        ErrorPackageNotInstalled,
        ErrorPackageNotFound,
        ErrorPackageAlreadyInstalled,
        ErrorPackageDownloadFailed,
        ErrorGroupNotFound,
        ErrorGroupListInvalid,
        ErrorDepResolutionFailed,
        ErrorFilterInvalid,
        ErrorCreateThreadFailed,
        ErrorTransactionError,
        ErrorTransactionCancelled,
        ErrorNoCache,
        ErrorRepoNotFound,
        ErrorCannotRemoveSystemPackage,
        ErrorProcessKill,
        ErrorFailedInitialization,
        ErrorFailedFinalise,
        ErrorFailedConfigParsing,
        ErrorCannotCancel,
        ErrorCannotGetLock,
        ErrorNoPackagesToUpdate,
        ErrorCannotWriteRepoConfig,
        ErrorLocalInstallFailed,
        ErrorBadGpgSignature,
        ErrorMissingGpgSignature,
        ErrorCannotInstallSourcePackage,
        ErrorRepoConfigurationError,
        ErrorNoLicenseAgreement,
        ErrorFileConflicts,
        ErrorPackageConflicts,
        ErrorRepoNotAvailable,
        ErrorInvalidPackageFile,
        ErrorPackageInstallBlocked,
        ErrorPackageCorrupt,
        ErrorAllPackagesAlreadyInstalled,
        ErrorFileNotFound,
        ErrorNoMoreMirrorsToTry,
        ErrorNoDistroUpgradeData,
        ErrorIncompatibleArchitecture,
        ErrorNoSpaceOnDevice,
        ErrorMediaChangeRequired,
        ErrorNotAuthorized,
        ErrorUpdateNotFound,
        ErrorCannotInstallRepoUnsigned,
        ErrorCannotUpdateRepoUnsigned,
        ErrorCannotGetFilelist,
        ErrorCannotGetRequires,
        ErrorCannotDisableRepository,
        ErrorRestrictedDownload,
        ErrorPackageFailedToConfigure,
        ErrorPackageFailedToBuild,
        ErrorPackageFailedToInstall,
        ErrorPackageFailedToRemove,
        ErrorUpdateFailedDueToRunningProcess,
        ErrorPackageDatabaseChanged,
        ErrorProvideTypeNotSupported,
        ErrorInstallRootInvalid,
        ErrorCannotFetchSources,
        ErrorCancelledPriority,
        ErrorUnfinishedTransaction,
        ErrorLockRequired
    };
    Q_ENUM(Error)

    /**
     * Describes how the transaction finished
     * \sa Transaction::finished()
     */
    enum Exit {
        ExitUnknown,
        ExitSuccess,
        ExitFailed,
        ExitCancelled,
        ExitKeyRequired,
        ExitEulaRequired,
        ExitKilled, /* when we forced the cancel, but had to sigkill */
        ExitMediaChangeRequired,
        ExitNeedUntrusted,
        ExitCancelledPriority,
        ExitRepairRequired
    };
    Q_ENUM(Exit)

    /**
     * Describes the different package filters
     */
    enum Filter {
        FilterUnknown        = 0x0000001,
        FilterNone           = 0x0000002,
        FilterInstalled      = 0x0000004,
        FilterNotInstalled   = 0x0000008,
        FilterDevel          = 0x0000010,
        FilterNotDevel       = 0x0000020,
        FilterGui            = 0x0000040,
        FilterNotGui         = 0x0000080,
        FilterFree           = 0x0000100,
        FilterNotFree        = 0x0000200,
        FilterVisible        = 0x0000400,
        FilterNotVisible     = 0x0000800,
        FilterSupported      = 0x0001000,
        FilterNotSupported   = 0x0002000,
        FilterBasename       = 0x0004000,
        FilterNotBasename    = 0x0008000,
        FilterNewest         = 0x0010000,
        FilterNotNewest      = 0x0020000,
        FilterArch           = 0x0040000,
        FilterNotArch        = 0x0080000,
        FilterSource         = 0x0100000,
        FilterNotSource      = 0x0200000,
        FilterCollections    = 0x0400000,
        FilterNotCollections = 0x0800000,
        FilterApplication    = 0x1000000,
        FilterNotApplication = 0x2000000,
        FilterDownloaded     = 0x4000000,
        FilterNotDownloaded  = 0x8000000,
        /* this always has to be at the end of the list */
        FilterLast           = 0x10000000
    };
    Q_ENUM(Filter)
    Q_DECLARE_FLAGS(Filters, Filter)

    /**
     * Describes the current state of the transaction
     */
    enum Status {
        StatusUnknown,
        StatusWait,
        StatusSetup,
        StatusRunning,
        StatusQuery,
        StatusInfo,
        StatusRemove,
        StatusRefreshCache,
        StatusDownload,
        StatusInstall,
        StatusUpdate,
        StatusCleanup,
        StatusObsolete,
        StatusDepResolve,
        StatusSigCheck,
        StatusTestCommit,
        StatusCommit,
        StatusRequest,
        StatusFinished,
        StatusCancel,
        StatusDownloadRepository,
        StatusDownloadPackagelist,
        StatusDownloadFilelist,
        StatusDownloadChangelog,
        StatusDownloadGroup,
        StatusDownloadUpdateinfo,
        StatusRepackaging,
        StatusLoadingCache,
        StatusScanApplications,
        StatusGeneratePackageList,
        StatusWaitingForLock,
        StatusWaitingForAuth,
        StatusScanProcessList,
        StatusCheckExecutableFiles,
        StatusCheckLibraries,
        StatusCopyFiles,
        StatusRunHook
    };
    Q_ENUM(Status)

    /**
     * Describes what kind of media is required
     */
    enum MediaType {
        MediaTypeUnknown,
        MediaTypeCd,
        MediaTypeDvd,
        MediaTypeDisc
    };
    Q_ENUM(MediaType)

    /**
     * Describes an distro upgrade state
     */
    enum DistroUpgrade {
        DistroUpgradeUnknown,
        DistroUpgradeStable,
        DistroUpgradeUnstable
    };
    Q_ENUM(DistroUpgrade)

    /**
     * Describes the type of distribution upgrade to perform
     * \sa Daemon::upgradeSystem()
      */
    enum UpgradeKind {
        UpgradeKindUnknown,
        UpgradeKindMinimal,
        UpgradeKindDefault,
        UpgradeKindComplete
    };
    Q_ENUM(UpgradeKind)

    /**
     * Describes the type of distribution upgrade to perform
     * \sa Daemon::upgradeSystem()
     */
    enum TransactionFlag {
        TransactionFlagNone           = 1 << 0, // Since: 0.8.1
        TransactionFlagOnlyTrusted    = 1 << 1, // Since: 0.8.1
        TransactionFlagSimulate       = 1 << 2, // Since: 0.8.1
        TransactionFlagOnlyDownload   = 1 << 3,  // Since: 1.0.2
        TransactionFlagAllowReinstall = 1 << 4,  // Since: 1.0.2
        TransactionFlagJustReinstall  = 1 << 5,  // Since: 1.0.2
        TransactionFlagAllowDowngrade = 1 << 6  // Since: 0.8.1
    };
    Q_DECLARE_FLAGS(TransactionFlags, TransactionFlag)
    Q_ENUM(TransactionFlag)

    /**
     * Describes a restart type
     */
    enum Restart {
        RestartUnknown,
        RestartNone,
        RestartApplication,
        RestartSession,
        RestartSystem,
        RestartSecuritySession, /* a library that is being used by this package has been updated for security */
        RestartSecuritySystem
    };
    Q_ENUM(Restart)

    /**
     * Describes an update's state
     */
    enum UpdateState {
        UpdateStateUnknown,
        UpdateStateStable,
        UpdateStateUnstable,
        UpdateStateTesting
    };
    Q_ENUM(UpdateState)

    /**
     * Describes the different package groups
     */
    enum Group {
        GroupUnknown,
        GroupAccessibility,
        GroupAccessories,
        GroupAdminTools,
        GroupCommunication,
        GroupDesktopGnome,
        GroupDesktopKde,
        GroupDesktopOther,
        GroupDesktopXfce,
        GroupEducation,
        GroupFonts,
        GroupGames,
        GroupGraphics,
        GroupInternet,
        GroupLegacy,
        GroupLocalization,
        GroupMaps,
        GroupMultimedia,
        GroupNetwork,
        GroupOffice,
        GroupOther,
        GroupPowerManagement,
        GroupProgramming,
        GroupPublishing,
        GroupRepos,
        GroupSecurity,
        GroupServers,
        GroupSystem,
        GroupVirtualization,
        GroupScience,
        GroupDocumentation,
        GroupElectronics,
        GroupCollections,
        GroupVendor,
        GroupNewest
    };
    typedef Bitfield Groups;
    Q_ENUM(Group)

    /**
     * Describes the state of a package
     */
    enum Info {
        InfoUnknown,
        InfoInstalled,
        InfoAvailable,
        InfoLow,
        InfoEnhancement,
        InfoNormal,
        InfoBugfix,
        InfoImportant,
        InfoSecurity,
        InfoBlocked,
        InfoDownloading,
        InfoUpdating,
        InfoInstalling,
        InfoRemoving,
        InfoCleanup,
        InfoObsoleting,
        InfoCollectionInstalled,
        InfoCollectionAvailable,
        InfoFinished,
        InfoReinstalling,
        InfoDowngrading,
        InfoPreparing,
        InfoDecompressing,
        InfoUntrusted,
        InfoTrusted,
        InfoUnavailable
    };
    Q_ENUM(Info)

    /**
     * Describes a signature type
     */
    enum SigType {
        SigTypeUnknown,
        SigTypeGpg
    };
    Q_ENUM(SigType)

    /**
     * Create a transaction object with transaction id \p tid
     *
     * Before using any members wait for roleChanged() signal
     * to be emitted, this is because we ask for the Transaction
     * properties in async mode.
     *
     * The transaction is automatically deleted once finished()
     * is emitted.
     *
     */
    Transaction(const QDBusObjectPath &tid);

    /**
     * Destructor
     */
    ~Transaction();

    /**
     * \brief Returns the TID of the Transaction
     *
     * The TID (Transaction ID) uniquely identifies the transaction.
     *
     * \return the TID of the current transaction
     */
    QDBusObjectPath tid() const;

    /**
     * Indicates whether you can cancel the transaction or not
     * i.e. the backend forbids cancelling the transaction while
     * it's installing packages
     *
     * \return true if you are able cancel the transaction, false else
     */
    bool allowCancel() const;

    /**
     * Indicates whether the transaction caller is active or not
     *
     * The caller can be inactive if it has quit before the transaction finished.
     *
     * \return true if the caller is active, false else
     */
    bool isCallerActive() const;

    /**
     * Returns the last package processed by the transaction
     *
     * This is mostly used when getting an already existing Transaction, to
     * display a more complete summary of the transaction.
     *
     * \return the last package processed by the transaction
     */
    QString lastPackage() const;

    /**
     * The percentage complete of the whole transaction.
     * \return percentage, or 101 if not known.
     */
    uint percentage() const;

    /**
     * The amount of time elapsed during the transaction in seconds.
     * \return time in seconds.
     */
    uint elapsedTime() const;

    /**
     * The estimated time remaining of the transaction in seconds, or 0 if not known.
     * \return time in seconds, or 0 if not known.
     */
    uint remainingTime() const;

    /**
     * Returns the estimated speed of the transaction (copying, downloading, etc.)
     * \return speed bits per second, or 0 if not known.
     */
    uint speed() const;
    
    /**
     * Returns the number of bytes remaining to download
     * \return bytes to download, or 0 if nothing is left to download.
     */
    qulonglong downloadSizeRemaining() const;

    /**
     * Returns information describing the transaction
     * like InstallPackages, SearchName or GetUpdates
     * \return the current role of the transaction
     */
    Role role() const;

    /**
     * Returns the current state of the transaction
     * \return a Transaction::Status value describing the status of the transaction
     */
    Status status() const;

    /**
     * The current flags set in this transaction
     * \return a \sa Transaction::TransactionFlags of this transaction
     */
    TransactionFlags transactionFlags() const;

    /**
     * Returns the date at which the transaction was created
     * \return a QDateTime object containing the date at which the transaction was created
     * \note This function only returns a real value for old transactions returned by getOldTransactions
     */
    QDateTime timespec() const;

    /**
     * Returns whether the transaction succeded or not
     * \return true if the transaction succeeded, false else
     * \note This function only returns a real value for old transactions returned by getOldTransactions
     */
    bool succeeded() const;

    /**
     * Returns the time the transaction took to finish
     * \return the number of milliseconds the transaction took to finish
     * \note This function only returns a real value for old transactions returned by getOldTransactions
     */
    uint duration() const;

    /**
     * Returns some data set by the backend to pass additional information
     * \return a string set by the backend
     * \note This function only returns a real value for old transactions returned by getOldTransactions
     */
    QString data() const;

    /**
     * Returns the UID of the calling process
     * \return the uid of the calling process
     * \note This function only returns a real value for old transactions returned by getOldTransactions
     */
    uint uid() const;

    /**
     * Returns the command line for the calling process
     * \return a string of the command line for the calling process
     * \note This function only returns a real value for old transactions returned by getOldTransactions
     */
    QString cmdline() const;

    /**
     * \brief Tells the underlying package manager to use the given \p hints
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
     * \sa Daemon::setHints
     */
    QDBusPendingReply<> setHints(const QStringList &hints);

    /**
     * Convenience function to set this transaction \p hints
     * \sa getDetails(const QStringList &hints)
     */
    QDBusPendingReply<> setHints(const QString &hints);

    /**
     * Cancels the transaction
     *
     * \warning check \sa errorCode() signal to know if it the call has any error
     */
    Q_INVOKABLE QDBusPendingReply<> cancel();

    /**
     * Returns the package name from the \p packageID
     */
    static QString packageName(const QString &packageID);

    /**
     * Returns the package version from the \p packageID
     */
    static QString packageVersion(const QString &packageID);

    /**
     * Returns the package arch from the \p packageID
     */
    static QString packageArch(const QString &packageID);

    /**
     * Returns the package data from the \p packageID
     */
    static QString packageData(const QString &packageID);

Q_SIGNALS:
    void allowCancelChanged();

    void isCallerActiveChanged();

    void downloadSizeRemainingChanged();

    void elapsedTimeChanged();

    void lastPackageChanged();

    void percentageChanged();

    void remainingTimeChanged();

    void roleChanged();

    void speedChanged();

    void statusChanged();

    void transactionFlagsChanged();

    void uidChanged();

    /**
     * \brief Sends a category
     *
     * \li \p parentId is the id of the parent category. A blank parent means a root category
     * \li \p categoryId is the id of the category
     * \li \p name is the category's name. This name is localized.
     * \li \p summary is the category's summary. It is localized.
     * \li \p icon is the icon identifier eg. server-cfg. If unknown, it is set to icon-missing.
     *
     * \sa getCategories()
     */
    void category(const QString &parentId, const QString &categoryId, const QString &name, const QString &summary, const QString &icon);

    /**
     * Emitted when a distribution upgrade is available
     * \sa getDistroUpgrades()
     */
    void distroUpgrade(PackageKit::Transaction::DistroUpgrade type, const QString &name, const QString &description);

    /**
     * Emitted when an error occurs
     */
    void errorCode(PackageKit::Transaction::Error error, const QString &details);

    /**
     * Emitted when an EULA agreement prevents the transaction from running
     * \li \c eulaId is the EULA identifier
     * \li \c package is the package for which an EULA is required
     * \li \c vendorName is the vendor name
     * \li \c licenseAgreement is the EULA text
     *
     * \note You will need to relaunch the transaction after accepting the EULA
     * \sa acceptEula()
     */
    void eulaRequired(const QString &eulaID, const QString &packageID, const QString &vendor, const QString &licenseAgreement);

    /**
     * Emitted when a different media is required in order to fetch packages
     * which prevents the transaction from running
     * \note You will need to relaunch the transaction after changing the media
     * \sa Transaction::MediaType
     */
    void mediaChangeRequired(PackageKit::Transaction::MediaType type, const QString &id, const QString &text);

    /**
     * Sends the \p item current progress \p percentage
     * Currently only a package id is emitted
     */
    void itemProgress(const QString &itemID, PackageKit::Transaction::Status status, uint percentage);

    /**
     * Sends the \p filenames contained in package \p package
     * \sa getFiles()
     */
    void files(const QString &packageID, const QStringList &filenames);

    /**
     * Emitted when the transaction finishes
     *
     * \p status describes the exit status, \p runtime is the number of seconds it took to complete the transaction
     */
    void finished(PackageKit::Transaction::Exit status, uint runtime);

    /**
     * Emitted when the transaction sends a new package
     */
    void package(PackageKit::Transaction::Info info, const QString &packageID, const QString &summary);

    /**
     * Emitted when the transaction sends details of a package
     */
    void details(const PackageKit::Details &values);

    /**
     * Emitted when the transaction sends details of an update
     */
    void updateDetail(const QString &packageID,
                      const QStringList &updates,
                      const QStringList &obsoletes,
                      const QStringList &vendorUrls,
                      const QStringList &bugzillaUrls,
                      const QStringList &cveUrls,
                      PackageKit::Transaction::Restart restart,
                      const QString &updateText,
                      const QString &changelog,
                      PackageKit::Transaction::UpdateState state,
                      const QDateTime &issued,
                      const QDateTime &updated);

    /**
      * Sends some additional details about a software repository
      * \sa getRepoList()
      */
    void repoDetail(const QString &repoId, const QString &description, bool enabled);

    /**
     * Emitted when the user has to validate a repository's signature
     * \sa installSignature()
     */
    void repoSignatureRequired(const QString &packageID,
                               const QString &repoName,
                               const QString &keyUrl,
                               const QString &keyUserid,
                               const QString &keyId,
                               const QString &keyFingerprint,
                               const QString &keyTimestamp,
                               PackageKit::Transaction::SigType type);

    /**
     * Indicates that a restart is required
     * \p package is the package who triggered the restart signal
     */
    void requireRestart(PackageKit::Transaction::Restart type, const QString &packageID);

    /**
     * Sends an old transaction
     * \sa getOldTransactions()
     *
     * These objects must be manually deleted
     */
    void transaction(PackageKit::Transaction *transaction);

protected:
    static Transaction::InternalError parseError(const QString &errorName);

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

    TransactionPrivate * const d_ptr;

    /**
     * Creates a new transaction object
     * asking PackageKit for a new TID
     *
     * This constructor is used by Daemon
     *
     */
    Transaction();

    Transaction(TransactionPrivate *d);

private:
    friend class Daemon;
    Q_DECLARE_PRIVATE(Transaction)
    Q_DISABLE_COPY(Transaction)
    Q_PRIVATE_SLOT(d_func(), void distroUpgrade(uint type, const QString &name, const QString &description))
    Q_PRIVATE_SLOT(d_func(), void details(const QVariantMap &values))
    Q_PRIVATE_SLOT(d_func(), void errorCode(uint error, const QString &details))
    Q_PRIVATE_SLOT(d_func(), void mediaChangeRequired(uint mediaType, const QString &mediaId, const QString &mediaText))
    Q_PRIVATE_SLOT(d_func(), void finished(uint exitCode, uint runtime))
    Q_PRIVATE_SLOT(d_func(), void Package(uint info, const QString &pid, const QString &summary))
    Q_PRIVATE_SLOT(d_func(), void ItemProgress(const QString &itemID, uint status, uint percentage))
    Q_PRIVATE_SLOT(d_func(), void RepoSignatureRequired(const QString &pid, const QString &repoName, const QString &keyUrl, const QString &keyUserid, const QString &keyId, const QString &keyFingerprint, const QString &keyTimestamp, uint type))
    Q_PRIVATE_SLOT(d_func(), void requireRestart(uint type, const QString &pid))
    Q_PRIVATE_SLOT(d_func(), void transaction(const QDBusObjectPath &oldTid, const QString &timespec, bool succeeded, uint role, uint duration, const QString &data, uint uid, const QString &cmdline))
    Q_PRIVATE_SLOT(d_func(), void UpdateDetail(const QString &package_id, const QStringList &updates, const QStringList &obsoletes, const QStringList &vendor_urls, const QStringList &bugzilla_urls, const QStringList &cve_urls, uint restart, const QString &update_text, const QString &changelog, uint state, const QString &issued, const QString &updated))
    Q_PRIVATE_SLOT(d_func(), void destroy())
    Q_PRIVATE_SLOT(d_func(), void daemonQuit())
    Q_PRIVATE_SLOT(d_func(), void propertiesChanged(QString,QVariantMap,QStringList))
    Q_PRIVATE_SLOT(d_func(), void updateProperties(QVariantMap))
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Transaction::Filters)
Q_DECLARE_OPERATORS_FOR_FLAGS(Transaction::TransactionFlags)

} // End namespace PackageKit
Q_DECLARE_METATYPE(PackageKit::Transaction::InternalError)
Q_DECLARE_METATYPE(PackageKit::Transaction::Role)
Q_DECLARE_METATYPE(PackageKit::Transaction::Error)
Q_DECLARE_METATYPE(PackageKit::Transaction::Exit)
Q_DECLARE_METATYPE(PackageKit::Transaction::Status)
Q_DECLARE_METATYPE(PackageKit::Transaction::MediaType)
Q_DECLARE_METATYPE(PackageKit::Transaction::DistroUpgrade)
Q_DECLARE_METATYPE(PackageKit::Transaction::TransactionFlag)
Q_DECLARE_METATYPE(PackageKit::Transaction::Restart)
Q_DECLARE_METATYPE(PackageKit::Transaction::UpdateState)
Q_DECLARE_METATYPE(PackageKit::Transaction::Group)
Q_DECLARE_METATYPE(PackageKit::Transaction::Info)
Q_DECLARE_METATYPE(PackageKit::Transaction::SigType)
Q_DECLARE_METATYPE(PackageKit::Transaction::Filter)
Q_DECLARE_METATYPE(PackageKit::Transaction::TransactionFlags)
Q_DECLARE_METATYPE(PackageKit::Transaction::Filters)
Q_DECLARE_METATYPE(PackageKit::Transaction::UpgradeKind)

#endif
