#include <QtTest/QtTest>

#include "../circleFitXT.h"
#include <fliplib/NullSourceFilter.h>

#include <fliplib/BaseFilter.h>
#include <overlay/overlayCanvas.h>

class TestCircleFitXT : public QObject
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

    typedef fliplib::SynchronePipe<precitec::interface::GeoVecAnnotatedDPointarray> pointlist_pipe_t;
    typedef fliplib::SynchronePipe<precitec::interface::GeoDoublearray> scalar_pipe_t;


    enum ScalarPipes
    {
        eRadiusMin,
        eRadiusMax,
        eValidXStart,
        eValidYStart,
        eValidXEnd,
        eValidYEnd,
        NUMSCALARPIPES
    };
	pointlist_pipe_t m_pipePointList;
    std::array<std::unique_ptr<scalar_pipe_t> ,NUMSCALARPIPES> m_pScalarPipes;

    precitec::interface::GeoVecAnnotatedDPointarray m_pointList;
    std::array<precitec::interface::GeoDoublearray,NUMSCALARPIPES> m_oScalarData;

    DummyInput()
    : m_pipePointList{ &m_sourceFilter, "pointlist_id"}
    {
        for (int i = 0; i < NUMSCALARPIPES; i++)
        {
            m_pScalarPipes[i].reset( new scalar_pipe_t{&m_sourceFilter, std::to_string(i)});
        }

        m_pipePointList.setTag("PointList");
        m_pScalarPipes[eRadiusMin]->setTag("RadiusMin");
        m_pScalarPipes[eRadiusMax]->setTag("RadiusMax");
        m_pScalarPipes[eValidXStart]->setTag("XStart");
        m_pScalarPipes[eValidYStart]->setTag("YStart");
        m_pScalarPipes[eValidXEnd]->setTag("XEnd");
        m_pScalarPipes[eValidYEnd]->setTag("YEnd");
    }
    
    bool connectToFilter(fliplib::BaseFilter * pFilter)
    {
        int group = 1;
        //connect  pipes
        bool ok = pFilter->connectPipe(&( m_pipePointList ), group);
        for (int i = 0; i < NUMSCALARPIPES; i++)
        {
            ok &= pFilter->connectPipe(m_pScalarPipes[i].get(), group);
        }
        return ok;
    }
    
    
    void fillData( int imageNumber, precitec::geo2d::Point trafoOffset,
                   precitec::geo2d::DPoint expectedCenterInROI, double expectedRadius,
                   int numPoints, double startDegrees, double endDegrees, bool addNoise,
                   const precitec::geo2d::Point &validAreaStartInCanvas,
                   const precitec::geo2d::Point & validAreaEndInCanvas )
    {
        using namespace precitec::interface;
        using namespace precitec::geo2d;
        
        SmpTrafo oTrafo{new LinearTrafo(trafoOffset)};
        ImageContext context (ImageContext{}, oTrafo);
        context.setImageNumber(imageNumber);

        ImageContext contextCanvas (context, SmpTrafo {new Trafo()});
 
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
        m_oScalarData[ eRadiusMin ] = GeoDoublearray{context, precitec::geo2d::Doublearray(1,5.0, precitec::filter::eRankMax), AnalysisOK, 1.0 };
        m_oScalarData[ eRadiusMax ] = GeoDoublearray{context, precitec::geo2d::Doublearray(1,15.0, precitec::filter::eRankMax), AnalysisOK, 1.0 };
        m_oScalarData[ eValidXStart ] = GeoDoublearray{contextCanvas, precitec::geo2d::Doublearray(1, validAreaStartInCanvas.x, precitec::filter::eRankMax), AnalysisOK, 1.0 };
        m_oScalarData[ eValidYStart ] = GeoDoublearray{contextCanvas, precitec::geo2d::Doublearray(1, validAreaStartInCanvas.y, precitec::filter::eRankMax), AnalysisOK, 1.0 };
        m_oScalarData[ eValidXEnd ] = GeoDoublearray{contextCanvas, precitec::geo2d::Doublearray(1, validAreaEndInCanvas.x, precitec::filter::eRankMax), AnalysisOK, 1.0 };
        m_oScalarData[ eValidYEnd ] = GeoDoublearray{context, precitec::geo2d::Doublearray(1, validAreaEndInCanvas.y - trafoOffset.y, precitec::filter::eRankMax), AnalysisOK, 1.0 };

    }

    void signal()
    {
        m_pipePointList.signal(m_pointList);
        for (int i = 0; i < NUMSCALARPIPES; i++)
        {
            m_pScalarPipes[i]->signal(m_oScalarData[i]);
        }
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

void TestCircleFitXT::testCtor()
{
    precitec::filter::CircleFitXT filter;
    QCOMPARE(filter.name(), std::string("CircleFitXT"));
    QVERIFY(filter.findPipe("CircleX") != nullptr);
    QVERIFY(filter.findPipe("CircleY") != nullptr);
    QVERIFY(filter.findPipe("CircleR") != nullptr);
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);
    
    // check parameters of type double
    for ( auto entry : std::vector<std::pair<std::string, double>> {
                            {"HoughRadiusStep", 1.0}
                            })
    {
        auto parameter = entry.first;
        auto value = entry.second;
        QVERIFY(filter.getParameters().exists(parameter));
        QCOMPARE(filter.getParameters().findParameter(parameter).getType(), fliplib::Parameter::TYPE_double);
        QCOMPARE(filter.getParameters().findParameter(parameter).getValue().convert<double>(), value);
    }
    // check parameters of type int
    for (auto entry : std::vector<std::pair<std::string, int>> {
                                    {"Algorithm", 1},
                                    {"HoughScoreType", 0},
                                    {"HoughScoreThreshold", -1},
                                    {"HoughCandidates", 1}})
    {
        auto parameter = entry.first;
        auto value = entry.second;
        QVERIFY(filter.getParameters().exists(parameter));
        QCOMPARE(filter.getParameters().findParameter(parameter).getType(), fliplib::Parameter::TYPE_int);
        QCOMPARE(filter.getParameters().findParameter(parameter).getValue().convert<int>(), value);   
    }
    
}



void TestCircleFitXT::testProceed_data()
{
    QTest::addColumn<int>("parameterAlgorithm");
    QTest::addColumn<int>("numPoints");
    QTest::addColumn<double>("angleStart");
    QTest::addColumn<double>("angleEnd");
    QTest::addColumn<bool>("withNoise");
    QTest::addColumn<int>("offsetStart");
    QTest::addColumn<int>("offsetEnd");
    QTest::addColumn<bool>("expectedResultInValidArea");

    for (int offsetStart : {-1000, -2, 3 })
    {
        for (int offsetEnd : {1000, 2, -3 })
        {
            bool expectedResultInValidArea = (offsetStart < 0 && offsetEnd > 0);

            //TODO check why least squares does not fit a full circle
            QTest::newRow("LeastSquaresCircleFit_Arc") << 0 << 10 << 0.0 << 90.0 << false << offsetStart << offsetEnd << expectedResultInValidArea;

            QTest::newRow("LeastSquaresCircleFit_Sparse") << 0 << 5 << 0.0 << 350.0 << false << offsetStart << offsetEnd << expectedResultInValidArea;
            QTest::newRow("LeastSquaresCircleFit_50") << 0 << 50 << 10.0 << 360.0 << false << offsetStart << offsetEnd << expectedResultInValidArea;
            QTest::newRow("HoughCircleFit_Arc") <<  1 << 7 << 0.0 << 90.0 << true << offsetStart << offsetEnd << expectedResultInValidArea;
            QTest::newRow("HoughCircleFit_Sparse") <<  1 << 5 << 0.0 << 360.0 << true << offsetStart << offsetEnd << expectedResultInValidArea;
            QTest::newRow("HoughCircleFit_50") << 1 << 50 << 0.0 << 360.0 << true << offsetStart << offsetEnd << expectedResultInValidArea;


        }
    }

}
void TestCircleFitXT::testProceed()
{
    precitec::filter::CircleFitXT filter;
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
    auto outPipeScore = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("Score"));
    QVERIFY(outPipeScore);
    
    DummyFilter dummyFilter;
    QVERIFY(dummyFilter.connectPipe(outPipeX, 0));
    QVERIFY(dummyFilter.connectPipe(outPipeY, 0));
    QVERIFY(dummyFilter.connectPipe(outPipeRadius, 0));
    QVERIFY(dummyFilter.connectPipe(outPipeScore, 0));

    QFETCH(int, parameterAlgorithm);
    QFETCH(int, numPoints);
    QFETCH(double, angleStart);
    QFETCH(double, angleEnd);
    QFETCH(bool, withNoise);
    QFETCH(int, offsetStart);
    QFETCH(int, offsetEnd);
    QFETCH(bool, expectedResultInValidArea);

    filter.getParameters().update(std::string("Algorithm"), fliplib::Parameter::TYPE_int, parameterAlgorithm);
    filter.setParameter();
    
    precitec::geo2d::DPoint expectedCenterInROI{50,48.0};
    double expectedRadius = 10;

    int imageNumber = 0;
    precitec::geo2d::Point trafoOffset {510,100};

    precitec::geo2d::Point validAreaStartInCanvas(expectedCenterInROI.x + trafoOffset.x + offsetStart, expectedCenterInROI.y + trafoOffset.y + offsetStart);
    precitec::geo2d::Point validAreaEndInCanvas(expectedCenterInROI.x + trafoOffset.x + offsetEnd , expectedCenterInROI.y + trafoOffset.y + offsetEnd);

    dummyInput.fillData(imageNumber, trafoOffset, expectedCenterInROI, expectedRadius,
                        numPoints, angleStart, angleEnd, withNoise,
                        validAreaStartInCanvas, validAreaEndInCanvas);


    QCOMPARE(dummyFilter.isProceedCalled(), false);
    dummyInput.signal();
    QCOMPARE(dummyFilter.isProceedCalled(), true);
    dummyFilter.resetProceedCalled();

    const auto resultX = outPipeX->read(imageNumber);
    const auto resultY = outPipeY->read(imageNumber);
    const auto resultRadius = outPipeRadius->read(imageNumber);
    const auto resultScore = outPipeScore->read(imageNumber);

    //check the size of the output
    QCOMPARE(resultX.ref().size(), 1ul);
    QCOMPARE(resultY.ref().size(), 1ul);
    QCOMPARE(resultRadius.ref().size(), 1ul);
    QCOMPARE(resultScore.ref().size(), 1ul);

    //check the trafo of the output
    for (auto * pContext : {&resultX.context(), &resultY.context()})
    {
        QCOMPARE(pContext->getTrafoX(), trafoOffset.x);
        QCOMPARE(pContext->getTrafoY(), trafoOffset.y);
    }

    //check that all the outputs have the same rank
    QVERIFY(resultX.ref().getRank().front() == resultY.ref().getRank().front());
    QVERIFY(resultX.ref().getRank().front() == resultRadius.ref().getRank().front());
    QVERIFY(resultScore.ref().getRank().front() == 255); //score has always max rank

    // check that the output is consistent with the input

    auto resultRank = resultX.ref().getRank().front();
    auto x = resultX.ref().getData().front();
    auto y = resultY.ref().getData().front();

    if (resultRank != 0)
    {
        auto validAreaStartInROI = resultX.context().trafo()->applyReverse(validAreaStartInCanvas);
        auto validAreaEndInROI = resultX.context().trafo()->applyReverse(validAreaEndInCanvas);
        QVERIFY(x >= validAreaStartInROI.x && x <= validAreaEndInROI.x);
        QVERIFY(y >= validAreaStartInROI.y && y <= validAreaEndInROI.y);
        QVERIFY(resultScore.ref().getData().front() > 0);
    }

    // compare to the expected result
    if ( expectedResultInValidArea )
    {
        QCOMPARE(x, expectedCenterInROI.x);
        QCOMPARE(y, expectedCenterInROI.y);
        QCOMPARE(resultRadius.ref().getData().front(), expectedRadius);
        QCOMPARE(resultRank, 255);
    }

}



QTEST_GUILESS_MAIN(TestCircleFitXT)
#include "testCircleFitXT.moc"
