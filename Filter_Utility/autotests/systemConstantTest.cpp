#include <QTest>

#include "../systemConstant.h"

#include <fliplib/NullSourceFilter.h>
#include <fliplib/SinkFilter.h>
#include <filter/sensorFilterInterface.h>
#include <util/calibDataSingleton.h>
#include <Calibration3DCoordsLoader.h>

using precitec::filter::SystemConstant;


namespace {
    const precitec::geo2d::DPoint gTCP{20.0,30.0};
    const precitec::geo2d::Point gHWROI{4,8};
    
    const precitec::math::CoaxCalibrationData gCoaxData = [] ()
        {
            precitec::math::CoaxCalibrationData oCoaxData;
            //same values test values as  TestCalibration3DCoords
            oCoaxData.m_oBeta0 = 0.1;
            oCoaxData.m_oBetaZ = 0.2489175749;
            oCoaxData.m_oBetaZ2 = 0.2969726798;
            oCoaxData.m_oBetaZTCP = 0.5;
            oCoaxData.m_oDpixX = 1e-3;
            oCoaxData.m_oDpixY = oCoaxData.m_oDpixX;
            oCoaxData.m_oWidth = 1024;
            oCoaxData.m_oHeight = 1024;
            oCoaxData.m_oOrigX = oCoaxData.m_oWidth/2;
            oCoaxData.m_oOrigY = oCoaxData.m_oHeight/2;;
            return oCoaxData;
        }();
}

class SystemConstantTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testProceed_data();
    void testProceed();
    void testTCPWithOffset();

private:
    QTemporaryDir m_dir;
};

void SystemConstantTest::testCtor()
{
    SystemConstant filter;
    QCOMPARE(filter.name(), std::string("SystemConstant"));
    QVERIFY(filter.findPipe("DataOut") != nullptr);
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);
    QVERIFY(filter.getParameters().exists(std::string("Constant")));

    QCOMPARE(filter.getParameters().findParameter(std::string("Constant")).getType(), fliplib::Parameter::TYPE_int);
    QCOMPARE(filter.getParameters().findParameter(std::string("Constant")).getValue().convert<int>(), 0);
}

void SystemConstantTest::initTestCase()
{
    auto &calibData = precitec::system::CalibDataSingleton::getCalibrationDataReference(precitec::math::SensorId::eSensorId0);
    precitec::math::Calibration3DCoords coords;
    bool ok = precitec::math::loadCoaxModel(coords, gCoaxData, false);
    QVERIFY(ok);
    QCOMPARE(coords.isScheimpflugCase(), false);
    precitec::math::CalibrationParamMap params;
    params.setDouble("xtcp", gTCP.x);
    params.setDouble("ytcp", gTCP.y);
    params.setDouble("ytcp_2", 200);
    params.setDouble("ytcp_tcp", 300);
    calibData.reload(coords, params);

    QVERIFY(m_dir.isValid());
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());
    QDir{}.mkpath(m_dir.filePath(QStringLiteral("config")));
}

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

void SystemConstantTest::testProceed_data()
{
    QTest::addColumn<int>("constant");
    QTest::addColumn<double>("expectedResult");

    QTest::newRow("eNull") << int(precitec::filter::eNull) << 0.0;
    QTest::newRow("eTCP_X") << int(precitec::filter::eTCP_X) << gTCP.x;
    QTest::newRow("eTCP_Y") << int(precitec::filter::eTCP_Y) << gTCP.y;

    QTest::newRow("eHWROI_X") << int(precitec::filter::eHWROI_X) << double(gHWROI.x);
    QTest::newRow("eHWROI_Y") << int(precitec::filter::eHWROI_Y) << double(gHWROI.y);
    QTest::newRow("eImageNumber") << int(precitec::filter::eImageNumber) << 5.0;
    QTest::newRow("ePosition") << int(precitec::filter::ePosition) << 300.0;
    QTest::newRow("eInputImage_W") << int(precitec::filter::eInput_W ) << 60.0;
    QTest::newRow("eInputImage_H") << int(precitec::filter::eInput_H ) << 70.0;
    QTest::newRow("eUpper") << int(precitec::filter::eUpper ) << 0.0; 
    QTest::newRow("eLower") << int(precitec::filter::eLower ) << 0.0; 
    QTest::newRow("eDpixX") << int(precitec::filter::eDpixX ) << 0.0106; 
    QTest::newRow("eDpixY") << int(precitec::filter::eDpixY ) << 0.0106; 
    QTest::newRow("eBeta0") << int(precitec::filter::eBeta0 ) << 0.5; 
    QTest::newRow("eBetaZ") << int(precitec::filter::eBetaZ ) << 0.5; 
    QTest::newRow("eBetaZ_2") << int(precitec::filter::eBetaZ_2 ) << 0.5; 
    QTest::newRow("eBetaZ_TCP") << int(precitec::filter::eBetaZ_TCP ) << 0.5; 
    QTest::newRow("eRatio_pix_mm_horizontal") << int(precitec::filter::eRatio_pix_mm_horizontal) << gCoaxData.m_oBeta0/gCoaxData.m_oDpixX;
    QTest::newRow("getRatio_pix_mm_Z1") << int(precitec::filter::eRatio_pix_mm_Z1) << gCoaxData.m_oBetaZ/gCoaxData.m_oDpixY;
    QTest::newRow("getRatio_pix_mm_Z2") << int(precitec::filter::eRatio_pix_mm_Z2) << gCoaxData.m_oBetaZ2/gCoaxData.m_oDpixY;
    QTest::newRow("getRatio_pix_mm_Z3") << int(precitec::filter::eRatio_pix_mm_Z3) << gCoaxData.m_oBetaZTCP/gCoaxData.m_oDpixY;
    QTest::newRow("eContextTCP_X") << int(precitec::filter::eContextTCP_X) << gTCP.x - gHWROI.x;
    QTest::newRow("eContextTCP_Y") << int(precitec::filter::eContextTCP_Y) << gTCP.y - gHWROI.y;
    
    QTest::newRow("random value") << 123456789 << 0.0;
    
}

void SystemConstantTest::testProceed()
{
    // prepare filter graph
    // a null source filter connected with SystemConstant filter, connected with DummyFilter
    SystemConstant filter;
    fliplib::NullSourceFilter sourceFilter;
    fliplib::SynchronePipe<precitec::interface::ImageFrame> pipe{ &sourceFilter, precitec::interface::SensorFilterInterface::SENSOR_IMAGE_FRAME_PIPE };

    QVERIFY(filter.connectPipe(&pipe, 0));

    DummyFilter dummyFilter;
    QVERIFY(dummyFilter.connectPipe(filter.findPipe("DataOut"), 0));

    // parameterize the filter
    QFETCH(int, constant);
    filter.getParameters().update(std::string("Constant"), fliplib::Parameter::TYPE_int, constant);

    filter.setParameter();

    // and process
    // first generate the image frame with some dummy data
    precitec::interface::ImageFrame frame;
    auto oSensorImage = precitec::image::genModuloPattern(precitec::geo2d::Size(gCoaxData.m_oWidth, gCoaxData.m_oHeight), 2);
    frame.data() = precitec::image::BImage(oSensorImage, precitec::geo2d::Size{60,70}, true); //the result image has size 60,70

    frame.context().setImageNumber(5);
    frame.context().setPosition(300);
    frame.context().HW_ROI_x0 = gHWROI.x;
    frame.context().HW_ROI_y0 = gHWROI.y;
    // now signal the pipe, this processes the complete filter graph
    QCOMPARE(dummyFilter.isProceedCalled(), false);
    pipe.signal(frame);
    QCOMPARE(dummyFilter.isProceedCalled(), true);

    // access the out pipe data of the SystemConstant filter
    auto outPipe = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("DataOut"));
    QVERIFY(outPipe);
    // the result is a GeoDoublearray with one element
    const auto result = outPipe->read(frame.context().imageNumber());
    QCOMPARE(result.ref().size(), 1);
    QFETCH(double, expectedResult);
    QVERIFY(precitec::math::isClose(result.ref().getData()[0], expectedResult, 1e-3));
}

void SystemConstantTest::testTCPWithOffset()
{
    // prepare filter graph
    // a null source filter connected with SystemConstant filter, connected with DummyFilter
    SystemConstant filter;
    fliplib::NullSourceFilter sourceFilter;
    fliplib::SynchronePipe<precitec::interface::ImageFrame> pipe{ &sourceFilter, precitec::interface::SensorFilterInterface::SENSOR_IMAGE_FRAME_PIPE };

    QVERIFY(filter.connectPipe(&pipe, 0));

    DummyFilter dummyFilter;
    QVERIFY(dummyFilter.connectPipe(filter.findPipe("DataOut"), 0));


    
    auto &calibData = precitec::system::CalibDataSingleton::getCalibrationDataReference(precitec::math::SensorId::eSensorId0);
    std::vector<std::pair<precitec::geo2d::DPoint, precitec::coordinates::CalibrationCameraCorrection>> offsetsInit = 
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
    
    //see current constructor of CalibrationCorrrectionContainer
    std::vector<std::pair<precitec::geo2d::DPoint, precitec::coordinates::CalibrationCameraCorrection>> offsets = 
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
              //interpolated values
              {{ 10.0, 20.0},   {-1.95, 3.1} },
              {{ 0.0, 10.0},   {-1.1, 2.05} },
              {{ 10.0, 10.0},   {-1.025, 0.95} },
            };
    
    
    const precitec::geo2d::DPoint expectedBaseTCP {gTCP.x, gTCP.y};
    
    int imageNumber = 0;
    for (int constant : {precitec::filter::eTCP_X, precitec::filter::eTCP_Y})
    {
        // parameterize the filter
        filter.getParameters().update(std::string("Constant"), fliplib::Parameter::TYPE_int, constant);
        for (auto & rEntry :  offsets)
            {
                auto & rScannerPos = rEntry.first;
                auto & rExpectedOffset = rEntry.second;
                double expectedResult = (constant == precitec::filter::eTCP_X ? expectedBaseTCP.x + rExpectedOffset.m_oTCPXOffset :
                                                                                expectedBaseTCP.y + rExpectedOffset.m_oTCPYOffset
                                                                                );
                
                calibData.updateCameraCorrection(rScannerPos.x, rScannerPos.y, (imageNumber % g_oNbPar));
                        
                filter.setParameter();

                // and process
                // first generate the image frame with some dummy data
                precitec::interface::ImageFrame frame;
                auto oSensorImage = precitec::image::genModuloPattern({80, 80}, 2);
                frame.data() = precitec::image::BImage(oSensorImage, precitec::geo2d::Size{61,71}); //the result image has size 60,70

                frame.context().setImageNumber(imageNumber);
                frame.context().setPosition(300);
                frame.context().HW_ROI_x0 = 40;
                frame.context().HW_ROI_y0 = 50;
                frame.context().m_ScannerInfo.m_hasPosition = true;
                frame.context().m_ScannerInfo.m_x = rScannerPos.x;
                frame.context().m_ScannerInfo.m_y = rScannerPos.y;
                // now signal the pipe, this processes the complete filter graph
                QCOMPARE(dummyFilter.isProceedCalled(), false);
                pipe.signal(frame);
                QCOMPARE(dummyFilter.isProceedCalled(), true);

                // access the out pipe data of the SystemConstant filter
                auto outPipe = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("DataOut"));
                QVERIFY(outPipe);
                // the result is a GeoDoublearray with one element
                const auto result = outPipe->read(frame.context().imageNumber());
                QCOMPARE(result.ref().size(), 1);
                QCOMPARE(result.ref().getData()[0], expectedResult);
                
                dummyFilter.resetProceedCalled();
                imageNumber++;
            }
    }
}

QTEST_GUILESS_MAIN(SystemConstantTest)
#include "systemConstantTest.moc"
