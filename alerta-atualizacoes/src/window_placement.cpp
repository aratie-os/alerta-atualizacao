#include "window_placement.h"

QPoint bottomRightPosition(const QRect &availableGeometry, const QSize &windowSize, int margin)
{
    return {
        availableGeometry.right() - windowSize.width() + 1 - margin,
        availableGeometry.bottom() - windowSize.height() + 1 - margin,
    };
}

LayerShellSettings notificationLayerShellSettings()
{
    return {
        Qt::BottomEdge | Qt::RightEdge,
        QMargins(0, 0, 24, 24),
        0,
    };
}
