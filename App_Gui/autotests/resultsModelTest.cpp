#include <QTest>
#include <QSignalSpy>

#include "../src/resultsModel.h"

using precitec::gui::ResultsModel;

class ResultsModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testRoleNames();
    void testData_data();
    void testData();
};

void ResultsModelTest::testCtor()
{
    ResultsModel model;
    QCOMPARE(model.rowCount(), 5);
    const auto index = model.index(0, 0);
    QVERIFY(index.isValid());
    QCOMPARE(model.rowCount(index), 0);
}

void ResultsModelTest::testRoleNames()
{
    ResultsModel model;
    const auto roleNames = model.roleNames();
    QCOMPARE(roleNames.size(), 4);
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("display"));
    QCOMPARE(roleNames[Qt::DecorationRole], QByteArrayLiteral("icon"));
    QCOMPARE(roleNames[Qt::UserRole], QByteArrayLiteral("enabled"));
    QCOMPARE(roleNames[Qt::UserRole + 1], QByteArrayLiteral("component"));
}

void ResultsModelTest::testData_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QString>("display");
    QTest::addColumn<QString>("icon");
    QTest::addColumn<bool>("enabled_product");
    QTest::addColumn<bool>("enabled_series");
    QTest::addColumn<bool>("enabled_results");
    QTest::addColumn<ResultsModel::ResultsComponent>("component");

    int row = 0;
    QTest::newRow("Product") << row++ << QStringLiteral("Product") << QStringLiteral("select-product") << true << true << true << ResultsModel::ResultsComponent::Product;
    QTest::newRow("Instance") << row++ << QStringLiteral("Instance") << QStringLiteral("select-assembly") << false << true << true << ResultsModel::ResultsComponent::Instance;
    QTest::newRow("Series") << row++ << QStringLiteral("Series") << QStringLiteral("select-seam-series") << false << true << true << ResultsModel::ResultsComponent::Series;
    QTest::newRow("Seam") << row++ << QStringLiteral("Seam") << QStringLiteral("select-seam") << false << false << true << ResultsModel::ResultsComponent::Seam;
    QTest::newRow("Results") << row++ << QStringLiteral("Results") << QStringLiteral("view-plot") << false << false << true << ResultsModel::ResultsComponent::Results;
}

void ResultsModelTest::testData()
{
    ResultsModel model;

    QSignalSpy dataChangedSpy{&model, &ResultsModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    QFETCH(int, row);
    const auto index = model.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data(Qt::DisplayRole).toString(), "display");
    QTEST(index.data(Qt::DecorationRole).toString(), "icon");
    QTEST(index.data(Qt::UserRole).toBool(), "enabled_product");
    QTEST(index.data(Qt::UserRole + 1).value<ResultsModel::ResultsComponent>(), "component");

    model.setActiveLevel(ResultsModel::ResultsComponent::Product);
    QCOMPARE(dataChangedSpy.count(), 0);

    model.setActiveLevel(ResultsModel::ResultsComponent::Series);
    QCOMPARE(dataChangedSpy.count(), 1);
    QTEST(index.data(Qt::UserRole).toBool(), "enabled_series");

    model.setActiveLevel(ResultsModel::ResultsComponent::Results);
    QCOMPARE(dataChangedSpy.count(), 2);
    QTEST(index.data(Qt::UserRole).toBool(), "enabled_results");

    auto args = dataChangedSpy.takeFirst();
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(args.size(), 3);

    auto topLeft = args.at(0).toModelIndex();
    QVERIFY(topLeft.isValid());
    QCOMPARE(topLeft.row(), 0);
    QCOMPARE(topLeft.column(), 0);
    auto bottomRight = args.at(1).toModelIndex();
    QVERIFY(bottomRight.isValid());
    QCOMPARE(bottomRight.row(), 4);
    QCOMPARE(bottomRight.column(), 0);
    auto roles = args.at(2).value<QVector<int>>();
    QCOMPARE(roles.size(), 1);
    QVERIFY(roles.contains(Qt::UserRole));
}

QTEST_GUILESS_MAIN(ResultsModelTest)
#include "resultsModelTest.moc"
