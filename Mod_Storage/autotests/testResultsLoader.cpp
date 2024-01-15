#include <QTest>
#include <QSignalSpy>

#include "../src/product.h"
#include "../src/seam.h"
#include "../src/seamSeries.h"
#include "../src/resultsSerializer.h"
#include "../src/resultsLoader.h"

#include "event/results.h"

using precitec::storage::Product;
using precitec::storage::ResultsSerializer;
using precitec::storage::ResultsLoader;

class TestResultsLoader : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testSetSeam();
    void testSetProductInstance();
    void testLoading();
    void testLoadingNoResults();
};

void TestResultsLoader::testCtor()
{
    ResultsLoader loader;
    QCOMPARE(loader.seam(), -1);
    QCOMPARE(loader.seamSeries(), -1);
    QCOMPARE(loader.isLoading(), false);
    QCOMPARE(loader.productInstance().isFile(), false);
    QCOMPARE(loader.productInstance().isDir(), false);
    QCOMPARE(loader.productInstance().exists(), false);
}

void TestResultsLoader::testSetSeam()
{
    // need to load some test data
    const QString fileName = QFINDTESTDATA("testdata/products/seamuuid.json");
    QVERIFY(!fileName.isEmpty());
    std::unique_ptr<Product> product{Product::fromJson(fileName)};
    QVERIFY(product);
    auto seam = product->findSeam(QUuid{QStringLiteral("3F086211-FBD4-4493-A580-6FF11E4925DF")});
    QVERIFY(seam);

    ResultsLoader loader;
    QSignalSpy seamChangedSpy(&loader, &ResultsLoader::seamChanged);
    QVERIFY(seamChangedSpy.isValid());
    QSignalSpy loadingChangedSpy(&loader, &ResultsLoader::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());

    QCOMPARE(loader.seam(), -1);

    // setting to same seam chould not emit signal
    loader.setSeam(-1);
    QCOMPARE(seamChangedSpy.count(), 0);
    // setting to other seam should emit signal
    loader.setSeam(seam->number());
    QCOMPARE(loader.seam(), seam->number());
    QCOMPARE(seamChangedSpy.count(), 1);
    // again setting to same seam should not emit signal
    loader.setSeam(seam->number());
    QCOMPARE(seamChangedSpy.count(), 1);

    // the loading signal should never have been emitted as directory is not set
    QCOMPARE(loadingChangedSpy.count(), 0);
}

void TestResultsLoader::testSetProductInstance()
{
    QTemporaryDir dir;
    QFileInfo info(dir.path());
    QCOMPARE(info.isDir(), true);

    ResultsLoader loader;
    QSignalSpy productInstanceChangedSpy(&loader, &ResultsLoader::productInstanceChanged);
    QVERIFY(productInstanceChangedSpy.isValid());
    QSignalSpy loadingChangedSpy(&loader, &ResultsLoader::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());

    QCOMPARE(loader.productInstance().isDir(), false);

    // setting a proper one should work
    loader.setProductInstance(info);
    QCOMPARE(loader.productInstance(), info);
    QCOMPARE(productInstanceChangedSpy.count(), 1);

    // setting same again should not emit signal
    loader.setProductInstance(info);
    QCOMPARE(productInstanceChangedSpy.count(), 1);

    // setting to an invalid should change
    loader.setProductInstance(QFileInfo());
    QCOMPARE(productInstanceChangedSpy.count(), 2);

    // the loading signal should never have been emitted as directory is not set
    QCOMPARE(loadingChangedSpy.count(), 0);
}

void TestResultsLoader::testLoading()
{
    // first create some result files
    QTemporaryDir dir;
    QString subDir = QStringLiteral("/seam_series0002/seam0002/");
    QVERIFY(QDir{dir.path()}.mkpath(dir.path() + subDir));
    for (int i = 0; i < 5; i++)
    {
        ResultsSerializer serializer;
        serializer.setDirectory(dir.path() + subDir);
        serializer.setFileName(QString::number(i) + QStringLiteral(".result"));

        precitec::interface::GeoDoublearray values;
        values.ref().getData().push_back(0.0);
        values.ref().getData().push_back(0.1);
        values.ref().getData().push_back(1.0);
        values.ref().getRank().assign(3, 255);
        precitec::interface::ResultDoubleArray result{Poco::UUIDGenerator().createRandom(), precitec::interface::ResultType(i), precitec::interface::XCoordOutOfLimits, precitec::interface::ImageContext{}, values, precitec::geo2d::TRange<double>(0.0, 2.0), true};

        serializer.serialize(std::vector<precitec::interface::ResultArgs>{result});

        QFile file(dir.path() + subDir + QString::number(i) + QStringLiteral(".result"));
        QCOMPARE(file.exists(), true);
    }

    // need to load some test data
    const QString fileName = QFINDTESTDATA("testdata/products/seamuuid.json");
    QVERIFY(!fileName.isEmpty());
    std::unique_ptr<Product> product{Product::fromJson(fileName)};
    QVERIFY(product);
    auto seam = product->findSeam(QUuid{QStringLiteral("3F086211-FBD4-4493-A580-6FF11E4925DF")});
    QVERIFY(seam);

    ResultsLoader loader;
    QSignalSpy resultsLoadedSpy(&loader, &ResultsLoader::resultsLoaded);
    QVERIFY(resultsLoadedSpy.isValid());
    QSignalSpy loadingChangedSpy(&loader, &ResultsLoader::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());

    loader.setProductInstance(QFileInfo(dir.path()));
    QCOMPARE(loader.isLoading(), false);
    loader.setSeam(seam->number());
    loader.setSeamSeries(seam->seamSeries()->number());
    QCOMPARE(loader.isLoading(), true);
    QCOMPARE(loadingChangedSpy.count(), 1);

    QVERIFY(resultsLoadedSpy.wait());
    QCOMPARE(resultsLoadedSpy.count(), 1);
    QCOMPARE(loader.isLoading(), false);
    QCOMPARE(loadingChangedSpy.count(), 2);

    QCOMPARE(loader.results().size(), 5);
    const auto vec{std::move(loader.takeResults())};
    QCOMPARE(vec.size(), 5);
    QCOMPARE(loader.results().size(), 0);
}

void TestResultsLoader::testLoadingNoResults()
{
    // no result files in the directory
    QTemporaryDir dir;
    QString subDir = QStringLiteral("/seam_series0002/seam0002/");
    QVERIFY(QDir{dir.path()}.mkpath(dir.path() + subDir));

    // need to load some test data
    const QString fileName = QFINDTESTDATA("testdata/products/seamuuid.json");
    QVERIFY(!fileName.isEmpty());
    std::unique_ptr<Product> product{Product::fromJson(fileName)};
    QVERIFY(product);
    auto seam = product->findSeam(QUuid{QStringLiteral("3F086211-FBD4-4493-A580-6FF11E4925DF")});
    QVERIFY(seam);

    ResultsLoader loader;
    QSignalSpy resultsLoadedSpy(&loader, &ResultsLoader::resultsLoaded);
    QVERIFY(resultsLoadedSpy.isValid());
    QSignalSpy loadingChangedSpy(&loader, &ResultsLoader::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());

    loader.setProductInstance(QFileInfo(dir.path()));
    QCOMPARE(loader.isLoading(), false);
    loader.setSeam(seam->number());
    loader.setSeamSeries(seam->seamSeries()->number());
    QCOMPARE(loader.isLoading(), false);
    QCOMPARE(loadingChangedSpy.count(), 0);
    QCOMPARE(resultsLoadedSpy.count(), 1);
    QCOMPARE(loader.results().size(), 0);
}

QTEST_GUILESS_MAIN(TestResultsLoader)
#include "testResultsLoader.moc"
