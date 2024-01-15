
#include <QtTest/QtTest>

#include "../conditionalContour.h"
#include <fliplib/NullSourceFilter.h>

#include <fliplib/BaseFilter.h>

using precitec::interface::GeoVecAnnotatedDPointarray;

Q_DECLARE_METATYPE(precitec::geo2d::DPoint);

// If some printed output is required
#define  TEXT_OUT  0

class ConditionalContourTest : public QObject
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

void ConditionalContourTest::testCtor()
{
    precitec::filter::ConditionalContour filter;

    QCOMPARE(filter.name(), std::string("ConditionalContour"));
    QVERIFY(filter.findPipe("ContourOut") != nullptr);
    QVERIFY(filter.findPipe("ContourOut2") == nullptr);
}


void ConditionalContourTest::testProceed_data()
{

    QTest::addColumn< std::vector<std::vector<precitec::geo2d::DPoint>> > ("param_contour_a");   // contourA
    QTest::addColumn< std::vector<std::vector<precitec::geo2d::DPoint>> > ("param_contour_b");   // contourB
    QTest::addColumn< std::vector<double> >                               ("param_quality_a");   // QualityA
    QTest::addColumn< std::vector<double> >                               ("param_quality_b");   // QualityB
    QTest::addColumn< std::vector<std::vector<precitec::geo2d::DPoint>> > ("expectedContours");  // expectedContours
    QTest::addColumn< std::vector<int> >                                  ("expectedResult");    // expectedResult

    QTest::newRow("Test qa > qb")   // Displayed test name

            << std::vector<std::vector<precitec::geo2d::DPoint>>  { {precitec::geo2d::DPoint(6.2, 2.2),       precitec::geo2d::DPoint(5.2, 0.3),     precitec::geo2d::DPoint(5.2, 10.0)} }
            << std::vector<std::vector<precitec::geo2d::DPoint>>  { {precitec::geo2d::DPoint(-65.2, 24.2),    precitec::geo2d::DPoint(63.2, -657.3), precitec::geo2d::DPoint(5.234, 106.5640)} }
            << std::vector<double>                                { 0.5 }
            << std::vector<double>                                { 0.4 }
            << std::vector<std::vector<precitec::geo2d::DPoint>>  { {precitec::geo2d::DPoint(6.2, 2.2),       precitec::geo2d::DPoint(5.2, 0.3),     precitec::geo2d::DPoint(5.2, 10.0)} }
            << std::vector<int>                                   { 1 };

    QTest::newRow("Test qa < qb")

            << std::vector<std::vector<precitec::geo2d::DPoint>>  { {precitec::geo2d::DPoint(6.2, 37.4),      precitec::geo2d::DPoint(5.2, 0.3),     precitec::geo2d::DPoint(5.2, -100.46)} }
            << std::vector<std::vector<precitec::geo2d::DPoint>>  { {precitec::geo2d::DPoint(-65.1, 24.1),    precitec::geo2d::DPoint(678, -657.3),  precitec::geo2d::DPoint(5.234, 10.5640)} }
            << std::vector<double>                                { -0.5 }
            << std::vector<double>                                { 77 }
            << std::vector<std::vector<precitec::geo2d::DPoint>>  { {precitec::geo2d::DPoint(-65.1, 24.1),    precitec::geo2d::DPoint(678, -657.3),  precitec::geo2d::DPoint(5.234, 10.5640)} }
            << std::vector<int>                                   { -1 };

    QTest::newRow("Test qa == qb")

            << std::vector<std::vector<precitec::geo2d::DPoint>>  { {precitec::geo2d::DPoint(-6.457, -37.4),  precitec::geo2d::DPoint(5.2, 547.3),   precitec::geo2d::DPoint(0.0, -100.46)} }
            << std::vector<std::vector<precitec::geo2d::DPoint>>  { {precitec::geo2d::DPoint(-65.2, 24.2),    precitec::geo2d::DPoint(678, -657.3)} }
            << std::vector<double>                                { -77 }
            << std::vector<double>                                { -77 }
            << std::vector<std::vector<precitec::geo2d::DPoint>>  { {precitec::geo2d::DPoint(-6.457, -37.4),  precitec::geo2d::DPoint(5.2, 547.3),   precitec::geo2d::DPoint(0.0, -100.46)} }
            << std::vector<int>                                   { 1 };

    QTest::newRow("Test oversampling one quality values qa > qb")

            << std::vector<std::vector<precitec::geo2d::DPoint>>  { {precitec::geo2d::DPoint(-47.457, -37.4), precitec::geo2d::DPoint(5.2, 547.3),   precitec::geo2d::DPoint(0.0, -100.46)} }
            << std::vector<std::vector<precitec::geo2d::DPoint>>  { {precitec::geo2d::DPoint(-65.2, 24.2),    precitec::geo2d::DPoint(678, -657.3),  precitec::geo2d::DPoint(346.3, 946.258)},
                                                                    {precitec::geo2d::DPoint(-585.2, -288.2), precitec::geo2d::DPoint(4590, -657.7), precitec::geo2d::DPoint(-4869.3, 9.258)}
                                                                  }
            << std::vector<double>                                { 0.5 }
            << std::vector<double>                                { 0.4 }
            << std::vector<std::vector<precitec::geo2d::DPoint>>  { {precitec::geo2d::DPoint(-47.457, -37.4), precitec::geo2d::DPoint(5.2, 547.3),   precitec::geo2d::DPoint(0.0, -100.46)} }
            << std::vector<int>                                   { 1 };

    QTest::newRow("Test oversampling mixed quality values")

            << std::vector<std::vector<precitec::geo2d::DPoint>>  { {precitec::geo2d::DPoint(-6.457, -37.4),  precitec::geo2d::DPoint(5.2, 547.3),   precitec::geo2d::DPoint(0.0, -100.46)} }
            << std::vector<std::vector<precitec::geo2d::DPoint>>  { {precitec::geo2d::DPoint(-65.2, 24.2),    precitec::geo2d::DPoint(678, -657.3),  precitec::geo2d::DPoint(346.3, 946.258)},
                                                                    {precitec::geo2d::DPoint(-585.2, -288.2), precitec::geo2d::DPoint(4590, -657.7), precitec::geo2d::DPoint(-4869.3, 9.258)}
                                                                  }
            << std::vector<double>                                { 0.5, 0.3, -112 }
            << std::vector<double>                                { 0.6, 0.2, 1111 }
            << std::vector<std::vector<precitec::geo2d::DPoint>>  { {precitec::geo2d::DPoint(-65.2, 24.2),    precitec::geo2d::DPoint(678, -657.3),  precitec::geo2d::DPoint(346.3, 946.258)},
                                                                    {precitec::geo2d::DPoint(-585.2, -288.2), precitec::geo2d::DPoint(4590, -657.7), precitec::geo2d::DPoint(-4869.3, 9.258)}
                                                                  }
            << std::vector<int>                                   { -1 };

    QTest::newRow("Test mixed quality values")

            << std::vector<std::vector<precitec::geo2d::DPoint>>  { {precitec::geo2d::DPoint(-0.1, -37.4),  precitec::geo2d::DPoint(5.2, 547.3),   precitec::geo2d::DPoint(0.0, -100.46)},
                                                                    {precitec::geo2d::DPoint(-0.2, -288.2), precitec::geo2d::DPoint(4590, -657.7), precitec::geo2d::DPoint(-469.3, 9.258)},
                                                                    {precitec::geo2d::DPoint(-0.3, -288.2), precitec::geo2d::DPoint(490, -657.7), precitec::geo2d::DPoint(489.3, -9.258)}
                                                                  }
            << std::vector<std::vector<precitec::geo2d::DPoint>>  { {precitec::geo2d::DPoint(-1.1, 24.2),    precitec::geo2d::DPoint(678, -657.3),  precitec::geo2d::DPoint(346.3, 946.258)},
                                                                    {precitec::geo2d::DPoint(-1.2, -288.2), precitec::geo2d::DPoint(4590, -657.7), precitec::geo2d::DPoint(-4869.3, 9.258)},
                                                                    {precitec::geo2d::DPoint(-1.3, -37.4),  precitec::geo2d::DPoint(5.2, 547.3),   precitec::geo2d::DPoint(0.0, -100.46)}
                                                                  }
            << std::vector<double>                                { 0.5, 0.3, -112 }
            << std::vector<double>                                { 0.6, 0.2, 1111 }
            << std::vector<std::vector<precitec::geo2d::DPoint>>  { {precitec::geo2d::DPoint(-1.1, 24.2),    precitec::geo2d::DPoint(678, -657.3),  precitec::geo2d::DPoint(346.3, 946.258)},
                                                                    {precitec::geo2d::DPoint(-0.2, -288.2), precitec::geo2d::DPoint(4590, -657.7), precitec::geo2d::DPoint(-469.3, 9.258)},
                                                                    {precitec::geo2d::DPoint(-1.3, -37.4),  precitec::geo2d::DPoint(5.2, 547.3),   precitec::geo2d::DPoint(0.0, -100.46)}
                                                                  }
            << std::vector<int>                                   { -1, 1, -1 };

    QTest::newRow("Test oversampling diffrent no. of quality values")

            << std::vector<std::vector<precitec::geo2d::DPoint>>  { {precitec::geo2d::DPoint(-6.457, -37.4),  precitec::geo2d::DPoint(5.2, 547.3),   precitec::geo2d::DPoint(0.0, -100.46)} }
            << std::vector<std::vector<precitec::geo2d::DPoint>>  { {precitec::geo2d::DPoint(-65.2, 24.2),    precitec::geo2d::DPoint(678, -657.3),  precitec::geo2d::DPoint(346.3, 946.258)} }
            << std::vector<double>                                { 0.0, 0.3, -112 }
            << std::vector<double>                                { 666 }
            << std::vector<std::vector<precitec::geo2d::DPoint>>  { {precitec::geo2d::DPoint(-65.2, 24.2),    precitec::geo2d::DPoint(678, -657.3),  precitec::geo2d::DPoint(346.3, 946.258)} }
            << std::vector<int>                                   { -1 };

    QTest::newRow("Test oversampling empty quality values")

            << std::vector<std::vector<precitec::geo2d::DPoint>>  { {precitec::geo2d::DPoint(-6.457, -37.4),  precitec::geo2d::DPoint(5.2, 547.3),   precitec::geo2d::DPoint(0.0, -100.46)},
                                                                    {precitec::geo2d::DPoint(-9.37, -37.4),   precitec::geo2d::DPoint(84.4, 547.3),  precitec::geo2d::DPoint(2.0, -10.49)}
                                                                  }
            << std::vector<std::vector<precitec::geo2d::DPoint>>  { {precitec::geo2d::DPoint(-65.2, 24.2),    precitec::geo2d::DPoint(678, -657.3),  precitec::geo2d::DPoint(346.3, 946.258)} }
            << std::vector<double>                                {  }
            << std::vector<double>                                {  }
            << std::vector<std::vector<precitec::geo2d::DPoint>>  {  }
            << std::vector<int>                                   {  };

    QTest::newRow("Test empty pipe contour")

            << std::vector<std::vector<precitec::geo2d::DPoint>>  {  }
            << std::vector<std::vector<precitec::geo2d::DPoint>>  { {precitec::geo2d::DPoint(-65.2, 24.2),    precitec::geo2d::DPoint(346.3, 946.258)} }
            << std::vector<double>                                { 0.0, 0.3, -112 }
            << std::vector<double>                                { 666 }
            << std::vector<std::vector<precitec::geo2d::DPoint>>  {  }
            << std::vector<int>                                   {  };
}

void ConditionalContourTest::testProceed()
{
    precitec::filter::ConditionalContour filter;

    // In-Pipe "data_a"
    fliplib::NullSourceFilter sourceFilterContourA;
    fliplib::SynchronePipe< precitec::interface::GeoVecAnnotatedDPointarray >  pipeDA{ &sourceFilterContourA, "contour_a" };
    pipeDA.setTag("contour_a");
    QVERIFY(filter.connectPipe(&pipeDA, 1));

    // In-Pipe "data_b"
    fliplib::NullSourceFilter sourceFilterContourB;
    fliplib::SynchronePipe< precitec::interface::GeoVecAnnotatedDPointarray >  pipeDB{ &sourceFilterContourB, "contour_b" };
    pipeDB.setTag("contour_b");
    QVERIFY(filter.connectPipe(&pipeDB, 1));

    // In-Pipe "quality_a"
    fliplib::NullSourceFilter sourceFilterQualA;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeQA{ &sourceFilterQualA, "quality_a" };
    pipeQA.setTag("quality_a");
    QVERIFY(filter.connectPipe(&pipeQA, 1));

    // In-Pipe "quality_b"
    fliplib::NullSourceFilter sourceFilterQualB;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeQB{ &sourceFilterQualB, "quality_b" };
    pipeQB.setTag("quality_b");
    QVERIFY(filter.connectPipe(&pipeQB, 1));

    // Create Out-Pipes and connect to Conditional filter
    // --------------------------------------------------

    DummyFilter filterOutData;
    QVERIFY(filterOutData.connectPipe(filter.findPipe("ContourOut"), 1));
    QVERIFY(filterOutData.connectPipe(filter.findPipe("OperationResult_out"), 1));

    // dummy data

    int imageNumber  =   0;
    int position     = 300;

    precitec::interface::ImageContext context;
    context.setImageNumber(imageNumber);
    context.setPosition(position);

    // parse test data

    QFETCH(std::vector<std::vector<precitec::geo2d::DPoint>> , param_contour_a);
    QFETCH(std::vector<std::vector<precitec::geo2d::DPoint>> , param_contour_b);
    QFETCH(std::vector<double>                               , param_quality_a);
    QFETCH(std::vector<double>                               , param_quality_b);
    QFETCH(std::vector<std::vector<precitec::geo2d::DPoint>> , expectedContours);
    QFETCH(std::vector<int>                                  , expectedResult);

    // convert test array into appropriate data structure
    std::vector<precitec::geo2d::TAnnotatedArray<precitec::geo2d::DPoint>> contourA;
    auto addA = [&](const std::vector<precitec::geo2d::DPoint>& a)
    {
        precitec::geo2d::TAnnotatedArray<precitec::geo2d::DPoint> contourTAA;
        contourTAA.getData().insert( contourTAA.getData().begin(), a.begin(), a.end());
        contourTAA.getRank().assign(contourTAA.getData().size(), precitec::filter::eRankMax);
        contourA.push_back(contourTAA);
    };
    std::for_each(param_contour_a.begin(), param_contour_a.end(), addA);


    std::vector<precitec::geo2d::TAnnotatedArray<precitec::geo2d::DPoint>>	contourB;
    auto addB = [&](const std::vector<precitec::geo2d::DPoint>& a)
    {
        precitec::geo2d::TAnnotatedArray<precitec::geo2d::DPoint> contourTAA;
        contourTAA.getData().insert( contourTAA.getData().begin(), a.begin(), a.end());
        contourTAA.getRank().assign(contourTAA.getData().size(), precitec::filter::eRankMax);
        contourB.push_back(contourTAA);
    };
    std::for_each(param_contour_b.begin(), param_contour_b.end(), addB);

    // signal pipes
    auto inPipe = precitec::interface::GeoVecAnnotatedDPointarray {
        context,
        contourA,
        precitec::interface::ResultType::AnalysisOK,
        precitec::interface::Limit
    };
    inPipe.rank(255);
    pipeDA.signal(inPipe);

    inPipe = precitec::interface::GeoVecAnnotatedDPointarray {
        context,
        contourB,
        precitec::interface::ResultType::AnalysisOK,
        precitec::interface::Limit
    };
    inPipe.rank(255);
    pipeDB.signal(inPipe);

    precitec::geo2d::Doublearray qada;
    qada.getData().insert(qada.getData().begin(), param_quality_a.begin(), param_quality_a.end());
    qada.getRank().assign(qada.getData().size(), precitec::filter::eRankMax);

    auto qinPipe = precitec::interface::GeoDoublearray {
        context,
        qada,
        precitec::interface::ResultType::AnalysisOK,
        precitec::interface::Limit
    };
    pipeQA.signal(qinPipe);

    precitec::geo2d::Doublearray qbda;
    qbda.getData().insert(qbda.getData().begin(), param_quality_b.begin(), param_quality_b.end());
    qbda.getRank().assign(qbda.getData().size(), precitec::filter::eRankMax);

    qinPipe = precitec::interface::GeoDoublearray {
        context,
        qbda,
        precitec::interface::ResultType::AnalysisOK,
        precitec::interface::Limit
    };
    pipeQB.signal(qinPipe);

    //verify that the filter has run
    QVERIFY(filterOutData.isProceedCalled());

    // compare signaled data

    // check if Line is correct
    auto outPipeData = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoVecAnnotatedDPointarray>*>(filter.findPipe("ContourOut"));
    QVERIFY(outPipeData);
    QCOMPARE(outPipeData->read( context.imageNumber()).ref().size() , expectedContours.size() );
    for (size_t i= 0; i < outPipeData->read(context.imageNumber()).ref().size() && i < expectedContours.size() ; i++)
    {
#if TEXT_OUT
        std::cout << "Output pipe 'Data': " << outPipeData->read(context.imageNumber()).ref().at(i).getData()[0] << "  exp: " << expectedContours.at(i)[0] << std::endl;
#endif
        QCOMPARE(outPipeData->read(context.imageNumber()).ref().at(i).getData(), expectedContours.at(i));
        QCOMPARE(outPipeData->read(context.imageNumber()).ref()[i].getData().size() , expectedContours[i].size() );
    }

    // check if compare result is correct
    auto outPipeCompResult = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("OperationResult_out"));
    QVERIFY(outPipeCompResult);
    QCOMPARE(outPipeCompResult->read(context.imageNumber()).ref().size(), expectedResult.size());

#if TEXT_OUT
    std::cout << "       Queue size : " << outPipeCompResult->read(context.imageNumber()).ref().size() << "  exp: " << expectedResult.size() << std::endl;
#endif

    for (size_t i= 0; i < outPipeCompResult->read(context.imageNumber()).ref().size() && i < expectedResult.size(); i++)
    {
         QCOMPARE(outPipeCompResult->read(context.imageNumber()).ref().getData()[i], expectedResult[i]);
    }
}

QTEST_GUILESS_MAIN(ConditionalContourTest)

#include "conditionalContourTest.moc"
