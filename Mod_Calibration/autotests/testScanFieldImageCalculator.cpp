#include <QTest>

#include "../include/calibration/ScanFieldImageCalculator.h"
#include "util/CalibrationScanMasterData.h"
#include "util/ScanFieldImageParameters.h"
#include "common/bitmap.h"
#include <Poco/Util/Util.h>
#include <Poco/Util/IniFileConfiguration.h>
#include <Poco/Util/XMLConfiguration.h>
#include <mathCommon.h>

typedef std::vector<std::tuple<std::string,double,double>>  TImageWithScannerPositionList;
Q_DECLARE_METATYPE(TImageWithScannerPositionList)

namespace
{
    void getScannerLimitsInnputImages(const TImageWithScannerPositionList & inputImages, 
        double & xmin_mm, double & ymin_mm, double & xmax_mm, double & ymax_mm)
    {
        bool init = false;
        for (auto & rInput : inputImages)
        {
            double x = std::get<1>(rInput);
            double y = std::get<2>(rInput);
            if (init)
            {
                xmin_mm = std::min(xmin_mm,x);
                ymin_mm = std::min(ymin_mm,y);
                xmax_mm = std::max(xmax_mm,x);
                ymax_mm = std::max(ymax_mm,y);
            }
            else
            {
                xmin_mm = x;
                ymin_mm = y;
                xmax_mm = x;
                ymax_mm = y;
                init = true;   
            }
        }
    };
}
class TestScanFieldImageCalculator : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor_data();
    void testCtor();
    void testCoordinateTransform_data();
    void testCoordinateTransform();
    void testStaticKeys();
    void testMakeConfiguration();
    void testGetKeyValue();
    void testLoadConfiguration();
};

using precitec::image::BImage;
using precitec::calibration::ScanMasterCalibrationData;
using precitec::calibration::ScanFieldImageCalculator;
using precitec::calibration::ScanFieldImageParameters;
using precitec::interface::SmpKeyValue;

void TestScanFieldImageCalculator::testCtor_data()
{
    QTest::addColumn<double>("pix_to_mm_x");
    QTest::addColumn<double>("pix_to_mm_y");
    QTest::addColumn<double>("pix_slope");
    
    QTest::addColumn<TImageWithScannerPositionList>("inputImages");
    QTest::addColumn<QString>("subfolder");
    
    
    
    double pix_to_mm_x = -1/26.9; 
    double pix_to_mm_y = 1/26.9;
    
    {
        //test 1: text with delta = 20
        
        std::string testFolder = "/opt/wm_inst/data/scanfieldimage/first_test/";
        TImageWithScannerPositionList inputImages = {
            {testFolder+"row_0_col_0.bmp", -40, -40}, 
            {testFolder+"row_0_col_1.bmp", -20, -40},
            {testFolder+"row_0_col_2.bmp", 0,   -40},
            {testFolder+"row_0_col_3.bmp", 20,  -40},
            {testFolder+"row_1_col_0.bmp", -40,  -20},
            {testFolder+"row_1_col_1.bmp", -20,  -20},
            {testFolder+"row_1_col_2.bmp", 0,    -20},
            {testFolder+"row_1_col_3.bmp", 20,   -20},
            {testFolder+"row_2_col_0.bmp", -40,  0},
            {testFolder+"row_2_col_1.bmp", -20,  0},
            {testFolder+"row_2_col_2.bmp", 0,     0},
            {testFolder+"row_2_col_3.bmp", 20,    0}
                    
        };
        for (double pix_slope : { 0.0, 0.019, 0.02})
        {
            QString slope = QString::number(pix_slope);
            QString imageName = QString::fromLatin1("test1_Output_%1/").arg(slope);
            QTest::addRow("Test1_%f", pix_slope) << pix_to_mm_x << pix_to_mm_y << pix_slope<< inputImages << imageName;
        }
    }
    {
        //test 2: delta = 30
        for (auto entry: std::map<std::string,std::string>{
                            { "chessboard", "/opt/wm_inst/data/scanfieldimage/second_test/20200213_152030/"},
                            { "text",       "/opt/wm_inst/data/scanfieldimage/second_test/20200213_151250/"}})
        {
            std::string testName = entry.first;
            std::string testFolder = entry.second;
            
            TImageWithScannerPositionList inputImages = {
                            {testFolder+"row_0_col_0.bmp", -50, -50}, 
                            {testFolder+"row_0_col_1.bmp", -20, -50},
                            {testFolder+"row_0_col_2.bmp",  10, -50},
                            {testFolder+"row_0_col_3.bmp",  40, -50},
                            
                            {testFolder+"row_1_col_0.bmp", -50, -20},
                            {testFolder+"row_1_col_1.bmp", -20, -20},
                            {testFolder+"row_1_col_2.bmp",  10, -20},
                            {testFolder+"row_1_col_3.bmp",  40, -20},
                            
                            {testFolder+"row_2_col_0.bmp", -50,  10},
                            {testFolder+"row_2_col_1.bmp", -20,  10},
                            {testFolder+"row_2_col_2.bmp",  10,  10},
                            {testFolder+"row_2_col_3.bmp",  40,  10},
                            
                            {testFolder+"row_3_col_0.bmp", -50, 40},
                            {testFolder+"row_3_col_1.bmp", -20, 40},
                            {testFolder+"row_3_col_2.bmp",  10, 40},
                            {testFolder+"row_3_col_3.bmp",  40, 40}
                                    
                        };
            
            for (double pix_slope : { 0.0, 0.02, -0.02})
            {
                QString slope = QString::number(pix_slope);
                QString subfolder = QString::fromLatin1("test2_Output_%1/").arg( slope);
                QTest::addRow("Test2_%s_%f", testName.c_str(), pix_slope) << pix_to_mm_x << pix_to_mm_y << pix_slope<< inputImages << subfolder;
            }
        }
    }
    
    {
        // test 3: pattern image,  random positions
        TImageWithScannerPositionList inputImages = {
            {"", -60, 20},
            {"", 0,  0},
            {"", 60, -20},
        };
        for (double pix_slope : { 0.0, -0.02, 0.02})
        {
            QString slope = QString::number(pix_slope);
            QString subfolder = QString::fromLatin1("test3_Output_%1/").arg(slope);
            QTest::addRow("Test1_%f", pix_slope) << pix_to_mm_x << pix_to_mm_y << pix_slope<< inputImages << subfolder;
        }
    }
}


void TestScanFieldImageCalculator::testCtor()
{
    using namespace precitec::calibration;
    
    QFETCH(QString,subfolder);
    QFETCH(TImageWithScannerPositionList, inputImages);
    QTemporaryDir dir;
    
    QVERIFY(QDir{dir.path()}.mkdir(subfolder));
    
    std::string oScanFieldImageFolder = QDir{dir.path()}.absolutePath().toStdString() + "/" +subfolder.toStdString();
    
    QVERIFY(inputImages.size() > 0);
    
    double xmin_mm, ymin_mm, xmax_mm, ymax_mm;
    getScannerLimitsInnputImages(inputImages, xmin_mm, ymin_mm, xmax_mm, ymax_mm );
    
    bool usePatternImage = false;
    BImage oImage;
    {
        //check first image dimension
        const auto & rImageFilename = std::get<0>(inputImages[0]);
        fileio::Bitmap oBitmap(rImageFilename); 
        if (!oBitmap.validSize())
        {
            //QSKIP(std::string("System does not have test data " + rImageFilename).c_str());
            usePatternImage = true;
            oImage = precitec::image::genModuloPattern({1024, 1024}, 20);;
        }
        else
        {
            QVERIFY2(oBitmap.validSize(), rImageFilename.c_str());
            oImage.resize({oBitmap.width(), oBitmap.height()});
            bool ok = oBitmap.load( oImage.begin());
            QVERIFY2(ok, rImageFilename.c_str());
        }
        
    }
    QFETCH(double, pix_to_mm_x);
    QFETCH(double, pix_to_mm_y);
    QFETCH(double, pix_slope);
    
    ScanMasterCalibrationData oScanMasterCalibrationData {1.0/pix_to_mm_x,  1.0/pix_to_mm_y,  pix_slope};
    
    ScanFieldImageCalculator oScanFieldImageCalculator(ScanFieldImageParameters::computeParameters(
        oScanMasterCalibrationData, oImage.width(),  oImage.height(),  xmin_mm, xmax_mm,  ymin_mm,  ymax_mm));
    
    for (auto & rInput : inputImages)
    {
        const auto & rImageFilename = std::get<0>(rInput);    
        double x = std::get<1>(rInput);
        double y = std::get<2>(rInput);
        
        if (!usePatternImage)
        {
            fileio::Bitmap oBitmap(rImageFilename);    
            QVERIFY2(oBitmap.isValid(), rImageFilename.c_str());
            oImage.resize({oBitmap.width(), oBitmap.height()});
            bool ok = oBitmap.load( oImage.begin());
            QVERIFY2(ok, rImageFilename.c_str());
        }
        auto oPositionInScanfieldImage = oScanFieldImageCalculator.getParameters().getTopLeftCornerInScanFieldImageBeforeMirroring(oImage.size(), x,y);

        bool ok = oScanFieldImageCalculator.pasteImage(oImage,oPositionInScanfieldImage.x, oPositionInScanfieldImage.y);
        QVERIFY(ok);

    }
    
    // compute final image, but do not save result to disk
    auto oFinalScanFieldResult = oScanFieldImageCalculator.computeAndWriteScanFieldImage("");
    QVERIFY(std::get<1>(oFinalScanFieldResult).empty());
    QVERIFY(std::get<2>(oFinalScanFieldResult).empty());
    
    // compute final image and save result to disk
    oFinalScanFieldResult = oScanFieldImageCalculator.computeAndWriteScanFieldImage(oScanFieldImageFolder);
    QVERIFY(! std::get<1>(oFinalScanFieldResult).empty());
    
    //std::cout <<  "Image and configuration saved to " << oFinalScanFieldResult.second << std::endl;
    auto oFinalScanFieldImage = std::get<0>(oFinalScanFieldResult);
    QVERIFY(oFinalScanFieldImage.size().area() > 0);
    QVERIFY(oFinalScanFieldImage.size().area() > oImage.size().area());
    
    QVERIFY2(oFinalScanFieldImage.isContiguos(), "test must be changed, max can't be computed with accumulate" );
    QVERIFY(std::accumulate(oFinalScanFieldImage.begin(), oFinalScanFieldImage.end(), 0.0) > 0);
    
    auto oConfigFilename = std::get<2>(oFinalScanFieldResult);
    
    Poco::AutoPtr<Poco::Util::IniFileConfiguration> pConfIn;
    
    // poco syntax exception might be thrown or sax parse excpetion
    
    pConfIn = new Poco::Util::IniFileConfiguration(oConfigFilename);
    ScanFieldImageParameters oSavedParameters;
    oSavedParameters.load(pConfIn);
    
        
    QCOMPARE(oFinalScanFieldImage.size(), oScanFieldImageCalculator.getParameters().m_scanfieldimageSize);
    for (auto & rInput : inputImages)
    {
        double x = std::get<1>(rInput);
        double y = std::get<2>(rInput);
        auto oPositionInScanfieldImage = oScanFieldImageCalculator.getParameters().getCenterInScanFieldImage(x,y);
        
        auto oComputedScannerPosition = oSavedParameters.getScannerPositionFromScanFieldImage(oPositionInScanfieldImage.x, oPositionInScanfieldImage.y);
        QVERIFY(precitec::math::isClose(oComputedScannerPosition.x, x,  1e-12));
        QVERIFY(precitec::math::isClose(oComputedScannerPosition.y, y,  1e-12));
    }
    
    
    
}


void TestScanFieldImageCalculator::testCoordinateTransform_data()
{
    QTest::addColumn<double>("XmmToPixel");
    QTest::addColumn<double>("YmmToPixel");
    QTest::addColumn<double>("pix_slope");
    QTest::addColumn<double>("xmin_mm");
    QTest::addColumn<double>("xmax_mm");
    QTest::addColumn<double>("ymin_mm");
    QTest::addColumn<double>("ymax_mm");
    QTest::addColumn<int>("imageWidth");
    QTest::addColumn<int>("imageHeight");
 
    QTest::addRow("Test1024x1024_noSlope") << -27.0 <<  27.0 << 0.0 <<  -50.0 << 50.0 <<  -50.0 << 50.0 <<  1024 << 1024;
    QTest::addRow("Test1024x1024_Slope") << -27.0 <<  27.0 << 0.01 <<  -50.0 << 50.0 <<  -50.0 << 50.0 <<  1024 << 1024;
    QTest::addRow("Test1280x1024_noSlope") << -27.0 <<  27.0 << 0.0 <<  -50.0 << 50.0 <<  -50.0 << 50.0 <<  1280 << 1024;
    QTest::addRow("Test1280x1024_Slope") << -27.0 <<  27.0 << 0.01 <<  -50.0 << 50.0 <<  -50.0 << 50.0 <<  1280 << 1024;
}    

void TestScanFieldImageCalculator::testCoordinateTransform()
{
    QFETCH(double, XmmToPixel);
    QFETCH(double, YmmToPixel);
    QFETCH(double, pix_slope);
    QFETCH(double, xmin_mm);
    QFETCH(double, xmax_mm);
    QFETCH(double, ymin_mm);
    QFETCH(double, ymax_mm);
    QFETCH(int, imageWidth);
    QFETCH(int, imageHeight);
    
    precitec::image::Size2d oImageSize{imageWidth,  imageHeight};
    ScanMasterCalibrationData oScanMasterCalibrationData {XmmToPixel, YmmToPixel,  pix_slope};
    
    auto oScanFieldImageParameters = ScanFieldImageParameters::computeParameters(oScanMasterCalibrationData, 
        oImageSize.width,  oImageSize.height,  xmin_mm, xmax_mm,  ymin_mm,  ymax_mm);
  
    
    ScanFieldImageCalculator oScanFieldImageCalculator(oScanFieldImageParameters);
    
    
    for (double x = xmin_mm, deltax = (xmax_mm - xmin_mm)/5; x <= xmax_mm; x += deltax)
    {
        for (double y = ymin_mm, deltay = (ymax_mm - ymin_mm)/5; y <= ymax_mm; y += deltay)
        {
            precitec::geo2d::DPoint expectedScannerPosition {x, y};
            auto oPixelPosition = oScanFieldImageParameters.getCenterInScanFieldImage(expectedScannerPosition.x, expectedScannerPosition.y);
            auto oScannerPosition = oScanFieldImageParameters.getScannerPositionFromScanFieldImage(oPixelPosition.x,  oPixelPosition.y);
            QCOMPARE(oScannerPosition.x,  expectedScannerPosition.x);
            QCOMPARE(oScannerPosition.y,  expectedScannerPosition.y);
        }
    }
    
}


void  TestScanFieldImageCalculator::testStaticKeys()
{    
    {
        QCOMPARE(ScanMasterCalibrationData::s_keys.size(), ScanMasterCalibrationData::key_id::NUM_IDs);
        std::set<std::string> unique_keys;
        //verify that all the keys are set and unique
        for (auto & key : ScanMasterCalibrationData::s_keys)
        {
            QVERIFY(key != "");
            unique_keys.insert(key);
        }
        QCOMPARE( unique_keys.size(),  ScanMasterCalibrationData::key_id::NUM_IDs);
    }
    
    {
        QCOMPARE(ScanFieldImageParameters::s_keys.size(), ScanFieldImageParameters::key_id::NUM_IDs);
        std::set<std::string> unique_keys;
        //verify that all the keys are set and unique
        for (auto & key : ScanFieldImageParameters::s_keys)
        {
            QVERIFY(key != "");
            unique_keys.insert(key);
        }
        QCOMPARE( unique_keys.size(), ScanFieldImageParameters::key_id::NUM_IDs);
    }
}

void TestScanFieldImageCalculator::testMakeConfiguration()
{
    ScanFieldImageParameters defaultParameters;
    auto config = defaultParameters.makeConfiguration();
    QCOMPARE(config.size(), ScanFieldImageParameters::key_id::NUM_IDs + ScanMasterCalibrationData::key_id::NUM_IDs );
   
    //verify that all the keys are  unique and correspond to the key list
    std::set<std::string> unique_keys;
    std::set<int> unique_ids_image;
    std::set<int> unique_ids_data;
    
    //verify that the default values are actually used in the constructor
    precitec::interface::Configuration configSetToDefault;
    for (const auto & rKeyValue : config)
    {
        auto itImageParameters = std::find(ScanFieldImageParameters::s_keys.begin(), ScanFieldImageParameters::s_keys.end(), rKeyValue->key());
        if(itImageParameters != ScanFieldImageParameters::s_keys.end())
        {
            unique_keys.insert(rKeyValue->key());
            unique_ids_image.insert(int(std::distance(ScanFieldImageParameters::s_keys.begin(), itImageParameters)));
            
            auto keyValueCopy = rKeyValue->clone();
            keyValueCopy->resetToDefault();
            QCOMPARE(rKeyValue->toString(),  keyValueCopy->toString());
        }
        else
        {
            auto itCalibParameter = std::find(ScanMasterCalibrationData::s_keys.begin(), ScanMasterCalibrationData::s_keys.end(), rKeyValue->key());
            QVERIFY( itCalibParameter !=  ScanMasterCalibrationData::s_keys.end());
            unique_keys.insert(rKeyValue->key());
            unique_ids_data.insert(int(std::distance(ScanMasterCalibrationData::s_keys.begin(), itCalibParameter)));
            
            auto keyValueCopy = rKeyValue->clone();
            keyValueCopy->resetToDefault();
            QCOMPARE(rKeyValue->toString(), keyValueCopy->toString());
        }
    }
    
    QCOMPARE( unique_keys.size(), config.size());
    QCOMPARE( unique_ids_image.size(),  ScanFieldImageParameters::key_id::NUM_IDs);
    QCOMPARE( unique_ids_data.size(),  ScanMasterCalibrationData::key_id::NUM_IDs);
}

void TestScanFieldImageCalculator::testGetKeyValue()
{
    ScanFieldImageParameters defaultParameters;
    
    //test wrong key
    {
        auto kv = defaultParameters.getKeyValue("");
        QVERIFY(kv.isNull() || !kv->isHandleValid());
    }
    
    //test all the valid keys
    for (auto & key :  ScanFieldImageParameters::s_keys)
    {
        auto kv = defaultParameters.getKeyValue(key);
        QCOMPARE(kv->key(), key);               
    }
    for (auto & key :  ScanMasterCalibrationData::s_keys)
    {
        auto kv = defaultParameters.getKeyValue(key);
        QCOMPARE(kv->key(), key);               
    }
}


void TestScanFieldImageCalculator::testLoadConfiguration()
{

    Poco::AutoPtr<Poco::Util::XMLConfiguration> pConfIn {new Poco::Util::XMLConfiguration};
    pConfIn->loadEmpty("key_value_configuration");
    
    pConfIn->setDouble("SM_scanXToPixel", 1.0);
    pConfIn->setDouble("SM_scanYToPixel", 2.0);
    pConfIn->setDouble("SM_slopePixel", 3.0);
    pConfIn->setBool("SM_mirrorX", false);
    pConfIn->setBool("SM_mirrorY", true);
    pConfIn->setDouble("xMinLeft_mm", 4.0);
    pConfIn->setDouble("yMinTop_mm", 5.0);
    pConfIn->setInt("ScanFieldImageWidth", 6);
    pConfIn->setInt("ScanFieldImageHeight", 7);
    
    ScanFieldImageParameters oParams;
    oParams.load(pConfIn);
    
    QCOMPARE(oParams.m_ScanMasterData.m_XmmToPixel, 1.0);
    QCOMPARE(oParams.m_ScanMasterData.m_YmmToPixel, 2.0);
    QCOMPARE(oParams.m_ScanMasterData.m_Slope, 3.0);
    QCOMPARE(oParams.m_ScanMasterData.m_mirrorX, false);
    QCOMPARE(oParams.m_ScanMasterData.m_mirrorY, true);
    QCOMPARE(oParams.m_xMinLeft_mm, 4.0);
    QCOMPARE(oParams.m_yMinTop_mm, 5.0);
    QCOMPARE(oParams.m_scanfieldimageSize.width, 6);
    QCOMPARE(oParams.m_scanfieldimageSize.height, 7);
    
    QCOMPARE(oParams.getKeyValue("SM_scanXToPixel")->value<double>(), 1.0);
    QCOMPARE(oParams.getKeyValue("SM_scanYToPixel")->value<double>(), 2.0);
    QCOMPARE(oParams.getKeyValue("SM_slopePixel")->value<double>(), 3.0);
    QCOMPARE(oParams.getKeyValue("SM_mirrorX")->value<bool>(), false);
    QCOMPARE(oParams.getKeyValue("SM_mirrorY")->value<bool>(), true);
    QCOMPARE(oParams.getKeyValue("xMinLeft_mm")->value<double>(), 4.0);
    QCOMPARE(oParams.getKeyValue("yMinTop_mm")->value<double>(), 5.0);
    QCOMPARE(oParams.getKeyValue("ScanFieldImageWidth")->value<int>(), 6);
    QCOMPARE(oParams.getKeyValue("ScanFieldImageHeight")->value<int>(), 7);
    
}



QTEST_GUILESS_MAIN(TestScanFieldImageCalculator)
#include "testScanFieldImageCalculator.moc"
