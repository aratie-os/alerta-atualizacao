#include "startup.h"
#include "update_state.h"
#include "window_placement.h"

#include <QTest>

#include <cstdlib>

class UpdateCheckerTests final : public QObject
{
    Q_OBJECT

private slots:
    void aptListingHeaderIsNotAnUpdate();
    void aptPackageLineIsAnUpdate();
    void flatpakEmptyOutputIsNotAnUpdate();
    void flatpakReferenceIsAnUpdate();
    void scenarioAListingAndEmptyFlatpakDoesNotShowWindow();
    void scenarioBAptPackageShowsWindow();
    void scenarioCFlatpakReferenceShowsWindow();
    void scenarioDAllCommandsSucceedWithoutUpdatesDoesNotShowWindow();
    void liveCommandLineIsDetected();
    void installedCommandLineIsNotLive();
    void partialCasperTokenIsNotLive();
    void liveGuardDoesNotStartInstalledSession();
    void x11PositionUsesBottomRightMargin();
    void waylandSettingsUseBottomRightAnchors();
};

void UpdateCheckerTests::aptListingHeaderIsNotAnUpdate()
{
    QVERIFY(!aptOutputHasUpdates(QStringLiteral("Listing...\n")));
    QVERIFY(!aptOutputHasUpdates(
        QStringLiteral("WARNING: apt does not have a stable CLI interface.\nListing...\n")));
}

void UpdateCheckerTests::aptPackageLineIsAnUpdate()
{
    QVERIFY(aptOutputHasUpdates(QStringLiteral(
        "example/resolute-updates 2.0-1 amd64 [upgradable from: 1.0-1]\n")));
}

void UpdateCheckerTests::flatpakEmptyOutputIsNotAnUpdate()
{
    QVERIFY(!flatpakOutputHasUpdates({}));
    QVERIFY(!flatpakOutputHasUpdates(QStringLiteral("  \n\n")));
}

void UpdateCheckerTests::flatpakReferenceIsAnUpdate()
{
    QVERIFY(flatpakOutputHasUpdates(
        QStringLiteral("org.example.Application/x86_64/stable\n")));
}

void UpdateCheckerTests::scenarioAListingAndEmptyFlatpakDoesNotShowWindow()
{
    UpdateState state;
    state.aptQuerySucceeded = true;
    state.aptHasUpdates = aptOutputHasUpdates(QStringLiteral("Listing...\n"));
    state.flatpakSystemQuerySucceeded = true;
    state.flatpakSystemHasUpdates = flatpakOutputHasUpdates({});

    QVERIFY(!state.hasUpdates());
}

void UpdateCheckerTests::scenarioBAptPackageShowsWindow()
{
    UpdateState state;
    state.aptQuerySucceeded = true;
    state.aptHasUpdates = aptOutputHasUpdates(QStringLiteral(
        "example/resolute-updates 2.0-1 amd64 [upgradable from: 1.0-1]\n"));
    QVERIFY(state.hasUpdates());
}

void UpdateCheckerTests::scenarioCFlatpakReferenceShowsWindow()
{
    UpdateState state;
    state.flatpakSystemQuerySucceeded = true;
    state.flatpakSystemHasUpdates = flatpakOutputHasUpdates(
        QStringLiteral("org.example.Application/x86_64/stable\n"));
    QVERIFY(state.hasUpdates());
}

void UpdateCheckerTests::scenarioDAllCommandsSucceedWithoutUpdatesDoesNotShowWindow()
{
    UpdateState state;
    state.aptUpdateSucceeded = true;
    state.aptQuerySucceeded = true;
    state.flatpakSystemQuerySucceeded = true;
    state.flatpakUserQuerySucceeded = true;

    QVERIFY(!state.aptHasUpdates);
    QVERIFY(!state.flatpakSystemHasUpdates);
    QVERIFY(!state.flatpakUserHasUpdates);
    QVERIFY(!state.hasUpdates());
}

void UpdateCheckerTests::liveCommandLineIsDetected()
{
    QVERIFY(commandLineIndicatesLiveSession(
        QStringLiteral("BOOT_IMAGE=/casper/vmlinuz boot=casper quiet splash ---")));
    QVERIFY(commandLineIndicatesLiveSession(
        QStringLiteral("BOOT_IMAGE=/casper/vmlinuz quiet splash boot=casper\n")));
}

void UpdateCheckerTests::installedCommandLineIsNotLive()
{
    QVERIFY(!commandLineIndicatesLiveSession(
        QStringLiteral("BOOT_IMAGE=/boot/vmlinuz-6.17 root=UUID=abc ro quiet splash")));
}

void UpdateCheckerTests::partialCasperTokenIsNotLive()
{
    QVERIFY(!commandLineIndicatesLiveSession(
        QStringLiteral("BOOT_IMAGE=/boot/vmlinuz foo=boot=casper boot=casper-test")));
}

void UpdateCheckerTests::liveGuardDoesNotStartInstalledSession()
{
    bool installedSessionStarted = false;
    const int result = runWithLiveSessionGuard(
        [] { return true; },
        [&] {
            installedSessionStarted = true;
            return 1;
        });

    QCOMPARE(result, EXIT_SUCCESS);
    QVERIFY(!installedSessionStarted);
}

void UpdateCheckerTests::x11PositionUsesBottomRightMargin()
{
    const QPoint position = bottomRightPosition(QRect(100, 50, 1920, 1040),
                                                QSize(450, 160));
    QCOMPARE(position, QPoint(1546, 906));
}

void UpdateCheckerTests::waylandSettingsUseBottomRightAnchors()
{
    const LayerShellSettings settings = notificationLayerShellSettings();
    QVERIFY(settings.anchors.testFlag(Qt::BottomEdge));
    QVERIFY(settings.anchors.testFlag(Qt::RightEdge));
    QVERIFY(!settings.anchors.testFlag(Qt::TopEdge));
    QVERIFY(!settings.anchors.testFlag(Qt::LeftEdge));
    QCOMPARE(settings.margins, QMargins(0, 0, 24, 24));
    QCOMPARE(settings.exclusiveZone, 0);
}

QTEST_GUILESS_MAIN(UpdateCheckerTests)

#include "updatechecker_tests.moc"
