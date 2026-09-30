#include "update_checker.h"

#include <QLoggingCategory>
#include <QProcessEnvironment>

Q_LOGGING_CATEGORY(logUpdateChecker, "system-upgrade.checker")

namespace {
constexpr int aptUpdateTimeoutMilliseconds = 300'000;
constexpr int queryTimeoutMilliseconds = 60'000;
}

UpdateChecker::UpdateChecker(QObject *parent)
    : QObject(parent)
{
    m_timeout.setSingleShot(true);

    connect(&m_process,
            &QProcess::started,
            this,
            [this] { m_started = true; });
    connect(&m_process,
            qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this,
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
                if (m_stepCompleted) {
                    return;
                }
                m_timeout.stop();
                m_stepCompleted = true;
                const bool succeeded = m_started && exitStatus == QProcess::NormalExit
                    && exitCode == 0;
                completeStep(succeeded,
                             QString::fromLocal8Bit(m_process.readAllStandardOutput()),
                             QString::fromLocal8Bit(m_process.readAllStandardError()));
            });
    connect(&m_process,
            &QProcess::errorOccurred,
            this,
            [this](QProcess::ProcessError error) {
                if (error != QProcess::FailedToStart || m_stepCompleted) {
                    return;
                }
                m_timeout.stop();
                m_stepCompleted = true;
                completeStep(false, {}, m_process.errorString());
            });
    connect(&m_timeout, &QTimer::timeout, this, [this] {
        if (m_stepCompleted) {
            return;
        }
        qCWarning(logUpdateChecker) << "Update command timed out:" << m_process.program();
        m_process.kill();
    });
}

void UpdateChecker::start()
{
    if (m_step != Step::Idle) {
        return;
    }
    m_step = Step::AptUpdate;
    runNextStep();
}

void UpdateChecker::runCommand(Step step,
                               const QString &program,
                               const QStringList &arguments,
                               int timeoutMilliseconds)
{
    m_step = step;
    m_started = false;
    m_stepCompleted = false;

    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("LC_ALL"), QStringLiteral("C"));
    environment.insert(QStringLiteral("LANG"), QStringLiteral("C"));
    m_process.setProcessEnvironment(environment);
    m_process.setProgram(program);
    m_process.setArguments(arguments);
    m_timeout.start(timeoutMilliseconds);
    m_process.start();
}

void UpdateChecker::completeStep(bool commandSucceeded,
                                 const QString &standardOutput,
                                 const QString &standardError)
{
    if (!commandSucceeded) {
        qCWarning(logUpdateChecker).noquote()
            << "Update command failed:" << m_process.program()
            << standardError.simplified();
    }

    switch (m_step) {
    case Step::AptUpdate:
        m_state.aptUpdateSucceeded = commandSucceeded;
        m_state.aptHasUpdates = false;
        m_step = commandSucceeded ? Step::AptQuery : Step::FlatpakSystem;
        break;
    case Step::AptQuery:
        m_state.aptQuerySucceeded = commandSucceeded;
        m_state.aptHasUpdates = commandSucceeded && aptSimulationHasUpdates(standardOutput);
        m_step = Step::FlatpakSystem;
        break;
    case Step::FlatpakSystem:
        m_state.flatpakSystemQuerySucceeded = commandSucceeded;
        m_state.flatpakSystemHasUpdates = commandSucceeded
            && flatpakOutputHasUpdates(standardOutput);
        m_step = Step::FlatpakUser;
        break;
    case Step::FlatpakUser:
        m_state.flatpakUserQuerySucceeded = commandSucceeded;
        m_state.flatpakUserHasUpdates = commandSucceeded
            && flatpakOutputHasUpdates(standardOutput);
        m_step = Step::Complete;
        break;
    case Step::Idle:
    case Step::Complete:
        return;
    }

    runNextStep();
}

void UpdateChecker::runNextStep()
{
    switch (m_step) {
    case Step::AptUpdate:
        runCommand(Step::AptUpdate,
                   QStringLiteral("/usr/bin/sudo"),
                   {QStringLiteral("-n"),
                    QStringLiteral("/usr/bin/apt-get"),
                    QStringLiteral("-o"),
                    QStringLiteral("Acquire::Retries=3"),
                    QStringLiteral("-o"),
                    QStringLiteral("APT::Update::Error-Mode=any"),
                    QStringLiteral("update")},
                   aptUpdateTimeoutMilliseconds);
        break;
    case Step::AptQuery:
        runCommand(Step::AptQuery,
                   QStringLiteral("/usr/bin/apt-get"),
                   {QStringLiteral("-s"),
                    QStringLiteral("-o"),
                    QStringLiteral("Debug::NoLocking=1"),
                    QStringLiteral("upgrade")},
                   queryTimeoutMilliseconds);
        break;
    case Step::FlatpakSystem:
        runCommand(Step::FlatpakSystem,
                   QStringLiteral("/usr/bin/flatpak"),
                   {QStringLiteral("--system"),
                    QStringLiteral("remote-ls"),
                    QStringLiteral("--updates"),
                    QStringLiteral("--columns=ref")},
                   queryTimeoutMilliseconds);
        break;
    case Step::FlatpakUser:
        runCommand(Step::FlatpakUser,
                   QStringLiteral("/usr/bin/flatpak"),
                   {QStringLiteral("--user"),
                    QStringLiteral("remote-ls"),
                    QStringLiteral("--updates"),
                    QStringLiteral("--columns=ref")},
                   queryTimeoutMilliseconds);
        break;
    case Step::Complete:
        emit finished(m_state);
        break;
    case Step::Idle:
        break;
    }
}
