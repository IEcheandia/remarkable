#include <QTest>

#include "../circleFit.h"
#include <fliplib/NullSourceFilter.h>

#include <fliplib/BaseFilter.h>
#include <overlay/overlayCanvas.h>

class TestCircleFit : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testProceed();
    void testProceed_data();
};

struct DummyInput
{       
    fliplib::NullSourceFilter m_sourceFilter;
    fliplib::SynchronePipe<precitec::interface::GeoVecAnnotatedDPointarray> m_pipeInPointList;
    
    precitec::interface::GeoVecAnnotatedDPointarray m_pointList;
    
    DummyInput()
    : m_pipeInPointList{ &m_sourceFilter, "pointlist_id"}
    {
    }
    
    bool connectToFilter(fliplib::BaseFilter * pFilter)
    {
        int group = 0;
        //connect  pipes
        bool ok = pFilter->connectPipe(&( m_pipeInPointList ), group);
        return ok;
    }
    
    
    void fillData( int imageNumber, precitec::geo2d::Point trafoOffset, precitec::geo2d::DPoint expectedCenterInROI, double expectedRadius, int numPoints, double startDegrees, double endDegrees, bool addNoise)
    {
        using namespace precitec::interface;
        using namespace precitec::geo2d;
        
        SmpTrafo oTrafo{new LinearTrafo(trafoOffset)};
        ImageContext context (ImageContext{}, oTrafo);
        context.setImageNumber(imageNumber);
 
        AnnotatedDPointarray firstPointListArray;
        auto & pointList = firstPointListArray.getData();

        double angleStart_rad = startDegrees * M_PI / 180.0;
        double angleEnd_rad = endDegrees * M_PI / 180.0;
        double deltaAngle = (angleEnd_rad - angleStart_rad) / double(numPoints);

        double angle_rad = angleStart_rad;
        for (int i= 0; i < numPoints; i++, angle_rad += deltaAngle)
        {
            //round to simulate points extracted from an image
            int x = std::round( expectedCenterInROI.x + expectedRadius * std::cos(angle_rad));
            int y = std::round( expectedCenterInROI.y + expectedRadius * std::sin(angle_rad));
            pointList.emplace_back(x,y);
        }
        if (addNoise)
        {
            pointList.emplace_back(100,100);
            pointList.emplace_back(150,100);
        }
        firstPointListArray.getRank().assign(pointList.size(), 255);
        
        m_pointList = GeoVecAnnotatedDPointarray{context, std::vector<AnnotatedDPointarray>{firstPointListArray}, AnalysisOK, 1.0 };
        
    }
    void signal()
    {
        m_pipeInPointList.signal(m_pointList);
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

void TestCircleFit::testCtor()
{
    precitec::filter::CircleFit filter;
    QCOMPARE(filter.name(), std::string("CircleFit"));
    QVERIFY(filter.findPipe("CircleX") != nullptr);
    QVERIFY(filter.findPipe("CircleY") != nullptr);
    QVERIFY(filter.findPipe("CircleR") != nullptr);
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);
    
    // check parameters of type double
    for ( auto entry : std::vector<std::pair<std::string, double>> {{"MinRadius", 10.0}, {"MaxRadius", 100.0}})
    {
        auto parameter = entry.first;
        auto value = entry.second;
        QVERIFY(filter.getParameters().exists(parameter));
        QCOMPARE(filter.getParameters().findParameter(parameter).getType(), fliplib::Parameter::TYPE_double);
        QCOMPARE(filter.getParameters().findParameter(parameter).getValue().convert<double>(), value);   
    }
    // check parameters of type int
    for (auto entry : std::vector<std::pair<std::string, int>> {{"PartOfSpreadStart",0}, {"PartOfSpreadEnd", 100}, {"Mode", 0}})
    {
        auto parameter = entry.first;
        auto value = entry.second;
        QVERIFY(filter.getParameters().exists(parameter));
        QCOMPARE(filter.getParameters().findParameter(parameter).getType(), fliplib::Parameter::TYPE_int);
        QCOMPARE(filter.getParameters().findParameter(parameter).getValue().convert<int>(), value);   
    }
    
}

void TestCircleFit::testProceed_data()
{
    QTest::addColumn<int>("numPoints");
    QTest::addColumn<double>("angleStart");
    QTest::addColumn<double>("angleEnd");
    QTest::addColumn<bool>("withNoise");

    //TODO check why least squares does not fit a full circle
    QTest::newRow("LeastSquaresCircleFit_Arc") << 10 << 0.0 << 90.0 << false;
    QTest::newRow("LeastSquaresCircleFit_Sparse") << 5 << 0.0 << 350.0 << false;
    QTest::newRow("LeastSquaresCircleFit_50") << 50 << 10.0 << 360.0 << false;


}
void TestCircleFit::testProceed()
{
    precitec::filter::CircleFit filter;
    std::unique_ptr<precitec::image::OverlayCanvas> pcanvas {new precitec::image::OverlayCanvas};
    filter.setCanvas(pcanvas.get());
    
    DummyInput dummyInput;
    QVERIFY(dummyInput.connectToFilter(&filter));
            
    auto outPipeX = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("CircleX"));
    QVERIFY(outPipeX);
    auto outPipeY = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("CircleY"));
    QVERIFY(outPipeY);
    auto outPipeRadius = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("CircleR"));
    QVERIFY(outPipeRadius);
    
    DummyFilter dummyFilter;
    QVERIFY(dummyFilter.connectPipe(outPipeX, 0));
    QVERIFY(dummyFilter.connectPipe(outPipeY, 0));
    QVERIFY(dummyFilter.connectPipe(outPipeRadius, 0));

    QFETCH(int, numPoints);
    QFETCH(double, angleStart);
    QFETCH(double, angleEnd);
    QFETCH(bool, withNoise);
    
    filter.getParameters().update(std::string("Mode"), fliplib::Parameter::TYPE_int, 0);
    filter.getParameters().update(std::string("MinRadius"), fliplib::Parameter::TYPE_double, 5);
    filter.getParameters().update(std::string("MaxRadius"), fliplib::Parameter::TYPE_double, 15);
    filter.setParameter();
    
    precitec::geo2d::DPoint expectedCenterInROI{50,48.0};
    double expectedRadius = 10;

    int imageNumber = 0;
    precitec::geo2d::Point trafoOffset {510,100};
    dummyInput.fillData(imageNumber, trafoOffset, expectedCenterInROI, expectedRadius, numPoints, angleStart, angleEnd, withNoise);
    QCOMPARE(dummyFilter.isProceedCalled(), false);
    dummyInput.signal();
    QCOMPARE(dummyFilter.isProceedCalled(), true);
    dummyFilter.resetProceedCalled(); 
    
    const auto resultX = outPipeX->read(imageNumber);
    const auto resultY = outPipeY->read(imageNumber);
    const auto resultRadius = outPipeRadius->read(imageNumber);
    
    QCOMPARE(resultX.ref().size(), 1ul);
    QCOMPARE(resultY.ref().size(), 1ul);
    QCOMPARE(resultRadius.ref().size(), 1ul);
    
    precitec::geo2d::Point ROI_O{0,0};
    QCOMPARE(resultX.context().trafo()->apply(ROI_O), trafoOffset);
    QCOMPARE(resultY.context().trafo()->apply(ROI_O), trafoOffset);

    QCOMPARE(resultX.ref().getData().front(), expectedCenterInROI.x);
    QCOMPARE(resultY.ref().getData().front(), expectedCenterInROI.y);
    QCOMPARE(resultRadius.ref().getData().front(), expectedRadius);
}



QTEST_GUILESS_MAIN(TestCircleFit)
#include "testCircleFit.moc"
