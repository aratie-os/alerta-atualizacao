#pragma once

#include "update_state.h"

#include <QObject>
#include <QProcess>
#include <QTimer>

class UpdateChecker final : public QObject
{
    Q_OBJECT

public:
    explicit UpdateChecker(QObject *parent = nullptr);

    void start();

signals:
    void finished(const UpdateState &state);

private:
    enum class Step {
        Idle,
        AptUpdate,
        AptQuery,
        FlatpakSystem,
        FlatpakUser,
        Complete,
    };

    void runCommand(Step step,
                    const QString &program,
                    const QStringList &arguments,
                    int timeoutMilliseconds);
    void completeStep(bool commandSucceeded,
                      const QString &standardOutput,
                      const QString &standardError);
    void runNextStep();

    QProcess m_process;
    QTimer m_timeout;
    UpdateState m_state;
    Step m_step = Step::Idle;
    bool m_started = false;
    bool m_stepCompleted = false;
};
