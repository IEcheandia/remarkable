#include <QTest>

#include "../src/KeyValueConfigUpdater/nutConfigUpdater.h"

Q_DECLARE_METATYPE(precitec::NutConfigUpdater::Ups)

class NutConfigUpdaterTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testUpdate_data();
    void testUpdate();
};

void NutConfigUpdaterTest::testUpdate_data()
{
    QTest::addColumn<QString>("sourceDirPath");
    QTest::addColumn<QString>("expectedNutConfPath");
    QTest::addColumn<QString>("expectedUpsConfPath");
    QTest::addColumn<precitec::NutConfigUpdater::Ups>("ups");

    QTest::newRow("ABB - none") << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/abb/")
                                << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/none/nut.conf")
                                << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/abb/ups.conf") << precitec::NutConfigUpdater::Ups::None;

    QTest::newRow("APC - none") << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/apc/")
                                << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/none/nut.conf")
                                << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/apc/ups.conf") << precitec::NutConfigUpdater::Ups::None;

    QTest::newRow("ABB G2 - none") << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/abbPowerValue11RTG2/")
                                << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/none/nut.conf")
                                << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/abbPowerValue11RTG2/ups.conf") << precitec::NutConfigUpdater::Ups::None;

    QTest::newRow("OmronUSB - none") << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/omronS8BAusb/")
                                << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/none/nut.conf")
                                << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/omronS8BAusb/ups.conf") << precitec::NutConfigUpdater::Ups::None;

    QTest::newRow("none - none") << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/none/")
                                 << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/none/nut.conf")
                                 << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/none/ups.conf") << precitec::NutConfigUpdater::Ups::None;

    QTest::newRow("APC - ABB") << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/apc/")
                               << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/abb/nut.conf")
                               << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/abb/ups.conf") << precitec::NutConfigUpdater::Ups::ABBPowerValue11RT;

    QTest::newRow("APC - ABB G2") << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/apc/")
                               << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/abbPowerValue11RTG2/nut.conf")
                               << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/abbPowerValue11RTG2/ups.conf") << precitec::NutConfigUpdater::Ups::ABBPowerValue11RTG2;

    QTest::newRow("APC - OmronUSB") << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/apc/")
                               << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/omronS8BAusb/nut.conf")
                               << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/omronS8BAusb/ups.conf") << precitec::NutConfigUpdater::Ups::OmronS8BA24D24D120LF_usb;

    QTest::newRow("ABB - APC") << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/abb/")
                               << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/apc/nut.conf")
                               << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/apc/ups.conf") << precitec::NutConfigUpdater::Ups::APCSmartUps;

    QTest::newRow("ABB - ABB G2") << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/abb/")
                               << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/abbPowerValue11RTG2/nut.conf")
                               << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/abbPowerValue11RTG2/ups.conf") << precitec::NutConfigUpdater::Ups::ABBPowerValue11RTG2;

    QTest::newRow("ABB - OmronUSB") << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/abb/")
                               << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/omronS8BAusb/nut.conf")
                               << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/omronS8BAusb/ups.conf") << precitec::NutConfigUpdater::Ups::OmronS8BA24D24D120LF_usb;

    QTest::newRow("ABB G2 - OmronUSB") << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/abbPowerValue11RTG2/")
                               << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/omronS8BAusb/nut.conf")
                               << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/omronS8BAusb/ups.conf") << precitec::NutConfigUpdater::Ups::OmronS8BA24D24D120LF_usb;

    QTest::newRow("OmronUSB - ABB") << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/omronS8BAusb/")
                                << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/abb/nut.conf")
                                << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/abb/ups.conf") << precitec::NutConfigUpdater::Ups::ABBPowerValue11RT;

    QTest::newRow("OmronUSB - APC") << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/omronS8BAusb/")
                                << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/apc/nut.conf")
                                << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/apc/ups.conf") << precitec::NutConfigUpdater::Ups::APCSmartUps;

    QTest::newRow("OmronUSB - ABB G2") << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/omronS8BAusb/")
                                << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/abbPowerValue11RTG2/nut.conf")
                                << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/abbPowerValue11RTG2/ups.conf") << precitec::NutConfigUpdater::Ups::ABBPowerValue11RTG2;

    QTest::newRow("none - ABB") << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/none/")
                                << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/abb/nut.conf")
                                << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/abb/ups.conf") << precitec::NutConfigUpdater::Ups::ABBPowerValue11RT;

    QTest::newRow("none - APC") << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/none/")
                                << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/apc/nut.conf")
                                << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/apc/ups.conf") << precitec::NutConfigUpdater::Ups::APCSmartUps;

    QTest::newRow("none - ABB G2") << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/none/")
                                << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/abbPowerValue11RTG2/nut.conf")
                                << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/abbPowerValue11RTG2/ups.conf") << precitec::NutConfigUpdater::Ups::ABBPowerValue11RTG2;

    QTest::newRow("none - OmronUsb") << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/none/")
                                << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/omronS8BAusb/nut.conf")
                                << QFINDTESTDATA("../plugins/configuration/autotests/testdata/upsModel/omronS8BAusb/ups.conf") << precitec::NutConfigUpdater::Ups::OmronS8BA24D24D120LF_usb;
}

void NutConfigUpdaterTest::testUpdate()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QFETCH(QString, sourceDirPath);
    QDir sourceDir{sourceDirPath};
    QVERIFY(QFile::copy(sourceDir.filePath(QStringLiteral("nut.conf")), dir.filePath(QStringLiteral("nut.conf"))));
    QVERIFY(QFile::copy(sourceDir.filePath(QStringLiteral("ups.conf")), dir.filePath(QStringLiteral("ups.conf"))));

    QFETCH(precitec::NutConfigUpdater::Ups, ups);
    precitec::NutConfigUpdater updater{ups, dir.path() + QStringLiteral("/")};
    updater();

    QFile nutConf{dir.filePath(QStringLiteral("nut.conf"))};
    QVERIFY(nutConf.open(QIODevice::ReadOnly));

    QFETCH(QString, expectedNutConfPath);
    QFile expectedNutConf{expectedNutConfPath};
    QVERIFY(expectedNutConf.open(QIODevice::ReadOnly));

    QCOMPARE(nutConf.readAll(), expectedNutConf.readAll());

    QFile upsConf{dir.filePath(QStringLiteral("ups.conf"))};
    QVERIFY(upsConf.open(QIODevice::ReadOnly));

    QFETCH(QString, expectedUpsConfPath);
    QFile expectedUpsConf{expectedUpsConfPath};
    QVERIFY(expectedUpsConf.open(QIODevice::ReadOnly));

    QCOMPARE(upsConf.readAll(), expectedUpsConf.readAll());
}

QTEST_GUILESS_MAIN(NutConfigUpdaterTest)
#include "nutConfigUpdaterTest.moc"
