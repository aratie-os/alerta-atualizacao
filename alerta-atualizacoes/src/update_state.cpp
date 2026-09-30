#include "update_state.h"

#include <QRegularExpression>

namespace {
const QRegularExpression aptPackageLine(
    QStringLiteral(R"(^[a-z0-9][a-z0-9+.-]*(?::[a-z0-9][a-z0-9-]*)?/[\S]+\s+\S+\s+\S+\s+\[upgradable from:\s+.+\]$)"),
    QRegularExpression::CaseInsensitiveOption);

const QRegularExpression flatpakReferenceLine(
    QStringLiteral(R"(^[^\s/]+/[^\s/]+/[^\s/]+$)"));
}

bool UpdateState::hasUpdates() const
{
    return aptHasUpdates || flatpakSystemHasUpdates || flatpakUserHasUpdates;
}

bool aptOutputHasUpdates(const QString &standardOutput)
{
    const QStringList lines = standardOutput.split(QChar::LineFeed);
    for (const QString &rawLine : lines) {
        const QString line = rawLine.trimmed();
        if (line.isEmpty() || line == QStringLiteral("Listing...")
            || line.startsWith(QStringLiteral("WARNING:"), Qt::CaseInsensitive)) {
            continue;
        }
        if (aptPackageLine.match(line).hasMatch()) {
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
