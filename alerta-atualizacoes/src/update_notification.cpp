#include "update_notification.h"

#include "window_placement.h"

#include <LayerShellQt/Window>

#include <QGuiApplication>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QScreen>
#include <QSvgRenderer>
#include <QVBoxLayout>
#include <QWindow>

namespace {
constexpr auto preferredIconPath = "/usr/share/icons/kora-cyan/status/scalable/package-broken.svg";
constexpr int alertIconSize = 64;

class AlertIconWidget final : public QWidget
{
public:
    explicit AlertIconWidget(QWidget *parent = nullptr)
        : QWidget(parent)
        , m_renderer(QString::fromLatin1(preferredIconPath), this)
        , m_fallback(QIcon::fromTheme(QStringLiteral("dialog-warning")))
    {
        setFixedSize(alertIconSize, alertIconSize);
        setAccessibleName(tr("Alerta de atualização"));
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        if (m_renderer.isValid()) {
            const QSize sourceSize = m_renderer.defaultSize();
            QSize targetSize = sourceSize.isValid()
                ? sourceSize.scaled(size(), Qt::KeepAspectRatio)
                : size();
            const QRect targetRect(QPoint((width() - targetSize.width()) / 2,
                                          (height() - targetSize.height()) / 2),
                                   targetSize);
            m_renderer.render(&painter, targetRect);
            return;
        }
        m_fallback.paint(&painter, rect(), Qt::AlignCenter, QIcon::Normal, QIcon::On);
    }

private:
    QSvgRenderer m_renderer;
    QIcon m_fallback;
};
}

UpdateNotification::UpdateNotification(bool waylandSession, QWidget *parent)
    : QDialog(parent)
    , m_waylandSession(waylandSession)
{
    setWindowTitle(tr("Atualizações disponíveis"));
    QIcon windowIcon(QString::fromLatin1(preferredIconPath));
    if (windowIcon.isNull()) {
        windowIcon = QIcon::fromTheme(QStringLiteral("dialog-warning"));
    }
    setWindowIcon(windowIcon);
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setModal(false);
    setMinimumWidth(420);
    setMaximumWidth(520);

    auto *titleLabel = new QLabel(tr("Atualizações disponíveis"), this);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSizeF(titleFont.pointSizeF() + 1.0);
    titleLabel->setFont(titleFont);

    auto *messageLabel = new QLabel(
        tr("Existem atualizações disponíveis para o sistema."), this);
    messageLabel->setWordWrap(true);
    messageLabel->setMinimumWidth(290);

    auto *updateButton = new QPushButton(
        QIcon::fromTheme(QStringLiteral("system-software-update")), tr("Atualizar"), this);
    auto *cancelButton = new QPushButton(
        QIcon::fromTheme(QStringLiteral("dialog-cancel")), tr("Cancelar"), this);
    updateButton->setDefault(true);

    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->addWidget(updateButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();

    auto *textLayout = new QVBoxLayout;
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(10);
    textLayout->addWidget(titleLabel);
    textLayout->addWidget(messageLabel);
    textLayout->addStretch();
    textLayout->addLayout(buttonLayout);

    auto *rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(20, 18, 20, 18);
    rootLayout->setSpacing(18);
    rootLayout->addLayout(textLayout, 1);
    rootLayout->addWidget(new AlertIconWidget(this), 0, Qt::AlignVCenter);

    connect(updateButton, &QPushButton::clicked, this, &UpdateNotification::updateRequested);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void UpdateNotification::showInPrimaryScreenCorner()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    adjustSize();

    if (m_waylandSession) {
        createWinId();
        QWindow *nativeWindow = windowHandle();
        if (nativeWindow) {
            nativeWindow->setScreen(screen);
            LayerShellQt::Window *layerWindow = LayerShellQt::Window::get(nativeWindow);
            const LayerShellSettings settings = notificationLayerShellSettings();

            LayerShellQt::Window::Anchors anchors = LayerShellQt::Window::AnchorNone;
            if (settings.anchors.testFlag(Qt::BottomEdge)) {
                anchors |= LayerShellQt::Window::AnchorBottom;
            }
            if (settings.anchors.testFlag(Qt::RightEdge)) {
                anchors |= LayerShellQt::Window::AnchorRight;
            }

            layerWindow->setAnchors(anchors);
            layerWindow->setMargins(settings.margins);
            layerWindow->setExclusiveZone(settings.exclusiveZone);
            layerWindow->setLayer(LayerShellQt::Window::LayerTop);
            layerWindow->setKeyboardInteractivity(
                LayerShellQt::Window::KeyboardInteractivityOnDemand);
            layerWindow->setActivateOnShow(true);
            layerWindow->setScreen(screen);
            layerWindow->setScope(QStringLiteral("system-upgrade"));
            layerWindow->setDesiredSize(size());
        }
    } else if (screen) {
        move(bottomRightPosition(screen->availableGeometry(), size()));
    }

    show();
}
