#pragma once

#include <QString>

#include <functional>

[[nodiscard]] bool commandLineIndicatesLiveSession(const QString &commandLine);
[[nodiscard]] bool isLiveSession();
[[nodiscard]] int runWithLiveSessionGuard(const std::function<bool()> &liveDetector,
                                          const std::function<int()> &installedSession);
