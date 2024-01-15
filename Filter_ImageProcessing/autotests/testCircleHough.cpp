#include <QTest>

#include "../circleHough.h"
#include <fliplib/NullSourceFilter.h>

#include <fliplib/BaseFilter.h>
#include <overlay/overlayCanvas.h>

class TestCircleHough : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testProceed();
};

struct DummyInput
{       
    fliplib::NullSourceFilter m_sourceFilter;
    fliplib::SynchronePipe<precitec::interface::ImageFrame> m_pipeInROI;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> m_pipeInRadiusStart;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> m_pipeInRadiusEnd;
    
    precitec::interface::ImageFrame m_frame;
    precitec::interface::GeoDoublearray m_radiusStart;
    precitec::interface::GeoDoublearray m_radiusEnd;    
    
    DummyInput()
    : m_pipeInROI{&m_sourceFilter, "image_id"},
        m_pipeInRadiusStart{ &m_sourceFilter, "radiusStart_id"},
        m_pipeInRadiusEnd{ &m_sourceFilter, "radiusEnd_id"}
    {
        m_pipeInROI.setTag("image");
        m_pipeInRadiusStart.setTag("radiusStart");
        m_pipeInRadiusEnd.setTag("radiusEnd");    
    }
    
    bool connectToFilter(fliplib::BaseFilter * pFilter)
    {
        int group =1;
        //connect  pipes
        bool ok = pFilter->connectPipe(&(m_pipeInROI), group);
        ok &= pFilter->connectPipe(&(m_pipeInRadiusStart), group);
        ok &= pFilter->connectPipe(&(m_pipeInRadiusEnd), group);
        return ok;
    }
    
    precitec::interface::GeoDoublearray createGeoDoubleArray(precitec::interface::ImageContext baseContext, double value, int rank )
    {
        using namespace precitec::interface;
        return GeoDoublearray( ImageContext{baseContext}, 
                                   precitec::geo2d::Doublearray{1, value, rank}, 
                                   ResultType::AnalysisOK, Limit);
    }
    
    void fillData( int imageNumber, precitec::geo2d::DPoint expectedCenter, double expectedRadius, double radiusMin, double radiusMax)
    {
        using namespace precitec::interface;
        using namespace precitec::geo2d;
        
        ImageContext baseContext;
        baseContext.setImageNumber(imageNumber);
 
        //build a circle on a black image
        precitec::image::BImage imageWithCircle(precitec::geo2d::Size{200,200});
        for (int i= 0; i < 5; i++)
        {
            double angle_rad = i * 2 * M_PI / 5.0;
            int x = std::round(expectedCenter.x + expectedRadius * std::cos(angle_rad));
            int y = std::round(expectedCenter.y + expectedRadius * std::sin(angle_rad));
            if (x >= 0 &&  x < imageWithCircle.width() && y >= 0 && y < imageWithCircle.height())
            {
                imageWithCircle[y][x] = 255;
            }
        }
        //add some noise
        imageWithCircle[100][100] = 255;
        imageWithCircle[150][100] = 255;
        
        m_frame = precitec::interface::ImageFrame{
            ImageContext{},
            imageWithCircle,
            ResultType::AnalysisOK
        };
        
        m_radiusStart = createGeoDoubleArray(baseContext, radiusMin, 255);
        m_radiusEnd = createGeoDoubleArray(baseContext, radiusMax, 255);
        
    }
    void signal()
    {
        m_pipeInROI.signal(m_frame);
        m_pipeInRadiusStart.signal(m_radiusStart);
        m_pipeInRadiusEnd.signal(m_radiusEnd);
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

void TestCircleHough::testCtor()
{
    precitec::filter::CircleHough filter;
    QCOMPARE(filter.name(), std::string("CircleHough"));
    QVERIFY(filter.findPipe("CenterX") != nullptr);
    QVERIFY(filter.findPipe("CenterY") != nullptr);
    QVERIFY(filter.findPipe("Radius") != nullptr);
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);
    
    // check parameters of type double´
    for ( auto entry : std::vector<std::pair<std::string, double>> {{"RadiusStep", 5.0}, {"ScoreThreshold", -1.0}})
    {
        auto parameter = entry.first;
        auto value = entry.second;
        QVERIFY(filter.getParameters().exists(parameter));
        QCOMPARE(filter.getParameters().findParameter(parameter).getType(), fliplib::Parameter::TYPE_double);
        QCOMPARE(filter.getParameters().findParameter(parameter).getValue().convert<double>(), value);   
    }
    // check parameters of type int
    for (auto entry : std::vector<std::pair<std::string, int>> {{"NumberOfMax",1}, {"IntensityThreshold", 10}})
    {
        auto parameter = entry.first;
        auto value = entry.second;
        QVERIFY(filter.getParameters().exists(parameter));
        QCOMPARE(filter.getParameters().findParameter(parameter).getType(), fliplib::Parameter::TYPE_int);
        QCOMPARE(filter.getParameters().findParameter(parameter).getValue().convert<int>(), value);   
    }
    
    // check parameters of type bool 
    for (auto entry : std::vector<std::pair<std::string, bool>> {{"SearchOutsideROI", false}})
    {
        auto parameter = entry.first;
        auto value = entry.second;
        QVERIFY(filter.getParameters().exists(parameter));
        QCOMPARE(filter.getParameters().findParameter(parameter).getType(), fliplib::Parameter::TYPE_bool);
        QCOMPARE(filter.getParameters().findParameter(parameter).getValue().convert<bool>(), value);   

    }
}

void TestCircleHough::testProceed()
{
    precitec::filter::CircleHough filter;
    std::unique_ptr<precitec::image::OverlayCanvas> pcanvas {new precitec::image::OverlayCanvas};
    filter.setCanvas(pcanvas.get());
    
    DummyInput dummyInput;
    QVERIFY(dummyInput.connectToFilter(&filter));
            
    auto outPipeX = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("CenterX"));
    QVERIFY(outPipeX);
    auto outPipeY = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("CenterY"));
    QVERIFY(outPipeY);
    auto outPipeRadius = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("Radius"));
    QVERIFY(outPipeRadius);
    
    DummyFilter dummyFilter;
    QVERIFY(dummyFilter.connectPipe(outPipeX, 0));
    QVERIFY(dummyFilter.connectPipe(outPipeY, 0));
    QVERIFY(dummyFilter.connectPipe(outPipeRadius, 0));
    
    filter.getParameters().update(std::string("RadiusStep"), fliplib::Parameter::TYPE_double, 1.0);
    filter.getParameters().update(std::string("ScoreThreshold"), fliplib::Parameter::TYPE_double, -1.0);
    filter.setParameter();
    
    precitec::geo2d::DPoint expectedCenter{50,48.0};
    double expectedRadius = 10.0;
    dummyInput.fillData(0,expectedCenter, expectedRadius, 5, 15);
    QCOMPARE(dummyFilter.isProceedCalled(), false);
    dummyInput.signal();
    QCOMPARE(dummyFilter.isProceedCalled(), true);
    dummyFilter.resetProceedCalled(); 
    

    auto imageNumber = dummyInput.m_frame.context().imageNumber();
    const auto resultX = outPipeX->read(imageNumber);
    const auto resultY = outPipeY->read(imageNumber);
    const auto resultRadius = outPipeRadius->read(imageNumber);
    
    QCOMPARE(resultX.ref().size(), 1ul);
    QCOMPARE(resultY.ref().size(), 1ul);
    QCOMPARE(resultRadius.ref().size(), 1ul);
    
    
    QCOMPARE(resultX.ref().getData().front(), expectedCenter.x);
    QCOMPARE(resultY.ref().getData().front(), expectedCenter.y);
    QCOMPARE(resultRadius.ref().getData().front(), expectedRadius);
    for (auto * result : {&resultX, &resultY, &resultRadius})
    {
        //result will always have good rank, because ScoreThreshold is set to -1 (best fit)
        QCOMPARE(result->ref().getRank().front(), 255);
    }
}



QTEST_GUILESS_MAIN(TestCircleHough)
#include "testCircleHough.moc"
