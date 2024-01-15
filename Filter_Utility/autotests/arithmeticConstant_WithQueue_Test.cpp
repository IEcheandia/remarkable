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


class ArithmeticConstantTest_WithQueue : public QObject
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


void ArithmeticConstantTest_WithQueue::testCtor()
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



void ArithmeticConstantTest_WithQueue::testArm()
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


void ArithmeticConstantTest_WithQueue::testProceed_data()
{
    QTest::addColumn<int>                 ("op");
    QTest::addColumn<std::vector<double>> ("value_data_a");
    QTest::addColumn<std::vector<int>>    ("rank_data_a");        // 0 .. 255
    QTest::addColumn<std::vector<double>> ("value_const");
    QTest::addColumn<std::vector<double>> ("expectedResult");
    QTest::addColumn<std::vector<int>>    ("expectedRank");       // 0 .. 255
    QTest::addColumn<std::vector<int>>    ("expectedQueueSize");  // m_oWindow = internal queue

    int param_op = (int) ArithmeticConstant::Operation::eMaxInWindow;
    QTest::newRow("MaxInWindow")      <<  param_op  // Name  << op
                                          // value_data_a
                                      <<  std::vector<double>  { -5.0,  2.0, -1.0, -8.0,  1.0, -3.0, -5.0, -1.0, -4.0, -2.0, -3.0, -5.0 }
                                          // rank_data_a
                                      <<  std::vector<int>     {  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100 }
                                          // value_const
                                      <<  std::vector<double>  {  3.0,  3.0,  3.0,  3.0,  3.0,  3.0,  3.0,  3.0,  3.0,  3.0,  3.0,  3.0 }
                                          // expectedResult
                                      <<  std::vector<double>  { -5.0,  2.0,  2.0,  2.0,  2.0,  1.0,  1.0,  1.0, -1.0, -1.0, -1.0, -2.0 }
                                          // expectedRank
                                      <<  std::vector<int>     {  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100 }
                                          // expectedQueueSize
                                      <<  std::vector<int>     {    1,    2,    3,    3,    3,    3,    3,    3,    3,    3,    3,    3 };

    param_op = (int) ArithmeticConstant::Operation::eMinInWindow;
    QTest::newRow("MinInWindow")      <<  param_op  // Name  << op
                                          // value_data_a
                                      <<  std::vector<double>  { -5.0,  2.0, -1.0, -8.0,  1.0, -3.0, -5.0, -1.0, -4.0, -2.0, -3.0, -5.0 }
                                          // rank_data_a
                                      <<  std::vector<int>     {  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100 }
                                          // value_const
                                      <<  std::vector<double>  {  3.0,  3.0,  3.0,  3.0,  3.0,  3.0,  3.0,  3.0,  3.0,  3.0,  3.0,  3.0 }
                                          // expectedResult
                                      <<  std::vector<double>  { -5.0, -5.0, -5.0, -8.0, -8.0, -8.0, -8.0, -5.0, -5.0, -5.0, -4.0, -5.0 }
                                          // expectedRank
                                      <<  std::vector<int>     {  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100 }
                                          // expectedQueueSize
                                      <<  std::vector<int>     {    1,    2,    3,    3,    3,    3,    3,    3,    3,    3,    3,    3 };
                                      
    param_op = (int) ArithmeticConstant::Operation::eAverageInWindow;
    QTest::newRow("AverageInWindow")  <<  param_op  // Name  << op
                                          // value_data_a
                                      <<  std::vector<double>  { -2.0,  4.0, -8.0,  2.0,  6.0, -4.0, -8.0, -2.0,  6.0,  0.0, -4.0, -6.0 }
                                          // rank_data_a
                                      <<  std::vector<int>     {  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100 }
                                          // value_const
                                      <<  std::vector<double>  {  3.0,  3.0,  3.0,  3.0,  3.0,  3.0,  3.0,  3.0,  3.0,  3.0,  3.0,  3.0 }
                                          // expectedResult
                                      <<  std::vector<double>  { -2.0,  1.0, -2.0, -1.0,  1.0, -1.0, -1.0, -2.0, -2.0, -1.0,  0.0, -1.0 }
                                          // expectedRank
                                      <<  std::vector<int>     {  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100 }
                                          // expectedQueueSize
                                      <<  std::vector<int>     {    1,    2,    3,    3,    3,    3,    3,    3,    3,    3,    3,    3 };
                                      
}


void ArithmeticConstantTest_WithQueue::testProceed()
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

    QFETCH(std::vector<double>, value_data_a);
    QFETCH(std::vector<int>,    rank_data_a);
    
    QFETCH(std::vector<double>, value_const);

    QFETCH(std::vector<double>, expectedResult);
    QFETCH(std::vector<int>,    expectedRank);
    QFETCH(std::vector<int>,    expectedQueueSize);

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
    
    testFilter.arm(precitec::filter::ArmState::eSeamStart);
        
    for (unsigned int iImageCounter = 0; iImageCounter < value_data_a.size(); ++ imageNumber, ++iImageCounter)
    {   
        testFilter.getParameters().update(std::string("Value"), fliplib::Parameter::TYPE_double, value_const[imageNumber]);
        testFilter.setParameter();
        
        // Set new values for the pipes
        // ----------------------------
            
#if TEXT_OUT
        std::cout << "In pipe 'data_a': " << value_data_a[imageNumber] << "  " << rank_data_a[imageNumber] << std::endl;
        std::cout << "         const:   " << value_const[imageNumber]  << "  imgNo "  << imageNumber << std::endl;
#endif
        auto inPipe = precitec::interface::GeoDoublearray {
            context, 
            Doublearray{1, value_data_a[imageNumber], rank_data_a[imageNumber]},
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
        std::cout << "Output pipe 'Data': " << resFilterDataOut.ref().getData()[0] << "  exp: " << expectedResult[imageNumber]    << std::endl;
        std::cout << "             Rank : " << resFilterDataOut.ref().getRank()[0] << "  exp: " << expectedRank[imageNumber]      << std::endl;
        std::cout << "       Queue size : " << testFilter.m_oWindow.size()         << "  exp: " << expectedQueueSize[imageNumber] << std::endl;
#endif
    
        QCOMPARE(resFilterDataOut.ref().size(), 1);
        QCOMPARE(resFilterDataOut.ref().getData()[0], expectedResult[imageNumber]);
        QCOMPARE(resFilterDataOut.ref().getRank()[0], expectedRank[imageNumber]);
        QCOMPARE(testFilter.m_oWindow.size(), expectedQueueSize[imageNumber]);
    }
    
    // Filter must be triggered!
    QCOMPARE(filterOutData.isProceedCalled(),  true);
}


QTEST_GUILESS_MAIN(ArithmeticConstantTest_WithQueue)
#include "arithmeticConstant_WithQueue_Test.moc"
