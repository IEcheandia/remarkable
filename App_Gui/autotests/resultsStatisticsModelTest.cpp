#include <QTest>
#include <QSignalSpy>

#include "../src/resultsStatisticsModel.h"

using precitec::gui::ResultsStatisticsModel;

class ResultsStatisticsModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testRoleNames();
    void testData_data();
    void testData();
};

void ResultsStatisticsModelTest::testCtor()
{
    ResultsStatisticsModel model;
    QCOMPARE(model.rowCount(), 6);
    const auto index = model.index(0, 0);
    QVERIFY(index.isValid());
    QCOMPARE(model.rowCount(index), 0);
}

void ResultsStatisticsModelTest::testRoleNames()
{
    ResultsStatisticsModel model;
    const auto roleNames = model.roleNames();
    QCOMPARE(roleNames.size(), 4);
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("display"));
    QCOMPARE(roleNames[Qt::DecorationRole], QByteArrayLiteral("icon"));
    QCOMPARE(roleNames[Qt::UserRole], QByteArrayLiteral("enabled"));
    QCOMPARE(roleNames[Qt::UserRole + 1], QByteArrayLiteral("component"));
}

void ResultsStatisticsModelTest::testData_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QString>("display");
    QTest::addColumn<QString>("icon");
    QTest::addColumn<bool>("enabled_product");
    QTest::addColumn<bool>("enabled_series");
    QTest::addColumn<bool>("enabled_seams");
    QTest::addColumn<bool>("enabled_linked_seams");
    QTest::addColumn<ResultsStatisticsModel::ResultsComponent>("component");

    int row = 0;
    QTest::newRow("Product") << row++ << QStringLiteral("Product") << QStringLiteral("select-product") << true << true << true << true << ResultsStatisticsModel::ResultsComponent::Product;
    QTest::newRow("Instance") << row++ << QStringLiteral("Instance Statistics") << QStringLiteral("select-assembly") << false << true << true << true << ResultsStatisticsModel::ResultsComponent::Instance;
    QTest::newRow("Series") << row++ << QStringLiteral("Series Statistics") << QStringLiteral("select-seam-series") << false << true << true << true <<ResultsStatisticsModel::ResultsComponent::Series;
    QTest::newRow("Seams") << row++ << QStringLiteral("Seams Statistics") << QStringLiteral("select-seam") << false << false << true << true <<ResultsStatisticsModel::ResultsComponent::Seams;
    QTest::newRow("Seam") << row++ << QStringLiteral("Seam Statistics") << QStringLiteral("view-plot") << false << false << true << true << ResultsStatisticsModel::ResultsComponent::Seam;
    QTest::newRow("Linked Seam") << row++ << QStringLiteral("Linked Seam Statistics") << QStringLiteral("view-plot") << false << false << false << true << ResultsStatisticsModel::ResultsComponent::LinkedSeam;
}

void ResultsStatisticsModelTest::testData()
{
    ResultsStatisticsModel model;

    QSignalSpy dataChangedSpy{&model, &ResultsStatisticsModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    QFETCH(int, row);
    const auto index = model.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data(Qt::DisplayRole).toString(), "display");
    QTEST(index.data(Qt::DecorationRole).toString(), "icon");
    QTEST(index.data(Qt::UserRole).toBool(), "enabled_product");
    QTEST(index.data(Qt::UserRole + 1).value<ResultsStatisticsModel::ResultsComponent>(), "component");

    model.setActiveLevel(ResultsStatisticsModel::ResultsComponent::Product);
    QCOMPARE(dataChangedSpy.count(), 0);

    model.setActiveLevel(ResultsStatisticsModel::ResultsComponent::Series);
    QCOMPARE(dataChangedSpy.count(), 1);
    QTEST(index.data(Qt::UserRole).toBool(), "enabled_series");

    model.setActiveLevel(ResultsStatisticsModel::ResultsComponent::Seam);
    QCOMPARE(dataChangedSpy.count(), 2);
    QTEST(index.data(Qt::UserRole).toBool(), "enabled_seams");

    model.setActiveLevel(ResultsStatisticsModel::ResultsComponent::LinkedSeam);
    QCOMPARE(dataChangedSpy.count(), 3);
    QTEST(index.data(Qt::UserRole).toBool(), "enabled_linked_seams");

    auto args = dataChangedSpy.takeFirst();
    QCOMPARE(dataChangedSpy.count(), 2);
    QCOMPARE(args.size(), 3);

    auto topLeft = args.at(0).toModelIndex();
    QVERIFY(topLeft.isValid());
    QCOMPARE(topLeft.row(), 0);
    QCOMPARE(topLeft.column(), 0);
    auto bottomRight = args.at(1).toModelIndex();
    QVERIFY(bottomRight.isValid());
    QCOMPARE(bottomRight.row(), 5);
    QCOMPARE(bottomRight.column(), 0);
    auto roles = args.at(2).value<QVector<int>>();
    QCOMPARE(roles.size(), 1);
    QVERIFY(roles.contains(Qt::UserRole));
}

QTEST_GUILESS_MAIN(ResultsStatisticsModelTest)
#include "resultsStatisticsModelTest.moc"
