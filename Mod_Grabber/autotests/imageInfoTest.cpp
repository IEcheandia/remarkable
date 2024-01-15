/**
*
* @defgroup ModGrabber
*
* @file
* @brief  Test case for ImageInfo
* @copyright    Precitec GmbH & Co. KG
* @author GM
*/
#include "testHelper.h"
#include "trigger/vdrFileInfo.h"

using precitec::VdrFileInfo;

class ImageInfoTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(ImageInfoTest);
CPPUNIT_TEST(testDefaultCtor);
CPPUNIT_TEST(testPath);
CPPUNIT_TEST(testKey);
CPPUNIT_TEST_SUITE_END();

public:
    void testDefaultCtor();
    void testPath();
    void testKey();
};

void ImageInfoTest::testDefaultCtor()
{
    VdrFileInfo imageInfo;
    CPPUNIT_ASSERT_EQUAL(std::string(), imageInfo.getPath());
    CPPUNIT_ASSERT_EQUAL(std::string(), imageInfo.getKey());
}

void ImageInfoTest::testPath()
{
    VdrFileInfo imageInfo;
    CPPUNIT_ASSERT_EQUAL(std::string(), imageInfo.getPath());
    imageInfo.setPath(std::string("foo"));
    CPPUNIT_ASSERT_EQUAL(std::string("foo"), imageInfo.getPath());
    imageInfo.setPath(std::string("bar"));
    CPPUNIT_ASSERT_EQUAL(std::string("bar"), imageInfo.getPath());
}

void ImageInfoTest::testKey()
{
    VdrFileInfo imageInfo;
    CPPUNIT_ASSERT_EQUAL(std::string(), imageInfo.getKey());
    imageInfo.setKey(std::string("foo"));
    CPPUNIT_ASSERT_EQUAL(std::string("foo"), imageInfo.getKey());
    imageInfo.setKey(std::string("bar"));
    CPPUNIT_ASSERT_EQUAL(std::string("bar"), imageInfo.getKey());
}

TEST_MAIN(ImageInfoTest)
