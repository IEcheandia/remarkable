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
#include <algorithm>

using precitec::filter::ImageArithmetic;
using precitec::image::genModuloPattern;
using precitec::image::BImage;
using precitec::analyzer::ProductData;
using precitec::geo2d::Point;
using precitec::geo2d::Rect;
using precitec::geo2d::Size;
using precitec::geo2d::Doublearray;
using precitec::image::Sample;




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


struct DummyInput
{       
    fliplib::NullSourceFilter m_sourceFilter;
    fliplib::SynchronePipe<precitec::interface::ImageFrame> m_imagePipe;
    
    precitec::interface::ImageFrame m_frame;   
	std::vector<BImage> m_inputImages;
    
    DummyInput(std::vector<BImage> inputImages)
    : m_imagePipe{&m_sourceFilter, "image_id"}
    , m_inputImages(inputImages)
    {
        //no tag needed
    }
    
    bool connectToFilter(fliplib::BaseFilter * pFilter)
    {
        int group =0;
        //connect  pipes
        bool ok = pFilter->connectPipe(&(m_imagePipe), group);
        return ok;
    }
    
    precitec::interface::GeoDoublearray createGeoDoubleArray(precitec::interface::ImageContext baseContext, double value, int rank )
    {
        using namespace precitec::interface;
        return GeoDoublearray( ImageContext{baseContext}, 
                                   precitec::geo2d::Doublearray{1, value, rank}, 
                                   ResultType::AnalysisOK, Limit);
    }
    
    void fillImage( int counter )
    {
        
        precitec::interface::ImageContext context;
        context.setImageNumber(counter);
        context.setPosition(counter*10);
 
        m_frame = precitec::interface::ImageFrame{
            context,
            m_inputImages[counter],
            precitec::interface::ResultType::AnalysisOK
        };            
        
        
    }
    
    BImage fillROI( int counter, const Point & rROIOffset )
    {
        
        precitec::interface::ImageContext contextSource;
        contextSource.setImageNumber(counter);
        contextSource.setPosition(counter*10);
 
        const auto & rImageSource = m_inputImages[counter];
        BImage oROI(rImageSource, intersect(rImageSource.size(), Rect(rROIOffset,Size{90, 5})));
        
        precitec::LinearTrafo oSubTrafo(rROIOffset);
        precitec::interface::ImageContext contextROI{contextSource, oSubTrafo(contextSource.trafo())};
        
        m_frame = precitec::interface::ImageFrame{
            contextROI, 
            oROI,
            precitec::interface::ResultType::AnalysisOK
        }; 
        return oROI;
    }

    void signal()
    {
        m_imagePipe.signal(m_frame);
    }
};


class TestImageArithmetic : public QObject
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
	std::vector<int> m_inputPixelValue;
	unsigned int m_testX = 80;
	unsigned int m_testY = 29; //last line
    const precitec::image::Size2d m_testSquareImageSize{5,6};
private Q_SLOTS:
	void init();
	void initTestCase();
    void testCtor();
    void testProceed_data();
    void testProceed(); 
    void testProceedSubRoi_data();
    void testProceedSubRoi(); 
    void testEmptyImage_data();
    void testEmptyImage();
    void testSampling_data();
    void testSampling();
};

	


void TestImageArithmetic::init()
{
        
    m_inputImage = std::vector<BImage> {
                precitec::image::genModuloPattern({100,30},50),
                precitec::image::genModuloPattern({100,30},60),
                precitec::image::genModuloPattern({100,30},70),
                precitec::image::genModuloPattern({100,30},9),
                precitec::image::genModuloPattern({90 ,30},79),  //different size, reset buffer
                precitec::image::genModuloPattern({100,30},60),  //different size, reset buffer
                precitec::image::genModuloPattern({100,30},50),
                precitec::image::genModuloPattern({100,30},10)
        };
    
	
	
    m_inputPixelValue = std::vector<int> {
									30, 
                                    20, 
                                    10, 
                                    8,
                                    1, //different size, reset buffer
                                    20, //different size, reset buffer
                                    30,
                                    0
    };
}

void TestImageArithmetic::initTestCase()
{    
		
	//verify that the values of in m_inputPixelValue are correct, and that we are actually using modulo images
    for (unsigned int i = 0; i < m_inputPixelValue.size(); i++)
    {
        QCOMPARE(int(m_inputImage[i].getValue(m_testX,m_testY)), m_inputPixelValue[i]);
        for (int x = 0; x < 9 ; ++x)
        {
            QCOMPARE(int(m_inputImage[i].getValue(x,10)), x);
        }
    }
    
	
}

void TestImageArithmetic::testCtor()
{
    ImageArithmetic filter;
    QCOMPARE(filter.name(), std::string("ImageArithmetic"));
    QVERIFY(filter.findPipe("ImageFrame") != nullptr);
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);
    
    //check parameters of type UInt32
    
    std::vector< std::tuple<std::string, unsigned int>> parametersUInt32 = 
    {
            {"ResolutionX", 1},
            {"ResolutionY", 1},
            {"TimeWindow",  2},
            {"OutputMinValue", 0},
            {"OutputMaxValue", 255}
        };
    
    for (auto paramTuple: parametersUInt32)
    {
        std::string paramName = std::get<0>(paramTuple);
        unsigned int defaultValue = std::get<1>(paramTuple);
        
        QVERIFY(filter.getParameters().exists(paramName));
        QCOMPARE(filter.getParameters().findParameter(paramName).getType(), fliplib::Parameter::TYPE_UInt32);
        QCOMPARE(filter.getParameters().findParameter(paramName).getValue().convert<unsigned int>(), defaultValue);
    }   
    
    //check parameters of type bool
        
    std::vector< std::tuple<std::string, bool>> parametersBool = 
    {
            {"RescalePixelIntensity", false},
            {"PassThroughBadRank", true},
            {"InvertLUT", false},
            {"ResetOnSeamStart", true},
            {"ResampleOutput", true}
        };
    
    for (auto paramTuple: parametersBool)
    {
        std::string paramName = std::get<0>(paramTuple);
        bool defaultValue = std::get<1>(paramTuple);
        
        QVERIFY(filter.getParameters().exists(paramName));
        QCOMPARE(filter.getParameters().findParameter(paramName).getType(), fliplib::Parameter::TYPE_bool);
        QCOMPARE(filter.getParameters().findParameter(paramName).getValue().convert<bool>(), defaultValue);
    }   
    std::string paramName = "Operation";
    QVERIFY(filter.getParameters().exists(paramName));
    QCOMPARE(filter.getParameters().findParameter(paramName).getType(), fliplib::Parameter::TYPE_int);
    QCOMPARE(filter.getParameters().findParameter(paramName).getValue().convert<int>(), 9);

    paramName = "StartImage";
    QVERIFY(filter.getParameters().exists(paramName));
    QCOMPARE(filter.getParameters().findParameter(paramName).getType(), fliplib::Parameter::TYPE_int);
    QCOMPARE(filter.getParameters().findParameter(paramName).getValue().convert<int>(), 0);
}

void TestImageArithmetic::testProceed_data()
{
    QTest::addColumn<int>("param_Operation");
    QTest::addColumn<int>("param_TimeWindow");
    QTest::addColumn<int>("param_ResolutionY");
    QTest::addColumn<std::vector<int>>("expectedResult");
    QTest::addColumn<bool>("test_preserves_ramp");
    
    QTest::newRow("Sum")
        << 0 //Param_Operation
        << 3 //Param_TimeWindow
        << 1 //Param_ResolutionY
        << std::vector<int> {30, //sum(30)
                             50, //sum(30,20)
                             60, //sum(30,20,10) 
                             38, //sum(20,10,8)
                             1, //different size, reset buffer
                             20, //different size, reset buffer sum(20)
                             50, // sum(20,30)
                             50  // sum(20,30,0)
    } // expectedResult
    << false; //test_preserves_ramp
        
    QTest::newRow("SumWithMovingWindow")
        << 0 //Param_Operation
        << 4 //Param_TimeWindow
        << 1 //Param_ResolutionY
        << std::vector<int> {30, //sum(30)
                             50, //sum(30,20)
                             60, //sum(30,20,10) 
                             68, //sum(30,20,10,8)
                             1, //different size, reset buffer
                             20, //different size, reset buffer sum(20)
                             50, // sum(20,30)
                             50  // sum(20,30,0)
    } // expectedResult
    << false; //test_preserves_ramp
    
    QTest::newRow("Diff")
        << 1 //Param_Operation
        << 2 //Param_TimeWindow
        << 1 //Param_ResolutionY
        << std::vector<int> {30, //(30)
                             0, //(30,20) -10
                             0, //(20,10) -30 
                             0, //(10,8) -2 
                             1, //different size, reset buffer (1)
                             20, //different size, reset buffer (20)
                             10, // (20,30)
                             0  // (30,0) -30
    } // expectedResult
    << false; //test_preserves_ramp
    
    QTest::newRow("Min")
        << 2 //Param_Operation
        << 3 //Param_TimeWindow
        << 1 //Param_ResolutionY
        << std::vector<int> {30, //(30)
                             20, //(30,20)
                             10, //(30,20,10) 
                             8, //(20,10,8) 
                             1, //different size, reset buffer (1)
                             20, //different size, reset buffer (20)
                             20, // (20,30)
                             0  // (20,30,0) 
    } // expectedResult
    << true; //test_preserves_ramp
        
    QTest::newRow("Max")
        << 3 //Param_Operation
        << 3 //Param_TimeWindow
        << 1 //Param_ResolutionY
        << std::vector<int> {30, //(30)
                             30, //(30,20)
                             30, //(30,20,10) 
                             20, //(20,10,8) 
                             1, //different size, reset buffer (1)
                             20, //different size, reset buffer (20)
                             30, // (20,30)
                             30  // (20,30,0) 
    } // expectedResult
    << true; //test_preserves_ramp
    
        
    QTest::newRow("Mean")
        << 4 //Param_Operation
        << 3 //Param_TimeWindow
        << 1 //Param_ResolutionY
        << std::vector<int> {30, //(30)
                             25, //(30,20)
                             20, //(30,20,10) 
                             12, //(20,10,8) =12.67
                             1, //different size, reset buffer (1)
                             20, //different size, reset buffer (20)
                             25, // (20,30)
                             16  // (20,30,0) = 16.67
    } // expectedResult
    << false; //test_preserves_ramp
    

    QTest::newRow("Mean_Sampling")
        << 4 //Param_Operation
        << 3 //Param_TimeWindow
        << 2 //Param_ResolutionY
        << std::vector<int> {30, //(30)
                             25, //(30,20)
                             20, //(30,20,10)
                             12, //(20,10,8) =12.67
                             1, //different size, reset buffer (1)
                             20, //different size, reset buffer (20)
                             25, // (20,30)
                             16  // (20,30,0) = 16.67
    } // expectedResult
    << false; //test_preserves_ramp
    
            
    QTest::newRow("StdDev")
        << 5 //Param_Operation
        << 3 //Param_TimeWindow
        << 1 //Param_ResolutionY
        << std::vector<int> {0, //(30)
                             7, //(30,20)
                             10, //(30,20,10) 
                             6, //(20,10,8) 
                             0, //different size, reset buffer (1)
                             0, //different size, reset buffer (20)
                             7, // (20,30)
                             15  // (20,30,0) 
    } // expectedResult
    << false; //test_preserves_ramp
    
    QTest::newRow("Range")
        << 6 //Param_Operation
        << 3 //Param_TimeWindow
        << 1 //Param_ResolutionY
        << std::vector<int> {0, //(30)
                             10, //(30,20)
                             20, //(30,20,10) 
                             12, //(20,10,8) 
                             0, //different size, reset buffer (1)
                             0, //different size, reset buffer (20)
                             10, // (20,30)
                             30  // (20,30,0) 
    } // expectedResult
    << false; //test_preserves_ramp
    
    //InvalidOperation: just copy the input
    QTest::newRow("InvalidOperation")
        << 999 //Param_Operation
        << 3 //Param_TimeWindow
        << 1 //Param_ResolutionY
        << std::vector<int> {30, //(30)
                             20, //(30,20)
                             10, //(30,20,10) 
                             8, //(20,10,8) 
                             1, //different size, reset buffer (1)
                             20, //different size, reset buffer (20)
                             30, // (20,30)
                             0  // (20,30,0) 
    } // expectedResult
        << true; //test_preserves_ramp

        
    
    QTest::newRow("Median")
        << 7 //Param_Operation
        << 3 //Param_TimeWindow
        << 1 //Param_ResolutionY
        << std::vector<int> {30, //median(30)
                             30, //median(30,20)
                             20, //median(30,20,10) 
                             10, //median(20,10,8)
                             1, //different size, reset buffer (1)
                             20, //different size, reset buffer  median(20)
                             30, // median(20,30)
                             20  // median(20,30,0)
    } // expectedResult
    << true; //test_preserves_ramp
    
    
    // all operations with window = 1
    
    for (int operation = 0; operation < 9; operation++)
    {
        std::string testName = "SingleImage_" + std::to_string(operation);
        
        //the operations Range  and Std deviation give a blank image with window=1
        if (operation == 5 || operation == 6)
        {
            QTest::newRow(testName.c_str() )  << operation << 1 << 1 
            << std::vector<int> {0, 0, 0, 0, 0, 0, 0, 0} // expectedResult
            << false; //test_preserves_ramp
        }
        else
        {
            QTest::newRow(testName.c_str() )  << operation << 1 << 1
            << m_inputPixelValue // expectedResult
            << true; //test_preserves_ramp
        }
    }
    
        
};


void TestImageArithmetic::testProceed()
{
    
    QFETCH(std::vector<int>, expectedResult);
    QFETCH(int, param_Operation);
    QFETCH(int, param_TimeWindow);
    QFETCH(int, param_ResolutionY);
    QFETCH(bool, test_preserves_ramp);

    
    // a null source filter connected with ImageArithmetic filter, connected with DummyFilter
    DummyInput dummyInput(m_inputImage);
    
    ImageArithmetic oFilter;
    DummyFilter dummyFilter;
    
    
    //another imageArithmetic filter, that with m_oIntensityRescale = true
    ImageArithmetic oRescalingFilter;
    DummyFilter dummyFilter_rescaling;
    
    QVERIFY(dummyInput.connectToFilter(&oFilter));
    QVERIFY(dummyInput.connectToFilter(&oRescalingFilter));
    
    QVERIFY(dummyFilter.connectPipe(oFilter.findPipe("ImageFrame"), 0));
    QVERIFY(dummyFilter_rescaling.connectPipe(oRescalingFilter.findPipe("ImageFrame"), 0));
    
    // access the out pipe data of the ImageArithmetic filter
    auto outPipe = dynamic_cast<fliplib::SynchronePipe<precitec::interface::ImageFrame>*>(oFilter.findPipe("ImageFrame"));
    auto outPipe_rescaling = dynamic_cast<fliplib::SynchronePipe<precitec::interface::ImageFrame>*>(oRescalingFilter.findPipe("ImageFrame"));
    QVERIFY(outPipe);
    QVERIFY(outPipe_rescaling);

    std::unique_ptr<precitec::image::OverlayCanvas> pcanvas {new precitec::image::OverlayCanvas};
    
    // parameterize the filter
    oRescalingFilter.getParameters().update(std::string("RescalePixelIntensity"), fliplib::Parameter::TYPE_bool, true);
    
    //common operations: finish parametrization, set canvas, arm
    for (auto && pFilter: {&oFilter, &oRescalingFilter})
    {
        pFilter->setExternalData(&m_oExternalProductData);

        pFilter->setCanvas(pcanvas.get());
        
        pFilter->getParameters().update(std::string("Verbosity"), fliplib::Parameter::TYPE_int, 1);
        pFilter->getParameters().update(std::string("TimeWindow"), fliplib::Parameter::TYPE_UInt32, param_TimeWindow);
        pFilter->getParameters().update(std::string("Operation"), fliplib::Parameter::TYPE_int, param_Operation);
        pFilter->getParameters().update(std::string("ResolutionY"), fliplib::Parameter::TYPE_UInt32, param_ResolutionY);        
        pFilter->setParameter();

        pFilter->arm(precitec::filter::ArmState::eSeamStart);

    }
    
    QCOMPARE(oFilter.hasCanvas(), true);
    QCOMPARE(oRescalingFilter.hasCanvas(), true);

    QCOMPARE(oFilter.getParameters().findParameter("RescalePixelIntensity").getValue().convert<bool>(), false);
    QCOMPARE(oRescalingFilter.getParameters().findParameter("RescalePixelIntensity").getValue().convert<bool>(), true);

  
    // now signal the pipe, this processes the complete filter graph
    
    for (unsigned int counter = 0; counter < m_inputImage.size(); ++counter)
    {   
        //std::cout << "Image nr " << counter << std::endl;
        dummyInput.fillImage(counter);
        
        QCOMPARE(dummyFilter.getProceedCalled(), counter);
        QCOMPARE(dummyFilter_rescaling.getProceedCalled(), counter);
        
        dummyInput.signal();
        
        QCOMPARE(dummyFilter.getProceedCalled(), counter+1);
        QCOMPARE(dummyFilter_rescaling.getProceedCalled(), counter+1);

        // the result is a frame with one element
        auto rResultImage = outPipe->read(counter).data();

        //print debug messages before failing the test
        if (int(rResultImage.getValue(m_testX,m_testY))!= expectedResult[counter])
        {
            std::cout << "counter " << counter ;
            if (param_ResolutionY == 1)
            {
                auto & rBuffer = oFilter.m_oFrameBuffer;
                std::cout << " buffer content:\n " ;
                
                for(int i = 0; rBuffer.isValidIndex(i); i++)
                {
                    //std::cout << i << ") " ;
                    if (rBuffer.isBufferPositionInitialized(i))
                    {
                        //we can directly access the image in the buffer because resolution is 1
                        std::cout << int(rBuffer.getCachedElement(i).m_image.getValue(m_testX,m_testY));
                    }
                    std::cout << "; ";
                }
            }
            std::cout << std::endl;
        }
        
        QCOMPARE(int(rResultImage.getValue(m_testX,m_testY)), expectedResult[counter]);

        if (test_preserves_ramp)
        {
            //no rescale applied, with the median operation the output image is still a ramp
            for (int x = 0; x < 9 ; ++x)
            {
                QCOMPARE(int(rResultImage.getValue(x,10)), x);
            }
        }
        
       // Now compare the output of the rescaling filter to the same operation without rescale
       // Since this rescaling is done to avoid overflow/underflow, it's not always possible to compare these two 
       //images directly, but *with the input provided here* that is a problem only for the operation ePixelDiff
       
        auto rRescaledResultImage = outPipe_rescaling->read(counter).data();
        
        //check if the image is isContiguos (just to be able to use std algorithms later )
        QVERIFY(rResultImage.isContiguos());
        QVERIFY(rRescaledResultImage.isContiguos());

        
       if (param_Operation == 999 )
       {
           //for an invalid operation, the input image is just copied, we do not test further
           return;
        }
       
        auto extremes1 = std::minmax_element(rResultImage.begin(), rResultImage.end());
        auto extremes2 = std::minmax_element(rRescaledResultImage.begin(), rRescaledResultImage.end());
        
        double range1 = (*extremes1.second) - (*extremes1.first);
        if (range1 != 0)
        {
            QCOMPARE(*(extremes2.first), 0);
            QCOMPARE(*(extremes2.second), 255);
        }   
        
       if (param_Operation == 1)
       {
           //pixel difference, in this case we do not have access to the pixel values before rescaling and we can't test further
           return;
        }
       
        if (range1 != 0)
        {            
            for (auto pPixelOriginal = rResultImage.begin(), pPixelRescaled = rRescaledResultImage.begin(), pEndPixelOriginal = rResultImage.end()  ;
                 pPixelOriginal != pEndPixelOriginal;
            ++pPixelOriginal, ++pPixelRescaled)
            {
                double rescaledValue = (*pPixelOriginal - *extremes1.first) / range1 *255.0;                
                QCOMPARE(*pPixelRescaled, byte(rescaledValue));
            }            
        }
        else
        {
            QCOMPARE((*extremes2.second) - (*extremes2.first), 0);
        }


    }
    
}


void TestImageArithmetic::testProceedSubRoi_data()
{
    QTest::addColumn<int>("param_Operation");
    QTest::addColumn<int>("param_TimeWindow");
    QTest::addColumn<int>("param_ResolutionY");
    QTest::addColumn<std::vector<int>>("expectedResult");
    
    QTest::newRow("Sum")
        << 0 //Param_Operation
        << 3 //Param_TimeWindow
        << 1 //Param_ResolutionY
        << std::vector<int> {30, //sum(30)
                             50, //sum(30,20)
                             60, //sum(30,20,10) 
                             38, //sum(20,10,8)
                             1, //different size, reset buffer
                             20, //different size, reset buffer sum(20)
                             50, // sum(20,30)
                             50  // sum(20,30,0)
    }; // expectedResult
    
    
    QTest::newRow("Diff")
        << 1 //Param_Operation
        << 2 //Param_TimeWindow
        << 1 //Param_ResolutionY
        << std::vector<int> {30, //(30)
                             0, //(30,20) -10
                             0, //(20,10) -30 
                             0, //(10,8) -2 
                             1, //different size, reset buffer (1)
                             20, //different size, reset buffer (20)
                             10, // (20,30)
                             0  // (30,0) -30
    }; // expectedResult
    
    QTest::newRow("Min")
        << 2 //Param_Operation
        << 3 //Param_TimeWindow
        << 1 //Param_ResolutionY
        << std::vector<int> {30, //(30)
                             20, //(30,20)
                             10, //(30,20,10) 
                             8, //(20,10,8) 
                             1, //different size, reset buffer (1)
                             20, //different size, reset buffer (20)
                             20, // (20,30)
                             0  // (20,30,0) 
    }; // expectedResult
        
    QTest::newRow("Max")
        << 3 //Param_Operation
        << 3 //Param_TimeWindow
        << 1 //Param_ResolutionY
        << std::vector<int> {30, //(30)
                             30, //(30,20)
                             30, //(30,20,10) 
                             20, //(20,10,8) 
                             1, //different size, reset buffer (1)
                             20, //different size, reset buffer (20)
                             30, // (20,30)
                             30  // (20,30,0) 
    }; // expectedResult
    
        
    QTest::newRow("Mean")
        << 4 //Param_Operation
        << 3 //Param_TimeWindow
        << 1 //Param_ResolutionY
        << std::vector<int> {30, //(30)
                             25, //(30,20)
                             20, //(30,20,10) 
                             12, //(20,10,8) =12.67
                             1, //different size, reset buffer (1)
                             20, //different size, reset buffer (20)
                             25, // (20,30)
                             16  // (20,30,0) = 16.67
    }; // expectedResult
    

    QTest::newRow("Mean_Sampling")
        << 4 //Param_Operation
        << 3 //Param_TimeWindow
        << 2 //Param_ResolutionY
        << std::vector<int> {30, //(30)
                             25, //(30,20)
                             20, //(30,20,10)
                             12, //(20,10,8) =12.67
                             1, //different size, reset buffer (1)
                             20, //different size, reset buffer (20)
                             25, // (20,30)
                             16  // (20,30,0) = 16.67
    }; // expectedResult
    
            
    QTest::newRow("StdDev")
        << 5 //Param_Operation
        << 3 //Param_TimeWindow
        << 1 //Param_ResolutionY
        << std::vector<int> {0, //(30)
                             7, //(30,20)
                             10, //(30,20,10) 
                             6, //(20,10,8) 
                             0, //different size, reset buffer (1)
                             0, //different size, reset buffer (20)
                             7, // (20,30)
                             15  // (20,30,0) 
    }; // expectedResult
    
    QTest::newRow("Range")
        << 6 //Param_Operation
        << 3 //Param_TimeWindow
        << 1 //Param_ResolutionY
        << std::vector<int> {0, //(30)
                             10, //(30,20)
                             20, //(30,20,10) 
                             12, //(20,10,8) 
                             0, //different size, reset buffer (1)
                             0, //different size, reset buffer (20)
                             10, // (20,30)
                             30  // (20,30,0) 
    }; // expectedResult
    
    //InvalidOperation: just copy the input
    QTest::newRow("InvalidOperation")
        << 999 //Param_Operation
        << 3 //Param_TimeWindow
        << 1 //Param_ResolutionY
        << std::vector<int> {30, //(30)
                             20, //(30,20)
                             10, //(30,20,10) 
                             8, //(20,10,8) 
                             1, //different size, reset buffer (1)
                             20, //different size, reset buffer (20)
                             30, // (20,30)
                             0  // (20,30,0) 
    }; // expectedResult
    
    QTest::newRow("Median")
        << 7 //Param_Operation
        << 3 //Param_TimeWindow
        << 1 //Param_ResolutionY
        << std::vector<int> {30, //median(30)
                             30, //median(30,20)
                             20, //median(30,20,10) 
                             10, //median(20,10,8)
                             1, //different size, reset buffer (1)
                             20, //different size, reset buffer  median(20)
                             30, // median(20,30)
                             20  // median(20,30,0)
    }; // expectedResult
        
    // all operations with window = 1
    
    for (int operation = 0; operation < 9; operation++)
    {
        std::string testName = "SingleImage_" + std::to_string(operation);
        
        //the operations Range  and Std deviation give a blank image with window=1
        if (operation == 5 || operation == 6)
        {
            QTest::newRow(testName.c_str() )  << operation << 1 << 1 
            << std::vector<int> {0, 0, 0, 0, 0, 0, 0, 0};
        }
        else
        {
            QTest::newRow(testName.c_str() )  << operation << 1 << 1
            << m_inputPixelValue;
        }
    }
    
        
};


void TestImageArithmetic::testProceedSubRoi()
{
      
    QFETCH(std::vector<int>, expectedResult);
    QFETCH(int, param_Operation);
    QFETCH(int, param_TimeWindow);
    QFETCH(int, param_ResolutionY);
    //QFETCH(bool, test_preserves_ramp);

    
    // a null source filter connected with ImageArithmetic filter, connected with DummyFilter
    DummyInput dummyInput(m_inputImage);

    ImageArithmetic oFilter;
    DummyFilter dummyFilter;
        
    //another imageArithmetic filter, that with m_oIntensityRescale = true
    ImageArithmetic oRescalingFilter;
    DummyFilter dummyFilter_rescaling;
    
    
    fliplib::NullSourceFilter sourceFilter;
    
    QVERIFY(dummyInput.connectToFilter(&oFilter));
    QVERIFY(dummyInput.connectToFilter(&oRescalingFilter));
    
    QVERIFY(dummyFilter.connectPipe(oFilter.findPipe("ImageFrame"), 0));
    QVERIFY(dummyFilter_rescaling.connectPipe(oRescalingFilter.findPipe("ImageFrame"), 0));
    
    // access the out pipe data of the ImageArithmetic filter
    auto outPipe = dynamic_cast<fliplib::SynchronePipe<precitec::interface::ImageFrame>*>(oFilter.findPipe("ImageFrame"));
    auto outPipe_rescaling = dynamic_cast<fliplib::SynchronePipe<precitec::interface::ImageFrame>*>(oRescalingFilter.findPipe("ImageFrame"));
    QVERIFY(outPipe);
    QVERIFY(outPipe_rescaling);

    std::unique_ptr<precitec::image::OverlayCanvas> pcanvas {new precitec::image::OverlayCanvas};
    
    // parameterize the filter
    oRescalingFilter.getParameters().update(std::string("RescalePixelIntensity"), fliplib::Parameter::TYPE_bool, true);
    
    //common operations: finish parametrization, set canvas, arm
    for (auto && pFilter: {&oFilter, &oRescalingFilter})
    {
        pFilter->setExternalData(&m_oExternalProductData);

        pFilter->setCanvas(pcanvas.get());
        
        pFilter->getParameters().update(std::string("Verbosity"), fliplib::Parameter::TYPE_int, 1);
        pFilter->getParameters().update(std::string("TimeWindow"), fliplib::Parameter::TYPE_UInt32, param_TimeWindow);
        pFilter->getParameters().update(std::string("Operation"), fliplib::Parameter::TYPE_int, param_Operation);
        pFilter->getParameters().update(std::string("ResolutionY"), fliplib::Parameter::TYPE_UInt32, param_ResolutionY);        
        pFilter->setParameter();

        pFilter->arm(precitec::filter::ArmState::eSeamStart);

    }
    
    QCOMPARE(oFilter.hasCanvas(), true);
    QCOMPARE(oRescalingFilter.hasCanvas(), true);

    QCOMPARE(oFilter.getParameters().findParameter("RescalePixelIntensity").getValue().convert<bool>(), false);
    QCOMPARE(oRescalingFilter.getParameters().findParameter("RescalePixelIntensity").getValue().convert<bool>(), true);

    
  
    // now signal the pipe, this processes the complete filter graph
    
    const Point oROIOffset {5,10};
    
    for (unsigned int counter = 0; counter < m_inputImage.size(); ++counter)
    {   
        auto oROI = dummyInput.fillROI(counter, oROIOffset);
        
        QCOMPARE(dummyFilter.getProceedCalled(), counter);
        QCOMPARE(dummyFilter_rescaling.getProceedCalled(), counter);
        
        dummyInput.signal();
        
        QCOMPARE(dummyFilter.getProceedCalled(), counter+1);
        QCOMPARE(dummyFilter_rescaling.getProceedCalled(), counter+1);

        // the result is a frame with one element
        auto rResultImage = outPipe->read(counter).data();

        //print debug messages before failing the test
        int X = m_testX - oROIOffset.x;
        int Y = oROI.height()-1; //all the lines are equal
        
        if (int(rResultImage.getValue(X , Y))!= expectedResult[counter])
        {
            std::cout << "counter " << counter ;
            if (param_ResolutionY == 1)
            {
                auto & rBuffer = oFilter.m_oFrameBuffer;
                std::cout << " buffer content:\n " ;
                
                for(int i = 0; rBuffer.isValidIndex(i); i++)
                {
                    //std::cout << i << ") " ;
                    if (rBuffer.isBufferPositionInitialized(i))
                    {
                        //we can directly access the image in the buffer because resolution is 1
                        std::cout << int(rBuffer.getCachedElement(i).m_image.getValue(X,Y));
                    }
                    std::cout << "; ";
                }
            }
            std::cout << std::endl;
        }
        
        QCOMPARE(int(rResultImage.getValue(X,Y)), expectedResult[counter]);


        
       // Now compare the output of the rescaling filter to the same operation without rescale
       // Since this rescaling is done to avoid overflow/underflow, it's not always possible to compare these two 
       // images directly, but *with the input provided here* that is a problem only for the operation ePixelDiff
       
        auto rRescaledResultImage = outPipe_rescaling->read(counter).data();
        
        
       if (param_Operation == 999 )
       {
           //for an invalid operation, the input image is just copied, we do not test further
           return;
        }
       
        //check if the image is isContiguos (just to be able to use std algorithms later )
        QVERIFY(rResultImage.isContiguos());
        QVERIFY(rRescaledResultImage.isContiguos());

        auto extremes1 = std::minmax_element(rResultImage.begin(), rResultImage.end());
        auto extremes2 = std::minmax_element(rRescaledResultImage.begin(), rRescaledResultImage.end());
        
        double range1 = (*extremes1.second) - (*extremes1.first);
        if (range1 != 0)
        {
            QCOMPARE(*(extremes2.first), 0);
            QCOMPARE(*(extremes2.second), 255);
        }   
        
       if (param_Operation == 1)
       {
           //pixel difference, in this case we do not have access to the pixel values before rescaling and we can't test further
           return;
        }
       
        if (range1 != 0)
        {            
            for (auto pPixelOriginal = rResultImage.begin(), pPixelRescaled = rRescaledResultImage.begin(), pEndPixelOriginal = rResultImage.end()  ;
                 pPixelOriginal != pEndPixelOriginal;
            ++pPixelOriginal, ++pPixelRescaled)
            {
                double rescaledValue = (*pPixelOriginal - *extremes1.first) / range1 *255.0;                
                QCOMPARE(*pPixelRescaled, byte(rescaledValue));
            }            
        }
        else
        {
            QCOMPARE((*extremes2.second) - (*extremes2.first), 0);
        }


    }
    
}



void TestImageArithmetic::testEmptyImage_data()
{
    QTest::addColumn<int>("param_Operation");
    QTest::addColumn<int>("param_TimeWindow");
    QTest::addColumn<int>("param_ResolutionY");
    for (int i = 0; i < 10; ++i)
    {
        QTest::newRow("") << i << 1 << 1;
        QTest::newRow("") << i << 5 << 1;
        QTest::newRow("") << i << 1 << 5;
    }
    
    
}
void TestImageArithmetic::testEmptyImage()
{
    QFETCH(int, param_Operation);
    QFETCH(int, param_TimeWindow);
    QFETCH(int, param_ResolutionY);
    
    // a null source filter connected with ImageArithmetic filter, connected with DummyFilter
    ImageArithmetic oFilter;
    DummyFilter dummyFilter;

    
    fliplib::NullSourceFilter sourceFilter;
    fliplib::SynchronePipe<precitec::interface::ImageFrame> inImagePipe { &sourceFilter, precitec::interface::SensorFilterInterface::SENSOR_IMAGE_FRAME_PIPE };    
    
    int group = 0;

    QVERIFY(oFilter.connectPipe(&inImagePipe, group));
    QVERIFY(dummyFilter.connectPipe(oFilter.findPipe("ImageFrame"), 0));

    auto outPipe = dynamic_cast<fliplib::SynchronePipe<precitec::interface::ImageFrame>*>(oFilter.findPipe("ImageFrame"));
    QVERIFY(outPipe);

    std::unique_ptr<precitec::image::OverlayCanvas> pcanvas {new precitec::image::OverlayCanvas};
    
    oFilter.setExternalData(&m_oExternalProductData);

    oFilter.setCanvas(pcanvas.get());
    
    oFilter.getParameters().update(std::string("Verbosity"), fliplib::Parameter::TYPE_int, 1);
    oFilter.getParameters().update(std::string("TimeWindow"), fliplib::Parameter::TYPE_UInt32, param_TimeWindow);
    oFilter.getParameters().update(std::string("Operation"), fliplib::Parameter::TYPE_int, param_Operation);
    oFilter.getParameters().update(std::string("ResolutionY"), fliplib::Parameter::TYPE_UInt32, param_ResolutionY);        
    oFilter.setParameter();

    oFilter.arm(precitec::filter::ArmState::eSeamStart);

    QCOMPARE(oFilter.hasCanvas(), true);


    // and process

    int position = 300;
    
  
    // now signal the pipe, this processes the complete filter graph
    
    for (unsigned int counter = 0; counter < m_inputImage.size(); ++counter)
    {   
        //std::cout << "Image nr " << counter << std::endl;
        precitec::interface::ImageContext context;
        context.setImageNumber(counter);
        context.setPosition(position);
        BImage oEmptyImage;
        auto inPipe = precitec::interface::ImageFrame {
            context, 
            oEmptyImage,
            precitec::interface::ResultType::AnalysisOK, 
        };
        
        QCOMPARE(dummyFilter.getProceedCalled(), counter);
        
        inImagePipe.signal(inPipe);
        
        QCOMPARE(dummyFilter.getProceedCalled(), counter+1);

        auto rResultImage = outPipe->read(counter).data();
        QCOMPARE(rResultImage.width(),0);
        QCOMPARE(rResultImage.height(),0);
    }
}

void TestImageArithmetic::testSampling_data()
{
    QTest::addColumn<int>("param_Operation");
    QTest::addColumn<int>("param_TimeWindow");
    QTest::addColumn<int>("param_ResolutionX");
    QTest::addColumn<int>("param_ResolutionY");
    QTest::addColumn<bool>("param_ResampleOutput");
    QTest::addColumn<std::vector<int>>("expectedResult00");
    QTest::addColumn<std::vector<int>>("expectedResult11");
    
    std::vector<int> meanPixel00Window3 {0, (0 + 30)/2, (0 + 30 + 60)/3, (30 + 60 + 90)/3, (60 + 90 + 120)/3, (90+120+150)/3 };    
    
    {
        std::vector<int> meanPixel11Window3(meanPixel00Window3.size());
        std::transform(meanPixel00Window3.begin(), meanPixel00Window3.end(), meanPixel11Window3.begin(), [](int value){return value + 5*1+1;});

        QTest::newRow("Mean_noSampling_0")
            << 4 //Param_Operation
            << 3 //Param_TimeWindow
            << 1 //Param_ResolutionX
            << 1 //Param_ResolutionY
            << false //param_ResampleOutput
            << meanPixel00Window3
            << meanPixel11Window3; 
            
        QTest::newRow("Mean_noSampling_1")
            << 4 //Param_Operation
            << 3 //Param_TimeWindow
            << 1 //Param_ResolutionX
            << 1 //Param_ResolutionY
            << true //param_ResampleOutput
            << meanPixel00Window3
            << meanPixel11Window3; 
    }
    {
        std::vector<int> meanPixel32Window3(meanPixel00Window3.size());
        std::transform(meanPixel00Window3.begin(), meanPixel00Window3.end(), meanPixel32Window3.begin(), [](int value){return value + 5*2+3;});

        QTest::newRow("Mean_3x2_w3_0")
            << 4 //Param_Operation
            << 3 //Param_TimeWindow
            << 3 //Param_ResolutionX
            << 2 //Param_ResolutionY
            << false //param_ResampleOutput
            << meanPixel00Window3
            << meanPixel32Window3; 
        
    
        QTest::newRow("Mean_3x2_w3_1")
                << 4 //Param_Operation
                << 3 //Param_TimeWindow
                << 3 //Param_ResolutionX
                << 2 //Param_ResolutionY
                << true //param_ResampleOutput
                << meanPixel00Window3
                << meanPixel00Window3; //output image is resampled, pixel 1,1 is equal to pixel 00 
    }
    {
        std::vector<int> meanPixel00Window1 {0, 30, 60, 90, 120, 150 };  
        std::vector<int> meanPixel32Window1(meanPixel00Window1.size());
        std::transform(meanPixel00Window1.begin(), meanPixel00Window1.end(), meanPixel32Window1.begin(), [](int value){return value + 5*2+3;});

        QTest::newRow("Mean_3x2_w1_0")
            << 4 //Param_Operation
            << 1 //Param_TimeWindow
            << 3 //Param_ResolutionX
            << 2 //Param_ResolutionY
            << false //param_ResampleOutput
            << meanPixel00Window1
            << meanPixel32Window1; 
        
    
        QTest::newRow("Mean_3x2_w1_1")
                << 4 //Param_Operation
                << 1 //Param_TimeWindow
                << 3 //Param_ResolutionX
                << 2 //Param_ResolutionY
                << true //param_ResampleOutput
                << meanPixel00Window1
                << meanPixel00Window1; //output image is resampled, pixel 1,1 is equal to pixel 00 
    }
    
}


void TestImageArithmetic::testSampling()
{
    QFETCH(std::vector<int>, expectedResult00);
    QFETCH(std::vector<int>, expectedResult11);
    QFETCH(int, param_Operation);
    QFETCH(int, param_TimeWindow);
    QFETCH(int, param_ResolutionX);
    QFETCH(int, param_ResolutionY);
    QFETCH(bool, param_ResampleOutput);
    
    std::vector<BImage> inputImages;
        
    for (int counter = 0; counter < 6; counter++)
    {
        //image of size 5 X 6, where each pixel has a gradually increasing value
        int minPixelIntensity = (counter * 30) % 256;
        inputImages.push_back(precitec::genSquaresImagePattern(1,1,5,6, minPixelIntensity, 30 + minPixelIntensity));    
        
        auto & squareImage = inputImages.back();
        QVERIFY2(squareImage.size() == m_testSquareImageSize, "Unexpected size in test image");
        QVERIFY2(squareImage.getValue(3,2) ==(minPixelIntensity + 5*2 + 3), "Unexpected value in test image");
    }
    
    DummyInput dummyInput(inputImages);
    ImageArithmetic oFilter;
    DummyFilter dummyFilter;
    

    auto outPipe = dynamic_cast<fliplib::SynchronePipe<precitec::interface::ImageFrame>*>(oFilter.findPipe("ImageFrame"));
    QVERIFY(outPipe);
    
    std::unique_ptr<precitec::image::OverlayCanvas> pcanvas {new precitec::image::OverlayCanvas};

    QVERIFY(dummyInput.connectToFilter(&oFilter));
    QVERIFY(dummyFilter.connectPipe(oFilter.findPipe("ImageFrame"), 0));
    
    oFilter.setExternalData(&m_oExternalProductData);

    oFilter.setCanvas(pcanvas.get());
    
    oFilter.getParameters().update(std::string("Verbosity"), fliplib::Parameter::TYPE_int, 1);
    oFilter.getParameters().update(std::string("TimeWindow"), fliplib::Parameter::TYPE_UInt32, param_TimeWindow);
    oFilter.getParameters().update(std::string("Operation"), fliplib::Parameter::TYPE_int, param_Operation);
    oFilter.getParameters().update(std::string("ResolutionX"), fliplib::Parameter::TYPE_UInt32, param_ResolutionX);
    oFilter.getParameters().update(std::string("ResolutionY"), fliplib::Parameter::TYPE_UInt32, param_ResolutionY);
    oFilter.getParameters().update(std::string("ResampleOutput"), fliplib::Parameter::TYPE_bool, param_ResampleOutput);
    oFilter.setParameter();

    oFilter.arm(precitec::filter::ArmState::eSeamStart);
    for (unsigned int counter = 0; counter < inputImages.size(); ++counter)
    {

        dummyInput.fillImage(counter);
        
        QCOMPARE(dummyFilter.getProceedCalled(), counter);
        
        dummyInput.signal();
        
        QCOMPARE(dummyFilter.getProceedCalled(), counter+1);
        auto rResultImage = outPipe->read(counter).data();
        QCOMPARE(int(rResultImage.getValue(0,0)), expectedResult00[counter]);
        QCOMPARE(int(rResultImage.getValue(1,1)), expectedResult11[counter]);

    }
}





QTEST_MAIN(TestImageArithmetic)
#include "testImageArithmetic.moc"
