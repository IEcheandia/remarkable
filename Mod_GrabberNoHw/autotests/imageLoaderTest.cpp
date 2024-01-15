/**
*
* @defgroup ModGrabber
*
* @file
* @brief  Test case for ImageLoader
* @copyright    Precitec GmbH & Co. KG
* @author GM
*/
#include "testHelper.h"
#include "grabber/sharedMemoryImageProvider.h"
#include "trigger/imageDataHolder.h"
#include "trigger/sequenceLoader.h"

#include <Poco/Thread.h>

#include <stdlib.h>
#include <iostream>
#include <memory>


using precitec::ImageDataHolder;
using precitec::SequenceLoader;


class ImageLoaderTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(ImageLoaderTest);
CPPUNIT_TEST(testImages);
CPPUNIT_TEST(testCacheMiss);
CPPUNIT_TEST(testVdrImages);
CPPUNIT_TEST_SUITE_END();
public:
    void setUp() override;
    void tearDown() override;
    
    void testImages();
    void testCacheMiss();
    void testVdrImages();
    
private:
    void testImage(uint32_t productNumber, uint32_t seamSeriesNumber, uint32_t seamNumber, uint32_t imageNumber, int width, int height);
    void testVdrImage(uint32_t productNumber, uint32_t seamSeriesNumber, uint32_t seamNumber, uint32_t imageNumber, int width, int height, int offsetX, int offsetY, int vdrImageNumber);
    std::unique_ptr<TemporaryDirectory> m_tempDir;
    std::unique_ptr<precitec::grabber::SharedMemoryImageProvider> m_sharedMemory;
    SequenceLoader m_loader;
};

void ImageLoaderTest::setUp()
{
    m_tempDir = std::make_unique<TemporaryDirectory>(std::string("ImageLoaderTest"));
    const auto baseDir = m_tempDir->path();
    setenv("WM_BASE_DIR", baseDir.c_str(), 1);
    m_sharedMemory = std::make_unique<precitec::grabber::SharedMemoryImageProvider>();
    m_sharedMemory->init();
    m_loader.setBasepath(baseDir);
    Poco::Path path{baseDir};
    
    // setup the product tree
    // a random file, should be skipped by loader
    Poco::Path file(path, "foo.bar");
    CPPUNIT_ASSERT(Poco::File(file).createFile());
        
    // create an invalid folder
    CPPUNIT_ASSERT(m_tempDir->createDirectory("SN-INVALID"));
    // valid product directories
    CPPUNIT_ASSERT(m_tempDir->createDirectory("SN-1"));
    CPPUNIT_ASSERT(m_tempDir->createDirectory("SN-2"));
    CPPUNIT_ASSERT(m_tempDir->createDirectory("SN-300"));
    // invalid seam series
    CPPUNIT_ASSERT(m_tempDir->createDirectory(std::string("SN-1/seam_series_invalid")));
    // valid seam series    
    CPPUNIT_ASSERT(m_tempDir->createDirectory(std::string("SN-1/seam_series1")));
    CPPUNIT_ASSERT(m_tempDir->createDirectory(std::string("SN-1/seam_series2")));
    CPPUNIT_ASSERT(m_tempDir->createDirectory(std::string("SN-1/seam_series3")));
    CPPUNIT_ASSERT(m_tempDir->createDirectory(std::string("SN-1/seam_series4")));
    CPPUNIT_ASSERT(m_tempDir->createDirectory(std::string("SN-2/seam_series1")));
    CPPUNIT_ASSERT(m_tempDir->createDirectory(std::string("SN-2/seam_series2")));
    CPPUNIT_ASSERT(m_tempDir->createDirectory(std::string("SN-2/seam_series3")));
    CPPUNIT_ASSERT(m_tempDir->createDirectory(std::string("SN-300/seam_series400")));
    // invalid seaml
    CPPUNIT_ASSERT(m_tempDir->createDirectory(std::string("SN-300/seam_series400/seam_invalid")));
    // valid seam
    CPPUNIT_ASSERT(m_tempDir->createDirectory(std::string("SN-1/seam_series1/seam1")));
    CPPUNIT_ASSERT(m_tempDir->createDirectory(std::string("SN-1/seam_series1/seam2")));
    CPPUNIT_ASSERT(m_tempDir->createDirectory(std::string("SN-1/seam_series1/seam3")));
    CPPUNIT_ASSERT(m_tempDir->createDirectory(std::string("SN-1/seam_series1/seam4")));
    CPPUNIT_ASSERT(m_tempDir->createDirectory(std::string("SN-1/seam_series1/seam5")));
    CPPUNIT_ASSERT(m_tempDir->createDirectory(std::string("SN-1/seam_series1/seam6")));
    // invalid things in a seam
    // directory is not allowed
    CPPUNIT_ASSERT(m_tempDir->createDirectory(std::string("SN-1/seam_series1/seam1/foo")));
    // files not called .bmp not allowed
    CPPUNIT_ASSERT(m_tempDir->createFile(std::string("SN-1/seam_series1/seam1/file.png")));
    // without number
    CPPUNIT_ASSERT(m_tempDir->createFile(std::string("SN-1/seam_series1/seam1/file.bmp")));
    
    // now create some real bitmaps
    CPPUNIT_ASSERT(m_tempDir->createBitmap(100, 200, 4, std::string("SN-1/seam_series1/seam1/1.bmp")));
    CPPUNIT_ASSERT(m_tempDir->createBitmap(100, 200, 5, std::string("SN-1/seam_series1/seam1/2.bmp")));
    CPPUNIT_ASSERT(m_tempDir->createBitmap(100, 200, 6, std::string("SN-1/seam_series1/seam1/3.bmp")));
    CPPUNIT_ASSERT(m_tempDir->createBitmap(100, 200, 7, std::string("SN-1/seam_series1/seam1/4.bmp")));
    // seam 2
    CPPUNIT_ASSERT(m_tempDir->createBitmap(200, 50, 3, std::string("SN-1/seam_series1/seam2/1.bmp")));
    CPPUNIT_ASSERT(m_tempDir->createBitmap(200, 50, 4, std::string("SN-1/seam_series1/seam2/2.bmp")));
    CPPUNIT_ASSERT(m_tempDir->createBitmap(200, 50, 5, std::string("SN-1/seam_series1/seam2/3.bmp")));
    CPPUNIT_ASSERT(m_tempDir->createBitmap(200, 50, 6, std::string("SN-1/seam_series1/seam2/4.bmp")));
    // create some VDR bitmaps
    CPPUNIT_ASSERT(m_tempDir->createBitmap(150, 80, 3, std::string("SN-1/seam_series1/seam3/1.bmp"), std::vector<unsigned char>{{char{0}, char{0}, char{0}, char{0}, char{0}, char{0}, char{0}, char{0} }}));
    CPPUNIT_ASSERT(m_tempDir->createBitmap(150, 80, 3, std::string("SN-1/seam_series1/seam3/2.bmp"), std::vector<unsigned char>{{char{0}, char{0}, char{0}, char{1}, char{1}, char{1}, char{5}, char{0} }}));
    
    // start loader
    m_loader.init(m_sharedMemory.get());
    // give worker thread some time to scanFolders
    Poco::Thread::sleep(1000);
}

void ImageLoaderTest::tearDown()
{
    m_loader.close();
}

void ImageLoaderTest::testImage(uint32_t productNumber, uint32_t seamSeriesNumber, uint32_t seamNumber, uint32_t imageNumber, int width, int height)
{
    ImageDataHolder dataHolder;
    CPPUNIT_ASSERT_EQUAL(true, m_loader.getImage(productNumber, seamSeriesNumber, seamNumber, imageNumber, dataHolder));
    CPPUNIT_ASSERT_EQUAL(false, dataHolder.getIsExtraDataValid());
    CPPUNIT_ASSERT_EQUAL(0, dataHolder.getImageNumber());
    CPPUNIT_ASSERT_EQUAL(m_tempDir->path() +
                         "/SN-" + std::to_string(productNumber) +
                         "/seam_series" + std::to_string(seamSeriesNumber) +
                         "/seam" + std::to_string(seamNumber) +
                         "/" + std::to_string(imageNumber) + ".bmp", dataHolder.getPath());
    CPPUNIT_ASSERT_EQUAL(std::to_string(productNumber) + "." + std::to_string(seamSeriesNumber) + "." + std::to_string(seamNumber) + "." + std::to_string(imageNumber), dataHolder.getKey());
    auto image = dataHolder.getByteImage();
    CPPUNIT_ASSERT_EQUAL(width, image.width());
    CPPUNIT_ASSERT_EQUAL(height, image.height());
}

void ImageLoaderTest::testImages()
{
    testImage(1, 1, 1, 1, 100, 200);
    testImage(1, 1, 1, 2, 100, 200);
    testImage(1, 1, 1, 3, 100, 200);
    testImage(1, 1, 1, 4, 100, 200);
    testImage(1, 1, 2, 1, 200, 50);
    testImage(1, 1, 2, 2, 200, 50);
    testImage(1, 1, 2, 3, 200, 50);
    testImage(1, 1, 2, 4, 200, 50);
}

void ImageLoaderTest::testVdrImage(uint32_t productNumber, uint32_t seamSeriesNumber, uint32_t seamNumber, uint32_t imageNumber, int width, int height, int offsetX, int offsetY, int vdrImageNumber)
{
    ImageDataHolder dataHolder;
    CPPUNIT_ASSERT_EQUAL(true, m_loader.getImage(productNumber, seamSeriesNumber, seamNumber, imageNumber, dataHolder));
    CPPUNIT_ASSERT_EQUAL(true, dataHolder.getIsExtraDataValid());
    CPPUNIT_ASSERT_EQUAL(offsetX, dataHolder.getHardwareRoiOffsetX());
    CPPUNIT_ASSERT_EQUAL(offsetY, dataHolder.getHardwareRoiOffsetY());
    CPPUNIT_ASSERT_EQUAL(vdrImageNumber, dataHolder.getImageNumber());
    CPPUNIT_ASSERT_EQUAL(m_tempDir->path() +
                         "/SN-" + std::to_string(productNumber) +
                         "/seam_series" + std::to_string(seamSeriesNumber) +
                         "/seam" + std::to_string(seamNumber) +
                         "/" + std::to_string(imageNumber) + ".bmp", dataHolder.getPath());
    CPPUNIT_ASSERT_EQUAL(std::to_string(productNumber) + "." + std::to_string(seamSeriesNumber) + "." + std::to_string(seamNumber) + "." + std::to_string(imageNumber), dataHolder.getKey());
    auto image = dataHolder.getByteImage();
    CPPUNIT_ASSERT_EQUAL(width, image.width());
    CPPUNIT_ASSERT_EQUAL(height, image.height());
}

void ImageLoaderTest::testVdrImages()
{
    testVdrImage(1, 1, 3, 1, 150, 80, 0, 0, 0);
    testVdrImage(1, 1, 3, 2, 150, 80, 256, 257, 5);
}

void ImageLoaderTest::testCacheMiss()
{
    ImageDataHolder dataHolder;
    CPPUNIT_ASSERT_EQUAL(false, m_loader.getImage(1, 2, 1, 1, dataHolder));
}

TEST_MAIN(ImageLoaderTest)
