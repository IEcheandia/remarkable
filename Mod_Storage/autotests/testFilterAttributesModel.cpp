#include <QTest>
#include <QSignalSpy>

#include "../src/filterAttributeModel.h"
#include "../src/attributeModel.h"
#include "../src/graphModel.h"
#include "../src/subGraphModel.h"
#include "../src/attributeGroup.h"
#include "../src/attributeGroupItem.h"

using precitec::storage::FilterAttributeModel;
using precitec::storage::AttributeModel;
using precitec::storage::GraphModel;
using precitec::storage::SubGraphModel;
using precitec::storage::AttributeGroup;
using precitec::storage::AttributeGroupItem;

class TestFilterAttributeModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testRoleNames();
    void testFilterInstance();
    void testGraphId();
    void testGraphModel();
    void testSubGraphModel();
    void testAttributeModel();
    void testLoadGraph();
    void testLoadSubGraph();
};

void TestFilterAttributeModel::testCtor()
{
    auto model = new FilterAttributeModel{this};

    QCOMPARE(model->rowCount(), 0);
    QVERIFY(model->filterInstance().isNull());
    QVERIFY(model->graphId().isNull());
    QVERIFY(!model->attributeModel());
    QVERIFY(!model->subGraphModel());
    QVERIFY(!model->graphModel());
}

void TestFilterAttributeModel::testRoleNames()
{
    auto model = new FilterAttributeModel{this};

    const auto roleNames = model->roleNames();
    QCOMPARE(roleNames.size(), 13);
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("contentName"));
    QCOMPARE(roleNames[Qt::UserRole], QByteArrayLiteral("unit"));
    QCOMPARE(roleNames[Qt::UserRole + 1], QByteArrayLiteral("description"));
    QCOMPARE(roleNames[Qt::UserRole + 2], QByteArrayLiteral("minValue"));
    QCOMPARE(roleNames[Qt::UserRole + 3], QByteArrayLiteral("maxValue"));
    QCOMPARE(roleNames[Qt::UserRole + 4], QByteArrayLiteral("defaultValue"));
    QCOMPARE(roleNames[Qt::UserRole + 5], QByteArrayLiteral("userLevel"));
    QCOMPARE(roleNames[Qt::UserRole + 6], QByteArrayLiteral("helpFile"));
    QCOMPARE(roleNames[Qt::UserRole + 7], QByteArrayLiteral("selected"));
    QCOMPARE(roleNames[Qt::UserRole + 8], QByteArrayLiteral("editListOrder"));
    QCOMPARE(roleNames[Qt::UserRole + 9], QByteArrayLiteral("visible"));
    QCOMPARE(roleNames[Qt::UserRole + 10], QByteArrayLiteral("publicity"));
    QCOMPARE(roleNames[Qt::UserRole + 11], QByteArrayLiteral("group"));
}

void TestFilterAttributeModel::testFilterInstance()
{
    auto model = new FilterAttributeModel{this};

    QSignalSpy filterInstanceChangedSpy{model, &FilterAttributeModel::filterInstanceChanged};
    QVERIFY(filterInstanceChangedSpy.isValid());

    QSignalSpy modelResetSpy{model, &FilterAttributeModel::modelReset};
    QVERIFY(modelResetSpy.isValid());

    QVERIFY(model->filterInstance().isNull());

    // setting same should not emit
    model->setFilterInstance({});
    QCOMPARE(filterInstanceChangedSpy.count(), 0);
    QCOMPARE(modelResetSpy.count(), 0);

    const auto uuid = QUuid::createUuid();
    model->setFilterInstance(uuid);
    QCOMPARE(model->filterInstance(), uuid);
    QCOMPARE(filterInstanceChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    // setting same uuid should not change
    model->setFilterInstance(uuid);
    QCOMPARE(filterInstanceChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);
}

void TestFilterAttributeModel::testGraphId()
{
    auto model = new FilterAttributeModel{this};

    QSignalSpy graphIdChangedSpy{model, &FilterAttributeModel::graphIdChanged};
    QVERIFY(graphIdChangedSpy.isValid());

    QSignalSpy graphChangedSpy{model, &FilterAttributeModel::graphChanged};
    QVERIFY(graphChangedSpy.isValid());

    QSignalSpy modelResetSpy{model, &FilterAttributeModel::modelReset};
    QVERIFY(modelResetSpy.isValid());

    QVERIFY(model->graphId().isNull());

    // setting same should not emit
    model->setGraphId({});
    QCOMPARE(graphIdChangedSpy.count(), 0);
    QCOMPARE(graphChangedSpy.count(), 0);
    QCOMPARE(modelResetSpy.count(), 0);

    const auto uuid = QUuid::createUuid();

    model->setGraphId(uuid);
    QCOMPARE(model->graphId(), uuid);
    QCOMPARE(graphIdChangedSpy.count(), 1);
    QCOMPARE(graphChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    // setting same uuid should not change
    QCOMPARE(model->graphId(), uuid);
    QCOMPARE(graphIdChangedSpy.count(), 1);
    QCOMPARE(graphChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);
}

void TestFilterAttributeModel::testGraphModel()
{
    auto model = new FilterAttributeModel{this};

    QSignalSpy graphModelChangedSpy{model, &FilterAttributeModel::graphModelChanged};
    QVERIFY(graphModelChangedSpy.isValid());

    QSignalSpy graphChangedSpy{model, &FilterAttributeModel::graphChanged};
    QVERIFY(graphChangedSpy.isValid());

    QSignalSpy modelResetSpy{model, &FilterAttributeModel::modelReset};
    QVERIFY(modelResetSpy.isValid());

    QVERIFY(!model->graphModel());

    model->setGraphModel(nullptr);
    QCOMPARE(graphModelChangedSpy.count(), 0);
    QCOMPARE(graphChangedSpy.count(), 0);
    QCOMPARE(modelResetSpy.count(), 0);

    auto graphModel = std::make_unique<GraphModel>();

    model->setGraphModel(graphModel.get());
    QCOMPARE(model->graphModel(), graphModel.get());
    QCOMPARE(graphModelChangedSpy.count(), 1);
    QCOMPARE(graphChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    // setting same should not change
    model->setGraphModel(graphModel.get());
    QCOMPARE(graphModelChangedSpy.count(), 1);
    QCOMPARE(graphChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    // delete graph model should result in it being unset
    graphModel.reset();
    QCOMPARE(graphModelChangedSpy.count(), 2);
    QCOMPARE(graphChangedSpy.count(), 2);
    QCOMPARE(modelResetSpy.count(), 2);
    QVERIFY(!model->graphModel());
}

void TestFilterAttributeModel::testSubGraphModel()
{
    auto model = new FilterAttributeModel{this};

    QSignalSpy subGraphModelChangedSpy{model, &FilterAttributeModel::subGraphModelChanged};
    QVERIFY(subGraphModelChangedSpy.isValid());

    QSignalSpy graphChangedSpy{model, &FilterAttributeModel::graphChanged};
    QVERIFY(graphChangedSpy.isValid());

    QSignalSpy modelResetSpy{model, &FilterAttributeModel::modelReset};
    QVERIFY(modelResetSpy.isValid());

    QVERIFY(!model->subGraphModel());

    model->setSubGraphModel(nullptr);
    QCOMPARE(subGraphModelChangedSpy.count(), 0);
    QCOMPARE(graphChangedSpy.count(), 0);
    QCOMPARE(modelResetSpy.count(), 0);

    auto subGraphModel = std::make_unique<SubGraphModel>();

    model->setSubGraphModel(subGraphModel.get());
    QCOMPARE(model->subGraphModel(), subGraphModel.get());
    QCOMPARE(subGraphModelChangedSpy.count(), 1);
    QCOMPARE(graphChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    // setting same should not change
    model->setSubGraphModel(subGraphModel.get());
    QCOMPARE(subGraphModelChangedSpy.count(), 1);
    QCOMPARE(graphChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    // delete graph model should result in it being unset
    subGraphModel.reset();
    QCOMPARE(subGraphModelChangedSpy.count(), 2);
    QCOMPARE(graphChangedSpy.count(), 2);
    QCOMPARE(modelResetSpy.count(), 2);
    QVERIFY(!model->subGraphModel());
}

void TestFilterAttributeModel::testAttributeModel()
{
    auto model = new FilterAttributeModel{this};

    QSignalSpy attributeModelChangedSpy(model, &FilterAttributeModel::attributeModelChanged);
    QVERIFY(attributeModelChangedSpy.isValid());

    QSignalSpy modelResetSpy{model, &FilterAttributeModel::modelReset};
    QVERIFY(modelResetSpy.isValid());

    QVERIFY(!model->attributeModel());

    model->setAttributeModel(nullptr);
    QCOMPARE(attributeModelChangedSpy.count(), 0);
    QCOMPARE(modelResetSpy.count(), 0);

    std::unique_ptr<AttributeModel> am = std::make_unique<AttributeModel>();

    model->setAttributeModel(am.get());
    QCOMPARE(model->attributeModel(), am.get());
    QCOMPARE(attributeModelChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    model->setAttributeModel(am.get());
    QCOMPARE(attributeModelChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    QSignalSpy attributeModelResetSpy(am.get(), &AttributeModel::modelReset);
    QVERIFY(attributeModelResetSpy.isValid());

    am->load(QFINDTESTDATA("testdata/attributes/attributes.json"));
    QVERIFY(attributeModelResetSpy.wait());
    QCOMPARE(attributeModelResetSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 2);
}

void TestFilterAttributeModel::testLoadGraph()
{
    auto model = new FilterAttributeModel{this};

    QSignalSpy modelResetSpy{model, &FilterAttributeModel::modelReset};
    QVERIFY(modelResetSpy.isValid());

    QSignalSpy graphChangedSpy{model, &FilterAttributeModel::graphChanged};
    QVERIFY(graphChangedSpy.isValid());

    auto graphModel = std::make_unique<GraphModel>();

    model->setGraphId(QUuid{QStringLiteral("e58abf42-77a6-4456-9f78-56e002b38549")});
    QCOMPARE(modelResetSpy.count(), 1);
    QCOMPARE(graphChangedSpy.count(), 1);

    model->setGraphModel(graphModel.get());
    QCOMPARE(model->graphModel(), graphModel.get());
    QCOMPARE(modelResetSpy.count(), 2);
    QCOMPARE(graphChangedSpy.count(), 2);

    // no graphs in the model and no graph uuid set
    // no filter instance set
    // no AttributeModel set
    QCOMPARE(model->rowCount(), 0);

    graphModel->loadGraphs(QFINDTESTDATA("testdata/graphs"));
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 3);
    QCOMPARE(graphChangedSpy.count(), 3);

    // no filter instance set
    // no AttributeModel set
    QCOMPARE(model->rowCount(), 0);

    std::unique_ptr<AttributeModel> am = std::make_unique<AttributeModel>();

    model->setAttributeModel(am.get());
    QCOMPARE(model->attributeModel(), am.get());
    QCOMPARE(modelResetSpy.count(), 4);
    QCOMPARE(graphChangedSpy.count(), 3);

    // no filter instance set
    QCOMPARE(model->rowCount(), 0);

    model->setFilterInstance({QStringLiteral("cb9f223c-6762-4d8e-9faa-bca71f951444")});
    QCOMPARE(modelResetSpy.count(), 5);
    QCOMPARE(graphChangedSpy.count(), 3);

    QCOMPARE(model->rowCount(), 1);
    QVERIFY(model->index(0, 0).isValid());

    QCOMPARE(model->index(0, 0).data(Qt::DisplayRole).toString(), QStringLiteral("An attribute"));
    QCOMPARE(model->index(0, 0).data(Qt::UserRole).toString(), QLatin1String(""));
    QCOMPARE(model->index(0, 0).data(Qt::UserRole + 1).toString(), QLatin1String(""));
    QCOMPARE(model->index(0, 0).data(Qt::UserRole + 2), QVariant());
    QCOMPARE(model->index(0, 0).data(Qt::UserRole + 3), QVariant());
    QCOMPARE(model->index(0, 0).data(Qt::UserRole + 4), QVariant());
    QCOMPARE(model->index(0, 0).data(Qt::UserRole + 5).toInt(), 3);
    QCOMPARE(model->index(0, 0).data(Qt::UserRole + 6).toString(), QStringLiteral("test.pdf"));
    QCOMPARE(model->index(0, 0).data(Qt::UserRole + 7).toBool(), false);
    QCOMPARE(model->index(0, 0).data(Qt::UserRole + 8).toInt(), 0);
    QCOMPARE(model->index(0, 0).data(Qt::UserRole + 9).toBool(), true);
    QCOMPARE(model->index(0, 0).data(Qt::UserRole + 10).toBool(), true);
    QCOMPARE(model->indexForAttributeInstance(QUuid{"0b0e26ea-cd24-4298-af96-4b3655f62ff4"}), model->index(0, 0));
    QCOMPARE(model->indexForAttributeInstance(QUuid{"0b0e26ea-cd24-4298-af96-4b3655f62ff5"}).isValid(), false);

    auto attibuteNotFound = model->index(0, 0).data(Qt::UserRole + 11).value<AttributeGroup*>()->items().at(0);

    QVERIFY(!attibuteNotFound->attribute());
    QCOMPARE(attibuteNotFound->name(), QStringLiteral("An attribute"));
    QCOMPARE(attibuteNotFound->uuid(), QUuid("640BB3EE-6145-4E25-BBAB-015534C6C3C2"));
    QCOMPARE(attibuteNotFound->instanceId(), QUuid("0b0e26ea-cd24-4298-af96-4b3655f62ff4"));
    QCOMPARE(attibuteNotFound->userLevel(), 3);
    QCOMPARE(attibuteNotFound->publicity(), true);
    QCOMPARE(attibuteNotFound->value(), QStringLiteral("1"));
    QCOMPARE(attibuteNotFound->helpFile(), QStringLiteral("test.pdf"));
    QCOMPARE(attibuteNotFound->unit(), QLatin1String(""));
    QCOMPARE(attibuteNotFound->description(), QLatin1String(""));
    QCOMPARE(attibuteNotFound->defaultValue(), QVariant());
    QCOMPARE(attibuteNotFound->maxValue(), QVariant());
    QCOMPARE(attibuteNotFound->minValue(), QVariant());
    QCOMPARE(attibuteNotFound->groupIndex(), 0);

    am->load(QFINDTESTDATA("testdata/attributes/attributes.json"));
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 6);

    QCOMPARE(model->index(0, 0).data(Qt::DisplayRole).toString(), QStringLiteral("Precitec.Filter.Attribute.LinearLut.Min.Name"));
    QCOMPARE(model->index(0, 0).data(Qt::UserRole).toString(), QStringLiteral("Precitec.Filter.Attribute.LinearLut.Min.Unit"));
    QCOMPARE(model->index(0, 0).data(Qt::UserRole + 1).toString(), QStringLiteral("Precitec.Filter.Attribute.LinearLut.Min.Beschreibung"));
    QCOMPARE(model->index(0, 0).data(Qt::UserRole + 2), QVariant(0));
    QCOMPARE(model->index(0, 0).data(Qt::UserRole + 3), QVariant(255));
    QCOMPARE(model->index(0, 0).data(Qt::UserRole + 4), QVariant(1));
    QCOMPARE(model->index(0, 0).data(Qt::UserRole + 5).toInt(), 3);
    QCOMPARE(model->index(0, 0).data(Qt::UserRole + 6).toString(), QStringLiteral("test.pdf"));
    QCOMPARE(model->index(0, 0).data(Qt::UserRole + 7).toBool(), false);
    QCOMPARE(model->index(0, 0).data(Qt::UserRole + 8).toInt(), 1);
    QCOMPARE(model->index(0, 0).data(Qt::UserRole + 9).toBool(), true);
    QCOMPARE(model->index(0, 0).data(Qt::UserRole + 10).toBool(), true);

    QCOMPARE(model->index(0, 0).data(Qt::UserRole + 11).value<AttributeGroup*>()->items().size(), 1);

    auto attibute = model->index(0, 0).data(Qt::UserRole + 11).value<AttributeGroup*>()->items().at(0);

    QVERIFY(attibute->attribute());
    QCOMPARE(attibute->name(), QStringLiteral("An attribute"));
    QCOMPARE(attibute->uuid(), QUuid("640BB3EE-6145-4E25-BBAB-015534C6C3C2"));
    QCOMPARE(attibute->instanceId(), QUuid("0b0e26ea-cd24-4298-af96-4b3655f62ff4"));
    QCOMPARE(attibute->userLevel(), 3);
    QCOMPARE(attibute->publicity(), true);
    QCOMPARE(attibute->value(), QStringLiteral("1"));
    QCOMPARE(attibute->helpFile(), QStringLiteral("test.pdf"));
    QCOMPARE(attibute->unit(), QLatin1String("Precitec.Filter.Attribute.LinearLut.Min.Unit"));
    QCOMPARE(attibute->description(), QLatin1String("Precitec.Filter.Attribute.LinearLut.Min.Beschreibung"));
    QCOMPARE(attibute->defaultValue(), QVariant(1));
    QCOMPARE(attibute->maxValue(), QVariant(255));
    QCOMPARE(attibute->minValue(), QVariant(0));
    QCOMPARE(attibute->groupIndex(), 2);
}

void TestFilterAttributeModel::testLoadSubGraph()
{
    auto model = new FilterAttributeModel{this};

    QSignalSpy modelResetSpy{model, &FilterAttributeModel::modelReset};
    QVERIFY(modelResetSpy.isValid());

    QSignalSpy graphChangedSpy{model, &FilterAttributeModel::graphChanged};
    QVERIFY(graphChangedSpy.isValid());

    auto subGraphModel = std::make_unique<SubGraphModel>();
    subGraphModel->loadSubGraphs(QFINDTESTDATA("testdata/subgraphs"));
    QTRY_COMPARE(subGraphModel->rowCount(), 5);

    model->setSubGraphModel(subGraphModel.get());
    QCOMPARE(model->subGraphModel(), subGraphModel.get());
    QCOMPARE(modelResetSpy.count(), 1);
    QCOMPARE(graphChangedSpy.count(), 1);

    // no graphs in the model and no graph uuid set
    // no filter instance set
    // no AttributeModel set
    QCOMPARE(model->rowCount(), 0);

    const auto graphId = subGraphModel->generateGraphId({{QByteArrayLiteral("6932b184-42b5-4a42-8417-7ff81d066cab"), QByteArrayLiteral("7772eaab-acf4-47cd-86dc-02affc8c68c0"), QByteArrayLiteral("ca23f1a1-874d-4383-95fb-f22b65c513e9")}});

    model->setGraphId(graphId);
    QCOMPARE(modelResetSpy.count(), 2);
    QCOMPARE(graphChangedSpy.count(), 2);

    // no filter instance set
    // no AttributeModel set
    QCOMPARE(model->rowCount(), 0);

    std::unique_ptr<AttributeModel> am = std::make_unique<AttributeModel>();

    model->setAttributeModel(am.get());
    QCOMPARE(model->attributeModel(), am.get());
    QCOMPARE(modelResetSpy.count(), 3);
    QCOMPARE(graphChangedSpy.count(), 2);

    model->setFilterInstance({QStringLiteral("6aeced48-9684-468d-93bc-ed006289f57c")});
    QCOMPARE(modelResetSpy.count(), 4);
    QCOMPARE(graphChangedSpy.count(), 2);

    QCOMPARE(model->rowCount(), 3);
}

QTEST_GUILESS_MAIN(TestFilterAttributeModel)
#include "testFilterAttributesModel.moc"
