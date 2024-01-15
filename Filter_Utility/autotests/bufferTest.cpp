
#include <QTest>

#include "geo/geo.h"
#include "../bufferRecorder.h"
#include "../bufferPlayer.h"
#include "filter/buffer.h"
#include <fliplib/NullSourceFilter.h>
#include <filter/productData.h>
#include <filter/armStates.h>
#include <tuple>

namespace
{

struct TestPosition
{
    double pos;
    int rankPos;
};

struct TestData
{
    double data;
    int rankData;
};

struct DummyRecorderInput
{
    fliplib::NullSourceFilter sourceFilter;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeInData{&sourceFilter, "Data"};
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeInPosition{&sourceFilter, "Position"};


    DummyRecorderInput()
    {
        pipeInData.setTag("data");
        pipeInPosition.setTag("pos");
    }

    bool connectToFilter(fliplib::BaseFilter* pFilter)
    {
        int group = 1;
        //connect pipes
        bool ok = pFilter->connectPipe(&(pipeInData), group);
        ok &= pFilter->connectPipe(&(pipeInPosition), group);
        return ok;
    }

    void fillDataAndSignal(int imageNumber, TestPosition inputPosition, TestData inputData)
    {
        using namespace precitec::interface;
        using namespace precitec::geo2d;

        GeoDoublearray geoDoubleArray;
        GeoDoublearray geoDoublePos;

        ImageContext context;
        context.setImageNumber(imageNumber);

        precitec::geo2d::Doublearray dataArray;
        dataArray.getData() = std::vector<double>{inputData.data};
        dataArray.getRank() = std::vector<int>(1, inputData.rankData);

        auto geoDataIn = GeoDoublearray{context, dataArray, ResultType::AnalysisOK, Limit};
        auto geoPosIn = GeoDoublearray{
            context,
            Doublearray(1, inputPosition.pos, inputPosition.rankPos),
            ResultType::AnalysisOK, Limit
        };
        pipeInPosition.signal(geoPosIn);
        pipeInData.signal(geoDataIn);
    }
};

struct DummyPlayerInput
{
    fliplib::NullSourceFilter sourceFilter;
    fliplib::SynchronePipe<precitec::interface::ImageFrame> pipeInImage{ &sourceFilter, "Image"};
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeInPosition{ &sourceFilter, "Position"};

    DummyPlayerInput()
    {
        pipeInImage.setTag("image");
        pipeInPosition.setTag("pos");
    }

    bool connectToFilter(fliplib::BaseFilter * pFilter)
    {
        int group = 1;
        //connect pipes
        bool ok = pFilter->connectPipe(&(pipeInImage), group);
        ok &= pFilter->connectPipe(&(pipeInPosition), group);
        return ok;
    }

    void fillDataAndSignal(int imageNumber, TestPosition inputPosition)
    {
        using namespace precitec::interface;
        using namespace precitec::geo2d;

        ImageContext context;
        context.setImageNumber(imageNumber);

        auto geoImageIn = ImageFrame
        {
            context,
            precitec::image::genModuloPattern(Size2D{100,100}, 50),
            ResultType::AnalysisOK
        };

        auto geoPosIn = GeoDoublearray
        {
            context,
            Doublearray(1, inputPosition.pos, inputPosition.rankPos),
            ResultType::AnalysisOK, Limit
        };

        pipeInPosition.signal(geoPosIn);
        pipeInImage.signal(geoImageIn);
    }
};
}

using namespace precitec::filter;
using posPerSeamSerie_t = std::map<int, std::map<int, std::vector<TestPosition>>>;
using dataPerSeamSerie_t = std::map<int, std::map<int, std::vector<TestData>>>;

Q_DECLARE_METATYPE(posPerSeamSerie_t)
Q_DECLARE_METATYPE(dataPerSeamSerie_t)

class BufferTest: public QObject
{
    Q_OBJECT

private:
    static void clearBufferSingleton()
    {
        BufferSingleton::getInstanceData().m_oArrays.clear();
        BufferSingleton::getInstancePos().m_oArrays.clear();
    };

private Q_SLOTS:

    void init();
    void testRecorderCtor();
    void testPlayerCtor();
    void testRecorderArm();
    void testProceed_data();
    void testProceed();
};

void BufferTest::init()
{
    clearBufferSingleton();
}

void BufferTest::testRecorderCtor()
{
    BufferRecorder filter;
    QCOMPARE(filter.name(), std::string("BufferRecorder"));
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);

    QVERIFY(filter.getParameters().exists(std::string("Slot")));

    QCOMPARE(filter.getParameters().findParameter(std::string("Slot")).getType(), fliplib::Parameter::TYPE_uint);
    QCOMPARE(filter.getParameters().findParameter(std::string("Slot")).getValue().convert<unsigned int>(), 1u);
}

void BufferTest::testPlayerCtor()
{
    BufferPlayer filter;
    QCOMPARE(filter.name(), std::string("BufferPlayer"));
    QVERIFY(filter.findPipe("Data") != nullptr);
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);
    QVERIFY(filter.getParameters().exists(std::string("Slot")));

    QCOMPARE(filter.getParameters().findParameter(std::string("Slot")).getType(), fliplib::Parameter::TYPE_uint);
    QCOMPARE(filter.getParameters().findParameter(std::string("Slot")).getValue().convert<unsigned int>(), 1u);

    for (auto intParameter: {"DataOffset", "SeamOffset", "SeamSeriesOffset"})
    {
        QVERIFY(filter.getParameters().exists(std::string(intParameter)));

        QCOMPARE(filter.getParameters().findParameter(std::string(intParameter)).getType(), fliplib::Parameter::TYPE_int);
        QCOMPARE(filter.getParameters().findParameter(std::string(intParameter)).getValue().convert<int>(), 0);
    }
}

void BufferTest::testRecorderArm()
{
    int seamSeries = 0;
    int seam = 0;
    int velocity = 30;
    int triggerDelta = 1;
    int numTrigger = 10;

    auto externalProductData = precitec::analyzer::ProductData {
        seamSeries, seam, velocity, triggerDelta, numTrigger
    };

    int expectedBufferSize = 0;
    for (unsigned int slot : {0,1})
    {
        BufferRecorder filter;
        filter.setExternalData(&externalProductData);

        auto & data = BufferSingleton::getInstanceData();
        auto & pos = BufferSingleton::getInstancePos();

        QVERIFY(!data.exists(slot, seamSeries, seam));
        QVERIFY(!pos.exists(slot, seamSeries, seam));

        filter.getParameters().update(std::string("Slot"), fliplib::Parameter::TYPE_uint, slot);
        filter.setParameter();

        filter.arm(ArmState::eSeamStart);
        expectedBufferSize++;


        QVERIFY(data.exists(slot, seamSeries, seam));
        QVERIFY(pos.exists(slot, seamSeries, seam));

        for (auto & armState : {ArmState::eSeamIntervalStart, ArmState::eSeamIntervalChange, ArmState::eSeamIntervalChange, ArmState::eSeamEnd, ArmState::eSeamSeriesStart, ArmState::eSeamSeriesStart})
        {
            filter.arm(armState);
            QVERIFY(data.exists(slot, seamSeries, seam));
            QVERIFY(pos.exists(slot, seamSeries, seam));
        }
    }
    auto& data = BufferSingleton::getInstanceData();
    auto& pos = BufferSingleton::getInstancePos();
    QCOMPARE(data.m_oArrays.size(), expectedBufferSize);
    QCOMPARE(pos.m_oArrays.size(), expectedBufferSize);
}

void BufferTest::testProceed_data()
{
    QTest::addColumn<int>("numTrigger");
    QTest::addColumn<posPerSeamSerie_t>("recorderInputPosition");
    QTest::addColumn<dataPerSeamSerie_t>("recorderInputData");
    QTest::addColumn<int>("playerSeamSeriesOffset");
    QTest::addColumn<int>("playerSeamOffset");
    QTest::addColumn<posPerSeamSerie_t>("playerInputPosition");
    QTest::addColumn<dataPerSeamSerie_t>("playerExpectedData");

    enum TestSerie {eSeamSeries0 = 0, eSeamSeries1, eSeamSeries2, eSeamSeries3};
    enum TestSeam {eSeam0 = 0, eSeam1, eSeam2, eSeam3}; // for readibility

    const TestData invalidResult{{}, eRankMin};
    {
        int numTrigger = 10;

        posPerSeamSerie_t recorderInputPosition;
        dataPerSeamSerie_t recorderInputData;

        for (int imageNumber = 0; imageNumber < numTrigger; ++imageNumber)
        {
            recorderInputPosition[eSeamSeries0][eSeam0].push_back(TestPosition{double(imageNumber), eRankMax});
            recorderInputPosition[eSeamSeries1][eSeam0].push_back(TestPosition{double(imageNumber + 100), eRankMax});
            TestData data;
            data.data = 1.1 * imageNumber;
            data.rankData = 255;
            recorderInputData[eSeamSeries0][eSeam0].push_back(data);
            recorderInputData[eSeamSeries1][eSeam0].push_back(data);
        };

        for (int imageNumber = 0; imageNumber < numTrigger; ++imageNumber)
        {
            recorderInputPosition[eSeamSeries0][eSeam1].push_back(TestPosition{double(imageNumber), eRankMax});
            recorderInputPosition[eSeamSeries1][eSeam1].push_back(TestPosition{double(imageNumber + 100), eRankMax});
            TestData data;
            data.data = 0.2 * imageNumber;
            data.rankData = 255;
            recorderInputData[eSeamSeries0][eSeam1].push_back(data);
            recorderInputData[eSeamSeries1][eSeam1].push_back(data);
        };

        {
            int playerSeamSeriesOffset = -2;
            int playerSeamOffset = -2;

            posPerSeamSerie_t playerInputPosition;
            dataPerSeamSerie_t playerExpectedData;
            playerInputPosition[eSeamSeries2][eSeam2] = recorderInputPosition[eSeamSeries0][eSeam0];
            playerInputPosition[eSeamSeries2][eSeam3] = recorderInputPosition[eSeamSeries0][eSeam1];
            playerInputPosition[eSeamSeries3][eSeam2] = recorderInputPosition[eSeamSeries1][eSeam0];
            playerInputPosition[eSeamSeries3][eSeam3] = recorderInputPosition[eSeamSeries1][eSeam1];
            playerExpectedData[eSeamSeries2][eSeam2] = recorderInputData[eSeamSeries0][eSeam0];
            playerExpectedData[eSeamSeries2][eSeam3] = recorderInputData[eSeamSeries0][eSeam1];
            playerExpectedData[eSeamSeries3][eSeam2] = recorderInputData[eSeamSeries1][eSeam0];
            playerExpectedData[eSeamSeries3][eSeam3] = recorderInputData[eSeamSeries1][eSeam1];

            QTest::newRow("ExactPositions")
                << numTrigger
                << recorderInputPosition
                << recorderInputData
                << playerSeamSeriesOffset
                << playerSeamOffset
                << playerInputPosition
                << playerExpectedData;
        }

        {
            int playerSeamSerieOffset = -2;
            int playerSeamOffset = -2;

            posPerSeamSerie_t playerInputPosition;
            dataPerSeamSerie_t playerExpectedData;
            //seam 0, image 5 of the recorder has the same encoder position as seam 0, image 0 of the player
            playerInputPosition[eSeamSeries2][eSeam2] = {recorderInputPosition[eSeamSeries0][eSeam0][5]};
            playerExpectedData[eSeamSeries2][eSeam2] = {recorderInputData[eSeamSeries0][eSeam0][5]};
            playerInputPosition[eSeamSeries3][eSeam2] = {recorderInputPosition[eSeamSeries1][eSeam0][5]};
            playerExpectedData[eSeamSeries3][eSeam2] = {recorderInputData[eSeamSeries1][eSeam0][5]};
            auto& data = recorderInputPosition[eSeamSeries0][eSeam1][5];
            data.pos -= 0.5;
            data = recorderInputPosition[eSeamSeries1][eSeam1][5];
            data.pos -= 0.5;
            //seam 0, image 5 of the recorder has "almost" the same encoder position as seam 0, image 0 of the player (nearest neghbour interpolation)
            playerInputPosition[eSeamSeries2][eSeam3] = {recorderInputPosition[eSeamSeries0][eSeam1][5]};
            playerExpectedData[eSeamSeries2][eSeam3] = {recorderInputData[eSeamSeries0][eSeam1][5]};
            playerInputPosition[eSeamSeries3][eSeam3] = {recorderInputPosition[eSeamSeries1][eSeam1][5]};
            playerExpectedData[eSeamSeries3][eSeam3] = {recorderInputData[eSeamSeries1][eSeam1][5]};

            QTest::newRow("PositionWithOffset")
                << numTrigger
                << recorderInputPosition
                << recorderInputData
                << playerSeamSerieOffset
                << playerSeamOffset
                << playerInputPosition
                << playerExpectedData;
        }

    {
        int numTrigger = 10;
        posPerSeamSerie_t recorderInputPosition;
        dataPerSeamSerie_t recorderInputData;

        for (int imageNumber = 0; imageNumber < numTrigger; ++imageNumber)
        {
            recorderInputPosition[eSeamSeries0][eSeam0].push_back(TestPosition{1.0, eRankMax});
            recorderInputPosition[eSeamSeries1][eSeam0].push_back(TestPosition{1.0, eRankMax});
            TestData data;
            data.data = 10;
            data.rankData = 255;
            recorderInputData[eSeamSeries0][eSeam0].push_back(data);
            data.data = 15;
            recorderInputData[eSeamSeries1][eSeam0].push_back(data);
        };

        int playerSeamSeriesOffset = -2;
        int playerSeamOffset = -2;

        posPerSeamSerie_t playerInputPosition;
        dataPerSeamSerie_t playerExpectedData;
        playerInputPosition[eSeamSeries2][eSeam2] = std::vector<TestPosition>(2, TestPosition{1.0, eRankMax});
        auto data = recorderInputData[eSeamSeries0][eSeam0].back(); //all the data in buffer have the same position, the player will find the last element
        playerExpectedData[eSeamSeries2][eSeam2] = std::vector<TestData>(2, data);
        playerInputPosition[eSeamSeries3][eSeam2] = std::vector<TestPosition>(2, TestPosition{1.0, eRankMax});
        data = recorderInputData[eSeamSeries1][eSeam0].back(); //all the data in buffer have the same position, the player will find the last element
        playerExpectedData[eSeamSeries3][eSeam2] = std::vector<TestData>(2, data);

        QTest::newRow("OverwrittenPosition")
            << numTrigger
            << recorderInputPosition
            << recorderInputData
            << playerSeamSeriesOffset
            << playerSeamOffset
            << playerInputPosition
            << playerExpectedData;
    }
    {
        int numTrigger = 5;
        posPerSeamSerie_t recorderInputPosition;
        dataPerSeamSerie_t recorderInputData;

        for (int imageNumber = 0; imageNumber < numTrigger; ++imageNumber)
        {
            recorderInputPosition[eSeamSeries0][eSeam0].push_back(TestPosition{double(imageNumber), eRankMax});
            TestData data;
            data.data = numTrigger;
            data.rankData = 255;
            recorderInputData[eSeamSeries0][eSeam0].push_back(data);
        };
        int playerSeamSeriesOffset = -2;
        int playerSeamOffset = -2;

        posPerSeamSerie_t playerInputPosition;
        dataPerSeamSerie_t playerExpectedData;

        for (int imageNumber = 0; imageNumber < numTrigger; ++imageNumber)
        {
            auto pos = recorderInputPosition[eSeamSeries0][eSeam0][imageNumber];
            pos.pos *= 10000;
            playerInputPosition[eSeamSeries2][eSeam2].push_back(pos);
            if (imageNumber == 0)
            {
                //image 0 must give the same result
                playerExpectedData[eSeamSeries2][eSeam2].push_back(recorderInputData[eSeamSeries0][eSeam0][0]);
            }
            else
            {
                //for the next images a different position is used in order not to trigger any result
                playerExpectedData[eSeamSeries2][eSeam2].push_back(invalidResult);
            }
        }

        QTest::newRow("ResultOnlyInImage0")
            << numTrigger
            << recorderInputPosition
            << recorderInputData
            << playerSeamSeriesOffset
            << playerSeamOffset
            << playerInputPosition
            << playerExpectedData;
    }
    }
}

class DummyFilter: public fliplib::TransformFilter
{
public:
    DummyFilter() : fliplib::TransformFilter("dummy") {}
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

    void resetProceedCalled()
    {
        m_proceedCalled = false;
    }


private:
    bool m_proceedCalled = false;
};

void BufferTest::testProceed()
{
    unsigned int slot = 2;
    int velocity = 30;
    int triggerDelta = 1;

    QFETCH(int, numTrigger);
    QFETCH(posPerSeamSerie_t, recorderInputPosition);
    QFETCH(dataPerSeamSerie_t, recorderInputData);

    //first seams: recorder filter
    {
        BufferRecorder filterRecorder;
        DummyRecorderInput input;
        DummyFilter dummyFilter;

        QVERIFY(input.connectToFilter(&filterRecorder));

        for (auto& testSeamSerieData: recorderInputData)
        {
            auto& seamSerie = testSeamSerieData.first;

            for (auto& testSeamData: testSeamSerieData.second)
            {
                auto& seam = testSeamData.first;
                auto& dataVector = testSeamData.second;

                //verify that the data saved in proceed_data is consistent
                auto itPos = recorderInputPosition.find(seamSerie);
                QVERIFY2(itPos != recorderInputPosition.end(), "error in test data");
                auto & posVector = itPos->second.at(seam);
                QVERIFY2(dataVector.size() == posVector.size(), "error in test data");


                auto externalProductData = precitec::analyzer::ProductData {
                    seamSerie, seam, velocity, triggerDelta, numTrigger
                };
                filterRecorder.setExternalData(&externalProductData);
                filterRecorder.getParameters().update(std::string("Slot"), fliplib::Parameter::TYPE_uint, slot);
                filterRecorder.setParameter();
                filterRecorder.arm(precitec::filter::ArmState::eSeamStart);

                for (int imageNumber = 0, end = dataVector.size(); imageNumber < end; imageNumber++)
                {
                    auto& rData = dataVector[imageNumber];
                    auto& rPos = posVector[imageNumber];
                    QCOMPARE(dummyFilter.isProceedCalled(), false);
                    input.fillDataAndSignal(imageNumber, rPos, rData);
                    //QCOMPARE(dummyFilter.isProceedCalled(), true);
                    dummyFilter.resetProceedCalled();
                }
            }
        }
    }

    //QVERIFY(verifySizeContourBufferSingleton(recorderInputData.size()));

    //next seams: buffer player filter
    {
        QFETCH(int, playerSeamSeriesOffset);
        QFETCH(int, playerSeamOffset);
        QFETCH(posPerSeamSerie_t, playerInputPosition);
        QFETCH(dataPerSeamSerie_t, playerExpectedData);

        BufferPlayer filter;
        DummyPlayerInput input;
        DummyFilter dummyFilter;

        QVERIFY(input.connectToFilter(&filter));
        QVERIFY(dummyFilter.connectPipe(filter.findPipe("Data"), 0));
        auto outPipe = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("Data"));
        QVERIFY(outPipe);

        for (const auto& testSeamSeriesData: playerExpectedData)
        {
            auto& seamSerie = testSeamSeriesData.first;
            for (const auto& testSeamData: testSeamSeriesData.second)
            {
                auto seam = testSeamData.first;
                auto& expectedDataVector = testSeamData.second;
                //verify that the data saved in proceed_data is consistent
                auto itPos = (*playerInputPosition.find(seamSerie)).second.find(seam);
                auto end = playerInputPosition.at(seamSerie).end();
                QVERIFY2(itPos != end, "error in test data");
                auto & posVector = itPos->second;
                QVERIFY2(expectedDataVector.size() == posVector.size(), "error in test data");


                auto oExternalProductData = precitec::analyzer::ProductData {
                    seamSerie, seam, velocity, triggerDelta, numTrigger
                };
                filter.setExternalData(&oExternalProductData);

                filter.getParameters().update(std::string("Slot"), fliplib::Parameter::TYPE_uint, slot);
                filter.getParameters().update(std::string("SeamSeriesOffset"), fliplib::Parameter::TYPE_int, playerSeamSeriesOffset);
                filter.getParameters().update(std::string("SeamOffset"), fliplib::Parameter::TYPE_int, playerSeamOffset);
                filter.getParameters().update(std::string("DataOffset"), fliplib::Parameter::TYPE_int, 0);
                filter.setParameter();

                filter.arm(precitec::filter::ArmState::eSeamStart);


                for (std::size_t imageNumber = 0; imageNumber < expectedDataVector.size(); ++imageNumber)
                {
                    auto& inputPos = posVector[imageNumber];
                    input.fillDataAndSignal(imageNumber, inputPos);
                    const auto result = outPipe->read(imageNumber);
                    QCOMPARE(result.ref().size(), 1);
                    dummyFilter.resetProceedCalled();
                    auto firstResult = result.ref()[0];
                    auto& expectedData = expectedDataVector[imageNumber];
                    QCOMPARE(std::get<0>(firstResult), expectedData.data);
                    dummyFilter.resetProceedCalled();
                }
            }
        }
    }
}

QTEST_GUILESS_MAIN(BufferTest)
#include "bufferTest.moc"
