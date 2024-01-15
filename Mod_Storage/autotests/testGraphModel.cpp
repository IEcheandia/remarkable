#include <QTest>
#include <QSaveFile>
#include <QSignalSpy>

#include "../src/graphModel.h"

using precitec::storage::GraphModel;

class GraphModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testLoad();
    void testLoadNoFiles();
    void testLoadFailed();
    void testLoadMultipleDirectories();
    void testGraphAdding();
    void testGraphModifying();
    void testUserGraphOverwritesSystemGraph();
};

void GraphModelTest::testCtor()
{
    GraphModel model;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.name(QUuid::createUuid()), QString());
    QCOMPARE(model.indexFor(QUuid::createUuid()).isValid(), false);
    QCOMPARE(model.graph(model.index(0, 0)).id, fliplib::GraphContainer().id);
    QCOMPARE(model.data(model.index(0, 0)).isValid(), false);
    QCOMPARE(model.isLoading(), false);
    QCOMPARE(model.property("loading").toBool(), false);

    const auto roleNames = model.roleNames();
    QCOMPARE(roleNames.size(), 7);
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("name"));
    QCOMPARE(roleNames[Qt::UserRole], QByteArrayLiteral("uuid"));
    QCOMPARE(roleNames[Qt::UserRole + 1], QByteArrayLiteral("groups"));
    QCOMPARE(roleNames[Qt::UserRole + 5], QByteArrayLiteral("image"));
    QCOMPARE(roleNames[Qt::UserRole + 6], QByteArrayLiteral("comment"));
    QCOMPARE(roleNames[Qt::UserRole + 7], QByteArrayLiteral("pdfAvailable"));
    QCOMPARE(roleNames[Qt::UserRole + 8], QByteArrayLiteral("groupName"));
}

void GraphModelTest::testLoad()
{
    GraphModel model;
    QSignalSpy modelResetSpy{&model, &GraphModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    QSignalSpy loadingChangedSpy{&model, &GraphModel::loadingChanged};
    QVERIFY(loadingChangedSpy.isValid());

    model.loadGraphs(QFINDTESTDATA("testdata/graphs/"));
    QCOMPARE(loadingChangedSpy.count(), 1);
    QCOMPARE(model.isLoading(), true);
    QCOMPARE(model.property("loading").toBool(), true);
    QCOMPARE(modelResetSpy.count(), 0);
    QVERIFY(loadingChangedSpy.wait());
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(model.isLoading(), false);
    QCOMPARE(model.property("loading").toBool(), false);
    QCOMPARE(modelResetSpy.count(), 1);

    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.index(0, 0).isValid(), true);
    QCOMPARE(model.rowCount(model.index(0, 0)), 0);
    QCOMPARE(model.index(0, 0).data(), QStringLiteral("Minimal"));
    QCOMPARE(model.index(0, 0).data(Qt::UserRole).toUuid(), QUuid(QStringLiteral("e58abf42-77a6-4456-9f78-56e002b38549")));
    QCOMPARE(model.index(0, 0).data(Qt::UserRole+1).toStringList(), QStringList() << QStringLiteral("Not grouped"));
    QCOMPARE(model.index(0, 0).data(Qt::UserRole+2).isValid(), false);
    QCOMPARE(model.index(0, 0).data(Qt::UserRole+6).toString(), QStringLiteral("This is a comment"));

    QCOMPARE(model.name(QUuid(QStringLiteral("e58abf42-77a6-4456-9f78-56e002b38549"))), QStringLiteral("Minimal"));
    QCOMPARE(model.name(QUuid::createUuid()), QString());

    QCOMPARE(model.indexFor(QUuid(QStringLiteral("e58abf42-77a6-4456-9f78-56e002b38549"))), model.index(0, 0));
    QCOMPARE(model.indexFor(QUuid::createUuid()).isValid(), false);

    QCOMPARE(model.graph(model.index(0, 0)).id, Poco::UUID("e58abf42-77a6-4456-9f78-56e002b38549"));
    QCOMPARE(model.graph(model.index(1, 0)).id.isNull(), true);
}

void GraphModelTest::testLoadNoFiles()
{
    GraphModel model;
    QSignalSpy loadingChangedSpy{&model, &GraphModel::loadingChanged};
    QVERIFY(loadingChangedSpy.isValid());

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    model.loadGraphs(dir.path());
    QCOMPARE(model.isLoading(), false);
    QCOMPARE(loadingChangedSpy.count(), 1);
}

void GraphModelTest::testLoadFailed()
{
    GraphModel model;
    QSignalSpy loadingChangedSpy{&model, &GraphModel::loadingChanged};
    QVERIFY(loadingChangedSpy.isValid());

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/graphs/invalid.xml"), dir.filePath(QStringLiteral("invalid.xml"))));

    model.loadGraphs(dir.path());
    QCOMPARE(model.isLoading(), true);
    QCOMPARE(loadingChangedSpy.count(), 1);
    QVERIFY(loadingChangedSpy.wait());
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(model.isLoading(), false);

    QCOMPARE(model.rowCount(), 0);
}

void GraphModelTest::testLoadMultipleDirectories()
{
    GraphModel model;
    QSignalSpy loadingChangedSpy{&model, &GraphModel::loadingChanged};
    QVERIFY(loadingChangedSpy.isValid());

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/graphs/invalid.xml"), dir.filePath(QStringLiteral("invalid.xml"))));

    model.loadGraphs(dir.path(), QFINDTESTDATA("testdata/graphs/"));
    QCOMPARE(model.isLoading(), true);
    QCOMPARE(loadingChangedSpy.count(), 1);
    QVERIFY(loadingChangedSpy.wait());
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(model.isLoading(), false);

    QCOMPARE(model.rowCount(), 1);
}

void GraphModelTest::testGraphAdding()
{
    GraphModel model;
    QSignalSpy modelResetSpy{&model, &GraphModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    QSignalSpy loadingChangedSpy{&model, &GraphModel::loadingChanged};
    QVERIFY(loadingChangedSpy.isValid());
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QCOMPARE(modelResetSpy.count(), 0);
    model.loadGraphs(dir.path(), QFINDTESTDATA("testdata/graphs/"));
    QCOMPARE(model.isLoading(), true);
    QCOMPARE(loadingChangedSpy.count(), 1);
    QVERIFY(loadingChangedSpy.wait());
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(model.isLoading(), false);
    QCOMPARE(modelResetSpy.count(), 1);

    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/graphs/invalid.xml"), dir.filePath(QStringLiteral("invalid.xml"))));
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 2);
    QTRY_COMPARE(loadingChangedSpy.count(), 4);
    QCOMPARE(model.isLoading(), false);

    // delete again
    QVERIFY(QFile::remove(dir.filePath(QStringLiteral("invalid.xml"))));
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 3);
    QTRY_COMPARE(loadingChangedSpy.count(), 6);
    QCOMPARE(model.isLoading(), false);

    // add some other file
    model.loadGraphs(dir.path());
    QCOMPARE(modelResetSpy.count(), 4);
    QCOMPARE(loadingChangedSpy.count(), 7);
    QCOMPARE(model.isLoading(), false);
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/products/empty.json"), dir.filePath(QStringLiteral("empty.json"))));
    QVERIFY(!modelResetSpy.wait(500));
    QCOMPARE(modelResetSpy.count(), 4);
    QTRY_COMPARE(loadingChangedSpy.count(), 8);
    QCOMPARE(model.isLoading(), false);
}

void GraphModelTest::testGraphModifying()
{
    GraphModel model;
    QSignalSpy modelResetSpy{&model, &GraphModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    QSignalSpy loadingChangedSpy{&model, &GraphModel::loadingChanged};
    QVERIFY(loadingChangedSpy.isValid());
    QSignalSpy dataChangedSpy{&model, &GraphModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    QTemporaryDir tempDir{};
    QVERIFY(tempDir.isValid());

    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/graphs/minimal.xml"), tempDir.filePath(QStringLiteral("minimal.xml"))));

    model.loadGraphs(tempDir.path());
    QCOMPARE(loadingChangedSpy.count(), 1);
    QCOMPARE(model.isLoading(), true);
    QCOMPARE(model.property("loading").toBool(), true);
    QCOMPARE(modelResetSpy.count(), 0);
    QVERIFY(loadingChangedSpy.wait());
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(model.isLoading(), false);
    QCOMPARE(model.property("loading").toBool(), false);
    QCOMPARE(modelResetSpy.count(), 1);

    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.index(0, 0).isValid(), true);
    QCOMPARE(model.rowCount(model.index(0, 0)), 0);
    QCOMPARE(model.index(0, 0).data(), QStringLiteral("Minimal"));
    QCOMPARE(model.index(0, 0).data(Qt::UserRole).toUuid(), QUuid(QStringLiteral("e58abf42-77a6-4456-9f78-56e002b38549")));
    QCOMPARE(model.index(0, 0).data(Qt::UserRole+1).toStringList(), QStringList() << QStringLiteral("Not grouped"));
    QCOMPARE(model.index(0, 0).data(Qt::UserRole+2).isValid(), false);

    QCOMPARE(model.name(QUuid(QStringLiteral("e58abf42-77a6-4456-9f78-56e002b38549"))), QStringLiteral("Minimal"));
    QCOMPARE(model.name(QUuid::createUuid()), QString());

    QCOMPARE(model.indexFor(QUuid(QStringLiteral("e58abf42-77a6-4456-9f78-56e002b38549"))), model.index(0, 0));
    QCOMPARE(model.indexFor(QUuid::createUuid()).isValid(), false);

    QCOMPARE(model.graph(model.index(0, 0)).id, Poco::UUID("e58abf42-77a6-4456-9f78-56e002b38549"));
    QCOMPARE(model.graph(model.index(1, 0)).id.isNull(), true);

    QVERIFY(dataChangedSpy.isEmpty());

    QFile readFile{tempDir.filePath(QStringLiteral("minimal.xml"))};
    QVERIFY(readFile.open(QIODevice::ReadOnly));
    QByteArray data = readFile.readAll();
    readFile.close();

    QSaveFile xmlFile{tempDir.filePath(QStringLiteral("minimal.xml"))};
    QVERIFY(xmlFile.open(QIODevice::WriteOnly));
    xmlFile.write(data);
    QVERIFY(xmlFile.commit());
    QTRY_COMPARE(dataChangedSpy.count(), 1);

    QCOMPARE(dataChangedSpy.at(0).at(0).toModelIndex(), model.index(0, 0));
    QCOMPARE(dataChangedSpy.at(0).at(1).toModelIndex(), model.index(0, 0));
    QCOMPARE(dataChangedSpy.at(0).at(0).value<QVector<int>>().empty(), true);
}

void GraphModelTest::testUserGraphOverwritesSystemGraph()
{
    GraphModel model;
    QSignalSpy modelResetSpy{&model, &GraphModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    QSignalSpy loadingChangedSpy{&model, &GraphModel::loadingChanged};
    QVERIFY(loadingChangedSpy.isValid());
    QSignalSpy dataChangedSpy{&model, &GraphModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    QTemporaryDir tempDir{};
    QVERIFY(tempDir.isValid());

    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/graphs/minimal.xml"), tempDir.filePath(QStringLiteral("minimal.xml"))));

    model.loadGraphs(QFINDTESTDATA("testdata/graphs/"), tempDir.path());
    QCOMPARE(loadingChangedSpy.count(), 1);
    QCOMPARE(model.isLoading(), true);
    QCOMPARE(model.property("loading").toBool(), true);
    QCOMPARE(modelResetSpy.count(), 0);
    QVERIFY(loadingChangedSpy.wait());
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(model.isLoading(), false);
    QCOMPARE(model.property("loading").toBool(), false);
    QCOMPARE(modelResetSpy.count(), 1);

    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.index(0, 0).isValid(), true);
    QCOMPARE(model.rowCount(model.index(0, 0)), 0);
    QCOMPARE(model.index(0, 0).data(), QStringLiteral("Minimal"));
    QCOMPARE(model.index(0, 0).data(Qt::UserRole).toUuid(), QUuid(QStringLiteral("e58abf42-77a6-4456-9f78-56e002b38549")));
    QCOMPARE(model.index(0, 0).data(Qt::UserRole+1).toStringList(), QStringList() << QStringLiteral("Not grouped"));
    QCOMPARE(model.index(0, 0).data(Qt::UserRole+2).isValid(), false);

    QCOMPARE(model.name(QUuid(QStringLiteral("e58abf42-77a6-4456-9f78-56e002b38549"))), QStringLiteral("Minimal"));
    QCOMPARE(model.name(QUuid::createUuid()), QString());

    QCOMPARE(model.indexFor(QUuid(QStringLiteral("e58abf42-77a6-4456-9f78-56e002b38549"))), model.index(0, 0));
    QCOMPARE(model.indexFor(QUuid::createUuid()).isValid(), false);

    QCOMPARE(model.graph(model.index(0, 0)).id, Poco::UUID("e58abf42-77a6-4456-9f78-56e002b38549"));
    QCOMPARE(model.graph(model.index(1, 0)).id.isNull(), true);
}

QTEST_GUILESS_MAIN(GraphModelTest)
#include "testGraphModel.moc"
