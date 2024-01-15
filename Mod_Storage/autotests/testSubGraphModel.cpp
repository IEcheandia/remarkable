#include <QTest>
#include <QSaveFile>
#include <QSignalSpy>

#include "../src/subGraphCategoryFilterModel.h"
#include "../src/subGraphCheckedFilterModel.h"
#include "../src/subGraphModel.h"
#include "../src/product.h"
#include "../src/seamSeries.h"
#include "../src/seam.h"

using precitec::storage::SubGraphModel;
using precitec::storage::SubGraphCheckedFilterModel;
using precitec::storage::SubGraphCategoryFilterModel;

Q_DECLARE_METATYPE(std::vector<precitec::storage::SubGraphModel::BridgedDataType>)

class TestSubGraphModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testLoadSubgraphs_data();
    void testLoadSubgraphs();
    void testLoadSubgraphsUserConfig();
    void testUserGraphOverwritesSystemGraph();
    void testGraphModifying();
    void testName_data();
    void testName();
    void testComment_data();
    void testComment();
    void testSortOfCategoryModel();
    void testImage_data();
    void testImage();
    void testCategory_data();
    void testCategory();
    void testSinkBridges_data();
    void testSinkBridges();
    void testSourceBridges_data();
    void testSourceBridges();
    void testEnabled_data();
    void testEnabled();
    void testCheck();
    void testCheckIndex();
    void testCheckIndexWithSeam();
    void testGenerateGraphId();
    void testCombinedGraph();
    void testCategoryTranslation_data();
    void testCategoryTranslation();
    void testGraphAdding();
};

void TestSubGraphModel::testCtor()
{
    SubGraphModel graphModel;
    QCOMPARE(graphModel.rowCount(), 0);
    QCOMPARE(graphModel.category(graphModel.index(0, 0)), QString());
    QCOMPARE(graphModel.sourceBridges(graphModel.index(0, 0)).empty(), true);
    QCOMPARE(graphModel.sinkBridges(graphModel.index(0, 0)).empty(), true);
    QCOMPARE(graphModel.availableCategories(), QStringList());

    SubGraphCategoryFilterModel filterModel;
    QCOMPARE(filterModel.categoryName(), QString());
}

void TestSubGraphModel::testLoadSubgraphs_data()
{
    QTest::addColumn<QString>("category");
    QTest::addColumn<int>("rowCount");

    QTest::newRow("source") << QStringLiteral("source") << 2;
    QTest::newRow("inspection") << QStringLiteral("inspection") << 1;
    QTest::newRow("tracking") << QStringLiteral("tracking") << 1;
    QTest::newRow("utility") << QStringLiteral("utility") << 1;
    QTest::newRow("doesnotexist") << QStringLiteral("doesnotexist") << 0;

    QTest::newRow("SOURCE") << QStringLiteral("SOURCE") << 2;
    QTest::newRow("iNspection") << QStringLiteral("iNspection") << 1;
    QTest::newRow("traCking") << QStringLiteral("traCking") << 1;
    QTest::newRow("utilitY") << QStringLiteral("utilitY") << 1;
}

void TestSubGraphModel::testLoadSubgraphs()
{
    SubGraphModel graphModel;
    QSignalSpy loadingChangedSpy{&graphModel, &SubGraphModel::loadingChanged};
    QCOMPARE(graphModel.availableCategories(), QStringList());
    graphModel.loadSubGraphs(QFINDTESTDATA("testdata/subgraphs/"));
    QCOMPARE(loadingChangedSpy.count(), 1);
    QCOMPARE(graphModel.isLoading(), true);
    QVERIFY(loadingChangedSpy.wait());
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(graphModel.isLoading(), false);

    QCOMPARE(graphModel.rowCount(), 5);
    QCOMPARE(graphModel.availableCategories(), QStringList() << QStringLiteral("inspection") << QStringLiteral("source") << QStringLiteral("tracking") <<  QStringLiteral("utility"));

    SubGraphCategoryFilterModel filterModel;
    filterModel.setDisabledFilter(true);
    QSignalSpy categoryNameChangedSpy{&filterModel, &SubGraphCategoryFilterModel::categoryNameChanged};
    QVERIFY(categoryNameChangedSpy.isValid());
    QCOMPARE(filterModel.categoryName(), QString());
    filterModel.setSourceModel(&graphModel);
    QCOMPARE(filterModel.rowCount(), 0);
    QFETCH(QString, category);
    filterModel.setCategoryName(category);
    QCOMPARE(filterModel.categoryName(), category);
    QCOMPARE(categoryNameChangedSpy.count(), 1);
    QTEST(filterModel.rowCount(), "rowCount");

    // setting same name should not change
    filterModel.setCategoryName(category);
    QCOMPARE(categoryNameChangedSpy.count(), 1);
}

void TestSubGraphModel::testLoadSubgraphsUserConfig()
{
    SubGraphModel graphModel;
    QSignalSpy loadingChangedSpy{&graphModel, &SubGraphModel::loadingChanged};
    QCOMPARE(graphModel.availableCategories(), QStringList());
    graphModel.loadSubGraphs({}, QFINDTESTDATA("testdata/subgraphs/utility"));
    QCOMPARE(loadingChangedSpy.count(), 1);
    QCOMPARE(graphModel.isLoading(), true);
    QVERIFY(loadingChangedSpy.wait());
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(graphModel.isLoading(), false);

    QCOMPARE(graphModel.rowCount(), 1);
    QCOMPARE(graphModel.availableCategories(), QStringList() << QStringLiteral("utility"));
    const auto index = graphModel.index(0, 0);
    QCOMPARE(graphModel.data(index).toString(), QStringLiteral("Line profile"));
    QCOMPARE(graphModel.data(index, Qt::UserRole).toUuid(), QUuid{"ca23f1a1-874d-4383-95fb-f22b65c513e9"});
}

void TestSubGraphModel::testUserGraphOverwritesSystemGraph()
{
    SubGraphModel graphModel;
    QSignalSpy loadingChangedSpy{&graphModel, &SubGraphModel::loadingChanged};
    QCOMPARE(graphModel.availableCategories(), QStringList());

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/subgraphs/utility/Line profile.xml"), dir.filePath(QStringLiteral("Line profile.xml"))));

    graphModel.loadSubGraphs(QFINDTESTDATA("testdata/subgraphs/"), dir.path());
    QCOMPARE(loadingChangedSpy.count(), 1);
    QCOMPARE(graphModel.isLoading(), true);
    QVERIFY(loadingChangedSpy.wait());
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(graphModel.isLoading(), false);

    QCOMPARE(graphModel.rowCount(), 5);
    QCOMPARE(graphModel.availableCategories(), QStringList() << QStringLiteral("inspection") << QStringLiteral("source") <<  QFileInfo{dir.path()}.fileName() << QStringLiteral("tracking"));
}

void TestSubGraphModel::testGraphModifying()
{
    SubGraphModel graphModel;
    QSignalSpy loadingChangedSpy{&graphModel, &SubGraphModel::loadingChanged};
    QCOMPARE(graphModel.availableCategories(), QStringList());
    QSignalSpy dataChangedSpy{&graphModel, &SubGraphModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    QTemporaryDir tempDir{};
    QVERIFY(tempDir.isValid());

    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/subgraphs/utility/Line profile.xml"), tempDir.filePath(QStringLiteral("Line profile.xml"))));

    graphModel.loadSubGraphs({}, tempDir.path());
    QCOMPARE(loadingChangedSpy.count(), 1);
    QCOMPARE(graphModel.isLoading(), true);
    QVERIFY(loadingChangedSpy.wait());
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(graphModel.isLoading(), false);

    QCOMPARE(graphModel.rowCount(), 1);
    QCOMPARE(graphModel.availableCategories(), QStringList() << QFileInfo{tempDir.path()}.fileName());
    const auto index = graphModel.index(0, 0);
    QCOMPARE(graphModel.data(index).toString(), QStringLiteral("Line profile"));
    QCOMPARE(graphModel.data(index, Qt::UserRole).toUuid(), QUuid{"ca23f1a1-874d-4383-95fb-f22b65c513e9"});

    QVERIFY(dataChangedSpy.isEmpty());

    QFile readFile{tempDir.filePath(QStringLiteral("Line profile.xml"))};
    QVERIFY(readFile.open(QIODevice::ReadOnly));
    QByteArray data = readFile.readAll();
    readFile.close();

    QSaveFile xmlFile{tempDir.filePath(QStringLiteral("Line profile.xml"))};
    QVERIFY(xmlFile.open(QIODevice::WriteOnly));
    xmlFile.write(data);
    QVERIFY(xmlFile.commit());
    QTRY_COMPARE(dataChangedSpy.count(), 1);

    QCOMPARE(dataChangedSpy.at(0).at(0).toModelIndex(), graphModel.index(0, 0));
    QCOMPARE(dataChangedSpy.at(0).at(1).toModelIndex(), graphModel.index(0, 0));
    QCOMPARE(dataChangedSpy.at(0).at(0).value<QVector<int>>().empty(), true);
}

void TestSubGraphModel::testName_data()
{
    QTest::addColumn<QUuid>("uuid");
    QTest::addColumn<QString>("name");

    QTest::newRow("Inspection line finding") << QUuid{"0940cf62-a092-4d9f-b59d-2edd97e038d5"} << QStringLiteral("Inspection line finding");
    QTest::newRow("ImageSource to sink") << QUuid{"6932b184-42b5-4a42-8417-7ff81d066cab"} << QStringLiteral("ImageSource to sink");
    QTest::newRow("LaserPower to sink") << QUuid{"251a012c-c4ac-4a44-9f12-5cf07a2a1ae0"} << QStringLiteral("LaserPower to sink");
    QTest::newRow("Line finding") << QUuid{"7772eaab-acf4-47cd-86dc-02affc8c68c0"} << QStringLiteral("Line finding");
    QTest::newRow("Line profile") << QUuid{"ca23f1a1-874d-4383-95fb-f22b65c513e9"} << QStringLiteral("Line profile");
    QTest::newRow("invalid") << QUuid{""} << QStringLiteral("");
}

void TestSubGraphModel::testName()
{
    SubGraphModel graphModel;
    QSignalSpy loadingChangedSpy{&graphModel, &SubGraphModel::loadingChanged};
    graphModel.loadSubGraphs(QFINDTESTDATA("testdata/subgraphs/"));
    QCOMPARE(loadingChangedSpy.count(), 1);
    QCOMPARE(graphModel.isLoading(), true);
    QVERIFY(loadingChangedSpy.wait());
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(graphModel.isLoading(), false);

    QCOMPARE(graphModel.rowCount(), 5);

    QFETCH(QUuid, uuid);
    QTEST(graphModel.name(uuid), "name");
    const auto index = graphModel.indexFor(uuid);
    QCOMPARE(index.isValid(), !uuid.isNull());
    QTEST(graphModel.data(index).toString(), "name");
    QCOMPARE(graphModel.data(index, Qt::UserRole).toUuid(), uuid);
}

void TestSubGraphModel::testComment_data()
{
    QTest::addColumn<QUuid>("uuid");
    QTest::addColumn<QString>("comment");

    QTest::newRow("Inspection line finding") << QUuid{"0940cf62-a092-4d9f-b59d-2edd97e038d5"} << QStringLiteral("This is a comment");
    QTest::newRow("ImageSource to sink") << QUuid{"6932b184-42b5-4a42-8417-7ff81d066cab"} << QStringLiteral("Image");
    QTest::newRow("LaserPower to sink") << QUuid{"251a012c-c4ac-4a44-9f12-5cf07a2a1ae0"} << QStringLiteral("LaserPower");
    QTest::newRow("Line finding") << QUuid{"7772eaab-acf4-47cd-86dc-02affc8c68c0"} << QStringLiteral("Line");
    QTest::newRow("Line profile") << QUuid{"ca23f1a1-874d-4383-95fb-f22b65c513e9"} << QStringLiteral("Profile");
    QTest::newRow("invalid") << QUuid{""} << QStringLiteral("");
}

void TestSubGraphModel::testComment()
{
    SubGraphModel graphModel;
    QSignalSpy loadingChangedSpy{&graphModel, &SubGraphModel::loadingChanged};
    graphModel.loadSubGraphs(QFINDTESTDATA("testdata/subgraphs/"));
    QCOMPARE(loadingChangedSpy.count(), 1);
    QCOMPARE(graphModel.isLoading(), true);
    QVERIFY(loadingChangedSpy.wait());
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(graphModel.isLoading(), false);

    QCOMPARE(graphModel.rowCount(), 5);

    QFETCH(QUuid, uuid);
    const auto index = graphModel.indexFor(uuid);
    QCOMPARE(index.isValid(), !uuid.isNull());
    QTEST(graphModel.data(index, Qt::UserRole + 6).toString(), "comment");
    QCOMPARE(graphModel.data(index, Qt::UserRole).toUuid(), uuid);
}

void TestSubGraphModel::testSortOfCategoryModel()
{
    SubGraphModel graphModel;
    QSignalSpy loadingChangedSpy{&graphModel, &SubGraphModel::loadingChanged};
    QCOMPARE(graphModel.availableCategories(), QStringList());
    graphModel.loadSubGraphs(QFINDTESTDATA("testdata/subgraphs/"));
    QCOMPARE(loadingChangedSpy.count(), 1);
    QCOMPARE(graphModel.isLoading(), true);
    QVERIFY(loadingChangedSpy.wait());
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(graphModel.isLoading(), false);

    QCOMPARE(graphModel.rowCount(), 5);

    SubGraphCategoryFilterModel filterModel;
    QSignalSpy categoryNameChangedSpy{&filterModel, &SubGraphCategoryFilterModel::categoryNameChanged};
    QVERIFY(categoryNameChangedSpy.isValid());
    filterModel.setSourceModel(&graphModel);
    filterModel.setCategoryName(QStringLiteral("source"));
    QCOMPARE(categoryNameChangedSpy.count(), 1);
    QCOMPARE(filterModel.rowCount(), 2);

    QCOMPARE(filterModel.index(0, 0).data(), QStringLiteral("ImageSource to sink"));
    QCOMPARE(filterModel.index(1, 0).data(), QStringLiteral("LaserPower to sink"));
}

void TestSubGraphModel::testImage_data()
{
    QTest::addColumn<QUuid>("uuid");
    QTest::addColumn<QUrl>("image");

    QTest::newRow("Inspection line finding") << QUuid{"0940cf62-a092-4d9f-b59d-2edd97e038d5"} << QUrl::fromLocalFile(QFINDTESTDATA("testdata/subgraphs/inspection/0940cf62-a092-4d9f-b59d-2edd97e038d5.png"));
    QTest::newRow("ImageSource to sink") << QUuid{"6932b184-42b5-4a42-8417-7ff81d066cab"} << QUrl::fromLocalFile(QFINDTESTDATA("testdata/subgraphs/source/6932b184-42b5-4a42-8417-7ff81d066cab.png"));
    QTest::newRow("LaserPower to sink") << QUuid{"251a012c-c4ac-4a44-9f12-5cf07a2a1ae0"} << QUrl::fromLocalFile(QFINDTESTDATA("testdata/subgraphs/source/251a012c-c4ac-4a44-9f12-5cf07a2a1ae0.png"));
    QTest::newRow("Line finding") << QUuid{"7772eaab-acf4-47cd-86dc-02affc8c68c0"} << QUrl::fromLocalFile(QFINDTESTDATA("testdata/subgraphs/tracking/7772eaab-acf4-47cd-86dc-02affc8c68c0.png"));
    // For utility the image is not created
    QTest::newRow("Line profile") << QUuid{"ca23f1a1-874d-4383-95fb-f22b65c513e9"} << QUrl{};
    QTest::newRow("invalid") << QUuid{""} << QUrl{};
}

void TestSubGraphModel::testImage()
{
    SubGraphModel graphModel;
    QCOMPARE(graphModel.roleNames()[Qt::UserRole + 5], QByteArrayLiteral("image"));
    QSignalSpy loadingChangedSpy{&graphModel, &SubGraphModel::loadingChanged};
    graphModel.loadSubGraphs(QFINDTESTDATA("testdata/subgraphs/"));
    QCOMPARE(loadingChangedSpy.count(), 1);
    QCOMPARE(graphModel.isLoading(), true);
    QVERIFY(loadingChangedSpy.wait());
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(graphModel.isLoading(), false);

    QCOMPARE(graphModel.rowCount(), 5);

    QFETCH(QUuid, uuid);
    const auto index = graphModel.indexFor(uuid);

    QTEST(graphModel.data(index, Qt::UserRole + 5).toUrl(), "image");
}

void TestSubGraphModel::testCategory_data()
{
    QTest::addColumn<QUuid>("uuid");
    QTest::addColumn<QString>("category");

    QTest::newRow("Inspection line finding") << QUuid{"0940cf62-a092-4d9f-b59d-2edd97e038d5"} << QStringLiteral("inspection");
    QTest::newRow("ImageSource to sink") << QUuid{"6932b184-42b5-4a42-8417-7ff81d066cab"} << QStringLiteral("source");
    QTest::newRow("LaserPower to sink") << QUuid{"251a012c-c4ac-4a44-9f12-5cf07a2a1ae0"} << QStringLiteral("source");
    QTest::newRow("Line finding") << QUuid{"7772eaab-acf4-47cd-86dc-02affc8c68c0"} << QStringLiteral("tracking");
    QTest::newRow("Line profile") << QUuid{"ca23f1a1-874d-4383-95fb-f22b65c513e9"} << QStringLiteral("utility");
    QTest::newRow("invalid") << QUuid{""} << QStringLiteral("");
}

void TestSubGraphModel::testCategory()
{
    SubGraphModel graphModel;
    QSignalSpy loadingChangedSpy{&graphModel, &SubGraphModel::loadingChanged};
    graphModel.loadSubGraphs(QFINDTESTDATA("testdata/subgraphs/"));
    QCOMPARE(loadingChangedSpy.count(), 1);
    QCOMPARE(graphModel.isLoading(), true);
    QVERIFY(loadingChangedSpy.wait());
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(graphModel.isLoading(), false);

    QCOMPARE(graphModel.rowCount(), 5);

    QFETCH(QUuid, uuid);
    QTEST(graphModel.category(graphModel.indexFor(uuid)), "category");
    QTEST(graphModel.indexFor(uuid).data(Qt::UserRole + 4).toString(), "category");
}

void TestSubGraphModel::testSinkBridges_data()
{
    QTest::addColumn<QUuid>("uuid");
    QTest::addColumn<std::size_t>("count");
    QTest::addColumn<std::vector<QByteArray>>("bridgeNames");
    QTest::addColumn<std::vector<SubGraphModel::BridgedDataType>>("types");

    QTest::newRow("Inspection line finding") << QUuid{"0940cf62-a092-4d9f-b59d-2edd97e038d5"} << 2ul << std::vector<QByteArray>{QByteArrayLiteral("Inspection line roi image"), QByteArrayLiteral("tracking line")} << std::vector<SubGraphModel::BridgedDataType>{SubGraphModel::BridgedDataType::ImageFrame, SubGraphModel::BridgedDataType::Line};
    QTest::newRow("ImageSource to sink") << QUuid{"6932b184-42b5-4a42-8417-7ff81d066cab"} << 1ul << std::vector<QByteArray>{QByteArrayLiteral("image")} << std::vector<SubGraphModel::BridgedDataType>{SubGraphModel::BridgedDataType::ImageFrame};
    QTest::newRow("LaserPower to sink") << QUuid{"251a012c-c4ac-4a44-9f12-5cf07a2a1ae0"} << 1ul << std::vector<QByteArray>{QByteArrayLiteral("laserpower")} << std::vector<SubGraphModel::BridgedDataType>{SubGraphModel::BridgedDataType::Double};
    QTest::newRow("Line finding") << QUuid{"7772eaab-acf4-47cd-86dc-02affc8c68c0"} << 2ul << std::vector<QByteArray>{QByteArrayLiteral("tracking line roi image"), QByteArrayLiteral("tracking line")} << std::vector<SubGraphModel::BridgedDataType>{SubGraphModel::BridgedDataType::ImageFrame, SubGraphModel::BridgedDataType::Line};
    QTest::newRow("Line profile") << QUuid{"ca23f1a1-874d-4383-95fb-f22b65c513e9"} << 0ul << std::vector<QByteArray>{} << std::vector<SubGraphModel::BridgedDataType>{};
    QTest::newRow("invalid") << QUuid{""} << 0ul << std::vector<QByteArray>{} << std::vector<SubGraphModel::BridgedDataType>{};
}

void TestSubGraphModel::testSinkBridges()
{
    SubGraphModel graphModel;
    QSignalSpy loadingChangedSpy{&graphModel, &SubGraphModel::loadingChanged};
    graphModel.loadSubGraphs(QFINDTESTDATA("testdata/subgraphs/"));
    QCOMPARE(loadingChangedSpy.count(), 1);
    QCOMPARE(graphModel.isLoading(), true);
    QVERIFY(loadingChangedSpy.wait());
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(graphModel.isLoading(), false);

    QCOMPARE(graphModel.rowCount(), 5);

    QFETCH(QUuid, uuid);
    const auto &bridges = graphModel.sinkBridges(graphModel.indexFor(uuid));
    QTEST(bridges.size(), "count");
    QFETCH(std::vector<QByteArray>, bridgeNames);
    QFETCH(std::vector<SubGraphModel::BridgedDataType>, types);
    QCOMPARE(bridgeNames.size(), types.size());
    QCOMPARE(bridgeNames.size(), bridges.size());
    for (std::size_t i = 0; i < bridgeNames.size(); i++)
    {
        const auto &bridgeName = bridgeNames.at(i);
        auto it = std::find_if(bridges.begin(), bridges.end(), [bridgeName] (const auto &bridge) { return bridge.name == bridgeName.toStdString(); });
        QVERIFY(it != bridges.end());
        QCOMPARE(it->type, SubGraphModel::BridgeType::Sink);
        QCOMPARE(it->dataType, types.at(i));
    }
}

void TestSubGraphModel::testSourceBridges_data()
{
    QTest::addColumn<QUuid>("uuid");
    QTest::addColumn<std::size_t>("count");
    QTest::addColumn<std::vector<QByteArray>>("bridgeNames");
    QTest::addColumn<std::vector<SubGraphModel::BridgedDataType>>("types");

    QTest::newRow("Inspection line finding") << QUuid{"0940cf62-a092-4d9f-b59d-2edd97e038d5"} << 1ul << std::vector<QByteArray>{QByteArrayLiteral("image")} << std::vector<SubGraphModel::BridgedDataType>{SubGraphModel::BridgedDataType::ImageFrame};
    QTest::newRow("ImageSource to sink") << QUuid{"6932b184-42b5-4a42-8417-7ff81d066cab"} << 0ul << std::vector<QByteArray>{} << std::vector<SubGraphModel::BridgedDataType>{};
    QTest::newRow("LaserPower to sink") << QUuid{"251a012c-c4ac-4a44-9f12-5cf07a2a1ae0"} << 0ul << std::vector<QByteArray>{} << std::vector<SubGraphModel::BridgedDataType>{};
    QTest::newRow("Line finding") << QUuid{"7772eaab-acf4-47cd-86dc-02affc8c68c0"} << 1ul << std::vector<QByteArray>{QByteArrayLiteral("image")} << std::vector<SubGraphModel::BridgedDataType>{SubGraphModel::BridgedDataType::ImageFrame};
    QTest::newRow("Line profile") << QUuid{"ca23f1a1-874d-4383-95fb-f22b65c513e9"} << 2ul << std::vector<QByteArray>{QByteArrayLiteral("tracking line"), QByteArrayLiteral("tracking line roi image")} << std::vector<SubGraphModel::BridgedDataType>{SubGraphModel::BridgedDataType::Line, SubGraphModel::BridgedDataType::ImageFrame};
    QTest::newRow("invalid") << QUuid{""} << 0ul << std::vector<QByteArray>{} << std::vector<SubGraphModel::BridgedDataType>{};
}

void TestSubGraphModel::testSourceBridges()
{
    SubGraphModel graphModel;
    QSignalSpy loadingChangedSpy{&graphModel, &SubGraphModel::loadingChanged};
    graphModel.loadSubGraphs(QFINDTESTDATA("testdata/subgraphs/"));
    QCOMPARE(loadingChangedSpy.count(), 1);
    QCOMPARE(graphModel.isLoading(), true);
    QVERIFY(loadingChangedSpy.wait());
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(graphModel.isLoading(), false);

    QCOMPARE(graphModel.rowCount(), 5);

    QFETCH(QUuid, uuid);
    const auto &bridges = graphModel.sourceBridges(graphModel.indexFor(uuid));
    QTEST(bridges.size(), "count");
    QFETCH(std::vector<QByteArray>, bridgeNames);
    QFETCH(std::vector<SubGraphModel::BridgedDataType>, types);
    QCOMPARE(bridgeNames.size(), types.size());
    QCOMPARE(bridgeNames.size(), bridges.size());
    for (std::size_t i = 0; i < bridgeNames.size(); i++)
    {
        const auto &bridgeName = bridgeNames.at(i);
        auto it = std::find_if(bridges.begin(), bridges.end(), [bridgeName] (const auto &bridge) { return bridge.name == bridgeName.toStdString(); });
        QVERIFY(it != bridges.end());
        QCOMPARE(it->type, SubGraphModel::BridgeType::Source);
        QCOMPARE(it->dataType, types.at(i));
    }
}

void TestSubGraphModel::testEnabled_data()
{
    QTest::addColumn<QUuid>("uuid");
    QTest::addColumn<bool>("enabled");

    QTest::newRow("Inspection line finding") << QUuid{"0940cf62-a092-4d9f-b59d-2edd97e038d5"} << false;
    QTest::newRow("ImageSource to sink") << QUuid{"6932b184-42b5-4a42-8417-7ff81d066cab"} << true;
    QTest::newRow("LaserPower to sink") << QUuid{"251a012c-c4ac-4a44-9f12-5cf07a2a1ae0"} << true;
    QTest::newRow("Line finding") << QUuid{"7772eaab-acf4-47cd-86dc-02affc8c68c0"} << false;
    QTest::newRow("Line profile") << QUuid{"ca23f1a1-874d-4383-95fb-f22b65c513e9"} << false;
    QTest::newRow("invalid") << QUuid{""} << false;
}

void TestSubGraphModel::testEnabled()
{
    SubGraphModel graphModel;
    QSignalSpy loadingChangedSpy{&graphModel, &SubGraphModel::loadingChanged};
    graphModel.loadSubGraphs(QFINDTESTDATA("testdata/subgraphs/"));
    QCOMPARE(loadingChangedSpy.count(), 1);
    QCOMPARE(graphModel.isLoading(), true);
    QVERIFY(loadingChangedSpy.wait());
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(graphModel.isLoading(), false);

    QCOMPARE(graphModel.rowCount(), 5);

    QFETCH(QUuid, uuid);
    QTEST(graphModel.data(graphModel.indexFor(uuid), Qt::UserRole + 2).toBool(), "enabled");
    // checked is always false
    QCOMPARE(graphModel.data(graphModel.indexFor(uuid), Qt::UserRole + 3).toBool(), false);
}

void TestSubGraphModel::testCheck()
{
    SubGraphModel graphModel;
    QSignalSpy loadingChangedSpy{&graphModel, &SubGraphModel::loadingChanged};
    graphModel.loadSubGraphs(QFINDTESTDATA("testdata/subgraphs/"));
    QCOMPARE(loadingChangedSpy.count(), 1);
    QCOMPARE(graphModel.isLoading(), true);
    QVERIFY(loadingChangedSpy.wait());
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(graphModel.isLoading(), false);

    QCOMPARE(graphModel.rowCount(), 5);

    QCOMPARE(graphModel.checkedGraphs().empty(), true);

    const QModelIndex inspectionLineFinding = graphModel.indexFor({QByteArrayLiteral("0940cf62-a092-4d9f-b59d-2edd97e038d5")});
    const QModelIndex imageSourceSink = graphModel.indexFor({QByteArrayLiteral("6932b184-42b5-4a42-8417-7ff81d066cab")});
    const QModelIndex laserPowerSink = graphModel.indexFor({QByteArrayLiteral("251a012c-c4ac-4a44-9f12-5cf07a2a1ae0")});
    const QModelIndex lineFinding= graphModel.indexFor({QByteArrayLiteral("7772eaab-acf4-47cd-86dc-02affc8c68c0")});
    const QModelIndex lineProfile = graphModel.indexFor({QByteArrayLiteral("ca23f1a1-874d-4383-95fb-f22b65c513e9")});

    const auto &imageSourceSinkBridges = graphModel.sinkBridges(imageSourceSink);
    QCOMPARE(imageSourceSinkBridges.size(), 1);
    QVERIFY(!graphModel.isSinkBridgeUsed(imageSourceSinkBridges.front()));

    const auto &lineFindingSinkBridges = graphModel.sinkBridges(lineFinding);
    QCOMPARE(lineFindingSinkBridges.size(), 2);
    QVERIFY(!graphModel.isSinkBridgeUsed(lineFindingSinkBridges.front()));

    QSignalSpy dataChangedSpy{&graphModel, &SubGraphModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    // check two graphs not connected - laserpower and line profile
    graphModel.check({{QByteArrayLiteral("251a012c-c4ac-4a44-9f12-5cf07a2a1ae0"), QByteArrayLiteral("ca23f1a1-874d-4383-95fb-f22b65c513e9")}});
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.at(0).at(0).toModelIndex(), graphModel.index(0, 0));
    QCOMPARE(dataChangedSpy.at(0).at(1).toModelIndex(), graphModel.index(4, 0));
    QCOMPARE(dataChangedSpy.at(0).at(2).value<QVector<int>>(), QVector<int>() << Qt::UserRole + 2 << Qt::UserRole + 3);
    // only laser power should be checked
    QCOMPARE(laserPowerSink.data(Qt::UserRole + 3).toBool(), true);
    QCOMPARE(imageSourceSink.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(inspectionLineFinding.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(lineFinding.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(lineProfile.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(graphModel.checkedGraphs().size(), std::size_t(1));
    QCOMPARE(graphModel.checkedGraphs().at(0), QUuid(QByteArrayLiteral("251a012c-c4ac-4a44-9f12-5cf07a2a1ae0")));

    // laser power and image source should be enabled
    QCOMPARE(laserPowerSink.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(imageSourceSink.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(inspectionLineFinding.data(Qt::UserRole + 2).toBool(), false);
    QCOMPARE(lineFinding.data(Qt::UserRole + 2).toBool(), false);
    QCOMPARE(lineProfile.data(Qt::UserRole + 2).toBool(), false);

    // check some connected graphs
    // laser source, line finding, line profile
    graphModel.check({{QByteArrayLiteral("6932b184-42b5-4a42-8417-7ff81d066cab"), QByteArrayLiteral("ca23f1a1-874d-4383-95fb-f22b65c513e9"), QByteArrayLiteral("7772eaab-acf4-47cd-86dc-02affc8c68c0")}});
    QCOMPARE(dataChangedSpy.count(), 2);

    // verify checked state
    QCOMPARE(laserPowerSink.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(imageSourceSink.data(Qt::UserRole + 3).toBool(), true);
    QCOMPARE(inspectionLineFinding.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(lineFinding.data(Qt::UserRole + 3).toBool(), true);
    QCOMPARE(lineProfile.data(Qt::UserRole + 3).toBool(), true);
    QCOMPARE(graphModel.checkedGraphs().size(), std::size_t(3));

    // verify that sink bridges are used
    QVERIFY(graphModel.isSinkBridgeUsed(imageSourceSinkBridges.front()));
    QVERIFY(graphModel.isSinkBridgeUsed(lineFindingSinkBridges.front()));

    // everything should be enabled, except inspection line finding as line finding is enabled
    QCOMPARE(laserPowerSink.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(imageSourceSink.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(inspectionLineFinding.data(Qt::UserRole + 2).toBool(), false);
    QCOMPARE(lineFinding.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(lineProfile.data(Qt::UserRole + 2).toBool(), true);

    // check with graph that doesn't exist
    graphModel.check({{QByteArrayLiteral("6932b184-42b5-4a42-8417-7ff81d066cab"), QByteArrayLiteral("fa064039-bbe6-4f07-847e-e6e6c5182725")}});
    QCOMPARE(dataChangedSpy.count(), 3);
    // only image source checked
    QCOMPARE(laserPowerSink.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(imageSourceSink.data(Qt::UserRole + 3).toBool(), true);
    QCOMPARE(inspectionLineFinding.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(lineFinding.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(lineProfile.data(Qt::UserRole + 3).toBool(), false);

    QCOMPARE(laserPowerSink.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(imageSourceSink.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(inspectionLineFinding.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(lineFinding.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(lineProfile.data(Qt::UserRole + 2).toBool(), false);

    // and disable all again
    graphModel.check({});
    QCOMPARE(dataChangedSpy.count(), 4);
    QCOMPARE(laserPowerSink.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(imageSourceSink.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(inspectionLineFinding.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(lineFinding.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(lineProfile.data(Qt::UserRole + 3).toBool(), false);

    // only source enabled
    QCOMPARE(laserPowerSink.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(imageSourceSink.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(inspectionLineFinding.data(Qt::UserRole + 2).toBool(), false);
    QCOMPARE(lineFinding.data(Qt::UserRole + 2).toBool(), false);
    QCOMPARE(lineProfile.data(Qt::UserRole + 2).toBool(), false);

    QVERIFY(!graphModel.isSinkBridgeUsed(imageSourceSinkBridges.front()));
    QVERIFY(!graphModel.isSinkBridgeUsed(lineFindingSinkBridges.front()));

    // graph with same sink bridges: image source, line finding, inpsection line finding
    graphModel.check({{QByteArrayLiteral("6932b184-42b5-4a42-8417-7ff81d066cab"), QByteArrayLiteral("7772eaab-acf4-47cd-86dc-02affc8c68c0"), QByteArrayLiteral("0940cf62-a092-4d9f-b59d-2edd97e038d5")}});
    QCOMPARE(dataChangedSpy.count(), 5);
    QCOMPARE(laserPowerSink.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(imageSourceSink.data(Qt::UserRole + 3).toBool(), true);
    QCOMPARE(inspectionLineFinding.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(lineFinding.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(lineProfile.data(Qt::UserRole + 3).toBool(), false);

    QCOMPARE(laserPowerSink.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(imageSourceSink.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(inspectionLineFinding.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(lineFinding.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(lineProfile.data(Qt::UserRole + 2).toBool(), false);

    QVERIFY(!graphModel.isSinkBridgeUsed(lineFindingSinkBridges.front()));

    // graph with alternatives
    graphModel.check({{QByteArrayLiteral("6932b184-42b5-4a42-8417-7ff81d066cab"), QByteArrayLiteral("7772eaab-acf4-47cd-86dc-02affc8c68c0"), QByteArrayLiteral("ca23f1a1-874d-4383-95fb-f22b65c513e9")}});
    QCOMPARE(dataChangedSpy.count(), 6);
    QCOMPARE(laserPowerSink.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(imageSourceSink.data(Qt::UserRole + 3).toBool(), true);
    QCOMPARE(inspectionLineFinding.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(lineFinding.data(Qt::UserRole + 3).toBool(), true);
    QCOMPARE(lineProfile.data(Qt::UserRole + 3).toBool(), true);

    QCOMPARE(laserPowerSink.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(imageSourceSink.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(inspectionLineFinding.data(Qt::UserRole + 2).toBool(), false);
    QCOMPARE(lineFinding.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(lineProfile.data(Qt::UserRole + 2).toBool(), true);

    QVERIFY(graphModel.isSinkBridgeUsed(lineFindingSinkBridges.front()));

    graphModel.switchToAlternative(lineFinding, inspectionLineFinding);

    QCOMPARE(dataChangedSpy.count(), 7);
    QCOMPARE(laserPowerSink.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(imageSourceSink.data(Qt::UserRole + 3).toBool(), true);
    QCOMPARE(inspectionLineFinding.data(Qt::UserRole + 3).toBool(), true);
    QCOMPARE(lineFinding.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(lineProfile.data(Qt::UserRole + 3).toBool(), false);

    QVERIFY(!graphModel.isSinkBridgeUsed(lineFindingSinkBridges.front()));
}

void TestSubGraphModel::testCheckIndex()
{
    SubGraphModel graphModel;
    SubGraphCheckedFilterModel filterModel;
    filterModel.setSourceModel(&graphModel);
    QCOMPARE(filterModel.rowCount(), 0);
    QSignalSpy loadingChangedSpy{&graphModel, &SubGraphModel::loadingChanged};
    graphModel.loadSubGraphs(QFINDTESTDATA("testdata/subgraphs/"));
    QCOMPARE(loadingChangedSpy.count(), 1);
    QCOMPARE(graphModel.isLoading(), true);
    QVERIFY(loadingChangedSpy.wait());
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(graphModel.isLoading(), false);

    QCOMPARE(graphModel.rowCount(), 5);

    const QModelIndex inspectionLineFinding = graphModel.indexFor({QByteArrayLiteral("0940cf62-a092-4d9f-b59d-2edd97e038d5")});
    const QModelIndex imageSourceSink = graphModel.indexFor({QByteArrayLiteral("6932b184-42b5-4a42-8417-7ff81d066cab")});
    const QModelIndex laserPowerSink = graphModel.indexFor({QByteArrayLiteral("251a012c-c4ac-4a44-9f12-5cf07a2a1ae0")});
    const QModelIndex lineFinding= graphModel.indexFor({QByteArrayLiteral("7772eaab-acf4-47cd-86dc-02affc8c68c0")});
    const QModelIndex lineProfile = graphModel.indexFor({QByteArrayLiteral("ca23f1a1-874d-4383-95fb-f22b65c513e9")});

    QSignalSpy dataChangedSpy{&graphModel, &SubGraphModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    // invalid index should not change
    graphModel.check({}, true);
    QVERIFY(dataChangedSpy.isEmpty());

    // not enabled should not change
    graphModel.check(lineFinding, true);
    QVERIFY(dataChangedSpy.isEmpty());

    // same value should not change
    graphModel.check(imageSourceSink, false);
    QVERIFY(dataChangedSpy.isEmpty());

    // check image source
    graphModel.check(imageSourceSink, true);
    QCOMPARE(filterModel.rowCount(), 1);
    QCOMPARE(filterModel.mapFromSource(imageSourceSink).isValid(), true);
    QCOMPARE(filterModel.mapFromSource(inspectionLineFinding).isValid(), false);
    QCOMPARE(filterModel.mapFromSource(laserPowerSink).isValid(), false);
    QCOMPARE(filterModel.mapFromSource(lineFinding).isValid(), false);
    QCOMPARE(filterModel.mapFromSource(lineProfile).isValid(), false);
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(imageSourceSink.data(Qt::UserRole + 3).toBool(), true);
    QCOMPARE(laserPowerSink.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(inspectionLineFinding.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(lineFinding.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(lineProfile.data(Qt::UserRole + 3).toBool(), false);

    QCOMPARE(laserPowerSink.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(imageSourceSink.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(inspectionLineFinding.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(lineFinding.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(lineProfile.data(Qt::UserRole + 2).toBool(), false);

    // check line finding
    graphModel.check(lineFinding, true);
    QCOMPARE(filterModel.rowCount(), 2);
    QCOMPARE(filterModel.mapToSource(filterModel.index(0, 0)), imageSourceSink);
    QCOMPARE(filterModel.mapToSource(filterModel.index(1, 0)), lineFinding);
    QCOMPARE(filterModel.mapFromSource(imageSourceSink).isValid(), true);
    QCOMPARE(filterModel.mapFromSource(inspectionLineFinding).isValid(), false);
    QCOMPARE(filterModel.mapFromSource(laserPowerSink).isValid(), false);
    QCOMPARE(filterModel.mapFromSource(lineFinding).isValid(), true);
    QCOMPARE(filterModel.mapFromSource(lineProfile).isValid(), false);
    QCOMPARE(dataChangedSpy.count(), 2);
    QCOMPARE(imageSourceSink.data(Qt::UserRole + 3).toBool(), true);
    QCOMPARE(laserPowerSink.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(inspectionLineFinding.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(lineFinding.data(Qt::UserRole + 3).toBool(), true);
    QCOMPARE(lineProfile.data(Qt::UserRole + 3).toBool(), false);

    QCOMPARE(laserPowerSink.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(imageSourceSink.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(inspectionLineFinding.data(Qt::UserRole + 2).toBool(), false);
    QCOMPARE(lineFinding.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(lineProfile.data(Qt::UserRole + 2).toBool(), true);

    // check line profile
    graphModel.check(lineProfile, true);
    QCOMPARE(filterModel.rowCount(), 3);
    QCOMPARE(filterModel.mapToSource(filterModel.index(0, 0)), imageSourceSink);
    QCOMPARE(filterModel.mapToSource(filterModel.index(1, 0)), lineFinding);
    QCOMPARE(filterModel.mapToSource(filterModel.index(2, 0)), lineProfile);
    QCOMPARE(filterModel.mapFromSource(imageSourceSink).isValid(), true);
    QCOMPARE(filterModel.mapFromSource(inspectionLineFinding).isValid(), false);
    QCOMPARE(filterModel.mapFromSource(laserPowerSink).isValid(), false);
    QCOMPARE(filterModel.mapFromSource(lineFinding).isValid(), true);
    QCOMPARE(filterModel.mapFromSource(lineProfile).isValid(), true);
    QCOMPARE(dataChangedSpy.count(), 3);
    QCOMPARE(imageSourceSink.data(Qt::UserRole + 3).toBool(), true);
    QCOMPARE(laserPowerSink.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(inspectionLineFinding.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(lineFinding.data(Qt::UserRole + 3).toBool(), true);
    QCOMPARE(lineProfile.data(Qt::UserRole + 3).toBool(), true);

    QCOMPARE(laserPowerSink.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(imageSourceSink.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(inspectionLineFinding.data(Qt::UserRole + 2).toBool(), false);
    QCOMPARE(lineFinding.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(lineProfile.data(Qt::UserRole + 2).toBool(), true);

    // disable image source
    graphModel.check(imageSourceSink, false);
    QCOMPARE(filterModel.rowCount(), 0);
    QCOMPARE(filterModel.mapFromSource(imageSourceSink).isValid(), false);
    QCOMPARE(filterModel.mapFromSource(inspectionLineFinding).isValid(), false);
    QCOMPARE(filterModel.mapFromSource(laserPowerSink).isValid(), false);
    QCOMPARE(filterModel.mapFromSource(lineFinding).isValid(), false);
    QCOMPARE(filterModel.mapFromSource(lineProfile).isValid(), false);
    QCOMPARE(dataChangedSpy.count(), 4);
    QCOMPARE(imageSourceSink.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(laserPowerSink.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(inspectionLineFinding.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(lineFinding.data(Qt::UserRole + 3).toBool(), false);
    QCOMPARE(lineProfile.data(Qt::UserRole + 3).toBool(), false);

    QCOMPARE(laserPowerSink.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(imageSourceSink.data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(inspectionLineFinding.data(Qt::UserRole + 2).toBool(), false);
    QCOMPARE(lineFinding.data(Qt::UserRole + 2).toBool(), false);
    QCOMPARE(lineProfile.data(Qt::UserRole + 2).toBool(), false);
}

void TestSubGraphModel::testCheckIndexWithSeam()
{
    SubGraphModel graphModel;
    SubGraphCheckedFilterModel filterModel;
    filterModel.setSourceModel(&graphModel);
    QCOMPARE(filterModel.rowCount(), 0);
    QSignalSpy loadingChangedSpy{&graphModel, &SubGraphModel::loadingChanged};
    graphModel.loadSubGraphs(QFINDTESTDATA("testdata/subgraphs/"));
    QCOMPARE(loadingChangedSpy.count(), 1);
    QCOMPARE(graphModel.isLoading(), true);
    QVERIFY(loadingChangedSpy.wait());
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(graphModel.isLoading(), false);

    QCOMPARE(filterModel.rowCount(), 0);
    const QModelIndex imageSourceSink = graphModel.indexFor({QByteArrayLiteral("6932b184-42b5-4a42-8417-7ff81d066cab")});
    graphModel.check(imageSourceSink, true);
    QCOMPARE(filterModel.rowCount(), 1);
    graphModel.check(imageSourceSink, false);
    QCOMPARE(filterModel.rowCount(), 0);

    auto product = std::make_unique<precitec::storage::Product>(QUuid::createUuid());
    product->createFirstSeamSeries();
    auto seam = product->createSeam();
    seam->setSubGraphs({QUuid{QByteArrayLiteral("6932b184-42b5-4a42-8417-7ff81d066cab")}});
    filterModel.setSeam(seam);
    QCOMPARE(filterModel.rowCount(), 1);

    product.reset();
    // trigger crash
    filterModel.invalidate();
    QCOMPARE(filterModel.rowCount(), 0);
}

void TestSubGraphModel::testGenerateGraphId()
{
    SubGraphModel graphModel;
    const auto noneUuid = graphModel.generateGraphId({});
    QCOMPARE(noneUuid.isNull(), false);
    QCOMPARE(graphModel.generateGraphId({}), noneUuid);

    QUuid uuid1 = QUuid::createUuid();
    QUuid uuid2 = QUuid::createUuid();
    QUuid uuid3 = QUuid::createUuid();
    QUuid uuid4 = QUuid::createUuid();

    const auto combinedUuid = graphModel.generateGraphId({uuid1, uuid2, uuid3, uuid4});
    QCOMPARE(combinedUuid.isNull(), false);
    QVERIFY(combinedUuid != noneUuid);
    QCOMPARE(graphModel.generateGraphId({uuid1, uuid2, uuid3, uuid4}), combinedUuid);
    QVERIFY(graphModel.generateGraphId({uuid4, uuid3, uuid2, uuid1}) != combinedUuid);
}

void TestSubGraphModel::testCombinedGraph()
{
    SubGraphModel graphModel;
    // not existing graph
    auto graph = graphModel.combinedGraph(QUuid::createUuid());
    QCOMPARE(graph.id, Poco::UUID());
    QCOMPARE(graph.filterGroups.empty(), true);
    QCOMPARE(graph.filterDescriptions.empty(), true);
    QCOMPARE(graph.instanceFilters.empty(), true);
    QCOMPARE(graph.pipes.empty(), true);

    // load graph
    QSignalSpy loadingChangedSpy{&graphModel, &SubGraphModel::loadingChanged};
    graphModel.loadSubGraphs(QFINDTESTDATA("testdata/subgraphs/"));
    QCOMPARE(loadingChangedSpy.count(), 1);
    QCOMPARE(graphModel.isLoading(), true);
    QVERIFY(loadingChangedSpy.wait());
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(graphModel.isLoading(), false);

    QCOMPARE(graphModel.rowCount(), 5);

    const auto graphId = graphModel.generateGraphId({{QByteArrayLiteral("6932b184-42b5-4a42-8417-7ff81d066cab"), QByteArrayLiteral("7772eaab-acf4-47cd-86dc-02affc8c68c0"), QByteArrayLiteral("ca23f1a1-874d-4383-95fb-f22b65c513e9")}});
    QCOMPARE(graphId.isNull(), false);

    graph = graphModel.combinedGraph(graphId);
    QCOMPARE(graph.id, Poco::UUID(graphId.toString(QUuid::WithoutBraces).toStdString()));
    QCOMPARE(graph.filterDescriptions.size(), std::size_t(4));
    QCOMPARE(graph.filterDescriptions.at(0).name, std::string("precitec::filter::ImageSource"));
    QCOMPARE(graph.filterDescriptions.at(1).name, std::string("precitec::filter::ROISelector"));
    QCOMPARE(graph.filterDescriptions.at(2).name, std::string("precitec::filter::LineTracking"));
    QCOMPARE(graph.filterDescriptions.at(3).name, std::string("precitec::filter::LineProfileResult"));

    QCOMPARE(graph.instanceFilters.size(), std::size_t(4));
    QCOMPARE(graph.instanceFilters.at(0).name, std::string("Bildquelle"));
    QCOMPARE(graph.instanceFilters.at(1).name, std::string("ROI Auswahl"));
    QCOMPARE(graph.instanceFilters.at(2).name, std::string("Line Tracking"));
    QCOMPARE(graph.instanceFilters.at(3).name, std::string("Profildaten"));

    QCOMPARE(graph.instanceFilters.at(0).group, 0);
    QCOMPARE(graph.instanceFilters.at(1).group, 1);
    QCOMPARE(graph.instanceFilters.at(2).group, 1);
    QCOMPARE(graph.instanceFilters.at(3).group, 2);

    QCOMPARE(graph.filterGroups.size(), std::size_t(3));
    QCOMPARE(graph.filterGroups.at(0).number, 0);
    QCOMPARE(graph.filterGroups.at(1).number, 1);
    QCOMPARE(graph.filterGroups.at(2).number, 2);
    QCOMPARE(graph.filterGroups.at(0).parent, -1);
    QCOMPARE(graph.filterGroups.at(1).parent, -1);
    QCOMPARE(graph.filterGroups.at(2).parent, -1);
    QCOMPARE(graph.filterGroups.at(0).name, std::string("Not grouped"));
    QCOMPARE(graph.filterGroups.at(1).name, std::string("Not grouped"));
    QCOMPARE(graph.filterGroups.at(2).name, std::string("Not grouped"));
    QCOMPARE(graph.filterGroups.at(0).sourceGraph, Poco::UUID("6932b184-42b5-4a42-8417-7ff81d066cab"));
    QCOMPARE(graph.filterGroups.at(1).sourceGraph, Poco::UUID("7772eaab-acf4-47cd-86dc-02affc8c68c0"));
    QCOMPARE(graph.filterGroups.at(2).sourceGraph, Poco::UUID("ca23f1a1-874d-4383-95fb-f22b65c513e9"));

    QCOMPARE(graph.pipes.size(), std::size_t(4));
    // ImageSource - ROISelector
    QCOMPARE(graph.pipes.at(0).sender, graph.instanceFilters.at(0).id);
    QCOMPARE(graph.pipes.at(0).receiver, graph.instanceFilters.at(1).id);
    // RoiSelector - LineTracking
    QCOMPARE(graph.pipes.at(1).sender, graph.instanceFilters.at(1).id);
    QCOMPARE(graph.pipes.at(1).receiver, graph.instanceFilters.at(2).id);
    // RoiSelector - LineProfileResult
    QCOMPARE(graph.pipes.at(2).sender, graph.instanceFilters.at(1).id);
    QCOMPARE(graph.pipes.at(2).receiver, graph.instanceFilters.at(3).id);
    // LineTracking - LineProfileResult
    QCOMPARE(graph.pipes.at(3).sender, graph.instanceFilters.at(2).id);
    QCOMPARE(graph.pipes.at(3).receiver, graph.instanceFilters.at(3).id);
}

void TestSubGraphModel::testCategoryTranslation_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");

    QTest::newRow("Input") << QStringLiteral("1 Input") << QStringLiteral("Input");
    QTest::newRow("INPUT") << QStringLiteral("1 INPUT") << QStringLiteral("Input");
    QTest::newRow("ROI & pre search") << QStringLiteral("2 ROI & preSearch") << QStringLiteral("ROI & Pre-search");
    QTest::newRow("Pre process") << QStringLiteral("3 Pre-Process") << QStringLiteral("Pre-processing");
    QTest::newRow("Process") << QStringLiteral("4 Process") << QStringLiteral("Processing");
    QTest::newRow("Post Process") << QStringLiteral("5 Post-Process") << QStringLiteral("Post-processing");
    QTest::newRow("Output") << QStringLiteral("6 Output") << QStringLiteral("Output");

    QTest::newRow("foo") << QStringLiteral("foo") << QStringLiteral("foo");
}

void TestSubGraphModel::testCategoryTranslation()
{
    SubGraphModel model;
    QFETCH(QString, input);
    QTEST(model.categoryToName(input), "output");
}

void TestSubGraphModel::testGraphAdding()
{
    SubGraphModel model;
    QSignalSpy modelResetSpy{&model, &SubGraphModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    QSignalSpy loadingChangedSpy{&model, &SubGraphModel::loadingChanged};
    QVERIFY(loadingChangedSpy.isValid());
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QCOMPARE(modelResetSpy.count(), 0);
    model.loadSubGraphs(QFINDTESTDATA("testdata/subgraphs/"), dir.path());
    QCOMPARE(model.isLoading(), true);
    QCOMPARE(loadingChangedSpy.count(), 1);
    QVERIFY(loadingChangedSpy.wait());
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(model.isLoading(), false);
    QCOMPARE(modelResetSpy.count(), 1);
    QCOMPARE(model.rowCount(), 5);

    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/graphs/minimal.xml"), dir.filePath(QStringLiteral("minimal.xml"))));
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 2);
    QTRY_COMPARE(loadingChangedSpy.count(), 4);
    QCOMPARE(model.isLoading(), false);
    QCOMPARE(model.rowCount(), 6);

    // delete again
    QVERIFY(QFile::remove(dir.filePath(QStringLiteral("minimal.xml"))));
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 3);
    QTRY_COMPARE(loadingChangedSpy.count(), 6);
    QCOMPARE(model.isLoading(), false);
    QCOMPARE(model.rowCount(), 5);
}

QTEST_GUILESS_MAIN(TestSubGraphModel)
#include "testSubGraphModel.moc"
