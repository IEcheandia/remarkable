#include "../../Mod_GrabberNoHw/autotests/testHelper.h"

#include "mockProductListProvider.h"
#include "../include/commandServer.h"
#include "analyzer/centralDeviceManager.h"

#include "videoRecorder/fileCommand.h"

using precitec::Simulation::CommandServer;
using precitec::interface::SimulationFrameStatus;
using precitec::interface::SimulationInitStatus;
using precitec::analyzer::CentralDeviceManager;

class CommandServerTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(CommandServerTest);
CPPUNIT_TEST(testCtor);
CPPUNIT_TEST(testNextNotInited);
CPPUNIT_TEST(testPreviousNotInited);
CPPUNIT_TEST(testPreviousNotInited);
CPPUNIT_TEST(testPreviousSeamNotInited);
CPPUNIT_TEST(testJumpToFrameNotInited);
CPPUNIT_TEST(testInitNoProducts);
CPPUNIT_TEST(testInitNoFiles);
CPPUNIT_TEST(testInitWithFiles);
CPPUNIT_TEST(testInspect);
CPPUNIT_TEST_SUITE_END();
public:
    void testCtor();
    void testNextNotInited();
    void testPreviousNotInited();
    void testNextSeamNotInited();
    void testPreviousSeamNotInited();
    void testJumpToFrameNotInited();
    void testInitNoProducts();
    void testInitNoFiles();
    void testInitWithFiles();
    void testInspect();
};

void CommandServerTest::testCtor()
{
    CommandServer server(nullptr, nullptr, precitec::workflow::SharedProductListProvider{});
    CPPUNIT_ASSERT_EQUAL(false, server.hasNext());
    CPPUNIT_ASSERT_EQUAL(false, server.hasPrevious());
    CPPUNIT_ASSERT_EQUAL(false, server.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(false, server.hasPreviousSeam());
    SimulationFrameStatus status{server.currentStatus()};
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(0u, status.frameIndex());
}

void CommandServerTest::testNextNotInited()
{
    CommandServer server(nullptr, nullptr, precitec::workflow::SharedProductListProvider{});
    SimulationFrameStatus status{server.nextFrame()};
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(0u, status.frameIndex());
}

void CommandServerTest::testPreviousNotInited()
{
    CommandServer server(nullptr, nullptr, precitec::workflow::SharedProductListProvider{});
    SimulationFrameStatus status{server.previousFrame()};
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(0u, status.frameIndex());
}

void CommandServerTest::testNextSeamNotInited()
{
    CommandServer server(nullptr, nullptr, precitec::workflow::SharedProductListProvider{});
    SimulationFrameStatus status{server.nextSeam()};
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(0u, status.frameIndex());
}

void CommandServerTest::testPreviousSeamNotInited()
{
    CommandServer server(nullptr, nullptr, precitec::workflow::SharedProductListProvider{});
    SimulationFrameStatus status{server.previousSeam()};
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(0u, status.frameIndex());
}

void CommandServerTest::testJumpToFrameNotInited()
{
    CommandServer server(nullptr, nullptr, precitec::workflow::SharedProductListProvider{});
    SimulationFrameStatus status{server.jumpToFrame(1)};
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(0u, status.frameIndex());
}

void CommandServerTest::testInitNoProducts()
{
    Poco::SharedPtr<MockProductListProvider> productListProvider{new MockProductListProvider};
    CommandServer server(nullptr, nullptr, productListProvider);
    SimulationInitStatus status{server.initSimulation(Poco::UUID(), Poco::UUID(), Poco::UUID())};
    CPPUNIT_ASSERT_EQUAL(true, status.imageData().empty());
    CPPUNIT_ASSERT_EQUAL(std::string{}, status.imageBasePath());
    CPPUNIT_ASSERT_EQUAL(false, status.status().hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.status().hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.status().hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(false, status.status().hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(0u, status.status().frameIndex());
}

void CommandServerTest::testInitNoFiles()
{
    TemporaryDirectory dir{std::string{"CommandServerTestTestInitNoFiles"}};
    const std::string baseDir{dir.path()};
    setenv("WM_BASE_DIR", baseDir.c_str(), 1);
    Poco::Path path{baseDir};

    auto createDirectory = [&] (const std::string &name)
    {
        Poco::Path dir(path, name);
        CPPUNIT_ASSERT(Poco::File(dir).createDirectory());
        return dir;
    };
    createDirectory("video");
    createDirectory("video/WM-QNX-PC");
    auto &generator = Poco::UUIDGenerator::defaultGenerator();
    Poco::UUID product{generator.create()};
    std::string productPath = "video/WM-QNX-PC/" + product.toString();
    createDirectory(productPath);
    const auto productInstanceDir =createDirectory(productPath + "/0b370fbe-e639-11e7-8031-000c29043e1d-SN-1234");
    createDirectory(productPath + "/0b370fbe-e639-11e7-8031-000c29043e1d-SN-1234/seam_series0000");
    createDirectory(productPath + "/0b370fbe-e639-11e7-8031-000c29043e1d-SN-1234/seam_series0000/seam0000");
    auto createFile = [&] (const std::string &name)
    {
        Poco::Path dir(productInstanceDir, name);
        CPPUNIT_ASSERT(Poco::File(dir).createFile());
    };
    createFile("0b370fbe-e639-11e7-8031-000c29043e1d.id");

    CentralDeviceManager deviceManager;
    Poco::SharedPtr<MockProductListProvider> productListProvider{new MockProductListProvider};
    productListProvider->addProduct(product, generator.create(), generator.create(), 1, true, 1, 1, "Test", 0, -1);
    CommandServer server(nullptr, &deviceManager, productListProvider);
    SimulationInitStatus initStatus{server.initSimulation(product, Poco::UUID("0b370fbe-e639-11e7-8031-000c29043e1d"), product)};
    CPPUNIT_ASSERT_EQUAL(true, initStatus.imageData().empty());
    CPPUNIT_ASSERT_EQUAL(productInstanceDir.absolute().toString() + std::string("/"), initStatus.imageBasePath());
    CPPUNIT_ASSERT_EQUAL(false, initStatus.status().hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(false, initStatus.status().hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(false, initStatus.status().hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(false, initStatus.status().hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(0u, initStatus.status().frameIndex());
}

void CommandServerTest::testInitWithFiles()
{
    TemporaryDirectory dir{std::string{"CommandServerTestTestInitWithFiles"}};
    const std::string baseDir{dir.path()};
    setenv("WM_BASE_DIR", baseDir.c_str(), 1);
    Poco::Path path{baseDir};

    auto createDirectory = [&] (const std::string &name)
    {
        Poco::Path dir(path, name);
        CPPUNIT_ASSERT(Poco::File(dir).createDirectory());
        return dir;
    };
    createDirectory("video");
    createDirectory("video/WM-QNX-PC");
    auto &generator = Poco::UUIDGenerator::defaultGenerator();
    Poco::UUID product{generator.create()};
    std::string productPath = "video/WM-QNX-PC/" + product.toString();
    createDirectory(productPath);
    const auto productInstanceDir =createDirectory(productPath + "/0b370fbe-e639-11e7-8031-000c29043e1d-SN-12345");
    createDirectory(productPath + "/0b370fbe-e639-11e7-8031-000c29043e1d-SN-12345/seam_series0000");
    createDirectory(productPath + "/0b370fbe-e639-11e7-8031-000c29043e1d-SN-12345/seam_series0000/seam0000");
    createDirectory(productPath + "/0b370fbe-e639-11e7-8031-000c29043e1d-SN-12345/seam_series0000/seam0001");
    createDirectory(productPath + "/0b370fbe-e639-11e7-8031-000c29043e1d-SN-12345/seam_series0000/seam0002");
    createDirectory(productPath + "/0b370fbe-e639-11e7-8031-000c29043e1d-SN-12345/seam_series0001");
    createDirectory(productPath + "/0b370fbe-e639-11e7-8031-000c29043e1d-SN-12345/seam_series0001/seam0000");
    createDirectory(productPath + "/0b370fbe-e639-11e7-8031-000c29043e1d-SN-12345/seam_series0001/seam0001");
    createDirectory(productPath + "/0b370fbe-e639-11e7-8031-000c29043e1d-SN-12345/seam_series0001/seam0002");

    auto createFile = [&] (const std::string &name)
    {
        Poco::Path dir(productInstanceDir, name);
        CPPUNIT_ASSERT(Poco::File(dir).createFile());
    };
    createFile("seam_series0000/seam0000/00000.bmp");
    createFile("seam_series0000/seam0000/00001.bmp");
    createFile("seam_series0000/seam0000/00002.bmp");
    createFile("seam_series0000/seam0001/00000.bmp");
    createFile("seam_series0000/seam0001/00001.smp");
    createFile("seam_series0000/seam0001/00002.smp");
    createFile("seam_series0000/seam0002/00000.bmp");
    createFile("seam_series0001/seam0000/00000.bmp");
    createFile("seam_series0001/seam0001/00000.bmp");
    createFile("seam_series0001/seam0002/00000.bmp");
    createFile("0b370fbe-e639-11e7-8031-000c29043e1d.id");

    CentralDeviceManager deviceManager;
    Poco::SharedPtr<MockProductListProvider> productListProvider{new MockProductListProvider};
    productListProvider->addProduct(product, generator.create(), generator.create(), 1, true, 1, 1, "Test", 0, -1);

    MockTriggerCmd &trigger = productListProvider->triggerCmd();

    CommandServer server(&trigger, &deviceManager, productListProvider);
    SimulationInitStatus initStatus{server.initSimulation(product, Poco::UUID("0b370fbe-e639-11e7-8031-000c29043e1d"), product)};
    CPPUNIT_ASSERT_EQUAL(productInstanceDir.absolute().toString() + std::string("/"), initStatus.imageBasePath());
    CPPUNIT_ASSERT_EQUAL(false, initStatus.imageData().empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t{10}, initStatus.imageData().size());
    CPPUNIT_ASSERT_EQUAL(0u, initStatus.imageData().at(0).seamSeries);
    CPPUNIT_ASSERT_EQUAL(0u, initStatus.imageData().at(0).seam);
    CPPUNIT_ASSERT_EQUAL(0u, initStatus.imageData().at(0).image);
    CPPUNIT_ASSERT_EQUAL(0u, initStatus.imageData().at(1).seamSeries);
    CPPUNIT_ASSERT_EQUAL(0u, initStatus.imageData().at(1).seam);
    CPPUNIT_ASSERT_EQUAL(1u, initStatus.imageData().at(1).image);
    CPPUNIT_ASSERT_EQUAL(0u, initStatus.imageData().at(2).seamSeries);
    CPPUNIT_ASSERT_EQUAL(0u, initStatus.imageData().at(2).seam);
    CPPUNIT_ASSERT_EQUAL(2u, initStatus.imageData().at(2).image);
    CPPUNIT_ASSERT_EQUAL(0u, initStatus.imageData().at(3).seamSeries);
    CPPUNIT_ASSERT_EQUAL(1u, initStatus.imageData().at(3).seam);
    CPPUNIT_ASSERT_EQUAL(0u, initStatus.imageData().at(3).image);
    CPPUNIT_ASSERT_EQUAL(0u, initStatus.imageData().at(4).seamSeries);
    CPPUNIT_ASSERT_EQUAL(1u, initStatus.imageData().at(4).seam);
    CPPUNIT_ASSERT_EQUAL(1u, initStatus.imageData().at(4).image);
    CPPUNIT_ASSERT_EQUAL(0u, initStatus.imageData().at(5).seamSeries);
    CPPUNIT_ASSERT_EQUAL(1u, initStatus.imageData().at(5).seam);
    CPPUNIT_ASSERT_EQUAL(2u, initStatus.imageData().at(5).image);
    CPPUNIT_ASSERT_EQUAL(0u, initStatus.imageData().at(6).seamSeries);
    CPPUNIT_ASSERT_EQUAL(2u, initStatus.imageData().at(6).seam);
    CPPUNIT_ASSERT_EQUAL(0u, initStatus.imageData().at(6).image);
    CPPUNIT_ASSERT_EQUAL(1u, initStatus.imageData().at(7).seamSeries);
    CPPUNIT_ASSERT_EQUAL(0u, initStatus.imageData().at(7).seam);
    CPPUNIT_ASSERT_EQUAL(0u, initStatus.imageData().at(7).image);
    CPPUNIT_ASSERT_EQUAL(1u, initStatus.imageData().at(8).seamSeries);
    CPPUNIT_ASSERT_EQUAL(1u, initStatus.imageData().at(8).seam);
    CPPUNIT_ASSERT_EQUAL(0u, initStatus.imageData().at(8).image);
    CPPUNIT_ASSERT_EQUAL(1u, initStatus.imageData().at(9).seamSeries);
    CPPUNIT_ASSERT_EQUAL(2u, initStatus.imageData().at(9).seam);
    CPPUNIT_ASSERT_EQUAL(0u, initStatus.imageData().at(9).image);

    CPPUNIT_ASSERT_EQUAL(true, initStatus.status().hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(false, initStatus.status().hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(true, initStatus.status().hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(false, initStatus.status().hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(0u, initStatus.status().frameIndex());

    // let's go forward through next
    for (uint32_t i = 1u; i < 9u; i++)
    {
        SimulationFrameStatus status{server.nextFrame()};
        CPPUNIT_ASSERT_EQUAL(true, status.hasNextFrame());
        CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousFrame());
        CPPUNIT_ASSERT_EQUAL(true, status.hasNextSeam());
        CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousSeam());
        CPPUNIT_ASSERT_EQUAL(i, status.frameIndex());

        CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), int(initStatus.imageData().at(i).seamSeries));
        CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), int(initStatus.imageData().at(i).seam));
        CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), int(initStatus.imageData().at(i).image));
    }
    SimulationFrameStatus status{server.nextFrame()};
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(9u, status.frameIndex());
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 1);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 2);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);

    // now move backwards
    for (uint32_t i = 8u; i > 0; i--)
    {
        SimulationFrameStatus status{server.previousFrame()};
        CPPUNIT_ASSERT_EQUAL(true, status.hasNextFrame());
        CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousFrame());
        CPPUNIT_ASSERT_EQUAL(true, status.hasNextSeam());
        CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousSeam());
        CPPUNIT_ASSERT_EQUAL(i, status.frameIndex());

        CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), int(initStatus.imageData().at(i).seamSeries));
        CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), int(initStatus.imageData().at(i).seam));
        CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), int(initStatus.imageData().at(i).image));
    }
    status = server.previousFrame();
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(0u, status.frameIndex());
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);

    // now move by seams
    status = server.nextSeam();
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(3u, status.frameIndex());
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 1);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);

    // test stop
    status = server.stop();
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(0u, status.frameIndex());
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);

    // and continue forwards
    status = server.nextSeam();
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(3u, status.frameIndex());
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 1);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);

    status = server.nextSeam();
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(6u, status.frameIndex());
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 2);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);
    status = server.nextSeam();
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(7u, status.frameIndex());
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 1);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);
    status = server.nextSeam();
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(8u, status.frameIndex());
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 1);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 1);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);
    status = server.nextSeam();
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(9u, status.frameIndex());
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 1);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 2);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);

    // same frame
    status = server.sameFrame();
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(9u, status.frameIndex());
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 1);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 2);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);

    // and seam backwards
    status = server.previousSeam();
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(8u, status.frameIndex());
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 1);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 1);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);
    status = server.previousSeam();
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(7u, status.frameIndex());
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 1);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);
    status = server.previousSeam();
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(6u, status.frameIndex());
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 2);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);
    status = server.previousSeam();
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(3u, status.frameIndex());
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 1);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);
    status = server.previousSeam();
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(0u, status.frameIndex());
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);

    // special case: previous seam when not at seam start
    // first jump to frame 1
    status = server.jumpToFrame(1);
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(1u, status.frameIndex());
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 1);
    status = server.previousSeam();
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(false, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(0u, status.frameIndex());
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);

    // let's jump to end
    status = server.jumpToFrame(9);
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(false, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(9u, status.frameIndex());
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 1);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 2);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);
    // jump somewhere in the sequence
    status = server.jumpToFrame(5);
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(5u, status.frameIndex());
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 1);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 2);
    // jump to not existing image
    status = server.jumpToFrame(1000);
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(true, status.hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(true, status.hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(5u, status.frameIndex());
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 1);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 2);
}

void CommandServerTest::testInspect()
{
    TemporaryDirectory dir{std::string{"CommandServerTestTestInitWithFiles"}};
    const std::string baseDir{dir.path()};
    setenv("WM_BASE_DIR", baseDir.c_str(), 1);
    Poco::Path path{baseDir};

    auto createDirectory = [&] (const std::string &name)
    {
        Poco::Path dir(path, name);
        CPPUNIT_ASSERT(Poco::File(dir).createDirectory());
        return dir;
    };
    createDirectory("video");
    createDirectory("video/WM-QNX-PC");
    auto &generator = Poco::UUIDGenerator::defaultGenerator();
    Poco::UUID product{generator.create()};
    std::string productPath = "video/WM-QNX-PC/" + product.toString();
    createDirectory(productPath);
    const auto productInstanceDir =createDirectory(productPath + "/0b370fbe-e639-11e7-8031-000c29043e1d-SN-12345");
    createDirectory(productPath + "/0b370fbe-e639-11e7-8031-000c29043e1d-SN-12345/seam_series0000");
    createDirectory(productPath + "/0b370fbe-e639-11e7-8031-000c29043e1d-SN-12345/seam_series0000/seam0000");
    createDirectory(productPath + "/0b370fbe-e639-11e7-8031-000c29043e1d-SN-12345/seam_series0000/seam0001");
    createDirectory(productPath + "/0b370fbe-e639-11e7-8031-000c29043e1d-SN-12345/seam_series0000/seam0002");
    createDirectory(productPath + "/0b370fbe-e639-11e7-8031-000c29043e1d-SN-12345/seam_series0001");
    createDirectory(productPath + "/0b370fbe-e639-11e7-8031-000c29043e1d-SN-12345/seam_series0001/seam0000");
    createDirectory(productPath + "/0b370fbe-e639-11e7-8031-000c29043e1d-SN-12345/seam_series0001/seam0001");
    createDirectory(productPath + "/0b370fbe-e639-11e7-8031-000c29043e1d-SN-12345/seam_series0001/seam0002");

    auto createFile = [&] (const std::string &name)
    {
        Poco::Path dir(productInstanceDir, name);
        CPPUNIT_ASSERT(Poco::File(dir).createFile());
    };
    createFile("seam_series0000/seam0000/00000.bmp");
    createFile("seam_series0000/seam0000/00001.bmp");
    createFile("seam_series0000/seam0000/00002.bmp");
    createFile("seam_series0000/seam0001/00000.bmp");
    createFile("seam_series0000/seam0001/00001.smp");
    createFile("seam_series0000/seam0001/00002.smp");
    createFile("seam_series0000/seam0002/00000.bmp");
    createFile("seam_series0001/seam0000/00000.bmp");
    createFile("seam_series0001/seam0001/00000.bmp");
    createFile("seam_series0001/seam0002/00000.bmp");
    createFile("0b370fbe-e639-11e7-8031-000c29043e1d.id");

    // create a sequence info file for seam 1/2 to point to 1/0 and make it a linked seam
    precitec::vdr::Counters counters{};
    precitec::vdr::WriteConfCmd confCmd{Poco::File{Poco::Path{productInstanceDir, std::string("seam_series0001/seam0002/sequence_info.xml")}}, {}, {1, 0, 20, std::string{"2"}}, counters};
    confCmd.execute();

    CentralDeviceManager deviceManager;
    Poco::SharedPtr<MockProductListProvider> productListProvider{new MockProductListProvider};
    productListProvider->addProduct(product, generator.create(), generator.create(), 1, true, 1, 1, "Test", 0, -1);

    MockTriggerCmd &trigger = productListProvider->triggerCmd();

    CommandServer server(&trigger, &deviceManager, productListProvider);
    SimulationInitStatus initStatus{server.initSimulation(product, Poco::UUID("0b370fbe-e639-11e7-8031-000c29043e1d"), product)};
    CPPUNIT_ASSERT_EQUAL(productInstanceDir.absolute().toString() + std::string("/"), initStatus.imageBasePath());
    CPPUNIT_ASSERT_EQUAL(false, initStatus.imageData().empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t{10}, initStatus.imageData().size());
    CPPUNIT_ASSERT_EQUAL(true, initStatus.status().hasNextFrame());
    CPPUNIT_ASSERT_EQUAL(false, initStatus.status().hasPreviousFrame());
    CPPUNIT_ASSERT_EQUAL(true, initStatus.status().hasNextSeam());
    CPPUNIT_ASSERT_EQUAL(false, initStatus.status().hasPreviousSeam());
    CPPUNIT_ASSERT_EQUAL(0u, initStatus.status().frameIndex());
    CPPUNIT_ASSERT_EQUAL(false, productListProvider->inspectionSeam().has_value());
    CPPUNIT_ASSERT_EQUAL(false, productListProvider->inspectionSeamSeries().has_value());
    CPPUNIT_ASSERT_EQUAL(std::string{}, productListProvider->inspectionSeamLabel());

    // let's go forward through next
    for (uint32_t i = 1u; i < 9u; i++)
    {
        server.nextFrame();
        CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), int(initStatus.imageData().at(i).seamSeries));
        CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), int(initStatus.imageData().at(i).seam));
        CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), int(initStatus.imageData().at(i).image));
        CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeam().has_value());
        CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeamSeries().has_value());
    }
    server.nextFrame();
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 1);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 2);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeam().has_value());
    // linked seam 2 -> 0
    CPPUNIT_ASSERT_EQUAL(0, productListProvider->inspectionSeam().value());
    CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeamSeries().has_value());
    CPPUNIT_ASSERT_EQUAL(1, productListProvider->inspectionSeamSeries().value());
    CPPUNIT_ASSERT_EQUAL(std::string{"2"}, productListProvider->inspectionSeamLabel());

    // now move backwards
    for (uint32_t i = 8u; i > 0; i--)
    {
        server.previousFrame();
        CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), int(initStatus.imageData().at(i).seamSeries));
        CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), int(initStatus.imageData().at(i).seam));
        CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), int(initStatus.imageData().at(i).image));
        CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeam().has_value());
        CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeamSeries().has_value());
    }
    server.previousFrame();
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeam().has_value());
    CPPUNIT_ASSERT_EQUAL(0, productListProvider->inspectionSeam().value());
    CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeamSeries().has_value());
    CPPUNIT_ASSERT_EQUAL(0, productListProvider->inspectionSeamSeries().value());
    CPPUNIT_ASSERT_EQUAL(std::string{}, productListProvider->inspectionSeamLabel());

    // now move by seams
    server.nextSeam();
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 1);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeam().has_value());
    CPPUNIT_ASSERT_EQUAL(1, productListProvider->inspectionSeam().value());
    CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeamSeries().has_value());
    CPPUNIT_ASSERT_EQUAL(0, productListProvider->inspectionSeamSeries().value());
    CPPUNIT_ASSERT_EQUAL(std::string{}, productListProvider->inspectionSeamLabel());
    server.nextSeam();
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 2);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeam().has_value());
    CPPUNIT_ASSERT_EQUAL(2, productListProvider->inspectionSeam().value());
    CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeamSeries().has_value());
    CPPUNIT_ASSERT_EQUAL(0, productListProvider->inspectionSeamSeries().value());
    CPPUNIT_ASSERT_EQUAL(std::string{}, productListProvider->inspectionSeamLabel());
    server.nextSeam();
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 1);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeam().has_value());
    CPPUNIT_ASSERT_EQUAL(0, productListProvider->inspectionSeam().value());
    CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeamSeries().has_value());
    CPPUNIT_ASSERT_EQUAL(1, productListProvider->inspectionSeamSeries().value());
    CPPUNIT_ASSERT_EQUAL(std::string{}, productListProvider->inspectionSeamLabel());
    server.nextSeam();
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 1);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 1);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeam().has_value());
    CPPUNIT_ASSERT_EQUAL(1, productListProvider->inspectionSeam().value());
    CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeamSeries().has_value());
    CPPUNIT_ASSERT_EQUAL(1, productListProvider->inspectionSeamSeries().value());
    CPPUNIT_ASSERT_EQUAL(std::string{}, productListProvider->inspectionSeamLabel());
    server.nextSeam();
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 1);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 2);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeam().has_value());
    // linked seam 2 -> 0
    CPPUNIT_ASSERT_EQUAL(0, productListProvider->inspectionSeam().value());
    CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeamSeries().has_value());
    CPPUNIT_ASSERT_EQUAL(1, productListProvider->inspectionSeamSeries().value());
    CPPUNIT_ASSERT_EQUAL(std::string{"2"}, productListProvider->inspectionSeamLabel());

    server.sameFrame();
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 1);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 2);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeam().has_value());
    // linked seam 2 -> 0
    CPPUNIT_ASSERT_EQUAL(0, productListProvider->inspectionSeam().value());
    CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeamSeries().has_value());
    CPPUNIT_ASSERT_EQUAL(1, productListProvider->inspectionSeamSeries().value());
    CPPUNIT_ASSERT_EQUAL(std::string{"2"}, productListProvider->inspectionSeamLabel());

    // and seam backwards
    server.previousSeam();
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 1);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 1);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeam().has_value());
    CPPUNIT_ASSERT_EQUAL(1, productListProvider->inspectionSeam().value());
    CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeamSeries().has_value());
    CPPUNIT_ASSERT_EQUAL(1, productListProvider->inspectionSeamSeries().value());
    CPPUNIT_ASSERT_EQUAL(std::string{}, productListProvider->inspectionSeamLabel());
    server.previousSeam();
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 1);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeam().has_value());
    CPPUNIT_ASSERT_EQUAL(0, productListProvider->inspectionSeam().value());
    CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeamSeries().has_value());
    CPPUNIT_ASSERT_EQUAL(1, productListProvider->inspectionSeamSeries().value());
    CPPUNIT_ASSERT_EQUAL(std::string{}, productListProvider->inspectionSeamLabel());
    server.previousSeam();
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 2);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeam().has_value());
    CPPUNIT_ASSERT_EQUAL(2, productListProvider->inspectionSeam().value());
    CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeamSeries().has_value());
    CPPUNIT_ASSERT_EQUAL(0, productListProvider->inspectionSeamSeries().value());
    CPPUNIT_ASSERT_EQUAL(std::string{}, productListProvider->inspectionSeamLabel());
    server.previousSeam();
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 1);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeam().has_value());
    CPPUNIT_ASSERT_EQUAL(1, productListProvider->inspectionSeam().value());
    CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeamSeries().has_value());
    CPPUNIT_ASSERT_EQUAL(0, productListProvider->inspectionSeamSeries().value());
    CPPUNIT_ASSERT_EQUAL(std::string{}, productListProvider->inspectionSeamLabel());
    server.previousSeam();
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamSeriesNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().getSeamNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(trigger.lastContext().imageNumber(), 0);
    CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeam().has_value());
    CPPUNIT_ASSERT_EQUAL(0, productListProvider->inspectionSeam().value());
    CPPUNIT_ASSERT_EQUAL(true, productListProvider->inspectionSeamSeries().has_value());
    CPPUNIT_ASSERT_EQUAL(0, productListProvider->inspectionSeamSeries().value());
    CPPUNIT_ASSERT_EQUAL(std::string{}, productListProvider->inspectionSeamLabel());
}

TEST_MAIN(CommandServerTest)
