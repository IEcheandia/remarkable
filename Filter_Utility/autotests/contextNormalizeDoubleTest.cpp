#include <QTest>

#include "../contextNormalizeDouble.h"

#include <fliplib/NullSourceFilter.h>
#include <fliplib/SinkFilter.h>
#include <filter/sensorFilterInterface.h>

using precitec::filter::ContextNormalizeDouble;
using namespace precitec::interface;

class ContextNormalizeDoubleTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testProceed_data();
    void testProceed();
};

void ContextNormalizeDoubleTest::testCtor()
{
    ContextNormalizeDouble filter;
    QCOMPARE(filter.name(), std::string("ContextNormalizeDouble"));
    QVERIFY(filter.findPipe("Result1") != nullptr);
    QVERIFY(filter.findPipe("Result2") != nullptr);
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);
    QVERIFY(filter.getParameters().exists(std::string("Type1")));
    QVERIFY(filter.getParameters().exists(std::string("Type2")));
    QVERIFY(filter.getParameters().exists(std::string("HandleSampling")));

    QCOMPARE(filter.getParameters().findParameter(std::string("Type1")).getType(), fliplib::Parameter::TYPE_int);
    QCOMPARE(filter.getParameters().findParameter(std::string("Type1")).getValue().convert<int>(), 0);
    
    QCOMPARE(filter.getParameters().findParameter(std::string("Type2")).getType(), fliplib::Parameter::TYPE_int);
    QCOMPARE(filter.getParameters().findParameter(std::string("Type2")).getValue().convert<int>(), 1);
    
    QCOMPARE(filter.getParameters().findParameter(std::string("HandleSampling")).getType(), fliplib::Parameter::TYPE_bool);
    QCOMPARE(filter.getParameters().findParameter(std::string("HandleSampling")).getValue().convert<bool>(), false);
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


Q_DECLARE_METATYPE(ImageFrame);
Q_DECLARE_METATYPE(GeoDoublearray);

void ContextNormalizeDoubleTest::testProceed_data()
{
    using namespace precitec::geo2d;
    
    QTest::addColumn<int>("parameter_type1");
    QTest::addColumn<int>("parameter_type2");
    QTest::addColumn<bool>("parameter_handleSampling");
    QTest::addColumn<ImageFrame>("inputReference");
    QTest::addColumn<GeoDoublearray>("input1");
    QTest::addColumn<GeoDoublearray>("input2");
    QTest::addColumn<double>("expectedValue1");
    QTest::addColumn<double>("expectedValue2");
    
    auto oSensorImage = precitec::image::genModuloPattern({300,300},2);
    
    {   // No Trafo or sampling in Image, Trafo only in point

        ImageContext contextReference{};
        ImageFrame inputReference { contextReference, oSensorImage, AnalysisOK };
        
        
        SmpTrafo oPointTrafo = new LinearTrafo{101,201};
        DPoint testPoint {0.3, 0.4};
    
        GeoDoublearray inputX { ImageContext{contextReference, oPointTrafo}, TArray<double>{1, testPoint.x, 255}, AnalysisOK, 1.0 };
        GeoDoublearray inputY { ImageContext{contextReference, oPointTrafo}, TArray<double>{1, testPoint.y, 255}, AnalysisOK, 1.0 };
        
        DPoint pointInImageTrafo {101 + 0.3, 201 + 0.4};
        QTest::addRow("PointTrafo_xy") << 0 << 1 << false <<  inputReference << inputX << inputY << pointInImageTrafo.x << pointInImageTrafo.y;
        QTest::addRow("PointTrafo_yx") << 1 << 0 << true << inputReference << inputY << inputX << pointInImageTrafo.y << pointInImageTrafo.x;
        QTest::addRow("PointTrafo_onlyvalue_x") << 2 << 0 << true << inputReference << inputY << inputX << testPoint.y << pointInImageTrafo.x;
        QTest::addRow("PointTrafo_AsSize") << 3 << 4 << false <<  inputReference << inputX << inputY << testPoint.x << testPoint.y;
        QTest::addRow("PointTrafo_AsSize") << 3 << 4 << true <<  inputReference << inputX << inputY << testPoint.x << testPoint.y;
        
    }
    {   
        // Trafo in Image,
        ImageContext context00{};
        
        SmpTrafo oImageTrafo = new LinearTrafo{101,201};
        ImageContext contextReference{context00, oImageTrafo};
        ImageFrame inputReference { contextReference, oSensorImage, AnalysisOK };
        
        DPoint testPoint {0.3, 0.4};

        GeoDoublearray inputX { ImageContext{context00}, TArray<double>{1, testPoint.x, 255}, AnalysisOK, 1.0 };
        GeoDoublearray inputY { ImageContext{context00}, TArray<double>{1, testPoint.y, 255}, AnalysisOK, 1.0 };
        
        DPoint pointInImageTrafo {0.3 -101, 0.4 - 201};
        QTest::addRow("ImageTrafo_yx") << 1 << 0 << true <<inputReference << inputY << inputX << pointInImageTrafo.y << pointInImageTrafo.x;
        QTest::addRow("ImageTrafo_y_onlyvalue") << 1 << 2 << false <<inputReference << inputY << inputX << pointInImageTrafo.y << testPoint.x;
        QTest::addRow("ImageTrafo_AsSize") << 3 << 4 << true  <<  inputReference << inputX << inputY << testPoint.x << testPoint.y;
        
        
        GeoDoublearray inputX_sameInputTrafo { ImageContext{contextReference}, TArray<double>{1, testPoint.x, 255}, AnalysisOK, 1.0 };
        GeoDoublearray inputY_sameInputTrafo { ImageContext{contextReference}, TArray<double>{1, testPoint.y, 255}, AnalysisOK, 1.0 };
        QTest::addRow("SameInputTrafo") << 0 << 1 << true << inputReference << inputX_sameInputTrafo << inputY_sameInputTrafo << testPoint.x << testPoint.y;
        QTest::addRow("SameInputTrafo_AsSize") << 3 << 4 << true  <<  inputReference << inputX << inputY << testPoint.x << testPoint.y;

    }
    {   
        ImageContext contextReference{};
        ImageFrame inputReference { contextReference, oSensorImage, AnalysisOK };
        
        GeoDoublearray inputX { ImageContext{contextReference, SmpTrafo{new LinearTrafo{100,200}}}, TArray<double>{1, 0.1, 255}, AnalysisOK, 1.0 };
        GeoDoublearray inputY { ImageContext{contextReference, SmpTrafo{new LinearTrafo{300,400}}}, TArray<double>{1, 0.4, 255}, AnalysisOK, 1.0 };
        
        QTest::addRow("DifferentTrafo") << 0 << 1 << false << inputReference << inputX << inputY << 100.1 << 400.4;
    }
    {   
        ImageContext context00{};

        ImageContext contextReference{context00};

        contextReference.SamplingX_ = 1.0/2.0; // featureimage.width / inputimage.width
        contextReference.SamplingY_ = 1.0/3.0; // featureimage.height / inputimage.height
        
        ImageFrame inputReference { contextReference, oSensorImage, AnalysisOK };
        
        {
            GeoDoublearray inputX { ImageContext{context00}, TArray<double>{1, 2.0, 255}, AnalysisOK, 1.0 };
            GeoDoublearray inputY { ImageContext{context00}, TArray<double>{1, 3.0, 255}, AnalysisOK, 1.0 };
            
            QTest::addRow("downsampledImage") << 0 << 1 << true << inputReference << inputX << inputY << 1.0 << 1.0;
            QTest::addRow("downsampledImageIgnoringSampling") << 0 << 1 << false << inputReference << inputX << inputY << 2.0 << 3.0;

            QTest::addRow("SizeDownsampledImage") << 0 << 1 << true << inputReference << inputX << inputY << 1.0 << 1.0;
        }
        
        {
            GeoDoublearray inputX { ImageContext{context00, SmpTrafo{new LinearTrafo{100,200}}}, TArray<double>{1, 0, 255}, AnalysisOK, 1.0 };
            GeoDoublearray inputY { ImageContext{context00, SmpTrafo{new LinearTrafo{300,400}}}, TArray<double>{1, 0, 255}, AnalysisOK, 1.0 };
            QTest::addRow("downsampledImage_convertTrafo") << 0 << 1 << true << inputReference << inputX << inputY << 100.0/2.0 << 400.0/3.0;
            QTest::addRow("downsampledImage_convertTrafoIgnoringSampling") << 0 << 1 << false << inputReference << inputX << inputY << 100.0 << 400.0;


        }
        {
            GeoDoublearray inputX { ImageContext{context00, SmpTrafo{new LinearTrafo{100,200}}}, TArray<double>{1, 2.0, 255}, AnalysisOK, 1.0 };
            GeoDoublearray inputY { ImageContext{context00, SmpTrafo{new LinearTrafo{300,400}}}, TArray<double>{1, 3.0, 255}, AnalysisOK, 1.0 };
            QTest::addRow("downsampledImage_pointTrafo") << 0 << 1 << true << inputReference << inputX << inputY << 100.0/2.0 + 2.0/2.0 << 400.0/3.0 + 3.0/3.0;
            QTest::addRow("downsampledImage_onlyvalue") << 2 << 2 << true << inputReference << inputX << inputY << 2.0 << 3.0;
            QTest::addRow("downsampledImage_pointTrafoIgnoringSampling") << 0 << 1 << false << inputReference << inputX << inputY << 100.0 + 2.0 << 400.0 + 3.0;
            QTest::addRow("downsampledImage_onlyvalueIgnoringSampling") << 2 << 2 << false << inputReference << inputX << inputY << 2.0 << 3.0;

            QTest::addRow("downsampledImage_size") << 3 << 4 << true  <<  inputReference << inputX << inputY << 2.0 / 2.0 << 3.0/3.0;
            QTest::addRow("downsampledImage_sizeIgnoringSampling") << 3 << 4 << false  <<  inputReference << inputX << inputY << 2.0  << 3.0;
        }
        
    }
    {   
        ImageContext context00{};

        ImageContext contextReference{context00};
        ImageFrame inputReference { contextReference, oSensorImage, AnalysisOK };
        
        {
            ImageContext contextPoint{context00};
            contextPoint.SamplingX_ = 1.0/2.0; // featureimage.width / inputimage.width
            contextPoint.SamplingY_ = 1.0/3.0; // featureimage.height / inputimage.height
            
            GeoDoublearray inputX { ImageContext{contextPoint}, TArray<double>{1, 1.0, 255}, AnalysisOK, 1.0 };
            GeoDoublearray inputY { ImageContext{contextPoint}, TArray<double>{1, 1.0, 255}, AnalysisOK, 1.0 };
            
            QTest::addRow("downsampledPointCoordinates") << 0 << 1 << true << inputReference << inputX << inputY << 2.0 << 3.0;
        }
        {
            //simulate point coming from featureimage, computed on a roi at 300,400
            ImageContext contextPoint{context00, SmpTrafo{new LinearTrafo{300,400}}};
            contextPoint.SamplingX_ = 1.0/2.0; // featureimage.width / inputimage.width
            contextPoint.SamplingY_ = 1.0/3.0; // featureimage.height / inputimage.height
            
            GeoDoublearray inputFirstX { ImageContext{contextPoint}, TArray<double>{1, 0.0, 255}, AnalysisOK, 1.0 };
            GeoDoublearray inputFirstY { ImageContext{contextPoint}, TArray<double>{1, 0.0, 255}, AnalysisOK, 1.0 };
            double featureimageWidth = 10.0;
            double featureimageHeight = 10.0;
            GeoDoublearray inputLastX { ImageContext{contextPoint}, TArray<double>{1, featureimageWidth, 255}, AnalysisOK, 1.0 };
            GeoDoublearray inputLastY { ImageContext{contextPoint}, TArray<double>{1, featureimageHeight, 255}, AnalysisOK, 1.0 };
            QTest::addRow("firstPointFeatureImage") << 0 << 1 << true << inputReference << inputFirstX << inputFirstY << 300.0 << 400.0;
            QTest::addRow("lastPointFeatureImage")  << 0 << 1 << true << inputReference << inputLastX << inputLastY << 320.0 << 430.0;
            QTest::addRow("sizeFeatureImage")  << 3 << 4 << true << inputReference << inputLastX << inputLastY << featureimageWidth*2.0 << featureimageHeight*3.0;
        }
        
        
    }
}

void ContextNormalizeDoubleTest::testProceed()
{
    // prepare filter graph
    // a null source filter connected with SystemConstant filter, connected with DummyFilter
    ContextNormalizeDouble filter;
    fliplib::NullSourceFilter sourceFilter;
    fliplib::SynchronePipe<precitec::interface::ImageFrame> pipeReference{ &sourceFilter, precitec::interface::SensorFilterInterface::SENSOR_IMAGE_FRAME_PIPE };
    pipeReference.setTag("referenceImage");
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipe1{ &sourceFilter, "pipe1ID" };
    pipe1.setTag("value1");
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipe2{ &sourceFilter, "pipe2ID" };
    pipe2.setTag("value2");

    QVERIFY(filter.connectPipe(&pipeReference, 1));
    QVERIFY(filter.connectPipe(&pipe1, 1));
    QVERIFY(filter.connectPipe(&pipe2, 1));

    DummyFilter dummyFilter;
    QVERIFY(dummyFilter.connectPipe(filter.findPipe("Result1"), 0));
    QVERIFY(dummyFilter.connectPipe(filter.findPipe("Result2"), 0));

    // parameterize the filter
    QFETCH(int, parameter_type1);
    QFETCH(int, parameter_type2);
    QFETCH(bool, parameter_handleSampling);
    filter.getParameters().update(std::string("Type1"), fliplib::Parameter::TYPE_int, parameter_type1);
    filter.getParameters().update(std::string("Type2"), fliplib::Parameter::TYPE_int, parameter_type2);
    filter.getParameters().update(std::string("HandleSampling"), fliplib::Parameter::TYPE_bool, parameter_handleSampling);

    filter.setParameter();

    // and process
    // first generate the image frame with some dummy data
    QFETCH(ImageFrame, inputReference);
    QFETCH(GeoDoublearray, input1);
    QFETCH(GeoDoublearray, input2);
    
    // now signal the pipe, this processes the complete filter graph
    QCOMPARE(dummyFilter.isProceedCalled(), false);
    pipeReference.signal(inputReference);
    pipe1.signal(input1);
    pipe2.signal(input2);
    QCOMPARE(dummyFilter.isProceedCalled(), true);

    // access the out pipe data of the SystemConstant filter
    auto outPipe1 = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray>*>(filter.findPipe("Result1"));
    QVERIFY(outPipe1);
    // the result is a GeoDoublearray with one element
    const auto result1 = outPipe1->read(inputReference.context().imageNumber());
    QCOMPARE(result1.ref().size(), 1);
    QTEST(result1.ref().getData().front(), "expectedValue1");
    
    auto outPipe2 = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray>*>(filter.findPipe("Result2"));
    QVERIFY(outPipe2);
    const auto result2 = outPipe2->read(inputReference.context().imageNumber());
    QCOMPARE(result2.ref().size(), 1);
    QTEST(result2.ref().getData().front(), "expectedValue2");
    
    //verify that the output context is the same as the reference
    auto inputReferenceTrafo = inputReference.context().trafo();
    for (auto && rContext : {result1.context(), result2.context()})
    {
        auto trafo = rContext.trafo();
        QCOMPARE(trafo->dx(), inputReferenceTrafo->dx()); 
        QCOMPARE(trafo->dy(), inputReferenceTrafo->dy()); 
        QCOMPARE(rContext.SamplingX_, inputReference.context().SamplingX_);
        QCOMPARE(rContext.SamplingY_, inputReference.context().SamplingY_);
    }

}


QTEST_GUILESS_MAIN(ContextNormalizeDoubleTest)
#include "contextNormalizeDoubleTest.moc"
