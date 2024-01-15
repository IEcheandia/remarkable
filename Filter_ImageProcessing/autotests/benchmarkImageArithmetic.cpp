#include <QTest>

#include "../imageArithmetic.h" 
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

using precitec::filter::ImageArithmetic;
using precitec::image::genModuloPattern;
using precitec::image::BImage;
using precitec::analyzer::ProductData;


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
    
    void resetProceedCalled() {
        m_proceedCalled = 0;
    }

private:
    unsigned int m_proceedCalled = 0;
};


class BenchmarkImageArithmetic : public QObject
{
    Q_OBJECT
private:
	ProductData m_oExternalProductData = ProductData {
		0, //seam series
		0, //seam
		30, //m_pActiveSeam->m_oVelocity,
		1 //m_pActiveSeam->m_oTriggerDelta,
    };	
	std::vector<BImage> m_inputImage;

private Q_SLOTS:
	void initTestCase();
    void benchmarkProceed_data();
    void benchmarkProceed();
    
    void benchmarkProceedSimple_data();
    void benchmarkProceedSimple();
};


void BenchmarkImageArithmetic::initTestCase()
{
      
    m_inputImage = std::vector<BImage> {
            precitec::image::genModuloPattern({1024,1024},50),
            precitec::image::genModuloPattern({1024, 1024},60),
            precitec::image::genModuloPattern({1024, 1024},70),
            precitec::image::genModuloPattern({1024, 1024},9),
            precitec::image::genModuloPattern({100, 30},79),  //different size, reset buffer
            precitec::image::genModuloPattern({1024, 1024},60),  //different size, reset buffer
            precitec::image::genModuloPattern({1024, 1024},50),
            precitec::image::genModuloPattern({1024, 1024},10)
    };
    

	
}

void BenchmarkImageArithmetic::benchmarkProceed_data()
{
    QTest::addColumn<int>("param_Operation");
    QTest::addColumn<int>("param_TimeWindow");
    QTest::addColumn<int>("param_ResolutionX");
    QTest::addColumn<int>("param_ResolutionY");
    QTest::addColumn<int>("param_OutputMinValue");
    QTest::addColumn<int>("param_OutputMaxValue");
    QTest::addColumn<bool>("param_RescalePixelIntensity");
    QTest::addColumn<bool>("param_InvertLUT");
    QTest::addColumn<bool>("applyROI");

    const std::vector<int> testWindow{1,10};
    const std::vector<int> testResolutionX{1};
    const std::vector<int> testResolutionY{1};
    const std::vector<int> testoutputValueRange{255};
    const std::vector<bool> testRescale{true, false};
    const std::vector<bool> testInvert{false};
    
    for (int operation = 0; operation < 9; operation++)
    {
        int counter = 0;
        for (int window : testWindow)
        {
            int resolutionX = 1;
          //  for (int resolutionX: testResolutionX)
            {
                int resolutionY = resolutionX;
                //for (int resolutionY: testResolutionX)
                {
                    int outputValueRange = 255;
                   // for (int outputValueRange: testoutputValueRange)
                    {
                        
                        for (bool rescale: testRescale)
                        {
                            bool invert = false;
                            //for (bool invert: testInvert)
                            {                        
                                for (bool useRoi: {true, false})
                                {
                                    std::string testName = "Operation_" + std::to_string(operation) + "_" + std::to_string(counter);
                                    std::cout << testName;
                                    int minValue = (255 - outputValueRange)/2;
                                    int maxValue = minValue + outputValueRange;
                                    QTest::newRow(testName.c_str()) << operation << window << resolutionX << resolutionY
                                        << minValue << maxValue << rescale << invert << useRoi;
                                        counter ++;
                                }
                                
                            }
                        }
                    }
                }
            }
        }
    }
        
};
    
void BenchmarkImageArithmetic::benchmarkProceed()
{
    
     // redirect redirectWmLogToNull
        extern FILE * g_pLoggerStream;
        extern bool g_oLoggerStreamOpened;
        if (g_oLoggerStreamOpened)
        {
            fclose(g_pLoggerStream);
            g_oLoggerStreamOpened = false;
        }
        g_pLoggerStream = fopen("/dev/null", "w");
        g_oLoggerStreamOpened = true;
    
    
    using precitec::geo2d::Doublearray;
    using precitec::image::Sample;
    
    QFETCH(int, param_Operation);
    QFETCH(int, param_TimeWindow);
    QFETCH(int, param_ResolutionY);
    QFETCH(int, param_ResolutionX);
    QFETCH(int,param_OutputMinValue);
    QFETCH(int,param_OutputMaxValue);
    QFETCH(bool,param_RescalePixelIntensity);
    QFETCH(bool,param_InvertLUT);
    QFETCH(bool,applyROI);
    
    std::cout << "Operation " << param_Operation << " w " << param_TimeWindow << " res " << param_ResolutionX << ", " << param_ResolutionY
    << " output range " << param_OutputMinValue << "," << param_OutputMaxValue << " rescale " << param_RescalePixelIntensity 
    << " invert " << param_InvertLUT << "apply roi " << applyROI << std::endl;

    
    // a null source filter connected with ImageArithmetic filter, connected with DummyFilter
    ImageArithmetic oFilter;
    DummyFilter dummyFilter;
    
    oFilter.setExternalData(&m_oExternalProductData);
    
    

    std::unique_ptr<precitec::image::OverlayCanvas> pcanvas {new precitec::image::OverlayCanvas};
    oFilter.setCanvas(pcanvas.get());
    
    
    // parameterize the filter
    
    oFilter.getParameters().update(std::string("Verbosity"), fliplib::Parameter::TYPE_int, 1);
    oFilter.getParameters().update(std::string("TimeWindow"), fliplib::Parameter::TYPE_UInt32, param_TimeWindow);
    oFilter.getParameters().update(std::string("Operation"), fliplib::Parameter::TYPE_int, param_Operation);
    oFilter.getParameters().update(std::string("ResolutionX"), fliplib::Parameter::TYPE_UInt32, param_ResolutionX);
    oFilter.getParameters().update(std::string("ResolutionY"), fliplib::Parameter::TYPE_UInt32, param_ResolutionY);
    oFilter.getParameters().update(std::string("OutputMinValue"), fliplib::Parameter::TYPE_UInt32, param_OutputMinValue);
    oFilter.getParameters().update(std::string("OutputMaxValue"), fliplib::Parameter::TYPE_UInt32, param_OutputMaxValue);
    oFilter.getParameters().update(std::string("RescalePixelIntensity"), fliplib::Parameter::TYPE_bool, param_RescalePixelIntensity);
    oFilter.getParameters().update(std::string("InvertLUT"), fliplib::Parameter::TYPE_bool, param_InvertLUT);

    
    oFilter.setParameter();

    // and process

    int position = 300;
    
  
    fliplib::NullSourceFilter sourceFilter;
    fliplib::SynchronePipe<precitec::interface::ImageFrame> inImagePipe { &sourceFilter, precitec::interface::SensorFilterInterface::SENSOR_IMAGE_FRAME_PIPE };
    
    int group = 0;
    //inImagePipe.setTag("data");
    QVERIFY(oFilter.connectPipe(&inImagePipe, group));
    
    QVERIFY(dummyFilter.connectPipe(oFilter.findPipe("ImageFrame"), 0));
    
    // access the out pipe data of the ImageArithmetic filter
    auto outPipe = dynamic_cast<fliplib::SynchronePipe<precitec::interface::ImageFrame>*>(oFilter.findPipe("ImageFrame"));
    QVERIFY(outPipe);

    
    precitec::interface::ImageContext context;
    
    QBENCHMARK{
        
        const unsigned int n = m_inputImage.size();
        for (unsigned int counter = 0; counter < 200; ++counter)
        {   
            if (counter%100 == 0)
            {                
                //dummyFilter.resetProceedCalled();
                oFilter.arm(precitec::filter::ArmState::eSeamStart);
            }
            
             //dummyFilter.resetProceedCalled();
          
            context.setImageNumber(counter);
            context.setPosition(position);
            
            auto & rSourceImage = m_inputImage[counter % n];
            BImage image = applyROI ? BImage{ rSourceImage, 
                precitec::geo2d::Rect(10,10, rSourceImage.width()/2, rSourceImage.height()/2)} : rSourceImage;
            
            auto inPipe = precitec::interface::ImageFrame {
                context, 
                image,
                precitec::interface::ResultType::AnalysisOK, 
            };
            
            //QCOMPARE(dummyFilter.getProceedCalled(), counter);
            inImagePipe.signal(inPipe);
            //QCOMPARE(dummyFilter.getProceedCalled(), counter+1);

            // the result is a frame with one element
            auto output = outPipe->read(counter).data();
            output.isValid();
        }
    }
    
}


void BenchmarkImageArithmetic::benchmarkProceedSimple_data()
{
    QTest::addColumn<int>("param_Operation");
    QTest::addColumn<int>("param_TimeWindow");
    QTest::addColumn<bool>("param_RescalePixelIntensity");
    QTest::addColumn<bool>("applyROI");

    const std::vector<int> testWindow{1,10};
    const std::vector<int> testResolutionX{1};
    const std::vector<int> testResolutionY{1};
    const std::vector<int> testoutputValueRange{255};
    const std::vector<bool> testRescale{true, false};
    const std::vector<bool> testInvert{false};
    
    for (auto  && operation : {
                int(precitec::filter::Operations::ePixelSum), int(precitec::filter::Operations::ePixelRange), int(precitec::filter::Operations::eRepeat)})
    {
        int counter = 0;
        for (int window : testWindow)
        {
                  
            for (bool rescale: testRescale)
            {
                for (bool useRoi: {true, false})
                {
                    std::string testName = "Operation_" + std::to_string(operation) + "_w" + std::to_string(window) + "_stretch" +std::to_string(rescale) ;
                    if (useRoi)
                    {
                        testName += "roi";
                    }
                    
                    QTest::newRow(testName.c_str()) << operation << window << rescale << useRoi;
                        counter ++;
                }                
            }
        }
    }
};
    
void BenchmarkImageArithmetic::benchmarkProceedSimple()
{
    
     // redirect redirectWmLogToNull
        extern FILE * g_pLoggerStream;
        extern bool g_oLoggerStreamOpened;
        if (g_oLoggerStreamOpened)
        {
            fclose(g_pLoggerStream);
            g_oLoggerStreamOpened = false;
        }
        g_pLoggerStream = fopen("/dev/null", "w");
        g_oLoggerStreamOpened = true;
    
    
    using precitec::geo2d::Doublearray;
    using precitec::image::Sample;
    
    QFETCH(int, param_Operation);
    QFETCH(int, param_TimeWindow);
    //QFETCH(int, param_ResolutionY);
   // QFETCH(int, param_ResolutionX);
    //QFETCH(int,param_OutputMinValue);
    //QFETCH(int,param_OutputMaxValue);
    QFETCH(bool,param_RescalePixelIntensity);
    //QFETCH(bool,param_InvertLUT);
    QFETCH(bool,applyROI);
   
    /*
    std::cout << "Operation " << param_Operation << " w " << param_TimeWindow 
    << " rescale " << param_RescalePixelIntensity 
    << "apply roi " << applyROI << std::endl;
*/
    
    // a null source filter connected with ImageArithmetic filter, connected with DummyFilter
    ImageArithmetic oFilter;
    DummyFilter dummyFilter;
    
    oFilter.setExternalData(&m_oExternalProductData);
    
    

    std::unique_ptr<precitec::image::OverlayCanvas> pcanvas {new precitec::image::OverlayCanvas};
    oFilter.setCanvas(pcanvas.get());
    
    
    // parameterize the filter
    
    oFilter.getParameters().update(std::string("Verbosity"), fliplib::Parameter::TYPE_int, 1);
    oFilter.getParameters().update(std::string("TimeWindow"), fliplib::Parameter::TYPE_UInt32, param_TimeWindow);
    oFilter.getParameters().update(std::string("Operation"), fliplib::Parameter::TYPE_int, param_Operation);
    oFilter.getParameters().update(std::string("RescalePixelIntensity"), fliplib::Parameter::TYPE_bool, param_RescalePixelIntensity);
    
    
    oFilter.setParameter();

    // and process

    int position = 300;
    
  
    fliplib::NullSourceFilter sourceFilter;
    fliplib::SynchronePipe<precitec::interface::ImageFrame> inImagePipe { &sourceFilter, precitec::interface::SensorFilterInterface::SENSOR_IMAGE_FRAME_PIPE };
    
    int group = 0;
    //inImagePipe.setTag("data");
    QVERIFY(oFilter.connectPipe(&inImagePipe, group));
    
    QVERIFY(dummyFilter.connectPipe(oFilter.findPipe("ImageFrame"), 0));
    
    // access the out pipe data of the ImageArithmetic filter
    auto outPipe = dynamic_cast<fliplib::SynchronePipe<precitec::interface::ImageFrame>*>(oFilter.findPipe("ImageFrame"));
    QVERIFY(outPipe);

    
    precitec::interface::ImageContext context;
    
    QBENCHMARK{
        
        const unsigned int n = m_inputImage.size();
        for (unsigned int counter = 0; counter < 200; ++counter)
        {   
            if (counter%100 == 0)
            {                
                //dummyFilter.resetProceedCalled();
                oFilter.arm(precitec::filter::ArmState::eSeamStart);
            }
            
             //dummyFilter.resetProceedCalled();
          
            context.setImageNumber(counter);
            context.setPosition(position);
            
            auto & rSourceImage = m_inputImage[counter % n];
            BImage image = applyROI ? BImage{ rSourceImage, 
                precitec::geo2d::Rect(10,10, rSourceImage.width()/4, rSourceImage.height()/3)} : rSourceImage;
            
            auto inPipe = precitec::interface::ImageFrame {
                context, 
                image,
                precitec::interface::ResultType::AnalysisOK, 
            };
            
            //QCOMPARE(dummyFilter.getProceedCalled(), counter);
            inImagePipe.signal(inPipe);
            //QCOMPARE(dummyFilter.getProceedCalled(), counter+1);

            // the result is a frame with one element
            auto output = outPipe->read(counter).data();
            output.isValid();
        }
    }
    
}



QTEST_MAIN(BenchmarkImageArithmetic)
#include "benchmarkImageArithmetic.moc"
