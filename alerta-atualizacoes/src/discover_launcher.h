#pragma once

#include <QProcessEnvironment>

class QProcess;

void configureDiscoverProcess(QProcess &process,
                              const QProcessEnvironment &sessionEnvironment);

[[nodiscard]] bool startDiscoverDetached(
    const QProcessEnvironment &sessionEnvironment);
