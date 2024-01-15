// qt includes
#include <QTest>
// project includes
#include "../arithmeticConstant.h"
#include <fliplib/NullSourceFilter.h>
#include <fliplib/SinkFilter.h>
#include <filter/sensorFilterInterface.h>
#include <filter/armStates.h>
#include <util/calibDataSingleton.h>

using precitec::filter::ArithmeticConstant;
using precitec::geo2d::Doublearray;

// If some printed output is required
#define  TEXT_OUT  0


class ArithmeticConstantTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    
    void testCtor();
    void testArm();
    void testProceed_data();
    void testProceed();
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


void ArithmeticConstantTest::testCtor()
{
    ArithmeticConstant filter;
    QCOMPARE(filter.name(), std::string("ArithmeticConstant"));

    // Outputs of the filter
    QVERIFY(filter.findPipe("DataOut") != nullptr);
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);

    // Internal parameters of the filter
    QVERIFY(filter.getParameters().exists(std::string("Operation")));
    QCOMPARE(filter.getParameters().findParameter(std::string("Operation")).getType(), fliplib::Parameter::TYPE_int);
    // Default operation = Addition
    int op = (int) ArithmeticConstant::Operation::eAddition;
    QCOMPARE(filter.getParameters().findParameter(std::string("Operation")).getValue().convert<int>(), op );
    
    QVERIFY(filter.getParameters().exists(std::string("Value")));
    QCOMPARE(filter.getParameters().findParameter(std::string("Value")).getType(), fliplib::Parameter::TYPE_double);
    QCOMPARE(filter.getParameters().findParameter(std::string("Value")).getValue().convert<double>(), 0.0 );
}



void ArithmeticConstantTest::testArm()
{
    ArithmeticConstant filter;

    // Set operation to "eAverageInWindow". That will clear the internal variable "m_oWindow"
    int op = (int) ArithmeticConstant::Operation::eAverageInWindow;
    filter.getParameters().update(std::string("Operation"), fliplib::Parameter::TYPE_int, op);  
    filter.getParameters().update(std::string("Value"), fliplib::Parameter::TYPE_double, 0.0);
    
    filter.setParameter();

    fliplib::ArmStateBase state( precitec::filter::ArmState::eSeamStart );
    QVERIFY( filter.m_oWindow.empty() );   

}


void ArithmeticConstantTest::testProceed_data()
{
    QTest::addColumn<int>    ("op");
    QTest::addColumn<double> ("value_data_a");
    QTest::addColumn<int>    ("rank_data_a");        // 0 .. 255
    QTest::addColumn<double> ("value_const");
    QTest::addColumn<double> ("expectedResult");
    QTest::addColumn<int>    ("expectedRank");       // 0 .. 255
    QTest::addColumn<int>    ("expectedQueueSize");  // m_oWindow = internal queue

    int param_op = (int) ArithmeticConstant::Operation::eAddition;
    //             Name                         op     data_a     rank_a     const     expRes     expRank    expQueueSize
    QTest::newRow("Addition 1")       <<  param_op  <<    8.0  <<    101  <<   3.5  <<   11.5  <<     101 <<            0 ;
    QTest::newRow("Addition 2")       <<  param_op  <<   -8.0  <<    102  <<  -3.5  <<  -11.5  <<     102 <<            0 ;
    QTest::newRow("Addition 3")       <<  param_op  <<    8.3  <<    103  <<  -8.3  <<    0.0  <<     103 <<            0 ;

    param_op = (int) ArithmeticConstant::Operation::eSubtraction;
    QTest::newRow("Subtraction 1")    <<  param_op  <<    8.0  <<    104  <<   3.5  <<    4.5  <<     104 <<            0 ;
    QTest::newRow("Subtraction 2")    <<  param_op  <<   -8.0  <<    105  <<  -3.5  <<   -4.5  <<     105 <<            0 ;
    QTest::newRow("Subtraction 3")    <<  param_op  <<    8.3  <<    106  <<   8.3  <<    0.0  <<     106 <<            0 ;

    param_op = (int) ArithmeticConstant::Operation::eMultiplication;
    QTest::newRow("Multiplication 1") <<  param_op  <<    8.0  <<    107  <<   3.5  <<   28.0  <<     107 <<            0 ;
    QTest::newRow("Multiplication 2") <<  param_op  <<   -8.0  <<    108  <<  -3.5  <<   28.0  <<     108 <<            0 ;
    QTest::newRow("Multiplication 3") <<  param_op  <<    8.0  <<    109  <<   0.0  <<    0.0  <<     109 <<            0 ;
    
    param_op = (int) ArithmeticConstant::Operation::eDivision;
    QTest::newRow("Division 1")       <<  param_op  <<  100.0  <<    110  <<   2.5  <<   40.0  <<     110 <<            0 ;
    QTest::newRow("Division 2")       <<  param_op  << -100.0  <<    111  <<   2.5  <<  -40.0  <<     111 <<            0 ;
    QTest::newRow("Division 3")       <<  param_op  <<    0.0  <<    112  <<   2.5  <<    0.0  <<     112 <<            0 ;
    QTest::newRow("Division 4")       <<  param_op  <<  100.0  <<    113  <<   0.0  <<    0.0  <<     113 <<            0 ;

    param_op = (int) ArithmeticConstant::Operation::eModulo;
    QTest::newRow("Modulo 1")         <<  param_op  <<   20.0  <<    100  <<   7.0  <<    6.0  <<     100 <<            0 ;
    QTest::newRow("Modulo 2")         <<  param_op  <<  -20.0  <<    100  <<   7.0  <<   -6.0  <<     100 <<            0 ;
    QTest::newRow("Modulo 3")         <<  param_op  <<   20.0  <<    100  <<  -7.0  <<    6.0  <<     100 <<            0 ;
    QTest::newRow("Modulo 4")         <<  param_op  <<  -20.0  <<    100  <<  -7.0  <<   -6.0  <<     100 <<            0 ;

    param_op = (int) ArithmeticConstant::Operation::eMaximum;
    QTest::newRow("Maximum 1")        <<  param_op  <<   20.0  <<    100  <<   7.0  <<   20.0  <<     100 <<            0 ;
    QTest::newRow("Maximum 2")        <<  param_op  <<  -20.0  <<    100  <<   0.7  <<    0.7  <<     100 <<            0 ;
    QTest::newRow("Maximum 3")        <<  param_op  <<   -0.2  <<    100  <<  -7.0  <<   -0.2  <<     100 <<            0 ;
    QTest::newRow("Maximum 4")        <<  param_op  <<    0.6  <<    100  <<   0.6  <<    0.6  <<     100 <<            0 ;

    param_op = (int) ArithmeticConstant::Operation::eMinimum;
    QTest::newRow("Minimum 1")        <<  param_op  <<   20.0  <<    100  <<   7.0  <<    7.0  <<     100 <<            0 ;
    QTest::newRow("Minimum 2")        <<  param_op  <<  -20.0  <<    100  <<   0.7  <<  -20.0  <<     100 <<            0 ;
    QTest::newRow("Minimum 3")        <<  param_op  <<   -0.2  <<    100  <<  -7.0  <<   -7.0  <<     100 <<            0 ;
    QTest::newRow("Minimum 4")        <<  param_op  <<    0.6  <<    100  <<   0.6  <<    0.6  <<     100 <<            0 ;

    param_op = (int) ArithmeticConstant::Operation::eReaches;
    QTest::newRow("Reaches 1")        <<  param_op  <<   20.0  <<    100  <<   7.0  <<    1.0  <<     255 <<            0 ;
    QTest::newRow("Reaches 2")        <<  param_op  <<  -20.0  <<    100  <<   0.7  <<    0.0  <<     255 <<            0 ;
    QTest::newRow("Reaches 3")        <<  param_op  <<   -0.2  <<    100  <<  -7.0  <<    1.0  <<     255 <<            0 ;
    QTest::newRow("Reaches 4")        <<  param_op  <<    0.6  <<    100  <<   0.6  <<    1.0  <<     255 <<            0 ;

    param_op = (int) ArithmeticConstant::Operation::eReachesNot;
    QTest::newRow("ReachesNot 1")     <<  param_op  <<   20.0  <<    100  <<  27.0  <<    1.0  <<     255 <<            0 ;
    QTest::newRow("ReachesNot 2")     <<  param_op  <<  -20.0  <<    100  <<   0.7  <<    1.0  <<     255 <<            0 ;
    QTest::newRow("ReachesNot 3")     <<  param_op  <<   -0.2  <<    100  <<  -7.0  <<    0.0  <<     255 <<            0 ;
    QTest::newRow("ReachesNot 4")     <<  param_op  <<    0.6  <<    100  <<   0.6  <<    0.0  <<     255 <<            0 ;

    param_op = (int) ArithmeticConstant::Operation::eExponentialFunction;
    QTest::newRow("Exponential 1")    <<  param_op  <<    0.0  <<    100  <<   3.5  <<    1.0  <<     100 <<            0 ;

    param_op = (int) ArithmeticConstant::Operation::eTruncate;
    QTest::newRow("Truncate 1")       <<  param_op  <<  20.054 <<    100  <<   2.0  <<   20.05  <<    100 <<            0 ;
    QTest::newRow("Truncate 2")       <<  param_op  << -20.054 <<    100  <<   2.0  <<  -20.05  <<    100 <<            0 ;
    QTest::newRow("Truncate 3")       <<  param_op  <<  -0.5   <<    100  <<   0.0  <<   -0.0   <<    100 <<            0 ;
    QTest::newRow("Truncate 4")       <<  param_op  <<   0.5   <<    100  <<   3.0  <<    0.5   <<    100 <<            0 ;
    QTest::newRow("Truncate 5")       <<  param_op  <<   0.5   <<    100  <<  -3.0  <<    0.5   <<    100 <<            0 ;
    QTest::newRow("Truncate 6")       <<  param_op  <<   0.26  <<    100  <<   1.1  <<    0.2   <<    100 <<            0 ;

    param_op = (int) ArithmeticConstant::Operation::eRound;
    QTest::newRow("Round 1")          <<  param_op  <<  20.054 <<    100  <<   2.0  <<   20.05  <<    100 <<            0 ;
    QTest::newRow("Round 2")          <<  param_op  << -20.054 <<    100  <<   2.0  <<  -20.05  <<    100 <<            0 ;
    QTest::newRow("Round 3")          <<  param_op  <<  -0.5   <<    100  <<   0.0  <<   -1.0   <<    100 <<            0 ;
    QTest::newRow("Round 4")          <<  param_op  <<   0.5   <<    100  <<   3.0  <<    0.5   <<    100 <<            0 ;
    QTest::newRow("Round 5")          <<  param_op  <<   0.5   <<    100  <<  -3.0  <<    0.5   <<    100 <<            0 ;
    QTest::newRow("Round 6")          <<  param_op  <<   0.26  <<    100  <<   1.1  <<    0.3   <<    100 <<            0 ;
}


void ArithmeticConstantTest::testProceed()
{
    // prepare filter graph
    // --------------------
    
    // a null source filter, connected with ArithmeticConstant filter, connected with DummyOutFilter
    
    // create arithmetic filter
    ArithmeticConstant testFilter;
    
    // Create In-Pipes and connect to ArithmeticConstant filter
    // --------------------------------------------------------
    
    // group = 0  =>  signal pipe as soon as data for a pipe are available
    // group = 1  =>  signal pipes only when all pipes have data
    int group = 0;

    fliplib::NullSourceFilter sourceFilter;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipe{ &sourceFilter, precitec::interface::SensorFilterInterface::SENSOR_SAMPLE_FRAME_PIPE };
    QVERIFY(testFilter.connectPipe(&pipe, group));
    
    // Create Out-Pipes and connect to ArithmeticConstant filter
    // ---------------------------------------------------------
    
    DummyOutFilter filterOutData;
    QVERIFY(filterOutData.connectPipe(testFilter.findPipe("DataOut"), 0));

    // Set parameters for the In-Pipes and the filter
    // ----------------------------------------------
    
    QFETCH(int, op);
    testFilter.getParameters().update(std::string("Operation"), fliplib::Parameter::TYPE_int, op);

    QFETCH(double, value_data_a);
    QFETCH(int,    rank_data_a);
    
    QFETCH(double, value_const);
    testFilter.getParameters().update(std::string("Value"), fliplib::Parameter::TYPE_double, value_const);

    QFETCH(double, expectedResult);
    QFETCH(int,    expectedRank);
    QFETCH(int,    expectedQueueSize);

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
    std::cout << "         const:   " << value_const  << std::endl;
#endif
    auto inPipe = precitec::interface::GeoDoublearray {
        context, 
        Doublearray{1, value_data_a, rank_data_a},
        precitec::interface::ResultType::AnalysisOK, 
        precitec::interface::Limit 
    };
    pipe.signal(inPipe);
        
    // access the out pipes data
    // -------------------------
 
    auto outPipe = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(testFilter.findPipe("DataOut"));
    QVERIFY(outPipe);
    // the result is a GeoDoublearray with one element
    precitec::interface::GeoDoublearray resDataOut;
    auto resFilterDataOut = outPipe->read(resDataOut.context().imageNumber());
#if TEXT_OUT
    std::cout << "Output pipe 'Data': " << resFilterDataOut.ref().getData()[0] << "  exp: " << expectedResult    << std::endl;
    std::cout << "             Rank : " << resFilterDataOut.ref().getRank()[0] << "  exp: " << expectedRank      << std::endl;
    std::cout << "       Queue size : " << testFilter.m_oWindow.size()         << "  exp: " << expectedQueueSize << std::endl;
#endif
    
    QCOMPARE(resFilterDataOut.ref().size(), 1);
    QCOMPARE(resFilterDataOut.ref().getData()[0], expectedResult);
    QCOMPARE(resFilterDataOut.ref().getRank()[0], expectedRank);
    QCOMPARE(testFilter.m_oWindow.size(), expectedQueueSize);
    
    // Filter must be triggered!
    QCOMPARE(filterOutData.isProceedCalled(),  true);
}


QTEST_GUILESS_MAIN(ArithmeticConstantTest)
#include "arithmeticConstantTest.moc"
