#include <QTest>

#include "../conditional.h"

#include <fliplib/NullSourceFilter.h>
#include <fliplib/SinkFilter.h>
#include <filter/sensorFilterInterface.h>

using precitec::filter::Conditional;
using precitec::geo2d::Doublearray;

// If some printed output is required
#define  TEXT_OUT  0


class ConditionalTest : public QObject
{
    Q_OBJECT
    
private Q_SLOTS:
    void testCtor();
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


void ConditionalTest::testCtor()
{
    Conditional testFilter;
    QCOMPARE(testFilter.name(), std::string("Conditional"));
    
    // Outputs of the filter
    QVERIFY(testFilter.findPipe("Data_out") != nullptr);
    QVERIFY(testFilter.findPipe("OperationResult_out") != nullptr);
    QVERIFY(testFilter.findPipe("NotAValidPipe") == nullptr);
    
    // Internal parameters of the filter
    QVERIFY(testFilter.getParameters().exists(std::string("Operation")));
    QCOMPARE(testFilter.getParameters().findParameter(std::string("Operation")).getType(), fliplib::Parameter::TYPE_int);
    // Operation:  0 = "Compare Mode", 1 = "Encoder compare mode"
    QCOMPARE(testFilter.getParameters().findParameter(std::string("Operation")).getValue().convert<int>(), 0);
}


void ConditionalTest::testProceed_data()
{
    QTest::addColumn<int>   ("op");                // op

    QTest::addColumn< std::vector<double> > ("param_data_a");      // DA
    QTest::addColumn< std::vector<double> > ("param_data_b");      // DB
    QTest::addColumn< std::vector<double> > ("param_quality_a");   // QA
    QTest::addColumn< std::vector<double> > ("param_quality_b");   // QB
    QTest::addColumn< std::vector<double> > ("expectedData");      // expD
    QTest::addColumn< std::vector<double> > ("expectedResult");    // expR

    QTest::newRow("Compare")   // Displayed test name
            <<  0              // op
                               // img   1     2     3
            << std::vector<double>  {  5.2,  5.2,  5.2 }    // DA = data_a
            << std::vector<double>  {  2.1,  2.1,  2.1 }    // DB = data_b
            << std::vector<double>  {  0.5,  0.4,  0.5 }    // QA = quality_a
            << std::vector<double>  {  0.4,  0.5,  0.5 }    // QB = quality_b
            << std::vector<double>  {  5.2,  2.1,  5.2 }    // expD = expectedData
            << std::vector<double>  {  1.0, -1.0,  1.0 } ;  // expR = expectedResult

    QTest::newRow("Comp Null") // Displayed test name
            <<  0              // op
                               // img   1     2     3
            << std::vector<double>  {  0.0,  0.0,  0.0 }    // DA = data_a
            << std::vector<double>  {  0.0,  0.0,  0.0 }    // DB = data_b
            << std::vector<double>  {  0.0,  0.5,  0.3 }    // QA = quality_a
            << std::vector<double>  {  0.0,  0.3,  0.5 }    // QB = quality_b
            << std::vector<double>  {  0.0,  0.0,  0.0 }    // expD = expectedData
            << std::vector<double>  {  1.0,  1.0, -1.0 } ;  // expR = expectedResult

    QTest::newRow("Encoder 1") // Displayed test name
            <<  1              // op
                               // img   1     2     3
            << std::vector<double>  {  2.0,  3.0,  4.0 }    // DA = data_a
            << std::vector<double>  {  5.0,  6.0,  7.0 }    // DB = data_b
            << std::vector<double>  {  0.4,  0.5,  0.5 }    // QA = quality_a
            << std::vector<double>  {  0.5,  0.5,  0.4 }    // QB = quality_b
            << std::vector<double>  {  2.0,  2.0,  2.0 }    // expD = expectedData
            << std::vector<double>  {  1.0,  0.0,  0.0 } ;  // expR = expectedResult

    QTest::newRow("Encoder 2") // Displayed test name
            <<  1              // op
                               // img   1     2     3
            << std::vector<double>  {  2.0,  3.0,  4.0 }    // DA = data_a
            << std::vector<double>  {  8.0,  8.0,  8.0 }    // DB = data_b
            << std::vector<double>  {  0.4,  0.4,  0.5 }    // QA = quality_a
            << std::vector<double>  {  0.5,  0.5,  0.4 }    // QB = quality_b
            << std::vector<double>  {  2.0,  3.0,  3.0 }    // expD = expectedData
            << std::vector<double>  {  1.0,  1.0,  0.0 } ;  // expR = expectedResult
}


void ConditionalTest::testProceed()
{
    // prepare filter graph
    // --------------------
    
    // a null source filter, connected with Conditional filter, connected with DummyOutFilter
    
    // Create Conditional filter
    Conditional testFilter;
    
    // Create In-Pipes and connect to Conditional filter
    // -------------------------------------------------
    
    // group = 0  =>  signal pipe as soon as data for a pipe are available
    // group = 1  =>  signal pipes only when all pipes have data
    int group = 1;

    // In-Pipe "data_a"
    fliplib::NullSourceFilter sourceFilterDataA;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeDA{ &sourceFilterDataA, "data_a"};
    pipeDA.setTag("data_a");
    QVERIFY(testFilter.connectPipe(&pipeDA, group));

    // In-Pipe "data_b"
    fliplib::NullSourceFilter sourceFilterDataB;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeDB{ &sourceFilterDataB, "data_b"};
    pipeDB.setTag("data_b");
    QVERIFY(testFilter.connectPipe(&pipeDB, group));
    
    // In-Pipe "quality_a"
    fliplib::NullSourceFilter sourceFilterQualA;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeQA{ &sourceFilterQualA, "quality_a"};
    pipeQA.setTag("quality_a");
    QVERIFY(testFilter.connectPipe(&pipeQA, group));
    
    // In-Pipe "quality_b"
    fliplib::NullSourceFilter sourceFilterQualB;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeQB{ &sourceFilterQualB, "quality_b"};
    pipeQB.setTag("quality_b");
    QVERIFY(testFilter.connectPipe(&pipeQB, group));
    
    // Create Out-Pipes and connect to Conditional filter
    // --------------------------------------------------
    
    DummyOutFilter filterOutData;
    QVERIFY(filterOutData.connectPipe(testFilter.findPipe("Data_out"), 0));
    
    DummyOutFilter filterOutOpRes;
    QVERIFY(filterOutOpRes.connectPipe(testFilter.findPipe("OperationResult_out"), 0));

    // Set parameters for the In-Pipes and the filter
    // ----------------------------------------------
    
    QFETCH(int, op);
    testFilter.getParameters().update(std::string("Operation"), fliplib::Parameter::TYPE_int, op);

    QFETCH(std::vector<double>, param_data_a);
    QFETCH(std::vector<double>, param_data_b);
    QFETCH(std::vector<double>, param_quality_a);
    QFETCH(std::vector<double>, param_quality_b);
    QFETCH(std::vector<double>, expectedData);
    QFETCH(std::vector<double>, expectedResult);

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
    
    int    iDummyRank = 255;   // int 0..255
    
    // Filter not yet triggered!
    QCOMPARE(filterOutData.isProceedCalled(),  false);
    QCOMPARE(filterOutOpRes.isProceedCalled(), false);

    // Go through the given cycles
    for ( unsigned int iTriggerCycle = 1; iTriggerCycle <= 3; iTriggerCycle++)
    {
    
#if TEXT_OUT
        std::cout << "Cycle no. " << iTriggerCycle << std::endl;
#endif

        // Set new values for the pipes
        // ----------------------------
        
#if TEXT_OUT
        std::cout << "In pipe 'data_a': " << param_data_a[imageNumber] << "  " << iDummyRank << std::endl;
#endif
        auto inPipe = precitec::interface::GeoDoublearray {
            context, 
            Doublearray{1, param_data_a[imageNumber], iDummyRank},
            precitec::interface::ResultType::AnalysisOK, 
            precitec::interface::Limit 
        };
        pipeDA.signal(inPipe);
        
#if TEXT_OUT
        std::cout << "In pipe 'data_b': " << param_data_b[imageNumber] << "  " << iDummyRank << std::endl;
#endif
        inPipe = precitec::interface::GeoDoublearray {
            context, 
            Doublearray{1, param_data_b[imageNumber], iDummyRank},
            precitec::interface::ResultType::AnalysisOK, 
            precitec::interface::Limit 
        };
        pipeDB.signal(inPipe);
        
#if TEXT_OUT
        std::cout << "In pipe 'quality_a': " << param_quality_a[imageNumber] << "  " << iDummyRank << std::endl;
#endif
        inPipe = precitec::interface::GeoDoublearray {
            context, 
            Doublearray{1, param_quality_a[imageNumber], iDummyRank},
            precitec::interface::ResultType::AnalysisOK, 
            precitec::interface::Limit 
        };
        pipeQA.signal(inPipe);
        
#if TEXT_OUT
        std::cout << "In pipe 'quality_b': " << param_quality_b[imageNumber] << "  " << iDummyRank << std::endl;
#endif
        inPipe = precitec::interface::GeoDoublearray {
            context, 
            Doublearray{1, param_quality_b[imageNumber], iDummyRank},
            precitec::interface::ResultType::AnalysisOK, 
            precitec::interface::Limit 
        };
        pipeQB.signal(inPipe);
        
        // access the out pipes data
        // -------------------------

        auto outPipe = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(testFilter.findPipe("Data_out"));
        QVERIFY(outPipe);
        // the result is a GeoDoublearray with one element
        precitec::interface::GeoDoublearray resDataOut;
        const auto resFilterDataOut = outPipe->read(resDataOut.context().imageNumber());
#if TEXT_OUT
        std::cout << "Output pipe 'Data_out': " << resFilterDataOut.ref().getData()[0] << "  exp: " << expectedData[imageNumber] << std::endl;
#endif
    
        QCOMPARE(resFilterDataOut.ref().size(), 1);
        QCOMPARE(resFilterDataOut.ref().getData()[0], expectedData[imageNumber]);
        
        outPipe = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(testFilter.findPipe("OperationResult_out"));
        QVERIFY(outPipe);
        // the result is a GeoDoublearray with one element
        precitec::interface::GeoDoublearray resOpResOut;
        const auto resFilterOpResOut = outPipe->read(resOpResOut.context().imageNumber());
#if TEXT_OUT
        std::cout << "Output pipe 'OperationResult_out': " << resFilterOpResOut.ref().getData()[0] << "  exp: " << expectedResult[imageNumber] << std::endl;
#endif

        QCOMPARE(resFilterOpResOut.ref().size(), 1);
        QCOMPARE(resFilterOpResOut.ref().getData()[0], expectedResult[imageNumber]);
        
        // Prepare for next image
        imageNumber++;
        
    }  // for (int iTriggerCycle = 1; iTriggerCycle < 2; iTriggercycle++)
    
    // Filter must be triggered!
    QCOMPARE(filterOutData.isProceedCalled(),  true);
    QCOMPARE(filterOutOpRes.isProceedCalled(), true);

}

QTEST_GUILESS_MAIN(ConditionalTest)
#include "conditionalTest.moc"
