#include <QTest>

#include "../ContourBufferRecorder.h"
#include "../ContourBufferPlayer.h"
#include "filter/buffer.h"
#include "geo/geo.h"

#include <fliplib/NullSourceFilter.h>
#include <fliplib/SinkFilter.h>
#include <filter/sensorFilterInterface.h>
#include <filter/productData.h>
#include <filter/armStates.h>


namespace {

    struct TestPosition
    {        
        double pos;
        int rankPos;
    };

    struct TestData
    {
        std::vector<precitec::geo2d::DPoint> points;
        int rankPoints;
    };

    struct DummyRecorderInput
    {
        fliplib::NullSourceFilter sourceFilter;
        fliplib::SynchronePipe<precitec::interface::GeoVecAnnotatedDPointarray> pipeInContour{&sourceFilter, "contour"};
        fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeInPosition{&sourceFilter, "position"};

        DummyRecorderInput()
        {
            pipeInContour.setTag("data");
            pipeInPosition.setTag("pos");
        }

        bool connectToFilter(fliplib::BaseFilter * pFilter)
        {
            int group =1;
            //connect pipes
            bool ok = pFilter->connectPipe(&(pipeInContour), group);
            ok &= pFilter->connectPipe(&(pipeInPosition), group);
            return ok;
        }

        void fillDataAndSignal(int imageNumber, TestPosition inputPosition, TestData inputData)
        {
            using namespace precitec::interface;
            using namespace precitec::geo2d;

            ImageContext context;
            context.setImageNumber(imageNumber);

            precitec::geo2d::AnnotatedDPointarray oPointArray;
            oPointArray.getData() = inputData.points;
            oPointArray.getRank() = std::vector<int> (inputData.points.size(), inputData.rankPoints);
            auto geoContourIn = GeoVecAnnotatedDPointarray {context, {oPointArray}, ResultType::AnalysisOK, Limit};

            auto geoPosIn = precitec::interface::GeoDoublearray
            {
                context, 
                Doublearray(1,inputPosition.pos,inputPosition.rankPos), 
                ResultType::AnalysisOK, Limit
            };
            pipeInPosition.signal(geoPosIn);
            pipeInContour.signal(geoContourIn);
        }
    };

    struct DummyPlayerInput
    {
        fliplib::NullSourceFilter sourceFilter;
        fliplib::SynchronePipe<precitec::interface::ImageFrame> pipeInImage{ &sourceFilter, "contour"};
        fliplib::SynchronePipe<precitec::interface::GeoDoublearray> pipeInPosition{ &sourceFilter, "absolute_position_x"};

        DummyPlayerInput()
        {
            pipeInImage.setTag("image");
            pipeInPosition.setTag("pos");
        }

        bool connectToFilter(fliplib::BaseFilter * pFilter)
        {
            int group =1;
            //connect  pipes
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

            auto geoImageIn = ImageFrame {context, precitec::image::genModuloPattern(Size2D{100,100},50), ResultType::AnalysisOK};
            
            auto geoPosIn = precitec::interface::GeoDoublearray
            {
                context, 
                Doublearray(1, inputPosition.pos, inputPosition.rankPos),
                ResultType::AnalysisOK, Limit
            };
            pipeInPosition.signal(geoPosIn);
            pipeInImage.signal(geoImageIn);
        }
    };

    class DummyFilter : public fliplib::BaseFilter
    {
    public:
        DummyFilter(): fliplib::BaseFilter("dummy"){}
        void proceed(const void *sender, fliplib::PipeEventArgs &event) override
        {
            Q_UNUSED(sender)
            Q_UNUSED(event)
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
}


using namespace precitec::filter;

using posPerSeamSeries_t = std::map<int, std::map<int,std::vector<TestPosition>>>;
using dataPerSeamSeries_t = std::map<int, std::map<int,std::vector<TestData>>>;

Q_DECLARE_METATYPE(posPerSeamSeries_t)
Q_DECLARE_METATYPE(dataPerSeamSeries_t)

class ContourBufferTest: public QObject
{
    Q_OBJECT

private:
    static void clearBufferSingleton()
    {
        BufferSingleton::getInstanceData().m_oArrays.clear();
        BufferSingleton::getInstancePos().m_oArrays.clear();
    };
    static void clearContourBufferSingleton()
    {
        ContourBufferSingleton::getInstanceData().m_oArrays.clear();
        ContourBufferSingleton::getInstancePos().m_oArrays.clear();
    };
    static bool verifySizeContourBufferSingleton(unsigned int expectedSize)
    {
        return ContourBufferSingleton::getInstanceData().m_oArrays.size() == expectedSize &&
        ContourBufferSingleton::getInstancePos().m_oArrays.size() == expectedSize;
    }
    
private Q_SLOTS:
    
    void init();
    void testRecorderCtor();
    void testPlayerCtor();
    void testRecorderArm();
    void testProceed_data();
    void testProceed();
};


using namespace precitec::filter;
void ContourBufferTest::init()
{
    // clear the the arrays in the singleton before each test
    clearBufferSingleton();
    clearContourBufferSingleton();
    QVERIFY(verifySizeContourBufferSingleton(0));
}

void ContourBufferTest::testRecorderCtor()
{
    ContourBufferRecorder filter;
    QCOMPARE(filter.name(), std::string("ContourBufferRecorder"));
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);

    QVERIFY(filter.getParameters().exists(std::string("Slot")));

    QCOMPARE(filter.getParameters().findParameter(std::string("Slot")).getType(), fliplib::Parameter::TYPE_uint);
    QCOMPARE(filter.getParameters().findParameter(std::string("Slot")).getValue().convert<unsigned int>(), 1u);
}

void ContourBufferTest::testPlayerCtor()
{
    ContourBufferPlayer filter;
    QCOMPARE(filter.name(), std::string("ContourBufferPlayer"));
    QVERIFY(filter.findPipe("Data") != nullptr);
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);
    QVERIFY(filter.getParameters().exists(std::string("Slot")));

    QCOMPARE(filter.getParameters().findParameter(std::string("Slot")).getType(), fliplib::Parameter::TYPE_uint);
    QCOMPARE(filter.getParameters().findParameter(std::string("Slot")).getValue().convert<unsigned int>(), 1u);

    for (auto intParameter : {"DataOffset", "SeamOffset", "SeamSeriesOffset"})
    {
        QVERIFY(filter.getParameters().exists(std::string(intParameter)));
        QCOMPARE(filter.getParameters().findParameter(std::string(intParameter)).getType(), fliplib::Parameter::TYPE_int);
        QCOMPARE(filter.getParameters().findParameter(std::string(intParameter)).getValue().convert<int>(), 0);
    }
}

void ContourBufferTest::testRecorderArm()
{
    int seamSeries = 0;
    int seam = 0;
    int velocity = 30;
    int triggerDelta = 1;
    int numTrigger = 10;
    
    auto oExternalProductData = precitec::analyzer::ProductData {
        seamSeries, seam, velocity, triggerDelta, numTrigger
    };
    
    int expectedBufferSize = 0;
    for (unsigned int slot: {0,1})
    {
        ContourBufferRecorder filter;	
        filter.setExternalData(&oExternalProductData);

        auto& rData = ContourBufferSingleton::getInstanceData();
        auto& rPos = ContourBufferSingleton::getInstancePos();

        QVERIFY(!rData.exists(slot, seamSeries, seam));
        QVERIFY(!rPos.exists(slot, seamSeries, seam));
        QVERIFY(verifySizeContourBufferSingleton(expectedBufferSize));

        filter.getParameters().update(std::string("Slot"), fliplib::Parameter::TYPE_uint, slot);
        filter.setParameter();
        filter.arm(ArmState::eSeamStart);
        expectedBufferSize++;

        QVERIFY(rData.exists(slot,seamSeries,seam));
        QVERIFY(rPos.exists(slot,seamSeries,seam));
        QVERIFY(verifySizeContourBufferSingleton(expectedBufferSize));

        for (auto & armState : {ArmState::eSeamIntervalStart, ArmState::eSeamIntervalChange, ArmState::eSeamIntervalChange, ArmState::eSeamEnd, ArmState::eSeamSeriesStart, ArmState::eSeamSeriesStart})
        {
            filter.arm(armState);
            QVERIFY(rData.exists(slot,seamSeries,seam));
            QVERIFY(rPos.exists(slot,seamSeries,seam));
            QVERIFY(verifySizeContourBufferSingleton(expectedBufferSize));
        }
    }
}


void ContourBufferTest::testProceed_data()
{
    QTest::addColumn<int>("numTrigger");
    QTest::addColumn<posPerSeamSeries_t>("recorderInputPosition");
    QTest::addColumn<dataPerSeamSeries_t>("recorderInputData");
    QTest::addColumn<int>("playerSeamSeriesOffset");
    QTest::addColumn<int>("playerSeamOffset");
    QTest::addColumn<posPerSeamSeries_t>("playerInputPosition");
    QTest::addColumn<dataPerSeamSeries_t>("playerExpectedData");

    enum TestSeam {eSeam0 = 0, eSeam1, eSeam2, eSeam3}; // for readibility
    enum TestSeamSeries {eSeamSeries0 = 0, eSeamSeries1, eSeamSeries2, eSeamSeries3};
    const TestData invalidResult{{},eRankMin};

    {
        int numTrigger = 10;
        posPerSeamSeries_t recorderInputPosition;
        dataPerSeamSeries_t recorderInputData;

        for (int imageNumber = 0; imageNumber < numTrigger; ++imageNumber)
        {
            recorderInputPosition[eSeamSeries0][eSeam0].push_back(TestPosition{double(imageNumber), eRankMax});
            recorderInputPosition[eSeamSeries1][eSeam0].push_back(TestPosition{double(imageNumber + 100), eRankMax});
            TestData data;
            data.points = std::vector<precitec::geo2d::DPoint>(10, {0.1, 1.1*imageNumber});
            data.rankPoints = 255;
            recorderInputData[eSeamSeries0][eSeam0].push_back(data);
            data.points = std::vector<precitec::geo2d::DPoint>(10, {0.2, 1.2*imageNumber});
            recorderInputData[eSeamSeries1][eSeam0].push_back(data);
        };

        for (int imageNumber = 0; imageNumber < numTrigger; ++imageNumber)
        {
            recorderInputPosition[eSeamSeries0][eSeam1].push_back(TestPosition{double(imageNumber), eRankMax});
            recorderInputPosition[eSeamSeries1][eSeam1].push_back(TestPosition{double(imageNumber + 100), eRankMax});
            TestData data;
            data.points = std::vector<precitec::geo2d::DPoint>(5, {-0.2*imageNumber, 0.0});
            data.rankPoints = 255;
            recorderInputData[eSeamSeries0][eSeam1].push_back(data);
            data.points = std::vector<precitec::geo2d::DPoint>(5, {-0.3*imageNumber, 0.3});
            recorderInputData[eSeamSeries1][eSeam1].push_back(data);
        };

        {
            int playerSeamSeriesOffset = -2;
            int playerSeamOffset = -2;

            posPerSeamSeries_t playerInputPosition;
            dataPerSeamSeries_t playerExpectedData;
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
            int playerSeamSeriesOffset = -2;
            int playerSeamOffset = -2;

            posPerSeamSeries_t playerInputPosition;
            dataPerSeamSeries_t playerExpectedData;
            //seam 0, image 5 of the recorder has the same encoder position as seam 0, image 0 of the player 

            playerInputPosition[eSeamSeries2][eSeam2] = {recorderInputPosition[eSeamSeries0][eSeam0][5]};
            playerExpectedData[eSeamSeries2][eSeam2] = {recorderInputData[eSeamSeries0][eSeam0][5]};
            playerInputPosition[eSeamSeries3][eSeam2] = {recorderInputPosition[eSeamSeries1][eSeam0][5]};
            playerExpectedData[eSeamSeries3][eSeam2] = {recorderInputData[eSeamSeries1][eSeam0][5]};

            //seam 0, image 5 of the recorder has "almost" the same encoder position as seam 0, image 0 of the player (nearest neghbour interpolation)
            auto& data = recorderInputPosition[eSeamSeries0][eSeam1][5];
            data.pos -= 0.5;
            data = recorderInputPosition[eSeamSeries1][eSeam1][5];
            data.pos -= 0.5;
            playerInputPosition[eSeamSeries2][eSeam3] = {recorderInputPosition[eSeamSeries0][eSeam1][5]};
            playerExpectedData[eSeamSeries2][eSeam3] = {recorderInputData[eSeamSeries0][eSeam1][5]};
            playerInputPosition[eSeamSeries3][eSeam3] = {recorderInputPosition[eSeamSeries1][eSeam1][5]};
            playerExpectedData[eSeamSeries3][eSeam3] = {recorderInputData[eSeamSeries1][eSeam1][5]};

            QTest::newRow("PositionWithOffset")
                << numTrigger
                << recorderInputPosition
                << recorderInputData
                << playerSeamSeriesOffset
                << playerSeamOffset
                << playerInputPosition
                << playerExpectedData;
        }
    }
    {
        int numTrigger = 10;
        posPerSeamSeries_t recorderInputPosition;
        dataPerSeamSeries_t recorderInputData;

        for (int imageNumber = 0; imageNumber < numTrigger; ++imageNumber)
        {
            recorderInputPosition[eSeamSeries0][eSeam0].push_back(TestPosition{1.0, eRankMax});
            TestData data;
            data.points = std::vector<precitec::geo2d::DPoint>(10, {0.1, 1.1*imageNumber});
            data.rankPoints = 255;
            recorderInputData[eSeamSeries0][eSeam0].push_back(data);
        };
        int playerSeamSeriesOffset = -2;
        int playerSeamOffset = -2;

        posPerSeamSeries_t playerInputPosition;
        dataPerSeamSeries_t playerExpectedData;

        playerInputPosition[eSeamSeries2][eSeam2] = std::vector<TestPosition>(2, TestPosition{1.0, eRankMax});
        auto data = recorderInputData[eSeamSeries0][0].back(); //all the data in buffer have the same position, the player will find the last element
        playerExpectedData[eSeamSeries2][eSeam2] = std::vector<TestData>(2, data);

        QTest::newRow("OverwrittenPosition")
            << numTrigger
            << recorderInputPosition
            << recorderInputData
            << playerSeamSeriesOffset
            << playerSeamOffset
            << playerInputPosition
            << playerExpectedData;
    }
    // contour with variable length, starting with empty contour (0,1,2) - 
    {
        int numTrigger = 3;
        posPerSeamSeries_t recorderInputPosition;
        dataPerSeamSeries_t recorderInputData;

        for (int imageNumber = 0; imageNumber < numTrigger; ++imageNumber)
        {
            recorderInputPosition[eSeamSeries0][eSeam0].push_back(TestPosition{100.0*imageNumber, eRankMax});
            TestData data;
            data.points = std::vector<precitec::geo2d::DPoint>(numTrigger, {0.1, 1.1*imageNumber});
            data.rankPoints = 255;
            recorderInputData[eSeamSeries0][0].push_back(data);
        };
        int playerSeamSeriesOffset = -2;
        int playerSeamOffset = -2;

        posPerSeamSeries_t playerInputPosition;
        dataPerSeamSeries_t  playerExpectedData;
        playerInputPosition[eSeamSeries2][eSeam2] = recorderInputPosition[eSeamSeries0][eSeam0];
        playerExpectedData[eSeamSeries2][eSeam2] = recorderInputData[eSeamSeries0][eSeam0];

        QTest::newRow("EmptyContour")
            << numTrigger
            << recorderInputPosition
            << recorderInputData
            << playerSeamSeriesOffset
            << playerSeamOffset
            << playerInputPosition
            << playerExpectedData;
    }

    {
        int numTrigger = 3;
        posPerSeamSeries_t recorderInputPosition;
        dataPerSeamSeries_t recorderInputData;

        for (int imageNumber = 0; imageNumber < numTrigger; ++imageNumber)
        {
            recorderInputPosition[eSeamSeries0][eSeam0].push_back(TestPosition{100.0*imageNumber, eRankMax});
            TestData data;
            data.points = std::vector<precitec::geo2d::DPoint>(numTrigger, {0.1, 1.1*imageNumber});
            data.rankPoints = 255;
            recorderInputData[eSeamSeries0][eSeam0].push_back(data);
        };
        int playerSeamSeriesOffset = -2;
        int playerSeamOffset = -2;

        posPerSeamSeries_t playerInputPosition;
        dataPerSeamSeries_t  playerExpectedData;
        playerInputPosition[eSeamSeries3][eSeam3] = recorderInputPosition[eSeamSeries0][eSeam0];
        playerExpectedData[eSeamSeries3][eSeam3] = std::vector<TestData>(numTrigger, invalidResult);;

        QTest::newRow("SeamWithoutBuffer")
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
        posPerSeamSeries_t recorderInputPosition;
        dataPerSeamSeries_t recorderInputData;

        for (int imageNumber = 0; imageNumber < numTrigger; ++imageNumber)
        {
            recorderInputPosition[eSeamSeries0][eSeam0].push_back(TestPosition{double(imageNumber), eRankMax});
            TestData data;
            data.points = std::vector<precitec::geo2d::DPoint>(numTrigger, {0.1, 1.1*imageNumber});
            data.rankPoints = 255;
            recorderInputData[eSeamSeries0][eSeam0].push_back(data);
        };
        int playerSeamSeriesOffset = -2;
        int playerSeamOffset = -2;

        posPerSeamSeries_t playerInputPosition;
        dataPerSeamSeries_t playerExpectedData;

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

void ContourBufferTest::testProceed()
{
    unsigned int slot = 2;
    int velocity = 30;
    int triggerDelta = 1;

    QFETCH(int, numTrigger);
    QFETCH(posPerSeamSeries_t, recorderInputPosition);
    QFETCH(dataPerSeamSeries_t, recorderInputData);

    //first seams: recorder filter
    {
        ContourBufferRecorder filterRecorder;
        DummyRecorderInput input;

        QVERIFY(input.connectToFilter(&filterRecorder));
        QVERIFY(verifySizeContourBufferSingleton(0));

        for (const auto& testSeamSerieData: recorderInputData)
        {
            auto& seamSeries = testSeamSerieData.first;
            for (const auto& testSeamData: testSeamSerieData.second)
            {
                auto& seam = testSeamData.first;
                auto& dataVector = testSeamData.second;

                //verify that the data saved in proceed_data is consistent
                auto itPos = (*recorderInputPosition.find(seamSeries)).second.find(seam);
                auto end = recorderInputPosition.at(seamSeries).end();
                QVERIFY2(itPos != end, "error in test data");
                auto & posVector = itPos->second;
                QVERIFY2(dataVector.size() == posVector.size(), "error in test data");

                auto externalProductData = precitec::analyzer::ProductData {
                    seamSeries, seam, velocity, triggerDelta, numTrigger
                };
                filterRecorder.setExternalData(&externalProductData);
                filterRecorder.getParameters().update(std::string("Slot"), fliplib::Parameter::TYPE_uint, slot);
                filterRecorder.setParameter();
                filterRecorder.arm(precitec::filter::ArmState::eSeamStart);

                for (std::size_t imageNumber = 0; imageNumber < dataVector.size(); imageNumber++)
                {
                    auto & rData = dataVector[imageNumber];
                    auto & rPos = posVector[imageNumber];
                    input.fillDataAndSignal(imageNumber, rPos, rData);
                }
            }
        }
    }

    //QVERIFY(verifySizeContourBufferSingleton(recorderInputData.size()));

    //next seams: buffer player filter
    {
        QFETCH(int, playerSeamSeriesOffset);
        QFETCH(int, playerSeamOffset);
        QFETCH(posPerSeamSeries_t, playerInputPosition);
        QFETCH(dataPerSeamSeries_t, playerExpectedData);

        ContourBufferPlayer filter;
        DummyPlayerInput input;
        DummyFilter dummyFilter;

        QVERIFY(input.connectToFilter(&filter));
        QVERIFY(dummyFilter.connectPipe(filter.findPipe("Data"), 0));
        auto outPipe = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoVecAnnotatedDPointarray>*>(filter.findPipe("Data"));
        QVERIFY(outPipe);

        for (const auto& testSeamSeriesData: playerExpectedData)
        {
            auto& seamSeries = testSeamSeriesData.first;
            for (const auto& testSeamData: testSeamSeriesData.second)
            {
                auto& seam = testSeamData.first;
                auto& expectedDataVector = testSeamData.second;
                //verify that the data saved in proceed_data is consistent
                auto itPos = (*playerInputPosition.find(seamSeries)).second.find(seam);
                auto end = playerInputPosition.at(seamSeries).end();
                QVERIFY2(itPos != end, "error in test data");
                auto& posVector = itPos->second;
                QVERIFY2(expectedDataVector.size() == posVector.size(), "error in test data");

                auto externalProductData = precitec::analyzer::ProductData {
                    seamSeries, seam, velocity, triggerDelta, numTrigger
                };
                filter.setExternalData(&externalProductData);
                filter.getParameters().update(std::string("Slot"), fliplib::Parameter::TYPE_uint, slot);
                filter.getParameters().update(std::string("SeamSeriesOffset"), fliplib::Parameter::TYPE_int, playerSeamSeriesOffset);
                filter.getParameters().update(std::string("SeamOffset"), fliplib::Parameter::TYPE_int, playerSeamOffset);
                filter.getParameters().update(std::string("DataOffset"), fliplib::Parameter::TYPE_int, 0);
                filter.setParameter();
                filter.arm(precitec::filter::ArmState::eSeamStart);

                for (std::size_t imageNumber = 0; imageNumber < expectedDataVector.size(); imageNumber++)
                {
                    QCOMPARE(dummyFilter.isProceedCalled(), false);

                    auto& expectedData = expectedDataVector[imageNumber];
                    auto& inputPos = posVector[imageNumber];

                    input.fillDataAndSignal(imageNumber, inputPos);

                    const auto result = outPipe->read(imageNumber);
                    QCOMPARE(result.ref().size(), 1);
                    auto& firstContour = result.ref()[0];
                    QCOMPARE(firstContour.getData().size(), expectedData.points.size());
                    QCOMPARE(firstContour.getData(), expectedData.points);
                    QCOMPARE(dummyFilter.isProceedCalled(), true);
                    dummyFilter.resetProceedCalled();
                }
            }
        }
    }
}

QTEST_GUILESS_MAIN(ContourBufferTest)
#include "contourBufferTest.moc"
