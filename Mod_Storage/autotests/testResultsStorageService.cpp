#include <QTest>
#include <QTemporaryDir>
#include <QUuid>
#include <QJsonDocument>

#include "../src/resultsStorageService.h"
#include "event/results.h"
#include "../src/product.h"
#include "../src/productMetaData.h"
#include "../src/seam.h"
#include "../src/seamMetaData.h"
#include "../src/seamSeries.h"
#include "../src/seamSeriesMetaData.h"
#include "../src/parameterSet.h"
#include "../src/attribute.h"

using precitec::storage::Attribute;
using precitec::storage::ResultsStorageService;
using precitec::storage::SeamMetaData;
using precitec::storage::SeamSeriesMetaData;
using precitec::storage::Product;
using precitec::storage::ProductMetaData;
using precitec::interface::ImageContext;
using precitec::interface::ResultArgs;
using precitec::interface::ResultDoubleArray;
using precitec::interface::GeoDoublearray;

class ResultsStorageServiceTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testNioResultsSwitchedOff();
    void testDirectory();
    void testSetMaxCacheEntries();
    void testDirectoriesCreated();
    void testCacheRotation();
    void testResults();
    void testSetEnabled();
    void testMaxDiskUsage_data();
    void testMaxDiskUsage();
    void testShutdown();
    void testEndSeamInspectionForDeletedProduct();
    void testStartProductInspectionWithoutProduct();
    void testEndProductInspectionForDeletedProduct();
    void testResultAfterSeamEnd();
    void testSerialNumberMismatch();
    void testProductInstanceMismatch();
    void testProductMetaData();
    void testSeamSeriesMetaData();
    void testSeamMetaData();
    void testSeamLwmWithoutResult();
    void testSeamLwmWithResultAfterSeamEnd();
    void testSeamAndProductLwmWithoutResult();
    void testLwmWithResultAfterProductEnd();
};

void ResultsStorageServiceTest::testCtor()
{
    ResultsStorageService service;
    QCOMPARE(service.resultsDirectory(), QString());
    QCOMPARE(service.nioResultsSwitchedOff(), false);
    QCOMPARE(service.maxCacheEntries(), std::size_t(500));
    QCOMPARE(service.isEnabled(), true);
    QCOMPARE(service.isShutdown(), false);
    QCOMPARE(service.maxRelativeDiskUsage(), 0.9);
    QCOMPARE(service.processingState(), ResultsStorageService::ProcessingState::Idle);
    QCOMPARE(service.seamProcessingWithExternalLwm(), false);
    QCOMPARE(service.isCommunicationToLWMDeviceActive(), false);
}

void ResultsStorageServiceTest::testNioResultsSwitchedOff()
{
    ResultsStorageService service;
    QCOMPARE(service.nioResultsSwitchedOff(), false);
    service.setNioResultsSwitchedOff(true);
    QCOMPARE(service.nioResultsSwitchedOff(), true);
    service.setNioResultsSwitchedOff(false);
    QCOMPARE(service.nioResultsSwitchedOff(), false);
}

void ResultsStorageServiceTest::testDirectory()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    ResultsStorageService service;
    QCOMPARE(service.resultsDirectory(), QString());
    service.setResultsDirectory(dir.path());
    QCOMPARE(service.resultsDirectory(), dir.path());
}

void ResultsStorageServiceTest::testSetMaxCacheEntries()
{
    ResultsStorageService service;
    QCOMPARE(service.maxCacheEntries(), std::size_t(500));
    service.setMaxCacheEntries(5);
    QCOMPARE(service.maxCacheEntries(), std::size_t(5));
}

void ResultsStorageServiceTest::testDirectoriesCreated()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QDir resultsDir{dir.filePath(QStringLiteral("results"))};
    QVERIFY(QDir{}.mkpath(resultsDir.path()));
    QVERIFY(resultsDir.exists());
    QFile cacheFile{resultsDir.filePath(QStringLiteral(".results_cache"))};

    ResultsStorageService service;
    service.setResultsDirectory(resultsDir.path());

    // create a product and seam for inspection
    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto seam = p.createSeam();

    auto productInstance = QUuid::createUuid();

    QCOMPARE(service.processingState(), ResultsStorageService::ProcessingState::Idle);
    service.startProductInspection(&p, productInstance, {});
    QCOMPARE(service.processingState(), ResultsStorageService::ProcessingState::ProductInspection);
    QDateTime now = QDateTime::currentDateTimeUtc();
    service.startSeamInspection(seam, productInstance, 1234);
    QCOMPARE(service.processingState(), ResultsStorageService::ProcessingState::SeamInspection);
    service.endSeamInspection();
    QCOMPARE(service.processingState(), ResultsStorageService::ProcessingState::ProductInspection);
    QCOMPARE(cacheFile.exists(), false);
    service.endProductInspection(&p);
    QCOMPARE(service.processingState(), ResultsStorageService::ProcessingState::Idle);
    QTRY_VERIFY(cacheFile.exists());

    QVERIFY(cacheFile.open(QIODevice::ReadOnly));
    const auto data = cacheFile.readAll();
    QVERIFY(!data.isEmpty());
    const auto lines = data.split('\n');
    QCOMPARE(lines.size(), 2);
    QCOMPARE(lines.at(1), QByteArray());

    QDir productInstanceDir{QString::fromUtf8(lines.at(0))};
    QVERIFY(productInstanceDir.exists());
    QVERIFY(productInstanceDir.exists(QStringLiteral("metadata.json")));

    const auto parts = productInstanceDir.dirName().split(QStringLiteral("-SN-"));
    QCOMPARE(parts.count(), 2);
    QCOMPARE(QUuid(parts.at(0)), productInstance);
    QCOMPARE(parts.at(1).toInt(), 1234);

    const auto metadata = ProductMetaData::parse(productInstanceDir);
    QVERIFY(metadata.isUuidValid());
    QVERIFY(metadata.isNumberValid());
    QVERIFY(metadata.isDateValid());
    QVERIFY(metadata.isNioValid());
    QVERIFY(metadata.isNioSwitchedOffValid());

    QCOMPARE(metadata.uuid(), productInstance);
    QCOMPARE(metadata.number(), 1234u);
    QCOMPARE(metadata.nio(), true);
    QCOMPARE(metadata.nioSwitchedOff(), false);
    QCOMPARE(metadata.date(), now);

    QVERIFY(productInstanceDir.cdUp());
    QCOMPARE(QUuid(productInstanceDir.dirName()), p.uuid());
}

namespace
{
QString toString(const QUuid &uuid)
{
    auto ret = uuid.toString();
    ret.remove(0, 1).chop(1);
    return ret;
}
}

void ResultsStorageServiceTest::testCacheRotation()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QDir resultsDir{dir.filePath(QStringLiteral("results"))};
    QVERIFY(QDir{}.mkpath(resultsDir.path()));
    QVERIFY(resultsDir.exists());
    QFile cacheFile{resultsDir.filePath(QStringLiteral(".results_cache"))};

    ResultsStorageService service;
    service.setMaxCacheEntries(5);
    service.setMaxRelativeDiskUsage(1.0);
    service.setResultsDirectory(resultsDir.path());

    // create a product and seam for inspection
    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto seam = p.createSeam();

    auto productInstance1 = QUuid::createUuid();
    auto productInstance2 = QUuid::createUuid();
    auto productInstance3 = QUuid::createUuid();
    auto productInstance4 = QUuid::createUuid();
    auto productInstance5 = QUuid::createUuid();
    auto productInstance6 = QUuid::createUuid();

    service.startProductInspection(&p, productInstance1, {});
    service.startSeamInspection(seam, productInstance1, 1234);
    service.endSeamInspection();
    service.endProductInspection(&p);

    QDir productDir{resultsDir.filePath(toString(p.uuid()))};
    QTRY_VERIFY(productDir.exists());

    QDir productInstance1Dir{productDir.filePath(toString(productInstance1) + QStringLiteral("-SN-1234"))};
    QTRY_VERIFY(productInstance1Dir.exists());

    service.startProductInspection(&p, productInstance2, {});
    service.startSeamInspection(seam, productInstance2, 2234);
    service.endSeamInspection();
    service.endProductInspection(&p);

    service.startProductInspection(&p, productInstance3, {});
    service.startSeamInspection(seam, productInstance3, 3234);
    service.endSeamInspection();
    service.endProductInspection(&p);

    service.startProductInspection(&p, productInstance4, {});
    service.startSeamInspection(seam, productInstance4, 4234);
    service.endSeamInspection();
    service.endProductInspection(&p);

    service.startProductInspection(&p, productInstance5, {});
    service.startSeamInspection(seam, productInstance5, 5234);
    service.endSeamInspection();
    service.endProductInspection(&p);

    service.startProductInspection(&p, productInstance6, {});
    service.startSeamInspection(seam, productInstance6, 6234);
    service.endSeamInspection();
    service.endProductInspection(&p);

    QDir productInstance2Dir{productDir.filePath(toString(productInstance2) + QStringLiteral("-SN-2234"))};
    QTRY_VERIFY(productInstance2Dir.exists());
    QDir productInstance3Dir{productDir.filePath(toString(productInstance3) + QStringLiteral("-SN-3234"))};
    QTRY_VERIFY(productInstance3Dir.exists());
    QDir productInstance4Dir{productDir.filePath(toString(productInstance4) + QStringLiteral("-SN-4234"))};
    QTRY_VERIFY(productInstance4Dir.exists());
    QDir productInstance5Dir{productDir.filePath(toString(productInstance5) + QStringLiteral("-SN-5234"))};
    QTRY_VERIFY(productInstance5Dir.exists());
    QDir productInstance6Dir{productDir.filePath(toString(productInstance6) + QStringLiteral("-SN-6234"))};
    QTRY_VERIFY(productInstance6Dir.exists());
    QCOMPARE(productInstance1Dir.exists(), false);

    QVERIFY(cacheFile.open(QIODevice::ReadOnly));
    const auto data = cacheFile.readAll();
    QVERIFY(!data.isEmpty());
    const auto lines = data.split('\n');
    QCOMPARE(lines.size(), 6);
    QVERIFY(lines.at(0).endsWith("2234/"));
    QVERIFY(lines.at(1).endsWith("3234/"));
    QVERIFY(lines.at(2).endsWith("4234/"));
    QVERIFY(lines.at(3).endsWith("5234/"));
    QVERIFY(lines.at(4).endsWith("6234/"));
    QCOMPARE(lines.at(5), QByteArray());

    // changing the number of products to keep should delete some entries
    service.setMaxCacheEntries(2);
    QTRY_COMPARE(productInstance2Dir.exists(), false);
    QTRY_COMPARE(productInstance3Dir.exists(), false);
    QTRY_COMPARE(productInstance4Dir.exists(), false);
    // waiting a little bit to ensure the other directories don't get deleted
    QTest::qWait(100);
    QCOMPARE(productInstance5Dir.exists(), true);
    QCOMPARE(productInstance6Dir.exists(), true);
}

void ResultsStorageServiceTest::testResults()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QDir resultsDir{dir.filePath(QStringLiteral("results"))};
    QVERIFY(QDir{}.mkpath(resultsDir.path()));
    QVERIFY(resultsDir.exists());
    QFile cacheFile{resultsDir.filePath(QStringLiteral(".results_cache"))};

    ResultsStorageService service;
    service.setMaxCacheEntries(5);
    service.setResultsDirectory(resultsDir.path());

    // create a product and seam for inspection
    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto seam = p.createSeam();

    auto productInstance = QUuid::createUuid();

    service.startProductInspection(&p, productInstance, {});
    service.startSeamInspection(seam, productInstance, 1234);

    GeoDoublearray values;
    values.ref().getData().push_back(0.0);
    values.ref().getData().push_back(0.1);
    values.ref().getData().push_back(1.0);
    values.ref().getRank().assign(3, 255);
    ResultDoubleArray result{Poco::UUIDGenerator().createRandom(), precitec::interface::Missmatch, precitec::interface::XCoordOutOfLimits, ImageContext{}, values, precitec::geo2d::TRange<double>(0.0, 2.0), true};
    QCOMPARE(result.isValid(), true);

    service.addResult(result);
    service.addResult(result);

    service.endSeamInspection();
    QCOMPARE(cacheFile.exists(), false);
    service.endProductInspection(&p);
    QTRY_VERIFY(cacheFile.exists());

    QDir productDir{resultsDir.filePath(toString(p.uuid()))};
    QTRY_VERIFY(productDir.exists());

    QDir productInstanceDir{productDir.filePath(toString(productInstance) + QStringLiteral("-SN-1234"))};
    QTRY_VERIFY(productInstanceDir.exists());

    QDir seamSeriesDir{productInstanceDir.filePath(QStringLiteral("seam_series0000"))};
    QVERIFY(seamSeriesDir.exists());
    QDir seamDir{seamSeriesDir.filePath(QStringLiteral("seam0000"))};
    QVERIFY(seamDir.exists());
    QVERIFY(seamDir.exists(QStringLiteral("2.result")));
}

void ResultsStorageServiceTest::testSetEnabled()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QDir resultsDir{dir.filePath(QStringLiteral("results"))};
    QVERIFY(QDir{}.mkpath(resultsDir.path()));
    QVERIFY(resultsDir.exists());
    QFile cacheFile{resultsDir.filePath(QStringLiteral(".results_cache"))};

    ResultsStorageService service;
    service.setResultsDirectory(resultsDir.path());
    QCOMPARE(service.isEnabled(), true);
    service.setEnabled(false);
    QCOMPARE(service.isEnabled(), false);

    // create a product and seam for inspection
    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto seam = p.createSeam();

    auto productInstance = QUuid::createUuid();
    service.startProductInspection(&p, productInstance, {});

    // now enable again
    service.setEnabled(true);
    QCOMPARE(service.isEnabled(), true);

    service.startSeamInspection(seam, productInstance, 1234);

    GeoDoublearray values;
    values.ref().getData().push_back(0.0);
    values.ref().getData().push_back(0.1);
    values.ref().getData().push_back(1.0);
    values.ref().getRank().assign(3, 255);
    ResultDoubleArray result{Poco::UUIDGenerator().createRandom(), precitec::interface::Missmatch, precitec::interface::XCoordOutOfLimits, ImageContext{}, values, precitec::geo2d::TRange<double>(0.0, 2.0), true};
    QCOMPARE(result.isValid(), true);
    service.addResult(result);

    service.endSeamInspection();
    QCOMPARE(cacheFile.exists(), false);
    service.endProductInspection(&p);
    QTest::qWait(100);
    QCOMPARE(cacheFile.exists(), false);

    QCOMPARE(service.isEnabled(), true);
    // and another product
    productInstance = QUuid::createUuid();
    service.startProductInspection(&p, productInstance, {});
    service.startSeamInspection(seam, productInstance, 1235);
    service.endSeamInspection();
    service.endProductInspection(&p);
    QTRY_COMPARE(cacheFile.exists(), true);
}

void ResultsStorageServiceTest::testMaxDiskUsage_data()
{
    QTest::addColumn<double>("maxRelativeDiskUsage");
    QTest::addColumn<double>("expectedMaxRelativeDiskUsage");

    QTest::newRow("0.0") << 0.0 << 0.0;
    QTest::newRow("< 0") << -1.0 << 0.0;
    QTest::newRow("1.0") << 1.0 << 1.0;
    QTest::newRow(">1") << 1.1 << 1.0;
    QTest::newRow("0.5") << 0.5 << 0.5;
}

void ResultsStorageServiceTest::testMaxDiskUsage()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QDir resultsDir{dir.filePath(QStringLiteral("results"))};
    QVERIFY(QDir{}.mkpath(resultsDir.path()));
    QVERIFY(resultsDir.exists());
    ResultsStorageService service;
    service.setResultsDirectory(resultsDir.path());
    QCOMPARE(service.maxRelativeDiskUsage(), 0.9);
    QFETCH(double, maxRelativeDiskUsage);
    service.setMaxRelativeDiskUsage(maxRelativeDiskUsage);
    QTEST(service.maxRelativeDiskUsage(), "expectedMaxRelativeDiskUsage");
    QCOMPARE(service.isShutdown(), false);
}

void ResultsStorageServiceTest::testShutdown()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QDir resultsDir{dir.filePath(QStringLiteral("results"))};
    QVERIFY(QDir{}.mkpath(resultsDir.path()));
    QVERIFY(resultsDir.exists());
    ResultsStorageService service;
    service.setResultsDirectory(resultsDir.path());
    service.setMaxRelativeDiskUsage(0.0);
    QCOMPARE(service.isShutdown(), false);
    QFile cacheFile{resultsDir.filePath(QStringLiteral(".results_cache"))};

    // create a product and seam for inspection
    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto seam = p.createSeam();

    auto productInstance = QUuid::createUuid();
    service.startProductInspection(&p, productInstance, {});
    QCOMPARE(service.isShutdown(), true);

    service.startSeamInspection(seam, productInstance, 1234);

    // now enable again
    service.setMaxRelativeDiskUsage(1.0);
    QCOMPARE(service.isShutdown(), true);

    GeoDoublearray values;
    values.ref().getData().push_back(0.0);
    values.ref().getData().push_back(0.1);
    values.ref().getData().push_back(1.0);
    values.ref().getRank().assign(3, 255);
    ResultDoubleArray result{Poco::UUIDGenerator().createRandom(), precitec::interface::Missmatch, precitec::interface::XCoordOutOfLimits, ImageContext{}, values, precitec::geo2d::TRange<double>(0.0, 2.0), true};
    QCOMPARE(result.isValid(), true);
    service.addResult(result);

    result.setNio(true);
    QCOMPARE(result.isNio(), true);
    service.addNio(result);

    service.endSeamInspection();
    QCOMPARE(cacheFile.exists(), false);
    service.endProductInspection(&p);
    QTest::qWait(100);
    QCOMPARE(cacheFile.exists(), false);

    QCOMPARE(service.isShutdown(), true);
    // and another product
    productInstance = QUuid::createUuid();
    service.startProductInspection(&p, productInstance, {});
    QCOMPARE(service.isShutdown(), false);
    service.startSeamInspection(seam, productInstance, 1235);
    service.endSeamInspection();
    service.endProductInspection(&p);
    QTRY_COMPARE(cacheFile.exists(), true);
}

void ResultsStorageServiceTest::testEndSeamInspectionForDeletedProduct()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QDir resultsDir{dir.filePath(QStringLiteral("results"))};
    QVERIFY(QDir{}.mkpath(resultsDir.path()));
    QVERIFY(resultsDir.exists());
    QFile cacheFile{resultsDir.filePath(QStringLiteral(".results_cache"))};

    ResultsStorageService service;
    service.setMaxCacheEntries(5);
    service.setMaxRelativeDiskUsage(1.0);
    service.setResultsDirectory(resultsDir.path());

    // create a product and seam for inspection
    QScopedPointer<Product> p{new Product{QUuid::createUuid()}};
    QScopedPointer<Product> copy{new Product{p->uuid()}};
    copy->createFirstSeamSeries();
    auto seam = copy->createSeam();

    auto productInstance1 = QUuid::createUuid();

    service.startProductInspection(p.data(), productInstance1, {});
    service.startSeamInspection(seam, productInstance1, 1234);

    // now delete product, which triggered a crash
    QCOMPARE(service.currentProduct().isNull(), false);
    p.reset();
    QCOMPARE(service.currentProduct().isNull(), true);

    service.endSeamInspection();
    service.endProductInspection({});
}

void ResultsStorageServiceTest::testStartProductInspectionWithoutProduct()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QDir resultsDir{dir.filePath(QStringLiteral("results"))};
    QVERIFY(QDir{}.mkpath(resultsDir.path()));
    QVERIFY(resultsDir.exists());
    QFile cacheFile{resultsDir.filePath(QStringLiteral(".results_cache"))};

    ResultsStorageService service;
    service.setMaxCacheEntries(5);
    service.setMaxRelativeDiskUsage(1.0);
    service.setResultsDirectory(resultsDir.path());

    QCOMPARE(service.currentProduct().isNull(), true);
    service.startProductInspection({}, QUuid::createUuid(), {});
    QCOMPARE(service.currentProduct().isNull(), true);
}

void ResultsStorageServiceTest::testEndProductInspectionForDeletedProduct()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QDir resultsDir{dir.filePath(QStringLiteral("results"))};
    QVERIFY(QDir{}.mkpath(resultsDir.path()));
    QVERIFY(resultsDir.exists());
    QFile cacheFile{resultsDir.filePath(QStringLiteral(".results_cache"))};

    ResultsStorageService service;
    service.setMaxCacheEntries(5);
    service.setMaxRelativeDiskUsage(1.0);
    service.setResultsDirectory(resultsDir.path());

    // create a product and seam for inspection
    QScopedPointer<Product> p{new Product{QUuid::createUuid()}};
    QScopedPointer<Product> copy{new Product{p->uuid()}};
    copy->createFirstSeamSeries();
    auto seam = copy->createSeam();

    auto productInstance1 = QUuid::createUuid();

    service.startProductInspection(p.data(), productInstance1, {});
    service.startSeamInspection(seam, productInstance1, 1234);
    service.endSeamInspection();

    // now delete product, which triggered a crash
    QCOMPARE(service.currentProduct().isNull(), false);
    p.reset();
    QCOMPARE(service.currentProduct().isNull(), true);

    service.endProductInspection({});
}

void ResultsStorageServiceTest::testResultAfterSeamEnd()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QDir resultsDir{dir.filePath(QStringLiteral("results"))};
    QVERIFY(QDir{}.mkpath(resultsDir.path()));
    QVERIFY(resultsDir.exists());
    QFile cacheFile{resultsDir.filePath(QStringLiteral(".results_cache"))};

    ResultsStorageService service;
    service.setMaxCacheEntries(5);
    service.setResultsDirectory(resultsDir.path());

    // create a product and seam for inspection
    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto seam = p.createSeam();

    auto productInstance = QUuid::createUuid();

    service.startProductInspection(&p, productInstance, {});
    service.startSeamInspection(seam, productInstance, 1234);

    GeoDoublearray values;
    values.ref().getData().push_back(0.0);
    values.ref().getData().push_back(0.1);
    values.ref().getData().push_back(1.0);
    values.ref().getRank().assign(3, 255);
    ResultDoubleArray result{Poco::UUIDGenerator().createRandom(), precitec::interface::Missmatch, precitec::interface::XCoordOutOfLimits, ImageContext{}, values, precitec::geo2d::TRange<double>(0.0, 2.0), true};
    QCOMPARE(result.isValid(), true);

    service.endSeamInspection();

    service.addResults({result});

    QCOMPARE(cacheFile.exists(), false);
    service.endProductInspection(&p);
    QTRY_VERIFY(cacheFile.exists());

    QDir productDir{resultsDir.filePath(toString(p.uuid()))};
    QTRY_VERIFY(productDir.exists());

    QDir productInstanceDir{productDir.filePath(toString(productInstance) + QStringLiteral("-SN-1234"))};
    QTRY_VERIFY(productInstanceDir.exists());

    QDir seamSeriesDir{productInstanceDir.filePath(QStringLiteral("seam_series0000"))};
    QVERIFY(seamSeriesDir.exists());
    QDir seamDir{seamSeriesDir.filePath(QStringLiteral("seam0000"))};
    QVERIFY(seamDir.exists());
    QVERIFY(!seamDir.exists(QStringLiteral("2.result")));
}

void ResultsStorageServiceTest::testSerialNumberMismatch()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QDir resultsDir{dir.filePath(QStringLiteral("results"))};
    QVERIFY(QDir{}.mkpath(resultsDir.path()));
    QVERIFY(resultsDir.exists());
    QFile cacheFile{resultsDir.filePath(QStringLiteral(".results_cache"))};

    ResultsStorageService service;
    service.setMaxCacheEntries(5);
    service.setMaxRelativeDiskUsage(1.0);
    service.setResultsDirectory(resultsDir.path());

    // create a product and seam for inspection
    QScopedPointer<Product> p{new Product{QUuid::createUuid()}};
    p->createFirstSeamSeries();
    auto seam = p->createSeam();
    auto seam2 = p->createSeam();

    auto productInstance1 = QUuid::createUuid();

    service.startProductInspection(p.data(), productInstance1, {});
    service.startSeamInspection(seam, productInstance1, 1234);
    service.endSeamInspection();

    QCOMPARE(service.currentProduct().isNull(), false);
    service.startSeamInspection(seam2, productInstance1, 1235);
    QCOMPARE(service.currentProduct().isNull(), true);
    service.endSeamInspection();
    service.endProductInspection(p.data());
}

void ResultsStorageServiceTest::testProductInstanceMismatch()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QDir resultsDir{dir.filePath(QStringLiteral("results"))};
    QVERIFY(QDir{}.mkpath(resultsDir.path()));
    QVERIFY(resultsDir.exists());
    QFile cacheFile{resultsDir.filePath(QStringLiteral(".results_cache"))};

    ResultsStorageService service;
    service.setMaxCacheEntries(5);
    service.setMaxRelativeDiskUsage(1.0);
    service.setResultsDirectory(resultsDir.path());

    // create a product and seam for inspection
    QScopedPointer<Product> p{new Product{QUuid::createUuid()}};
    p->createFirstSeamSeries();
    auto seam = p->createSeam();

    auto productInstance1 = QUuid::createUuid();

    service.startProductInspection(p.data(), productInstance1, {});
    QCOMPARE(service.currentProduct().isNull(), false);
    service.startSeamInspection(seam, QUuid::createUuid(), 1234);
    QCOMPARE(service.currentProduct().isNull(), true);
    service.endSeamInspection();
    service.endProductInspection(p.data());
}

void ResultsStorageServiceTest::testProductMetaData()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QDir resultsDir{dir.filePath(QStringLiteral("results"))};
    QVERIFY(QDir{}.mkpath(resultsDir.path()));
    QVERIFY(resultsDir.exists());
    QFile cacheFile{resultsDir.filePath(QStringLiteral(".results_cache"))};

    ResultsStorageService service;
    service.setMaxCacheEntries(5);
    service.setResultsDirectory(resultsDir.path());

    // create a product and seam for inspection
    Product product{QUuid::createUuid()};
    product.setName(QStringLiteral("This is a test product"));
    product.setType(105);
    product.createFirstSeamSeries();
    auto seamSeries = product.seamSeries().front();
    auto seam = product.createSeam();
    auto seam2 = product.createSeam();
    auto seamSeries2 = product.createSeamSeries();
    auto seam3 = seamSeries2->createSeam();
    auto seamLink = seamSeries2->createSeamLink(seam3, QStringLiteral("2"));

    const auto& productInstance = QUuid::createUuid();

    service.startProductInspection(&product, productInstance, QStringLiteral("Serial 123abc\nType fooBar"));
    service.startSeamInspection(seam, productInstance, 1234);

    GeoDoublearray values;
    values.ref().getData().push_back(0.0);
    values.ref().getData().push_back(0.1);
    values.ref().getData().push_back(1.0);
    values.ref().getRank().assign(3, 255);
    ResultDoubleArray result{Poco::UUIDGenerator().createRandom(), precitec::interface::Missmatch, precitec::interface::XCoordOutOfLimits, ImageContext{}, values, precitec::geo2d::TRange<double>(0.0, 2.0), true};
    QCOMPARE(result.isValid(), true);

    service.addNio(result);

    service.endSeamInspection();
    QCOMPARE(cacheFile.exists(), false);

    service.startSeamInspection(seam2, productInstance, 1234);

    service.addResult(result);

    service.endSeamInspection();
    QCOMPARE(cacheFile.exists(), false);

    service.startSeamInspection(seam3, productInstance, 1234);
    service.endSeamInspection();
    QCOMPARE(cacheFile.exists(), false);

    service.startSeamInspection(seamLink, productInstance, 1234);
    service.endSeamInspection();
    QCOMPARE(cacheFile.exists(), false);

    service.endProductInspection(&product);

    QTRY_VERIFY(cacheFile.exists());

    QDir productDir{resultsDir.filePath(toString(product.uuid()))};
    QTRY_VERIFY(productDir.exists());

    QDir productInstanceDir{productDir.filePath(toString(productInstance) + QStringLiteral("-SN-1234"))};
    QTRY_VERIFY(productInstanceDir.exists());

    QTRY_VERIFY(productInstanceDir.exists(QStringLiteral("metadata.json")));

    const auto& metadata = ProductMetaData::parse(productInstanceDir);

    QVERIFY(metadata.productUuid());
    QVERIFY(metadata.productName());
    QVERIFY(metadata.productType());

    QCOMPARE(metadata.productUuid().value(), product.uuid());
    QCOMPARE(metadata.productName().value(), QStringLiteral("This is a test product"));
    QCOMPARE(metadata.productType().value(), 105);

    QVERIFY(metadata.isUuidValid());
    QVERIFY(metadata.isNumberValid());
    QVERIFY(metadata.isNioValid());
    QVERIFY(metadata.isNioSwitchedOffValid());
    QVERIFY(metadata.isDateValid());
    QVERIFY(metadata.isSeamsValid());
    QVERIFY(metadata.isSeamSeriesValid());

    QCOMPARE(metadata.uuid(), productInstance);
    QCOMPARE(metadata.number(), 1234);
    QCOMPARE(metadata.extendedProductInfo(), QStringLiteral("Serial 123abc\nType fooBar"));
    QCOMPARE(metadata.nio(), true);
    QCOMPARE(metadata.nioSwitchedOff(), false);
    QCOMPARE(metadata.date().date(), QDate::currentDate());
    QCOMPARE(metadata.nios().size(), 2);
    QCOMPARE(metadata.nios().at(0).type, precitec::interface::XCoordOutOfLimits);
    QCOMPARE(metadata.nios().at(0).count, 1);
    QCOMPARE(metadata.nios().at(1).type, precitec::interface::NoResultsError);
    QCOMPARE(metadata.nios().at(1).count, 2);

    const auto& processedSeams = metadata.seams();
    QCOMPARE(processedSeams.size(), 4u);

    const auto& processedSeam1 = processedSeams.at(0);
    QVERIFY(processedSeam1.isUuidValid());
    QVERIFY(processedSeam1.isNumberValid());
    QCOMPARE(processedSeam1.number(), 0);
    QVERIFY(!processedSeam1.isSeamSeriesValid());
    QVERIFY(processedSeam1.isSeamSeriesUuidValid());
    QVERIFY(processedSeam1.isNioValid());
    QVERIFY(!processedSeam1.isNioSwitchedOffValid());;
    QVERIFY(!processedSeam1.isLinkToValid());
    QCOMPARE(processedSeam1.uuid(), seam->uuid());
    QCOMPARE(processedSeam1.seamSeriesUuid(), seam->seamSeries()->uuid());
    QCOMPARE(processedSeam1.nio(), true);
    QCOMPARE(processedSeam1.nios().size(), 1);
    QCOMPARE(processedSeam1.nios().at(0).type, precitec::interface::XCoordOutOfLimits);
    QCOMPARE(processedSeam1.nios().at(0).count, 1);

    const auto& processedSeam2 = processedSeams.at(1);
    QVERIFY(processedSeam2.isUuidValid());
    QVERIFY(processedSeam2.isNumberValid());
    QCOMPARE(processedSeam2.number(), 1);
    QVERIFY(!processedSeam2.isSeamSeriesValid());
    QVERIFY(processedSeam2.isSeamSeriesUuidValid());
    QVERIFY(processedSeam2.isNioValid());
    QVERIFY(!processedSeam2.isNioSwitchedOffValid());
    QVERIFY(!processedSeam2.isLinkToValid());
    QCOMPARE(processedSeam2.uuid(), seam2->uuid());
    QCOMPARE(processedSeam2.seamSeriesUuid(), seam2->seamSeries()->uuid());
    QCOMPARE(processedSeam2.nio(), false);
    QCOMPARE(processedSeam2.nios().size(), 0);

    const auto& processedSeam3 = processedSeams.at(2);
    QVERIFY(processedSeam3.isUuidValid());
    QVERIFY(processedSeam3.isNumberValid());
    QCOMPARE(processedSeam3.number(), 0);
    QVERIFY(!processedSeam3.isSeamSeriesValid());
    QVERIFY(processedSeam3.isSeamSeriesUuidValid());
    QVERIFY(processedSeam3.isNioValid());
    QVERIFY(!processedSeam3.isNioSwitchedOffValid());
    QVERIFY(!processedSeam3.isLinkToValid());
    QCOMPARE(processedSeam3.uuid(), seam3->uuid());
    QCOMPARE(processedSeam3.seamSeriesUuid(), seam3->seamSeries()->uuid());
    QCOMPARE(processedSeam3.nio(), true);
    QCOMPARE(processedSeam3.nios().size(), 1);
    QCOMPARE(processedSeam3.nios().at(0).type, precitec::interface::NoResultsError);
    QCOMPARE(processedSeam3.nios().at(0).count, 1);

    const auto& processedSeam4 = processedSeams.at(3);
    QVERIFY(processedSeam4.isUuidValid());
    QVERIFY(processedSeam4.isNumberValid());
    QCOMPARE(processedSeam4.number(), 2);
    QVERIFY(!processedSeam4.isSeamSeriesValid());
    QVERIFY(processedSeam4.isSeamSeriesUuidValid());
    QVERIFY(processedSeam4.isNioValid());
    QVERIFY(!processedSeam4.isNioSwitchedOffValid());
    QVERIFY(processedSeam4.isLinkToValid());
    QCOMPARE(processedSeam4.uuid(), seamLink->uuid());
    QCOMPARE(processedSeam4.seamSeriesUuid(), seamLink->seamSeries()->uuid());
    QCOMPARE(processedSeam4.linkTo(), seam3->uuid());
    QCOMPARE(processedSeam4.nio(), true);
    QCOMPARE(processedSeam4.nios().size(), 1);
    QCOMPARE(processedSeam4.nios().at(0).type, precitec::interface::NoResultsError);
    QCOMPARE(processedSeam4.nios().at(0).count, 1);

    const auto& processedSeries = metadata.seamSeries();
    QCOMPARE(processedSeries.size(), 2u);

    const auto& processedSeries1 = processedSeries.at(0);
    QVERIFY(processedSeries1.isUuidValid());
    QVERIFY(processedSeries1.isNumberValid());
    QCOMPARE(processedSeries1.number(), 0);
    QVERIFY(!processedSeries1.isSeamsValid());
    QVERIFY(processedSeries1.isNioValid());
    QVERIFY(!processedSeries1.isNioSwitchedOffValid());
    QCOMPARE(processedSeries1.uuid(), seamSeries->uuid());
    QCOMPARE(processedSeries1.nio(), true);
    QCOMPARE(processedSeries1.nios().size(), 1);
    QCOMPARE(processedSeries1.nios().at(0).type, precitec::interface::XCoordOutOfLimits);
    QCOMPARE(processedSeries1.nios().at(0).count, 1);

    const auto& processedSeries2 = processedSeries.at(1);
    QVERIFY(processedSeries2.isUuidValid());
    QVERIFY(processedSeries2.isNumberValid());
    QCOMPARE(processedSeries2.number(), 1);
    QVERIFY(!processedSeries2.isSeamsValid());
    QVERIFY(processedSeries2.isNioValid());
    QVERIFY(!processedSeries2.isNioSwitchedOffValid());
    QCOMPARE(processedSeries2.uuid(), seamSeries2->uuid());
    QCOMPARE(processedSeries2.nio(), true);
    QCOMPARE(processedSeries2.nios().size(), 1);
    QCOMPARE(processedSeries2.nios().at(0).type, precitec::interface::NoResultsError);
    QCOMPARE(processedSeries2.nios().at(0).count, 2);
}

void ResultsStorageServiceTest::testSeamSeriesMetaData()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QDir resultsDir{dir.filePath(QStringLiteral("results"))};
    QVERIFY(QDir{}.mkpath(resultsDir.path()));
    QVERIFY(resultsDir.exists());
    QFile cacheFile{resultsDir.filePath(QStringLiteral(".results_cache"))};

    ResultsStorageService service;
    service.setMaxCacheEntries(5);
    service.setResultsDirectory(resultsDir.path());

    // create a product and seam for inspection
    Product product{QUuid::createUuid()};
    product.createFirstSeamSeries();
    auto seamSeries = product.seamSeries().front();
    auto seam = product.createSeam();
    auto seamLink = seamSeries->createSeamLink(seam, QStringLiteral("2"));
    auto seam2 = product.createSeam();

    auto productInstance = QUuid::createUuid();

    service.startProductInspection(&product, productInstance, {});

    service.startSeamInspection(seamLink, productInstance, 1234);
    service.endSeamInspection();

    GeoDoublearray values;
    values.ref().getData().push_back(0.0);
    values.ref().getData().push_back(0.1);
    values.ref().getData().push_back(1.0);
    values.ref().getRank().assign(3, 255);

    ResultDoubleArray result{Poco::UUIDGenerator().createRandom(), precitec::interface::Missmatch, precitec::interface::XCoordOutOfLimits, ImageContext{}, values, precitec::geo2d::TRange<double>(0.0, 2.0), true};
    QCOMPARE(result.isValid(), true);

    ResultDoubleArray result2{Poco::UUIDGenerator().createRandom(), precitec::interface::Missmatch, precitec::interface::YCoordOutOfLimits, ImageContext{}, values, precitec::geo2d::TRange<double>(0.0, 2.0), true};
    QCOMPARE(result2.isValid(), true);

    service.startSeamInspection(seam, productInstance, 1234);

    service.addResult(result);
    service.addNio(result2);
    service.addNio(result2);

    service.endSeamInspection();

    service.startSeamInspection(seam2, productInstance, 1234);

    service.addResult(result);

    service.endSeamInspection();
    QCOMPARE(cacheFile.exists(), false);

    service.endProductInspection(&product);

    QTRY_VERIFY(cacheFile.exists());

    QDir productDir{resultsDir.filePath(toString(product.uuid()))};
    QTRY_VERIFY(productDir.exists());

    QDir productInstanceDir{productDir.filePath(toString(productInstance) + QStringLiteral("-SN-1234"))};
    QTRY_VERIFY(productInstanceDir.exists());

    QDir seamSeriesDir{productInstanceDir.filePath(QStringLiteral("seam_series0000"))};
    QVERIFY(seamSeriesDir.exists());

    QTRY_VERIFY(seamSeriesDir.exists(QStringLiteral("metadata.json")));

    const auto& metadata = SeamSeriesMetaData::parse(seamSeriesDir);

    QVERIFY(metadata.isUuidValid());
    QVERIFY(metadata.isNumberValid());
    QVERIFY(metadata.isNioValid());
    QVERIFY(metadata.isNioSwitchedOffValid());
    QVERIFY(metadata.isSeamsValid());

    QCOMPARE(metadata.uuid(), seamSeries->uuid());
    QCOMPARE(metadata.number(), seamSeries->number());
    QCOMPARE(metadata.nio(), true);
    QCOMPARE(metadata.nioSwitchedOff(), false);

    const auto &nios = metadata.nios();
    QCOMPARE(nios.size(), 2u);
    QCOMPARE(nios.at(0).type, precitec::interface::YCoordOutOfLimits);
    QCOMPARE(nios.at(0).count, 2);
    QCOMPARE(nios.at(1).type, precitec::interface::NoResultsError);
    QCOMPARE(nios.at(1).count, 1);

    const auto &processedSeams = metadata.seams();
    QCOMPARE(processedSeams.size(), 3u);

    const auto& processedSeam1 = processedSeams.at(0);
    QVERIFY(processedSeam1.isUuidValid());
    QVERIFY(processedSeam1.isNumberValid());
    QVERIFY(!processedSeam1.isSeamSeriesValid());
    QVERIFY(!processedSeam1.isSeamSeriesUuidValid());
    QVERIFY(processedSeam1.isNioValid());
    QVERIFY(!processedSeam1.isNioSwitchedOffValid());
    QVERIFY(processedSeam1.isLinkToValid());
    QCOMPARE(processedSeam1.uuid(), seamLink->uuid());
    QCOMPARE(processedSeam1.linkTo(), seam->uuid());
    QCOMPARE(processedSeam1.nio(), true);
    QCOMPARE(processedSeam1.nios().size(), 1);
    QCOMPARE(processedSeam1.nios().at(0).type, precitec::interface::NoResultsError);
    QCOMPARE(processedSeam1.nios().at(0).count, 1);

    const auto& processedSeam2 = processedSeams.at(1);
    QVERIFY(processedSeam2.isUuidValid());
    QVERIFY(processedSeam2.isNumberValid());
    QVERIFY(!processedSeam2.isSeamSeriesValid());
    QVERIFY(!processedSeam2.isSeamSeriesUuidValid());
    QVERIFY(processedSeam2.isNioValid());
    QVERIFY(!processedSeam2.isNioSwitchedOffValid());
    QVERIFY(!processedSeam2.isLinkToValid());
    QCOMPARE(processedSeam2.uuid(), seam->uuid());
    QCOMPARE(processedSeam2.nio(), true);
    QCOMPARE(processedSeam2.nios().size(), 1);
    QCOMPARE(processedSeam2.nios().at(0).type, precitec::interface::YCoordOutOfLimits);
    QCOMPARE(processedSeam2.nios().at(0).count, 2);

    const auto& processedSeam3 = processedSeams.at(2);
    QVERIFY(processedSeam3.isUuidValid());
    QVERIFY(processedSeam3.isNumberValid());
    QVERIFY(!processedSeam3.isSeamSeriesValid());
    QVERIFY(!processedSeam3.isSeamSeriesUuidValid());
    QVERIFY(processedSeam3.isNioValid());
    QVERIFY(!processedSeam3.isNioSwitchedOffValid());
    QVERIFY(!processedSeam3.isLinkToValid());
    QCOMPARE(processedSeam3.uuid(), seam2->uuid());
    QCOMPARE(processedSeam3.nio(), false);
    QCOMPARE(processedSeam3.nios().empty(), true);
}

void ResultsStorageServiceTest::testSeamMetaData()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QDir resultsDir{dir.filePath(QStringLiteral("results"))};
    QVERIFY(QDir{}.mkpath(resultsDir.path()));
    QVERIFY(resultsDir.exists());
    QFile cacheFile{resultsDir.filePath(QStringLiteral(".results_cache"))};

    ResultsStorageService service;
    service.setMaxCacheEntries(5);
    service.setResultsDirectory(resultsDir.path());

    // create a product and seam for inspection
    Product product{QUuid::createUuid()};
    product.createFirstSeamSeries();
    auto seam = product.createSeam();
    auto seam2 = product.createSeam();

    auto productInstance = QUuid::createUuid();

    service.startProductInspection(&product, productInstance, {});
    service.startSeamInspection(seam, productInstance, 1234);

    GeoDoublearray values;
    values.ref().getData().push_back(0.0);
    values.ref().getData().push_back(0.1);
    values.ref().getData().push_back(1.0);
    values.ref().getRank().assign(3, 255);
    ResultDoubleArray result{Poco::UUIDGenerator().createRandom(), precitec::interface::Missmatch, precitec::interface::XCoordOutOfLimits, ImageContext{}, values, precitec::geo2d::TRange<double>(0.0, 2.0), true};
    QCOMPARE(result.isValid(), true);

    service.addNio(result);

    service.endSeamInspection();
    QCOMPARE(cacheFile.exists(), false);

    service.startSeamInspection(seam2, productInstance, 1234);

    service.addResult(result);

    service.endSeamInspection();
    QCOMPARE(cacheFile.exists(), false);

    service.endProductInspection(&product);

    QTRY_VERIFY(cacheFile.exists());

    QDir productDir{resultsDir.filePath(toString(product.uuid()))};
    QTRY_VERIFY(productDir.exists());

    QDir productInstanceDir{productDir.filePath(toString(productInstance) + QStringLiteral("-SN-1234"))};
    QTRY_VERIFY(productInstanceDir.exists());

    QDir seamSeriesDir{productInstanceDir.filePath(QStringLiteral("seam_series0000"))};
    QVERIFY(seamSeriesDir.exists());

    QDir seamDir{seamSeriesDir.filePath(QStringLiteral("seam0000"))};
    QVERIFY(seamDir.exists());
    QVERIFY(seamDir.exists(QStringLiteral("2.result")));

    QTRY_VERIFY(seamDir.exists(QStringLiteral("metadata.json")));

    QFile seamMetaDataFile(seamDir.absoluteFilePath(QStringLiteral("metadata.json")));
    QVERIFY(seamMetaDataFile.open(QIODevice::ReadOnly));

    const auto& seamMetadata = SeamMetaData::parse(seamDir);
    QVERIFY(seamMetadata.isUuidValid());
    QVERIFY(seamMetadata.isNumberValid());
    QVERIFY(seamMetadata.isSeamSeriesValid());
    QVERIFY(seamMetadata.isSeamSeriesUuidValid());
    QVERIFY(seamMetadata.isNioValid());
    QVERIFY(seamMetadata.isNioSwitchedOffValid());
    QVERIFY(!seamMetadata.isLinkToValid());

    QCOMPARE(seamMetadata.uuid(), seam->uuid());
    QCOMPARE(seamMetadata.number(), seam->number());
    QCOMPARE(seamMetadata.seamSeries(), seam->seamSeries()->number());
    QCOMPARE(seamMetadata.seamSeriesUuid(), seam->seamSeries()->uuid());
    QCOMPARE(seamMetadata.nio(), true);
    QCOMPARE(seamMetadata.nioSwitchedOff(), false);
    QCOMPARE(seamMetadata.nios().size(), 1);
    QCOMPARE(seamMetadata.nios().at(0).type, precitec::interface::XCoordOutOfLimits);
    QCOMPARE(seamMetadata.nios().at(0).count, 1);

    QDir seam2Dir{seamSeriesDir.filePath(QStringLiteral("seam0001"))};
    QVERIFY(seam2Dir.exists());
    QVERIFY(seam2Dir.exists(QStringLiteral("2.result")));

    QTRY_VERIFY(seam2Dir.exists(QStringLiteral("metadata.json")));

    const auto& seam2Metadata = SeamMetaData::parse(seam2Dir);
    QVERIFY(seam2Metadata.isUuidValid());
    QVERIFY(seam2Metadata.isNumberValid());
    QVERIFY(seam2Metadata.isSeamSeriesValid());
    QVERIFY(seam2Metadata.isSeamSeriesUuidValid());
    QVERIFY(seam2Metadata.isNioValid());
    QVERIFY(seam2Metadata.isNioSwitchedOffValid());
    QVERIFY(!seam2Metadata.isLinkToValid());

    QCOMPARE(seam2Metadata.uuid(), seam2->uuid());
    QCOMPARE(seam2Metadata.number(), seam2->number());
    QCOMPARE(seam2Metadata.seamSeries(), seam2->seamSeries()->number());
    QCOMPARE(seam2Metadata.seamSeriesUuid(), seam2->seamSeries()->uuid());
    QCOMPARE(seam2Metadata.nio(), false);
    QCOMPARE(seam2Metadata.nioSwitchedOff(), false);
    QCOMPARE(seam2Metadata.nios().size(), 0);
}

void ResultsStorageServiceTest::testSeamLwmWithoutResult()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QDir resultsDir{dir.filePath(QStringLiteral("results"))};
    QVERIFY(QDir{}.mkpath(resultsDir.path()));
    QVERIFY(resultsDir.exists());
    QFile cacheFile{resultsDir.filePath(QStringLiteral(".results_cache"))};

    ResultsStorageService service;
    service.setMaxCacheEntries(5);
    service.setResultsDirectory(resultsDir.path());
    service.setCommunicationToLWMDeviceActive(true);

    // create a product and seam for inspection
    Product product{QUuid::createUuid()};
    product.createFirstSeamSeries();
    auto seam = product.createSeam();
    seam->createHardwareParameters();

    Attribute lwmAttribute{QUuid::createUuid()};
    lwmAttribute.setName(QStringLiteral("LWM_Inspection_Active"));
    lwmAttribute.setType(precitec::storage::Parameter::DataType::Boolean);
    lwmAttribute.setVariantId(QUuid{QByteArrayLiteral("F42DDE6B-C8FF-4CE5-86DE-1A5CB51D633A")});
    seam->hardwareParameters()->createParameter(QUuid::createUuid(), &lwmAttribute, {}, true);
    auto seam2 = product.createSeam();

    auto productInstance = QUuid::createUuid();

    service.startProductInspection(&product, productInstance, {});
    QCOMPARE(service.seamProcessingWithExternalLwm(), false);
    service.startSeamInspection(seam, productInstance, 1234);
    QCOMPARE(service.seamProcessingWithExternalLwm(), true);
    QCOMPARE(service.processingState(), ResultsStorageService::ProcessingState::SeamInspection);

    // not sending in any result
    service.endSeamInspection();
    QCOMPARE(service.processingState(), ResultsStorageService::ProcessingState::WaitingForLwmResult);

    // start next seam
    service.startSeamInspection(seam2, productInstance, 1234);
    QCOMPARE(service.processingState(), ResultsStorageService::ProcessingState::SeamInspection);
    QCOMPARE(service.seamProcessingWithExternalLwm(), false);
    service.endSeamInspection();
    QCOMPARE(service.processingState(), ResultsStorageService::ProcessingState::ProductInspection);

    service.endProductInspection(&product);
    QCOMPARE(service.processingState(), ResultsStorageService::ProcessingState::Idle);

    QTRY_VERIFY(cacheFile.exists());

    QDir productDir{resultsDir.filePath(toString(product.uuid()))};
    QTRY_VERIFY(productDir.exists());

    QDir productInstanceDir{productDir.filePath(toString(productInstance) + QStringLiteral("-SN-1234"))};
    QTRY_VERIFY(productInstanceDir.exists());

    QDir seamSeriesDir{productInstanceDir.filePath(QStringLiteral("seam_series0000"))};
    QVERIFY(seamSeriesDir.exists());

    QDir seamDir{seamSeriesDir.filePath(QStringLiteral("seam0000"))};
    QVERIFY(seamDir.exists());

    QDir seam2Dir{seamSeriesDir.filePath(QStringLiteral("seam0001"))};
    QVERIFY(seam2Dir.exists());
}

void ResultsStorageServiceTest::testSeamLwmWithResultAfterSeamEnd()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QDir resultsDir{dir.filePath(QStringLiteral("results"))};
    QVERIFY(QDir{}.mkpath(resultsDir.path()));
    QVERIFY(resultsDir.exists());
    QFile cacheFile{resultsDir.filePath(QStringLiteral(".results_cache"))};

    ResultsStorageService service;
    service.setMaxCacheEntries(5);
    service.setResultsDirectory(resultsDir.path());
    service.setCommunicationToLWMDeviceActive(true);

    // create a product and seam for inspection
    Product product{QUuid::createUuid()};
    product.createFirstSeamSeries();
    auto seam = product.createSeam();
    seam->createHardwareParameters();

    Attribute lwmAttribute{QUuid::createUuid()};
    lwmAttribute.setName(QStringLiteral("LWM_Inspection_Active"));
    lwmAttribute.setType(precitec::storage::Parameter::DataType::Boolean);
    lwmAttribute.setVariantId(QUuid{QByteArrayLiteral("F42DDE6B-C8FF-4CE5-86DE-1A5CB51D633A")});
    seam->hardwareParameters()->createParameter(QUuid::createUuid(), &lwmAttribute, {}, true);
    auto seam2 = product.createSeam();

    auto productInstance = QUuid::createUuid();

    service.startProductInspection(&product, productInstance, {});
    QCOMPARE(service.seamProcessingWithExternalLwm(), false);
    service.startSeamInspection(seam, productInstance, 1234);
    QCOMPARE(service.seamProcessingWithExternalLwm(), true);
    QCOMPARE(service.processingState(), ResultsStorageService::ProcessingState::SeamInspection);

    // not sending in any result
    service.endSeamInspection();
    QCOMPARE(service.processingState(), ResultsStorageService::ProcessingState::WaitingForLwmResult);
    QCOMPARE(service.currentSeam().data(), seam);
    QCOMPARE(service.seamProcessingWithExternalLwm(), true);

    GeoDoublearray values;
    values.ref().getData().push_back(0.0);
    values.ref().getData().push_back(0.1);
    values.ref().getData().push_back(1.0);
    values.ref().getRank().assign(3, 255);
    ResultDoubleArray result{Poco::UUIDGenerator().createRandom(), precitec::interface::LWMStandardResult, precitec::interface::LWMStandardResult, ImageContext{}, values, precitec::geo2d::TRange<double>(0.0, 2.0), false};
    QCOMPARE(result.isValid(), true);
    service.addNio(result);

    QCOMPARE(service.processingState(), ResultsStorageService::ProcessingState::ProductInspection);
    QVERIFY(service.currentSeam().isNull());
    QCOMPARE(service.seamProcessingWithExternalLwm(), false);

    // start next seam
    service.startSeamInspection(seam2, productInstance, 1234);
    QCOMPARE(service.processingState(), ResultsStorageService::ProcessingState::SeamInspection);
    QCOMPARE(service.seamProcessingWithExternalLwm(), false);
    service.endSeamInspection();
    QCOMPARE(service.processingState(), ResultsStorageService::ProcessingState::ProductInspection);

    service.endProductInspection(&product);
    QCOMPARE(service.processingState(), ResultsStorageService::ProcessingState::Idle);

    QTRY_VERIFY(cacheFile.exists());

    QDir productDir{resultsDir.filePath(toString(product.uuid()))};
    QTRY_VERIFY(productDir.exists());

    QDir productInstanceDir{productDir.filePath(toString(productInstance) + QStringLiteral("-SN-1234"))};
    QTRY_VERIFY(productInstanceDir.exists());

    QDir seamSeriesDir{productInstanceDir.filePath(QStringLiteral("seam_series0000"))};
    QVERIFY(seamSeriesDir.exists());

    QDir seamDir{seamSeriesDir.filePath(QStringLiteral("seam0000"))};
    QVERIFY(seamDir.exists());
    QVERIFY(seamDir.exists(QString::number(int(precitec::interface::LWMStandardResult)) + QStringLiteral(".result")));

    QDir seam2Dir{seamSeriesDir.filePath(QStringLiteral("seam0001"))};
    QVERIFY(seam2Dir.exists());
}

void ResultsStorageServiceTest::testSeamAndProductLwmWithoutResult()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QDir resultsDir{dir.filePath(QStringLiteral("results"))};
    QVERIFY(QDir{}.mkpath(resultsDir.path()));
    QVERIFY(resultsDir.exists());
    QFile cacheFile{resultsDir.filePath(QStringLiteral(".results_cache"))};

    ResultsStorageService service;
    service.setMaxCacheEntries(5);
    service.setResultsDirectory(resultsDir.path());
    service.setCommunicationToLWMDeviceActive(true);

    // create a product and seam for inspection
    Product product{QUuid::createUuid()};
    product.createFirstSeamSeries();
    auto seam = product.createSeam();
    seam->createHardwareParameters();

    Attribute lwmAttribute{QUuid::createUuid()};
    lwmAttribute.setName(QStringLiteral("LWM_Inspection_Active"));
    lwmAttribute.setType(precitec::storage::Parameter::DataType::Boolean);
    lwmAttribute.setVariantId(QUuid{QByteArrayLiteral("F42DDE6B-C8FF-4CE5-86DE-1A5CB51D633A")});
    seam->hardwareParameters()->createParameter(QUuid::createUuid(), &lwmAttribute, {}, true);

    auto productInstance = QUuid::createUuid();

    service.startProductInspection(&product, productInstance, {});
    QCOMPARE(service.seamProcessingWithExternalLwm(), false);
    service.startSeamInspection(seam, productInstance, 1234);
    QCOMPARE(service.seamProcessingWithExternalLwm(), true);
    QCOMPARE(service.processingState(), ResultsStorageService::ProcessingState::SeamInspection);

    // not sending in any result
    service.endSeamInspection();
    QCOMPARE(service.processingState(), ResultsStorageService::ProcessingState::WaitingForLwmResult);
    QCOMPARE(service.seamProcessingWithExternalLwm(), true);

    service.endProductInspection(&product);
    QCOMPARE(service.processingState(), ResultsStorageService::ProcessingState::WaitingForLwmResultAtEndOfProduct);
    QCOMPARE(service.currentProduct().data(), &product);
    QCOMPARE(service.seamProcessingWithExternalLwm(), true);

    auto productInstance2 = QUuid::createUuid();
    service.startProductInspection(&product, productInstance2, {});
    QCOMPARE(service.processingState(), ResultsStorageService::ProcessingState::ProductInspection);
    QCOMPARE(service.seamProcessingWithExternalLwm(), false);

    QTRY_VERIFY(cacheFile.exists());

    QDir productDir{resultsDir.filePath(toString(product.uuid()))};
    QTRY_VERIFY(productDir.exists());

    QDir productInstanceDir{productDir.filePath(toString(productInstance) + QStringLiteral("-SN-1234"))};
    QTRY_VERIFY(productInstanceDir.exists());

    QDir seamSeriesDir{productInstanceDir.filePath(QStringLiteral("seam_series0000"))};
    QVERIFY(seamSeriesDir.exists());

    QDir seamDir{seamSeriesDir.filePath(QStringLiteral("seam0000"))};
    QVERIFY(seamDir.exists());

    service.endProductInspection(&product);
}

void ResultsStorageServiceTest::testLwmWithResultAfterProductEnd()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QDir resultsDir{dir.filePath(QStringLiteral("results"))};
    QVERIFY(QDir{}.mkpath(resultsDir.path()));
    QVERIFY(resultsDir.exists());
    QFile cacheFile{resultsDir.filePath(QStringLiteral(".results_cache"))};

    ResultsStorageService service;
    service.setMaxCacheEntries(5);
    service.setResultsDirectory(resultsDir.path());
    service.setCommunicationToLWMDeviceActive(true);

    // create a product and seam for inspection
    Product product{QUuid::createUuid()};
    product.createFirstSeamSeries();
    auto seam = product.createSeam();
    seam->createHardwareParameters();

    Attribute lwmAttribute{QUuid::createUuid()};
    lwmAttribute.setName(QStringLiteral("LWM_Inspection_Active"));
    lwmAttribute.setType(precitec::storage::Parameter::DataType::Boolean);
    lwmAttribute.setVariantId(QUuid{QByteArrayLiteral("F42DDE6B-C8FF-4CE5-86DE-1A5CB51D633A")});
    seam->hardwareParameters()->createParameter(QUuid::createUuid(), &lwmAttribute, {}, true);

    auto productInstance = QUuid::createUuid();

    service.startProductInspection(&product, productInstance, {});
    QCOMPARE(service.seamProcessingWithExternalLwm(), false);
    service.startSeamInspection(seam, productInstance, 1234);
    QCOMPARE(service.seamProcessingWithExternalLwm(), true);
    QCOMPARE(service.processingState(), ResultsStorageService::ProcessingState::SeamInspection);

    // not sending in any result
    service.endSeamInspection();
    QCOMPARE(service.processingState(), ResultsStorageService::ProcessingState::WaitingForLwmResult);
    QCOMPARE(service.currentSeam().data(), seam);
    QCOMPARE(service.seamProcessingWithExternalLwm(), true);

    service.endProductInspection(&product);
    QCOMPARE(service.processingState(), ResultsStorageService::ProcessingState::WaitingForLwmResultAtEndOfProduct);
    QCOMPARE(service.seamProcessingWithExternalLwm(), true);
    QCOMPARE(service.currentSeam().data(), seam);
    QCOMPARE(service.currentProduct().data(), &product);

    GeoDoublearray values;
    values.ref().getData().push_back(0.0);
    values.ref().getData().push_back(0.1);
    values.ref().getData().push_back(1.0);
    values.ref().getRank().assign(3, 255);
    ResultDoubleArray result{Poco::UUIDGenerator().createRandom(), precitec::interface::LWMStandardResult, precitec::interface::LWMStandardResult, ImageContext{}, values, precitec::geo2d::TRange<double>(0.0, 2.0), false};
    QCOMPARE(result.isValid(), true);
    service.addNio(result);

    QCOMPARE(service.processingState(), ResultsStorageService::ProcessingState::Idle);
    QVERIFY(service.currentSeam().isNull());
    QVERIFY(service.currentProduct().isNull());
    QCOMPARE(service.seamProcessingWithExternalLwm(), false);

    QTRY_VERIFY(cacheFile.exists());

    QDir productDir{resultsDir.filePath(toString(product.uuid()))};
    QTRY_VERIFY(productDir.exists());

    QDir productInstanceDir{productDir.filePath(toString(productInstance) + QStringLiteral("-SN-1234"))};
    QTRY_VERIFY(productInstanceDir.exists());

    QDir seamSeriesDir{productInstanceDir.filePath(QStringLiteral("seam_series0000"))};
    QVERIFY(seamSeriesDir.exists());

    QDir seamDir{seamSeriesDir.filePath(QStringLiteral("seam0000"))};
    QVERIFY(seamDir.exists());
    QVERIFY(seamDir.exists(QString::number(int(precitec::interface::LWMStandardResult)) + QStringLiteral(".result")));
}

QTEST_GUILESS_MAIN(ResultsStorageServiceTest)
#include "testResultsStorageService.moc"
