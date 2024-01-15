#include <QTest>
#include <QObject>
#include <QTemporaryDir>
#include <QSignalSpy>
#include <QString>

#include "../src/productInstancesCacheController.h"

#include <fstream>

class ProductInstancesCacheControllerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init()
    {
        m_controller.clearCacheBuffer();
    };

    void testInit();
    void testAddProductInstancePathToBuffer();
    void testCacheBuffer();

private:
    static quint32 numberOfLinesInFile(const QString &fileName);
    static quint32 numberOfDirectoriesInFolder(const QString &folder);

    std::vector<QString> getTestProductInstances(int productInstanceNumber,
                                                 const QTemporaryDir &dir,
                                                 const QString &productSubDirectory);
    QString getDataFile(const QTemporaryDir &dir);

    precitec::gui::ProductInstancesCacheController m_controller;
    const QString m_productSubDirectory = "video/WM-QNX-PC/f439a74d-8c56-41e9-abe3-d98d5dc0718b";
    const QString m_dataSubDirectory = "data";
    const QString m_cacheFileName = ".video_recorder_product_cache";
    const QString m_basicProductInstanceDirectoryPattern = "04797302-1703-11ec-b44a-8c8caa6438b5-SN-916173026";
    const uint productInstanceNumber = 10;
};

void ProductInstancesCacheControllerTest::testInit()
{
    QCOMPARE(m_controller.buffer().size(), 0);
}

void ProductInstancesCacheControllerTest::testAddProductInstancePathToBuffer()
{
    m_controller.addProductInstancePathToBuffer("test");
    QCOMPARE(m_controller.buffer().size(), 1);
}

void ProductInstancesCacheControllerTest::testCacheBuffer()
{
    // given
    QTemporaryDir dir;
    auto productInstances = getTestProductInstances(productInstanceNumber, dir, m_productSubDirectory);

    // when
    m_controller.setAbsoluteCacheFileName(getDataFile(dir));
    for (const auto &productInstance : productInstances)
    {
        m_controller.addProductInstancePathToBuffer(productInstance);
    }
    m_controller.cacheBuffer();

    // then
    QCOMPARE(numberOfLinesInFile(getDataFile(dir)), productInstanceNumber);
}

std::vector<QString> ProductInstancesCacheControllerTest::getTestProductInstances(int productInstanceNumber,
                                                                                  const QTemporaryDir &dir,
                                                                                  const QString &productSubDirectory)
{
    QDir(dir.path()).mkpath(productSubDirectory);
    QDir(dir.path()).mkpath(m_dataSubDirectory);

    std::vector<QString> productInstances;
    for (auto item = 0u; item < uint(productInstanceNumber); item++)
    {
        auto productInstanceSubDirectory(productSubDirectory + "/" + m_basicProductInstanceDirectoryPattern +
                                         QString::number(item));
        QDir(dir.path()).mkpath(productInstanceSubDirectory);
        productInstances.push_back(dir.path() + "/" + productInstanceSubDirectory);
    }

    return productInstances;
}

QString ProductInstancesCacheControllerTest::getDataFile(const QTemporaryDir &dir)
{
    return dir.path() + "/" + m_dataSubDirectory + "/" + m_cacheFileName;
}

quint32 ProductInstancesCacheControllerTest::numberOfLinesInFile(const QString &fileName)
{
    quint32 numberOfLines = 0;
    std::string line;
    std::ifstream file(fileName.toStdString());

    if (!file.is_open())
    {
        return 0;
    }

    while (std::getline(file, line))
    {
        ++numberOfLines;
    }

    return numberOfLines;
}

quint32 ProductInstancesCacheControllerTest::numberOfDirectoriesInFolder(const QString &folder)
{
    QDir productDirectory(folder);
    if (!productDirectory.exists())
    {
        return 0;
    };

    productDirectory.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
    return productDirectory.count();
}

QTEST_GUILESS_MAIN(ProductInstancesCacheControllerTest)
#include "productInstancesCacheControllerTest.moc"