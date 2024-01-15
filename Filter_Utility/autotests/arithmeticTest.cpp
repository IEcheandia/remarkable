#include <QTest>

#include "../arithmetic.h"

#include <fliplib/NullSourceFilter.h>
#include <fliplib/SinkFilter.h>
#include <filter/sensorFilterInterface.h>

using precitec::filter::Arithmetic;
using precitec::geo2d::Doublearray;

Q_DECLARE_METATYPE(Doublearray);

Doublearray doubleArray(std::vector<double> data, std::vector<int> rank = {})
{
    if (rank.empty())
    {
        rank.assign(data.size(), 255);
    }
    Q_ASSERT(data.size() == rank.size());
    Doublearray result;
    result.getData() = std::move(data);
    result.getRank() = std::move(rank);
    return result;
}

// If some printed output is required
#define  TEXT_OUT  0


class ArithmeticTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    
    void testCtor();
    void testProceed_data();
    void testProceed();
    void testMerge_data();
    void testMerge();
};


// Dummy-Filter for the output
class DummyOutFilter : public fliplib::BaseFilter
{
public:
    DummyOutFilter() : fliplib::BaseFilter("dummy") {}
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

private:
    bool m_proceedCalled = false;
};


void ArithmeticTest::testCtor()
{
    Arithmetic testFilter;
    QCOMPARE(testFilter.name(), std::string("Arithmetic"));

    // Outputs of the filter
    QVERIFY(testFilter.findPipe("Data") != nullptr);
    QVERIFY(testFilter.findPipe("NotAValidPipe") == nullptr);

    // Internal parameters of the filter
    QVERIFY(testFilter.getParameters().exists(std::string("Operation")));
    QCOMPARE(testFilter.getParameters().findParameter(std::string("Operation")).getType(), fliplib::Parameter::TYPE_int);
    QCOMPARE(testFilter.getParameters().findParameter(std::string("Operation")).getValue().convert<int>(), 0);
}


void ArithmeticTest::testProceed_data()
{
    // Achtung!   data_a = tag_b
    //            data_b = tag_a
    
    QTest::addColumn<int>    ("op");
    QTest::addColumn<double> ("value_data_a");     // tag_b !!
    QTest::addColumn<int>    ("rank_data_a");      // tag_b !!  0 .. 255
    QTest::addColumn<double> ("value_data_b");     // tag_a !!
    QTest::addColumn<int>    ("rank_data_b");      // tag_a !!  0 .. 255
    QTest::addColumn<double> ("expectedResult");
    QTest::addColumn<int>    ("expectedRank");     // 0 .. 255

    //             Name                   op     data_a     rank_a     data_b     rank_b     expRes     expRank
    QTest::newRow("Addition 1")       <<   0  <<    8.0  <<    200  <<    3.5  <<    100  <<   11.5  <<     100 ;
    QTest::newRow("Addition 2")       <<   0  <<   -8.0  <<    100  <<   -3.5  <<    200  <<  -11.5  <<     100 ;
    QTest::newRow("Addition 3")       <<   0  <<    8.3  <<    100  <<   -8.3  <<    100  <<    0.0  <<     100 ;

    QTest::newRow("Subtraction 1")    <<   1  <<    8.0  <<    200  <<    3.5  <<    100  <<    4.5  <<     100 ;
    QTest::newRow("Subtraction 2")    <<   1  <<   -8.0  <<    100  <<   -3.5  <<    200  <<   -4.5  <<     100 ;
    QTest::newRow("Subtraction 3")    <<   1  <<    8.3  <<    100  <<    8.3  <<    100  <<    0.0  <<     100 ;

    QTest::newRow("Multiplication 1") <<   2  <<    8.0  <<    200  <<    3.5  <<    100  <<   28.0  <<     100 ;
    QTest::newRow("Multiplication 2") <<   2  <<   -8.0  <<    100  <<   -3.5  <<    200  <<   28.0  <<     100 ;
    QTest::newRow("Multiplication 3") <<   2  <<    8.0  <<    100  <<    0.0  <<    100  <<    0.0  <<     100 ;
    
    QTest::newRow("Division 1")       <<   3  <<  100.0  <<    200  <<    2.5  <<    100  <<   40.0  <<     100 ;
    QTest::newRow("Division 2")       <<   3  << -100.0  <<    100  <<    2.5  <<    200  <<  -40.0  <<     100 ;
    QTest::newRow("Division 3")       <<   3  <<    0.0  <<    100  <<    2.5  <<    100  <<    0.0  <<     100 ;
    QTest::newRow("Division 4")       <<   3  <<  100.0  <<    100  <<    0.0  <<    100  <<    0.0  <<       0 ;

    QTest::newRow("Modulo 1")         <<   4  <<   20.0  <<    200  <<    7.0  <<    100  <<    6.0  <<     100 ;
    QTest::newRow("Modulo 2")         <<   4  <<  -20.0  <<    100  <<    7.0  <<    200  <<   -6.0  <<     100 ;
    QTest::newRow("Modulo 3")         <<   4  <<   20.0  <<    100  <<   -7.0  <<    100  <<    6.0  <<     100 ;
    QTest::newRow("Modulo 4")         <<   4  <<  -20.0  <<    100  <<   -7.0  <<    200  <<   -6.0  <<     100 ;

    QTest::newRow("Maximum 1")        <<   5  <<   20.0  <<    200  <<    7.0  <<    100  <<   20.0  <<     100 ;
    QTest::newRow("Maximum 2")        <<   5  <<  -20.0  <<    100  <<    0.7  <<    200  <<    0.7  <<     100 ;
    QTest::newRow("Maximum 3")        <<   5  <<   -0.2  <<    100  <<   -7.0  <<    100  <<   -0.2  <<     100 ;
    QTest::newRow("Maximum 4")        <<   5  <<    0.6  <<    200  <<    0.6  <<    100  <<    0.6  <<     100 ;

    QTest::newRow("Minimum 1")        <<   6  <<   20.0  <<    200  <<    7.0  <<    100  <<    7.0  <<     100 ;
    QTest::newRow("Minimum 2")        <<   6  <<  -20.0  <<    100  <<    0.7  <<    200  <<  -20.0  <<     100 ;
    QTest::newRow("Minimum 3")        <<   6  <<   -0.2  <<    100  <<   -7.0  <<    100  <<   -7.0  <<     100 ;
    QTest::newRow("Minimum 4")        <<   6  <<    0.6  <<    200  <<    0.6  <<    100  <<    0.6  <<     100 ;

    QTest::newRow("Set Rank 1")       <<   7  <<   11.0  <<    101  <<    0.7  <<    201  <<   11.0  <<       1 ;
    QTest::newRow("Set Rank 2")       <<   7  <<   12.0  <<    102  <<  255.3  <<    202  <<   12.0  <<     255 ;
    QTest::newRow("Set Rank 3")       <<   7  <<   13.0  <<    103  <<  255.7  <<    203  <<   13.0  <<     103 ;
    QTest::newRow("Set Rank 4")       <<   7  <<   14.0  <<    104  <<   -0.2  <<    204  <<   14.0  <<     104 ;

    QTest::newRow("log AND 1")        <<   8  <<  0.0000011 << 100  <<  0.0000011 << 200  <<    1.0  <<     255 ;
    QTest::newRow("log AND 2")        <<   8  <<  0.0000010 << 100  <<  0.0000011 << 200  <<    0.0  <<     255 ;
    QTest::newRow("log AND 3")        <<   8  <<  0.0000011 << 100  <<  0.0000010 << 200  <<    0.0  <<     255 ;
    QTest::newRow("log AND 4")        <<   8  <<  0.0000010 << 100  <<  0.0000010 << 200  <<    0.0  <<     255 ;
    QTest::newRow("log AND 5")        <<   8  << -0.0000011 << 100  << -0.0000011 << 200  <<    1.0  <<     255 ;
    QTest::newRow("log AND 6")        <<   8  << -0.0000010 << 100  << -0.0000011 << 200  <<    0.0  <<     255 ;
    QTest::newRow("log AND 7")        <<   8  << -0.0000011 << 100  << -0.0000010 << 200  <<    0.0  <<     255 ;
    QTest::newRow("log AND 8")        <<   8  << -0.0000010 << 100  << -0.0000010 << 200  <<    0.0  <<     255 ;

    QTest::newRow("log OR 1")         <<   9  <<  0.0000011 << 100  <<  0.0000011 << 200  <<    1.0  <<     255 ;
    QTest::newRow("log OR 2")         <<   9  <<  0.0000010 << 100  <<  0.0000011 << 200  <<    1.0  <<     255 ;
    QTest::newRow("log OR 3")         <<   9  <<  0.0000011 << 100  <<  0.0000010 << 200  <<    1.0  <<     255 ;
    QTest::newRow("log OR 4")         <<   9  <<  0.0000010 << 100  <<  0.0000010 << 200  <<    0.0  <<     255 ;
    QTest::newRow("log OR 5")         <<   9  << -0.0000011 << 100  << -0.0000011 << 200  <<    1.0  <<     255 ;
    QTest::newRow("log OR 6")         <<   9  << -0.0000010 << 100  << -0.0000011 << 200  <<    1.0  <<     255 ;
    QTest::newRow("log OR 7")         <<   9  << -0.0000011 << 100  << -0.0000010 << 200  <<    1.0  <<     255 ;
    QTest::newRow("log OR 8")         <<   9  << -0.0000010 << 100  << -0.0000010 << 200  <<    0.0  <<     255 ;

    QTest::newRow("a >= b 1")         <<  10  <<   10.0  <<    100  <<   10.0  <<    200  <<    1.0  <<     255 ;
    QTest::newRow("a >= b 2")         <<  10  <<   10.1  <<    100  <<   10.0  <<    200  <<    1.0  <<     255 ;
    QTest::newRow("a >= b 3")         <<  10  <<   10.0  <<    100  <<   10.1  <<    200  <<    0.0  <<     255 ;
    QTest::newRow("a >= b 4")         <<  10  <<  -10.0  <<    100  <<  -10.0  <<    200  <<    1.0  <<     255 ;
    QTest::newRow("a >= b 5")         <<  10  <<  -10.0  <<    100  <<  -10.1  <<    200  <<    1.0  <<     255 ;
    QTest::newRow("a >= b 6")         <<  10  <<  -10.1  <<    100  <<  -10.0  <<    200  <<    0.0  <<     255 ;

    QTest::newRow("b >= a 1")         <<  11  <<   10.0  <<    100  <<   10.0  <<    200  <<    1.0  <<     255 ;
    QTest::newRow("b >= a 2")         <<  11  <<   10.0  <<    100  <<   10.1  <<    200  <<    1.0  <<     255 ;
    QTest::newRow("b >= a 3")         <<  11  <<   10.1  <<    100  <<   10.0  <<    200  <<    0.0  <<     255 ;
    QTest::newRow("b >= a 4")         <<  11  <<  -10.0  <<    100  <<  -10.0  <<    200  <<    1.0  <<     255 ;
    QTest::newRow("b >= a 5")         <<  11  <<  -10.1  <<    100  <<  -10.0  <<    200  <<    1.0  <<     255 ;
    QTest::newRow("b >= a 6")         <<  11  <<  -10.0  <<    100  <<  -10.1  <<    200  <<    0.0  <<     255 ;

    QTest::newRow("EuclideanNorm")   <<  12  <<  -30.0  <<    255  <<   40.0  <<    255  <<    50.0  <<     255 ;
}


void ArithmeticTest::testProceed()
{
    // prepare filter graph
    // --------------------
    
    // a null source filter, connected with Arithmetic filter, connected with DummyOutFilter
    
    // Create Arithmetic filter
    Arithmetic testFilter;
    
    // Create In-Pipes and connect to Arithmetic filter
    // -------------------------------------------------
    
    // group = 0  =>  signal pipe as soon as data for a pipe are available
    // group = 1  =>  signal pipes only when all pipes have data
    int group = 1;

    // In-Pipe "data_a" with tag "data_b"
    fliplib::NullSourceFilter sourceFilterDataA;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeDA{ &sourceFilterDataA, "data_a"};
    pipeDA.setTag("data_b");
    QVERIFY(testFilter.connectPipe(&pipeDA, group));

    // In-Pipe "data_b" with tag "data_a"
    fliplib::NullSourceFilter sourceFilterDataB;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeDB{ &sourceFilterDataB, "data_b"};
    pipeDB.setTag("data_a");
    QVERIFY(testFilter.connectPipe(&pipeDB, group));
    
    // Create Out-Pipes and connect to Arithmetic filter
    // --------------------------------------------------
    
    DummyOutFilter filterOutData;
    QVERIFY(filterOutData.connectPipe(testFilter.findPipe("Data"), 0));
    
    // Set parameters for the In-Pipes and the filter
    // ----------------------------------------------
    
    QFETCH(int, op);
    testFilter.getParameters().update(std::string("Operation"), fliplib::Parameter::TYPE_int, op);

    QFETCH(double, value_data_a);
    QFETCH(int,    rank_data_a);
    QFETCH(double, value_data_b);
    QFETCH(int,    rank_data_b);
    QFETCH(double, expectedResult);
    QFETCH(int,    expectedRank);

#if TEXT_OUT
    testFilter.getParameters().update(std::string("Verbosity"), fliplib::Parameter::TYPE_int, 6);
#endif

    testFilter.setParameter();

    // signal the pipes
    // ----------------
    
    // Prepare dummy data for the filter's In-Pipes
    
    int imageNumber  =   0;
    int position     = 300;

    precitec::interface::ImageContext context;
    context.setImageNumber(imageNumber);
    context.setPosition(position);
    
    // Filter not yet triggered!
    QCOMPARE(filterOutData.isProceedCalled(),  false);

    // Set new values for the pipes
    // ----------------------------
        
#if TEXT_OUT
    std::cout << "In pipe 'data_a': " << value_data_a << "  " << rank_data_a << std::endl;
#endif
    auto inPipe = precitec::interface::GeoDoublearray {
        context, 
        Doublearray{1, value_data_a, rank_data_a},
        precitec::interface::ResultType::AnalysisOK, 
        precitec::interface::Limit 
    };
    pipeDA.signal(inPipe);
        
#if TEXT_OUT
    std::cout << "In pipe 'data_b': " << value_data_b << "  " << rank_data_b << std::endl;
    std::cout << "         intValB: " << (int) (value_data_b + 0.5) << std::endl;
#endif
    inPipe = precitec::interface::GeoDoublearray {
        context, 
        Doublearray{1, value_data_b, rank_data_b},
        precitec::interface::ResultType::AnalysisOK, 
        precitec::interface::Limit 
    };
    pipeDB.signal(inPipe);
    // Filter must be triggered!
    QCOMPARE(filterOutData.isProceedCalled(),  true);
        
    // access the out pipes data
    // -------------------------
 
    auto outPipeDataOut = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(testFilter.findPipe("Data"));
    QVERIFY(outPipeDataOut);
    // the result is a GeoDoublearray with one element
    precitec::interface::GeoDoublearray resDataOut;
    const auto resFilterDataOut = outPipeDataOut->read(resDataOut.context().imageNumber());
#if TEXT_OUT
    std::cout << "Output pipe 'Data': " << resFilterDataOut.ref().getData()[0] << "  exp: " << expectedResult << std::endl;
    std::cout << "             Rank : " << resFilterDataOut.ref().getRank()[0] << "  exp: " << expectedRank   << std::endl;
#endif
    
    QCOMPARE(resFilterDataOut.ref().size(), 1);
    QCOMPARE(resFilterDataOut.ref().getData()[0], expectedResult);
    QCOMPARE(resFilterDataOut.ref().getRank()[0], expectedRank);


}

void ArithmeticTest::testMerge_data()
{
    QTest::addColumn<Doublearray>("dataA");
    QTest::addColumn<Doublearray>("dataB");
    QTest::addColumn<Doublearray>("expectedResult");
    QTest::newRow("SingleValues") << doubleArray({1.0},{255}) << doubleArray({-0.5}, {0}) << doubleArray({1.0, -0.5}, {255, 0});
    QTest::newRow("LongerA") << doubleArray({1.0, 2.0, 3.0}) << doubleArray({-0.5}, {0}) << doubleArray({1.0, 2.0, 3.0, -0.5}, {255, 255, 255, 0});
    QTest::newRow("LongerB") << doubleArray({3.0}) << doubleArray({-1.0, -2.0 , -3.0}, {1, 2, 3}) << doubleArray({3.0, -1.0, -2.0, -3.0}, {255, 1, 2, 3});
    QTest::newRow("Merge") << doubleArray({1.2, 3.4}) << doubleArray({5.6, 7.8, 9.10}) << doubleArray({1.2, 3.4, 5.6, 7.8, 9.10});
    QTest::newRow("Invalid") << doubleArray({}) << doubleArray({}) << doubleArray({});
    QTest::newRow("InvalidA") << doubleArray({}) << doubleArray({0.0}) << doubleArray({0.0});
    QTest::newRow("InvalidB") << doubleArray({0.0}) << doubleArray({}) << doubleArray({0.0});
}

void ArithmeticTest::testMerge()
{
    // prepare filter graph
    // --------------------

    // a null source filter, connected with Arithmetic filter, connected with DummyOutFilter

    // Create Arithmetic filter
    Arithmetic testFilter;

    // Create In-Pipes and connect to Arithmetic filter
    // -------------------------------------------------

    // group = 0  =>  signal pipe as soon as data for a pipe are available
    // group = 1  =>  signal pipes only when all pipes have data
    int group = 1;

    // In-Pipe "data_a" with tag "data_b"
    fliplib::NullSourceFilter sourceFilterDataA;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeDA{ &sourceFilterDataA, "data_a"};
    pipeDA.setTag("data_b");
    QVERIFY(testFilter.connectPipe(&pipeDA, group));

    // In-Pipe "data_b" with tag "data_a"
    fliplib::NullSourceFilter sourceFilterDataB;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeDB{ &sourceFilterDataB, "data_b"};
    pipeDB.setTag("data_a");
    QVERIFY(testFilter.connectPipe(&pipeDB, group));

    // Create Out-Pipes and connect to Arithmetic filter
    // --------------------------------------------------

    DummyOutFilter filterOutData;
    QVERIFY(filterOutData.connectPipe(testFilter.findPipe("Data"), 0));

    // Set "Append" operation
    testFilter.getParameters().update(std::string("Operation"), fliplib::Parameter::TYPE_int, 13);

    QFETCH(Doublearray, dataA);
    QFETCH(Doublearray, dataB);
    QFETCH(Doublearray, expectedResult);

    testFilter.setParameter();

    // signal the pipes
    // ----------------

    // Prepare dummy data for the filter's In-Pipes

    int imageNumber  =   0;
    int position     = 300;

    precitec::interface::ImageContext context;
    context.setImageNumber(imageNumber);
    context.setPosition(position);

    // Filter not yet triggered!
    QCOMPARE(filterOutData.isProceedCalled(),  false);

    // Set new values for the pipes
    // ----------------------------

    auto inPipe = precitec::interface::GeoDoublearray {
        context,
        dataA,
        precitec::interface::ResultType::AnalysisOK,
        precitec::interface::Limit
    };
    pipeDA.signal(inPipe);


    inPipe = precitec::interface::GeoDoublearray {
        context,
        dataB,
        precitec::interface::ResultType::AnalysisOK,
        precitec::interface::Limit
    };
    pipeDB.signal(inPipe);
    // Filter must be triggered!
    QCOMPARE(filterOutData.isProceedCalled(),  true);

    // access the out pipes data
    // -------------------------

    auto outPipeDataOut = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(testFilter.findPipe("Data"));
    QVERIFY(outPipeDataOut);
    // the result is a GeoDoublearray with one element
    precitec::interface::GeoDoublearray resDataOut;
    const auto resFilterDataOut = outPipeDataOut->read(resDataOut.context().imageNumber());


    QCOMPARE(resFilterDataOut.ref().getData(), expectedResult.getData());
    QCOMPARE(resFilterDataOut.ref().getRank(), expectedResult.getRank());


}


QTEST_GUILESS_MAIN(ArithmeticTest)
#include "arithmeticTest.moc"
