#include "discover_launcher.h"
#include "startup.h"
#include "update_state.h"
#include "window_placement.h"

#include <QProcess>
#include <QProcessEnvironment>
#include <QTest>

#include <cstdlib>

class UpdateCheckerTests final : public QObject
{
    Q_OBJECT

private slots:
    void aptSimulationWithoutOperationsHasNoUpdates();
    void phasedUpdatesKeptBackHaveNoUpdates();
    void aptSimulationWithInstallOperationHasUpdates();
    void successfulAptQueryWithoutInstallOperationHasNoUpdates();
    void aptAndFlatpakWithoutUpdatesDoNotShowWindow();
    void phasedUpdatesRegressionDoesNotShowWindow();
    void flatpakEmptyOutputIsNotAnUpdate();
    void flatpakReferenceIsAnUpdate();
    void scenarioCFlatpakReferenceShowsWindow();
    void discoverUsesOriginalSessionEnvironment();
    void layerShellMutationIsNotPropagatedToDiscover();
    void liveCommandLineIsDetected();
    void installedCommandLineIsNotLive();
    void partialCasperTokenIsNotLive();
    void liveGuardDoesNotStartInstalledSession();
    void x11PositionUsesBottomRightMargin();
    void waylandSettingsUseBottomRightAnchors();
};

void UpdateCheckerTests::discoverUsesOriginalSessionEnvironment()
{
    QProcessEnvironment sessionEnvironment;
    sessionEnvironment.insert(QStringLiteral("PATH"), QStringLiteral("/usr/bin"));
    sessionEnvironment.insert(QStringLiteral("ORIGINAL_SESSION_MARKER"),
                              QStringLiteral("preserved"));

    QProcess discover;
    configureDiscoverProcess(discover, sessionEnvironment);

    QCOMPARE(discover.program(), QStringLiteral("/usr/bin/plasma-discover"));
    QCOMPARE(discover.arguments(),
             QStringList({QStringLiteral("--mode"), QStringLiteral("Update")}));
    QCOMPARE(discover.processEnvironment(), sessionEnvironment);
}

void UpdateCheckerTests::layerShellMutationIsNotPropagatedToDiscover()
{
    QProcessEnvironment sessionEnvironment;
    sessionEnvironment.insert(QStringLiteral("PATH"), QStringLiteral("/usr/bin"));

    QProcessEnvironment applicationEnvironment = sessionEnvironment;
    applicationEnvironment.insert(QStringLiteral("QT_WAYLAND_SHELL_INTEGRATION"),
                                  QStringLiteral("layer-shell"));
    QCOMPARE(applicationEnvironment.value(QStringLiteral("QT_WAYLAND_SHELL_INTEGRATION")),
             QStringLiteral("layer-shell"));

    QProcess discover;
    configureDiscoverProcess(discover, sessionEnvironment);

    QVERIFY(!discover.processEnvironment().contains(
        QStringLiteral("QT_WAYLAND_SHELL_INTEGRATION")));
    QCOMPARE(discover.processEnvironment(), sessionEnvironment);
}

void UpdateCheckerTests::aptSimulationWithoutOperationsHasNoUpdates()
{
    const QString output = QStringLiteral(
        "Reading package lists...\n"
        "Building dependency tree...\n"
        "Reading state information...\n"
        "Calculating upgrade...\n"
        "0 upgraded, 0 newly installed, 0 to remove and 3 not upgraded.\n");

    QVERIFY(!aptSimulationHasUpdates(output));
}

void UpdateCheckerTests::phasedUpdatesKeptBackHaveNoUpdates()
{
    const QString output = QStringLiteral(
        "Reading package lists...\n"
        "Building dependency tree...\n"
        "Reading state information...\n"
        "Calculating upgrade...\n"
        "The following packages have been kept back:\n"
        "  glycin-loaders glycin-thumbnailers libglycin-2-0\n"
        "0 upgraded, 0 newly installed, 0 to remove and 3 not upgraded.\n");

    QVERIFY(!aptSimulationHasUpdates(output));
}

void UpdateCheckerTests::aptSimulationWithInstallOperationHasUpdates()
{
    const QString output = QStringLiteral(
        "Reading package lists...\n"
        "Calculating upgrade...\n"
        "Inst example [1.0] (2.0 Ubuntu:26.04/resolute-updates [amd64])\n"
        "Conf example (2.0 Ubuntu:26.04/resolute-updates [amd64])\n");

    QVERIFY(aptSimulationHasUpdates(output));
}

void UpdateCheckerTests::successfulAptQueryWithoutInstallOperationHasNoUpdates()
{
    UpdateState state;
    state.aptUpdateSucceeded = true;
    state.aptQuerySucceeded = true;
    state.aptHasUpdates = aptSimulationHasUpdates(QStringLiteral(
        "Reading package lists...\n"
        "0 upgraded, 0 newly installed, 0 to remove and 0 not upgraded.\n"));

    QVERIFY(!state.aptHasUpdates);
}

void UpdateCheckerTests::aptAndFlatpakWithoutUpdatesDoNotShowWindow()
{
    UpdateState state;
    state.aptUpdateSucceeded = true;
    state.aptQuerySucceeded = true;
    state.aptHasUpdates = aptSimulationHasUpdates(QStringLiteral(
        "0 upgraded, 0 newly installed, 0 to remove and 3 not upgraded.\n"));
    state.flatpakSystemQuerySucceeded = true;
    state.flatpakSystemHasUpdates = flatpakOutputHasUpdates({});
    state.flatpakUserQuerySucceeded = true;
    state.flatpakUserHasUpdates = flatpakOutputHasUpdates({});

    QVERIFY(!state.hasUpdates());
}

void UpdateCheckerTests::phasedUpdatesRegressionDoesNotShowWindow()
{
    const QString aptListOutput = QStringLiteral(
        "glycin-loaders/resolute-updates 2.1.5+ds-0ubuntu0.2 amd64 "
        "[upgradable from: 2.1.1+ds-0ubuntu1]\n"
        "glycin-thumbnailers/resolute-updates 2.1.5+ds-0ubuntu0.2 amd64 "
        "[upgradable from: 2.1.1+ds-0ubuntu1]\n"
        "libglycin-2-0/resolute-updates 2.1.5+ds-0ubuntu0.2 amd64 "
        "[upgradable from: 2.1.1+ds-0ubuntu1]\n");
    const QString simulationOutput = QStringLiteral(
        "Reading package lists...\n"
        "Building dependency tree...\n"
        "Reading state information...\n"
        "Calculating upgrade...\n"
        "The following upgrades have been deferred due to phasing:\n"
        "  glycin-loaders glycin-thumbnailers libglycin-2-0\n"
        "0 upgraded, 0 newly installed, 0 to remove and 3 not upgraded.\n");

    QVERIFY(aptListOutput.contains(QStringLiteral("glycin-loaders")));
    UpdateState state;
    state.aptUpdateSucceeded = true;
    state.aptQuerySucceeded = true;
    state.aptHasUpdates = aptSimulationHasUpdates(simulationOutput);
    state.flatpakSystemQuerySucceeded = true;
    state.flatpakUserQuerySucceeded = true;

    QVERIFY(!state.hasUpdates());
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

void UpdateCheckerTests::scenarioCFlatpakReferenceShowsWindow()
{
    UpdateState state;
    state.flatpakSystemQuerySucceeded = true;
    state.flatpakSystemHasUpdates = flatpakOutputHasUpdates(
        QStringLiteral("org.example.Application/x86_64/stable\n"));
    QVERIFY(state.hasUpdates());
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
