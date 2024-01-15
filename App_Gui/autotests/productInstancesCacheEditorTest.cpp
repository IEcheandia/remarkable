#include <QTest>
#include <QObject>
#include <QTemporaryDir>
#include <QSignalSpy>

#include <memory>

#include "../src/productInstancesCacheEditor.h"

class ProductInstancesCacheEditorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testInit();
    void testAdd();
};

void ProductInstancesCacheEditorTest::testInit()
{
    precitec::gui::ProductInstancesCacheEditor editor();
}

void ProductInstancesCacheEditorTest::testAdd()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QString productSubDirectory("video/WM-QNX-PC/f439a74d-8c56-41e9-abe3-d98d5dc0718b");
    QDir(dir.path()).mkpath(productSubDirectory);
    QString dataSubDirectory("data");
    QDir(dir.path()).mkpath(dataSubDirectory);
    QString cacheFullFileName(dir.path() + "/" + dataSubDirectory + "/" + ".video_recorder_product_cache");

    QString basicProductInstanceDirectory("04797302-1703-11ec-b44a-8c8caa6438b5-SN-916173026");
    std::vector<QString> productInstances;
    for (auto item = 0u; item < 10; item++)
    {
        auto productInstanceSubDirectory(productSubDirectory + "/" + basicProductInstanceDirectory +
                                         QString::number(item));
        QDir(dir.path()).mkpath(productInstanceSubDirectory);
        productInstances.push_back(dir.path() + "/" + productInstanceSubDirectory);
    }

    precitec::gui::ProductInstancesCacheEditor editor;
    editor.setAbsoluteCacheFileName(cacheFullFileName);
    std::size_t maxProductInstances = 5;
    editor.setProductInstanceMaxNumber(maxProductInstances);
    editor.add(productInstances);

    QDir productDirectory(dir.path() + "/" + productSubDirectory);
    productDirectory.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
    QCOMPARE(productDirectory.count(), maxProductInstances);
}

QTEST_GUILESS_MAIN(ProductInstancesCacheEditorTest)
#include "productInstancesCacheEditorTest.moc"
