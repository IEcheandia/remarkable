#include <QtTest/QtTest>

#include "../conditionalLine.h"
#include <fliplib/NullSourceFilter.h>

#include <fliplib/BaseFilter.h>

using precitec::geo2d::Doublearray;

class TestConditionalLine : public QObject
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

void TestConditionalLine::testCtor()
{
    precitec::filter::ConditionalLine filter;

    QCOMPARE(filter.name(), std::string("ConditionalLine"));
    QVERIFY(filter.findPipe("LineOut") != nullptr);
    QVERIFY(filter.findPipe("LineOut2") == nullptr);
}


void TestConditionalLine::testProceed_data()
{

    QTest::addColumn< std::vector<std::vector<double>> > ("param_line_a");      // linesA
    QTest::addColumn< std::vector<std::vector<double>> > ("param_line_b");      // linesB
    QTest::addColumn< std::vector<double> >              ("param_quality_a");   // QualityA
    QTest::addColumn< std::vector<double> >              ("param_quality_b");   // QualityB
    QTest::addColumn< std::vector<std::vector<double>> > ("expectedLines");     // expectedLines
    QTest::addColumn< std::vector<int>                 > ("expectedResult");    // expectedResult

    QTest::newRow("Test qa > qb")   // Displayed test name

            << std::vector<std::vector<double>>  { { 6.2,  5.2,  5.2 } }
            << std::vector<std::vector<double>>  { { -23.12,  4.232,  -321.3213 } }
            << std::vector<double>                 { 0.5 }
            << std::vector<double>                 { 0.4 }
            << std::vector<std::vector<double>>  { { 6.2,  5.2,  5.2 } }
            << std::vector<int>                    { 1 };

    QTest::newRow("Test qa < qb")

            << std::vector<std::vector<double>>  { { 322,  32312,  321312, 523523, 23423 } }
            << std::vector<std::vector<double>>  { { 2.1,  2.1,  2.1 } }
            << std::vector<double>                 { -0.5 }
            << std::vector<double>                 { 77 }
            << std::vector<std::vector<double>>  { { 2.1,  2.1,  2.1 } }
            << std::vector<int>                    { -1 };

    QTest::newRow("Test qa == qb")

            << std::vector<std::vector<double>>  { { 322,  32312,  321312, 523523, 23423, 322,  32312,  321312, 523523, 23423 } }
            << std::vector<std::vector<double>>  { { 2.1,  2.1,  2.1  } }
            << std::vector<double>                 { -77 }
            << std::vector<double>                 { -77 }
            << std::vector<std::vector<double>>  { { 322,  32312,  321312, 523523, 23423, 322,  32312,  321312, 523523, 23423 } }
            << std::vector<int>                    { 1 };

    QTest::newRow("Test oversampling one quality value qa > qb")

            << std::vector<std::vector<double>>  { { 6.2,  5.2,  5.2 }, {  6.2,  5.2,  5.2 } , {  -423423,  0.0,  0.000003 } }
            << std::vector<std::vector<double>>  { { -23.12,  4.232,  -321.3213 }, {  -23.12,  4.232,  -321.3213 } , { -23.12,  4.232, -321.3213 } }
            << std::vector<double>                 { 0.5 }
            << std::vector<double>                 { 0.4 }
            << std::vector<std::vector<double>>  { { 6.2,  5.2,  5.2 }, {  6.2,  5.2,  5.2 } , {  -423423,  0.0,  0.000003 } }
            << std::vector<int>                    { 1, 1, 1 };

    QTest::newRow("Test oversampling one quality value qa < qb")

            << std::vector<std::vector<double>>  { { 6.2,  5.2,  5.2 }, {  6.2,  5.2,  5.2 } , {  -423423,  0.0,  0.000003 } }
            << std::vector<std::vector<double>>  { { -23.12,  4.232,  -321.3213 }, {  -23.12,  4.232,  -321.3213 } , {  -23.12,  4.232,  -321.3213 } }
            << std::vector<double>                 { -5423523 }
            << std::vector<double>                 {  5423523 }
            << std::vector<std::vector<double>>  { { -23.12,  4.232,  -321.3213 }, {  -23.12,  4.232,  -321.3213 } , {  -23.12,  4.232,  -321.3213 } }
            << std::vector<int>                    { -1, -1, -1 };

    QTest::newRow("Test oversampling mixed quality values")

            << std::vector<std::vector<double>>  { { 6.2,  5.2,  5.2 }, {  6.2,  5.2,  5.2 } , {  6.2,  5.2,  5.2 } }                                   // linesA
            << std::vector<std::vector<double>>  { { -23.12,  4.232,  -321.3213 }, {  -23.12,  4.232,  -321.3213 } , {  -23.12,  4.232,  -321.3213 } }  // linesB
            << std::vector<double>                 { 0.5, 0.3, -112 }                                                                                   // QualityA
            << std::vector<double>                 { 0.4, 0.2, 1111 }                                                                                   // QualityB
            << std::vector<std::vector<double>>  { { 6.2,  5.2,  5.2 }, {  6.2,  5.2,  5.2 }, {  -23.12,  4.232,  -321.3213 } }                         // expectedLines
            << std::vector<int>                    { 1, 1, -1 };                                                                                        // expectedResult

    QTest::newRow("Test oversampling diffrent no. of quality values")

            << std::vector<std::vector<double>>  { { 6.2,  5.2,  5.2 }, {  6.2,  5.2,  5.2 } , {  6.2,  5.2,  5.2 } }
            << std::vector<std::vector<double>>  { { -23.12,  4.232,  -321.3213 }, {  -23.12,  4.232,  -321.3213 } , {  -23.12,  4.232,  -321.3213 } }
            << std::vector<double>                 { 0.0, 0.3, -112 }
            << std::vector<double>                 { 666 }
            << std::vector<std::vector<double>>  { { -23.12,  4.232,  -321.3213 }, {  -23.12,  4.232,  -321.3213 } , {  -23.12,  4.232,  -321.3213 } }
            << std::vector<int>                    { -1, -1, -1 };

    QTest::newRow("Test oversampling empty quality values")

            << std::vector<std::vector<double>>  { { 6.2,  5.2,  5.2 }, {  6.2,  5.2,  5.2 } , {  6.2,  5.2,  5.2 } }
            << std::vector<std::vector<double>>  { { -23.12,  4.232,  -321.3213 }, {  -23.12,  4.232,  -321.3213 } , {  -23.12,  4.232,  -321.3213 } }
            << std::vector<double>                 {  }
            << std::vector<double>                 {  }
            << std::vector<std::vector<double>>    {  }
            << std::vector<int>                    {  };

    QTest::newRow("Test empty pipe line")

            << std::vector<std::vector<double>>    {  }
            << std::vector<std::vector<double>>    { { 6.2,  5.2,  5.2 } }
            << std::vector<double>                 { 0.0, 0.3, -112 }
            << std::vector<double>                 { 666 }
            << std::vector<std::vector<double>>    {  }
            << std::vector<int>                    {  };

     QTest::newRow("Test diffrent line length")   // Displayed test name

            << std::vector<std::vector<double>>  { { 6.2,  5.2,  5.2, 444 ,234 , 17 } }
            << std::vector<std::vector<double>>  { { -321.3213 } }
            << std::vector<double>                 { 0.5 }
            << std::vector<double>                 { 0.4 }
            << std::vector<std::vector<double>>  { { 6.2,  5.2,  5.2, 444 ,234 , 17 } }
            << std::vector<int>                    { 1 };

}

void TestConditionalLine::testProceed()
{
    precitec::filter::ConditionalLine filter;

    // In-Pipe "data_a"
    fliplib::NullSourceFilter sourceFilterLineA;
    fliplib::SynchronePipe< precitec::interface::GeoVecDoublearray >  pipeDA{ &sourceFilterLineA, "line_a"};
    pipeDA.setTag("line_a");
    QVERIFY(filter.connectPipe(&pipeDA, 1));

    // In-Pipe "data_b"
    fliplib::NullSourceFilter sourceFilterLineB;
    fliplib::SynchronePipe< precitec::interface::GeoVecDoublearray >  pipeDB{ &sourceFilterLineB, "line_b"};
    pipeDB.setTag("line_b");
    QVERIFY(filter.connectPipe(&pipeDB, 1));

    // In-Pipe "quality_a"
    fliplib::NullSourceFilter sourceFilterQualA;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeQA{ &sourceFilterQualA, "quality_a"};
    pipeQA.setTag("quality_a");
    QVERIFY(filter.connectPipe(&pipeQA, 1));

    // In-Pipe "quality_b"
    fliplib::NullSourceFilter sourceFilterQualB;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeQB{ &sourceFilterQualB, "quality_b"};
    pipeQB.setTag("quality_b");
    QVERIFY(filter.connectPipe(&pipeQB, 1));

    // Create Out-Pipes and connect to Conditional filter
    // --------------------------------------------------

    DummyFilter filterOutData;
    QVERIFY(filterOutData.connectPipe(filter.findPipe("LineOut"), 1));
    QVERIFY(filterOutData.connectPipe(filter.findPipe("OperationResult_out"), 1));

    // dummy data

    int imageNumber  =   0;
    int position     = 300;

    precitec::interface::ImageContext context;
    context.setImageNumber(imageNumber);
    context.setPosition(position);

    // parse test data

    QFETCH(std::vector<std::vector<double>> , param_line_a);
    QFETCH(std::vector<std::vector<double>> , param_line_b);
    QFETCH(std::vector<double>              , param_quality_a);
    QFETCH(std::vector<double>              , param_quality_b);
    QFETCH(std::vector<std::vector<double>> , expectedLines);
    QFETCH(std::vector<int>                 , expectedResult);

    // convert test array into appropriate data structure
    precitec::geo2d::VecDoublearray	lineA(param_line_a.size());
    for (size_t i = 0; i < param_line_a.size(); i++)
    {
        lineA[i].getData().insert(lineA[i].getData().begin(), param_line_a[i].begin(), param_line_a[i].end());
        lineA[i].getRank().assign(lineA[i].getData().size(), precitec::filter::eRankMax);
    }

    precitec::geo2d::VecDoublearray	lineB(param_line_b.size());
    for (size_t i = 0; i < param_line_b.size(); i++)
    {
        lineB[i].getData().insert(lineB[i].getData().begin(), param_line_b[i].begin(), param_line_b[i].end());
        lineB[i].getRank().assign(lineB[i].getData().size(), precitec::filter::eRankMax);
    }

    // signal pipes
    auto inPipe = precitec::interface::GeoVecDoublearray {
        context,
        lineA,
        precitec::interface::ResultType::AnalysisOK,
        precitec::interface::Limit
    };
    inPipe.rank(255);
    pipeDA.signal(inPipe);

    inPipe = precitec::interface::GeoVecDoublearray {
        context,
        lineB,
        precitec::interface::ResultType::AnalysisOK,
        precitec::interface::Limit
    };
    inPipe.rank(255);
    pipeDB.signal(inPipe);

    Doublearray qada;
    qada.getData().insert( qada.getData().begin(), param_quality_a.begin(), param_quality_a.end());
    qada.getRank().assign(qada.getData().size(), precitec::filter::eRankMax);

    auto qinPipe = precitec::interface::GeoDoublearray {
        context,
        qada,
        precitec::interface::ResultType::AnalysisOK,
        precitec::interface::Limit
    };
    pipeQA.signal(qinPipe);

    Doublearray qbda;
    qbda.getData().insert( qbda.getData().begin(), param_quality_b.begin(), param_quality_b.end());
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
    auto outPipeData = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoVecDoublearray>*>(filter.findPipe("LineOut"));
    QVERIFY(outPipeData);
    QCOMPARE(outPipeData->read( context.imageNumber()).ref().size() , expectedLines.size() );

    for (size_t i= 0; i < outPipeData->read(context.imageNumber()).ref().size() && i < expectedLines.size() ; i++) {
        QCOMPARE(outPipeData->read(context.imageNumber()).ref().at(i).getData(), expectedLines.at(i));
        QCOMPARE(outPipeData->read(context.imageNumber()).ref()[i].getData().size() , expectedLines[i].size() );
    }

    // check if compare result is correct
    auto outPipeCompResult = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("OperationResult_out"));
    QVERIFY(outPipeCompResult);
    QCOMPARE(outPipeCompResult->read(context.imageNumber()).ref().size(), expectedResult.size());

    for (size_t i= 0; i < outPipeCompResult->read(context.imageNumber()).ref().size() && i < expectedResult.size(); i++) {
         QCOMPARE(outPipeCompResult->read(context.imageNumber()).ref().getData()[i], expectedResult[i]);
    }
}

QTEST_GUILESS_MAIN(TestConditionalLine)

#include "testConditionalLine.moc"
