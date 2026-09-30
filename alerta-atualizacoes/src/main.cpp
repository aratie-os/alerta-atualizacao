#include "discover_launcher.h"
#include "startup.h"
#include "update_checker.h"
#include "update_notification.h"

#include <LayerShellQt/Shell>

#include <QApplication>
#include <QDir>
#include <QGuiApplication>
#include <QLockFile>
#include <QLoggingCategory>
#include <QProcessEnvironment>
#include <QStandardPaths>

#include <cstdlib>
#include <memory>

Q_LOGGING_CATEGORY(logApplication, "system-upgrade.application")

namespace {
constexpr auto discoverPath = "/usr/bin/plasma-discover";

QString lockFilePath()
{
    QString runtimeDirectory = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    if (runtimeDirectory.isEmpty()) {
        runtimeDirectory = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    }
    return QDir(runtimeDirectory).filePath(QStringLiteral("system-upgrade.lock"));
}

void openDiscover(const QProcessEnvironment &sessionEnvironment)
{
    if (!startDiscoverDetached(sessionEnvironment)) {
        qCWarning(logApplication) << "Could not start" << discoverPath;
    }
}
}

int runInstalledSession(int argc, char *argv[])
{
    QApplication application(argc, argv);
    QApplication::setApplicationName(QStringLiteral("system-upgrade"));
    QApplication::setApplicationDisplayName(QObject::tr("Atualizações disponíveis"));
    QApplication::setOrganizationName(QStringLiteral("Aratie OS"));
    QApplication::setQuitOnLastWindowClosed(true);

    QLockFile instanceLock(lockFilePath());
    instanceLock.setStaleLockTime(0);
    if (!instanceLock.tryLock()) {
        qCWarning(logApplication) << "Another update check is already running";
        return EXIT_SUCCESS;
    }

    const QProcessEnvironment sessionEnvironment = QProcessEnvironment::systemEnvironment();
    const bool waylandSession = QGuiApplication::platformName().contains(
        QStringLiteral("wayland"), Qt::CaseInsensitive);
    if (waylandSession) {
        LayerShellQt::Shell::useLayerShell();
    }

    UpdateChecker checker;
    std::unique_ptr<UpdateNotification> notification;

    QObject::connect(&checker,
                     &UpdateChecker::finished,
                     &application,
                     [&, sessionEnvironment](const UpdateState &state) {
                         const bool hasUpdates = state.hasUpdates();
                         if (!hasUpdates) {
                             application.quit();
                             return;
                         }

                         notification = std::make_unique<UpdateNotification>(waylandSession);
                         QObject::connect(notification.get(),
                                          &UpdateNotification::updateRequested,
                                          &application,
                                          [&application, sessionEnvironment] {
                                              openDiscover(sessionEnvironment);
                                              application.quit();
                                          });
                         QObject::connect(notification.get(),
                                          &QDialog::rejected,
                                          &application,
                                          &QApplication::quit);
                         notification->showInPrimaryScreenCorner();
                     });

    checker.start();
    return application.exec();
}

int main(int argc, char *argv[])
{
    // The installed-session callback contains QApplication, locks, commands and all UI.
    return runWithLiveSessionGuard(
        [] { return isLiveSession(); },
        [&] { return runInstalledSession(argc, argv); });
}
