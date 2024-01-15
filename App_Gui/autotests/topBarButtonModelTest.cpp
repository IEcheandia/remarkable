#include <QTest>
#include <QSignalSpy>

#include "../src/topBarButtonModel.h"
#include "../src/permissions.h"

using precitec::gui::TopBarButtonModel;
using precitec::gui::Permission;

class TopBarButtonModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testRoleNames();
    void testData_data();
    void testData();
    void testSimulationEnabled();
};

void TopBarButtonModelTest::testCtor()
{
    auto model = new TopBarButtonModel{this};
    QCOMPARE(model->rowCount(), 9);
    const auto index = model->index(0, 0);
    QVERIFY(index.isValid());
    QCOMPARE(model->rowCount(index), 0);
}

void TopBarButtonModelTest::testRoleNames()
{
    auto model = new TopBarButtonModel{this};
    const auto roleNames = model->roleNames();
    QCOMPARE(roleNames.size(), 6);
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("text"));
    QCOMPARE(roleNames[Qt::UserRole], QByteArrayLiteral("iconSource"));
    QCOMPARE(roleNames[Qt::UserRole + 1], QByteArrayLiteral("permission"));
    QCOMPARE(roleNames[Qt::UserRole + 2], QByteArrayLiteral("enabled"));
    QCOMPARE(roleNames[Qt::UserRole + 3], QByteArrayLiteral("objectName"));
    QCOMPARE(roleNames[Qt::UserRole + 4], QByteArrayLiteral("type"));
}

void TopBarButtonModelTest::testData_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QString>("text");
    QTest::addColumn<QString>("iconSource");
    QTest::addColumn<int>("permission");
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<QString>("objectName");
    QTest::addColumn<TopBarButtonModel::TopBarButton>("type");

    int row = 0;
    QTest::newRow("Overview") << row++ << QStringLiteral("Home") << QStringLiteral("qrc:/icons/home") << -1 << true << QStringLiteral("topBar-overview") << TopBarButtonModel::TopBarButton::Overview;
    QTest::newRow("Login") << row++ << QStringLiteral("Login") << QStringLiteral("qrc:/icons/user") << -1 << true << QStringLiteral("topBar-login") << TopBarButtonModel::TopBarButton::Login;
    QTest::newRow("Results") << row++ << QStringLiteral("Results") << QStringLiteral("qrc:/icons/fileopen") << -1 << true << QStringLiteral("topBar-results") << TopBarButtonModel::TopBarButton::Results;
    QTest::newRow("Statistics") << row++ << QStringLiteral("Statistics") << QStringLiteral("qrc:/icons/statistics") << -1 << true << QStringLiteral("topBar-statistics") << TopBarButtonModel::TopBarButton::Statistics;
    QTest::newRow("Simulation") << row++ << QStringLiteral("Simulation") << QStringLiteral("qrc:/icons/video") << -1 << false << QStringLiteral("topBar-simulation") << TopBarButtonModel::TopBarButton::Simulation;
    QTest::newRow("Simulation") << row++ << QStringLiteral("Head Monitor") << QStringLiteral("qrc:/icons/laserhead") << -1 << true << QStringLiteral("topBar-headMonitor") << TopBarButtonModel::TopBarButton::HeadMonitor;
    QTest::newRow("Wizard") << row++ << QStringLiteral("Configuration") << QStringLiteral("qrc:/icons/wizard") << static_cast<int> (Permission::RunHardwareAndProductWizard) << true << QStringLiteral("topBar-configuration") << TopBarButtonModel::TopBarButton::Wizard;
    QTest::newRow("Grapheditor") << row++ << QStringLiteral("Grapheditor") << QStringLiteral("qrc:/icons/grapheditor") << static_cast<int> (Permission::EditGraphsWithGrapheditor) << true << QStringLiteral("topBar-graphEditor") << TopBarButtonModel::TopBarButton::Grapheditor;
    QTest::newRow("Configuration") << row++ << QStringLiteral("Settings") << QStringLiteral("qrc:/icons/tool") << -1 << true << QStringLiteral("topBar-settings") << TopBarButtonModel::TopBarButton::Configuration;
}

void TopBarButtonModelTest::testData()
{
    auto model = new TopBarButtonModel{this};
    QFETCH(int, row);
    const auto &index = model->index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data(Qt::DisplayRole).toString(), "text");
    QTEST(index.data(Qt::UserRole).toString(), "iconSource");
    QTEST(index.data(Qt::UserRole + 1).toInt(), "permission");
    QTEST(index.data(Qt::UserRole + 2).toBool(), "enabled");
    QTEST(index.data(Qt::UserRole + 3).toString(), "objectName");
    QTEST(index.data(Qt::UserRole + 4).value<TopBarButtonModel::TopBarButton>(), "type");
}

void TopBarButtonModelTest::testSimulationEnabled()
{
    auto model = new TopBarButtonModel{this};
    QSignalSpy dataChangedSpy{model, &TopBarButtonModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    const auto &index = model->index(static_cast<int> (TopBarButtonModel::TopBarButton::Simulation), 0);
    QVERIFY(index.isValid());
    QCOMPARE(index.data(Qt::UserRole + 2).toBool(), false);

    model->setSimulationEnabled(false);
    const auto &index2 = model->index(static_cast<int> (TopBarButtonModel::TopBarButton::Simulation), 0);
    QVERIFY(index2.isValid());
    QCOMPARE(index2.data(Qt::UserRole + 2).toBool(), false);
    QCOMPARE(dataChangedSpy.count(), 0);

    model->setSimulationEnabled(true);
    const auto &index3 = model->index(static_cast<int> (TopBarButtonModel::TopBarButton::Simulation), 0);
    QVERIFY(index3.isValid());
    QCOMPARE(index3.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(dataChangedSpy.count(), 1);

    const auto &spyArguments = dataChangedSpy.at(0);
    QCOMPARE(spyArguments.at(0).toModelIndex(), model->index(4));
    QCOMPARE(spyArguments.at(1).toModelIndex(), model->index(4));
    QCOMPARE(spyArguments.at(2).value<QVector<int>>(), {Qt::UserRole + 2});
}

QTEST_GUILESS_MAIN(TopBarButtonModelTest)
#include "topBarButtonModelTest.moc"
