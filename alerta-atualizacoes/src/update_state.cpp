#include "update_state.h"

#include <QRegularExpression>

namespace {
const QRegularExpression flatpakReferenceLine(
    QStringLiteral(R"(^[^\s/]+/[^\s/]+/[^\s/]+$)"));
}

bool UpdateState::hasUpdates() const
{
    return aptHasUpdates || flatpakSystemHasUpdates || flatpakUserHasUpdates;
}

bool aptSimulationHasUpdates(const QString &standardOutput)
{
    const QStringList lines = standardOutput.split(QChar::LineFeed);
    for (const QString &rawLine : lines) {
        QString line = rawLine;
        if (line.endsWith(QChar::CarriageReturn)) {
            line.chop(1);
        }
        if (line.startsWith(QStringLiteral("Inst "))) {
            return true;
        }
    }
    return false;
}

bool flatpakOutputHasUpdates(const QString &standardOutput)
{
    const QStringList lines = standardOutput.split(QChar::LineFeed);
    for (const QString &rawLine : lines) {
        const QString line = rawLine.trimmed();
        if (!line.isEmpty() && flatpakReferenceLine.match(line).hasMatch()) {
            return true;
        }
    }
    return false;
}
