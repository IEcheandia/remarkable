#include <QTest>
#include <QSignalSpy>

#include "../src/detectionController.h"
#include "../src/deviceProxyWrapper.h"
#include "../src/systemStatusServer.h"

#include "attribute.h"
#include "product.h"
#include "seam.h"
#include "seamSeries.h"
#include "subGraphModel.h"

using precitec::gui::DetectionController;
using precitec::gui::DeviceProxyWrapper;
using precitec::gui::SystemStatusServer;
using precitec::storage::Attribute;
using precitec::storage::Product;
using precitec::storage::Seam;
using precitec::storage::SeamSeries;
using precitec::storage::SubGraphModel;

class DetectionControllerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testGetFilterParameter();
    void testHandleGraphChange();
    void testSaveChanges();
    void testUpdateFilterParameter();
    void testSetSystemStatus();
    void testCurrentGraphIdWithSubGraphs();
    void testGrabberDeviceProxy();

private:
    QTemporaryDir m_dir;
};

void DetectionControllerTest::initTestCase()
{
    QVERIFY(m_dir.isValid());
    QVERIFY(QDir{}.mkpath(m_dir.filePath(QStringLiteral("config"))));
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());
}

void DetectionControllerTest::testCtor()
{
    DetectionController controller;
    QVERIFY(controller.currentSeam() == nullptr);
    QCOMPARE(controller.liveMode(), false);
    QVERIFY(!controller.inspectionCmdProxy());
    QCOMPARE(controller.isUpdating(), false);
    QVERIFY(!controller.productModel());
    QVERIFY(!controller.systemStatus());
    QVERIFY(!controller.subGraphModel());
    QVERIFY(controller.currentGraphId().isNull());
    QVERIFY(!controller.grabberDeviceProxy());
}

void DetectionControllerTest::testGetFilterParameter()
{
    DetectionController controller;
    // no seam interval, thus null
    QVERIFY(!controller.getFilterParameter({}, nullptr, {}));

    // create product with seam interval
    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto seam = p.createSeam();
    QVERIFY(seam);

    QSignalSpy seamChangedSpy(&controller, &DetectionController::currentSeamChanged);
    QVERIFY(seamChangedSpy.isValid());
    controller.setCurrentSeam(seam);
    QCOMPARE(controller.currentSeam(), seam);
    QCOMPARE(seamChangedSpy.count(), 1);
    // setting to same should not change
    controller.setCurrentSeam(seam);
    QCOMPARE(seamChangedSpy.count(), 1);

    // without an attribute, null is returned
    const auto paramId = QUuid::createUuid();
    QVERIFY(!controller.getFilterParameter(paramId, nullptr, {}));

    // create the attribute
    Attribute a{QUuid::createUuid()};
    a.setDefaultValue(5);

    auto parameter = controller.getFilterParameter(paramId, &a, {});
    QVERIFY(parameter);
    QCOMPARE(parameter->value(), QVariant(5));

    // getting same filter parameter
    QCOMPARE(controller.getFilterParameter(paramId, nullptr, {}), parameter);

    // create different parameter from same attribute, but dedicated default value
    auto parameter2 = controller.getFilterParameter(QUuid::createUuid(), &a, {}, 3);
    QVERIFY(parameter2);
    QCOMPARE(parameter2->value(), QVariant(3));

    // delete seam interval
    seam->deleteLater();
    QVERIFY(seamChangedSpy.wait());
    QVERIFY(controller.currentSeam() == nullptr);
}

void DetectionControllerTest::testHandleGraphChange()
{
    DetectionController controller;
    // create interval
    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto seam = p.createSeam();
    QVERIFY(seam);

    controller.setCurrentSeam(seam);

    QSignalSpy hasChangesSpy(&controller, &DetectionController::markAsChanged);
    QVERIFY(hasChangesSpy.isValid());
    const auto graphParamId = seam->graphParamSet();
    seam->setGraph(QUuid::createUuid());
    QVERIFY(seam->graphParamSet() != graphParamId);
    QCOMPARE(hasChangesSpy.size(), 1);
    QCOMPARE(controller.currentGraphId(), seam->graph());
}

void DetectionControllerTest::testSaveChanges()
{
    DetectionController controller;
    QSignalSpy hasChangesSpy(&controller, &DetectionController::markAsChanged);
    QVERIFY(hasChangesSpy.isValid());

    // create interval
    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto seam = p.createSeam();
    QVERIFY(seam);

    controller.setCurrentSeam(seam);
    // no changes
    QCOMPARE(hasChangesSpy.count(), 0);

    // create changes
    seam->setGraph(QUuid::createUuid());
    QCOMPARE(hasChangesSpy.count(), 1);
}

void DetectionControllerTest::testUpdateFilterParameter()
{
    DetectionController controller;
    QSignalSpy hasChangesSpy(&controller, &DetectionController::markAsChanged);
    QVERIFY(hasChangesSpy.isValid());

    // no seam interval
    controller.updateFilterParameter({}, {});
    QVERIFY(hasChangesSpy.isEmpty());

    // create the interval
    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto seam = p.createSeam();
    QVERIFY(seam);

    controller.setCurrentSeam(seam);

    // parameter does not exist
    const auto paramId = QUuid::createUuid();
    controller.updateFilterParameter(paramId, {});
    QVERIFY(hasChangesSpy.isEmpty());

    // now create filter parameter
    Attribute a{QUuid::createUuid()};
    controller.getFilterParameter(paramId, &a, {});

    // now update should work
    controller.updateFilterParameter(paramId, {});
    QCOMPARE(hasChangesSpy.size(), 1);
}

void DetectionControllerTest::testSetSystemStatus()
{
    DetectionController controller;
    QVERIFY(controller.systemStatus() == nullptr);

    QSignalSpy systemStatusChangedSpy{&controller, &DetectionController::systemStatusChanged};
    QVERIFY(systemStatusChangedSpy.isValid());

    auto systemStatusServer = std::make_unique<SystemStatusServer>();
    controller.setSystemStatus(systemStatusServer.get());
    QCOMPARE(controller.systemStatus(), systemStatusServer.get());
    QCOMPARE(systemStatusChangedSpy.count(), 1);

    // setting same should not change
    controller.setSystemStatus(systemStatusServer.get());
    QCOMPARE(systemStatusChangedSpy.count(), 1);

    // deleting the server should change
    systemStatusServer.reset();
    QCOMPARE(systemStatusChangedSpy.count(), 2);
    QVERIFY(controller.systemStatus() == nullptr);
}

void DetectionControllerTest::testCurrentGraphIdWithSubGraphs()
{
    DetectionController controller;// create interval
    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto seam = p.createSeam();
    QVERIFY(seam);

    seam->setSubGraphs({QUuid::createUuid(), QUuid::createUuid()});
    controller.setCurrentSeam(seam);

    // no sub graph model yet
    QCOMPARE(controller.currentGraphId().isNull(), true);

    SubGraphModel model;
    controller.setSubGraphModel(&model);
    QCOMPARE(controller.currentGraphId().isNull(), false);
    QCOMPARE(controller.currentGraphId(), model.generateGraphId(seam->subGraphs()));
}

void DetectionControllerTest::testGrabberDeviceProxy()
{
    DetectionController controller;
    QVERIFY(!controller.grabberDeviceProxy());
    std::shared_ptr<precitec::interface::TDevice<precitec::interface::AbstractInterface>> proxy;
    auto *device = new DeviceProxyWrapper{proxy, precitec::gui::Permission::ViewGrabberDeviceConfig, precitec::gui::Permission::EditGrabberDeviceConfig, {}, this};

    QSignalSpy deviceProxyWrapperChangedSpy{&controller, &DetectionController::grabberDeviceProxyChanged};
    QVERIFY(deviceProxyWrapperChangedSpy.isValid());

    controller.setGrabberDeviceProxy(device);
    QCOMPARE(controller.grabberDeviceProxy(), device);
    QCOMPARE(deviceProxyWrapperChangedSpy.count(), 1);
    // setting same should not change
    controller.setGrabberDeviceProxy(device);
    QCOMPARE(controller.grabberDeviceProxy(), device);
    QCOMPARE(deviceProxyWrapperChangedSpy.count(), 1);

    // let's delete the device
    device->deleteLater();
    QVERIFY(deviceProxyWrapperChangedSpy.wait());
    QVERIFY(!controller.grabberDeviceProxy());
    QCOMPARE(deviceProxyWrapperChangedSpy.count(), 2);
}

QTEST_GUILESS_MAIN(DetectionControllerTest)
#include "detectionControllerTest.moc"
