#include <QTest>
#include <QSignalSpy>

#include "../digitalSlaveModel.h"

using precitec::gui::components::ethercat::DigitalSlaveModel;

class TestEthercatDigitalSlaveModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testBitData_data();
    void testBitData();
};

void TestEthercatDigitalSlaveModel::testCtor()
{
    DigitalSlaveModel model;
    QCOMPARE(model.rowCount(), 8);
    QCOMPARE(model.roleNames().count(), 1);
    QCOMPARE(model.roleNames()[Qt::DisplayRole], QByteArrayLiteral("bit"));
    QCOMPARE(model.byteData(), QByteArray());

    QCOMPARE(model.index(0, 0).data().toBool(), false);
    QCOMPARE(model.index(1, 0).data().toBool(), false);
    QCOMPARE(model.index(2, 0).data().toBool(), false);
    QCOMPARE(model.index(3, 0).data().toBool(), false);
    QCOMPARE(model.index(4, 0).data().toBool(), false);
    QCOMPARE(model.index(5, 0).data().toBool(), false);
    QCOMPARE(model.index(6, 0).data().toBool(), false);
    QCOMPARE(model.index(7, 0).data().toBool(), false);
}

void TestEthercatDigitalSlaveModel::testBitData_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<bool>("bit0");
    QTest::addColumn<bool>("bit1");
    QTest::addColumn<bool>("bit2");
    QTest::addColumn<bool>("bit3");
    QTest::addColumn<bool>("bit4");
    QTest::addColumn<bool>("bit5");
    QTest::addColumn<bool>("bit6");
    QTest::addColumn<bool>("bit7");

    QTest::newRow("0x1") << QByteArray(1, 0x1) << true << false << false << false << false << false << false << false;
    QTest::newRow("0x2") << QByteArray(1, 0x2) << false << true << false << false << false << false << false << false;
    QTest::newRow("0x4") << QByteArray(1, 0x4) << false << false << true  << false << false << false << false << false;
    QTest::newRow("0x8") << QByteArray(1, 0x8) << false << false << false << true << false << false << false << false;
    QTest::newRow("0x10") << QByteArray(1, 0x10) << false << false << false << false << true << false << false << false;
    QTest::newRow("0x20") << QByteArray(1, 0x20) << false << false << false << false << false << true << false << false;
    QTest::newRow("0x40") << QByteArray(1, 0x40) << false << false << false << false << false << false << true << false;
    QTest::newRow("0x80") << QByteArray(1, 0x80) << false << false << false << false << false << false << false << true;
    QTest::newRow("0xFF") << QByteArray(1, 0xFF) << true << true << true << true << true << true << true << true;
}

void TestEthercatDigitalSlaveModel::testBitData()
{
    DigitalSlaveModel model;
    QSignalSpy byteDataChangedSpy{&model, &DigitalSlaveModel::byteDataChanged};
    QVERIFY(byteDataChangedSpy.isValid());
    QSignalSpy dataChangedSpy{&model, &DigitalSlaveModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    QFETCH(QByteArray, data);
    model.setByteData(data);
    QCOMPARE(model.byteData(), data);
    QCOMPARE(byteDataChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.first().at(0).toModelIndex(), model.index(0, 0));
    QCOMPARE(dataChangedSpy.first().at(1).toModelIndex(), model.index(7, 0));
    QCOMPARE(dataChangedSpy.first().at(2).value<QVector<int>>(), QVector<int>() << Qt::DisplayRole);
    // setting same again should not change
    QCOMPARE(model.byteData(), data);
    QCOMPARE(byteDataChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.count(), 1);

    QTEST(model.index(0, 0).data().toBool(), "bit0");
    QTEST(model.index(1, 0).data().toBool(), "bit1");
    QTEST(model.index(2, 0).data().toBool(), "bit2");
    QTEST(model.index(3, 0).data().toBool(), "bit3");
    QTEST(model.index(4, 0).data().toBool(), "bit4");
    QTEST(model.index(5, 0).data().toBool(), "bit5");
    QTEST(model.index(6, 0).data().toBool(), "bit6");
    QTEST(model.index(7, 0).data().toBool(), "bit7");
}

QTEST_GUILESS_MAIN(TestEthercatDigitalSlaveModel)
#include "ethercatDigitalSlaveModelTest.moc"
