#include <QTest>
#include <QSignalSpy>

#include "../src/compatibility.h"
#include "../src/storageUpdateServer.h"
#include "parameter.h"
#include "parameterSet.h"
#include "product.h"
#include "productModel.h"
#include "seam.h"
#include "seamInterval.h"
#include "seamSeries.h"

#include "event/dbNotification.interface.h"

#include <QtConcurrentRun>

using precitec::storage::StorageUpdateServer;
using precitec::storage::ProductModel;

class MockDbNotification : public QObject, public precitec::interface::TDbNotification<precitec::interface::AbstractInterface>
{
    Q_OBJECT
public:
    MockDbNotification(QObject *parent = nullptr) : QObject(parent) {}
    ~MockDbNotification() override {}

    void setupProduct(const Poco::UUID& productID) override
    {
        emit setupProductReceived(precitec::storage::compatibility::toQt(productID));
    }
    void setupMeasureTask(const Poco::UUID& measureTaskID) override
    {
        Q_UNUSED(measureTaskID);
    }
    void setupFilterParameter(const Poco::UUID& measureTaskID, const Poco::UUID& filterID) override
    {
        emit setupFilterParameterReceived(precitec::storage::compatibility::toQt(measureTaskID), precitec::storage::compatibility::toQt(filterID));
    }
    void setupHardwareParameter(const Poco::UUID& hwParameterSatzID, const precitec::interface::Key key) override
    {
        Q_UNUSED(hwParameterSatzID)
        Q_UNUSED(key)
    }
    void resetCalibration(const int sensorId) override
    {
        Q_UNUSED(sensorId)
    }

Q_SIGNALS:
    void setupFilterParameterReceived(const QUuid &measureTaskId, const QUuid &filterId);
    void setupProductReceived(const QUuid &productId);
};

class TestStorageUpdateServer : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testUpdateWithoutProducts();
    void testUpdate_data();
    void testUpdate();
    void testMeasureTaskNotFound();
    void testParameterNotFound();
    void testReloadProduct();
};

void TestStorageUpdateServer::testCtor()
{
    StorageUpdateServer server;
    QVERIFY(!server.dbNotificationProxy());
    QVERIFY(!server.productModel());
}

void TestStorageUpdateServer::testUpdateWithoutProducts()
{
    StorageUpdateServer server;
    auto mockDbNotifications = std::make_shared<MockDbNotification>();
    server.setDbNotificationProxy(mockDbNotifications);
    QCOMPARE(server.dbNotificationProxy(), mockDbNotifications);
    QSignalSpy setupFilterParameterReceivedSpy{mockDbNotifications.get(), &MockDbNotification::setupFilterParameterReceived};
    QVERIFY(setupFilterParameterReceivedSpy.isValid());
    QSignalSpy setupProductReceivedSpy{mockDbNotifications.get(), &MockDbNotification::setupProductReceived};
    QVERIFY(setupProductReceivedSpy.isValid());
    server.filterParameterUpdated(Poco::UUID{}, {});
    QVERIFY(setupFilterParameterReceivedSpy.isEmpty());
    QVERIFY(setupProductReceivedSpy.isEmpty());
}

void TestStorageUpdateServer::testUpdate_data()
{
    QTest::addColumn<QUuid>("parameterId");
    QTest::addColumn<QUuid>("filterParameterId");
    QTest::addColumn<QVariant>("value");

    QTest::newRow("bool") << QUuid{"7F086211-FBD4-4493-A580-6FF11E4925DE"} << QUuid{"4F086211-FBD4-4493-A580-6FF11E4925DE"} << QVariant{false};
    QTest::newRow("uint") << QUuid{"9F086211-FBD4-4493-A580-6FF11E4925DE"} << QUuid{"4F086211-FBD4-4493-A580-6FF11E4925DE"} << QVariant{4u};
    QTest::newRow("float") << QUuid{"9F086211-FBD4-4493-A580-6FF11E4925DF"} << QUuid{"4F086211-FBD4-4493-A580-6FF11E4925DF"} << QVariant{4.2f};
    QTest::newRow("double") << QUuid{"9F086211-FBD4-4493-A580-6FF11E4925FF"} << QUuid{"4F086211-FBD4-4493-A580-6FF11E4925EF"} << QVariant{4.56};
}

void TestStorageUpdateServer::testUpdate()
{
    auto model = std::make_shared<ProductModel>();
    QDir dir(QFINDTESTDATA("testdata/products/"));
    QVERIFY(dir.exists());
    model->loadProducts(dir);

    StorageUpdateServer server;
    server.setProductModel(model);
    QCOMPARE(server.productModel(), model);
    auto mockDbNotifications = std::make_shared<MockDbNotification>();
    server.setDbNotificationProxy(mockDbNotifications);
    QSignalSpy setupFilterParameterReceivedSpy{mockDbNotifications.get(), &MockDbNotification::setupFilterParameterReceived};
    QVERIFY(setupFilterParameterReceivedSpy.isValid());
    QSignalSpy setupProductReceivedSpy{mockDbNotifications.get(), &MockDbNotification::setupProductReceived};
    QVERIFY(setupProductReceivedSpy.isValid());

    QFETCH(QUuid, parameterId);
    QFETCH(QVariant, value);

    auto product = model->findProduct({"2F086211-FBD4-4493-A580-6FF11E4925DE"});
    QVERIFY(product);
    auto parameterSet = product->filterParameterSet(QUuid{"6F086211-FBD4-4493-A580-6FF11E4925DD"});
    QVERIFY(parameterSet);
    auto parameters = parameterSet->parameters();
    auto it = std::find_if(parameters.begin(), parameters.end(), [parameterId] (auto parameter) { return parameter->uuid() == parameterId; });
    QVERIFY(it != parameters.end());
    (*it)->setValue(value);

    auto interval = product->seamSeries().front()->seams().front()->seamIntervals().front();
    QVERIFY(interval);
    server.filterParameterUpdated(precitec::storage::compatibility::toPoco(interval->uuid()), {(*it)->toFilterParameter()});
    QCOMPARE(setupFilterParameterReceivedSpy.count(), 1);
    QCOMPARE(setupFilterParameterReceivedSpy.first().first().toUuid(), interval->uuid());
    QTEST(setupFilterParameterReceivedSpy.first().last().toUuid(), "filterParameterId");
    QCOMPARE(setupProductReceivedSpy.count(), 1);
    QCOMPARE(setupProductReceivedSpy.first().first().toUuid(), product->uuid());
}

void TestStorageUpdateServer::testMeasureTaskNotFound()
{
    auto model = std::make_shared<ProductModel>();
    QDir dir(QFINDTESTDATA("testdata/products/"));
    QVERIFY(dir.exists());
    model->loadProducts(dir);

    StorageUpdateServer server;
    server.setProductModel(model);
    QCOMPARE(server.productModel(), model);
    auto mockDbNotifications = std::make_shared<MockDbNotification>();
    server.setDbNotificationProxy(mockDbNotifications);
    QSignalSpy setupFilterParameterReceivedSpy{mockDbNotifications.get(), &MockDbNotification::setupFilterParameterReceived};
    QVERIFY(setupFilterParameterReceivedSpy.isValid());
    QSignalSpy setupProductReceivedSpy{mockDbNotifications.get(), &MockDbNotification::setupProductReceived};
    QVERIFY(setupProductReceivedSpy.isValid());

    // update a parameter
    auto product = model->findProduct({"2F086211-FBD4-4493-A580-6FF11E4925DE"});
    QVERIFY(product);
    auto parameterSet = product->filterParameterSet(QUuid{"6F086211-FBD4-4493-A580-6FF11E4925DD"});
    QVERIFY(parameterSet);
    auto parameters = parameterSet->parameters();
    auto parameter = parameterSet->parameters().front();
    QVERIFY(parameter);
    parameter->setValue(true);

    // update with not existing uuid
    server.filterParameterUpdated(Poco::UUIDGenerator{}.createRandom(), {parameter->toFilterParameter()});
    QVERIFY(setupFilterParameterReceivedSpy.isEmpty());
    QVERIFY(setupProductReceivedSpy.isEmpty());
}

QString uuidToString(const QUuid &id)
{
    return id.toString(QUuid::WithoutBraces);
}

void TestStorageUpdateServer::testParameterNotFound()
{
    auto model = std::make_shared<ProductModel>();
    QDir dir(QFINDTESTDATA("testdata/products/"));
    QVERIFY(dir.exists());
    model->loadProducts(dir);

    StorageUpdateServer server;
    server.setProductModel(model);
    QCOMPARE(server.productModel(), model);
    auto mockDbNotifications = std::make_shared<MockDbNotification>();
    server.setDbNotificationProxy(mockDbNotifications);
    QSignalSpy setupFilterParameterReceivedSpy{mockDbNotifications.get(), &MockDbNotification::setupFilterParameterReceived};
    QVERIFY(setupFilterParameterReceivedSpy.isValid());
    QSignalSpy setupProductReceivedSpy{mockDbNotifications.get(), &MockDbNotification::setupProductReceived};
    QVERIFY(setupProductReceivedSpy.isValid());

    // update a parameter
    auto product = model->findProduct({"2F086211-FBD4-4493-A580-6FF11E4925DE"});
    QVERIFY(product);
    auto parameterSet = product->filterParameterSet(QUuid{"6F086211-FBD4-4493-A580-6FF11E4925DD"});
    QVERIFY(parameterSet);

    auto parameters = parameterSet->parameters();
    auto parameter = parameters.front();
    QVERIFY(parameter);
    auto json = parameter->toJson();
    json.insert(QStringLiteral("uuid"), uuidToString(QUuid::createUuid()));

    auto interval = product->seamSeries().front()->seams().front()->seamIntervals().front();
    QVERIFY(interval);
    server.filterParameterUpdated(precitec::storage::compatibility::toPoco(interval->uuid()), {precitec::storage::Parameter::fromJson(json, parameterSet)->toFilterParameter()});
    QVERIFY(setupFilterParameterReceivedSpy.isEmpty());
    QCOMPARE(setupProductReceivedSpy.count(), 1);
    QCOMPARE(setupProductReceivedSpy.first().first().toUuid(), product->uuid());
}

void TestStorageUpdateServer::testReloadProduct()
{
    auto model = std::make_shared<ProductModel>();
    QDir dir(QFINDTESTDATA("testdata/products/"));
    QVERIFY(dir.exists());
    model->loadProducts(dir);

    QSignalSpy dataChangedSpy{model.get(), &QAbstractItemModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    StorageUpdateServer server;
    // without product model
    QtConcurrent::run(&server, &StorageUpdateServer::reloadProduct, Poco::UUID("2F086211-FBD4-4493-A580-6FF11E4925DE"));
    QVERIFY(!dataChangedSpy.wait(200));

    server.setProductModel(model);
    QtConcurrent::run(&server, &StorageUpdateServer::reloadProduct, Poco::UUID("2F086211-FBD4-4493-A580-6FF11E4925DE"));
    QVERIFY(dataChangedSpy.wait());
    QCOMPARE(dataChangedSpy.count(), 1);
}

QTEST_GUILESS_MAIN(TestStorageUpdateServer)
#include "testStorageUpdateServer.moc"
