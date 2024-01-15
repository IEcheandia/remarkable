#include "../../Mod_Grabber/autotests/testHelper.h"

#include "message/simulationCmd.h"
#include "message/messageBuffer.h"

#include <climits>

using precitec::interface::SimulationFrameStatus;
using precitec::system::message::StaticMessageBuffer;

class SimulationFrameStatusTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(SimulationFrameStatusTest);
CPPUNIT_TEST(testDefaultCtor);
CPPUNIT_TEST(testHasNextFrame);
CPPUNIT_TEST(testHasPreviousFrame);
CPPUNIT_TEST(testHasNextSeam);
CPPUNIT_TEST(testHasPreviousSeam);
CPPUNIT_TEST(testFrameIndex);
CPPUNIT_TEST(testSerialization);
CPPUNIT_TEST_SUITE_END();
public:
    void testDefaultCtor();
    void testHasNextFrame();
    void testHasPreviousFrame();
    void testHasNextSeam();
    void testHasPreviousSeam();
    void testFrameIndex();
    void testSerialization();
};

void SimulationFrameStatusTest::testDefaultCtor()
{
    SimulationFrameStatus status;
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(0u, status.frameIndex());
}

void SimulationFrameStatusTest::testHasNextFrame()
{
    SimulationFrameStatus status;
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextFrame());
    status.setHasNextFrame(true);
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousFrame());
    status.setHasNextFrame(false);
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextFrame());
}

void SimulationFrameStatusTest::testHasPreviousFrame()
{
    SimulationFrameStatus status;
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousFrame());
    status.setHasPreviousFrame(true);
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextFrame());
    status.setHasPreviousFrame(false);
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousFrame());
}

void SimulationFrameStatusTest::testHasNextSeam()
{
    SimulationFrameStatus status;
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextSeam());
    status.setHasNextSeam(true);
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextFrame());
    status.setHasNextSeam(false);
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextSeam());
}

void SimulationFrameStatusTest::testHasPreviousSeam()
{
    SimulationFrameStatus status;
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousSeam());
    status.setHasPreviousSeam(true);
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextFrame());
    status.setHasPreviousSeam(false);
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousSeam());
}

void SimulationFrameStatusTest::testFrameIndex()
{
    SimulationFrameStatus status;
    CPPUNIT_ASSERT_EQUAL(0u, status.frameIndex());
    status.setFrameIndex(1);
    CPPUNIT_ASSERT_EQUAL(1u, status.frameIndex());
    status.setFrameIndex(UINT_MAX);
    CPPUNIT_ASSERT_EQUAL(UINT_MAX, status.frameIndex());
    status.setFrameIndex(0);
    CPPUNIT_ASSERT_EQUAL(0u, status.frameIndex());
}

void SimulationFrameStatusTest::testSerialization()
{
    StaticMessageBuffer buffer(sizeof(SimulationFrameStatus)*4);
    SimulationFrameStatus status;
    status.setHasNextFrame(true);
    status.setFrameIndex(100);
    status.serialize(buffer);

    SimulationFrameStatus status2;
    CPPUNIT_ASSERT_EQUAL(false, status2.hasNextFrame());
    buffer.rewind();
    status2.deserialize(buffer);
    CPPUNIT_ASSERT_EQUAL(true, status2.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(false, status2.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(false, status2.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(false, status2.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(100u, status2.frameIndex());

    status.setHasNextFrame(false);
    status.setHasPreviousFrame(true);
    buffer.rewind();
    status.serialize(buffer);
    buffer.rewind();
    status2.deserialize(buffer);
    CPPUNIT_ASSERT_EQUAL(false, status2.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(false, status2.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(false, status2.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(true, status2.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(100u, status2.frameIndex());

    status.setHasPreviousFrame(false);
    status.setHasNextSeam(true);
    buffer.rewind();
    status.serialize(buffer);
    buffer.rewind();
    status2.deserialize(buffer);
    CPPUNIT_ASSERT_EQUAL(false, status2.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(false, status2.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(true, status2.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(false, status2.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(100u, status2.frameIndex());

    status.setHasNextSeam(false);
    status.setHasPreviousSeam(true);
    buffer.rewind();
    status.serialize(buffer);
    buffer.rewind();
    status2.deserialize(buffer);
    CPPUNIT_ASSERT_EQUAL(false, status2.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(true, status2.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(false, status2.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(false, status2.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(100u, status2.frameIndex());
}

TEST_MAIN(SimulationFrameStatusTest)
