#include <QTest>
#include <QSignalSpy>

#include "../src/wobbleFigureModel.h"

using precitec::scanmaster::components::wobbleFigureEditor::WobbleFigureModel;

class WobbleFigureModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testRoleNames();
    void testRowCount();
    void testData();
    void testPointCount();
    void testCheckIfMicroVectorFactorIsEven();
    void testCalculateFrequency_data();
    void testCalculateFrequency();
    void testCheckIfFrequencyIsNotAlreadyPresent();
    void testIndexByMicroVectorFactor();
    void testFrequencyCalculation_data();
    void testFrequencyCalculation();
    void testDualChannelModulation();
    void testMicroVectorFactor();
    void testLowestFrequency();
    void testHighestFrequency();
};

void WobbleFigureModelTest::testCtor()
{
    WobbleFigureModel model;
    QCOMPARE(model.pointCount(), 0);
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.isDualChannelModulation(), false);
    QCOMPARE(model.microVectorFactor(), 0);
    QCOMPARE(model.frequencyIndex(), QModelIndex{});
    QCOMPARE(model.lowestFrequency(), 1);
    QCOMPARE(model.highestFrequency(), 1000);
    QCOMPARE(model.m_counter, 25000);
    QCOMPARE(model.m_possibleFrequencies.size(), 0);
}

void WobbleFigureModelTest::testRoleNames()
{
    WobbleFigureModel model;
    const auto roleNames = model.roleNames();
    QCOMPARE(roleNames.size(), 2);
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("frequency"));
    QCOMPARE(roleNames[Qt::UserRole], QByteArrayLiteral("microVectorFactor"));
}

void WobbleFigureModelTest::testRowCount()
{
    WobbleFigureModel model;
    model.m_possibleFrequencies = {{1, 10}, {2, 9}, {3, 8}, {4, 7}, {5, 6}, {6, 5}, {7, 4}, {8, 3}, {9, 2}, {10, 1}};
    QCOMPARE(model.rowCount(), model.m_possibleFrequencies.size());
}

void WobbleFigureModelTest::testData()
{
    WobbleFigureModel model;
    model.m_possibleFrequencies = {{1, 10}, {2, 9}, {3, 8}};

    for (int i = 0; i < model.rowCount(); i++)
    {
        QCOMPARE(model.data(model.index(i, 0), Qt::DisplayRole), model.m_possibleFrequencies.at(i).frequency);
        QCOMPARE(model.data(model.index(i, 0), Qt::UserRole), model.m_possibleFrequencies.at(i).microVectorFactor);
    }
}

void WobbleFigureModelTest::testPointCount()
{
    WobbleFigureModel model;

    QSignalSpy pointCountChanged{&model, &WobbleFigureModel::pointCountChanged};
    QVERIFY(pointCountChanged.isValid());
    QCOMPARE(pointCountChanged.count(), 0);

    model.setPointCount(100);
    QCOMPARE(pointCountChanged.count(), 1);
    QCOMPARE(model.pointCount(), 100);
}

void WobbleFigureModelTest::testCheckIfMicroVectorFactorIsEven()
{
    WobbleFigureModel model;
    model.m_microVectorFactor = 0;
    QVERIFY(model.checkIfMicroVectorFactorIsEven(model.m_microVectorFactor));
    model.m_microVectorFactor = 1;
    QVERIFY(!model.checkIfMicroVectorFactorIsEven(model.m_microVectorFactor));
    model.m_microVectorFactor = 2;
    QVERIFY(model.checkIfMicroVectorFactorIsEven(model.m_microVectorFactor));
    model.m_microVectorFactor = 3;
    QVERIFY(!model.checkIfMicroVectorFactorIsEven(model.m_microVectorFactor));
    model.m_microVectorFactor = 4;
    QVERIFY(model.checkIfMicroVectorFactorIsEven(model.m_microVectorFactor));
}

void WobbleFigureModelTest::testCalculateFrequency_data()
{
    QTest::addColumn<unsigned int>("pointCount");
    QTest::addColumn<unsigned int>("microVectorFactor");
    QTest::addColumn<int>("frequency");

    QTest::newRow("frequency1") << 9u << 2u << 6250;
    QTest::newRow("frequency2") << 9u << 10u << 1250;
    QTest::newRow("frequency3") << 9u << 20u << 625;
    QTest::newRow("frequency4") << 9u << 100u << 125;
    QTest::newRow("frequency5") << 9u << 200u << 63;
    QTest::newRow("frequency6") << 9u << 500u << 25;
    QTest::newRow("frequency7") << 17u << 10u << 625;
    QTest::newRow("frequency8") << 17u << 50u << 125;
    QTest::newRow("frequency9") << 17u << 100u << 63;
    QTest::newRow("frequency10") << 17u << 250u << 25;
}

void WobbleFigureModelTest::testCalculateFrequency()
{
    WobbleFigureModel model;
    QCOMPARE(model.calculateFrequency(0), 0);
    QCOMPARE(model.calculateFrequency(1), 0);
    model.setPointCount(9);
    QCOMPARE(model.calculateFrequency(0), 0);
    QCOMPARE(model.calculateFrequency(1), 12500);

    QFETCH(unsigned int, pointCount);
    model.setPointCount(pointCount);
    QFETCH(unsigned int, microVectorFactor);
    QTEST(model.calculateFrequency(microVectorFactor), "frequency");
}

void WobbleFigureModelTest::testCheckIfFrequencyIsNotAlreadyPresent()
{
    WobbleFigureModel model;
    model.m_possibleFrequencies = {{1, 10}, {2, 9}, {3, 8}, {4, 7}, {5, 6}, {6, 5}, {7, 4}, {8, 3}, {9, 2}, {10, 1}};

    QVERIFY(model.checkIfFrequencyIsNotAlreadyPresent(11));
    QVERIFY(!model.checkIfFrequencyIsNotAlreadyPresent(1));
    QVERIFY(!model.checkIfFrequencyIsNotAlreadyPresent(5));
    QVERIFY(!model.checkIfFrequencyIsNotAlreadyPresent(10));
}

void WobbleFigureModelTest::testIndexByMicroVectorFactor()
{
    WobbleFigureModel model;
    model.m_possibleFrequencies = {{1, 10}, {2, 9}, {3, 8}, {4, 7}, {5, 6}, {6, 5}, {7, 4}, {8, 3}, {9, 2}, {10, 1}};

    QSignalSpy frequencyIndexChanged{&model, &WobbleFigureModel::frequencyIndexChanged};
    QVERIFY(frequencyIndexChanged.isValid());
    QCOMPARE(frequencyIndexChanged.count(), 0);

    QCOMPARE(model.frequencyIndex(), QModelIndex{});
    model.indexByMicroVectorFactor();
    QCOMPARE(model.frequencyIndex(), QModelIndex{});
    QCOMPARE(frequencyIndexChanged.count(), 0);

    model.m_microVectorFactor = 5;
    model.indexByMicroVectorFactor();
    QCOMPARE(model.frequencyIndex(), model.index(5, 0));
    QCOMPARE(frequencyIndexChanged.count(), 1);

    model.m_microVectorFactor = 2;
    model.indexByMicroVectorFactor();
    QCOMPARE(model.frequencyIndex(), model.index(8, 0));
    QCOMPARE(frequencyIndexChanged.count(), 2);

    model.m_microVectorFactor = 0;
    model.indexByMicroVectorFactor();
    QCOMPARE(model.frequencyIndex(), QModelIndex{});
    QCOMPARE(frequencyIndexChanged.count(), 3);
}

void WobbleFigureModelTest::testFrequencyCalculation_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("frequency");
    QTest::addColumn<unsigned int>("microVectorFactor");

    int row = 0;
    QTest::newRow("MicroVectorFactor13") << row++ << 952 << 15u;
    QTest::newRow("MicroVectorFactor14") << row++ << 893 << 16u;
    QTest::newRow("MicroVectorFactor15") << row++ << 840 << 17u;
    QTest::newRow("MicroVectorFactor16") << row++ << 794 << 18u;
    QTest::newRow("MicroVectorFactor17") << row++ << 752 << 19u;
    QTest::newRow("MicroVectorFactor18") << row++ << 714 << 20u;
    QTest::newRow("MicroVectorFactor19") << row++ << 680 << 21u;
    QTest::newRow("MicroVectorFactor20") << row++ << 649 << 22u;
    QTest::newRow("MicroVectorFactor21") << row++ << 621 << 23u;
    QTest::newRow("MicroVectorFactor22") << row++ << 595 << 24u;
}

void WobbleFigureModelTest::testFrequencyCalculation()
{
    WobbleFigureModel model;
    model.m_counter = 30;
    model.setPointCount(8);
    model.calculatePossibleFrequencies();
    QFETCH(int, row);
    const auto index = model.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data(Qt::DisplayRole).toInt(), "frequency");
    QTEST(index.data(Qt::UserRole).toUInt(), "microVectorFactor");
}

void WobbleFigureModelTest::testDualChannelModulation()
{
    WobbleFigureModel model;
    model.setPointCount(8);
    model.m_microVectorFactor = 126;
    model.microVectorFactorChanged();
    QCOMPARE(model.m_microVectorFactor, 126);

    QSignalSpy isDualChannelModulationChanged{&model, &WobbleFigureModel::isDualChannelModulationChanged};
    QVERIFY(isDualChannelModulationChanged.isValid());
    QCOMPARE(isDualChannelModulationChanged.count(), 0);

    QSignalSpy reset{&model, &WobbleFigureModel::reset};
    QVERIFY(reset.isValid());
    QCOMPARE(reset.count(), 0);

    QSignalSpy microVectorFactorWillBeChanged{&model, &WobbleFigureModel::microVectorFactorWillBeChanged};
    QVERIFY(microVectorFactorWillBeChanged.isValid());
    QCOMPARE(microVectorFactorWillBeChanged.count(), 0);

    QSignalSpy modelReset{&model, &QAbstractListModel::modelReset};
    QVERIFY(modelReset.isValid());
    QCOMPARE(modelReset.count(), 0);

    QSignalSpy frequencyIndexChanged{&model, &WobbleFigureModel::frequencyIndexChanged};
    QVERIFY(frequencyIndexChanged.isValid());
    QCOMPARE(frequencyIndexChanged.count(), 0);

    model.setIsDualChannelModulation(true);
    QCOMPARE(isDualChannelModulationChanged.count(), 1);
    QCOMPARE(model.isDualChannelModulation(), true);
    QCOMPARE(reset.count(), 1);
    QCOMPARE(microVectorFactorWillBeChanged.count(), 0);
    QCOMPARE(model.m_microVectorFactor, 126);
    QCOMPARE(modelReset.count(), 1);
    QCOMPARE(frequencyIndexChanged.count(), 1);

    model.setIsDualChannelModulation(false);
    QCOMPARE(isDualChannelModulationChanged.count(), 2);
    QCOMPARE(model.isDualChannelModulation(), false);
    QCOMPARE(reset.count(), 2);
    QCOMPARE(microVectorFactorWillBeChanged.count(), 0);
    QCOMPARE(model.m_microVectorFactor, 126);
    QCOMPARE(modelReset.count(), 2);
    QCOMPARE(frequencyIndexChanged.count(), 2);

    model.m_microVectorFactor = 125;

    model.setIsDualChannelModulation(true);
    QCOMPARE(isDualChannelModulationChanged.count(), 3);
    QCOMPARE(model.isDualChannelModulation(), true);
    QCOMPARE(reset.count(), 3);
    QCOMPARE(microVectorFactorWillBeChanged.count(), 1);
    QCOMPARE(model.m_microVectorFactor, 126);
    QCOMPARE(modelReset.count(), 3);
    QCOMPARE(frequencyIndexChanged.count(), 3);
}

void WobbleFigureModelTest::testMicroVectorFactor()
{
    WobbleFigureModel model;
    model.setPointCount(8);

    QSignalSpy microVectorFactorChanged{&model, &WobbleFigureModel::microVectorFactorChanged};
    QVERIFY(microVectorFactorChanged.isValid());
    QCOMPARE(microVectorFactorChanged.count(), 0);

    QSignalSpy modelReset{&model, &QAbstractListModel::modelReset};
    QVERIFY(modelReset.isValid());
    QCOMPARE(modelReset.count(), 0);

    QSignalSpy frequencyIndexChanged{&model, &WobbleFigureModel::frequencyIndexChanged};
    QVERIFY(frequencyIndexChanged.isValid());
    QCOMPARE(frequencyIndexChanged.count(), 0);

    model.setMicroVectorFactor(125);
    QCOMPARE(model.microVectorFactor(), 125);
    QCOMPARE(microVectorFactorChanged.count(), 1);
    QCOMPARE(modelReset.count(), 1);
    QCOMPARE(frequencyIndexChanged.count(), 1);

    model.setMicroVectorFactor(200);
    QCOMPARE(model.microVectorFactor(), 200);
    QCOMPARE(microVectorFactorChanged.count(), 2);
    QCOMPARE(modelReset.count(), 2);
    QCOMPARE(frequencyIndexChanged.count(), 2);
}

void WobbleFigureModelTest::testLowestFrequency()
{
    WobbleFigureModel model;
    model.m_counter = 30;
    model.setPointCount(9);
    model.calculatePossibleFrequencies();
    model.m_microVectorFactor = 22;
    model.indexByMicroVectorFactor();
    QCOMPARE(model.frequencyIndex(), model.index(9, 0));

    QSignalSpy lowestFrequencyChanged{&model, &WobbleFigureModel::lowestFrequencyChanged};
    QVERIFY(lowestFrequencyChanged.isValid());
    QCOMPARE(lowestFrequencyChanged.count(), 0);

    QSignalSpy modelReset{&model, &QAbstractListModel::modelReset};
    QVERIFY(modelReset.isValid());
    QCOMPARE(modelReset.count(), 0);

    QSignalSpy microVectorFactorWillBeChanged{&model, &WobbleFigureModel::microVectorFactorWillBeChanged};
    QVERIFY(microVectorFactorWillBeChanged.isValid());
    QCOMPARE(microVectorFactorWillBeChanged.count(), 0);

    QSignalSpy frequencyIndexChanged{&model, &WobbleFigureModel::frequencyIndexChanged};
    QVERIFY(frequencyIndexChanged.isValid());
    QCOMPARE(frequencyIndexChanged.count(), 0);

    model.setLowestFrequency(590);
    QCOMPARE(lowestFrequencyChanged.count(), 1);
    QCOMPARE(modelReset.count(), 1);
    QCOMPARE(microVectorFactorWillBeChanged.count(), 1);
    QCOMPARE(frequencyIndexChanged.count(), 1);
    QCOMPARE(model.microVectorFactor(), 21);
    QCOMPARE(model.frequencyIndex(), model.index(8, 0));

    model.setLowestFrequency(500);
    QCOMPARE(lowestFrequencyChanged.count(), 2);
    QCOMPARE(modelReset.count(), 2);
    QCOMPARE(microVectorFactorWillBeChanged.count(), 1);
    QCOMPARE(frequencyIndexChanged.count(), 1);
    QCOMPARE(model.microVectorFactor(), 21);
    QCOMPARE(model.frequencyIndex(), model.index(8, 0));
}

void WobbleFigureModelTest::testHighestFrequency()
{
    WobbleFigureModel model;
    model.m_counter = 30;
    model.setPointCount(9);
    model.calculatePossibleFrequencies();
    model.m_microVectorFactor = 13;
    model.indexByMicroVectorFactor();
    QCOMPARE(model.frequencyIndex(), model.index(0, 0));

    QSignalSpy highestFrequencyChanged{&model, &WobbleFigureModel::highestFrequencyChanged};
    QVERIFY(highestFrequencyChanged.isValid());
    QCOMPARE(highestFrequencyChanged.count(), 0);

    QSignalSpy modelReset{&model, &QAbstractListModel::modelReset};
    QVERIFY(modelReset.isValid());
    QCOMPARE(modelReset.count(), 0);

    QSignalSpy microVectorFactorWillBeChanged{&model, &WobbleFigureModel::microVectorFactorWillBeChanged};
    QVERIFY(microVectorFactorWillBeChanged.isValid());
    QCOMPARE(microVectorFactorWillBeChanged.count(), 0);

    QSignalSpy frequencyIndexChanged{&model, &WobbleFigureModel::frequencyIndexChanged};
    QVERIFY(frequencyIndexChanged.isValid());
    QCOMPARE(frequencyIndexChanged.count(), 0);

    model.setHighestFrequency(900);
    QCOMPARE(highestFrequencyChanged.count(), 1);
    QCOMPARE(modelReset.count(), 1);
    QCOMPARE(microVectorFactorWillBeChanged.count(), 1);
    QCOMPARE(frequencyIndexChanged.count(), 0);
    QCOMPARE(model.microVectorFactor(), 14);
    QCOMPARE(model.frequencyIndex(), model.index(0, 0));

    model.setHighestFrequency(1000);
    QCOMPARE(highestFrequencyChanged.count(), 2);
    QCOMPARE(modelReset.count(), 2);
    QCOMPARE(microVectorFactorWillBeChanged.count(), 1);
    QCOMPARE(frequencyIndexChanged.count(), 1);
    QCOMPARE(model.microVectorFactor(), 14);
    QCOMPARE(model.frequencyIndex(), model.index(1, 0));
}

QTEST_GUILESS_MAIN(WobbleFigureModelTest)
#include "testWobbleFigureModel.moc"
