#include <QTest>

#include "../rangeCheck.h"
#include "DummyResultHandler.h"
#include <fliplib/NullSourceFilter.h>

using precitec::filter::RangeCheck;
using precitec::interface::ResultType;
using precitec::geo2d::TArray;
using precitec::geo2d::Doublearray;
using precitec::interface::ResultDoubleArray;

class testRangeCheck : public QObject
{
    Q_OBJECT
    typedef std::vector<std::pair<double,int>> dataarray_t;
private Q_SLOTS:
    void testCtor();
    void testProceed_data();
    void testProceed();
};


void testRangeCheck::testCtor()
{
    RangeCheck filter;
    QCOMPARE(filter.name(), std::string("RangeCheck"));
    QVERIFY(filter.findPipe("DoubleOK") != nullptr);
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);
    QVERIFY(filter.getParameters().exists(std::string("Min")));
    QVERIFY(filter.getParameters().exists(std::string("Max")));
    QVERIFY(filter.getParameters().exists(std::string("ResultType")));
    QVERIFY(filter.getParameters().exists(std::string("NioType")));

    QCOMPARE(filter.getParameters().findParameter(std::string("Min")).getType(), fliplib::Parameter::TYPE_double);
    QCOMPARE(filter.getParameters().findParameter(std::string("Min")).getValue().convert<double>(), 0.0);

    QCOMPARE(filter.getParameters().findParameter(std::string("Max")).getType(), fliplib::Parameter::TYPE_double);
    QCOMPARE(filter.getParameters().findParameter(std::string("Max")).getValue().convert<double>(), 0.0);
    
    QCOMPARE(filter.getParameters().findParameter(std::string("ResultType")).getType(), fliplib::Parameter::TYPE_int);
    QCOMPARE(filter.getParameters().findParameter(std::string("ResultType")).getValue().convert<int>(), ResultType::Value);
    
    QCOMPARE(filter.getParameters().findParameter(std::string("NioType")).getType(), fliplib::Parameter::TYPE_int);
    QCOMPARE(filter.getParameters().findParameter(std::string("NioType")).getValue().convert<int>(), ResultType::ValueOutOfLimits);
    
}

void testRangeCheck::testProceed_data()
{
    QTest::addColumn<dataarray_t>("inputArray");
    QTest::addColumn<double>("param_min");
    QTest::addColumn<double>("param_max");
    QTest::addColumn<bool>("expectedNio");
    
    QTest::newRow("EmptyArray")             << dataarray_t{}            << 1.0  << 1.6 << false;
    QTest::newRow("OneValue_io")            << dataarray_t{{-1.5,255}}  << -2.0 << 1.6 << false;
    QTest::newRow("RankMin_io")             << dataarray_t{{-1.5,0}}    << -2.0 << 1.6 << false;
    QTest::newRow("OneValue_RankMin_io")    << dataarray_t{{-1.5,0}}    << -2.0 << 1.6 << false;
    QTest::newRow("OneValue_nio")           << dataarray_t{{-1.5,255}}  << 1.0  << 1.6 << true;
    QTest::newRow("OneValue_RankMin_nio")   << dataarray_t{{-1.5,0}}    << 1.0  << 1.6 << true;
    QTest::newRow("Multiple_io")  << dataarray_t{{-1.5,255},{1,255}}    << -2.0 << 1.6 << false;
    QTest::newRow("Multiple_nio") << dataarray_t{{-1.5,255},{2,255}}    << -2.0 << 1.6 << true;

}

void testRangeCheck::testProceed()
{
    RangeCheck filter;
    fliplib::NullSourceFilter sourceFilter;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipe{ &sourceFilter, "inputPipe"};

    QVERIFY(filter.connectPipe(&pipe, 0));

    DummyResultHandler dummyFilter;
    QVERIFY(dummyFilter.connectPipe(filter.findPipe("DoubleOK"), 0));

    // parameterize the filter
    QFETCH(double, param_min);
    QFETCH(double, param_max);
    
    filter.getParameters().update(std::string("Min"), fliplib::Parameter::TYPE_double, param_min);
    filter.getParameters().update(std::string("Max"), fliplib::Parameter::TYPE_double, param_max);
    filter.getParameters().update(std::string("ResultType"), fliplib::Parameter::TYPE_int, int(ResultType::CoordPositionX));
    filter.getParameters().update(std::string("NioType"), fliplib::Parameter::TYPE_int, int(ResultType::XCoordOutOfLimits));
    
    filter.setParameter();
    
    //proceed
    
    QFETCH(dataarray_t, inputArray);
    Doublearray darray(inputArray.size());
    for (unsigned int i = 0; i < inputArray.size(); ++i)
    {
        darray[i] = inputArray[i];
    }
    int imageNumber = 1;
    precitec::interface::ImageContext context;
    context.setImageNumber(imageNumber);
    context.setPosition(2);
        
    auto inPipe = precitec::interface::GeoDoublearray {
            context, 
            darray, 
            precitec::interface::ResultType::AnalysisOK, 
            precitec::interface::Limit 
        };
        
    QCOMPARE(dummyFilter.isProceedCalled(), false);
    pipe.signal(inPipe);
    QCOMPARE(dummyFilter.isProceedCalled(), true);
    
    // test result
    auto outPipe = dynamic_cast<fliplib::SynchronePipe<ResultDoubleArray>*>(filter.findPipe("DoubleOK"));
    QVERIFY(outPipe);
    
    auto result = outPipe->read(imageNumber);
    QCOMPARE(result.value().size(), inputArray.size());
    for (unsigned int i = 0; i < inputArray.size(); ++i)
    {
        QCOMPARE(result.value()[i], inputArray[i].first);
        QCOMPARE(result.rank()[i], inputArray[i].second);        
    }
    
    QFETCH(bool, expectedNio);
    QCOMPARE(result.isNio(), expectedNio);
    QCOMPARE(result.resultType(), ResultType::CoordPositionX);
    QCOMPARE(result.nioType(), ResultType::XCoordOutOfLimits);
    
    
    // test conversion to nio
    QVERIFY(dynamic_cast<precitec::interface::NIOResult *>(&result));
   
    //test serialization
    precitec::system::message::StaticMessageBuffer buffer(1024);
    auto p_resultargs = dynamic_cast<precitec::interface::NIOResult *>(&result);

    p_resultargs->serialize(buffer);
    
    {
        buffer.rewind();
        precitec::interface::ResultArgs deserialized;
        deserialized.deserialize(buffer);
        QCOMPARE(deserialized.isNio(), result.isNio());
        QCOMPARE(deserialized.resultType(), result.resultType());
        QCOMPARE(deserialized.nioType(), result.nioType());
        QCOMPARE(deserialized.type(), result.type());
        QCOMPARE(deserialized.isValid(), result.isValid());
        if (deserialized.isValid())
        {
            QCOMPARE(deserialized.value<double>().size(), result.value().size());
            for (unsigned int i = 0; i < inputArray.size(); ++i)
            {
                QCOMPARE(deserialized.value<double>()[i], result.value()[i]);
                QCOMPARE(deserialized.rank()[i], result.rank()[i]);                
            }
        
        }
    }
    {
        buffer.rewind();
        ResultDoubleArray deserialized;
        deserialized.deserialize(buffer);
        QCOMPARE(deserialized.isNio(), result.isNio());
        QCOMPARE(deserialized.resultType(), result.resultType());
        QCOMPARE(deserialized.nioType(), result.nioType());
        QCOMPARE(deserialized.type(), result.type());
        QCOMPARE(deserialized.isValid(), result.isValid());
        if (deserialized.isValid())
        {
            QCOMPARE(deserialized.value().size(), result.value().size());
            for (unsigned int i = 0; i < inputArray.size(); ++i)
            {
                QCOMPARE(deserialized.value()[i], result.value()[i]);
                QCOMPARE(deserialized.rank()[i], result.rank()[i]);
            }
        
        }
    }
}

QTEST_GUILESS_MAIN(testRangeCheck)
#include "testRangeCheck.moc"
