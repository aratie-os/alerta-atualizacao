#include "startup.h"

#include <QFile>
#include <QLoggingCategory>

#include <cstdlib>

Q_LOGGING_CATEGORY(logStartup, "system-upgrade.startup")

bool commandLineIndicatesLiveSession(const QString &commandLine)
{
    const QStringList arguments = commandLine.trimmed().split(QChar::Space, Qt::SkipEmptyParts);
    for (const QString &argument : arguments) {
        if (argument == QStringLiteral("boot=casper")) {
            return true;
        }
    }
    return false;
}

bool isLiveSession()
{
    QFile commandLineFile(QStringLiteral("/proc/cmdline"));
    if (!commandLineFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(logStartup) << "Could not read /proc/cmdline:" << commandLineFile.errorString();
        return false;
    }
    return commandLineIndicatesLiveSession(QString::fromLocal8Bit(commandLineFile.readAll()));
}

int runWithLiveSessionGuard(const std::function<bool()> &liveDetector,
                            const std::function<int()> &installedSession)
{
    if (liveDetector()) {
        return EXIT_SUCCESS;
    }
    return installedSession();
}
