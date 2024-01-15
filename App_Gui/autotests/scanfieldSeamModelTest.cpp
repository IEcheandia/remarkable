#include <QTest>
#include <QSignalSpy>

#include "../src/scanfieldSeamModel.h"
#include "../src/scanfieldModule.h"
#include "../src/hardwareParametersModule.h"
#include "../src/hardwareParameters.h"
#include "product.h"
#include "seamSeries.h"
#include "seam.h"
#include "attributeModel.h"
#include "parameterSet.h"
#include "parameter.h"
#include "message/calibrationCoordinatesRequest.interface.h"

using precitec::gui::ScanfieldSeamModel;
using precitec::gui::HardwareParameters;
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

class ScanfieldSeamModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testRoles();
    void testUpdateModel();
    void testSeamSeries();
    void testSelectSeam();
    void testSelectSeams();
    void testCameraCenter_data();
    void testCameraCenter();
    void testLastValidSeam();
    void testTransformation();
    void testShowAllSeams();
    void testPointInSeam();

private:
    QTemporaryDir m_dir;
};

void ScanfieldSeamModelTest::initTestCase()
{
    QVERIFY(m_dir.isValid());
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());
}

void ScanfieldSeamModelTest::testCtor()
{
    auto model = new ScanfieldSeamModel{this};

    QCOMPARE(model->roleNames().count(), 7);
    QCOMPARE(model->rowCount(), 0);

    QVERIFY(model->scanfieldModule());
    QVERIFY(model->hardwareParametersModule());
    QVERIFY(!model->seamSeries());
    QVERIFY(!model->seam());
    QVERIFY(!model->lastValidSeam());
    QVERIFY(model->transformation().isIdentity());

    QCOMPARE(model->currentCenter(), QPointF(-1, -1));
    QCOMPARE(model->currentCenterInMM(), QPointF(-1, -1));
    QCOMPARE(model->lastCenter(), QPointF(-1, -1));
    QCOMPARE(model->lastCenterInMM(), QPointF(-1, -1));
    QCOMPARE(model->currentPaintedCenter(), QPointF(-1, -1));

    QVERIFY(!model->currentCenterValid());
    QVERIFY(!model->lastCenterValid());
    QVERIFY(model->currentRect().isNull());
    QVERIFY(!model->leftEnabled());
    QVERIFY(!model->rightEnabled());
    QVERIFY(!model->topEnabled());
    QVERIFY(!model->bottomEnabled());
    QVERIFY(model->showAllSeams());

    QVERIFY(!model->octWithReferenceArms());
    QVERIFY(!model->currentDriveWithOCTReference());
}

void ScanfieldSeamModelTest::testRoles()
{
    auto model = new ScanfieldSeamModel{this};
    const auto& roles = model->roleNames();
    QCOMPARE(roles.count(), 7);
    QCOMPARE(roles.value(Qt::DisplayRole), QByteArrayLiteral("text"));
    QCOMPARE(roles.value(Qt::UserRole), QByteArrayLiteral("cameraCenter"));
    QCOMPARE(roles.value(Qt::UserRole + 1), QByteArrayLiteral("cameraCenterValid"));
    QCOMPARE(roles.value(Qt::UserRole + 2), QByteArrayLiteral("cameraRect"));
    QCOMPARE(roles.value(Qt::UserRole + 3), QByteArrayLiteral("paintedCameraRect"));
    QCOMPARE(roles.value(Qt::UserRole + 4), QByteArrayLiteral("seam"));
    QCOMPARE(roles.value(Qt::UserRole + 5), QByteArrayLiteral("showSeam"));
}

void ScanfieldSeamModelTest::testUpdateModel()
{
    auto model = new ScanfieldSeamModel{this};

    QVERIFY(!model->seamSeries());
    QVERIFY(!model->seam());
    QCOMPARE(model->rowCount(), 0);

    QSignalSpy modelResetSpy{model, &ScanfieldSeamModel::modelReset};
    QVERIFY(modelResetSpy.isValid());

    QSignalSpy seamChangedSpy{model, &ScanfieldSeamModel::seamChanged};
    QVERIFY(seamChangedSpy.isValid());

    QSignalSpy dataChangedSpy{model, &ScanfieldSeamModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    model->setSeamSeries(nullptr);
    QCOMPARE(modelResetSpy.count(), 0);
    QCOMPARE(seamChangedSpy.count(), 0);
    QCOMPARE(dataChangedSpy.count(), 0);

    auto product = new Product{QUuid::createUuid(), this};
    auto series1 = product->createSeamSeries();
    QVERIFY(series1);

    model->setSeamSeries(series1);
    QCOMPARE(modelResetSpy.count(), 1);
    QCOMPARE(model->rowCount(), 0);
    QCOMPARE(seamChangedSpy.count(), 0);
    // seamSeriesChanged -> scanfiledModule::seriesChanged -> scanfiledModule::calibrationFileChanged -> currentCenterChanged -> dataChanged on UserRole + 5
    // seamSeriesChanged -> scanfiledModule::seriesChanged -> scanfiledModule::calibrationFileChanged -> dataChanged on UserRole, UserRole + 1, UserRole + 2, UserRole + 3
    QCOMPARE(dataChangedSpy.count(), 2);
    QCOMPARE(dataChangedSpy.at(0).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(0).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));
    QCOMPARE(dataChangedSpy.at(1).at(2).value<QVector<int>>().size(), 4);
    QVERIFY(dataChangedSpy.at(1).at(2).value<QVector<int>>().contains(Qt::UserRole));
    QVERIFY(dataChangedSpy.at(1).at(2).value<QVector<int>>().contains(Qt::UserRole + 1));
    QVERIFY(dataChangedSpy.at(1).at(2).value<QVector<int>>().contains(Qt::UserRole + 2));
    QVERIFY(dataChangedSpy.at(1).at(2).value<QVector<int>>().contains(Qt::UserRole + 3));

    model->setSeamSeries(series1);
    QCOMPARE(modelResetSpy.count(), 1);
    QCOMPARE(seamChangedSpy.count(), 0);
    QCOMPARE(dataChangedSpy.count(), 2);

    auto seam1 = series1->createSeam();
    QVERIFY(seam1);

    QCOMPARE(model->rowCount(), 1);
    QCOMPARE(modelResetSpy.count(), 2);
    QCOMPARE(seamChangedSpy.count(), 0);
    QCOMPARE(dataChangedSpy.count(), 2);

    model->selectSeam(seam1->uuid());
    QCOMPARE(model->seam(), seam1);
    QCOMPARE(seamChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.count(), 3);
    QCOMPARE(dataChangedSpy.at(2).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(2).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));

    series1->createSeam();
    QCOMPARE(model->rowCount(), 2);
    QCOMPARE(model->seam(), seam1);
    QCOMPARE(modelResetSpy.count(), 3);
    QCOMPARE(seamChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.count(), 3);

    auto series2 = product->createSeamSeries();
    QVERIFY(series2);
    series2->createSeam();

    model->setSeamSeries(series2);
    QCOMPARE(modelResetSpy.count(), 4);
    QCOMPARE(model->rowCount(), 1);
    QCOMPARE(seamChangedSpy.count(), 2);
    QCOMPARE(dataChangedSpy.count(), 6);
    // two from seamSeriesChanged, one from seamChanged
    QCOMPARE(dataChangedSpy.at(3).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(3).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));
    QCOMPARE(dataChangedSpy.at(4).at(2).value<QVector<int>>().size(), 4);
    QVERIFY(dataChangedSpy.at(4).at(2).value<QVector<int>>().contains(Qt::UserRole));
    QVERIFY(dataChangedSpy.at(4).at(2).value<QVector<int>>().contains(Qt::UserRole + 1));
    QVERIFY(dataChangedSpy.at(4).at(2).value<QVector<int>>().contains(Qt::UserRole + 2));
    QVERIFY(dataChangedSpy.at(4).at(2).value<QVector<int>>().contains(Qt::UserRole + 3));
    QCOMPARE(dataChangedSpy.at(5).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(5).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));
    QVERIFY(!model->seam());
}

void ScanfieldSeamModelTest::testSeamSeries()
{
    auto model = new ScanfieldSeamModel{this};

    QVERIFY(!model->seamSeries());
    QVERIFY(!model->seam());

    QSignalSpy seamSeriesChangedSpy(model, &ScanfieldSeamModel::seamSeriesChanged);
    QVERIFY(seamSeriesChangedSpy.isValid());

    QSignalSpy seamChangedSpy{model, &ScanfieldSeamModel::seamChanged};
    QVERIFY(seamChangedSpy.isValid());

    QSignalSpy dataChangedSpy{model, &ScanfieldSeamModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    model->setSeamSeries(nullptr);
    QCOMPARE(seamSeriesChangedSpy.count(), 0);
    QCOMPARE(dataChangedSpy.count(), 0);

    auto product = new Product{QUuid::createUuid(), this};
    auto series1 = product->createSeamSeries();
    QVERIFY(series1);

    model->setSeamSeries(series1);
    QCOMPARE(model->seamSeries(), series1);
    QCOMPARE(seamSeriesChangedSpy.count(), 1);
    // seamSeriesChanged -> scanfiledModule::seriesChanged -> scanfiledModule::calibrationFileChanged -> currentCenterChanged -> dataChanged on UserRole + 5
    // seamSeriesChanged -> scanfiledModule::seriesChanged -> scanfiledModule::calibrationFileChanged -> dataChanged on UserRole, UserRole + 1, UserRole + 2, UserRole + 3
    QCOMPARE(dataChangedSpy.count(), 2);
    QCOMPARE(dataChangedSpy.at(0).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(0).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));
    QCOMPARE(dataChangedSpy.at(1).at(2).value<QVector<int>>().size(), 4);
    QVERIFY(dataChangedSpy.at(1).at(2).value<QVector<int>>().contains(Qt::UserRole));
    QVERIFY(dataChangedSpy.at(1).at(2).value<QVector<int>>().contains(Qt::UserRole + 1));
    QVERIFY(dataChangedSpy.at(1).at(2).value<QVector<int>>().contains(Qt::UserRole + 2));
    QVERIFY(dataChangedSpy.at(1).at(2).value<QVector<int>>().contains(Qt::UserRole + 3));

    model->setSeamSeries(series1);
    QCOMPARE(seamSeriesChangedSpy.count(), 1);

    auto series2 = product->createSeamSeries();
    QVERIFY(series2);

    model->setSeamSeries(series2);
    QCOMPARE(seamSeriesChangedSpy.count(), 2);
    QCOMPARE(dataChangedSpy.count(), 4);
    QCOMPARE(dataChangedSpy.at(2).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(2).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));
    QCOMPARE(dataChangedSpy.at(3).at(2).value<QVector<int>>().size(), 4);
    QVERIFY(dataChangedSpy.at(3).at(2).value<QVector<int>>().contains(Qt::UserRole));
    QVERIFY(dataChangedSpy.at(3).at(2).value<QVector<int>>().contains(Qt::UserRole + 1));
    QVERIFY(dataChangedSpy.at(3).at(2).value<QVector<int>>().contains(Qt::UserRole + 2));
    QVERIFY(dataChangedSpy.at(3).at(2).value<QVector<int>>().contains(Qt::UserRole + 3));

    series1->deleteLater();
    QVERIFY(!seamSeriesChangedSpy.wait());
    QCOMPARE(seamSeriesChangedSpy.count(), 2);

    auto seam = series2->createSeam();
    QVERIFY(seam);
    model->selectSeam(seam->uuid());
    QCOMPARE(model->seam(), seam);
    QCOMPARE(seamChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.count(), 5);
    QCOMPARE(dataChangedSpy.at(4).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(4).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));

    series2->deleteLater();
    QVERIFY(seamSeriesChangedSpy.wait());
    QCOMPARE(seamSeriesChangedSpy.count(), 3);
    QCOMPARE(seamChangedSpy.count(), 2);
    QVERIFY(!model->seamSeries());
    QVERIFY(!model->seam());
    QCOMPARE(dataChangedSpy.count(), 6);
    // one from seamChanged
    // seamSeriesChanged does not trigger a calibrationFileChanged, as the previous config was invalid and remains invalid
    QCOMPARE(dataChangedSpy.at(5).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(5).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));
}

void ScanfieldSeamModelTest::testSelectSeam()
{
    auto model = new ScanfieldSeamModel{this};

    QVERIFY(!model->seam());

    QSignalSpy seamChangedSpy{model, &ScanfieldSeamModel::seamChanged};
    QVERIFY(seamChangedSpy.isValid());

    QSignalSpy dataChangedSpy{model, &ScanfieldSeamModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    auto product = new Product{QUuid::createUuid(), this};
    auto series1 = product->createSeamSeries();
    QVERIFY(series1);
    auto seam1 = series1->createSeam();
    QVERIFY(seam1);

    model->selectSeam({});
    QCOMPARE(seamChangedSpy.count(), 0);
    QCOMPARE(dataChangedSpy.count(), 0);

    model->selectSeam(seam1->uuid());
    QCOMPARE(seamChangedSpy.count(), 0);
    QCOMPARE(dataChangedSpy.count(), 0);

    model->setSeamSeries(series1);
    // seamSeriesChanged -> scanfiledModule::seriesChanged -> scanfiledModule::calibrationFileChanged -> currentCenterChanged -> dataChanged on UserRole + 5
    // seamSeriesChanged -> scanfiledModule::seriesChanged -> scanfiledModule::calibrationFileChanged -> dataChanged on UserRole, UserRole + 1, UserRole + 2, UserRole + 3
    QCOMPARE(dataChangedSpy.count(), 2);
    QCOMPARE(dataChangedSpy.at(0).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(0).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));
    QCOMPARE(dataChangedSpy.at(1).at(2).value<QVector<int>>().size(), 4);
    QVERIFY(dataChangedSpy.at(1).at(2).value<QVector<int>>().contains(Qt::UserRole));
    QVERIFY(dataChangedSpy.at(1).at(2).value<QVector<int>>().contains(Qt::UserRole + 1));
    QVERIFY(dataChangedSpy.at(1).at(2).value<QVector<int>>().contains(Qt::UserRole + 2));
    QVERIFY(dataChangedSpy.at(1).at(2).value<QVector<int>>().contains(Qt::UserRole + 3));

    model->selectSeam(seam1->uuid());
    QCOMPARE(model->seam(), seam1);
    QCOMPARE(seamChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.count(), 3);
    QCOMPARE(dataChangedSpy.at(2).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(2).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));

    model->selectSeam(QUuid::createUuid());
    QVERIFY(!model->seam());
    QCOMPARE(seamChangedSpy.count(), 2);
    QCOMPARE(dataChangedSpy.count(), 4);
    QCOMPARE(dataChangedSpy.at(3).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(3).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));

    model->selectSeam(seam1->uuid());
    QCOMPARE(model->seam(), seam1);
    QCOMPARE(seamChangedSpy.count(), 3);
    QCOMPARE(dataChangedSpy.count(), 5);
    QCOMPARE(dataChangedSpy.at(4).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(4).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));

    model->selectSeam({});
    QVERIFY(!model->seam());
    QCOMPARE(seamChangedSpy.count(), 4);
    QCOMPARE(dataChangedSpy.count(), 6);
    QCOMPARE(dataChangedSpy.at(5).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(5).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));

    model->selectSeam(seam1->uuid());
    QCOMPARE(model->seam(), seam1);
    QCOMPARE(seamChangedSpy.count(), 5);
    QCOMPARE(dataChangedSpy.count(), 7);
    QCOMPARE(dataChangedSpy.at(6).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(6).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));

    seam1->deleteLater();
    QVERIFY(seamChangedSpy.wait());
    QVERIFY(!model->seam());
    QCOMPARE(seamChangedSpy.count(), 6);
    QCOMPARE(dataChangedSpy.count(), 8);
    QCOMPARE(dataChangedSpy.at(7).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(7).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));
}

void ScanfieldSeamModelTest::testSelectSeams()
{
    auto model = new ScanfieldSeamModel{this};

    QCOMPARE(model->selectSeams({}), QVariantList());
    QCOMPARE(model->selectSeams({500, 700}), QVariantList());

    auto product = new Product{QUuid::createUuid(), this};
    auto series1 = product->createSeamSeries();
    QVERIFY(series1);
    model->setSeamSeries(series1);

    QCOMPARE(model->selectSeams({}), QVariantList());
    QCOMPARE(model->selectSeams({500, 700}), QVariantList());

    QMatrix4x4 transformation;
    transformation.scale(2);

    model->setTransformation(transformation);

    QCOMPARE(model->selectSeams({}), QVariantList());
    QCOMPARE(model->selectSeams({500, 700}), QVariantList());

    // setup attribute model, calibration proxy and calibration file
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

    model->hardwareParametersModule()->setAttributeModel(attributeModel);

    auto proxy = std::make_shared<precitec::interface::MockCalibrationProxy>();
    model->scanfieldModule()->setCalibrationCoordinatesRequestProxy(proxy);

    const auto& testdata = QFINDTESTDATA("testdata/scanner_data/1cd8f7cb-e57a-4c21-b05d-1b40cc2b0778/");
    model->scanfieldModule()->loadCalibrationFile(testdata);

    auto seam1 = series1->createSeam();
    QVERIFY(seam1);

    seam1->createHardwareParameters();
    auto seam1ParameterSet = seam1->hardwareParameters();

    // (700, 800) after calibration
    // Camera Rect in Image: (120,576 2560x2048)
    auto seam1xPosistion = seam1ParameterSet->createParameter(QUuid::createUuid(), xPositionAttribute, {});
    QVERIFY(seam1xPosistion);
    seam1xPosistion->setValue(7000);
    auto seam1yPosition = seam1ParameterSet->createParameter(QUuid::createUuid(), yPositionAttribute, {});
    QVERIFY(seam1yPosition);
    seam1yPosition->setValue(8000);

    QCOMPARE(model->selectSeams({}), QVariantList());
    QCOMPARE(model->selectSeams({500, 700}), QVariantList({QVariant::fromValue(seam1)}));

    auto seam2 = series1->createSeam();
    QVERIFY(seam2);

    seam2->createHardwareParameters();
    auto seam2ParameterSet = seam2->hardwareParameters();

    // (500, 400) after calibration - not a valid center (values are less than the minimum defined by the camera and image size)
    // Camera Rect in Image: (0,0 2560x2048)
    auto seam2xPosistion = seam2ParameterSet->createParameter(QUuid::createUuid(), xPositionAttribute, {});
    QVERIFY(seam2xPosistion);
    seam2xPosistion->setValue(5000);
    auto seam2yPosition = seam2ParameterSet->createParameter(QUuid::createUuid(), yPositionAttribute, {});
    QVERIFY(seam2yPosition);
    seam2yPosition->setValue(4000);

    QCOMPARE(model->selectSeams({}), QVariantList());
    QCOMPARE(model->selectSeams({500, 700}), QVariantList({QVariant::fromValue(seam1)}));

    auto seam3 = series1->createSeam();
    QVERIFY(seam3);

    seam3->createHardwareParameters();
    auto seam3ParameterSet = seam3->hardwareParameters();

    // (850, 800) after calibration
    // Camera Rect in Image: (420,576 2560x2048)
    auto seam3xPosistion = seam3ParameterSet->createParameter(QUuid::createUuid(), xPositionAttribute, {});
    QVERIFY(seam3xPosistion);
    seam3xPosistion->setValue(8500);
    auto seam3yPosition = seam3ParameterSet->createParameter(QUuid::createUuid(), yPositionAttribute, {});
    QVERIFY(seam3yPosition);
    seam3yPosition->setValue(8000);

    QCOMPARE(model->selectSeams({}), QVariantList());
    QCOMPARE(model->selectSeams({500, 700}), QVariantList({QVariant::fromValue(seam1), QVariant::fromValue(seam3)}));

    auto seam4 = series1->createSeam();
    QVERIFY(seam4);

    seam4->createHardwareParameters();
    auto seam4ParameterSet = seam4->hardwareParameters();

    // (1500, 1200) after calibration
    // Camera Rect in Image: (1720,1376 2560x2048) - does not contain point
    auto seam4xPosistion = seam4ParameterSet->createParameter(QUuid::createUuid(), xPositionAttribute, {});
    QVERIFY(seam4xPosistion);
    seam4xPosistion->setValue(15000);
    auto seam4yPosition = seam4ParameterSet->createParameter(QUuid::createUuid(), yPositionAttribute, {});
    QVERIFY(seam4yPosition);
    seam4yPosition->setValue(12000);

    QCOMPARE(model->selectSeams({}), QVariantList());
    QCOMPARE(model->selectSeams({500, 700}), QVariantList({QVariant::fromValue(seam1), QVariant::fromValue(seam3)}));

    // Show only the last valid seam in regard to the current seam in image, therefore allow only that seam to be selected through mouse click
    // No current seam selected
    model->setShowAllSeams(false);

    QCOMPARE(model->selectSeams({}), QVariantList());
    QCOMPARE(model->selectSeams({500, 700}), QVariantList());

    // Last valid seam of seam1 is seam1
    model->selectSeam(seam1->uuid());

    QCOMPARE(model->selectSeams({}), QVariantList());
    QCOMPARE(model->selectSeams({500, 700}), QVariantList({QVariant::fromValue(seam1)}));

    // Last valid seam of seam2 is seam1
    model->selectSeam(seam2->uuid());

    QCOMPARE(model->selectSeams({}), QVariantList());
    QCOMPARE(model->selectSeams({500, 700}), QVariantList({QVariant::fromValue(seam1)}));

    // Last valid seam of seam3 is seam3
    model->selectSeam(seam3->uuid());

    QCOMPARE(model->selectSeams({}), QVariantList());
    QCOMPARE(model->selectSeams({500, 700}), QVariantList({QVariant::fromValue(seam3)}));
}

void ScanfieldSeamModelTest::testCameraCenter_data()
{
    QTest::addColumn<bool>("octWithReferenceArms");

    QTest::newRow("no oct") << false;
    QTest::newRow("oct") << true;
}

void ScanfieldSeamModelTest::testCameraCenter()
{
    auto model = new ScanfieldSeamModel{this};
    QFETCH(bool, octWithReferenceArms);
    model->setOctWithReferenceArms(octWithReferenceArms);
    QCOMPARE(model->octWithReferenceArms(), octWithReferenceArms);

    QCOMPARE(model->cameraCenter(nullptr), QPointF(-1, -1));

    QSignalSpy dataChangedSpy{model, &ScanfieldSeamModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    QSignalSpy currentCenterChangedSpy{model, &ScanfieldSeamModel::currentCenterChanged};
    QVERIFY(currentCenterChangedSpy.isValid());

    auto product = new Product{QUuid::createUuid(), this};
    auto series1 = product->createSeamSeries();
    QVERIFY(series1);
    auto seam_noParams = series1->createSeam();
    QVERIFY(seam_noParams);

    QCOMPARE(model->cameraCenter(seam_noParams), QPointF(-1, -1));

    QCOMPARE(dataChangedSpy.count(), 0);
    QCOMPARE(currentCenterChangedSpy.count(), 0);

    // create attribute model and accquire ScannerNewXPosition and ScannerNewYPosition attributes
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

    auto seam_withParams = series1->createSeam();
    QVERIFY(seam_withParams);

    QCOMPARE(dataChangedSpy.count(), 0);
    QCOMPARE(currentCenterChangedSpy.count(), 0);

    seam_withParams->createHardwareParameters();
    auto parameterSet = seam_withParams->hardwareParameters();

    auto xPosistion = parameterSet->createParameter(QUuid::createUuid(), xPositionAttribute, {});
    QVERIFY(xPosistion);
    xPosistion->setValue(10);
    auto yPosition = parameterSet->createParameter(QUuid::createUuid(), yPositionAttribute, {});
    QVERIFY(yPosition);
    yPosition->setValue(15);

    QCOMPARE(dataChangedSpy.count(), 0);
    QCOMPARE(currentCenterChangedSpy.count(), 0);

    // no attribute model and calibration proxy
    QCOMPARE(model->cameraCenter(seam_withParams), QPointF(-1, -1));

    model->hardwareParametersModule()->setAttributeModel(attributeModel);

    QCOMPARE(dataChangedSpy.count(), 2);
    QCOMPARE(dataChangedSpy.at(0).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(0).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));
    QCOMPARE(dataChangedSpy.at(1).at(2).value<QVector<int>>().size(), 4);
    QVERIFY(dataChangedSpy.at(1).at(2).value<QVector<int>>().contains(Qt::UserRole));
    QVERIFY(dataChangedSpy.at(1).at(2).value<QVector<int>>().contains(Qt::UserRole + 1));
    QVERIFY(dataChangedSpy.at(1).at(2).value<QVector<int>>().contains(Qt::UserRole + 2));
    QVERIFY(dataChangedSpy.at(1).at(2).value<QVector<int>>().contains(Qt::UserRole + 3));
    QCOMPARE(currentCenterChangedSpy.count(), 1);

    // no calibration proxy
    QCOMPARE(model->cameraCenter(seam_noParams), QPointF(-1, -1));
    QCOMPARE(model->cameraCenter(seam_withParams), QPointF(-1, -1));

    auto proxy = std::make_shared<precitec::interface::MockCalibrationProxy>();
    model->scanfieldModule()->setCalibrationCoordinatesRequestProxy(proxy);

    QCOMPARE(dataChangedSpy.count(), 4);
    QCOMPARE(dataChangedSpy.at(2).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(2).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));
    QCOMPARE(dataChangedSpy.at(3).at(2).value<QVector<int>>().size(), 4);
    QVERIFY(dataChangedSpy.at(3).at(2).value<QVector<int>>().contains(Qt::UserRole));
    QVERIFY(dataChangedSpy.at(3).at(2).value<QVector<int>>().contains(Qt::UserRole + 1));
    QVERIFY(dataChangedSpy.at(3).at(2).value<QVector<int>>().contains(Qt::UserRole + 2));
    QVERIFY(dataChangedSpy.at(3).at(2).value<QVector<int>>().contains(Qt::UserRole + 3));
    QCOMPARE(currentCenterChangedSpy.count(), 2);

    // no valid calibration file
    QCOMPARE(model->cameraCenter(seam_noParams), QPointF(-1, -1));
    QCOMPARE(model->cameraCenter(seam_withParams), QPointF(-1, -1));

    model->setSeamSeries(series1);
    QVERIFY(!model->seam());

    QCOMPARE(dataChangedSpy.count(), 6);
    QCOMPARE(dataChangedSpy.at(4).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(4).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));
    QCOMPARE(dataChangedSpy.at(5).at(2).value<QVector<int>>().size(), 4);
    QVERIFY(dataChangedSpy.at(5).at(2).value<QVector<int>>().contains(Qt::UserRole));
    QVERIFY(dataChangedSpy.at(5).at(2).value<QVector<int>>().contains(Qt::UserRole + 1));
    QVERIFY(dataChangedSpy.at(5).at(2).value<QVector<int>>().contains(Qt::UserRole + 2));
    QVERIFY(dataChangedSpy.at(5).at(2).value<QVector<int>>().contains(Qt::UserRole + 3));
    QCOMPARE(currentCenterChangedSpy.count(), 3);

    const auto& testdata = QFINDTESTDATA("testdata/scanner_data/1cd8f7cb-e57a-4c21-b05d-1b40cc2b0778/");
    model->scanfieldModule()->loadCalibrationFile(testdata);

    QCOMPARE(dataChangedSpy.count(), 10);
    QCOMPARE(dataChangedSpy.at(6).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(6).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));
    QCOMPARE(dataChangedSpy.at(9).at(2).value<QVector<int>>().size(), 4);
    QVERIFY(dataChangedSpy.at(9).at(2).value<QVector<int>>().contains(Qt::UserRole));
    QVERIFY(dataChangedSpy.at(9).at(2).value<QVector<int>>().contains(Qt::UserRole + 1));
    QVERIFY(dataChangedSpy.at(9).at(2).value<QVector<int>>().contains(Qt::UserRole + 2));
    QVERIFY(dataChangedSpy.at(9).at(2).value<QVector<int>>().contains(Qt::UserRole + 3));
    QCOMPARE(currentCenterChangedSpy.count(), 5);

    // valid setup
    QCOMPARE(model->cameraCenter(seam_noParams), QPointF(-1, -1));
    QCOMPARE(model->cameraCenter(seam_withParams), QPointF(1, 1.5));

    // no current seam
    model->setCameraCenter({2, 2});
    QCOMPARE(dataChangedSpy.count(), 10);
    QCOMPARE(currentCenterChangedSpy.count(), 5);
    QVERIFY(!model->currentDriveWithOCTReference());

    model->selectSeam(seam_noParams->uuid());
    QVERIFY(model->seam());
    QCOMPARE(model->currentDriveWithOCTReference(), octWithReferenceArms);
    QCOMPARE(currentCenterChangedSpy.count(), 6);
    QCOMPARE(dataChangedSpy.count(), 11);
    QCOMPARE(dataChangedSpy.at(8).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(8).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));

    // to small, will be set bound to smallest possible value (i.e. cameraWidth / 2, cameraHeight / 2)
    model->setCameraCenter({2, 2});
    QCOMPARE(model->currentCenter(), QPointF(640, 512));
    QCOMPARE(model->cameraCenter(seam_noParams), QPointF(640, 512));
    QCOMPARE(model->cameraCenter(seam_withParams), QPointF(1, 1.5));
    // two signals - one from hardwareParametersChanged, when the set is created and one from setCameraCenter
    QCOMPARE(currentCenterChangedSpy.count(), 8);
    QCOMPARE(dataChangedSpy.count(), 14);
    QCOMPARE(dataChangedSpy.at(11).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(11).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));
    QCOMPARE(dataChangedSpy.at(12).at(0).toModelIndex(), model->index(0));
    QCOMPARE(dataChangedSpy.at(12).at(1).toModelIndex(), model->index(0));
    QCOMPARE(dataChangedSpy.at(12).at(2).value<QVector<int>>().size(), 4);
    QVERIFY(dataChangedSpy.at(12).at(2).value<QVector<int>>().contains(Qt::UserRole));
    QVERIFY(dataChangedSpy.at(12).at(2).value<QVector<int>>().contains(Qt::UserRole + 1));
    QVERIFY(dataChangedSpy.at(12).at(2).value<QVector<int>>().contains(Qt::UserRole + 2));
    QVERIFY(dataChangedSpy.at(12).at(2).value<QVector<int>>().contains(Qt::UserRole + 3));
    QCOMPARE(dataChangedSpy.at(13).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(13).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));


    QCOMPARE(model->hardwareParametersModule()->findParameter(model->seam()->hardwareParameters(), HardwareParameters::instance()->properties(HardwareParameters::Key::ScannerDriveToPosition).uuid) == nullptr, octWithReferenceArms);
    QCOMPARE(model->hardwareParametersModule()->findParameter(model->seam()->hardwareParameters(), HardwareParameters::instance()->properties(HardwareParameters::Key::ScannerDriveWithOCTReference).uuid) == nullptr, !octWithReferenceArms);

    QSignalSpy currentDriveWithOCTReferenceSpy{model, &ScanfieldSeamModel::currentDriveWithOCTReferenceChanged};
    QVERIFY(currentDriveWithOCTReferenceSpy.isValid());

    model->setDriveWithOCT(!octWithReferenceArms);
    QCOMPARE(currentDriveWithOCTReferenceSpy.count(), 1);

    QCOMPARE(model->hardwareParametersModule()->findParameter(model->seam()->hardwareParameters(), HardwareParameters::instance()->properties(HardwareParameters::Key::ScannerDriveToPosition).uuid) == nullptr, !octWithReferenceArms);
    QCOMPARE(model->hardwareParametersModule()->findParameter(model->seam()->hardwareParameters(), HardwareParameters::instance()->properties(HardwareParameters::Key::ScannerDriveWithOCTReference).uuid) == nullptr, octWithReferenceArms);

    model->setCameraCenter({641, 513});
    QCOMPARE(model->currentCenter(), QPointF(641, 513));
    QCOMPARE(model->cameraCenter(seam_noParams), QPointF(641, 513));
    QCOMPARE(model->cameraCenter(seam_withParams), QPointF(1, 1.5));
    // one signals - from setCameraCenter, hardwareParameters are already created
    QCOMPARE(currentCenterChangedSpy.count(), 9);
    QCOMPARE(dataChangedSpy.count(), 16);
    QCOMPARE(dataChangedSpy.at(14).at(0).toModelIndex(), model->index(0));
    QCOMPARE(dataChangedSpy.at(14).at(1).toModelIndex(), model->index(0));
    QCOMPARE(dataChangedSpy.at(14).at(2).value<QVector<int>>().size(), 4);
    QVERIFY(dataChangedSpy.at(14).at(2).value<QVector<int>>().contains(Qt::UserRole));
    QVERIFY(dataChangedSpy.at(14).at(2).value<QVector<int>>().contains(Qt::UserRole + 1));
    QVERIFY(dataChangedSpy.at(14).at(2).value<QVector<int>>().contains(Qt::UserRole + 2));
    QVERIFY(dataChangedSpy.at(14).at(2).value<QVector<int>>().contains(Qt::UserRole + 3));
    QCOMPARE(dataChangedSpy.at(15).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(15).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));

    model->resetCameraCenter();
    QCOMPARE(model->currentCenter(), QPointF(-1, -1));
    QCOMPARE(model->cameraCenter(seam_noParams), QPointF(-1, -1));
    QCOMPARE(model->cameraCenter(seam_withParams), QPointF(1, 1.5));
    QCOMPARE(currentCenterChangedSpy.count(), 10);
    QCOMPARE(dataChangedSpy.count(), 18);
    QCOMPARE(dataChangedSpy.at(16).at(0).toModelIndex(), model->index(0));
    QCOMPARE(dataChangedSpy.at(16).at(1).toModelIndex(), model->index(0));
    QCOMPARE(dataChangedSpy.at(16).at(2).value<QVector<int>>().size(), 4);
    QVERIFY(dataChangedSpy.at(16).at(2).value<QVector<int>>().contains(Qt::UserRole));
    QVERIFY(dataChangedSpy.at(16).at(2).value<QVector<int>>().contains(Qt::UserRole + 1));
    QVERIFY(dataChangedSpy.at(16).at(2).value<QVector<int>>().contains(Qt::UserRole + 2));
    QVERIFY(dataChangedSpy.at(16).at(2).value<QVector<int>>().contains(Qt::UserRole + 3));
    QCOMPARE(dataChangedSpy.at(17).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(17).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));

    QVERIFY(model->hardwareParametersModule()->findParameter(model->seam()->hardwareParameters(), HardwareParameters::instance()->properties(HardwareParameters::Key::ScannerNewXPosition).uuid) == nullptr);
    QVERIFY(model->hardwareParametersModule()->findParameter(model->seam()->hardwareParameters(), HardwareParameters::instance()->properties(HardwareParameters::Key::ScannerNewYPosition).uuid) == nullptr);
    QVERIFY(model->hardwareParametersModule()->findParameter(model->seam()->hardwareParameters(), HardwareParameters::instance()->properties(HardwareParameters::Key::ScannerDriveToPosition).uuid) == nullptr);
    QVERIFY(model->hardwareParametersModule()->findParameter(model->seam()->hardwareParameters(), HardwareParameters::instance()->properties(HardwareParameters::Key::ScannerDriveWithOCTReference).uuid) == nullptr);

    model->hardwareParametersModule()->setAttributeModel(nullptr);

    QCOMPARE(dataChangedSpy.count(), 20);
    QCOMPARE(dataChangedSpy.at(18).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(18).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));
    QCOMPARE(dataChangedSpy.at(19).at(2).value<QVector<int>>().size(), 4);
    QVERIFY(dataChangedSpy.at(19).at(2).value<QVector<int>>().contains(Qt::UserRole));
    QVERIFY(dataChangedSpy.at(19).at(2).value<QVector<int>>().contains(Qt::UserRole + 1));
    QVERIFY(dataChangedSpy.at(19).at(2).value<QVector<int>>().contains(Qt::UserRole + 2));
    QVERIFY(dataChangedSpy.at(19).at(2).value<QVector<int>>().contains(Qt::UserRole + 3));

    // no attribute model
    QCOMPARE(model->cameraCenter(seam_noParams), QPointF(-1, -1));
    QCOMPARE(model->cameraCenter(seam_withParams), QPointF(-1, -1));
}

void ScanfieldSeamModelTest::testLastValidSeam()
{
    auto model = new ScanfieldSeamModel{this};

    QVERIFY(!model->lastValidSeam());

    auto product = new Product{QUuid::createUuid(), this};
    auto series1 = product->createSeamSeries();
    QVERIFY(series1);
    model->setSeamSeries(series1);

    QVERIFY(!model->lastValidSeam());

    // setup attribute model, calibration proxy and calibration file
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

    model->hardwareParametersModule()->setAttributeModel(attributeModel);

    auto proxy = std::make_shared<precitec::interface::MockCalibrationProxy>();
    model->scanfieldModule()->setCalibrationCoordinatesRequestProxy(proxy);

    const auto& testdata = QFINDTESTDATA("testdata/scanner_data/1cd8f7cb-e57a-4c21-b05d-1b40cc2b0778/");
    model->scanfieldModule()->loadCalibrationFile(testdata);

    auto seam1 = series1->createSeam();
    QVERIFY(seam1);

    seam1->createHardwareParameters();
    auto seam1ParameterSet = seam1->hardwareParameters();

    // (700, 800) after calibration
    auto seam1xPosistion = seam1ParameterSet->createParameter(QUuid::createUuid(), xPositionAttribute, {});
    QVERIFY(seam1xPosistion);
    seam1xPosistion->setValue(7000);
    auto seam1yPosition = seam1ParameterSet->createParameter(QUuid::createUuid(), yPositionAttribute, {});
    QVERIFY(seam1yPosition);
    seam1yPosition->setValue(8000);

    QVERIFY(!model->lastValidSeam());

    // Last valid seam of seam1 is seam1
    model->selectSeam(seam1->uuid());

    QCOMPARE(model->lastValidSeam(), seam1);

    auto seam2 = series1->createSeam();
    QVERIFY(seam2);

    seam2->createHardwareParameters();
    auto seam2ParameterSet = seam2->hardwareParameters();

    // (500, 400) after calibration - not a valid center (values are less than the minimum defined by the camera and image size)
    auto seam2xPosistion = seam2ParameterSet->createParameter(QUuid::createUuid(), xPositionAttribute, {});
    QVERIFY(seam2xPosistion);
    seam2xPosistion->setValue(5000);
    auto seam2yPosition = seam2ParameterSet->createParameter(QUuid::createUuid(), yPositionAttribute, {});
    QVERIFY(seam2yPosition);
    seam2yPosition->setValue(4000);

    QCOMPARE(model->lastValidSeam(), seam1);
    QCOMPARE(model->lastValidCameraCenter(seam1), seam1);
    QCOMPARE(model->lastValidCameraCenter(seam2), seam1);

    model->selectSeam(seam2->uuid());
    QCOMPARE(model->lastValidSeam(), seam1);
    QCOMPARE(model->lastValidCameraCenter(seam1), seam1);
    QCOMPARE(model->lastValidCameraCenter(seam2), seam1);

    auto seam3 = series1->createSeam();
    QVERIFY(seam3);

    seam3->createHardwareParameters();
    auto seam3ParameterSet = seam3->hardwareParameters();

    // (850, 800) after calibration
    auto seam3xPosistion = seam3ParameterSet->createParameter(QUuid::createUuid(), xPositionAttribute, {});
    QVERIFY(seam3xPosistion);
    seam3xPosistion->setValue(8500);
    auto seam3yPosition = seam3ParameterSet->createParameter(QUuid::createUuid(), yPositionAttribute, {});
    QVERIFY(seam3yPosition);
    seam3yPosition->setValue(8000);

    QCOMPARE(model->lastValidSeam(), seam1);
    QCOMPARE(model->lastValidCameraCenter(seam1), seam1);
    QCOMPARE(model->lastValidCameraCenter(seam2), seam1);
    QCOMPARE(model->lastValidCameraCenter(seam3), seam3);

    model->selectSeam(seam3->uuid());
    QCOMPARE(model->lastValidSeam(), seam3);
    QCOMPARE(model->lastValidCameraCenter(seam1), seam1);
    QCOMPARE(model->lastValidCameraCenter(seam2), seam1);
    QCOMPARE(model->lastValidCameraCenter(seam3), seam3);

    auto seam4 = series1->createSeam();
    QVERIFY(seam4);

    seam4->createHardwareParameters();
    auto seam4ParameterSet = seam4->hardwareParameters();

    // (0, 0) after calibration - not a valid center (values are less than the minimum defined by the camera and image size)
    auto seam4xPosistion = seam4ParameterSet->createParameter(QUuid::createUuid(), xPositionAttribute, {});
    QVERIFY(seam4xPosistion);
    seam4xPosistion->setValue(0);
    auto seam4yPosition = seam4ParameterSet->createParameter(QUuid::createUuid(), yPositionAttribute, {});
    QVERIFY(seam4yPosition);
    seam4yPosition->setValue(0);

    QCOMPARE(model->lastValidSeam(), seam3);
    QCOMPARE(model->lastValidCameraCenter(seam1), seam1);
    QCOMPARE(model->lastValidCameraCenter(seam2), seam1);
    QCOMPARE(model->lastValidCameraCenter(seam3), seam3);
    QCOMPARE(model->lastValidCameraCenter(seam4), seam3);

    model->selectSeam(seam4->uuid());
    QCOMPARE(model->lastValidSeam(), seam3);
    QCOMPARE(model->lastValidCameraCenter(seam1), seam1);
    QCOMPARE(model->lastValidCameraCenter(seam2), seam1);
    QCOMPARE(model->lastValidCameraCenter(seam3), seam3);
    QCOMPARE(model->lastValidCameraCenter(seam4), seam3);

    series1->destroySeam(seam3);

    QCOMPARE(model->lastValidSeam(), seam1);
    QCOMPARE(model->lastValidCameraCenter(seam1), seam1);
    QCOMPARE(model->lastValidCameraCenter(seam2), seam1);
    QCOMPARE(model->lastValidCameraCenter(seam4), seam1);

    series1->destroySeam(seam1);

    QVERIFY(!model->lastValidSeam());
    QVERIFY(!model->lastValidCameraCenter(seam2));
    QVERIFY(!model->lastValidCameraCenter(seam4));
}

void ScanfieldSeamModelTest::testTransformation()
{
    auto model = new ScanfieldSeamModel{this};

    QVERIFY(model->transformation().isIdentity());

    QSignalSpy transformationChangedSpy{model, &ScanfieldSeamModel::transformationChanged};
    QVERIFY(transformationChangedSpy.isValid());

    QSignalSpy dataChangedSpy{model, &ScanfieldSeamModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    model->setTransformation({});
    QCOMPARE(transformationChangedSpy.count(), 0);

    QMatrix4x4 transformation;
    transformation.scale(2);

    model->setTransformation(transformation);
    QCOMPARE(model->transformation(), transformation);
    QCOMPARE(transformationChangedSpy.count(), 1);

    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.at(0).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(0).at(2).value<QVector<int>>().contains(Qt::UserRole + 3));

    model->setTransformation(transformation);
    QCOMPARE(transformationChangedSpy.count(), 1);

    QMatrix4x4 transformation2;
    transformation2.scale(5);

    model->setTransformation(transformation2);
    QCOMPARE(model->transformation(), transformation2);
    QCOMPARE(transformationChangedSpy.count(), 2);

    QCOMPARE(dataChangedSpy.count(), 2);
    QCOMPARE(dataChangedSpy.at(1).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(1).at(2).value<QVector<int>>().contains(Qt::UserRole + 3));
}

void ScanfieldSeamModelTest::testShowAllSeams()
{
    auto model = new ScanfieldSeamModel{this};

    QVERIFY(model->showAllSeams());

    QSignalSpy showAllSeamsChangedSpy{model, &ScanfieldSeamModel::showAllSeamsChanged};
    QVERIFY(showAllSeamsChangedSpy.isValid());

    QSignalSpy dataChangedSpy{model, &ScanfieldSeamModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    model->setShowAllSeams(true);
    QCOMPARE(showAllSeamsChangedSpy.count(), 0);

    model->setShowAllSeams(false);
    QVERIFY(!model->showAllSeams());
    QCOMPARE(showAllSeamsChangedSpy.count(), 1);

    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.at(0).at(2).value<QVector<int>>().size(), 1);
    QVERIFY(dataChangedSpy.at(0).at(2).value<QVector<int>>().contains(Qt::UserRole + 5));

    model->setShowAllSeams(false);
    QCOMPARE(showAllSeamsChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.count(), 1);
}

void ScanfieldSeamModelTest::testPointInSeam()
{
    auto model = new ScanfieldSeamModel{this};

    QVERIFY(!model->pointInSeam(nullptr, {500, 700}));

    auto product = new Product{QUuid::createUuid(), this};
    auto series1 = product->createSeamSeries();
    QVERIFY(series1);
    model->setSeamSeries(series1);

    QMatrix4x4 transformation;
    transformation.scale(2);

    model->setTransformation(transformation);

    QVERIFY(!model->pointInSeam(nullptr, {500, 700}));

    // setup attribute model, calibration proxy and calibration file
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

    model->hardwareParametersModule()->setAttributeModel(attributeModel);

    auto proxy = std::make_shared<precitec::interface::MockCalibrationProxy>();
    model->scanfieldModule()->setCalibrationCoordinatesRequestProxy(proxy);

    const auto& testdata = QFINDTESTDATA("testdata/scanner_data/1cd8f7cb-e57a-4c21-b05d-1b40cc2b0778/");
    model->scanfieldModule()->loadCalibrationFile(testdata);

    auto seam1 = series1->createSeam();
    QVERIFY(seam1);

    seam1->createHardwareParameters();
    auto seam1ParameterSet = seam1->hardwareParameters();

    // (700, 800) after calibration
    // Camera Rect in Image: (120,576 2560x2048)
    auto seam1xPosistion = seam1ParameterSet->createParameter(QUuid::createUuid(), xPositionAttribute, {});
    QVERIFY(seam1xPosistion);
    seam1xPosistion->setValue(7000);
    auto seam1yPosition = seam1ParameterSet->createParameter(QUuid::createUuid(), yPositionAttribute, {});
    QVERIFY(seam1yPosition);
    seam1yPosition->setValue(8000);

    QVERIFY(model->pointInSeam(seam1, {500, 700}));

    auto seam2 = series1->createSeam();
    QVERIFY(seam2);

    seam2->createHardwareParameters();
    auto seam2ParameterSet = seam2->hardwareParameters();

    // (500, 400) after calibration - not a valid center (values are less than the minimum defined by the camera and image size)
    // Camera Rect in Image: (0,0 2560x2048)
    auto seam2xPosistion = seam2ParameterSet->createParameter(QUuid::createUuid(), xPositionAttribute, {});
    QVERIFY(seam2xPosistion);
    seam2xPosistion->setValue(5000);
    auto seam2yPosition = seam2ParameterSet->createParameter(QUuid::createUuid(), yPositionAttribute, {});
    QVERIFY(seam2yPosition);
    seam2yPosition->setValue(4000);

    QVERIFY(model->pointInSeam(seam1, {500, 700}));
    QVERIFY(!model->pointInSeam(seam2, {500, 700}));

    auto seam3 = series1->createSeam();
    QVERIFY(seam3);

    seam3->createHardwareParameters();
    auto seam3ParameterSet = seam3->hardwareParameters();

    // (850, 800) after calibration
    // Camera Rect in Image: (420,576 2560x2048)
    auto seam3xPosistion = seam3ParameterSet->createParameter(QUuid::createUuid(), xPositionAttribute, {});
    QVERIFY(seam3xPosistion);
    seam3xPosistion->setValue(8500);
    auto seam3yPosition = seam3ParameterSet->createParameter(QUuid::createUuid(), yPositionAttribute, {});
    QVERIFY(seam3yPosition);
    seam3yPosition->setValue(8000);

    QVERIFY(model->pointInSeam(seam1, {500, 700}));
    QVERIFY(!model->pointInSeam(seam2, {500, 700}));
    QVERIFY(model->pointInSeam(seam3, {500, 700}));

    auto seam4 = series1->createSeam();
    QVERIFY(seam4);

    seam4->createHardwareParameters();
    auto seam4ParameterSet = seam4->hardwareParameters();

    // (1500, 1200) after calibration
    // Camera Rect in Image: (1720,1376 2560x2048) - does not contain point
    auto seam4xPosistion = seam4ParameterSet->createParameter(QUuid::createUuid(), xPositionAttribute, {});
    QVERIFY(seam4xPosistion);
    seam4xPosistion->setValue(15000);
    auto seam4yPosition = seam4ParameterSet->createParameter(QUuid::createUuid(), yPositionAttribute, {});
    QVERIFY(seam4yPosition);
    seam4yPosition->setValue(12000);

    QVERIFY(model->pointInSeam(seam1, {500, 700}));
    QVERIFY(!model->pointInSeam(seam2, {500, 700}));
    QVERIFY(model->pointInSeam(seam3, {500, 700}));
    QVERIFY(!model->pointInSeam(seam4, {500, 700}));

    QVERIFY(!model->pointInCurrentSeam({500, 700}));

    model->selectSeam(seam1->uuid());
    QVERIFY(model->pointInCurrentSeam({500, 700}));

    model->selectSeam(seam2->uuid());
    QVERIFY(!model->pointInCurrentSeam({500, 700}));

    model->selectSeam(seam3->uuid());
    QVERIFY(model->pointInCurrentSeam({500, 700}));

    model->selectSeam(seam4->uuid());
    QVERIFY(!model->pointInCurrentSeam({500, 700}));

    model->setTransformation({});

    QVERIFY(!model->pointInSeam(seam1, {500, 700}));
    QVERIFY(!model->pointInSeam(seam2, {500, 700}));
    QVERIFY(!model->pointInSeam(seam3, {500, 700}));
    QVERIFY(!model->pointInSeam(seam4, {500, 700}));
}

QTEST_GUILESS_MAIN(ScanfieldSeamModelTest)
#include "scanfieldSeamModelTest.moc"
