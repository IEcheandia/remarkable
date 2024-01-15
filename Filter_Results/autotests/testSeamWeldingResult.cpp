#include <QTest>

#include "../seamWeldingResult.h"
#include "DummyResultHandler.h"
#include <fliplib/NullSourceFilter.h>
#include <util/calibDataSingleton.h>
#include "math/Calibration3DCoordsLoader.h"

#include <fliplib/BaseFilter.h>
#include <overlay/overlayCanvas.h>

using precitec::interface::ResultType;
using precitec::geo2d::TArray;
using precitec::geo2d::Doublearray;
using precitec::interface::ResultDoubleArray;
using precitec::filter::SeamWeldingResult;

typedef std::vector<precitec::geo2d::DPoint> point_vector_t;
Q_DECLARE_METATYPE(point_vector_t)

class TestSeamWeldingResult : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init();
    void testCtor();
    void testProceed_data();
    void testProceed();
};

void TestSeamWeldingResult::init()
{
    precitec::math::Calibration3DCoords coords;
    precitec::math::loadCoaxModel(coords, precitec::math::CoaxCalibrationData{}, false);
    QCOMPARE(coords.isScheimpflugCase(), false);

    precitec::math::CalibrationParamMap params;
    params.setDouble("xtcp", 20.0);
    params.setDouble("ytcp", 30.0);
    //check that the correct ytcp is being used
    params.setDouble("ytcp_2", 200.0);
    params.setDouble("ytcp_tcp", 300.0);

    auto &calibData = precitec::system::CalibDataSingleton::getCalibrationDataReference(precitec::math::SensorId::eSensorId0);
    calibData.reload(coords, params);
    QVERIFY(calibData.hasData());

    float expected_x, expected_y;
    {
        coords.completeInitialization(precitec::math::SensorModel::eLinearMagnification);
        bool valid = coords.getCoordinates(expected_x,expected_y,0,0);
        QVERIFY(valid);
    }


    auto & rCoords3D = precitec::system::CalibDataSingleton::getCalibrationCoords(precitec::math::SensorId::eSensorId0);
    float x,y;
    bool valid = rCoords3D.getCoordinates(x,y,0,0);
    QCOMPARE(x, expected_x);
    QCOMPARE(y, expected_y);
    QVERIFY(valid);

    std::vector<std::pair<precitec::geo2d::DPoint, precitec::coordinates::CalibrationCameraCorrection>> offsetsInit  =
            {
              {{-20.0, -20.0},  {2.8, -3.9}},
              {{  0.0, -20.0},  {2.0, -6.0} },
              {{ 20.0, -20.0},  {2.0, -8.0} },
              {{-20.0, 0.0},    {-0.4, 2.4}},
              {{  0.0, 0.0},    {0.0, 0.0} },
              {{ 20.0, 0.0},    {-0.2,-2.4} },
              {{-20.0, 20.0},   {-2.7, 6.9}},
              {{  0.0, 20.0},   {-2.2, 4.1} },
              {{ 20.0, 20.0},   {-1.7, 2.1} },
            };
    calibData.setCalibrationCorrectionContainer(precitec::coordinates::CalibrationCameraCorrectionContainer(offsetsInit));
}

void TestSeamWeldingResult::testCtor()
{
    SeamWeldingResult filter;
    QCOMPARE(filter.name(), std::string("SeamWeldingResult"));
    QVERIFY(filter.findPipe("SeamWeldingResultOutput") != nullptr);
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);
}

struct DummyInput
{
    fliplib::NullSourceFilter sourceFilter;
    fliplib::SynchronePipe<precitec::interface::GeoVecAnnotatedDPointarray> pipeInContour{ &sourceFilter, "contour"};
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeInX{ &sourceFilter, "absolute_position_x"};
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeInY{ &sourceFilter, "absolute_position_y"};

    DummyInput()
    {
        pipeInContour.setTag("contour");
        pipeInX.setTag("absolute_position_x");
        pipeInY.setTag("absolute_position_y");
    }

    bool connectToFilter(fliplib::BaseFilter * pFilter, int group)
    {
        //connect  pipes
        bool ok = pFilter->connectPipe(&(pipeInContour), group);
        ok &= pFilter->connectPipe(&(pipeInX), group);
        ok &= pFilter->connectPipe(&(pipeInY), group);
        return ok;
    }


    void fillDataAndSignal(int imageNumber, double x_mm, double y_mm, point_vector_t points, int rankPoints, int rankY)
    {
        using namespace precitec::interface;

        ImageContext context;
        context.setImageNumber(imageNumber);
        context.m_ScannerInfo = ScannerContextInfo{true, x_mm, y_mm};

        precitec::geo2d::AnnotatedDPointarray oPointArray;
        oPointArray.getData() = points;
        oPointArray.getRank() = std::vector<int> (points.size(), rankPoints);
        auto & rPower = oPointArray.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPower);
        rPower.reserve(points.size());
        for (unsigned int i = 0; i < points.size(); i++)
        {
            rPower.push_back(0.01*i);
        }
        auto & rRingPower = oPointArray.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPowerRing);
        rRingPower.reserve(points.size());
        for (unsigned int i = 0; i < points.size(); i++)
        {
            rRingPower.push_back(0.02*i);
        }
        auto &rVelocity = oPointArray.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserVelocity);
        rVelocity.reserve(points.size());
        for (unsigned int i = 0; i < points.size(); i++)
        {
            rVelocity.push_back(0.05*i);
        }

        auto geoContourIn = GeoVecAnnotatedDPointarray {context, {oPointArray}, ResultType::AnalysisOK, Limit };
        auto geoXIn = GeoDoublearray {
            context,
            Doublearray(1,(x_mm *1000),255),
            ResultType::AnalysisOK, Limit
        };
        auto geoYIn = GeoDoublearray {
            context,
            Doublearray(1,(y_mm *1000), rankY),
            ResultType::AnalysisOK, Limit
        };
        pipeInX.signal( geoXIn);
        pipeInY.signal( geoYIn);
        pipeInContour.signal(geoContourIn);
    }
};


void TestSeamWeldingResult::testProceed_data()
{
    const int rankMax = precitec::filter::eRankMax;
    const int rankMin = precitec::filter::eRankMin;

    QTest::addColumn<int>("parameterInputContourType");
    QTest::addColumn<int>("parameterResult");
    QTest::addColumn<double>("scannerPositionX_mm");
    QTest::addColumn<double>("scannerPositionY_mm");
    QTest::addColumn<int>("scannerPositionY_rank");
    QTest::addColumn<point_vector_t>("input_points_distance_from_tcp"); //pix or mm depending on parameterInputContourType
    QTest::addColumn<int>("rankPoints");
    QTest::addColumn<point_vector_t>("expected_points_mm");
    QTest::addColumn<bool>("expectedResultValidity"); //checked in ResultsServer
    QTest::addColumn<int>("expectedFirstRank"); //checked in ResultsServer

    const int parameterResultWelding = 0;
    const int parameterResultMove = 1;
    const int parameterResultPreview = 2;

    double scannerPositionX_mm = 1.3;
    double scannerPositionY_mm = 1.9;

    QTest::newRow("singlePointContour_pix") << 0 << parameterResultWelding << scannerPositionX_mm << scannerPositionY_mm << 255 << point_vector_t{{0.0, 0.0}} << rankMax << point_vector_t{{1.3, 1.9}} << true << rankMax;
    QTest::newRow("emptyContour") << 0 << parameterResultWelding << scannerPositionX_mm << scannerPositionY_mm  << 255 << point_vector_t{} << rankMax << point_vector_t{} << true << rankMin;
    QTest::newRow("notValidContour") << 0 << parameterResultWelding << scannerPositionX_mm << scannerPositionY_mm  << 255 << point_vector_t{{0.0, 0.0}} << rankMin << point_vector_t{{0.0, 0.0}} << true << rankMin;
    QTest::newRow("threePointContour_mm") << 1 <<parameterResultWelding << scannerPositionX_mm << scannerPositionY_mm  << 255 << point_vector_t{{0.0, 0.0},{10.0, 0.0}, {10.0,-10.0}} << rankMax << point_vector_t{{1.3, 1.9},{11.3, 1.9},{11.3, 1.9-10.0}} << true << rankMax;
    QTest::newRow("threePointContour_invalid") << 1 << parameterResultWelding << scannerPositionX_mm << scannerPositionY_mm << 0 << point_vector_t{{0.0, 0.0},{10.0, 0.0}, {10.0,-10.0}} << rankMax << point_vector_t{{0.0, 0.0}} << false << rankMin;
    QTest::newRow("threePointContour_outoflimits") << 1 << parameterResultWelding << scannerPositionX_mm << scannerPositionY_mm << 0 << point_vector_t{{0.0, 0.0},{10.0, 0.0}, {200.0,-10.0}} << rankMax << point_vector_t{{0.0, 0.0}} << false << rankMin;


    QTest::newRow("singlePointContour_pix_move") << 0 << parameterResultMove << scannerPositionX_mm << scannerPositionY_mm << 255 << point_vector_t{{0.0, 0.0}} << rankMax << point_vector_t{{1.3, 1.9}} << true << rankMax;
    QTest::newRow("emptyContour_move") << 0 << parameterResultMove << scannerPositionX_mm << scannerPositionY_mm  << 255 << point_vector_t{} << rankMax << point_vector_t{} << true << rankMin;
    QTest::newRow("notValidContour_move") << 0 << parameterResultMove << scannerPositionX_mm << scannerPositionY_mm  << 255 << point_vector_t{{0.0, 0.0}} << rankMin << point_vector_t{{0.0, 0.0}} << true << rankMin;
    QTest::newRow("threePointContour_mm_move") << 1 <<parameterResultMove << scannerPositionX_mm << scannerPositionY_mm  << 255 << point_vector_t{{0.0, 0.0},{10.0, 0.0}, {10.0,-10.0}} << rankMax << point_vector_t{{1.3, 1.9}} << true << rankMax;


    QTest::newRow("singlePointContour_pix_preview") << 0 << parameterResultPreview << scannerPositionX_mm << scannerPositionY_mm << 255 << point_vector_t{{0.0, 0.0}} << rankMax << point_vector_t{{0.0, 0.0}} << false << rankMin;
    QTest::newRow("emptyContour_preview") << 0 << parameterResultPreview << scannerPositionX_mm << scannerPositionY_mm  << 255 << point_vector_t{} << rankMax << point_vector_t{} << false << rankMin;
    QTest::newRow("notValidContour_preview") << 0 << parameterResultPreview << scannerPositionX_mm << scannerPositionY_mm  << 255 << point_vector_t{{0.0, 0.0}} << rankMin << point_vector_t{{0.0, 0.0}} << false << rankMin;
    QTest::newRow("threePointContour_mm_preview") << 1 <<parameterResultPreview << scannerPositionX_mm << scannerPositionY_mm  << 255 << point_vector_t{{0.0, 0.0},{10.0, 0.0}, {10.0,-10.0}} << rankMax << point_vector_t{{0.0, 0.0}} << false << rankMin;

}

void TestSeamWeldingResult::testProceed()
{
    SeamWeldingResult filter;

    // create and connect pipes
    DummyResultHandler dummyFilter;
    DummyInput input;

    std::unique_ptr<precitec::image::OverlayCanvas> pcanvas {new precitec::image::OverlayCanvas};
    assert(pcanvas.get());
    filter.setCanvas(pcanvas.get());
    QVERIFY(input.connectToFilter(&filter,1));
    QVERIFY(dummyFilter.connectPipe(filter.findPipe("SeamWeldingResultOutput"), 0));

    auto outPipe = dynamic_cast<fliplib::SynchronePipe<ResultDoubleArray>*>(filter.findPipe("SeamWeldingResultOutput"));
    QVERIFY(outPipe);

    //set parameters
    QFETCH(int, parameterInputContourType);
    QFETCH(int, parameterResult);
    auto & rParameters = filter.getParameters();
    rParameters.update(std::string("InputContourType"), fliplib::Parameter::TYPE_int, parameterInputContourType);
    rParameters.update(std::string("Result"), fliplib::Parameter::TYPE_int, parameterResult);
    //rParameters.update(std::string("Verbosity"), fliplib::Parameter::TYPE_int, int(fliplib::BaseFilter::VerbosityType::eMax));
    filter.setParameter();

    // generate data
    int imageNumber = 0;
    QFETCH(double, scannerPositionX_mm);
    QFETCH(double, scannerPositionY_mm);
    QFETCH(int, scannerPositionY_rank);

    auto &calibData = precitec::system::CalibDataSingleton::getCalibrationDataReference(precitec::math::SensorId::eSensorId0);
    calibData.updateCameraCorrection(scannerPositionX_mm, scannerPositionY_mm, (imageNumber % g_oNbPar));

    //simple case HWROI 0,0 trafo 0,0
    auto current_tcp = calibData.getTCPCoordinate(imageNumber, precitec::filter::LaserLine::FrontLaserLine);
    QFETCH(point_vector_t, input_points_distance_from_tcp);
    QFETCH(int, rankPoints);
    QFETCH(point_vector_t, expected_points_mm);

    std::vector<precitec::geo2d::DPoint> points_pix = input_points_distance_from_tcp;

    if( parameterInputContourType == int (SeamWeldingResult::InputContourType::pixel))
    {
        for (auto & point : points_pix)
        {
            point = current_tcp + point;
        }
    }

    auto & rCoords3D = precitec::system::CalibDataSingleton::getCalibrationCoords(precitec::math::SensorId::eSensorId0);
    float x,y;
    bool valid = rCoords3D.getCoordinates(x,y,0,0);
    QVERIFY(valid);
    valid = rCoords3D.getCoordinates(x,y,current_tcp.x, current_tcp.y);

    QCOMPARE(dummyFilter.isProceedCalled(), false);
    //input.fillDataAndSignal(imageNumber, scannerPositionX_mm, scannerPositionY_mm, points_relative_mm);
    input.fillDataAndSignal(imageNumber, scannerPositionX_mm, scannerPositionY_mm, points_pix, rankPoints, scannerPositionY_rank);
    QCOMPARE(dummyFilter.isProceedCalled(), true);

    auto result = outPipe->read(imageNumber);
    switch(parameterResult)
    {
        case 0:
            QCOMPARE(result.resultType(), ResultType::ScanmasterSeamWelding);
            break;
        case 1:
            QCOMPARE(result.resultType(), ResultType::ScanmasterScannerMoving);
            break;
        case 2:
            //dummy result
            break;
    }
//    QTEST(result.isValid(), "expectedResultValidity");
    QVERIFY(!result.isNio());
    if (result.rank().front() == precitec::filter::eRankMax)
    {
        QCOMPARE(result.value().size()/4, expected_points_mm.size());
    }
    QTEST(result.rank().front(), "expectedFirstRank");

    if (result.rank().front() == precitec::filter::eRankMax)
    {
        int resultIndex = 0;
        int pointIndex = 0;
        for (auto & rExpectedPoint : expected_points_mm)
        {
            QCOMPARE(result.value()[resultIndex], rExpectedPoint.x);
            resultIndex++;
            QCOMPARE(result.value()[resultIndex], rExpectedPoint.y);
            resultIndex++;
            if (result.resultType() == ResultType::ScanmasterSeamWelding)
            {
                QCOMPARE(result.value()[resultIndex], 0.01 * pointIndex); // by construction, see fillDataAndSignal
            }
            resultIndex++;
            if (result.resultType() == ResultType::ScanmasterSeamWelding)
            {
                QCOMPARE(result.value()[resultIndex], 0.02 * pointIndex); // by construction, see fillDataAndSignal
            }
            resultIndex++;
            if (result.resultType() == ResultType::ScanmasterSeamWelding)
            {
                QCOMPARE(result.value()[resultIndex], 0.05 * pointIndex); // by construction, see fillDataAndSignal
            }
            resultIndex++;
            pointIndex++;
        }
    }
}


QTEST_GUILESS_MAIN(TestSeamWeldingResult)
#include "testSeamWeldingResult.moc"
