#include <QTest>
#include "../temporalLowPass.h"
#include "../temporalLowPass2.h"

#include <fliplib/NullSourceFilter.h>
#include <fliplib/SinkFilter.h>
#include <filter/sensorFilterInterface.h>

using precitec::filter::TemporalLowPass;
using precitec::filter::TemporalLowPass2;
using precitec::geo2d::Doublearray;
using namespace precitec::interface;

class TemporalLowPassTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor1();
    void testCtor2();
    void testProceed_data();
    void testProceed();
};

Q_DECLARE_METATYPE(precitec::geo2d::Doublearray)

namespace precitec
{
//dummy logger which raises an exception for log messages with a level > info
LogMessage* getLogMessageOfBaseModuleLogger()
{
    static LogMessage msg;
    return &msg;
}

void invokeIncreaseWriteIndex()
{
}

void redirectLogMessage(LogMessage* msg)
{
    qDebug() << msg->format().c_str();
    switch (msg->m_oType)
    {
    case eInfo:
    case eDebug:
        //do nothing
        break;
    default:
        throw "Unexpected Log Message";
        break;
    }
}
}

struct DummyInput
{

    fliplib::NullSourceFilter sourceFilter;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> m_pipeValueSingle{&sourceFilter, "position"};
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> m_pipeValue1{&sourceFilter, "input1"};
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> m_pipeValue2{&sourceFilter, "input2"};

    void fillDataAndSignal(int imageNumber, Doublearray inputValue1, Doublearray inputValue2)
    {
        using namespace precitec::interface;
        using namespace precitec::geo2d;

        ImageContext context;
        context.setImageNumber(imageNumber);

        auto geo1 = precitec::interface::GeoDoublearray {
            context,
                inputValue1,
                ResultType::AnalysisOK, Limit
        };
        auto geo2 = precitec::interface::GeoDoublearray {
            context,
                inputValue2,
                ResultType::AnalysisOK, Limit
        };
        m_pipeValueSingle.signal(geo1);
        m_pipeValue1.signal(geo1);
        m_pipeValue2.signal(geo2);
    }
};

class DummyFilter : public fliplib::BaseFilter
{
public:
    DummyFilter()
        : fliplib::BaseFilter("dummy")
    {
    }
    void proceed(const void* sender, fliplib::PipeEventArgs& event) override
    {
        Q_UNUSED(sender)
        Q_UNUSED(event)
        preSignalAction();
        m_proceedCalled = true;
    }
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e)
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

void TemporalLowPassTest::testCtor1()
{
    TemporalLowPass filter;
    QCOMPARE(filter.name(), std::string("TemporalLowPass"));
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);
    QVERIFY(filter.findPipe("Value"));

    QCOMPARE(filter.getParameters().findParameter(std::string("LowPassType")).convert<int>(), 0);
    QCOMPARE(filter.getParameters().findParameter(std::string("FilterLength")).convert<unsigned int>(), 3u);
    QCOMPARE(filter.getParameters().findParameter(std::string("StartImage")).convert<unsigned int>(), 1u);
    QCOMPARE(filter.getParameters().findParameter(std::string("MaxJump")).convert<double>(), 20.0);
    QCOMPARE(filter.getParameters().findParameter(std::string("MaxJumpAddOn")).convert<bool>(), true);
    QCOMPARE(filter.getParameters().findParameter(std::string("PassThroughBadRank")).convert<bool>(), true);
}

void TemporalLowPassTest::testCtor2()
{
    TemporalLowPass2 filter;
    QCOMPARE(filter.name(), std::string("TemporalLowPass2"));
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);
    QVERIFY(filter.findPipe("Value1"));
    QVERIFY(filter.findPipe("Value2"));

    QCOMPARE(filter.getParameters().findParameter(std::string("LowPassType")).convert<int>(), 0);
    QCOMPARE(filter.getParameters().findParameter(std::string("FilterLength")).convert<unsigned int>(), 3u);
    QCOMPARE(filter.getParameters().findParameter(std::string("StartImage")).convert<unsigned int>(), 1u);
    QCOMPARE(filter.getParameters().findParameter(std::string("MaxJump")).convert<double>(), 20.0);
    QCOMPARE(filter.getParameters().findParameter(std::string("MaxJumpAddOn")).convert<bool>(), true);
    QCOMPARE(filter.getParameters().findParameter(std::string("PassThroughBadRank")).convert<bool>(), true);
}

std::vector<Doublearray> splitDoubleArrayIntoFrames(const Doublearray& array, int frameSize)
{
    std::vector<Doublearray> result;
    if (frameSize == 0)
    {
        return result;
    }
    int numFrames = array.size() / frameSize;
    int dataCounter = 0;
    for (int imgNumber = 0; imgNumber < numFrames; imgNumber++)
    {
        Doublearray frame(frameSize);
        for (int i = 0; i < frameSize; i++)
        {
            assert(dataCounter < int(array.size()));
            frame[i] = array[dataCounter];
            dataCounter++;
        }
        result.push_back(frame);
    }
    return result;
}

void TemporalLowPassTest::testProceed_data()
{

    const std::vector<int> frameSizesToTest{1, 5, 30, 60, 0};

    QTest::addColumn<int>("parameter_lowpassType");
    QTest::addColumn<int>("parameter_filterLength");
    QTest::addColumn<int>("parameter_startImage");
    QTest::addColumn<double>("parameter_maxJump");
    QTest::addColumn<bool>("parameter_maxJumpAddOn");
    QTest::addColumn<bool>("parameter_passthrough");
    QTest::addColumn<std::vector<Doublearray>>("input1");
    QTest::addColumn<std::vector<Doublearray>>("expectedOutput1");
    QTest::addColumn<std::vector<Doublearray>>("input2");
    QTest::addColumn<std::vector<Doublearray>>("expectedOutput2");

    int startImage = 1;
    double maxJump = 20.0;
    bool maxJumpAddOn = true;
    bool passthrough = false;

    //////////////////////////////////////////////////////////////
    // Test passthrough and maxjump with mean filter (length 3)
    /////////////////////////////////////////////////////////////

    {
        int filterLength = 3;
        int filterType = 0; // mean
        double maxJump = 20.0;

        // numbers from 1 to 30, then back to 1
        Doublearray inputValues(60, 0, 255);
        double value = 1.0;
        for (int i = 0; i < 30; i++, value++)
        {
            inputValues.getData()[i] = value;
        }
        value = 30;
        for (int i = 29; i < 60; i++, value--)
        {
            inputValues.getData()[i] = value;
        }

        // output values without max jump  1 1.5 2 3 4 ...29 29.33 29 28 ... 1
        Doublearray outputValues(60, 0, 255);
        outputValues[0] = std::make_tuple(1.0, 255);
        outputValues[1] = std::make_tuple((1.0 + 2.0) / 2, 255);
        value = (1.0 + 2.0 + 3.0) / 3;
        for (int i = 2; i < 30; i++, value++)
        {
            outputValues.getData()[i] = value;
        }
        outputValues[30] = std::make_tuple((29.0 + 30.0 + 29.0) / 3, 255);
        outputValues[31] = std::make_tuple((30.0 + 29.0 + 28.0) / 3, 255);
        value = (29.0 + 28.0 + 27.0) / 3;
        for (int i = 32; i < 60; i++, value--)
        {
            outputValues.getData()[i] = value;
        }
        for (int frameSize : frameSizesToTest)
        {
            auto input1 = splitDoubleArrayIntoFrames(inputValues, frameSize);
            auto output1 = splitDoubleArrayIntoFrames(outputValues, frameSize);
            auto input2 = splitDoubleArrayIntoFrames(inputValues, 1);
            auto output2 = splitDoubleArrayIntoFrames(outputValues, 1);
            QTest::addRow("mean_%d", frameSize)
                << filterType << filterLength << startImage << maxJump << maxJumpAddOn << passthrough
                << input1 << output1 << input2 << output2;
        }

        // test bad rank in the middle of the sequence
        auto inputValuesBadRankMid = inputValues;
        for (int index = 5; index < 9; index++)
        {
            inputValuesBadRankMid.getRank()[index] = 0;
        }
        auto outputBadRankMid_NoPassThrough = outputValues;
        outputBadRankMid_NoPassThrough[4] = std::make_tuple(4.0, 255);                      //input: 5 with good rank, output: mean(3, 4, 5) = 4
        outputBadRankMid_NoPassThrough[5] = std::make_tuple(4.0, 0);                        // input has bad rank
        outputBadRankMid_NoPassThrough[6] = std::make_tuple(4.0, 0);                        // input has bad rank
        outputBadRankMid_NoPassThrough[7] = std::make_tuple(4.0, 0);                        // input has bad rank
        outputBadRankMid_NoPassThrough[8] = std::make_tuple(4.0, 0);                        // input has bad rank
        outputBadRankMid_NoPassThrough[9] = std::make_tuple((4.0 + 5.0 + 10.0) / 3, 255);   // input 10 with good rank
        outputBadRankMid_NoPassThrough[10] = std::make_tuple((5.0 + 10.0 + 11.0) / 3, 255); // input 11 with good rank
        for (int frameSize : frameSizesToTest)
        {
            auto input1 = splitDoubleArrayIntoFrames(inputValuesBadRankMid, frameSize);
            auto outputPassThrough1 = splitDoubleArrayIntoFrames(outputValues, frameSize);
            auto outputNoPassThrough1 = splitDoubleArrayIntoFrames(outputBadRankMid_NoPassThrough, frameSize);
            auto input2 = splitDoubleArrayIntoFrames(inputValuesBadRankMid, 1);
            auto outputPassThrough2 = splitDoubleArrayIntoFrames(outputValues, 1);
            auto outputNoPassThrough2 = splitDoubleArrayIntoFrames(outputBadRankMid_NoPassThrough, 1);
            QTest::addRow("BadRankMid_NoPassThrough_%d", frameSize)
                << filterType << filterLength << startImage << maxJump << maxJumpAddOn << false
                << input1 << outputNoPassThrough1 << input2 << outputNoPassThrough2;
            QTest::addRow("BadRankMid_PassThrough_%d", frameSize)
                << filterType << filterLength << startImage << maxJump << maxJumpAddOn << true
                << input1 << outputPassThrough1 << input2 << outputPassThrough2;
        }

        // test bad rank at the start of the sequence
        auto inputValuesBadRankStart = inputValues;
        for (int index = 0; index < 5; index++)
        {
            inputValuesBadRankStart.getRank()[index] = 0;
        }

        Doublearray outputBadRankStart_NoPassThrough = outputValues;
        outputBadRankStart_NoPassThrough[0] = std::make_tuple(0.0, 0);               // input 1 bad rank
        outputBadRankStart_NoPassThrough[1] = std::make_tuple(0.0, 0);               // input 2 bad rank
        outputBadRankStart_NoPassThrough[2] = std::make_tuple(0.0, 0);               // input 3 bad rank
        outputBadRankStart_NoPassThrough[3] = std::make_tuple(0.0, 0);               // input 4 bad rank
        outputBadRankStart_NoPassThrough[4] = std::make_tuple(0.0, 0);               // input 5 bad rank
        outputBadRankStart_NoPassThrough[5] = std::make_tuple(6.0, 255);             // input 6 good rank
        outputBadRankStart_NoPassThrough[6] = std::make_tuple((6.0 + 7.0) / 2, 255); // input 7 good rank
        outputBadRankStart_NoPassThrough[7] = std::make_tuple((6.0 + 7.0 + 8.0) / 3, 255);
        for (int frameSize : frameSizesToTest)
        {
            auto input1 = splitDoubleArrayIntoFrames(inputValuesBadRankStart, frameSize);
            auto outputPassThrough1 = splitDoubleArrayIntoFrames(outputValues, frameSize);
            auto outputNoPassThrough1 = splitDoubleArrayIntoFrames(outputBadRankStart_NoPassThrough, frameSize);
            auto input2 = splitDoubleArrayIntoFrames(inputValuesBadRankStart, 1);
            auto outputPassThrough2 = splitDoubleArrayIntoFrames(outputValues, 1);
            auto outputNoPassThrough2 = splitDoubleArrayIntoFrames(outputBadRankStart_NoPassThrough, 1);
            QTest::addRow("BadRankStart_NoPassThrough_%d", frameSize)
                << filterType << filterLength << startImage << maxJump << maxJumpAddOn << false
                << input1 << outputNoPassThrough1 << input2 << outputNoPassThrough2;
            QTest::addRow("BadRankStart_PassThrough_%d", frameSize)
                << filterType << filterLength << startImage << maxJump << maxJumpAddOn << true
                << input1 << outputPassThrough1 << input2 << outputPassThrough2;
            QTest::addRow("BadRankStartMaxJump_NoPassThrough_%d", frameSize)
                << filterType << filterLength << startImage << 2.0 << maxJumpAddOn << false
                << input1 << outputNoPassThrough1 << input2 << outputNoPassThrough2;
            QTest::addRow("BadRankStartMaxJump_PassThrough_%d", frameSize)
                << filterType << filterLength << startImage << 2.0 << maxJumpAddOn << true
                << input1 << outputPassThrough1 << input2 << outputPassThrough2;
        }

        // test max jump
        maxJump = 0.6;
        //output without addOn: 1 1.5 2 2 2 ...2 2 2 ... 2 2
        Doublearray outputValues_NoAddOn(60, 2.0, 127);
        outputValues_NoAddOn[0] = outputValues[0];
        outputValues_NoAddOn[1] = outputValues[1];
        outputValues_NoAddOn[2] = outputValues[2];
        outputValues_NoAddOn[58] = outputValues[58];

        //output with addOn: 1 1.5 2 2.6 3.2 ..37 35.4 ...
        Doublearray outputValues_AddOn(60, 2.0, 127);
        outputValues_AddOn[0] = outputValues[0];
        outputValues_AddOn[1] = outputValues[1];
        outputValues_AddOn[2] = outputValues[2];
        for (int i = 3; i < 37; i++)
        {
            outputValues_AddOn.getData()[i] = outputValues_AddOn.getData()[i - 1] + maxJump;
        }
        outputValues_AddOn[37] = std::make_tuple(23.0, 255);
        for (int i = 38; i < 60; i++)
        {
            outputValues_AddOn.getData()[i] = outputValues_AddOn.getData()[i - 1] - maxJump;
        }

        for (int frameSize : frameSizesToTest)
        {
            auto input1 = splitDoubleArrayIntoFrames(inputValues, frameSize);
            auto input2 = splitDoubleArrayIntoFrames(inputValues, 1);

            auto outputNoAddOn1 = splitDoubleArrayIntoFrames(outputValues_NoAddOn, frameSize);
            auto outputNoAddOn2 = splitDoubleArrayIntoFrames(outputValues_NoAddOn, 1);
            QTest::addRow("MaxJumpSmall_NoAddOn_%d", frameSize)
                << filterType << filterLength << startImage << maxJump << false << passthrough
                << input1 << outputNoAddOn1 << input2 << outputNoAddOn2;

            auto outputAddOn1 = splitDoubleArrayIntoFrames(outputValues_AddOn, frameSize);
            auto outputAddOn2 = splitDoubleArrayIntoFrames(outputValues_AddOn, 1);
            QTest::addRow("MaxJumpSmall_AddOn_%d", frameSize)
                << filterType << filterLength << startImage << maxJump << true << passthrough
                << input1 << outputAddOn1 << input2 << outputAddOn2;
        }
    }

    ////////////////////////////////////////////////////////
    // Test different filter types with length 3
    ////////////////////////////////////////////////////////

    // increasing: input numbers from 0 to 30
    Doublearray inputValues_0_30(30, 0, 255);
    std::iota(inputValues_0_30.getData().begin(), inputValues_0_30.getData().end(), 0);

    // decreasing: input numbers from 30 to 0
    Doublearray inputValues_30_0(30, 0, 255);
    std::iota(inputValues_30_0.getData().rbegin(), inputValues_30_0.getData().rend(), 1);

    int filterLength = 3;
    {
        int filterType = 0; // mean

        // input: 0 1 2 3 ...
        // output: 0 0.5 1 2 3 4 ...
        Doublearray outputValuesInc(30, 0, 255);
        outputValuesInc.getData()[0] = 0;
        outputValuesInc.getData()[1] = (0.0 + 1.0) / 2;
        outputValuesInc.getData()[2] = (0.0 + 1.0 + 2.0) / 3;
        std::iota(outputValuesInc.getData().begin() + 3, outputValuesInc.getData().end(), 2);

        for (int frameSize : frameSizesToTest)
        {
            auto input1 = splitDoubleArrayIntoFrames(inputValues_0_30, frameSize);
            auto output1 = splitDoubleArrayIntoFrames(outputValuesInc, frameSize);
            auto input2 = splitDoubleArrayIntoFrames(inputValues_0_30, 1);
            auto output2 = splitDoubleArrayIntoFrames(outputValuesInc, 1);
            QTest::addRow("mean_%d", frameSize)
                << filterType << filterLength << startImage << maxJump << maxJumpAddOn << passthrough
                << input1 << output1 << input2 << output2;
        }
    }

    {
        int filterType = 1; // median

        // output: 0 1 1 1 2 3 4 ...
        Doublearray outputArray(30, 0, 255);
        outputArray.getData()[0] = 0;
        outputArray.getData()[1] = 1;
        outputArray.getData()[2] = 1;
        std::iota(outputArray.getData().begin() + 3, outputArray.getData().end(), 2);

        for (int frameSize : frameSizesToTest)
        {
            auto input1 = splitDoubleArrayIntoFrames(inputValues_0_30, frameSize);
            auto output1 = splitDoubleArrayIntoFrames(outputArray, frameSize);
            auto input2 = splitDoubleArrayIntoFrames(inputValues_0_30, 1);
            auto output2 = splitDoubleArrayIntoFrames(outputArray, 1);
            QTest::addRow("median_%d", frameSize)
                << filterType << filterLength << startImage << maxJump << maxJumpAddOn << passthrough
                << input1 << output1 << input2 << output2;
        }
    }
    {
        int filterType = 2; // MinLowPass

        // output: 0 0 0 1 2
        Doublearray outputArray(30, 0, 255);
        outputArray.getData()[0] = 0;
        outputArray.getData()[1] = 0;
        outputArray.getData()[2] = 0;
        std::iota(outputArray.getData().begin() + 3, outputArray.getData().end(), 1);

        for (int frameSize : frameSizesToTest)
        {
            auto input1 = splitDoubleArrayIntoFrames(inputValues_0_30, frameSize);
            auto output1 = splitDoubleArrayIntoFrames(outputArray, frameSize);
            auto input2 = splitDoubleArrayIntoFrames(inputValues_0_30, 1);
            auto output2 = splitDoubleArrayIntoFrames(outputArray, 1);
            QTest::addRow("MinLowPass_%d", frameSize)
                << filterType << filterLength << startImage << maxJump << maxJumpAddOn << passthrough
                << input1 << output1 << input2 << output2;
        }
    }
    {
        int filterType = 3; // MaxLowPass

        // output: 0 1 2 3 ...
        Doublearray outputArray(30, 0, 255);
        outputArray.getData()[0] = 0;
        outputArray.getData()[1] = 1;
        outputArray.getData()[2] = 2;
        std::iota(outputArray.getData().begin() + 3, outputArray.getData().end(), 3);

        for (int frameSize : frameSizesToTest)
        {
            auto input1 = splitDoubleArrayIntoFrames(inputValues_0_30, frameSize);
            auto output1 = splitDoubleArrayIntoFrames(outputArray, frameSize);
            auto input2 = splitDoubleArrayIntoFrames(inputValues_0_30, 1);
            auto output2 = splitDoubleArrayIntoFrames(outputArray, 1);
            QTest::addRow("MaxLowPass_%d", frameSize)
                << filterType << filterLength << startImage << maxJump << maxJumpAddOn << passthrough
                << input1 << output1 << input2 << output2;
        }

        // decreasing sequence 30 29 28  27 .. 1
        outputArray.getData()[0] = 30;
        outputArray.getData()[1] = 30;
        outputArray.getData()[2] = 30;
        std::iota(outputArray.getData().rbegin(), outputArray.getData().rend() - 3, 3);
        for (int frameSize : frameSizesToTest)
        {
            auto input1 = splitDoubleArrayIntoFrames(inputValues_30_0, frameSize);
            auto output1 = splitDoubleArrayIntoFrames(outputArray, frameSize);
            auto input2 = splitDoubleArrayIntoFrames(inputValues_30_0, 1);
            auto output2 = splitDoubleArrayIntoFrames(outputArray, 1);
            QTest::addRow("MaxLowPass_dec_%d", frameSize)
                << filterType << filterLength << startImage << maxJump << maxJumpAddOn << passthrough
                << input1 << output1 << input2 << output2;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Test different filter types with length 1 (output equal to input, unless maxJump or bad rank)
    //////////////////////////////////////////////////////////////////////////////////////////////////

    // input values with bad rank and corresponding output without filtering
    auto inputValuesBadRankMid_0_30 = inputValues_0_30;
    auto outputNoFilter_BadRankMid_0_30 = inputValues_0_30;
    for (int index = 5; index < 9; index++)
    {
        inputValuesBadRankMid_0_30.getRank()[index] = 0;
        outputNoFilter_BadRankMid_0_30[index] = std::make_tuple(inputValues_0_30.getData()[4], 0);
    }

    auto inputValuesBadRankStart_0_30 = inputValues_0_30;
    auto outputNoFilter_BadRankStart_0_30 = inputValues_0_30;
    for (int index = 0; index < 5; index++)
    {
        inputValuesBadRankStart_0_30.getRank()[index] = 0;
        outputNoFilter_BadRankStart_0_30[index] = std::make_tuple(0.0, 0);
    }

    {
        int filterLength = 1;

        double maxJump = 30;
        startImage = 1;

        for (int filterType = 0; filterType <= 3; filterType++)
        {
            for (int frameSize : frameSizesToTest)
            {
                {
                    auto input1 = splitDoubleArrayIntoFrames(inputValues_0_30, frameSize);
                    auto output1 = splitDoubleArrayIntoFrames(inputValues_0_30, frameSize);
                    auto input2 = splitDoubleArrayIntoFrames(inputValues_0_30, 1);
                    auto output2 = splitDoubleArrayIntoFrames(inputValues_0_30, 1);
                    QTest::addRow("Length1GoodRank_%d_%d", filterType, frameSize)
                        << filterType << filterLength << startImage << maxJump << maxJumpAddOn << passthrough
                        << input1 << output1 << input2 << output2;
                }
                {
                    auto input1 = splitDoubleArrayIntoFrames(inputValuesBadRankStart_0_30, frameSize);
                    auto output1 = splitDoubleArrayIntoFrames(inputValuesBadRankStart_0_30, frameSize);
                    auto input2 = splitDoubleArrayIntoFrames(inputValuesBadRankStart_0_30, 1);
                    auto output2 = splitDoubleArrayIntoFrames(inputValuesBadRankStart_0_30, 1);
                    QTest::addRow("Length1BadStartPassthrough_%d_%d", filterType, frameSize)
                        << filterType << filterLength << startImage << maxJump << maxJumpAddOn << true
                        << input1 << output1 << input2 << output2;
                }
                {
                    auto input1 = splitDoubleArrayIntoFrames(inputValuesBadRankMid_0_30, frameSize);
                    auto output1 = splitDoubleArrayIntoFrames(inputValuesBadRankMid_0_30, frameSize);
                    auto input2 = splitDoubleArrayIntoFrames(inputValuesBadRankMid_0_30, 1);
                    auto output2 = splitDoubleArrayIntoFrames(inputValuesBadRankMid_0_30, 1);
                    QTest::addRow("Length1BadMidPassthrough_%d_%d", filterType, frameSize)
                        << filterType << filterLength << startImage << maxJump << maxJumpAddOn << true
                        << input1 << output1 << input2 << output2;
                }
                {
                    auto input1 = splitDoubleArrayIntoFrames(inputValuesBadRankStart_0_30, frameSize);
                    auto output1 = splitDoubleArrayIntoFrames(outputNoFilter_BadRankStart_0_30, frameSize);
                    auto input2 = splitDoubleArrayIntoFrames(inputValuesBadRankStart_0_30, 1);
                    auto output2 = splitDoubleArrayIntoFrames(outputNoFilter_BadRankStart_0_30, 1);
                    QTest::addRow("Length1BadStartNoPassthrough_%d_%d", filterType, frameSize)
                        << filterType << filterLength << startImage << maxJump << maxJumpAddOn << false
                        << input1 << output1 << input2 << output2;
                }
                {
                    auto input1 = splitDoubleArrayIntoFrames(inputValuesBadRankMid_0_30, frameSize);
                    auto output1 = splitDoubleArrayIntoFrames(outputNoFilter_BadRankMid_0_30, frameSize);
                    auto input2 = splitDoubleArrayIntoFrames(inputValuesBadRankMid_0_30, 1);
                    auto output2 = splitDoubleArrayIntoFrames(outputNoFilter_BadRankMid_0_30, 1);
                    QTest::addRow("Length1BadMidNoPassthrough_%d_%d", filterType, frameSize)
                        << filterType << filterLength << startImage << maxJump << maxJumpAddOn << false
                        << input1 << output1 << input2 << output2;
                }
            }
        }
    }
    /////////////////////////////////////////////////////////////////////////////////////
    // Test max jump with length 1
    /////////////////////////////////////////////////////////////////////////////////////
    {
        double maxJump = 3.0;
        int filterLength = 1;
        int filterType = 0; //doesn't really matter, length 1 doesn't filter

        startImage = 1;
        for (auto label : {"inc", "dec"})
        {
            auto& inputValues = (std::string(label) == "inc") ? inputValues_0_30 : inputValues_30_0;
            for (int frameSize : frameSizesToTest)
            {
                auto input1 = splitDoubleArrayIntoFrames(inputValues, frameSize);
                auto output1 = splitDoubleArrayIntoFrames(inputValues, frameSize);
                auto input2 = splitDoubleArrayIntoFrames(inputValues, 1);
                auto output2 = splitDoubleArrayIntoFrames(inputValues, 1);
                QTest::addRow("Length1_MaxJump_%s_%d", label, frameSize)
                    << filterType << filterLength << startImage << maxJump << maxJumpAddOn << passthrough
                    << input1 << output1 << input2 << output2;
            }
        }

        // test max jump add on
        maxJump = 0.1;

        Doublearray outputValues_0_30_NoAddOn(30, inputValues_0_30.getData().front(), 127);
        outputValues_0_30_NoAddOn.getRank()[0] = 255;
        Doublearray outputValues_30_0NoAddOn(30, inputValues_30_0.getData().front(), 127);
        outputValues_30_0NoAddOn.getRank()[0] = 255;

        Doublearray outputValues_0_30_AddOn(30, inputValues_0_30.getData().front(), 127);
        outputValues_0_30_AddOn.getRank()[0] = 255;
        Doublearray outputValues_30_0_AddOn(30, inputValues_30_0.getData().front(), 127);
        outputValues_30_0_AddOn.getRank()[0] = 255;
        for (int i = 1; i < 30; i++)
        {
            outputValues_0_30_AddOn.getData()[i] = outputValues_0_30_AddOn.getData()[i - 1] + maxJump;
            outputValues_30_0_AddOn.getData()[i] = outputValues_30_0_AddOn.getData()[i - 1] - maxJump;
        }
        for (auto label : {"inc", "dec"})
        {
            auto& inputValues = (std::string(label) == "inc") ? inputValues_0_30 : inputValues_30_0;
            auto& outputValuesNoAddOn = (std::string(label) == "inc") ? outputValues_0_30_NoAddOn : outputValues_30_0NoAddOn;
            auto& outputValuesAddOn = (std::string(label) == "inc") ? outputValues_0_30_AddOn : outputValues_30_0_AddOn;
            for (int frameSize : frameSizesToTest)
            {
                auto input1 = splitDoubleArrayIntoFrames(inputValues, frameSize);
                auto input2 = splitDoubleArrayIntoFrames(inputValues, 1);

                auto outputNoAddOn1 = splitDoubleArrayIntoFrames(outputValuesNoAddOn, frameSize);
                auto outputNoAddOn2 = splitDoubleArrayIntoFrames(outputValuesNoAddOn, 1);
                QTest::addRow("Length1_MaxJumpSmall_%s_%d_%d_NoAddOn", label, filterType, frameSize)
                    << filterType << filterLength << startImage << maxJump << false << passthrough
                    << input1 << outputNoAddOn1 << input2 << outputNoAddOn2;

                auto outputAddOn1 = splitDoubleArrayIntoFrames(outputValuesAddOn, frameSize);
                auto outputAddOn2 = splitDoubleArrayIntoFrames(outputValuesAddOn, 1);
                QTest::addRow("Length1_MaxJumpSmall_%s_%d_%d_AddOn", label, filterType, frameSize)
                    << filterType << filterLength << startImage << maxJump << true << passthrough
                    << input1 << outputAddOn1 << input2 << outputAddOn2;
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Test different filter types with a big start image (output equal to input, even with maxJump)
    ////////////////////////////////////////////////////////////////////////////////////////////////
    {
        int filterLength = 10;

        startImage = 100;
        double maxJump = 0.1;
        for (int filterType = 0; filterType <= 3; filterType++)
        {
            for (int frameSize : frameSizesToTest)
            {
                auto input1 = splitDoubleArrayIntoFrames(inputValues_30_0, frameSize);
                auto output1 = splitDoubleArrayIntoFrames(inputValues_30_0, frameSize);
                auto input2 = splitDoubleArrayIntoFrames(inputValues_30_0, 1);
                auto output2 = splitDoubleArrayIntoFrames(inputValues_30_0, 1);
                QTest::addRow("BigStartImage_%d_%d", filterType, frameSize)
                    << filterType << filterLength << startImage << maxJump << maxJumpAddOn << passthrough
                    << input1 << output1 << input2 << output2;

                // now test with bad rank in the middle

                input1 = splitDoubleArrayIntoFrames(inputValuesBadRankMid_0_30, frameSize);
                output1 = splitDoubleArrayIntoFrames(outputNoFilter_BadRankMid_0_30, frameSize);
                input2 = splitDoubleArrayIntoFrames(inputValuesBadRankMid_0_30, 1);
                output2 = splitDoubleArrayIntoFrames(outputNoFilter_BadRankMid_0_30, 1);
                QTest::addRow("BigStartImageBadRank_%d_%d", filterType, frameSize)
                    << filterType << filterLength << startImage << maxJump << maxJumpAddOn << passthrough
                    << input1 << output1 << input2 << output2;
            }
        }
    }
}

void TemporalLowPassTest::testProceed()
{
    bool verbose = false;

    TemporalLowPass filterSingleInput;
    auto outPipe1 = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray>*>(filterSingleInput.findPipe("Value"));
    QVERIFY(outPipe1);
    DummyInput dummyInput;
    QVERIFY(filterSingleInput.connectPipe(&dummyInput.m_pipeValueSingle, 0));
    DummyFilter dummyFilter1;
    QVERIFY(dummyFilter1.connectPipe(outPipe1, 0));

    TemporalLowPass2 filterDoubleInput;
    auto outPipe1_2 = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray>*>(filterDoubleInput.findPipe("Value1"));
    QVERIFY(outPipe1_2);
    auto outPipe2_2 = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray>*>(filterDoubleInput.findPipe("Value2"));
    QVERIFY(outPipe2_2);

    dummyInput.m_pipeValue1.setTag("input1");
    dummyInput.m_pipeValue2.setTag("input2");
    QVERIFY(filterDoubleInput.connectPipe(&dummyInput.m_pipeValue1, 1));
    QVERIFY(filterDoubleInput.connectPipe(&dummyInput.m_pipeValue2, 1));
    DummyFilter dummyFilter2;
    QVERIFY(dummyFilter2.connectPipe(outPipe1_2, 1));
    QVERIFY(dummyFilter2.connectPipe(outPipe2_2, 1));


    QFETCH(int, parameter_lowpassType);
    QFETCH(int, parameter_filterLength);
    QFETCH(int, parameter_startImage);
    QFETCH(double, parameter_maxJump);
    QFETCH(bool, parameter_maxJumpAddOn);
    QFETCH(bool, parameter_passthrough);

    for (auto * parameters : {&filterSingleInput.getParameters(), &filterDoubleInput.getParameters()})
    {
        parameters->update(std::string("LowPassType"), fliplib::Parameter::TYPE_int, parameter_lowpassType);
        parameters->update(std::string("FilterLength"), fliplib::Parameter::TYPE_UInt32, parameter_filterLength);
        parameters->update(std::string("StartImage"), fliplib::Parameter::TYPE_UInt32, parameter_startImage);
        parameters->update(std::string("MaxJump"), fliplib::Parameter::TYPE_double, parameter_maxJump);
        parameters->update(std::string("MaxJumpAddOn"), fliplib::Parameter::TYPE_bool, parameter_maxJumpAddOn);
        parameters->update(std::string("PassThroughBadRank"), fliplib::Parameter::TYPE_bool, parameter_passthrough);
    }
    filterSingleInput.setParameter();
    filterDoubleInput.setParameter();

    QFETCH(std::vector<Doublearray>, input1);
    QFETCH(std::vector<Doublearray>, expectedOutput1);
    QFETCH(std::vector<Doublearray>, input2);
    QFETCH(std::vector<Doublearray>, expectedOutput2);
    QVERIFY2(input1.size() == expectedOutput1.size()
    && input2.size() == expectedOutput2.size()
    && input1.size() <= input2.size(), "Test data badly initialized");
    for (auto imgNumber = 0u; imgNumber < input1.size(); imgNumber++)
    {
        dummyInput.fillDataAndSignal(imgNumber, input1[imgNumber], input2[imgNumber]);

        auto result1 = outPipe1->read(imgNumber).ref();
        auto result1_2 = outPipe1_2->read(imgNumber).ref();
        auto result2_2 = outPipe2_2->read(imgNumber).ref();
        if (verbose)
        {
            auto printData = [&](QString msg, bool always, const Doublearray & input, const Doublearray& output, const Doublearray& expectedOutput)
            {
                if (always || ( output.getData() != expectedOutput.getData() || output.getRank() != expectedOutput.getRank()))
                {
                    qDebug() << msg;
                    for (int i = 0; i < int(output.size()); i++)
                    {
                        qDebug() << imgNumber << " - " << i << ") "
                                    << "input " << input.getData().at(i) << "[" << input.getRank().at(i) << "] "
                                    << "result " << output.getData().at(i) << " [" << output.getRank().at(i) << "] "
                                    << expectedOutput.getData().at(i) << " [" << expectedOutput.getRank().at(i) << "] ";
                    }
                }
            };
            printData("Result 1 ", false, input1[imgNumber], result1, expectedOutput1[imgNumber]);
            printData("Result 1/2 ", false, input1[imgNumber], result1_2, expectedOutput1[imgNumber]);
            printData("Result 2/2 ", false, input2[imgNumber], result2_2, expectedOutput2[imgNumber]);

        }
        QCOMPARE(result1.getData(), expectedOutput1[imgNumber].getData());
        QCOMPARE(result1.getRank(), expectedOutput1[imgNumber].getRank());

        QCOMPARE(result1_2.getData(), expectedOutput1[imgNumber].getData());
        QCOMPARE(result1_2.getRank(), expectedOutput1[imgNumber].getRank());
        QCOMPARE(result2_2.getData(), expectedOutput2[imgNumber].getData());
        QCOMPARE(result2_2.getRank(), expectedOutput2[imgNumber].getRank());
    }
}

QTEST_GUILESS_MAIN(TemporalLowPassTest)
#include "testTemporalLowPass.moc"
