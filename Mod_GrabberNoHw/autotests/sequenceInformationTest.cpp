#include "testHelper.h"

#include "sequenceInformation.h"

#include <filesystem>

namespace fs = std::filesystem;

using precitec::SequenceInformation;

class SequenceInformationTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(SequenceInformationTest);
    CPPUNIT_TEST(findProductFolders_withEmptyFolder_isEmpty);
    CPPUNIT_TEST(findProductFolders_withSingleFile_isEmpty);
    CPPUNIT_TEST(findProductFolders_withNestedEmptyFolders_isEmpty);
    CPPUNIT_TEST(findProductFolders_withoutIdFileInMatchingFolder_hasMapEntryWithoutUUID);
    CPPUNIT_TEST(findProductFolders_withMatchingFolder_hasFullMapEntry);
    CPPUNIT_TEST(findProductFolders_withMatchingBaseDir_hasFullMapEntry);
    CPPUNIT_TEST(findProductFolders_withMultipleMatchingFolders_hasMultipleMapEntries);
    CPPUNIT_TEST(scanFolders_withoutBaseDir_isEmpty);
    CPPUNIT_TEST(scanFolders_withEmptyBaseDir_isEmpty);
    CPPUNIT_TEST(scanFolders_withMatchingImage_containsEntryWithoutInstanceUUID);
    CPPUNIT_TEST(scanFolders_withImagesWithoutNumber_assignsSubsequentImageNumbers);
    CPPUNIT_TEST(scanFolders_withMatchingImageAndIdFile_containsFullEntry);
    CPPUNIT_TEST_SUITE_END();

    std::unique_ptr<TemporaryDirectory> m_tempDir;
    fs::path m_baseDir;
    const std::uint32_t m_serialNumber = 1234;
    const Poco::UUID m_productInstanceUUID{"351fe72b-991b-4e08-9438-789099f2e471"};
    const fs::path m_productInstanceFolderName = m_productInstanceUUID.toString() + "-SN-" + std::to_string(m_serialNumber);
    const fs::path m_idFileName = m_productInstanceUUID.toString() + ".id";

    SequenceInformation::ProductFolderMap_t m_folderMap;
public:
    void setUp() override
    {
        m_tempDir = std::make_unique<TemporaryDirectory>("SequenceInformationTest");
        m_baseDir = m_tempDir->path();
        m_folderMap.clear();
    }

    void findProductFolders_withEmptyFolder_isEmpty()
    {
        SequenceInformation underTest;
        bool folderFound = underTest.findProductFolders(m_baseDir, m_folderMap);

        CPPUNIT_ASSERT_EQUAL(folderFound, false);
        CPPUNIT_ASSERT(m_folderMap.empty());
    }

    void findProductFolders_withSingleFile_isEmpty()
    {
        CPPUNIT_ASSERT(m_tempDir->createFile("NoFolder.txt"));

        SequenceInformation underTest;
        const bool folderFound = underTest.findProductFolders(m_baseDir, m_folderMap);

        CPPUNIT_ASSERT_EQUAL(folderFound, false);
        CPPUNIT_ASSERT(m_folderMap.empty());
    }

    void findProductFolders_withNestedEmptyFolders_isEmpty()
    {
        CPPUNIT_ASSERT(m_tempDir->createDirectory("EmptyFolder1"));
        CPPUNIT_ASSERT(m_tempDir->createDirectory("EmptyFolder2"));

        SequenceInformation underTest;
        const bool folderFound = underTest.findProductFolders(m_baseDir, m_folderMap);

        CPPUNIT_ASSERT_EQUAL(folderFound, false);
        CPPUNIT_ASSERT(m_folderMap.empty());
    }

    void findProductFolders_withoutIdFileInMatchingFolder_hasMapEntryWithoutUUID()
    {
        CPPUNIT_ASSERT(m_tempDir->createDirectory(m_productInstanceFolderName));

        SequenceInformation underTest;
        const bool folderFound = underTest.findProductFolders(m_baseDir, m_folderMap);

        CPPUNIT_ASSERT_EQUAL(folderFound, true);
        CPPUNIT_ASSERT_EQUAL(m_folderMap.size(), std::size_t{1});
        const auto foundIt = m_folderMap.find(m_baseDir / m_productInstanceFolderName);
        CPPUNIT_ASSERT(foundIt != m_folderMap.end());
        CPPUNIT_ASSERT(foundIt->second.first == Poco::UUID{});
        CPPUNIT_ASSERT_EQUAL(foundIt->second.second, 1234u);
    }

    void findProductFolders_withMatchingFolder_hasFullMapEntry()
    {
        CPPUNIT_ASSERT(m_tempDir->createDirectory(m_productInstanceFolderName));
        CPPUNIT_ASSERT(m_tempDir->createFile(m_productInstanceFolderName / m_idFileName));

        SequenceInformation underTest;
        const bool folderFound = underTest.findProductFolders(m_baseDir, m_folderMap);

        CPPUNIT_ASSERT_EQUAL(folderFound, true);
        CPPUNIT_ASSERT_EQUAL(m_folderMap.size(), std::size_t{1});
        const auto foundIt = m_folderMap.find(m_baseDir / m_productInstanceFolderName);
        CPPUNIT_ASSERT(foundIt != m_folderMap.end());
        CPPUNIT_ASSERT(foundIt->second.first == m_productInstanceUUID);
        CPPUNIT_ASSERT_EQUAL(foundIt->second.second, 1234u);
    }

    void findProductFolders_withMatchingBaseDir_hasFullMapEntry()
    {
        CPPUNIT_ASSERT(m_tempDir->createDirectory(m_productInstanceFolderName));
        CPPUNIT_ASSERT(m_tempDir->createFile(m_productInstanceFolderName / m_idFileName));

        const fs::path differentBaseDir = m_baseDir / m_productInstanceFolderName;
        SequenceInformation underTest;
        const bool folderFound = underTest.findProductFolders(differentBaseDir, m_folderMap);

        CPPUNIT_ASSERT_EQUAL(folderFound, true);
        CPPUNIT_ASSERT_EQUAL(m_folderMap.size(), std::size_t{1});
        const auto foundIt = m_folderMap.find(differentBaseDir);
        CPPUNIT_ASSERT(foundIt != m_folderMap.end());
        CPPUNIT_ASSERT(foundIt->second.first == m_productInstanceUUID);
        CPPUNIT_ASSERT_EQUAL(foundIt->second.second, 1234u);
    }

    void findProductFolders_withMultipleMatchingFolders_hasMultipleMapEntries()
    {
        CPPUNIT_ASSERT(m_tempDir->createDirectory(m_productInstanceFolderName));
        CPPUNIT_ASSERT(m_tempDir->createFile(m_productInstanceFolderName / m_idFileName));

        const Poco::UUID productInstanceUUID2{"ec4aca06-1eca-418b-bc88-7c96118cc8c1"};
        const fs::path productInstanceFolderName2 = productInstanceUUID2.toString() + "-SN-9876";
        CPPUNIT_ASSERT(m_tempDir->createDirectory(productInstanceFolderName2));
        CPPUNIT_ASSERT(m_tempDir->createFile(productInstanceFolderName2 / (productInstanceUUID2.toString() + ".id")));

        SequenceInformation underTest;
        const bool folderFound = underTest.findProductFolders(m_baseDir, m_folderMap);

        CPPUNIT_ASSERT_EQUAL(folderFound, true);
        CPPUNIT_ASSERT_EQUAL(m_folderMap.size(), std::size_t{2});
        const auto foundIt = m_folderMap.find(m_baseDir / productInstanceFolderName2);
        CPPUNIT_ASSERT(foundIt != m_folderMap.end());
        CPPUNIT_ASSERT(foundIt->second.first == productInstanceUUID2);
        CPPUNIT_ASSERT_EQUAL(foundIt->second.second, 9876u);
    }

    void scanFolders_withoutBaseDir_isEmpty()
    {
        SequenceInformation underTest;
        underTest.scanFolders();
        CPPUNIT_ASSERT(underTest.imageAndSampleVector().empty());
    }

    void scanFolders_withEmptyBaseDir_isEmpty()
    {
        SequenceInformation underTest;
        underTest.setBasepath(m_baseDir);

        underTest.scanFolders();
        CPPUNIT_ASSERT(underTest.imageAndSampleVector().empty());
    }

    void scanFolders_withMatchingImage_containsEntryWithoutInstanceUUID()
    {
        const auto subDir = m_productInstanceFolderName / "seam_series0032" / "seam0123";
        const auto seamDir = m_baseDir / subDir;
        const auto fileName = fs::path{"7.bmp"};
        CPPUNIT_ASSERT(fs::create_directories(seamDir));
        CPPUNIT_ASSERT(m_tempDir->createBitmap(100, 200, 4, subDir / fileName));

        SequenceInformation underTest;
        underTest.setBasepath(m_baseDir);
        underTest.scanFolders();
        CPPUNIT_ASSERT(underTest.SampleFolderVector().empty());
        CPPUNIT_ASSERT_EQUAL(1ul, underTest.ImageFolderVector().size());
        const precitec::VdrFileInfo& result = underTest.ImageFolderVector().at(0);

        std::string expectedKey;
        underTest.makeVdrFileKey(m_serialNumber, 32, 123, 7, expectedKey);
        CPPUNIT_ASSERT_EQUAL((seamDir / fileName).string(), result.getPath());
        CPPUNIT_ASSERT_EQUAL(expectedKey, result.getKey());
        CPPUNIT_ASSERT_EQUAL(m_serialNumber, result.product());
        CPPUNIT_ASSERT_EQUAL(Poco::UUID{}, result.productInstance());
        CPPUNIT_ASSERT_EQUAL(123u, result.seam());
        CPPUNIT_ASSERT_EQUAL(32u, result.seamSeries());
        CPPUNIT_ASSERT_EQUAL(7u, result.image());
    }

    void scanFolders_withImagesWithoutNumber_assignsSubsequentImageNumbers()
    {
        const auto seamSubDir1 = m_productInstanceFolderName / "seam_series0032" / "seam0123";
        const auto seamSubDir2 = m_productInstanceFolderName / "seam_series0032" / "seam0456";
        CPPUNIT_ASSERT(fs::create_directories(m_baseDir / seamSubDir1));
        CPPUNIT_ASSERT(fs::create_directories(m_baseDir / seamSubDir2));
        CPPUNIT_ASSERT(m_tempDir->createBitmap(100, 200, 4, seamSubDir1 / "A.bmp"));
        CPPUNIT_ASSERT(m_tempDir->createBitmap(100, 200, 4, seamSubDir1 / "B.bmp"));
        CPPUNIT_ASSERT(m_tempDir->createBitmap(100, 200, 4, seamSubDir2 / "A.bmp"));
        CPPUNIT_ASSERT(m_tempDir->createBitmap(100, 200, 4, seamSubDir2 / "C.bmp"));

        SequenceInformation underTest;
        underTest.setBasepath(m_baseDir);
        underTest.scanFolders();
        
        CPPUNIT_ASSERT_EQUAL(4ul, underTest.ImageFolderVector().size());
        CPPUNIT_ASSERT_EQUAL(0u, underTest.ImageFolderVector().at(0).image());
        CPPUNIT_ASSERT_EQUAL(1u, underTest.ImageFolderVector().at(1).image());
        CPPUNIT_ASSERT_EQUAL(0u, underTest.ImageFolderVector().at(2).image());
        CPPUNIT_ASSERT_EQUAL(1u, underTest.ImageFolderVector().at(3).image());
    }

    void scanFolders_withMatchingImageAndIdFile_containsFullEntry()
    {
        const auto subDir = m_productInstanceFolderName / "seam_series0032" / "seam0123";
        const auto seamDir = m_baseDir / subDir;
        const auto fileName = fs::path{"7.bmp"};
        CPPUNIT_ASSERT(fs::create_directories(seamDir));
        CPPUNIT_ASSERT(m_tempDir->createBitmap(100, 200, 4, subDir / fileName));
        const auto idFileName = fs::path{m_productInstanceUUID.toString() + ".id"};
        std::ofstream idFile{m_baseDir / m_productInstanceFolderName / idFileName};

        SequenceInformation underTest;
        underTest.setBasepath(m_baseDir);
        underTest.scanFolders();
        CPPUNIT_ASSERT(underTest.SampleFolderVector().empty());
        CPPUNIT_ASSERT_EQUAL(1ul, underTest.ImageFolderVector().size());
        const precitec::VdrFileInfo& result = underTest.ImageFolderVector().at(0);

        std::string expectedKey;
        underTest.makeVdrFileKey(m_serialNumber, 32, 123, 7, expectedKey);
        CPPUNIT_ASSERT_EQUAL((seamDir / fileName).string(), result.getPath());
        CPPUNIT_ASSERT_EQUAL(expectedKey, result.getKey());
        CPPUNIT_ASSERT_EQUAL(m_serialNumber, result.product());
        CPPUNIT_ASSERT_EQUAL(m_productInstanceUUID, result.productInstance());
        CPPUNIT_ASSERT_EQUAL(123u, result.seam());
        CPPUNIT_ASSERT_EQUAL(32u, result.seamSeries());
        CPPUNIT_ASSERT_EQUAL(7u, result.image());

        // Data should be gone after reset.
        underTest.reset();
        CPPUNIT_ASSERT(underTest.imageAndSampleVector().empty());

        // This is one level too deep into the hierarchy and should not find the image.
        underTest.setBasepath(m_baseDir / m_productInstanceFolderName);
        underTest.scanFolders();

        CPPUNIT_ASSERT(underTest.imageAndSampleVector().empty());
    }
};

TEST_MAIN(SequenceInformationTest)