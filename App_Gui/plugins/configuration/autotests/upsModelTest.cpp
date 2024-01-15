#include <QTest>
#include <QSignalSpy>

#include "../upsModel.h"

using precitec::gui::UpsModel;

class UpsModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testAbb();
    void testApc();
    void testAbbPowerValue11RTG2();
    void testNone();
    void testOmronS8BAusb();
    void testChange();
    void testChangeByModeAndDriver();
};

void UpsModelTest::testCtor()
{
    // no config dir specified, default values should be used
    UpsModel model;
    QCOMPARE(model.isModified(), false);
    QCOMPARE(model.property("modified").toBool(), false);
    QCOMPARE(model.nutConfigDir(), QStringLiteral("/etc/nut/"));
    QCOMPARE(model.rowCount(), 5);
    QCOMPARE(model.index(0, 0).data().toString(), QStringLiteral("No UPS"));
    QCOMPARE(model.index(1, 0).data().toString(), QStringLiteral("ABB Powervalue 11RT"));
    QCOMPARE(model.index(2, 0).data().toString(), QStringLiteral("APC Smart-ups"));
    QCOMPARE(model.index(3, 0).data().toString(), QStringLiteral("ABB PowerValue 11RT G2"));
    QCOMPARE(model.index(4, 0).data().toString(), QStringLiteral("Omron S8BA-24D24D120LF"));
    QCOMPARE(model.index(0, 0).data(Qt::DecorationRole).toString(), QString{});
    QCOMPARE(model.index(1, 0).data(Qt::DecorationRole).toString(), QStringLiteral("ABB PowerValue 11 RT.jpg"));
    QCOMPARE(model.index(2, 0).data(Qt::DecorationRole).toString(), QStringLiteral("APC Smart-ups 750 VA LCD RM 2U 120 V.jpg"));
    QCOMPARE(model.index(3, 0).data(Qt::DecorationRole).toString(), QStringLiteral("ABB PowerValue 11 RT G2.png"));
    QCOMPARE(model.index(4, 0).data(Qt::DecorationRole).toString(), QStringLiteral("Omron_S8BA-24D24D120LF.png"));

    QCOMPARE(model.selectedIndex(), model.index(0, 0));
    QCOMPARE(model.property("selectedIndex").toModelIndex(), model.index(0, 0));
}

void UpsModelTest::testAbb()
{
    UpsModel model;
    QCOMPARE(model.rowCount(), 5);
    QSignalSpy nutConfigChangedSpy{&model, &UpsModel::nutConfigDirChanged};
    QVERIFY(nutConfigChangedSpy.isValid());
    model.setNutConfigDir(QFINDTESTDATA(QStringLiteral("testdata/upsModel/abb")));
    QCOMPARE(nutConfigChangedSpy.count(), 1);
    QCOMPARE(model.nutConfigDir(), QFINDTESTDATA(QStringLiteral("testdata/upsModel/abb")));
    QVERIFY(!model.nutConfigDir().isEmpty());

    QCOMPARE(model.selectedIndex(), model.index(1, 0));
    QCOMPARE(model.property("selectedIndex").toModelIndex(), model.index(1, 0));

    QCOMPARE(model.isModified(), false);
}

void UpsModelTest::testNone()
{
    UpsModel model;
    QCOMPARE(model.rowCount(), 5);
    QSignalSpy nutConfigChangedSpy{&model, &UpsModel::nutConfigDirChanged};
    QVERIFY(nutConfigChangedSpy.isValid());
    model.setNutConfigDir(QFINDTESTDATA(QStringLiteral("testdata/upsModel/none")));
    QCOMPARE(nutConfigChangedSpy.count(), 1);
    QCOMPARE(model.nutConfigDir(), QFINDTESTDATA(QStringLiteral("testdata/upsModel/none")));
    QVERIFY(!model.nutConfigDir().isEmpty());

    QCOMPARE(model.selectedIndex(), model.index(0, 0));
    QCOMPARE(model.property("selectedIndex").toModelIndex(), model.index(0, 0));
    QCOMPARE(model.isModified(), false);
}

void UpsModelTest::testApc()
{
    UpsModel model;
    QCOMPARE(model.rowCount(), 5);
    QSignalSpy nutConfigChangedSpy{&model, &UpsModel::nutConfigDirChanged};
    QVERIFY(nutConfigChangedSpy.isValid());
    model.setNutConfigDir(QFINDTESTDATA(QStringLiteral("testdata/upsModel/apc")));
    QCOMPARE(nutConfigChangedSpy.count(), 1);
    QCOMPARE(model.nutConfigDir(), QFINDTESTDATA(QStringLiteral("testdata/upsModel/apc")));
    QVERIFY(!model.nutConfigDir().isEmpty());

    QCOMPARE(model.selectedIndex(), model.index(2, 0));
    QCOMPARE(model.property("selectedIndex").toModelIndex(), model.index(2, 0));
    QCOMPARE(model.isModified(), false);
}

void UpsModelTest::testAbbPowerValue11RTG2()
{
    UpsModel model;
    QCOMPARE(model.rowCount(), 5);
    QSignalSpy nutConfigChangedSpy{&model, &UpsModel::nutConfigDirChanged};
    QVERIFY(nutConfigChangedSpy.isValid());
    model.setNutConfigDir(QFINDTESTDATA(QStringLiteral("testdata/upsModel/abbPowerValue11RTG2")));
    QCOMPARE(nutConfigChangedSpy.count(), 1);
    QCOMPARE(model.nutConfigDir(), QFINDTESTDATA(QStringLiteral("testdata/upsModel/abbPowerValue11RTG2")));
    QVERIFY(!model.nutConfigDir().isEmpty());

    QCOMPARE(model.selectedIndex(), model.index(3, 0));
    QCOMPARE(model.property("selectedIndex").toModelIndex(), model.index(3, 0));

    QCOMPARE(model.isModified(), false);
}

void UpsModelTest::testOmronS8BAusb()
{
    UpsModel model;
    QCOMPARE(model.rowCount(), 5);
    QSignalSpy nutConfigChangedSpy{&model, &UpsModel::nutConfigDirChanged};
    QVERIFY(nutConfigChangedSpy.isValid());
    model.setNutConfigDir(QFINDTESTDATA(QStringLiteral("testdata/upsModel/omronS8BAusb")));
    QCOMPARE(nutConfigChangedSpy.count(), 1);
    QCOMPARE(model.nutConfigDir(), QFINDTESTDATA(QStringLiteral("testdata/upsModel/omronS8BAusb")));
    QVERIFY(!model.nutConfigDir().isEmpty());

    QCOMPARE(model.selectedIndex(), model.index(4, 0));
    QCOMPARE(model.property("selectedIndex").toModelIndex(), model.index(4, 0));

    QCOMPARE(model.isModified(), false);
}

void UpsModelTest::testChange()
{
    UpsModel model;
    QCOMPARE(model.rowCount(), 5);
    QSignalSpy nutConfigChangedSpy{&model, &UpsModel::nutConfigDirChanged};
    QVERIFY(nutConfigChangedSpy.isValid());
    model.setNutConfigDir(QFINDTESTDATA(QStringLiteral("testdata/upsModel/none")));
    QCOMPARE(nutConfigChangedSpy.count(), 1);
    QCOMPARE(model.nutConfigDir(), QFINDTESTDATA(QStringLiteral("testdata/upsModel/none")));
    QVERIFY(!model.nutConfigDir().isEmpty());

    QCOMPARE(model.selectedIndex(), model.index(0, 0));
    QCOMPARE(model.property("selectedIndex").toModelIndex(), model.index(0, 0));
    QCOMPARE(model.isModified(), false);

    QSignalSpy selectedIndexChangedSpy{&model, &UpsModel::selectedIndexChanged};
    QVERIFY(selectedIndexChangedSpy.isValid());
    QSignalSpy modifiedChangedSpy{&model, &UpsModel::modifiedChanged};
    QVERIFY(modifiedChangedSpy.isValid());

    model.select(0);
    QVERIFY(selectedIndexChangedSpy.isEmpty());
    QVERIFY(modifiedChangedSpy.isEmpty());
    QCOMPARE(model.selectedIndex(), model.index(0, 0));
    QCOMPARE(model.isModified(), false);

    model.select(1);
    QCOMPARE(selectedIndexChangedSpy.count(), 1);
    QCOMPARE(modifiedChangedSpy.count(), 1);
    QCOMPARE(model.selectedIndex(), model.index(1, 0));
    QCOMPARE(model.isModified(), true);
    QCOMPARE(model.property("modified").toBool(), true);

    // select same
    model.select(1);
    QCOMPARE(selectedIndexChangedSpy.count(), 1);
    QCOMPARE(modifiedChangedSpy.count(), 1);

    // next
    model.select(2);
    QCOMPARE(selectedIndexChangedSpy.count(), 2);
    QCOMPARE(modifiedChangedSpy.count(), 1);
    QCOMPARE(model.selectedIndex(), model.index(2, 0));

    // invalid
    model.select(5);
    QCOMPARE(selectedIndexChangedSpy.count(), 2);
    QCOMPARE(modifiedChangedSpy.count(), 1);
    QCOMPARE(model.selectedIndex(), model.index(2, 0));
}

void UpsModelTest::testChangeByModeAndDriver()
{
    UpsModel model;
    QCOMPARE(model.rowCount(), 5);
    QSignalSpy nutConfigChangedSpy{&model, &UpsModel::nutConfigDirChanged};
    QVERIFY(nutConfigChangedSpy.isValid());
    model.setNutConfigDir(QFINDTESTDATA(QStringLiteral("testdata/upsModel/none")));
    QCOMPARE(nutConfigChangedSpy.count(), 1);
    QCOMPARE(model.nutConfigDir(), QFINDTESTDATA(QStringLiteral("testdata/upsModel/none")));
    QVERIFY(!model.nutConfigDir().isEmpty());

    QCOMPARE(model.selectedIndex(), model.index(0, 0));
    QCOMPARE(model.property("selectedIndex").toModelIndex(), model.index(0, 0));
    QCOMPARE(model.isModified(), false);

    QSignalSpy selectedIndexChangedSpy{&model, &UpsModel::selectedIndexChanged};
    QVERIFY(selectedIndexChangedSpy.isValid());
    QSignalSpy modifiedChangedSpy{&model, &UpsModel::modifiedChanged};
    QVERIFY(modifiedChangedSpy.isValid());

    model.selectByModeAndDriver(0, 0);
    QVERIFY(selectedIndexChangedSpy.isEmpty());
    QVERIFY(modifiedChangedSpy.isEmpty());
    QCOMPARE(model.selectedIndex(), model.index(0, 0));
    QCOMPARE(model.isModified(), false);

    model.selectByModeAndDriver(1, 1);
    QCOMPARE(selectedIndexChangedSpy.count(), 1);
    QCOMPARE(modifiedChangedSpy.count(), 1);
    QCOMPARE(model.selectedIndex(), model.index(1, 0));
    QCOMPARE(model.isModified(), true);
    QCOMPARE(model.property("modified").toBool(), true);

    // select same
    model.selectByModeAndDriver(1, 1);
    QCOMPARE(selectedIndexChangedSpy.count(), 1);
    QCOMPARE(modifiedChangedSpy.count(), 1);

    // next
    model.selectByModeAndDriver(1, 2);
    QCOMPARE(selectedIndexChangedSpy.count(), 2);
    QCOMPARE(modifiedChangedSpy.count(), 1);
    QCOMPARE(model.selectedIndex(), model.index(2, 0));

    // next
    model.selectByModeAndDriver(1, 3);
    QCOMPARE(selectedIndexChangedSpy.count(), 3);
    QCOMPARE(modifiedChangedSpy.count(), 1);
    QCOMPARE(model.selectedIndex(), model.index(3, 0));

    // invalid
    model.selectByModeAndDriver(1, 4);
    QCOMPARE(selectedIndexChangedSpy.count(), 3);
    QCOMPARE(modifiedChangedSpy.count(), 1);
    QCOMPARE(model.selectedIndex(), model.index(3, 0));
}

QTEST_GUILESS_MAIN(UpsModelTest)
#include "upsModelTest.moc"
