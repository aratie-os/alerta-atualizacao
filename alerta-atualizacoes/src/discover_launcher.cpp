#include "discover_launcher.h"

#include <QProcess>

void configureDiscoverProcess(QProcess &process,
                              const QProcessEnvironment &sessionEnvironment)
{
    process.setProgram(QStringLiteral("/usr/bin/plasma-discover"));
    process.setArguments({QStringLiteral("--mode"), QStringLiteral("Update")});
    process.setProcessEnvironment(sessionEnvironment);
}

bool startDiscoverDetached(const QProcessEnvironment &sessionEnvironment)
{
    QProcess process;
    configureDiscoverProcess(process, sessionEnvironment);
    return process.startDetached();
}
