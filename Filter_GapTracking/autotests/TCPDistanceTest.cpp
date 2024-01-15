#include <QTest>

#include "../TCPDistance.h"

#include <fliplib/NullSourceFilter.h>
#include <fliplib/SinkFilter.h>
#include <filter/sensorFilterInterface.h>
#include <util/calibDataSingleton.h>
#include <Calibration3DCoordsLoader.h>
#include <overlay/overlayCanvas.h>

#include <iomanip>

using precitec::filter::TCPDistance;

using namespace precitec::interface;
using precitec::math::Calibration3DCoords;
using precitec::math::CalibrationParamMap;
using precitec::coordinates::CalibrationCameraCorrectionContainer;
using precitec::system::CalibDataSingleton;

class TCPDistanceTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testProceed_data();
    void testProceed();
};


struct DummyInput
{

    fliplib::NullSourceFilter m_sourceFilter;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> m_pipeInX{ &m_sourceFilter, "X"};
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> m_pipeInY{ &m_sourceFilter, "Y"};
    precitec::interface::GeoDoublearray m_inputDataX;
    precitec::interface::GeoDoublearray m_inputDataY;

    struct InputData
    {
        double value = 0.0;
        int rank = 255;
    };

    DummyInput()
    {
        m_pipeInX.setTag("position_x");
        m_pipeInY.setTag("position_y");
    }

    bool connectToFilter(fliplib::BaseFilter * pFilter)
    {
        int group =1;
        //connect  pipes
        bool ok = pFilter->connectPipe(&(m_pipeInX), group);
        ok &= pFilter->connectPipe(&(m_pipeInY), group);
        return ok;
    }

    precitec::interface::GeoDoublearray createGeoDoubleArray(precitec::interface::ImageContext baseContext, const InputData & inputData, int trafoX, int trafoY )
    {
        using namespace precitec::interface;
        return GeoDoublearray( ImageContext{baseContext, SmpTrafo{new LinearTrafo{ trafoX, trafoY }}},
                                   precitec::geo2d::Doublearray{1, inputData.value, inputData.rank},
                                   ResultType::AnalysisOK, Limit);
    }

    void fillData( int imageNumber, InputData inputDataX, InputData inputDataY, int trafoX, int trafoY, int hwroi_x, int hwroi_y, ScannerContextInfo scannerInfo)
    {
        using namespace precitec::interface;
        using namespace precitec::geo2d;

        ImageContext context;
        context.setImageNumber(imageNumber);
        context.HW_ROI_x0 = hwroi_x;
        context.HW_ROI_y0 = hwroi_y;
        context.m_ScannerInfo = scannerInfo;

        m_inputDataX = createGeoDoubleArray(context, inputDataX, trafoX, trafoY);
        m_inputDataY = createGeoDoubleArray(context, inputDataY, trafoX, trafoY);

    }
    void signal()
    {
        m_pipeInX.signal(m_inputDataX);
        m_pipeInY.signal(m_inputDataY);
    }
};


class DummyFilter : public fliplib::BaseFilter
{
public:
    DummyFilter() : fliplib::BaseFilter("dummy") {}
    void proceed(const void *sender, fliplib::PipeEventArgs &e) override
    {
        Q_UNUSED(sender)
        Q_UNUSED(e)
        preSignalAction();
        m_proceedCalled = true;
    }
    void proceedGroup(const void * sender, fliplib::PipeGroupEventArgs & e) override
    {
        Q_UNUSED(sender)
        Q_UNUSED(e)
        preSignalAction();
        m_proceedCalled = true;
    }
    int getFilterType() const override
    {
        return BaseFilterInterface::SINK;
    }

    bool isProceedCalled() const
    {
        return m_proceedCalled;
    }

    void resetProceedCalled()
    {
        m_proceedCalled = false;
    }


private:
    bool m_proceedCalled = false;
};

Q_DECLARE_METATYPE(DummyInput::InputData);
Q_DECLARE_METATYPE(Calibration3DCoords);
Q_DECLARE_METATYPE(CalibrationParamMap);
Q_DECLARE_METATYPE(CalibrationCameraCorrectionContainer);
Q_DECLARE_METATYPE(ScannerContextInfo);


void TCPDistanceTest::testProceed_data()
{
    QTest::addColumn<bool>("useCoaxCalibration");
    QTest::addColumn<bool>("useCorrection"); // TCP corrrection according to scanner position
    QTest::addColumn<Calibration3DCoords>("calib3DCoords");
    QTest::addColumn<CalibrationParamMap>("calibParams");
    QTest::addColumn<CalibrationCameraCorrectionContainer>("calibrationCameraCorrectionContainer");
    QTest::addColumn<int>("parameter_MeasurementType");
    QTest::addColumn<double>("input_positionX");
    QTest::addColumn<double>("input_positionY");
    QTest::addColumn<int>("input_trafoX");
    QTest::addColumn<int>("input_trafoY");
    QTest::addColumn<int>("HWROI_X");
    QTest::addColumn<int>("HWROI_Y");
    QTest::addColumn<ScannerContextInfo>("scannerInfo");
    QTest::addColumn<double>("expected_distanceX");
    QTest::addColumn<double>("expected_distanceY");


    ScannerContextInfo noScanner;
    CalibrationCameraCorrectionContainer emptyCorrectionContainer;

    // coax calibration
    {
        std::string testdatafolder = QFINDTESTDATA("../../Analyzer_Interface/autotests/testdata/coax/").toStdString();
        std::string calibConfigFilename = testdatafolder + "/config/calibrationData0.xml";

        Calibration3DCoords coords;
        CalibrationParamMap params;

        bool ok = params.readParamMapFromFile(calibConfigFilename);
        QVERIFY(ok && params.size() > 0);

        bool useOrientedLine = false;
        ok = precitec::math::loadCoaxModel(coords, {params}, useOrientedLine);
        QVERIFY(ok);
        QVERIFY(!coords.isScheimpflugCase());


        precitec::geo2d::DPoint inputPositionPixel{200,300};
        const std::map<int, precitec::geo2d::DPoint > expectedMeasurements
        {
            {0, {-0.90000057220458984375 , -1.9775714874267578125}},  // linelaser1
            {1, {-0.90000057220458984375 , 5.5285315513610839844}} ,  // linelaser2
            {2, {-0.90000057220458984375 , 3.2224001884460449219}}, // linelaser3
            {3, {-0.90000057220458984375 , -1.6875}}, //2D distance
            {4, {-0.90000057220458984375 , -1.6875}} // diistance to scanner center
        };


        for (auto & entry : expectedMeasurements)
        {
            auto & measurementType = entry.first;
            auto & expectedDistance_mm = entry.second;

            std::string testName = "Measurement_" + std::to_string(measurementType) + "_coax";
            QTest::newRow(testName.c_str())   << true << false << coords << params << emptyCorrectionContainer
                                        << measurementType
                                        << inputPositionPixel.x << inputPositionPixel.y
                                        << 10 << 20
                                        << 30 << 40
                                        << noScanner
                                        << expectedDistance_mm.x << expectedDistance_mm.y;
        }

        ScannerContextInfo scannerAtOrigin{true, 0.0, 0.0};

        for (auto & entry : expectedMeasurements)
        {
            auto & measurementType = entry.first;
            auto & expectedDistance_mm = entry.second;

            std::string testName = "Measurement_" + std::to_string(measurementType) + "_scanmaster_origin";
            QTest::newRow(testName.c_str())   << true << false << coords << params << emptyCorrectionContainer
                                        << measurementType
                                        << inputPositionPixel.x << inputPositionPixel.y
                                        << 10 << 20
                                        << 30 << 40
                                        << scannerAtOrigin
                                        << expectedDistance_mm.x << expectedDistance_mm.y;
        }

        {
            ScannerContextInfo scannerPosition{true, 10.5, -2.3};

            std::map<int, precitec::geo2d::DPoint > expectedMeasurementsWithScannerInfo = expectedMeasurements;
            expectedMeasurementsWithScannerInfo[4].x += scannerPosition.m_x;
            expectedMeasurementsWithScannerInfo[4].y += scannerPosition.m_y;

            for (auto & entry : expectedMeasurementsWithScannerInfo)
            {
                auto & measurementType = entry.first;
                auto & expectedDistance_mm = entry.second;

                std::string testName = "Measurement_" + std::to_string(measurementType) + "_scanmaster_nocorrection";

                QTest::newRow(testName.c_str())   << true << false << coords << params << emptyCorrectionContainer
                                            << measurementType
                                            << inputPositionPixel.x << inputPositionPixel.y
                                            << 10 << 20
                                            << 30 << 40
                                            << scannerPosition
                                            << expectedDistance_mm.x << expectedDistance_mm.y;


            }

            std::vector<std::pair<precitec::geo2d::DPoint, precitec::coordinates::CalibrationCameraCorrection>>offsets{
                {{scannerPosition.m_x, scannerPosition.m_y},  {2.0, 4.0}},
                {{  0.0, 0.0},    {0.0, 0.0}}};
            const CalibrationCameraCorrectionContainer correctionContainer(offsets);

            auto tcpOffset = correctionContainer.get( scannerPosition.m_x, scannerPosition.m_y);

            for (auto & entry : expectedMeasurementsWithScannerInfo)
            {
                auto & measurementType = entry.first;
                auto & expectedDistance_mm = entry.second;

                std::string testName = "Measurement_" + std::to_string(measurementType) + "_scanmaster_compensatecorrection";

                QTest::newRow(testName.c_str())   << true << true << coords << params << correctionContainer
                                            << measurementType
                                            << inputPositionPixel.x + tcpOffset.m_oTCPXOffset << inputPositionPixel.y + tcpOffset.m_oTCPYOffset
                                            << 10 << 20
                                            << 30 << 40
                                            << scannerPosition
                                            << expectedDistance_mm.x << expectedDistance_mm.y;
            }

        }
    }


    {

            CalibrationParamMap params;
            params.setDouble("xtcp", 250.5);
            params.setDouble("ytcp", 300);
            params.setDouble("ytcp_2", 400);
            params.setDouble("ytcp_tcp", 500);
            Calibration3DCoords coords;

            std::string testdatafolder = QFINDTESTDATA("../../Analyzer_Interface/autotests/testdata/scheimpflug/").toStdString();
            precitec::system::CamGridData oCamGridData;
            std::string p_oCamGridImageFilename = testdatafolder + "/config/calibImgData0fallback.csv";
            std::string oMsgError = oCamGridData.loadFromCSV(p_oCamGridImageFilename);
            QVERIFY(oMsgError.empty());
            bool ok = loadCamGridData(coords, oCamGridData);
            QVERIFY(ok);

            precitec::geo2d::DPoint inputPositionPixel{200,300};
            std::map<int, precitec::geo2d::DPoint > expectedMeasurements
            {
                {0, {-0.31309556961059570312, -1.4371249675750732422}},// linelaser1
                {1, {-0.17890167236328125, 0.97834467887878417969}} ,  // linelaser2 (not meaningful in scheimpflug case)
                {2, {-0.038227558135986328125, 3.5137185752391815186}},// linelaser3 (not meaningful in scheimpflug case)
                {3, {-0.31309556961059570312, 3.2286558151245117188}}, // 2D distance (not meaningful in scheimpflug case)
                {4, {-0.31309556961059570312, 3.2286558151245117188}}  // distance to scanner center (not meaningful in scheimpflug case)
            };

            for (auto & entry : expectedMeasurements)
            {
                auto & measurementType = entry.first;
                auto & expectedDistance_mm = entry.second;

                std::string testName = "Measurement_" + std::to_string(measurementType) + "_scheimpflug";
                QTest::newRow(testName.c_str())   << false << false << coords << params << emptyCorrectionContainer
                                            << measurementType
                                            << inputPositionPixel.x << inputPositionPixel.y
                                            << 10 << 20
                                            << 30 << 40
                                            << noScanner
                                            << expectedDistance_mm.x << expectedDistance_mm.y;
            }
    }

}


void TCPDistanceTest::testProceed()
{
    // initialize calibration
    QFETCH(bool, useCoaxCalibration);
    QFETCH(bool, useCorrection);
    {
        QFETCH(Calibration3DCoords, calib3DCoords);
        QFETCH(CalibrationParamMap, calibParams);
        QFETCH(CalibrationCameraCorrectionContainer, calibrationCameraCorrectionContainer);

        auto &calibData = CalibDataSingleton::getCalibrationDataReference(precitec::math::SensorId::eSensorId0);
        calibData.resetConfig();
        calibData.reload(calib3DCoords, calibParams);
        QCOMPARE(calibData.getCalibrationCoords().isScheimpflugCase(), !useCoaxCalibration);

        if (useCorrection)
        {
            calibData.setCalibrationCorrectionContainer(calibrationCameraCorrectionContainer);
        }

    }

    QCOMPARE(CalibDataSingleton::getCalibrationCoords(precitec::math::SensorId::eSensorId0).isScheimpflugCase(), !useCoaxCalibration);
    QCOMPARE(CalibDataSingleton::getCalibrationData(precitec::math::SensorId::eSensorId0).hasCameraCorrectionGrid(), useCorrection);

    // initialize filter and connectors

    TCPDistance filter;
    std::unique_ptr<precitec::image::OverlayCanvas> pcanvas {new precitec::image::OverlayCanvas};
    filter.setCanvas(pcanvas.get());

    auto outPipeX = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray>*>(filter.findPipe("xko"));
    auto outPipeY = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray>*>(filter.findPipe("yko"));
    QVERIFY(outPipeX);
    QVERIFY(outPipeY);

    DummyInput dummyInput;
    QVERIFY(dummyInput.connectToFilter(&filter));

    DummyFilter dummyFilter;
    QVERIFY(dummyFilter.connectPipe(outPipeX, 1));
    QVERIFY(dummyFilter.connectPipe(outPipeY, 1));

    // parametrize filter

    QFETCH(int, parameter_MeasurementType);
    filter.getParameters().update(std::string("TypeOfLaserLine"), fliplib::Parameter::TYPE_int, parameter_MeasurementType);
    filter.setParameter();

    // provide input and check output

    QFETCH(double, input_positionX);
    QFETCH(double, input_positionY);

    QFETCH(int, HWROI_X);
    QFETCH(int, HWROI_Y);
    QFETCH(int, input_trafoX);
    QFETCH(int, input_trafoY);
    QFETCH(ScannerContextInfo, scannerInfo);

    int imageNumber = 0;
    if (scannerInfo.m_hasPosition)
    {
        CalibDataSingleton::getCalibrationDataReference(precitec::math::SensorId::eSensorId0).updateCameraCorrection(scannerInfo.m_x, scannerInfo.m_y, (imageNumber % g_oNbPar));
    }

    dummyInput.fillData(imageNumber, DummyInput::InputData{input_positionX}, DummyInput::InputData{input_positionY}, HWROI_X, HWROI_Y, input_trafoX, input_trafoY, scannerInfo);
    QCOMPARE(dummyFilter.isProceedCalled(), false);
    dummyInput.signal();
    QCOMPARE(dummyFilter.isProceedCalled(), true);
    dummyFilter.resetProceedCalled();
    auto resultX = outPipeX->read(imageNumber);
    auto resultY = outPipeY->read(imageNumber);
    std::cout <<  "Results" << QTest::currentDataTag() <<  std::setprecision (20) << " [mm] = " << resultX.ref().getData().front() << ", " << resultY.ref().getData().front() << std::endl;
    QCOMPARE(resultX.ref().size(), 1ul);
    QTEST(resultX.ref().getData().front(), "expected_distanceX");
    QCOMPARE(resultY.ref().size(), 1ul);
    QTEST(resultY.ref().getData().front(), "expected_distanceY");


    imageNumber++;
    if (scannerInfo.m_hasPosition)
    {
        CalibDataSingleton::getCalibrationDataReference(precitec::math::SensorId::eSensorId0).updateCameraCorrection(scannerInfo.m_x, scannerInfo.m_y, (imageNumber % g_oNbPar));
    }
    dummyInput.fillData(imageNumber, DummyInput::InputData{input_positionX + input_trafoX}, DummyInput::InputData{input_positionY+input_trafoY}, HWROI_X, HWROI_Y, 0, 0, scannerInfo);
    QCOMPARE(dummyFilter.isProceedCalled(), false);
    dummyInput.signal();
    QCOMPARE(dummyFilter.isProceedCalled(), true);
    dummyFilter.resetProceedCalled();
    resultX = outPipeX->read(imageNumber);
    QCOMPARE(resultX.ref().size(), 1ul);
    QTEST(resultX.ref().getData().front(), "expected_distanceX");
    resultY = outPipeY->read(imageNumber);
    QCOMPARE(resultY.ref().size(), 1ul);
    QTEST(resultY.ref().getData().front(), "expected_distanceY");

    imageNumber++;
    if (scannerInfo.m_hasPosition)
    {
        CalibDataSingleton::getCalibrationDataReference(precitec::math::SensorId::eSensorId0).updateCameraCorrection(scannerInfo.m_x, scannerInfo.m_y, (imageNumber % g_oNbPar));
    }
    dummyInput.fillData(imageNumber, DummyInput::InputData{0.0}, DummyInput::InputData{0.0} , HWROI_X, HWROI_Y, input_positionX + input_trafoX, input_positionY + input_trafoY, scannerInfo);
    QCOMPARE(dummyFilter.isProceedCalled(), false);
    dummyInput.signal();
    QCOMPARE(dummyFilter.isProceedCalled(), true);
    dummyFilter.resetProceedCalled();
    resultX = outPipeX->read(imageNumber);
    QCOMPARE(resultX.ref().size(), 1ul);
    QTEST(resultX.ref().getData().front(), "expected_distanceX");
    resultY = outPipeY->read(imageNumber);
    QCOMPARE(resultY.ref().size(), 1ul);
    QTEST(resultY.ref().getData().front(), "expected_distanceY");

    imageNumber++;
    if (scannerInfo.m_hasPosition)
    {
        CalibDataSingleton::getCalibrationDataReference(precitec::math::SensorId::eSensorId0).updateCameraCorrection(scannerInfo.m_x, scannerInfo.m_y, (imageNumber % g_oNbPar));
    }
    dummyInput.fillData(imageNumber, DummyInput::InputData{input_positionX + HWROI_X}, DummyInput::InputData{input_positionY+HWROI_Y}, 0, 0, input_trafoX, input_trafoY, scannerInfo);
    QCOMPARE(dummyFilter.isProceedCalled(), false);
    dummyInput.signal();
    QCOMPARE(dummyFilter.isProceedCalled(), true);
    dummyFilter.resetProceedCalled();
    resultX = outPipeX->read(imageNumber);
    QCOMPARE(resultX.ref().size(), 1ul);
    QTEST(resultX.ref().getData().front(), "expected_distanceX");
    resultY = outPipeY->read(imageNumber);
    QCOMPARE(resultY.ref().size(), 1ul);
    QTEST(resultY.ref().getData().front(), "expected_distanceY");
}

QTEST_MAIN(TCPDistanceTest)
#include "TCPDistanceTest.moc"
