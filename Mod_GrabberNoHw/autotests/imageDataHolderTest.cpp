/**
*
* @defgroup ModGrabber
*
* @file
* @brief  Test case for ImageDataHolder
* @copyright    Precitec GmbH & Co. KG
* @author GM
*/
#include "testHelper.h"
#include "../include/trigger/imageDataHolder.h"

using precitec::ImageDataHolder;
using precitec::image::BImage;

class ImageDataHolderTest : public CppUnit::TestFixture
{

CPPUNIT_TEST_SUITE(ImageDataHolderTest);
CPPUNIT_TEST(testDefaultCtor);
CPPUNIT_TEST(testPath);
CPPUNIT_TEST(testKey);
CPPUNIT_TEST(testImageNumber);
CPPUNIT_TEST(testHardwareRoiOffsetX);
CPPUNIT_TEST(testHardwareRoiOffsetY);
CPPUNIT_TEST(testExtraDataValid);
CPPUNIT_TEST(testImage);
CPPUNIT_TEST_SUITE_END();

public:
    void testDefaultCtor();
    void testPath();
    void testKey();
    void testImageNumber();
    void testHardwareRoiOffsetX();
    void testHardwareRoiOffsetY();
    void testExtraDataValid();
    void testImage();
};

void ImageDataHolderTest::testDefaultCtor()
{
    ImageDataHolder dataHolder;
    CPPUNIT_ASSERT_EQUAL(std::string(), dataHolder.getPath());
    CPPUNIT_ASSERT_EQUAL(std::string(), dataHolder.getKey());
    CPPUNIT_ASSERT_EQUAL(0, dataHolder.getImageNumber());
    CPPUNIT_ASSERT_EQUAL(0, dataHolder.getHardwareRoiOffsetX());
    CPPUNIT_ASSERT_EQUAL(0, dataHolder.getHardwareRoiOffsetY());
    CPPUNIT_ASSERT_EQUAL(false, dataHolder.getIsExtraDataValid());

    // TODO: test default constructed BImage
    // cannot easily be done as it's missing operator=
}

void ImageDataHolderTest::testPath()
{
    ImageDataHolder dataHolder;
    CPPUNIT_ASSERT_EQUAL(std::string(), dataHolder.getPath());
    dataHolder.setPath(std::string("foo"));
    CPPUNIT_ASSERT_EQUAL(std::string("foo"), dataHolder.getPath());
    dataHolder.setPath(std::string("bar"));
    CPPUNIT_ASSERT_EQUAL(std::string("bar"), dataHolder.getPath());
}

void ImageDataHolderTest::testKey()
{
    ImageDataHolder dataHolder;
    CPPUNIT_ASSERT_EQUAL(std::string(), dataHolder.getKey());
    dataHolder.setKey(std::string("foo"));
    CPPUNIT_ASSERT_EQUAL(std::string("foo"), dataHolder.getKey());
    dataHolder.setKey(std::string("bar"));
    CPPUNIT_ASSERT_EQUAL(std::string("bar"), dataHolder.getKey());
}

void ImageDataHolderTest::testImageNumber()
{
    ImageDataHolder dataHolder;
    CPPUNIT_ASSERT_EQUAL(0, dataHolder.getImageNumber());
    dataHolder.setImageNumber(1);
    CPPUNIT_ASSERT_EQUAL(1, dataHolder.getImageNumber());
    dataHolder.setImageNumber(0);
    CPPUNIT_ASSERT_EQUAL(0, dataHolder.getImageNumber());
    dataHolder.setImageNumber(100);
    CPPUNIT_ASSERT_EQUAL(100, dataHolder.getImageNumber());
}

void ImageDataHolderTest::testHardwareRoiOffsetX()
{
    ImageDataHolder dataHolder;
    CPPUNIT_ASSERT_EQUAL(0, dataHolder.getHardwareRoiOffsetX());
    dataHolder.setHardwareRoiOffsetX(1);
    CPPUNIT_ASSERT_EQUAL(1, dataHolder.getHardwareRoiOffsetX());
    dataHolder.setHardwareRoiOffsetX(0);
    CPPUNIT_ASSERT_EQUAL(0, dataHolder.getHardwareRoiOffsetX());
    dataHolder.setHardwareRoiOffsetX(100);
    CPPUNIT_ASSERT_EQUAL(100, dataHolder.getHardwareRoiOffsetX());
}

void ImageDataHolderTest::testHardwareRoiOffsetY()
{
    ImageDataHolder dataHolder;
    CPPUNIT_ASSERT_EQUAL(0, dataHolder.getHardwareRoiOffsetY());
    dataHolder.setHardwareRoiOffsetY(1);
    CPPUNIT_ASSERT_EQUAL(1, dataHolder.getHardwareRoiOffsetY());
    dataHolder.setHardwareRoiOffsetY(0);
    CPPUNIT_ASSERT_EQUAL(0, dataHolder.getHardwareRoiOffsetY());
    dataHolder.setHardwareRoiOffsetY(100);
    CPPUNIT_ASSERT_EQUAL(100, dataHolder.getHardwareRoiOffsetY());
}

void ImageDataHolderTest::testExtraDataValid()
{
    ImageDataHolder dataHolder;
    CPPUNIT_ASSERT_EQUAL(false, dataHolder.getIsExtraDataValid());
    dataHolder.setIsExtraDataValid(true);
    CPPUNIT_ASSERT_EQUAL(true, dataHolder.getIsExtraDataValid());
    dataHolder.setIsExtraDataValid(false);
    CPPUNIT_ASSERT_EQUAL(false, dataHolder.getIsExtraDataValid());
}

void ImageDataHolderTest::testImage()
{
    const int seed = 5;
    const BImage image = precitec::image::genModuloPattern({20, 30}, seed);
    ImageDataHolder dataHolder;
    dataHolder.setByteImage(image);
    CPPUNIT_ASSERT_EQUAL(20, dataHolder.getByteImage().width());
    CPPUNIT_ASSERT_EQUAL(30, dataHolder.getByteImage().height());

    for (int y=0; y<image.height(); ++y)
    {
        for (int x=0; x<image.width(); ++x)
        {
            CPPUNIT_ASSERT_EQUAL(byte(x % seed), dataHolder.getByteImage()[y][x]);
        }
    }
}

TEST_MAIN(ImageDataHolderTest)
