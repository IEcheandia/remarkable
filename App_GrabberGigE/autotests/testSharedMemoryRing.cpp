#include <QTest>
#include <QSignalSpy>

#include "../src/camera.h"

using namespace precitec::grabber;

class TestSharedMemoryRing : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init();
    void initTestCase();
    void testInMemory_data();
    void testInMemory();

private:
    Camera m_camera;
    QTemporaryDir m_temporaryDir;
};

void TestSharedMemoryRing::initTestCase()
{
    QVERIFY(m_temporaryDir.isValid());
    qputenv("WM_BASE_DIR", m_temporaryDir.path().toUtf8());
    qputenv("WM_STATION_NAME", "AUTOTEST");
    m_camera.m_sharedMemory.init();
    m_camera.m_maxNumberImagesInSharedMemory = 100;
}

void TestSharedMemoryRing::init()
{
    m_camera.m_imagesInSharedMemory.clear();
}

void TestSharedMemoryRing::testInMemory_data()
{
    QTest::addColumn<std::size_t>("offset");

    QTest::newRow("front") << std::size_t{0};
    QTest::newRow("middle") << std::size_t(m_camera.m_sharedMemory.size() * 2);
    QTest::newRow("end") << m_camera.m_sharedMemory.size() - m_camera.m_sharedMemory.size() / m_camera.m_maxNumberImagesInSharedMemory;
}

void TestSharedMemoryRing::testInMemory()
{
    // no images in buffer yet
    QVERIFY(!m_camera.isImageInSharedMemory(0));

    QFETCH(std::size_t, offset);
    m_camera.m_sharedMemory.resetOffset(offset);

    const std::size_t sizeOfPointer = m_camera.m_sharedMemory.size() / m_camera.m_maxNumberImagesInSharedMemory;

    // create the buffers
    std::vector<byte*> pointers;
    pointers.reserve(m_camera.m_maxNumberImagesInSharedMemory * 2);
    for (std::size_t i = 0; i < m_camera.m_maxNumberImagesInSharedMemory * 2; i++)
    {
        auto ptr{m_camera.m_sharedMemory.nextImagePointer(sizeOfPointer)};
        pointers.push_back(ptr.get());
    }

    // now add to the ring
    // till safety margin everyting should work fine
    for (std::size_t i = 0; i < m_camera.m_maxNumberImagesInSharedMemory - 10; i++)
    {
        auto sharedMem = m_camera.m_sharedMemory.fromExistingPointer(pointers.at(i));
        precitec::image::Size2d size{100, 100};
        precitec::image::TLineImage<byte> image{sharedMem, size};

        m_camera.updateImageInSharedMemory(sharedMem, image, sizeOfPointer);
        QCOMPARE(std::get<0>(m_camera.m_imagesInSharedMemory.front()), 0);
        QCOMPARE(std::get<0>(m_camera.m_imagesInSharedMemory.back()), i);
        QVERIFY(m_camera.isImageInSharedMemory(0));
        QVERIFY(m_camera.isImageInSharedMemory(i));
    }
    // now let's add the next ten items, they should stay in m_imagesInSharedMemory
    // but first ten should be removed from isImageInSharedMemory
    {
        auto sharedMem = m_camera.m_sharedMemory.fromExistingPointer(pointers.at(90));
        precitec::image::Size2d size{100, 100};
        precitec::image::TLineImage<byte> image{sharedMem, size};

        m_camera.updateImageInSharedMemory(sharedMem, image, sizeOfPointer);
        QCOMPARE(std::get<0>(m_camera.m_imagesInSharedMemory.front()), 0);
        QCOMPARE(std::get<0>(m_camera.m_imagesInSharedMemory.back()), 90);
        QVERIFY(!m_camera.isImageInSharedMemory(0));
        QVERIFY(m_camera.isImageInSharedMemory(1));
        QVERIFY(m_camera.isImageInSharedMemory(90));
    }
    for (std::size_t i = 1u; i < 10; i++)
    {
        auto sharedMem = m_camera.m_sharedMemory.fromExistingPointer(pointers.at(90 + i));
        precitec::image::Size2d size{100, 100};
        precitec::image::TLineImage<byte> image{sharedMem, size};

        m_camera.updateImageInSharedMemory(sharedMem, image, sizeOfPointer);
        QCOMPARE(std::get<0>(m_camera.m_imagesInSharedMemory.front()), 0);
        QCOMPARE(std::get<0>(m_camera.m_imagesInSharedMemory.back()), 90 + i);
        QVERIFY(!m_camera.isImageInSharedMemory(i));
        QVERIFY(m_camera.isImageInSharedMemory(90 + i));
    }

    // now the ring should start removing buffers
    {
        auto sharedMem = m_camera.m_sharedMemory.fromExistingPointer(pointers.at(100));
        precitec::image::Size2d size{100, 100};
        precitec::image::TLineImage<byte> image{sharedMem, size};

        m_camera.updateImageInSharedMemory(sharedMem, image, sizeOfPointer);
        QCOMPARE(std::get<0>(m_camera.m_imagesInSharedMemory.front()), 1);
        QCOMPARE(std::get<0>(m_camera.m_imagesInSharedMemory.back()), 100);
        QVERIFY(!m_camera.isImageInSharedMemory(10));
        QVERIFY(m_camera.isImageInSharedMemory(11));
        QVERIFY(m_camera.isImageInSharedMemory(100));
    }

    // let's add the last remaining buffers we have

    for (std::size_t i = m_camera.m_maxNumberImagesInSharedMemory + 1; i < m_camera.m_maxNumberImagesInSharedMemory * 2; i++)
    {
        auto sharedMem = m_camera.m_sharedMemory.fromExistingPointer(pointers.at(i));
        precitec::image::Size2d size{100, 100};
        precitec::image::TLineImage<byte> image{sharedMem, size};

        m_camera.updateImageInSharedMemory(sharedMem, image, sizeOfPointer);
        QCOMPARE(std::get<0>(m_camera.m_imagesInSharedMemory.front()), i - m_camera.m_maxNumberImagesInSharedMemory + 1);
        QCOMPARE(std::get<0>(m_camera.m_imagesInSharedMemory.back()), i);
        QVERIFY(!m_camera.isImageInSharedMemory(i - m_camera.m_maxNumberImagesInSharedMemory + 10));
        QVERIFY(m_camera.isImageInSharedMemory(i - m_camera.m_maxNumberImagesInSharedMemory + 11));
        QVERIFY(m_camera.isImageInSharedMemory(i));
    }
}

QTEST_GUILESS_MAIN(TestSharedMemoryRing)
#include "testSharedMemoryRing.moc"
