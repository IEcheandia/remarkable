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
using precitec::geo2d::TArray;
using precitec::geo2d::Doublearray;

// If some printed output is required
#define  TEXT_OUT  0


class ArithmeticConstantTest_WithArray : public QObject
{
    Q_OBJECT
    
    typedef std::vector<std::pair<double,int>> dataArray_t;

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


void ArithmeticConstantTest_WithArray::testCtor()
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



void ArithmeticConstantTest_WithArray::testArm()
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


void ArithmeticConstantTest_WithArray::testProceed_data()
{
    QTest::addColumn<int>          ("op");
    QTest::addColumn<dataArray_t>  ("array_data_a");
    QTest::addColumn<double>       ("expectedResult");
    QTest::addColumn<int>          ("expectedRank");       // 0 .. 255
    QTest::addColumn<int>          ("expectedQueueSize");  // m_oWindow = internal queue

    int param_op = (int) ArithmeticConstant::Operation::eMaxElement;
    QTest::newRow("MaxElement 1")     <<  param_op  // Name  << op
                                          // array_data_a
                                      <<  dataArray_t          { { -1.5 , 100 } }
                                          // expResult      expRank      expQueueSize
                                      <<          -1.5  <<      100  <<       0 ;
    QTest::newRow("MaxElement 2")     <<  param_op  // Name  << op
                                          // array_data_a
                                      <<  dataArray_t          { {  2.1 , 101 } , {  5.2 , 102 } , {  3.3 , 103 } }
                                          // expResult      expRank      expQueueSize
                                      <<           5.2  <<      102  <<       0 ;
    QTest::newRow("MaxElement 3")     <<  param_op  // Name  << op
                                          // array_data_a
                                      <<  dataArray_t          { { -5.1 , 101 } , { -2.2 , 102 } , { -8.3 , 103 } }
                                          // expResult      expRank      expQueueSize
                                      <<          -2.2  <<      102  <<       0 ;

    param_op = (int) ArithmeticConstant::Operation::eMinElement;
    QTest::newRow("MinElement 1")     <<  param_op  // Name  << op
                                          // array_data_a
                                      <<  dataArray_t          { { -1.5 , 100 } }
                                          // expResult      expRank      expQueueSize
                                      <<          -1.5  <<      100  <<       0 ;
    QTest::newRow("MinElement 2")     <<  param_op  // Name  << op
                                          // array_data_a
                                      <<  dataArray_t          { {  2.1 , 101 } , {  5.2 , 102 } , {  3.3 , 103 } }
                                          // expResult      expRank      expQueueSize
                                      <<           2.1  <<      101  <<       0 ;
    QTest::newRow("MinElement 3")     <<  param_op  // Name  << op
                                          // array_data_a
                                      <<  dataArray_t          { { -5.1 , 101 } , { -2.2 , 102 } , { -8.3 , 103 } }
                                          // expResult      expRank      expQueueSize
                                      <<          -8.3  <<      103  <<       0 ;
                                      
}


void ArithmeticConstantTest_WithArray::testProceed()
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

    QFETCH(dataArray_t, array_data_a);
    
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
	
	Doublearray dInArray(array_data_a.size());
	for (unsigned int i = 0; i < array_data_a.size(); i++)
	{
		dInArray[i] = array_data_a[i];
	}
	
	// Filter not yet triggered!
	QCOMPARE(filterOutData.isProceedCalled(),  false);
	
	testFilter.arm(precitec::filter::ArmState::eSeamStart);
		
#if TEXT_OUT
	std::cout << "In pipe 'data_a': ";
	for (unsigned int i = 0; i < array_data_a.size(); i++)
	{
		std::cout << array_data_a[i].first << "  " << array_data_a[i].second << "  ";
	}
	std::cout << std::endl;
#endif
	
	auto inPipe = precitec::interface::GeoDoublearray {
		context, 
		dInArray,
		precitec::interface::ResultType::AnalysisOK, 
		precitec::interface::Limit 
	};
	pipe.signal(inPipe);
	
	// Filter must be triggered!
	QCOMPARE(filterOutData.isProceedCalled(),  true);
	
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
}


QTEST_GUILESS_MAIN(ArithmeticConstantTest_WithArray)
#include "arithmeticConstant_WithArray_Test.moc"
