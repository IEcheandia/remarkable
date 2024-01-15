#include <QTest>
#include <QTemporaryDir>
#include "../src/KeyValueConfigUpdater/etherCATStartupUpdater.h"
#include "../src/KeyValueConfigUpdater/etherCATConfigUpdater.h"

using precitec::EtherCATConfigUpdater;
using precitec::EtherCATStartupUpdater;

class EtherCATStartupUpdaterTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testStartup_data();
    void testStartup();
    void testConfigMissing();
    void testConfigExisting();
};

void EtherCATStartupUpdaterTest::testStartup_data()
{
    QTest::addColumn<QString>("connectConfig");
    QTest::addColumn<QString>("startupConfig");
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<QString>("expectedConnectConfig");
    QTest::addColumn<QString>("expectedStartupConfig");

    QTest::newRow("enable - enable") << QStringLiteral("StandaloneConnect.config") << QStringLiteral("StandaloneStartup.config") << true << QStringLiteral("StandaloneConnect.config") << QStringLiteral("StandaloneStartup.config");
    QTest::newRow("enable - disable") << QStringLiteral("StandaloneConnect.config") << QStringLiteral("StandaloneStartup.config") << false << QStringLiteral("StandaloneConnect_disabled.config") << QStringLiteral("StandaloneStartup_disabled.config");

    QTest::newRow("disable - disable") << QStringLiteral("StandaloneConnect_disabled.config") << QStringLiteral("StandaloneStartup_disabled.config") << false << QStringLiteral("StandaloneConnect_disabled.config") << QStringLiteral("StandaloneStartup_disabled.config");
    QTest::newRow("disable - enable") << QStringLiteral("StandaloneConnect_disabled.config") << QStringLiteral("StandaloneStartup_disabled.config") << true << QStringLiteral("StandaloneConnect.config") << QStringLiteral("StandaloneStartup.config");

    QTest::newRow("missing - enabled") << QStringLiteral("StandaloneConnect_missing.config") << QStringLiteral("StandaloneStartup_missing.config") << true << QStringLiteral("StandaloneConnect_missing_enabled.config") << QStringLiteral("StandaloneStartup_missing_enabled.config");

    QTest::newRow("missing - disabled") << QStringLiteral("StandaloneConnect_missing.config") << QStringLiteral("StandaloneStartup_missing.config") << false << QStringLiteral("StandaloneConnect_missing_disabled.config") << QStringLiteral("StandaloneStartup_missing_disabled.config");
}

void EtherCATStartupUpdaterTest::testStartup()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QDir{dir.path()}.mkdir(QStringLiteral("system_config")));

    QFETCH(QString, connectConfig);
    QFETCH(QString, startupConfig);
    QVERIFY(QFile::copy(QFINDTESTDATA(QStringLiteral("testdata/etherCATStartupUpdater/") + connectConfig), dir.filePath(QStringLiteral("system_config/StandaloneConnect.config"))));
    QVERIFY(QFile::copy(QFINDTESTDATA(QStringLiteral("testdata/etherCATStartupUpdater/") + startupConfig), dir.filePath(QStringLiteral("system_config/StandaloneStartup.config"))));

    QFETCH(bool, enabled);
    qputenv("WM_BASE_DIR", dir.path().toLocal8Bit());
    EtherCATStartupUpdater updater(enabled);
    updater();

    QFile connectConfigFile{dir.filePath(QStringLiteral("system_config/StandaloneConnect.config"))};
    QVERIFY(connectConfigFile.exists());
    QVERIFY(connectConfigFile.open(QIODevice::ReadOnly));
    QByteArray connectConfigData{connectConfigFile.readAll()};
    QVERIFY(!connectConfigData.isEmpty());

    QFile startupConfigFile{dir.filePath(QStringLiteral("system_config/StandaloneStartup.config"))};
    QVERIFY(startupConfigFile.exists());
    QVERIFY(startupConfigFile.open(QIODevice::ReadOnly));
    QByteArray startupConfigData{startupConfigFile.readAll()};
    QVERIFY(!startupConfigData.isEmpty());

    // expected files
    QFETCH(QString, expectedConnectConfig);
    QFile expectedConnectConfigFile{QFINDTESTDATA(QStringLiteral("testdata/etherCATStartupUpdater/") + expectedConnectConfig)};
    QVERIFY(expectedConnectConfigFile.exists());
    QVERIFY(expectedConnectConfigFile.open(QIODevice::ReadOnly));
    QCOMPARE(connectConfigData, expectedConnectConfigFile.readAll());

    QFETCH(QString, expectedStartupConfig);
    QFile expectedStartupConfigFile{QFINDTESTDATA(QStringLiteral("testdata/etherCATStartupUpdater/") + expectedStartupConfig)};
    QVERIFY(expectedStartupConfigFile.exists());
    QVERIFY(expectedStartupConfigFile.open(QIODevice::ReadOnly));
    QCOMPARE(startupConfigData, expectedStartupConfigFile.readAll());
}

void EtherCATStartupUpdaterTest::testConfigMissing()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(!QFile{dir.filePath(QStringLiteral("ethercat.conf"))}.exists());

    EtherCATConfigUpdater updater{QByteArrayLiteral("00:01:02:03:04:05"), dir.path() + QDir::separator()};
    updater();

    QFile confFile{dir.filePath(QStringLiteral("ethercat.conf"))};
    QVERIFY(confFile.exists());
    QVERIFY(confFile.open(QIODevice::ReadOnly));
    const QByteArray data{confFile.readAll()};
    QVERIFY(!data.isEmpty());

    QFile expected{QFINDTESTDATA(QStringLiteral("testdata/etherCATConfigUpdater/ethercat.conf"))};
    QVERIFY(expected.exists());
    QVERIFY(expected.open(QIODevice::ReadOnly));
    QCOMPARE(data, expected.readAll());
}

void EtherCATStartupUpdaterTest::testConfigExisting()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QFile::copy(QFINDTESTDATA(QStringLiteral("testdata/etherCATConfigUpdater/ethercat.conf")), dir.filePath(QStringLiteral("ethercat.conf"))));
    QVERIFY(QFile{dir.filePath(QStringLiteral("ethercat.conf"))}.exists());

    EtherCATConfigUpdater updater{QByteArrayLiteral("00:01:02:03:04:06"), dir.path() + QDir::separator()};
    updater();

    QFile confFile{dir.filePath(QStringLiteral("ethercat.conf"))};
    QVERIFY(confFile.exists());
    QVERIFY(confFile.open(QIODevice::ReadOnly));
    const QByteArray data{confFile.readAll()};
    QVERIFY(!data.isEmpty());

    QFile expected{QFINDTESTDATA(QStringLiteral("testdata/etherCATConfigUpdater/ethercat_modified.conf"))};
    QVERIFY(expected.exists());
    QVERIFY(expected.open(QIODevice::ReadOnly));
    QCOMPARE(data, expected.readAll());
}

QTEST_GUILESS_MAIN(EtherCATStartupUpdaterTest)
#include "etherCATStartupUpdaterTest.moc"
