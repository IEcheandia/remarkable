#include <QTest>
#include <QSignalSpy>
#include <QXmlStreamWriter>
#include <QTemporaryFile>

#include "../src/deviceServer.h"
#include "resultsStorageService.h"

using precitec::interface::KeyHandle;
using precitec::interface::SmpKeyValue;
using precitec::interface::TKeyValue;
using precitec::storage::DeviceServer;
using precitec::storage::ResultsStorageService;

static const std::string s_enabled{"ResultsStorageEnabled"};
static const std::string s_maxRelativeStorage("ResultsRelativeDiskUsage");
static const std::string s_maxCacheEntries("ResultsCacheEntries");
static const std::string s_shutoff("ResultsShutOff");

class DeviceServerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testNoResultsStorage();
    void testEnabled_data();
    void testEnabled();
    void testShutOff_data();
    void testShutOff();
    void testDiskUsage_data();
    void testDiskUsage();
    void testMaxCacheEntries_data();
    void testMaxCacheEntries();
    void testInitFromConfig_data();
    void testInitFromConfig();
    void testInitFromConfigNoFile();
    void testInitFromConfigEmptyFile();
    void testCreateFile_data();
    void testCreateFile();
};

void DeviceServerTest::testCtor()
{
    DeviceServer server;
    QCOMPARE(server.configFile(), QString());
    QVERIFY(server.resultsStorage() == nullptr);
}

void DeviceServerTest::testNoResultsStorage()
{
    DeviceServer server;
    QCOMPARE(server.get(0).empty(), true);
    QCOMPARE(server.get(s_enabled)->isHandleValid(), false);
    QCOMPARE(server.get(s_maxRelativeStorage)->isHandleValid(), false);
    QCOMPARE(server.get(s_maxCacheEntries)->isHandleValid(), false);
    QCOMPARE(server.get(s_shutoff)->isHandleValid(), false);
    // get for keyhandle should not be valid
    QCOMPARE(server.get(KeyHandle{1})->isHandleValid(), false);

    // set for all should return invalid handle
    QCOMPARE(server.set(SmpKeyValue(new TKeyValue<bool>(s_enabled, true, false, true, true))).handle(), -1);
    QCOMPARE(server.set(SmpKeyValue(new TKeyValue<double>(s_maxRelativeStorage, 0.9, 0.0, 1.0, 0.9))).handle(), -1);
    QCOMPARE(server.set(SmpKeyValue(new TKeyValue<uint>(s_maxCacheEntries, 500, 0, 999999, 500))).handle(), -1);
}

void DeviceServerTest::testEnabled_data()
{
    QTest::addColumn<bool>("initValue");
    QTest::addColumn<bool>("changeValue");

    QTest::newRow("true -> false") << true << false;
    QTest::newRow("false -> true") << false << true;
    QTest::newRow("true -> true") << true << true;
    QTest::newRow("false -> false") << false << false;
}

void DeviceServerTest::testEnabled()
{
    DeviceServer server;
    ResultsStorageService service;
    server.setResultsStorage(&service);
    QCOMPARE(server.resultsStorage(), &service);

    // set config file
    QTemporaryFile file;
    QVERIFY(file.open());
    server.setConfigFile(file.fileName());
    QCOMPARE(server.configFile(), file.fileName());

    QFETCH(bool, initValue);
    service.setEnabled(initValue);
    QCOMPARE(service.isEnabled(), initValue);

    // get the device key
    auto kv = server.get(s_enabled);
    QCOMPARE(kv->isHandleValid(), true);
    QCOMPARE(kv->isReadOnly(), false);
    QCOMPARE(kv->key(), s_enabled);
    QCOMPARE(kv->defValue<bool>(), true);
    QCOMPARE(kv->value<bool>(), initValue);

    // change it
    QFETCH(bool, changeValue);
    auto change = kv->clone();
    change->setValue(changeValue);
    QCOMPARE(server.set(change).handle(), 1);
    QCOMPARE(service.isEnabled(), changeValue);

    // get again
    kv = server.get(s_enabled);
    QCOMPARE(kv->isHandleValid(), true);
    QCOMPARE(kv->isReadOnly(), false);
    QCOMPARE(kv->key(), s_enabled);
    QCOMPARE(kv->value<bool>(), changeValue);

    // read in the config file
    const auto fileContent = file.readAll();
    QVERIFY(!fileContent.isEmpty());
    QVERIFY(fileContent.contains(QStringLiteral("<ResultsStorageEnabled>%1</ResultsStorageEnabled>").arg(changeValue ? QStringLiteral("true") : QStringLiteral("false")).toUtf8()));
}

void DeviceServerTest::testShutOff_data()
{
    QTest::addColumn<bool>("initValue");
    QTest::addColumn<bool>("changeValue");

    QTest::newRow("true -> false") << true << false;
    QTest::newRow("false -> true") << false << true;
    QTest::newRow("true -> true") << true << true;
    QTest::newRow("false -> false") << false << false;
}

void DeviceServerTest::testShutOff()
{
    DeviceServer server;
    ResultsStorageService service;
    server.setResultsStorage(&service);
    QCOMPARE(server.resultsStorage(), &service);

    QFETCH(bool, initValue);
    service.setShutdown(initValue);
    QCOMPARE(service.isShutdown(), initValue);

    // get the device key
    auto kv = server.get(s_shutoff);
    QCOMPARE(kv->isHandleValid(), true);
    QCOMPARE(kv->isReadOnly(), true);
    QCOMPARE(kv->key(), s_shutoff);
    QCOMPARE(kv->defValue<bool>(), false);
    QCOMPARE(kv->value<bool>(), initValue);

    // change it
    QFETCH(bool, changeValue);
    auto change = kv->clone();
    change->setValue(changeValue);
    QCOMPARE(server.set(change).handle(), -1);
    // read only values cannot be set
    QCOMPARE(service.isShutdown(), initValue);

    // get again
    kv = server.get(s_shutoff);
    QCOMPARE(kv->isHandleValid(), true);
    QCOMPARE(kv->isReadOnly(), true);
    QCOMPARE(kv->key(), s_shutoff);
    QCOMPARE(kv->value<bool>(), initValue);
}

void DeviceServerTest::testDiskUsage_data()
{
    QTest::addColumn<double>("initValue");
    QTest::addColumn<double>("expectedInitValue");
    QTest::addColumn<double>("changeValue");
    QTest::addColumn<double>("expectedChangeValue");

    QTest::newRow("0.0 -> 1.0") << 0.0 << 0.0 << 1.0 << 1.0;
    QTest::newRow("-1.0 -> 2.0") << -1.0 << 0.0 << 2.0 << 1.0;
    QTest::newRow("2.0 -> -1.0") << 2.0 << 1.0 << -1.0 << 0.0;
    QTest::newRow("0.5 -> 0.5") << 0.5 << 0.5 << 0.5 << 0.5;
}

void DeviceServerTest::testDiskUsage()
{
    DeviceServer server;
    ResultsStorageService service;
    server.setResultsStorage(&service);
    QCOMPARE(server.resultsStorage(), &service);

    // set config file
    QTemporaryFile file;
    QVERIFY(file.open());
    server.setConfigFile(file.fileName());
    QCOMPARE(server.configFile(), file.fileName());

    QFETCH(double, initValue);
    service.setMaxRelativeDiskUsage(initValue);
    QTEST(service.maxRelativeDiskUsage(), "expectedInitValue");

    // get the device key
    auto kv = server.get(s_maxRelativeStorage);
    QCOMPARE(kv->isHandleValid(), true);
    QCOMPARE(kv->isReadOnly(), false);
    QCOMPARE(kv->key(), s_maxRelativeStorage);
    QCOMPARE(kv->minima<double>(), 0.0);
    QCOMPARE(kv->maxima<double>(), 1.0);
    QCOMPARE(kv->defValue<double>(), 0.9);
    QTEST(kv->value<double>(), "expectedInitValue");

    // change
    QFETCH(double, changeValue);
    auto changeKv = kv->clone();
    changeKv->setValue(changeValue);
    QCOMPARE(server.set(changeKv).handle(), 1);
    QFETCH(double, expectedChangeValue);
    QCOMPARE(service.maxRelativeDiskUsage(), expectedChangeValue);

    // get key again
    kv = server.get(s_maxRelativeStorage);
    QCOMPARE(kv->isHandleValid(), true);
    QCOMPARE(kv->isReadOnly(), false);
    QCOMPARE(kv->key(), s_maxRelativeStorage);
    QCOMPARE(kv->value<double>(), expectedChangeValue);

    // read in the config file
    const auto fileContent = file.readAll();
    QVERIFY(!fileContent.isEmpty());
    QVERIFY(fileContent.contains(QStringLiteral("<ResultsRelativeDiskUsage>%1</ResultsRelativeDiskUsage>").arg(expectedChangeValue).toUtf8()));
}

void DeviceServerTest::testMaxCacheEntries_data()
{
    QTest::addColumn<uint>("initValue");
    QTest::addColumn<uint>("expectedInitValue");
    QTest::addColumn<uint>("changeValue");
    QTest::addColumn<uint>("expectedChangeValue");

    QTest::newRow("0 -> 100") << 0u << 0u << 100u << 100u;
    QTest::newRow("100 -> 1000000") << 100u << 100u << 1000000u << 999999u;
    QTest::newRow("1000000 -> 0") << 1000000u << 999999u << 0u << 0u;
    QTest::newRow("500 -> 500") << 500u << 500u << 500u << 500u;
}

void DeviceServerTest::testMaxCacheEntries()
{
    DeviceServer server;
    ResultsStorageService service;
    server.setResultsStorage(&service);
    QCOMPARE(server.resultsStorage(), &service);

    // set config file
    QTemporaryFile file;
    QVERIFY(file.open());
    server.setConfigFile(file.fileName());
    QCOMPARE(server.configFile(), file.fileName());

    QFETCH(uint, initValue);
    service.setMaxCacheEntries(initValue);
    QTEST(uint(service.maxCacheEntries()), "expectedInitValue");

    // get the device key
    auto kv = server.get(s_maxCacheEntries);
    QCOMPARE(kv->isHandleValid(), true);
    QCOMPARE(kv->isReadOnly(), false);
    QCOMPARE(kv->key(), s_maxCacheEntries);
    QCOMPARE(kv->minima<uint>(), 0u);
    QCOMPARE(kv->maxima<uint>(), 999999u);
    QCOMPARE(kv->defValue<uint>(), 500u);
    QTEST(kv->value<uint>(), "expectedInitValue");

    // change
    QFETCH(uint, changeValue);
    auto changeKv = kv->clone();
    changeKv->setValue(changeValue);
    QCOMPARE(server.set(changeKv).handle(), 1);
    QFETCH(uint, expectedChangeValue);
    QCOMPARE(uint(service.maxCacheEntries()), expectedChangeValue);

    // get key again
    kv = server.get(s_maxCacheEntries);
    QCOMPARE(kv->isHandleValid(), true);
    QCOMPARE(kv->isReadOnly(), false);
    QCOMPARE(kv->key(), s_maxCacheEntries);
    QCOMPARE(kv->value<uint>(), expectedChangeValue);

    // read in the config file
    const auto fileContent = file.readAll();
    QVERIFY(!fileContent.isEmpty());
    QVERIFY(fileContent.contains(QStringLiteral("<ResultsCacheEntries>%1</ResultsCacheEntries>").arg(expectedChangeValue).toUtf8()));
}

void DeviceServerTest::testInitFromConfig_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<double>("diskUsage");
    QTest::addColumn<uint>("cacheEntries");

    QTest::newRow("enabled/0.5/400") << true << 0.5 << 400u;
    QTest::newRow("disabled/0.0/999990u") << false << 0.0 << 999990u;
}

void DeviceServerTest::testInitFromConfig()
{
    QTemporaryFile file;
    QVERIFY(file.open());
    QXmlStreamWriter stream{&file};
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement(QStringLiteral("key_value_configuration"));
    QFETCH(bool, enabled);
    stream.writeTextElement(QStringLiteral("ResultsStorageEnabled"), enabled ? QStringLiteral("true") : QStringLiteral("false"));
    QFETCH(double, diskUsage);
    stream.writeTextElement(QStringLiteral("ResultsRelativeDiskUsage"), QString::number(diskUsage));
    QFETCH(uint, cacheEntries);
    stream.writeTextElement(QStringLiteral("ResultsCacheEntries"), QString::number(cacheEntries));
    stream.writeEndElement();
    stream.writeEndDocument();
    QVERIFY(file.flush());

    DeviceServer server;
    ResultsStorageService service;
    server.setResultsStorage(&service);
    QCOMPARE(server.resultsStorage(), &service);
    server.setConfigFile(file.fileName());

    server.initFromFile();
    auto configuration = server.get();
    QCOMPARE(configuration.empty(), false);
    QCOMPARE(configuration.size(), 4);
    auto it = std::find_if(configuration.begin(), configuration.end(), [] (const auto kv) { return kv->key() == s_enabled; });
    QVERIFY(it != configuration.end());
    QCOMPARE((*it)->value<bool>(), enabled);

    it = std::find_if(configuration.begin(), configuration.end(), [] (const auto kv) { return kv->key() == s_maxRelativeStorage; });
    QVERIFY(it != configuration.end());
    QCOMPARE((*it)->value<double>(), diskUsage);

    it = std::find_if(configuration.begin(), configuration.end(), [] (const auto kv) { return kv->key() == s_maxCacheEntries; });
    QVERIFY(it != configuration.end());
    QCOMPARE((*it)->value<uint>(), cacheEntries);
}

void DeviceServerTest::testInitFromConfigNoFile()
{
    DeviceServer server;
    ResultsStorageService service;
    server.setResultsStorage(&service);
    QCOMPARE(server.resultsStorage(), &service);

    server.initFromFile();
    auto configuration = server.get();
    QCOMPARE(configuration.empty(), false);
    QCOMPARE(configuration.size(), 4);
    auto it = std::find_if(configuration.begin(), configuration.end(), [] (const auto kv) { return kv->key() == s_enabled; });
    QVERIFY(it != configuration.end());
    QCOMPARE((*it)->value<bool>(), true);

    it = std::find_if(configuration.begin(), configuration.end(), [] (const auto kv) { return kv->key() == s_maxRelativeStorage; });
    QVERIFY(it != configuration.end());
    QCOMPARE((*it)->value<double>(), 0.9);

    it = std::find_if(configuration.begin(), configuration.end(), [] (const auto kv) { return kv->key() == s_maxCacheEntries; });
    QVERIFY(it != configuration.end());
    QCOMPARE((*it)->value<uint>(), 500);
}

void DeviceServerTest::testInitFromConfigEmptyFile()
{
    DeviceServer server;
    ResultsStorageService service;
    server.setResultsStorage(&service);
    QCOMPARE(server.resultsStorage(), &service);

    QTemporaryFile file;
    QVERIFY(file.open());
    server.setConfigFile(file.fileName());

    server.initFromFile();
    auto configuration = server.get();
    QCOMPARE(configuration.empty(), false);
    QCOMPARE(configuration.size(), 4);
    auto it = std::find_if(configuration.begin(), configuration.end(), [] (const auto kv) { return kv->key() == s_enabled; });
    QVERIFY(it != configuration.end());
    QCOMPARE((*it)->value<bool>(), true);

    it = std::find_if(configuration.begin(), configuration.end(), [] (const auto kv) { return kv->key() == s_maxRelativeStorage; });
    QVERIFY(it != configuration.end());
    QCOMPARE((*it)->value<double>(), 0.9);

    it = std::find_if(configuration.begin(), configuration.end(), [] (const auto kv) { return kv->key() == s_maxCacheEntries; });
    QVERIFY(it != configuration.end());
    QCOMPARE((*it)->value<uint>(), 500);
}

void DeviceServerTest::testCreateFile_data()
{
    QTest::addColumn<uint>("initValue");
    QTest::addColumn<uint>("expectedInitValue");
    QTest::addColumn<uint>("changeValue");
    QTest::addColumn<uint>("expectedChangeValue");

    QTest::newRow("0 -> 100") << 0u << 0u << 100u << 100u;
    QTest::newRow("100 -> 1000000") << 100u << 100u << 1000000u << 999999u;
    QTest::newRow("1000000 -> 0") << 1000000u << 999999u << 0u << 0u;
    QTest::newRow("500 -> 500") << 500u << 500u << 500u << 500u;
}

void DeviceServerTest::testCreateFile()
{
    DeviceServer server;
    ResultsStorageService service;
    server.setResultsStorage(&service);
    QCOMPARE(server.resultsStorage(), &service);

    // set config file
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const auto filePath = dir.filePath(QStringLiteral("storage.xml"));

    server.setConfigFile(filePath);
    QCOMPARE(server.configFile(), filePath);

    QFETCH(uint, initValue);
    service.setMaxCacheEntries(initValue);
    QTEST(uint(service.maxCacheEntries()), "expectedInitValue");

    // get the device key
    auto kv = server.get(s_maxCacheEntries);
    QCOMPARE(kv->isHandleValid(), true);
    QCOMPARE(kv->isReadOnly(), false);
    QCOMPARE(kv->key(), s_maxCacheEntries);
    QCOMPARE(kv->minima<uint>(), 0u);
    QCOMPARE(kv->maxima<uint>(), 999999u);
    QCOMPARE(kv->defValue<uint>(), 500u);
    QTEST(kv->value<uint>(), "expectedInitValue");

    // change
    QFETCH(uint, changeValue);
    auto changeKv = kv->clone();
    changeKv->setValue(changeValue);
    QCOMPARE(server.set(changeKv).handle(), 1);
    QFETCH(uint, expectedChangeValue);
    QCOMPARE(uint(service.maxCacheEntries()), expectedChangeValue);

    // get key again
    kv = server.get(s_maxCacheEntries);
    QCOMPARE(kv->isHandleValid(), true);
    QCOMPARE(kv->isReadOnly(), false);
    QCOMPARE(kv->key(), s_maxCacheEntries);
    QCOMPARE(kv->value<uint>(), expectedChangeValue);

    // read in the config file
    QFile file{filePath};
    QVERIFY(file.exists());
    QVERIFY(file.open(QIODevice::ReadOnly));
    const auto fileContent = file.readAll();
    QVERIFY(!fileContent.isEmpty());
    QVERIFY(fileContent.contains(QStringLiteral("<ResultsCacheEntries>%1</ResultsCacheEntries>").arg(expectedChangeValue).toUtf8()));
}

QTEST_GUILESS_MAIN(DeviceServerTest)
#include "testDeviceServer.moc"
