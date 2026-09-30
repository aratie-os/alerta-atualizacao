#pragma once

#include <QMargins>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <Qt>

struct LayerShellSettings {
    Qt::Edges anchors;
    QMargins margins;
    int exclusiveZone;
};

[[nodiscard]] QPoint bottomRightPosition(const QRect &availableGeometry,
                                         const QSize &windowSize,
                                         int margin = 24);
[[nodiscard]] LayerShellSettings notificationLayerShellSettings();
