#include <QTest>
#include <QSignalSpy>

#include "../src/assemblyImageFromProductInstanceTableModel.h"
#include "metaDataWriterCommand.h"

using precitec::gui::AssemblyImageInspectionModel;
using precitec::gui::AssemblyImageFromProductInstanceTableModel;
using precitec::storage::MetaDataWriterCommand;
using precitec::storage::Product;
using precitec::storage::ProductInstanceTableModel;

class TestAssemblyImageFromProductInstanceTableModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testRoleNames();
    void testInitWithoutModel();
    void testSetProductInstanceTableModel();
    void testInvalidRow();
    void testData();
};

void TestAssemblyImageFromProductInstanceTableModel::testCtor()
{
    AssemblyImageFromProductInstanceTableModel model;
    QCOMPARE(model.rowCount(), 0);
    QVERIFY(!model.productInstanceTableModel());
}

void TestAssemblyImageFromProductInstanceTableModel::testRoleNames()
{
    AssemblyImageFromProductInstanceTableModel model;
    const auto &roleNames = model.roleNames();
    QCOMPARE(roleNames.size(), 4);
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("seamName"));
    QCOMPARE(roleNames[Qt::UserRole], QByteArrayLiteral("position"));
    QCOMPARE(roleNames[Qt::UserRole + 2], QByteArrayLiteral("state"));
    QCOMPARE(roleNames[Qt::UserRole + 3], QByteArrayLiteral("seam"));
}

void TestAssemblyImageFromProductInstanceTableModel::testInitWithoutModel()
{
    AssemblyImageFromProductInstanceTableModel model;
    QSignalSpy resetSpy{&model, &AssemblyImageFromProductInstanceTableModel::modelReset};
    QVERIFY(resetSpy.isValid());
    model.init(0);
    QCOMPARE(resetSpy.count(), 1);
    QCOMPARE(model.rowCount(), 0);
}

void TestAssemblyImageFromProductInstanceTableModel::testSetProductInstanceTableModel()
{
    AssemblyImageFromProductInstanceTableModel model;
    QSignalSpy modelChangedSpy{&model, &AssemblyImageFromProductInstanceTableModel::productInstanceTableModelChanged};
    QVERIFY(modelChangedSpy.isValid());

    std::unique_ptr<ProductInstanceTableModel> productInstanceTableModel{new ProductInstanceTableModel};

    QVERIFY(!model.productInstanceTableModel());
    model.setProductInstanceTableModel(productInstanceTableModel.get());
    QCOMPARE(modelChangedSpy.size(), 1);
    QCOMPARE(model.productInstanceTableModel(), productInstanceTableModel.get());

    // set again should not change
    model.setProductInstanceTableModel(productInstanceTableModel.get());
    QCOMPARE(modelChangedSpy.size(), 1);

    productInstanceTableModel.reset();
    QCOMPARE(modelChangedSpy.size(), 2);
    QVERIFY(!model.productInstanceTableModel());
}

void TestAssemblyImageFromProductInstanceTableModel::testInvalidRow()
{
    AssemblyImageFromProductInstanceTableModel model;
    ProductInstanceTableModel productInstanceTableModel;
    QCOMPARE(productInstanceTableModel.rowCount(), 0);

    model.setProductInstanceTableModel(&productInstanceTableModel);
    model.init(0);
    QCOMPARE(model.rowCount(), 0);
    model.init(1);
    QCOMPARE(model.rowCount(), 0);
}

void TestAssemblyImageFromProductInstanceTableModel::testData()
{
    AssemblyImageFromProductInstanceTableModel model;

    // init the ProductInstanceTableModel
    // create a Product
    const QString fileName = QFINDTESTDATA("../../Mod_Storage/autotests/testdata/products/seamuuid.json");
    QVERIFY(!fileName.isEmpty());

    auto product = Product::fromJson(fileName);
    QVERIFY(product);
    QCOMPARE(product->uuid().toString(), QStringLiteral("{1f086211-fbd4-4493-a580-6ff11e4925de}"));

    if (auto seam = product->findSeam("99f71378-ef2a-4ada-8c1f-b022c53505e4"))
    {
        seam->setName(QStringLiteral("First Name"));
        seam->setPositionInAssemblyImage(QPointF{1, 2});
    }
    if (auto seam = product->findSeam("8471ced1-ab55-4138-90a8-20fe49426acc"))
    {
        seam->setName(QStringLiteral("Second"));
        seam->setPositionInAssemblyImage(QPointF{3, 4});
    }

    // and a directory
    QTemporaryDir dir;
    const auto path = dir.path() + QStringLiteral("/1f086211-fbd4-4493-a580-6ff11e4925de/1F086211-FBD4-4493-A580-6FF11E4925DE-SN-1/");
    QVERIFY(QDir{}.mkpath(path));

    QJsonArray seamJson {
        QJsonObject {
            {QStringLiteral("uuid"), "99f71378-ef2a-4ada-8c1f-b022c53505e4"},
            {QStringLiteral("nio"), false}
        },
        QJsonObject {
            {QStringLiteral("uuid"), "8471ced1-ab55-4138-90a8-20fe49426acc"},
            {QStringLiteral("nio"), false}
        },
        QJsonObject {
            {QStringLiteral("uuid"), "56178094-2d5c-47ed-81e5-528094e5f8fe"},
            {QStringLiteral("nio"), true}
        },
        QJsonObject {
            {QStringLiteral("uuid"), "b7872d0b-e19f-4f28-b731-3f6931b5199c"},
            {QStringLiteral("nio"), false}
        },
        QJsonObject {
            {QStringLiteral("uuid"), "68575ccc-3c34-4dc0-94ee-c92bb8811e53"},
            {QStringLiteral("nio"), true}
        }
    };

    QJsonArray seriesJson {
        QJsonObject {
            {QStringLiteral("uuid"), "bcc3332d-dc4d-4b94-9dc9-604262ec8666"},
            {QStringLiteral("nio"), false}
        },
        QJsonObject {
            {QStringLiteral("uuid"), "f9d9a202-965c-4500-9bc4-41a829072d15"},
            {QStringLiteral("nio"), true}
        },
        QJsonObject {
            {QStringLiteral("uuid"), "2F086211-FBD4-4493-A580-6FF11E4925DE"},
            {QStringLiteral("nio"), true}
        }
    };

    // create metadata
    MetaDataWriterCommand writer{path, {
        {QStringLiteral("serialNumber"), 1},
        {QStringLiteral("nio"), false},
        {QStringLiteral("nioSwitchedOff"), false},
        {QStringLiteral("date"), QDateTime{}.toString(Qt::ISODateWithMs)},
        {QStringLiteral("processedSeamSeries"), seriesJson},
        {QStringLiteral("processedSeams"), seamJson}
    }};
    writer.execute();

    auto productInstanceTableModel = new ProductInstanceTableModel{this};
    model.setProductInstanceTableModel(productInstanceTableModel);

    QSignalSpy dataChangedSpy(productInstanceTableModel, &ProductInstanceTableModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());

    QSignalSpy rowsInsertedSpy{productInstanceTableModel, &ProductInstanceTableModel::rowsInserted};
    QVERIFY(rowsInsertedSpy.isValid());

    QSignalSpy columnsInsertedSpy{productInstanceTableModel, &ProductInstanceTableModel::columnsInserted};
    QVERIFY(columnsInsertedSpy.isValid());

    QSignalSpy loadingChangedSpy(productInstanceTableModel, &ProductInstanceTableModel::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());

    productInstanceTableModel->setDirectory(dir.path());
    productInstanceTableModel->setProduct(product);

    QCOMPARE(loadingChangedSpy.count(), 1);
    QVERIFY(loadingChangedSpy.wait());

    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(productInstanceTableModel->loading(), false);

    QCOMPARE(productInstanceTableModel->rowCount(), 1);
    QCOMPARE(productInstanceTableModel->columnCount(), 7);

    QTRY_COMPARE(dataChangedSpy.count(), 6);

    model.init(0);
    QCOMPARE(model.rowCount(), 6);
    QCOMPARE(model.data(model.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("First Name"));
    QCOMPARE(model.data(model.index(0, 0), Qt::UserRole).toPointF(), QPointF(1, 2));
    QCOMPARE(model.data(model.index(0, 0), Qt::UserRole + 2).value<AssemblyImageInspectionModel::SeamState>(), AssemblyImageInspectionModel::SeamState::Success);

    QCOMPARE(model.data(model.index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("Second"));
    QCOMPARE(model.data(model.index(1, 0), Qt::UserRole).toPointF(), QPointF(3, 4));
    QCOMPARE(model.data(model.index(1, 0), Qt::UserRole + 2).value<AssemblyImageInspectionModel::SeamState>(), AssemblyImageInspectionModel::SeamState::Success);

    QCOMPARE(model.data(model.index(2, 0), Qt::DisplayRole).toString(), QString());
    QCOMPARE(model.data(model.index(2, 0), Qt::UserRole).toPointF(), QPointF(-1, -1));
    QCOMPARE(model.data(model.index(2, 0), Qt::UserRole + 2).value<AssemblyImageInspectionModel::SeamState>(), AssemblyImageInspectionModel::SeamState::Failure);

    QCOMPARE(model.data(model.index(3, 0), Qt::DisplayRole).toString(), QString());
    QCOMPARE(model.data(model.index(3, 0), Qt::UserRole).toPointF(), QPointF(-1, -1));
    QCOMPARE(model.data(model.index(3, 0), Qt::UserRole + 2).value<AssemblyImageInspectionModel::SeamState>(), AssemblyImageInspectionModel::SeamState::Success);

    QCOMPARE(model.data(model.index(4, 0), Qt::DisplayRole).toString(), QString());
    QCOMPARE(model.data(model.index(4, 0), Qt::UserRole).toPointF(), QPointF(-1, -1));
    QCOMPARE(model.data(model.index(4, 0), Qt::UserRole + 2).value<AssemblyImageInspectionModel::SeamState>(), AssemblyImageInspectionModel::SeamState::Failure);

    QCOMPARE(model.data(model.index(5, 0), Qt::DisplayRole).toString(), QString());
    QCOMPARE(model.data(model.index(5, 0), Qt::UserRole).toPointF(), QPointF(-1, -1));
    QCOMPARE(model.data(model.index(5, 0), Qt::UserRole + 2).value<AssemblyImageInspectionModel::SeamState>(), AssemblyImageInspectionModel::SeamState::Uninspected);
}

QTEST_GUILESS_MAIN(TestAssemblyImageFromProductInstanceTableModel)
#include "testAssemblyImageFromProductInstanceTableModel.moc"
