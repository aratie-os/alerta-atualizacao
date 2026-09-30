#pragma once

#include <QString>

struct UpdateState {
    bool aptUpdateSucceeded = false;
    bool aptQuerySucceeded = false;
    bool aptHasUpdates = false;

    bool flatpakSystemQuerySucceeded = false;
    bool flatpakSystemHasUpdates = false;

    bool flatpakUserQuerySucceeded = false;
    bool flatpakUserHasUpdates = false;

    [[nodiscard]] bool hasUpdates() const;
};

[[nodiscard]] bool aptOutputHasUpdates(const QString &standardOutput);
[[nodiscard]] bool flatpakOutputHasUpdates(const QString &standardOutput);
