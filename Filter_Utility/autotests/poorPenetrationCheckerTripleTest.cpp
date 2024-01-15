

#include <QtTest/QtTest>

#include "../poorPenetrationCheckerTriple.h"
#include <fliplib/NullSourceFilter.h>

#include <fliplib/BaseFilter.h>

Q_DECLARE_METATYPE(precitec::interface::GeoPoorPenetrationCandidatearray);
Q_DECLARE_METATYPE(precitec::geo2d::PoorPenetrationCandidate);

// If some printed output is required
#define  TEXT_OUT  0

class PoorPenetrationCheckerTripleTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testProceed();
    void testProceed_data();
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

    void proceedGroup ( const void * sender, fliplib::PipeGroupEventArgs & e ) override
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

void PoorPenetrationCheckerTripleTest::testCtor()
{
    precitec::filter::PoorPenetrationCheckerTriple filter;

    QCOMPARE(filter.name(), std::string("PoorPenetrationCheckerTriple"));
    QVERIFY(filter.findPipe("DataOut") != nullptr);

    QCOMPARE(filter.getParameters().findParameter(std::string("ActiveParams")).getType(), fliplib::Parameter::TYPE_int);
}


void PoorPenetrationCheckerTripleTest::testProceed_data()
{
    QTest::addColumn< std::vector<precitec::geo2d::PoorPenetrationCandidate> >  ("param_dataIn");
    QTest::addColumn< std::vector<int> >                                        ("expectedResult");

    QTest::newRow("Test 1")
            // width, length, gradient, greyvalGap, greyvalInner, greyvalOuter, stdDev, devLengthL, devLengthR
            << std::vector<precitec::geo2d::PoorPenetrationCandidate> { precitec::geo2d::PoorPenetrationCandidate( 5, 200,  70, 70, 15,  15, 25, 400, 500)
                                                                      , precitec::geo2d::PoorPenetrationCandidate(11, 201,  80, 71, 15,  25, 25, 700, 701)
                                                                      , precitec::geo2d::PoorPenetrationCandidate(35, 400, 251, 70, 15,  15, 25, 700, 499)
                                                                      }
            << std::vector<int>                                       { 131 };

    QTest::newRow("Test 2")
             << std::vector<precitec::geo2d::PoorPenetrationCandidate> { precitec::geo2d::PoorPenetrationCandidate(10, 200,  80, 70, 15,  25, 25, 500, 500)
                                                                       , precitec::geo2d::PoorPenetrationCandidate(33, 220, 240, 77, 15,  27, 25, 700, 499)
                                                                       }
             << std::vector<int>                                       { 222 };

    QTest::newRow("Test 3")
             << std::vector<precitec::geo2d::PoorPenetrationCandidate> { precitec::geo2d::PoorPenetrationCandidate(10, 200,  80, 120, 100, 245, 25,  900, 1000)
                                                                       , precitec::geo2d::PoorPenetrationCandidate(33, 220, 240, 110,  15,  27, 25, 1000,  959)
                                                                       }
             << std::vector<int>                                       { 21 };

    QTest::newRow("Test 4") //set1:3,5,7 set2:7 set3:1,3,7
             << std::vector<precitec::geo2d::PoorPenetrationCandidate> { precitec::geo2d::PoorPenetrationCandidate(34, 200,  75, 120, 100, 245, 25,  100, 300)
                                                                       }
             << std::vector<int>                                       { 0 };

    QTest::newRow("Test 5") //set1:3,5 set2:0 set3:1,3
             << std::vector<precitec::geo2d::PoorPenetrationCandidate> { precitec::geo2d::PoorPenetrationCandidate(34, 200,  75, 120, 100, 245, 25,  700, 300)
                                                                       }
             << std::vector<int>                                       { 10 };
}

void PoorPenetrationCheckerTripleTest::testProceed()
{
    precitec::filter::PoorPenetrationCheckerTriple filter;

    // In-Pipe
    fliplib::NullSourceFilter sourceFilter;
    fliplib::SynchronePipe< precitec::interface::GeoPoorPenetrationCandidatearray >  pipeIn{ &sourceFilter, "DataIn" };
    pipeIn.setTag("DataIn");
    QVERIFY(filter.connectPipe(&pipeIn, 0));

    // Create Out-Pipe and connect to PoorPenetrationCheckerTriple filter
    DummyFilter filterOutData;
    QVERIFY(filterOutData.connectPipe(filter.findPipe("DataOut"), 0));

    // update Parameters
    filter.getParameters().update(std::string("Verbosity"), fliplib::Parameter::TYPE_int, 0);
    filter.getParameters().update(std::string("ActiveParamSets"), fliplib::Parameter::TYPE_int, 3);
    filter.getParameters().update(std::string("ActiveParams"), fliplib::Parameter::TYPE_int, 7);
    filter.getParameters().update(std::string("MinWidth"), fliplib::Parameter::TYPE_int, 10);
    filter.getParameters().update(std::string("MinRatioInnerOuter"), fliplib::Parameter::TYPE_int, 10);
    filter.getParameters().update(std::string("MaxRatioInnerOuter"), fliplib::Parameter::TYPE_int, 24);
    filter.getParameters().update(std::string("ActiveParams1"), fliplib::Parameter::TYPE_int, 7);
    filter.getParameters().update(std::string("MinGradient1"), fliplib::Parameter::TYPE_int, 70);
    filter.getParameters().update(std::string("MaxGradient1"), fliplib::Parameter::TYPE_int, 260);
    filter.getParameters().update(std::string("MinRatioInnerOuter1"), fliplib::Parameter::TYPE_int, 10);
    filter.getParameters().update(std::string("MinDevelopedLength1"), fliplib::Parameter::TYPE_int, 450);
    filter.getParameters().update(std::string("ActiveParams2"), fliplib::Parameter::TYPE_int, 7);
    filter.getParameters().update(std::string("MaxWidth2"), fliplib::Parameter::TYPE_int, 33);
    filter.getParameters().update(std::string("MaxLength2"), fliplib::Parameter::TYPE_int, 500);
    filter.getParameters().update(std::string("MaxGradient2"), fliplib::Parameter::TYPE_int, 240);
    filter.getParameters().update(std::string("MinGreyvalGap2"), fliplib::Parameter::TYPE_int, 50);
    filter.getParameters().update(std::string("MaxGreyvalGap2"), fliplib::Parameter::TYPE_int, 150);
    filter.getParameters().update(std::string("MinRatioInnerOuter2"), fliplib::Parameter::TYPE_int, 15);
    filter.getParameters().update(std::string("MaxRatioInnerOuter2"), fliplib::Parameter::TYPE_int, 30);
    filter.getParameters().update(std::string("MaxStandardDeviation2"), fliplib::Parameter::TYPE_int, 40);
    filter.getParameters().update(std::string("MaxDevelopedLength2"), fliplib::Parameter::TYPE_int, 900);
    filter.setParameter();

    // dummy data
    int imageNumber  =   0;
    int position     = 300;

    precitec::interface::ImageContext context;
    context.setImageNumber(imageNumber);
    context.setPosition(position);

    // parse test data
    QFETCH(std::vector<precitec::geo2d::PoorPenetrationCandidate>, param_dataIn);
    QFETCH(std::vector<int>, expectedResult);

    // convert test array into appropriate data structure
    precitec::geo2d::PoorPenetrationCandidatearray dataIn;
    for (const auto& ppc: param_dataIn)
    {
        dataIn.getData().push_back(ppc);
        dataIn.getRank().push_back(precitec::filter::eRankMax);
    }
    auto inPipe = precitec::interface::GeoPoorPenetrationCandidatearray (context, dataIn, precitec::interface::ResultType::AnalysisOK, precitec::filter::eRankMax);
    std::cout << "inPipe size " << inPipe.ref().size() << ", " << inPipe.ref().getData().size() << ", dataIn size " << param_dataIn.size() << ", " << inPipe.rank() << std::endl; // TODO delete
    pipeIn.signal(inPipe);

    //verify that the filter has run
    QVERIFY(filterOutData.isProceedCalled());

    // compare signaled data
    auto outPipeData = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("DataOut"));
    QVERIFY(outPipeData);
    QCOMPARE(outPipeData->read( context.imageNumber()).ref().size() , expectedResult.size() );

    for (size_t i= 0; i < outPipeData->read(context.imageNumber()).ref().size() && i < expectedResult.size(); i++)
    {
         QCOMPARE(outPipeData->read(context.imageNumber()).ref().getData()[i], expectedResult[i]);
    }
}

QTEST_GUILESS_MAIN(PoorPenetrationCheckerTripleTest)

#include "poorPenetrationCheckerTripleTest.moc"
