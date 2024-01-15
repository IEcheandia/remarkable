#include <QTest>
#include <QSignalSpy>

#include "../src/scanfieldSeamController.h"
#include "../src/scanfieldModule.h"
#include "../src/hardwareParametersModule.h"
#include "product.h"
#include "seamSeries.h"
#include "seam.h"
#include "attributeModel.h"
#include "parameterSet.h"
#include "parameter.h"
#include "message/calibrationCoordinatesRequest.interface.h"

using precitec::gui::ScanfieldSeamController;
using precitec::storage::Product;
using precitec::storage::SeamSeries;
using precitec::storage::AttributeModel;
using precitec::geo2d::DPoint;

namespace precitec
{
namespace interface
{

class MockCalibrationProxy : public TCalibrationCoordinatesRequest<AbstractInterface>
{
public:
    MockCalibrationProxy() {};
    Vec3D get3DCoordinates(ScreenCooordinate_t pX, ScreenCooordinate_t pY, SensorId p_oSensorID, LaserLine p_LaserLine)
    {
        Q_UNUSED(pX)
        Q_UNUSED(pY)
        Q_UNUSED(p_oSensorID)
        Q_UNUSED(p_LaserLine)
        return {};
    }
    DPoint getCoordinatesFromGrayScaleImage(ScreenCooordinate_t pX, ScreenCooordinate_t pY, ScannerContextInfo scannerContext, SensorId p_oSensorID)
    {
        Q_UNUSED(pX)
        Q_UNUSED(pY)
        Q_UNUSED(scannerContext)
        Q_UNUSED(p_oSensorID)
        return {};
    }
    DPoint getTCPPosition(SensorId p_oSensorId, ScannerContextInfo scannerContext)
    {
        Q_UNUSED(p_oSensorId)
        Q_UNUSED(scannerContext)
        return {};
    }
    bool availableOCTCoordinates(OCT_Mode mode)
    {
        Q_UNUSED(mode)
        return false;
    }
    DPoint getNewsonPosition(double xScreen, double yScreen, OCT_Mode mode)
    {
        Q_UNUSED(xScreen)
        Q_UNUSED(yScreen)
        Q_UNUSED(mode)
        return {};
    }
    DPoint getScreenPositionFromNewson(double xNewson, double yNewson, OCT_Mode mode)
    {
        Q_UNUSED(xNewson)
        Q_UNUSED(yNewson)
        Q_UNUSED(mode)
        return {};
    }
    DPoint getScannerPositionFromScanFieldImage(double x_pixel, double y_pixel, Configuration scanfieldImageConfiguration)
    {
        Q_UNUSED(scanfieldImageConfiguration)
        return DPoint{10.0 * x_pixel, 10.0 * y_pixel};
    }
    DPoint getScanFieldImagePositionFromScannerPosition(double x_mm, double y_mm, Configuration scanfieldImageConfiguration)
    {
        Q_UNUSED(scanfieldImageConfiguration)
        return DPoint{0.1 * x_mm, 0.1 * y_mm};
    }
};

}
}

class ScanfieldSeamControllerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testSeam();
    void testTransformation();
    void testCameraCenter();
    void testPaintedRoi();

private:
    QTemporaryDir m_dir;
};

void ScanfieldSeamControllerTest::initTestCase()
{
    QVERIFY(m_dir.isValid());
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());
}

void ScanfieldSeamControllerTest::testCtor()
{
    auto controller = new ScanfieldSeamController{this};

    QVERIFY(controller->scanfieldModule());
    QVERIFY(controller->hardwareParametersModule());
    QVERIFY(controller->transformation().isIdentity());
    QVERIFY(!controller->seam());
    QVERIFY(!controller->cameraCenterValid());

    QCOMPARE(controller->cameraCenter(), QPointF(-1, -1));
    QCOMPARE(controller->roi(), QRectF());
    QCOMPARE(controller->paintedRoi(), QRectF());
}

void ScanfieldSeamControllerTest::testSeam()
{
    auto controller = new ScanfieldSeamController{this};

    QVERIFY(!controller->seam());

    QSignalSpy seamChangedSpy{controller, &ScanfieldSeamController::seamChanged};
    QVERIFY(seamChangedSpy.isValid());

    QSignalSpy roiChangedSpy{controller, &ScanfieldSeamController::roiChanged};
    QVERIFY(roiChangedSpy.isValid());

    QSignalSpy paintedRoiChangedSpy{controller, &ScanfieldSeamController::paintedRoiChanged};
    QVERIFY(paintedRoiChangedSpy.isValid());

    QSignalSpy cameraCenterChangedSpy{controller, &ScanfieldSeamController::cameraCenterChanged};
    QVERIFY(cameraCenterChangedSpy.isValid());

    controller->setSeam(nullptr);
    QCOMPARE(seamChangedSpy.count(), 0);
    QCOMPARE(roiChangedSpy.count(), 0);
    QCOMPARE(paintedRoiChangedSpy.count(), 0);
    QCOMPARE(cameraCenterChangedSpy.count(), 0);

    auto product = new Product{QUuid::createUuid(), this};
    auto series1 = product->createSeamSeries();
    QVERIFY(series1);
    auto seam1 = series1->createSeam();
    QVERIFY(seam1);

    controller->setSeam(seam1);
    QCOMPARE(controller->seam(), seam1);
    QCOMPARE(seamChangedSpy.count(), 1);
    // seamChanged -> scanfiledModule::seriesChanged -> scanfiledModule::calibrationFileChanged -> roiChanged
    // seamChanged -> roiChanged
    QCOMPARE(roiChangedSpy.count(), 2);
    QCOMPARE(paintedRoiChangedSpy.count(), 2);
    QCOMPARE(cameraCenterChangedSpy.count(), 2);

    controller->setSeam(seam1);
    QCOMPARE(seamChangedSpy.count(), 1);
    QCOMPARE(roiChangedSpy.count(), 2);
    QCOMPARE(paintedRoiChangedSpy.count(), 2);
    QCOMPARE(cameraCenterChangedSpy.count(), 2);

    seam1->setRoi({20, 40, 200, 100});
    QCOMPARE(roiChangedSpy.count(), 3);
    QCOMPARE(paintedRoiChangedSpy.count(), 3);

    seam1->deleteLater();
    QVERIFY(seamChangedSpy.wait());
    QVERIFY(!controller->seam());
    QCOMPARE(seamChangedSpy.count(), 2);
    // seamChanged does not trigger a calibrationFileChanged, as the previous config was invalid and remains invalid
    QCOMPARE(roiChangedSpy.count(), 4);
    QCOMPARE(paintedRoiChangedSpy.count(), 4);
    QCOMPARE(cameraCenterChangedSpy.count(), 3);
}

void ScanfieldSeamControllerTest::testTransformation()
{
    auto controller = new ScanfieldSeamController{this};

    QVERIFY(controller->transformation().isIdentity());

    QSignalSpy transformationChangedSpy{controller, &ScanfieldSeamController::transformationChanged};
    QVERIFY(transformationChangedSpy.isValid());

    QSignalSpy roiChangedSpy{controller, &ScanfieldSeamController::roiChanged};
    QVERIFY(roiChangedSpy.isValid());

    QSignalSpy paintedRoiChangedSpy{controller, &ScanfieldSeamController::paintedRoiChanged};
    QVERIFY(paintedRoiChangedSpy.isValid());

    controller->setTransformation({});
    QCOMPARE(transformationChangedSpy.count(), 0);
    QCOMPARE(roiChangedSpy.count(), 0);
    QCOMPARE(paintedRoiChangedSpy.count(), 0);

    QMatrix4x4 transformation;
    transformation.scale(2);

    controller->setTransformation(transformation);
    QCOMPARE(controller->transformation(), transformation);
    QCOMPARE(transformationChangedSpy.count(), 1);
    QCOMPARE(roiChangedSpy.count(), 0);
    QCOMPARE(paintedRoiChangedSpy.count(), 1);

    controller->setTransformation(transformation);
    QCOMPARE(transformationChangedSpy.count(), 1);
    QCOMPARE(roiChangedSpy.count(), 0);
    QCOMPARE(paintedRoiChangedSpy.count(), 1);

    QMatrix4x4 transformation2;
    transformation2.scale(5);

    controller->setTransformation(transformation2);
    QCOMPARE(controller->transformation(), transformation2);
    QCOMPARE(transformationChangedSpy.count(), 2);
    QCOMPARE(roiChangedSpy.count(), 0);
    QCOMPARE(paintedRoiChangedSpy.count(), 2);
}

void ScanfieldSeamControllerTest::testCameraCenter()
{
    auto controller = new ScanfieldSeamController{this};

    QCOMPARE(controller->cameraCenter(), QPointF(-1, -1));

    QSignalSpy cameraCenterChangedSpy{controller, &ScanfieldSeamController::cameraCenterChanged};
    QVERIFY(cameraCenterChangedSpy.isValid());

    auto product = new Product{QUuid::createUuid(), this};
    auto series1 = product->createSeamSeries();
    QVERIFY(series1);

    auto seam_withParams = series1->createSeam();
    QVERIFY(seam_withParams);

    // create attribute controller and accquire ScannerNewXPosition and ScannerNewYPosition attributes
    auto attributeModel = new AttributeModel{this};
    QSignalSpy modelResetSpy(attributeModel, &AttributeModel::modelReset);
    attributeModel->load(QFINDTESTDATA("testdata/scanner_attributes.json"));
    QVERIFY(modelResetSpy.isValid());
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(attributeModel->rowCount(), 4);

    auto xPositionAttribute = attributeModel->findAttribute("553C51F1-F66D-4B36-92B4-6E84D464F833");
    QVERIFY(xPositionAttribute);
    auto yPositionAttribute = attributeModel->findAttribute("F8DAF95D-B164-4686-8BAB-02FA473FCA94");
    QVERIFY(yPositionAttribute);

    seam_withParams->createHardwareParameters();
    auto parameterSet = seam_withParams->hardwareParameters();

    auto xPosistion = parameterSet->createParameter(QUuid::createUuid(), xPositionAttribute, {});
    QVERIFY(xPosistion);
    xPosistion->setValue(7000);
    auto yPosition = parameterSet->createParameter(QUuid::createUuid(), yPositionAttribute, {});
    QVERIFY(yPosition);
    yPosition->setValue(6000);

    QCOMPARE(cameraCenterChangedSpy.count(), 0);

    controller->setSeam(seam_withParams);
    QCOMPARE(cameraCenterChangedSpy.count(), 2);

    // no attribute controller and calibration proxy
    QCOMPARE(controller->cameraCenter(), QPointF(-1, -1));

    controller->hardwareParametersModule()->setAttributeModel(attributeModel);
    QCOMPARE(cameraCenterChangedSpy.count(), 3);

    // no calibration proxy
    QCOMPARE(controller->cameraCenter(), QPointF(-1, -1));

    auto proxy = std::make_shared<precitec::interface::MockCalibrationProxy>();
    controller->scanfieldModule()->setCalibrationCoordinatesRequestProxy(proxy);

    QCOMPARE(cameraCenterChangedSpy.count(), 4);

    // no valid calibration file
    QCOMPARE(controller->cameraCenter(), QPointF(-1, -1));

    const auto& testdata = QFINDTESTDATA("testdata/scanner_data/1cd8f7cb-e57a-4c21-b05d-1b40cc2b0778/");
    controller->scanfieldModule()->loadCalibrationFile(testdata);
    QCOMPARE(cameraCenterChangedSpy.count(), 6);

    // valid setup
    QCOMPARE(controller->cameraCenter(), QPointF(700, 600));

    auto seam_noParams = series1->createSeam();
    QVERIFY(seam_noParams);

    // last valid seam is seam_withParams
    controller->setSeam(seam_noParams);
    QCOMPARE(cameraCenterChangedSpy.count(), 7);

    QCOMPARE(controller->cameraCenter(), QPointF(700, 600));

    seam_noParams->createHardwareParameters();
    auto parameterSet2 = seam_noParams->hardwareParameters();

    auto xPosistion2 = parameterSet2->createParameter(QUuid::createUuid(), xPositionAttribute, {});
    QVERIFY(xPosistion2);
    xPosistion2->setValue(9000);
    auto yPosition2 = parameterSet2->createParameter(QUuid::createUuid(), yPositionAttribute, {});
    QVERIFY(yPosition2);
    yPosition2->setValue(7000);

    QCOMPARE(cameraCenterChangedSpy.count(), 8);
    QCOMPARE(controller->cameraCenter(), QPointF(900, 700));

    controller->hardwareParametersModule()->setAttributeModel(nullptr);

    // no attribute controller
    QCOMPARE(controller->cameraCenter(), QPointF(-1, -1));
}

void ScanfieldSeamControllerTest::testPaintedRoi()
{
    auto controller = new ScanfieldSeamController{this};

    QVERIFY(controller->transformation().isIdentity());
    QVERIFY(!controller->seam());
    QCOMPARE(controller->roi(), QRectF());
    QCOMPARE(controller->paintedRoi(), QRectF());

    QSignalSpy paintedRoiChangedSpy{controller, &ScanfieldSeamController::paintedRoiChanged};
    QVERIFY(paintedRoiChangedSpy.isValid());

    controller->setPaintedRoi({800, 300, 50, 60});
    controller->resetRoi();
    QCOMPARE(paintedRoiChangedSpy.count(), 0);

    auto product = new Product{QUuid::createUuid(), this};
    auto series1 = product->createSeamSeries();
    QVERIFY(series1);

    auto seam1 = series1->createSeam();
    QVERIFY(seam1);

    // create attribute controller and accquire ScannerNewXPosition and ScannerNewYPosition attributes
    auto attributeModel = new AttributeModel{this};
    QSignalSpy modelResetSpy(attributeModel, &AttributeModel::modelReset);
    attributeModel->load(QFINDTESTDATA("testdata/scanner_attributes.json"));
    QVERIFY(modelResetSpy.isValid());
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(attributeModel->rowCount(), 4);

    auto xPositionAttribute = attributeModel->findAttribute("553C51F1-F66D-4B36-92B4-6E84D464F833");
    QVERIFY(xPositionAttribute);
    auto yPositionAttribute = attributeModel->findAttribute("F8DAF95D-B164-4686-8BAB-02FA473FCA94");
    QVERIFY(yPositionAttribute);

    seam1->createHardwareParameters();
    auto parameterSet = seam1->hardwareParameters();

    auto xPosistion = parameterSet->createParameter(QUuid::createUuid(), xPositionAttribute, {});
    QVERIFY(xPosistion);
    xPosistion->setValue(7000);
    auto yPosition = parameterSet->createParameter(QUuid::createUuid(), yPositionAttribute, {});
    QVERIFY(yPosition);
    yPosition->setValue(6000);

    QCOMPARE(paintedRoiChangedSpy.count(), 0);

    controller->setSeam(seam1);
    QCOMPARE(paintedRoiChangedSpy.count(), 2);

    // no attribute controller and calibration proxy
    QCOMPARE(controller->roi(), QRectF());
    QCOMPARE(controller->paintedRoi(), QRectF());

    controller->hardwareParametersModule()->setAttributeModel(attributeModel);
    QCOMPARE(paintedRoiChangedSpy.count(), 2);

    // no calibration proxy
    QCOMPARE(controller->roi(), QRectF());
    QCOMPARE(controller->paintedRoi(), QRectF());

    auto proxy = std::make_shared<precitec::interface::MockCalibrationProxy>();
    controller->scanfieldModule()->setCalibrationCoordinatesRequestProxy(proxy);

    QCOMPARE(paintedRoiChangedSpy.count(), 3);

    // no valid calibration file
    QCOMPARE(controller->cameraCenter(), QPointF(-1, -1));

    const auto& testdata = QFINDTESTDATA("testdata/scanner_data/1cd8f7cb-e57a-4c21-b05d-1b40cc2b0778/");
    controller->scanfieldModule()->loadCalibrationFile(testdata);
    QCOMPARE(paintedRoiChangedSpy.count(), 4);

    // no transformation and roi
    QCOMPARE(controller->roi(), QRectF());
    QCOMPARE(controller->paintedRoi(), QRectF());

    // no transformation
    seam1->setRoi({700, 500, 100, 200});
    QCOMPARE(paintedRoiChangedSpy.count(), 5);
    // mirrored
    QCOMPARE(controller->roi(), QRectF(1280 - 700 - 100, 1024 - 500 - 200, 100, 200));
    QCOMPARE(controller->paintedRoi(), QRectF());

    QMatrix4x4 transformation;
    transformation.scale(2);

    controller->setTransformation(transformation);
    QCOMPARE(paintedRoiChangedSpy.count(), 6);

    // valid setup
    // mirrored
    QCOMPARE(controller->roi(), QRectF(1280 - 700 - 100, 1024 - 500 - 200, 100, 200));
    // mirrored, moved into camera image (center at 700 x 600) and scaled x2
    QCOMPARE(controller->paintedRoi(), QRectF(2 * (1280 - 700 - 100 + (700 - 640)), 2 * (1024 - 500 - 200 + (600 - 512)), 100 * 2, 200 * 2));

    controller->setPaintedRoi({800, 300, 50, 60});
    QCOMPARE(paintedRoiChangedSpy.count(), 7);

    // roi in seam must be mirrored from the according to the config
    QCOMPARE(seam1->roi(), QRect(1280 - 800 - 50, 1024 - 300 - 60, 50, 60));
    // roi in controller is in image coords so we mirror the seam roi again
    QCOMPARE(controller->roi(), QRectF(1280 - (1280 - 800 - 50) - 50, 1024 - (1024 - 300 - 60) - 60, 50, 60));
    // painted roi is then transposed to the camera image and scaled x2
    QCOMPARE(controller->paintedRoi(), QRectF(2 * (1280 - (1280 - 800 - 50) - 50 + (700 - 640)), 2 * (1024 - (1024 - 300 - 60) - 60 + (600 - 512)), 50 * 2, 60 * 2));

    controller->resetRoi();
    QCOMPARE(controller->roi(), QRectF());
    QCOMPARE(paintedRoiChangedSpy.count(), 8);
}

QTEST_GUILESS_MAIN(ScanfieldSeamControllerTest)
#include "scanfieldSeamControllerTest.moc"

