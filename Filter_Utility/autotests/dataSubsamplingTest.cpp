#include <QTest>

#include "../dataSubsampling.h"
#include "../dataSubsampling2.h"

#include <fliplib/NullSourceFilter.h>
#include <fliplib/SinkFilter.h>
#include <filter/sensorFilterInterface.h>

using precitec::filter::DataSubsampling;
using precitec::filter::DataSubsampling2;
using precitec::geo2d::Doublearray;
using namespace precitec::interface;

class DataSubsamplingTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testProceed_data();
    void testProceed();
};

void DataSubsamplingTest::testCtor()
{
    DataSubsampling filter;
    QCOMPARE(filter.name(), std::string("DataSubsampling"));
    QVERIFY(filter.findPipe("Data") != nullptr);
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);

    QVERIFY(filter.getParameters().exists(std::string("Operation")));
    QCOMPARE(filter.getParameters().findParameter(std::string("Operation")).getType(), fliplib::Parameter::TYPE_int);
    QCOMPARE(filter.getParameters().findParameter(std::string("Operation")).getValue().convert<int>(), 0);
    QVERIFY(filter.getParameters().exists(std::string("PassThroughBadRank")));
    QCOMPARE(filter.getParameters().findParameter(std::string("PassThroughBadRank")).getType(), fliplib::Parameter::TYPE_bool);
    QCOMPARE(filter.getParameters().findParameter(std::string("PassThroughBadRank")).getValue().convert<bool>(), false);
}


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

    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e) override
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




Q_DECLARE_METATYPE(Doublearray);

void DataSubsamplingTest::testProceed_data()
{
    using namespace precitec::geo2d;
    
    QTest::addColumn<int>("parameter_operation");
    QTest::addColumn<bool>("parameter_passthrough");
    QTest::addColumn<Doublearray>("input");
    QTest::addColumn<Doublearray>("expectedOutput");

    {
        Doublearray input;

        input.getData() = {1.0, 0.0, 5.0};
        input.getRank() = {255, 255, 255};

        for (int passthrough = 0; passthrough <= 1; passthrough++)
        {
            QTest::addRow("passthrough%d_none", passthrough) << 0 << bool(passthrough) << input << input;
            QTest::addRow("passthrough%d_last_value", passthrough) << 1  << bool(passthrough) << input << Doublearray{1, 5.0, 255};
            QTest::addRow("passthrough%d_mean", passthrough) << 2 << bool(passthrough) << input << Doublearray{1, 2.0, 255};
            QTest::addRow("passthrough%d_reduction10", passthrough) << 3 <<  bool(passthrough) <<input << Doublearray{1, 2.0, 255};
            QTest::addRow("passthrough%d_min", passthrough) << 4 << bool(passthrough) << input << Doublearray{1, 0.0, 255};
            QTest::addRow("passthrough%d_max", passthrough) << 5 << bool(passthrough) << input << Doublearray{1, 5.0, 255};
            QTest::addRow("passthrough%d_median", passthrough) << 6 << bool(passthrough) << input << Doublearray{1, 1.0, 255};
        }
    }

    {
        std::string test_category{"EmptyInput"};
        Doublearray empty_input;
        empty_input.getData() = {};
        empty_input.getRank() = {};
        Doublearray invalid_output {1, 0.0, 0};

        for (int operation = 0; operation < 6; operation ++)
        {
            for (int passthrough = 0; passthrough <= 1; passthrough++)
            {
                QTest::addRow("%s_%d_passthrough%d", test_category.c_str(), operation, passthrough) << operation << bool(passthrough) << empty_input << invalid_output;
            }
        }
    }
    {
        std::string test_category{"SingleInput"};

        Doublearray single_input{1, 5.0, 255};

        for (int operation = 0; operation < 6; operation ++)
        {
            for (int passthrough = 0; passthrough <= 1; passthrough++)
            {
                QTest::addRow("%s_%d_passthrough%d", test_category.c_str(), operation, passthrough) << operation << bool(passthrough) << single_input << single_input;
            }
        }
    }

    {
        std::string test_category{"BadRank"};
        Doublearray input;
        input.getData() = {1.0, 0.0, 5.0};
        input.getRank() = {255, 0, 0};

        bool passthrough = true;

        QTest::addRow("%s_none_passthrough%d", test_category.c_str(), passthrough) << 0 << bool(passthrough) << input << input;
        QTest::addRow("%s_last_value_passthrough%d", test_category.c_str(), passthrough) << 1 << bool(passthrough) << input << Doublearray{1, 5.0, 0};
        QTest::addRow("%s_mean_passthrough%d", test_category.c_str(), passthrough) << 2 << bool(passthrough) << input << Doublearray{1, 2.0, 0};
        QTest::addRow("%s_reduction10_passthrough%d", test_category.c_str(), passthrough) << 3  << bool(passthrough) << input << Doublearray{1, 2.0, 0};
        QTest::addRow("%s_min_passthrough%d", test_category.c_str(), passthrough) << 4  << bool(passthrough) << input << Doublearray{1, 0.0, 0};
        QTest::addRow("%s_max_passthrough%d", test_category.c_str(), passthrough) << 5  << bool(passthrough) << input << Doublearray{1, 5.0, 0};
        QTest::addRow("%s_median_passthrough%d", test_category.c_str(), passthrough) << 6 << bool(passthrough) << input << Doublearray{1, 1.0, 0};

        passthrough = false;

        QTest::addRow("%s_none_passthrough%d", test_category.c_str(), passthrough) << 0 << bool(passthrough) << input << input;
        QTest::addRow("%s_last_value_passthrough%d", test_category.c_str(), passthrough) << 1 << bool(passthrough) << input << Doublearray{1, 1.0, 255};
        QTest::addRow("%s_mean_passthrough%d", test_category.c_str(), passthrough) << 2 << bool(passthrough) << input << Doublearray{1, 1.0, 255};
        QTest::addRow("%s_reduction10_passthrough%d", test_category.c_str(), passthrough) << 3  << bool(passthrough) << input << Doublearray{1, 1.0, 255};
        QTest::addRow("%s_min_passthrough%d", test_category.c_str(), passthrough) << 4  << bool(passthrough) << input << Doublearray{1, 1.0, 255};
        QTest::addRow("%s_max_passthrough%d", test_category.c_str(), passthrough) << 5  << bool(passthrough) << input << Doublearray{1, 1.0, 255};
        QTest::addRow("%s_median_passthrough%d", test_category.c_str(), passthrough) << 6  << bool(passthrough) << input << Doublearray{1, 1.0, 255};
    }


    {
        std::string test_category{"Downsampling"};
        Doublearray input{ 20, 1.0, 255 };
        input.getData()[19] = 11.0;

        Doublearray expectedOutput(2, 1.0, 255);
        expectedOutput.getData()[1] = 2.0;

        bool passthrough = true;
        QTest::addRow("%s_reduction10_passthrough%d", test_category.c_str(), passthrough) << 3  << bool(passthrough) << input << expectedOutput;

        passthrough = false;
        QTest::addRow("%s_reduction10_passthrough%d", test_category.c_str(), passthrough) << 3  << bool(passthrough) << input << expectedOutput;

        input.getRank()[19] = 0;

        passthrough = true;
        expectedOutput.getData()[1] = 2.0;
        expectedOutput.getRank()[1] = 0;
        QTest::addRow("%s_reduction10_passthrough%d_badrank", test_category.c_str(), passthrough) << 3  << bool(passthrough) << input << expectedOutput;


        passthrough = false;
        expectedOutput.getData()[1] = 1.0;
        expectedOutput.getRank()[1] = 255;
        QTest::addRow("%s_reduction10_passthrough%d_badrank", test_category.c_str(), passthrough) << 3  << bool(passthrough) << input << expectedOutput;

    }
}

void DataSubsamplingTest::testProceed()
{

    // prepare filter graph
    DataSubsampling filterSingleInput;
    DataSubsampling2 filterDoubleInput;

    //connect the input and output pipes of filter1
    fliplib::NullSourceFilter sourceFilter;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipe1_1{ &sourceFilter, "pipe1ID" };
    pipe1_1.setTag("data_a");
    QVERIFY(filterSingleInput.connectPipe(&pipe1_1, 0));
    DummyFilter dummyFilter1;
    QVERIFY(dummyFilter1.connectPipe(filterSingleInput.findPipe("Data"), 0));

    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipe1_2{ &sourceFilter, "pipe11ID" };
    pipe1_2.setTag("input1");
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipe2_2{ &sourceFilter, "pipe12ID" };
    pipe2_2.setTag("input2");
    QVERIFY(filterDoubleInput.connectPipe(&pipe1_2, 1));
    QVERIFY(filterDoubleInput.connectPipe(&pipe2_2, 1));
    DummyFilter dummyFilter2;
    QVERIFY(dummyFilter2.connectPipe(filterDoubleInput.findPipe("Data1"), 1));
    QVERIFY(dummyFilter2.connectPipe(filterDoubleInput.findPipe("Data2"), 1));


    // parameterize the filters
    QFETCH(int, parameter_operation);
    QFETCH(bool, parameter_passthrough);
        filterSingleInput.getParameters().update(std::string("Operation"), fliplib::Parameter::TYPE_int, parameter_operation);
        filterSingleInput.getParameters().update(std::string("PassThroughBadRank"), fliplib::Parameter::TYPE_bool, parameter_passthrough);
        filterSingleInput.setParameter();
        filterDoubleInput.getParameters().update(std::string("Operation"), fliplib::Parameter::TYPE_int, parameter_operation);
        filterDoubleInput.getParameters().update(std::string("PassThroughBadRank"), fliplib::Parameter::TYPE_bool, parameter_passthrough);
        filterDoubleInput.setParameter();

    // and process
    QFETCH(Doublearray, input);
    auto geoInput =  GeoDoublearray{ImageContext{}, input, AnalysisOK, 1.0 };
    Doublearray input2 = input;
    for (auto && data : input2.getData())
    {
        data *= 2;
    }
    auto geoInput2 =  GeoDoublearray{ImageContext{}, input2, AnalysisOK, 1.0 };

    // now signal the pipe, this processes the complete filter graph
    QCOMPARE(dummyFilter1.isProceedCalled(), false);
    pipe1_1.signal(geoInput);
    QCOMPARE(dummyFilter1.isProceedCalled(), true);

    QCOMPARE(dummyFilter2.isProceedCalled(), false);
    pipe1_2.signal(geoInput);
    pipe2_2.signal(geoInput2);
    QCOMPARE(dummyFilter2.isProceedCalled(), true);


    // access the out pipe data of the filter

    QFETCH(Doublearray, expectedOutput);

    auto outPipe1_1 = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray>*>(filterSingleInput.findPipe("Data"));
    QVERIFY(outPipe1_1);

    auto geoResult1_1 = outPipe1_1->read(geoInput.context().imageNumber());
    auto result1_1 = geoResult1_1.ref();
    if (result1_1.getData() != expectedOutput.getData())
    {
        std::cout << "result " << std::endl;
        for (auto i : result1_1.getData())
        {
            std::cout << i << " ";
        }
        std::cout << std::endl;
        std::cout << "expectedOutput " << std::endl;
        for (auto i : expectedOutput.getData())
        {
            std::cout << i << " ";
        }
        std::cout << std::endl;
    }
    if (result1_1.getRank() != expectedOutput.getRank())
    {
        std::cout << "rank " << std::endl;
        for (auto i : result1_1.getRank())
        {
            std::cout << i << " ";
        }
        std::cout << std::endl;
        std::cout << "expected " << std::endl;
        for (auto i : expectedOutput.getRank())
        {
            std::cout << i << " ";
        }
        std::cout << std::endl;
    }

    QCOMPARE(result1_1.getData(), expectedOutput.getData());
    QCOMPARE(result1_1.getRank(), expectedOutput.getRank());


    auto outPipe1_2 = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray>*>(filterDoubleInput.findPipe("Data1"));
    QVERIFY(outPipe1_2);

    auto geoResult1_2 = outPipe1_2->read(geoInput.context().imageNumber());
    auto result1_2 = geoResult1_2.ref();
    QCOMPARE(result1_2.getData(), expectedOutput.getData());
    QCOMPARE(result1_2.getRank(), expectedOutput.getRank());

    auto outPipe2_2 = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray>*>(filterDoubleInput.findPipe("Data2"));
    QVERIFY(outPipe2_2);

    auto geoResult2_2 = outPipe2_2->read(geoInput.context().imageNumber());
    auto result2_2 = geoResult2_2.ref();
    QCOMPARE(result2_2.size(), expectedOutput.size());
    for (auto  i = 0u; i < result2_2.size(); i++)
    {
        QCOMPARE(result2_2.getData()[i], 2*expectedOutput.getData()[i]);
    }
    QCOMPARE(result2_2.getRank(), expectedOutput.getRank());
}


QTEST_GUILESS_MAIN( DataSubsamplingTest )
#include "dataSubsamplingTest.moc"
