#include "../../Mod_Grabber/autotests/testHelper.h"

#include "message/simulationCmd.h"
#include "message/messageBuffer.h"

using precitec::interface::SimulationInitStatus;
using precitec::interface::SimulationFrameStatus;
using precitec::system::message::StaticMessageBuffer;

class SimulationInitStatusTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(SimulationInitStatusTest);
CPPUNIT_TEST(testDefaultCtor);
CPPUNIT_TEST(testSetImageBasePath);
CPPUNIT_TEST(testImageData);
CPPUNIT_TEST(testFrameStatus);
CPPUNIT_TEST(testSerialization);
CPPUNIT_TEST_SUITE_END();
public:
    void testDefaultCtor();
    void testSetImageBasePath();
    void testImageData();
    void testFrameStatus();
    void testSerialization();
};

void SimulationInitStatusTest::testDefaultCtor()
{
    SimulationInitStatus status;
    CPPUNIT_ASSERT_EQUAL(std::string{}, status.imageBasePath());
    CPPUNIT_ASSERT_EQUAL(std::size_t{0}, status.imageData().size());
    CPPUNIT_ASSERT_EQUAL(false, status.status().hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.status().hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.status().hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(false, status.status().hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(0u, status.status().frameIndex());
}

void SimulationInitStatusTest::testSetImageBasePath()
{
    SimulationInitStatus status;
    CPPUNIT_ASSERT_EQUAL(std::string{}, status.imageBasePath());
    status.setImageBasePath(std::string{"foobar"});
    CPPUNIT_ASSERT_EQUAL(std::string{"foobar"}, status.imageBasePath());
}

void SimulationInitStatusTest::testImageData()
{
    SimulationInitStatus status;
    CPPUNIT_ASSERT_EQUAL(std::size_t{0}, status.imageData().size());
    status.setImageData(std::vector<SimulationInitStatus::ImageData>{
        {0, 0, 0},
        {0, 0, 1},
        {0, 1, 0},
        {0, 2, 0},
        {0, 2, 1},
        {1, 2, 0},
        {1, 2, 1},
        {1, 2, 2},
        {1, 2, 3}
    });
    CPPUNIT_ASSERT_EQUAL(std::size_t{9}, status.imageData().size());
    CPPUNIT_ASSERT_EQUAL(0u, status.imageData().at(0).seamSeries);
    CPPUNIT_ASSERT_EQUAL(0u, status.imageData().at(0).seam);
    CPPUNIT_ASSERT_EQUAL(0u, status.imageData().at(0).image);
    CPPUNIT_ASSERT_EQUAL(0u, status.imageData().at(1).seamSeries);
    CPPUNIT_ASSERT_EQUAL(0u, status.imageData().at(1).seam);
    CPPUNIT_ASSERT_EQUAL(1u, status.imageData().at(1).image);
    CPPUNIT_ASSERT_EQUAL(0u, status.imageData().at(2).seamSeries);
    CPPUNIT_ASSERT_EQUAL(1u, status.imageData().at(2).seam);
    CPPUNIT_ASSERT_EQUAL(0u, status.imageData().at(2).image);
    CPPUNIT_ASSERT_EQUAL(0u, status.imageData().at(3).seamSeries);
    CPPUNIT_ASSERT_EQUAL(2u, status.imageData().at(3).seam);
    CPPUNIT_ASSERT_EQUAL(0u, status.imageData().at(3).image);
    CPPUNIT_ASSERT_EQUAL(0u, status.imageData().at(4).seamSeries);
    CPPUNIT_ASSERT_EQUAL(2u, status.imageData().at(4).seam);
    CPPUNIT_ASSERT_EQUAL(1u, status.imageData().at(4).image);
    CPPUNIT_ASSERT_EQUAL(1u, status.imageData().at(5).seamSeries);
    CPPUNIT_ASSERT_EQUAL(2u, status.imageData().at(5).seam);
    CPPUNIT_ASSERT_EQUAL(0u, status.imageData().at(5).image);
    CPPUNIT_ASSERT_EQUAL(1u, status.imageData().at(6).seamSeries);
    CPPUNIT_ASSERT_EQUAL(2u, status.imageData().at(6).seam);
    CPPUNIT_ASSERT_EQUAL(1u, status.imageData().at(6).image);
    CPPUNIT_ASSERT_EQUAL(1u, status.imageData().at(7).seamSeries);
    CPPUNIT_ASSERT_EQUAL(2u, status.imageData().at(7).seam);
    CPPUNIT_ASSERT_EQUAL(2u, status.imageData().at(7).image);
    CPPUNIT_ASSERT_EQUAL(1u, status.imageData().at(8).seamSeries);
    CPPUNIT_ASSERT_EQUAL(2u, status.imageData().at(8).seam);
    CPPUNIT_ASSERT_EQUAL(3u, status.imageData().at(8).image);
}

void SimulationInitStatusTest::testFrameStatus()
{
    SimulationInitStatus status;
    CPPUNIT_ASSERT_EQUAL(false, status.status().hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.status().hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.status().hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(false, status.status().hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(0u, status.status().frameIndex());

    SimulationFrameStatus frameStatus;
    frameStatus.setFrameIndex(100u);
    frameStatus.setHasNextSeam(true);
    status.setStatus(frameStatus);
    CPPUNIT_ASSERT_EQUAL(false, status.status().hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.status().hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.status().hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(false, status.status().hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(100u, status.status().frameIndex());
}

void SimulationInitStatusTest::testSerialization()
{
    StaticMessageBuffer buffer(1024);
    SimulationInitStatus status;
    status.setImageBasePath("foobar");
    status.setImageData(std::vector<SimulationInitStatus::ImageData>{
        {0, 0, 0},
        {0, 0, 1},
        {0, 1, 0},
        {0, 2, 0},
        {0, 2, 1},
        {1, 2, 0},
        {1, 2, 1},
        {1, 2, 2},
        {1, 2, 3}
    });
    SimulationFrameStatus frameStatus;
    frameStatus.setFrameIndex(100u);
    frameStatus.setHasPreviousSeam(true);
    status.setStatus(frameStatus);

    status.serialize(buffer);
    buffer.rewind();

    SimulationInitStatus status2;
    status2.deserialize(buffer);
    CPPUNIT_ASSERT_EQUAL(std::string{"foobar"}, status2.imageBasePath());
    CPPUNIT_ASSERT_EQUAL(std::size_t{9}, status2.imageData().size());
    CPPUNIT_ASSERT_EQUAL(0u, status2.imageData().at(0).seamSeries);
    CPPUNIT_ASSERT_EQUAL(0u, status2.imageData().at(0).seam);
    CPPUNIT_ASSERT_EQUAL(0u, status2.imageData().at(0).image);
    CPPUNIT_ASSERT_EQUAL(0u, status2.imageData().at(1).seamSeries);
    CPPUNIT_ASSERT_EQUAL(0u, status2.imageData().at(1).seam);
    CPPUNIT_ASSERT_EQUAL(1u, status2.imageData().at(1).image);
    CPPUNIT_ASSERT_EQUAL(0u, status2.imageData().at(2).seamSeries);
    CPPUNIT_ASSERT_EQUAL(1u, status2.imageData().at(2).seam);
    CPPUNIT_ASSERT_EQUAL(0u, status2.imageData().at(2).image);
    CPPUNIT_ASSERT_EQUAL(0u, status2.imageData().at(3).seamSeries);
    CPPUNIT_ASSERT_EQUAL(2u, status2.imageData().at(3).seam);
    CPPUNIT_ASSERT_EQUAL(0u, status2.imageData().at(3).image);
    CPPUNIT_ASSERT_EQUAL(0u, status2.imageData().at(4).seamSeries);
    CPPUNIT_ASSERT_EQUAL(2u, status2.imageData().at(4).seam);
    CPPUNIT_ASSERT_EQUAL(1u, status2.imageData().at(4).image);
    CPPUNIT_ASSERT_EQUAL(1u, status2.imageData().at(5).seamSeries);
    CPPUNIT_ASSERT_EQUAL(2u, status2.imageData().at(5).seam);
    CPPUNIT_ASSERT_EQUAL(0u, status2.imageData().at(5).image);
    CPPUNIT_ASSERT_EQUAL(1u, status2.imageData().at(6).seamSeries);
    CPPUNIT_ASSERT_EQUAL(2u, status2.imageData().at(6).seam);
    CPPUNIT_ASSERT_EQUAL(1u, status2.imageData().at(6).image);
    CPPUNIT_ASSERT_EQUAL(1u, status2.imageData().at(7).seamSeries);
    CPPUNIT_ASSERT_EQUAL(2u, status2.imageData().at(7).seam);
    CPPUNIT_ASSERT_EQUAL(2u, status2.imageData().at(7).image);
    CPPUNIT_ASSERT_EQUAL(1u, status2.imageData().at(8).seamSeries);
    CPPUNIT_ASSERT_EQUAL(2u, status2.imageData().at(8).seam);
    CPPUNIT_ASSERT_EQUAL(3u, status2.imageData().at(8).image);
    CPPUNIT_ASSERT_EQUAL(false, status2.status().hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(false, status2.status().hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(false, status2.status().hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(true, status2.status().hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(100u, status2.status().frameIndex());
}

TEST_MAIN(SimulationInitStatusTest)
