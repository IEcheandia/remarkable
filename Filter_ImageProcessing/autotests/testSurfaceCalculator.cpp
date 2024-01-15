#include <QTest>

#include "../surfaceCalculator.h" 
#include <filter/algoArray.h>

#include <filter/armStates.h>
#include <filter/productData.h>
#include <filter/algoArray.h>


#include <fliplib/NullSourceFilter.h>
#include <fliplib/SinkFilter.h>
#include <filter/sensorFilterInterface.h>
#include <common/frame.h>
#include <filter/armStates.h>
#include <image/image.h>
#include <overlay/overlayCanvas.h>
#include <geo/rect.h>

using precitec::filter::SurfaceCalculator;
using precitec::geo2d::Rect;
using precitec::image::BImage;
using precitec::geo2d::Doublearray;
using precitec::geo2d::TileContainer;
using precitec::geo2d::SingleTile;
using precitec::image::Sample;

using namespace precitec::analyzer;
using precitec::image::genSquaresImagePattern;
using precitec::image::testSquaresImagePatternIntensity;


class TestSurfaceCalculator : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testHelperFunctions();
    void testCtor();
    void testTileOrder_data();
    void testTileOrder();
    void testProceed();
    void testSmallROI_data();
    void testSmallROI();
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
        m_proceedCalled++;
    }
    int getFilterType() const override
    {
        return BaseFilterInterface::SINK;
    }

    unsigned int getProceedCalled() const
    {
        return m_proceedCalled;
    }

private:
    unsigned int m_proceedCalled = 0;
};


void TestSurfaceCalculator::testHelperFunctions()
{
    {
        auto out = genSquaresImagePattern(10,20,20,2);
        for (int y = 0; y < out.height(); y ++)
        {
            auto * pRow = out.rowBegin(y);
            for (int x = 0; x < out.width(); x ++)
            {
                QCOMPARE( pRow[x] , testSquaresImagePatternIntensity (x,y, 10,20,20,2) );
            }
        }
    }
        
}

void TestSurfaceCalculator::testCtor()
{
    SurfaceCalculator filter;
    QCOMPARE(filter.name(), std::string("SurfaceCalculator"));
    QVERIFY(filter.findPipe("SurfaceInfo") != nullptr);
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);
    
    //check parameters of type UInt32
    
    std::map<std::string, int> parametersInt = 
    {
            {"TileWidth", 30},
            {"TileJumpX", 30},
            {"TileHeight", 30},
            {"TileJumpY", 30},
        };
    
    for (auto paramTuple: parametersInt)
    {
        std::string paramName = std::get<0>(paramTuple);
        int defaultValue = std::get<1>(paramTuple);
        
        QVERIFY(filter.getParameters().exists(paramName));
        QCOMPARE(filter.getParameters().findParameter(paramName).getType(), fliplib::Parameter::TYPE_int);
        QCOMPARE(filter.getParameters().findParameter(paramName).getValue().convert<int>(), defaultValue);
    }   
    
    //check parameters of type bool
        
    std::map <std::string, bool> parametersBool = 
    {
        {"EnsureTile", false},
        {"CalcMean", false},
        {"CalcRelInt", false},
        {"CalcVariation", false},
        {"CalcMinMiaxDistance", false},
        {"CalcSurface", false},
        {"CalcSurfaceX",false},
        {"CalcSurfaceY",false},
        {"CalcStructure",false},
        {"CalcTexture", false},
    };
    
    for (auto paramTuple: parametersBool)
    {
        std::string paramName = std::get<0>(paramTuple);
        bool defaultValue = std::get<1>(paramTuple);
        
        QVERIFY(filter.getParameters().exists(paramName));
        QCOMPARE(filter.getParameters().findParameter(paramName).getType(), fliplib::Parameter::TYPE_bool);
        QCOMPARE(filter.getParameters().findParameter(paramName).getValue().convert<bool>(), defaultValue);
    }   
    
}

void TestSurfaceCalculator::testTileOrder_data()
{
    //int squareWidth, int squareHeight, int squaresPerColumn, int squaresPerRow)
    QTest::addColumn<int>("squareWidth");
    QTest::addColumn<int>("squareHeight");
    QTest::addColumn<int>("squaresPerRow");
    QTest::addColumn<int>("squaresPerColumn");

    QTest::newRow("10tiles") << 30 << 40 << 5 << 2;
    QTest::newRow("1tile")   << 30 << 40 << 1 << 1;
    QTest::newRow("width0")  << 30 << 40 << 0 << 5;
    QTest::newRow("height0") << 30 << 40 << 5 << 0;
    QTest::newRow("1PixelTile") << 1 << 1 << 5 << 2;
    QTest::newRow("invalidWidth") << 0 << 40 << 5 << 2;
    QTest::newRow("invalidHeight") << 30 << 0 << 5 << 2;
    
    
}


void TestSurfaceCalculator::testTileOrder()
{
    
    
    SurfaceCalculator filter;
    
    fliplib::NullSourceFilter sourceFilter;
    fliplib::SynchronePipe<precitec::interface::ImageFrame> inImagePipe { &sourceFilter, precitec::interface::SensorFilterInterface::SENSOR_IMAGE_FRAME_PIPE };
    
    int group = 0;
    //inImagePipe.setTag("image");
    QVERIFY(filter.connectPipe(&inImagePipe, group));
    
    DummyFilter dummyFilter;
    QVERIFY(dummyFilter.connectPipe(filter.findPipe("SurfaceInfo"), 0));
    
    // access the out pipe data 
    auto outPipe = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoSurfaceInfoarray>*>(filter.findPipe("SurfaceInfo"));
    QVERIFY(outPipe);

    std::unique_ptr<precitec::image::OverlayCanvas> pcanvas {new precitec::image::OverlayCanvas};
    filter.setCanvas(pcanvas.get());
    
    
    QFETCH(int, squareWidth);
    QFETCH(int, squareHeight);
    QFETCH(int, squaresPerRow);
    QFETCH(int, squaresPerColumn);
    
    // parameterize the filter
    
    filter.getParameters().update(std::string("Verbosity"), fliplib::Parameter::TYPE_int, 6);
    filter.getParameters().update(std::string("CalcMean"), fliplib::Parameter::TYPE_bool, true);
    filter.getParameters().update(std::string("CalcMinMiaxDistance"), fliplib::Parameter::TYPE_bool, true);
    
    //not overlapping tiles
    filter.getParameters().update(std::string("TileWidth"), fliplib::Parameter::TYPE_int, squareWidth);
    filter.getParameters().update(std::string("TileJumpX"), fliplib::Parameter::TYPE_int, squareWidth);
    filter.getParameters().update(std::string("TileHeight"), fliplib::Parameter::TYPE_int, squareHeight);
    filter.getParameters().update(std::string("TileJumpY"), fliplib::Parameter::TYPE_int, squareHeight);
    
    filter.setParameter();
    //QCOMPARE(filter.hasCanvas(), true);
    

    // and process
  
    
    // now signal the pipe, this processes the complete filter graph

    precitec::interface::ImageContext context;    
    
    BImage inImage = genSquaresImagePattern(squareWidth, squareHeight, squaresPerRow, squaresPerColumn);
    auto inPipe = precitec::interface::ImageFrame {
        context, 
        inImage,
        precitec::interface::ResultType::AnalysisOK, 
    };

    QCOMPARE(dummyFilter.getProceedCalled(), 0u);
    inImagePipe.signal(inPipe);
    QCOMPARE(dummyFilter.getProceedCalled(), 1u);

    auto & rGeoSurfaceInfoIn = outPipe->read(0);
    //copy SurfaceInfo
    auto info = rGeoSurfaceInfoIn.ref().getData()[0];
    QCOMPARE (info.usesMean, true);
    QCOMPARE (info.usesMinMaxDistance, true);
    QCOMPARE (info.usesMean, true);
    QCOMPARE (info.usesRelBrightness, false);
    QCOMPARE (info.usesVariation, false);
    QCOMPARE (info.usesSurface, false);
    QCOMPARE (info.usesSurfaceX, false);
    QCOMPARE (info.usesSurfaceY, false);
    QCOMPARE (info.usesTexture, false);
    QCOMPARE (info.usesStructure, false);
    
    
  	TileContainer tileContainer = info.getTileContainer();
    
    if (inImage.size().area() > 0)
    {
        QCOMPARE(tileContainer.m_sizeX, squaresPerRow);
        QCOMPARE(tileContainer.m_sizeY, squaresPerColumn);
    }
    else
    {
        QCOMPARE(tileContainer.m_sizeX, 0);
        QCOMPARE(tileContainer.m_sizeY, 0);        
    }
    for (int j = 0; j < tileContainer.m_sizeY; j++)
	{
		for (int i = 0; i < tileContainer.m_sizeX; i++)
		{
			SingleTile tile = tileContainer.getSingleTile(i, j);
            QCOMPARE(tile.m_startX , i*squareWidth);
            QCOMPARE(tile.m_startY , j*squareHeight);
            QCOMPARE(tile.m_width, squareWidth);
            QCOMPARE(tile.m_height, squareHeight);
            
            QVERIFY(tile.m_isMeanValid);
            unsigned char expectedIntensity = inImage.getValue(tile.m_startX, tile.m_startY);
            QCOMPARE(tile.m_MeanValue, double(expectedIntensity));
            QCOMPARE(tile.m_MinMaxDistance, 0.0);
        }
    }


    
}


void TestSurfaceCalculator::testProceed()
{
    SurfaceCalculator filter;
    fliplib::NullSourceFilter sourceFilter;
    fliplib::SynchronePipe<precitec::interface::ImageFrame> inImagePipe { &sourceFilter, precitec::interface::SensorFilterInterface::SENSOR_IMAGE_FRAME_PIPE };
    
    int group = 0;
    //inImagePipe.setTag("image");
    QVERIFY(filter.connectPipe(&inImagePipe, group));
    
    DummyFilter dummyFilter;
    QVERIFY(dummyFilter.connectPipe(filter.findPipe("SurfaceInfo"), 0));
    
    // access the out pipe data 
    auto outPipe = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoSurfaceInfoarray>*>(filter.findPipe("SurfaceInfo"));
    QVERIFY(outPipe);

    std::unique_ptr<precitec::image::OverlayCanvas> pcanvas {new precitec::image::OverlayCanvas};
    filter.setCanvas(pcanvas.get());
    
    filter.getParameters().update(std::string("Verbosity"), fliplib::Parameter::TYPE_int, 6);
    filter.getParameters().update(std::string("CalcMean"), fliplib::Parameter::TYPE_bool, true);
    filter.getParameters().update(std::string("CalcMinMiaxDistance"), fliplib::Parameter::TYPE_bool, true);
    
    
    filter.setParameter();

    

    int borderX = 10;
    int borderY = 6;
    int numTilesX = 2;
    int numTilesY = 3; 
    const int tileWidth = 30;
    const int tileHeight = 30;
    for (auto & parameter : {"TileWidth", "TileJumpX"})
    {
        QCOMPARE(filter.getParameters().findParameter(parameter).getValue().convert<int>(), tileWidth);
    }
    for (auto & parameter : {"TileHeight", "TileJumpY"})
    {
        QCOMPARE(filter.getParameters().findParameter(parameter).getValue().convert<int>(), tileHeight);
    }
    

    
    int position = 0;
    unsigned int counter = 0;
    for (unsigned int n = 0; n < 10;  ++n)
    {   
        
        //create a source image 600x600, with squares of uniform intensity 30 pixel wide
        BImage oSensorImage = genSquaresImagePattern(30,30,20,20); 
        BImage oRoi;
        unsigned char firstTileIntensity;
        
        enum modeEnum {eUniform, eNotUniform};
        for (modeEnum mode : {eUniform, eNotUniform})
        {
            
            bool useEmptyImage = false;
            
            //prepare a ROI that fits numTilesX x numTilesY tiles, plus a border, such that each tile will fall in a square of uniform intensity
            int roiStartX = tileWidth - borderX;
            int roiStartY = tileHeight - borderY;
            int roiWidth = tileWidth*numTilesX + 2*borderX;
            int roiHeight = tileHeight*numTilesY + 2*borderY;
            
            if (mode == eUniform)
            {
                if (n % 5 == 0)
                {
                    useEmptyImage = true;
                    oRoi = BImage();
                }
                else
                {
                    oRoi = BImage(oSensorImage, Rect{ roiStartX , roiStartY, roiWidth, roiHeight }, true );
                    {
                        //verify that the tiles will be overlap the image area of uniform intensity
                        QCOMPARE(oRoi.width() / tileWidth , numTilesX);
                        QCOMPARE(oRoi.height() / tileHeight, numTilesY );
                        precitec::geo2d::Point firstPixel {borderX, borderY}; 
                        precitec::geo2d::Point lastPixel {borderX+tileWidth-1, borderY+tileHeight-1};
                        firstTileIntensity = oRoi.getValue(firstPixel.x, firstPixel.y);
                        QCOMPARE(oRoi.getValue(lastPixel.x, lastPixel.y), firstTileIntensity);
                        QVERIFY(oRoi.getValue(lastPixel.x+1, lastPixel.y) != firstTileIntensity);
                        QVERIFY(oRoi.getValue(lastPixel.x, lastPixel.y+1) != firstTileIntensity);
                        
                    }
                }
            }
            else
            {
                    //extract such that each tile will fall on TWO squares of uniform intensity
                    oRoi = BImage(oSensorImage, Rect{ roiStartX +2, roiStartY+2, roiWidth, roiHeight}, true );
                    {
                        //verify that the tiles will be overlap the image area of uniform intensity
                        QCOMPARE(oRoi.width() / tileWidth , numTilesX);
                        QCOMPARE(oRoi.height() / tileHeight, numTilesY );
                        precitec::geo2d::Point firstPixel {borderX, borderY}; 
                        precitec::geo2d::Point lastPixel {borderX+tileWidth-1, borderY+tileHeight-1};
                        firstTileIntensity = oRoi.getValue(firstPixel.x, firstPixel.y);
                        QVERIFY(oRoi.getValue(lastPixel.x, lastPixel.y) != firstTileIntensity);                        
                    }
            }
            
            precitec::interface::ImageContext context;
            context.setImageNumber(counter);
            context.setPosition(position +=2);
            auto inFrame = precitec::interface::ImageFrame {
                context, 
                oRoi,
                precitec::interface::ResultType::AnalysisOK, 
            };
            
            QCOMPARE(dummyFilter.getProceedCalled(), counter);
            
            inImagePipe.signal(inFrame);
            
            QCOMPARE(dummyFilter.getProceedCalled(), counter+1);
            counter++;
            
            // the result is a frame with one element
            auto & rGeoSurfaceInfoIn = outPipe->read(counter);
            //copy surfaceinfo
            auto info = rGeoSurfaceInfoIn.ref().getData()[0];
            QCOMPARE (info.usesMean, true);
            QCOMPARE (info.usesMinMaxDistance, true);
            QCOMPARE (info.usesRelBrightness, false);
            QCOMPARE (info.usesVariation, false);
            QCOMPARE (info.usesSurface, false);
            QCOMPARE (info.usesSurfaceX, false);
            QCOMPARE (info.usesSurfaceY, false);
            QCOMPARE (info.usesTexture, false);
            QCOMPARE (info.usesStructure, false);

            TileContainer tileContainer = info.getTileContainer();
            if (useEmptyImage)
            {
                QCOMPARE(tileContainer.m_sizeX, 0);
                QCOMPARE(tileContainer.m_sizeY, 0); 
            }
            else
            {       
            
                QCOMPARE(tileContainer.m_sizeX, numTilesX);
                QCOMPARE(tileContainer.m_sizeY, numTilesY);
                if (mode == eUniform)
                {
                    QCOMPARE(tileContainer.getSingleTile(0,0).m_MeanValue, double(firstTileIntensity));
                }
                else
                {
                    QVERIFY(tileContainer.getSingleTile(0,0).m_MeanValue != double(firstTileIntensity));
                }
                
                for (int j = 0; j < tileContainer.m_sizeY; j++)
                {
                    for (int i = 0; i < tileContainer.m_sizeX; i++)
                    {
                        SingleTile tile = tileContainer.getSingleTile(i, j);
                        QCOMPARE(tile.m_startX , i*30+borderX);
                        QCOMPARE(tile.m_startY , j*30+borderY);
                        QCOMPARE(tile.m_width, 30);
                        QCOMPARE(tile.m_height, 30);
                        
                        QVERIFY(tile.m_isMeanValid);
                        if (mode == eUniform)
                        {
                            QCOMPARE(tile.m_MeanValue, double(oRoi.getValue( tile.m_startX, tile.m_startY)) );
                            QCOMPARE(tile.m_MinMaxDistance , 0.0);
                        }
                        else
                        {
                            QVERIFY(tile.m_MeanValue != double(oRoi.getValue( tile.m_startX, tile.m_startY)) );
                            QVERIFY(tile.m_MinMaxDistance != 0.0);                            
                        }
                    }
                }
            }    
        }
    }    
}


void TestSurfaceCalculator::testSmallROI_data()
{
    QTest::addColumn<int>("roiWidth");
    QTest::addColumn<int>("roiHeight");
    QTest::addColumn<bool>("ensureTile");
    QTest::addColumn<int>("expectedTilesX");  //assuming tileWidth = 30
    QTest::addColumn<int>("expectedTilesY");  //assuming tileHeight = 30
    
    QTest::newRow("") <<  0 <<  0 << false << 0 << 0;
    QTest::newRow("") << 29 << 29 << false << 0 << 0;
    QTest::newRow("") << 30 << 30 << false << 1 << 1;
    QTest::newRow("") << 29 << 70 << false << 0 << 0;
    QTest::newRow("") << 70 << 29 << false << 0 << 0;
    
    QTest::newRow("") << 30 << 30 << true  << 1 << 1;
    QTest::newRow("") << 20 << 70 << true  << 1 << 2;
    QTest::newRow("") << 70 << 1  << true  << 2 << 1;
    QTest::newRow("") << 70 << 0  << true  << 0 << 0;
    QTest::newRow("") <<  0 <<  0 << true  << 0 << 0;
    QTest::newRow("") <<  0 << 70 << true  << 0 << 0;

}

void TestSurfaceCalculator::testSmallROI()
{
    
    QFETCH(int,roiWidth);
    QFETCH(int,roiHeight);
    QFETCH(bool,ensureTile);
    QFETCH(int,expectedTilesX);
    QFETCH(int,expectedTilesY);
    
    SurfaceCalculator filter;
    fliplib::NullSourceFilter sourceFilter;
    fliplib::SynchronePipe<precitec::interface::ImageFrame> inImagePipe { &sourceFilter, precitec::interface::SensorFilterInterface::SENSOR_IMAGE_FRAME_PIPE };
    
    int group = 0;
    //inImagePipe.setTag("image");
    QVERIFY(filter.connectPipe(&inImagePipe, group));
    
    DummyFilter dummyFilter;
    QVERIFY(dummyFilter.connectPipe(filter.findPipe("SurfaceInfo"), 0));
    
    // access the out pipe data 
    auto outPipe = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoSurfaceInfoarray>*>(filter.findPipe("SurfaceInfo"));
    QVERIFY(outPipe);

    std::unique_ptr<precitec::image::OverlayCanvas> pcanvas {new precitec::image::OverlayCanvas};
    filter.setCanvas(pcanvas.get());
    
    filter.getParameters().update(std::string("Verbosity"), fliplib::Parameter::TYPE_int, 6);
    filter.getParameters().update(std::string("CalcMean"), fliplib::Parameter::TYPE_bool, true);
    filter.getParameters().update(std::string("EnsureTile"), fliplib::Parameter::TYPE_bool, ensureTile);
    filter.setParameter();

    int position = 0;
    unsigned int counter = 0;
    
    BImage oRoi = genSquaresImagePattern(1,1,roiWidth,roiHeight); 
    QVERIFY(oRoi.width() == roiWidth && oRoi.height() == roiHeight);
    

    precitec::interface::ImageContext context;
    context.setImageNumber(counter);
    context.setPosition(position);
    auto inFrame = precitec::interface::ImageFrame {
        context, 
        oRoi,
        precitec::interface::ResultType::AnalysisOK, 
    };
    
    QCOMPARE(dummyFilter.getProceedCalled(), counter);
    
    inImagePipe.signal(inFrame);
    
    QCOMPARE(dummyFilter.getProceedCalled(), counter+1);
    counter++;
    
    // the result is a frame with one element
    auto & rGeoSurfaceInfoIn = outPipe->read(counter);
    //copy surfaceinfo
    auto info = rGeoSurfaceInfoIn.ref().getData()[0];

    TileContainer tileContainer = info.getTileContainer();

    QCOMPARE(tileContainer.m_sizeX, expectedTilesX);
    QCOMPARE(tileContainer.m_sizeY, expectedTilesY); 
    
}

QTEST_MAIN(TestSurfaceCalculator)
#include "testSurfaceCalculator.moc"

