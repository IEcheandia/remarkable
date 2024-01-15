#include <QTest>
#include <QSignalSpy>

#include "../src/graphModel.h"
#include "../src/subGraphModel.h"
#include "../src/filterInstanceModel.h"

using precitec::storage::GraphModel;
using precitec::storage::SubGraphModel;
using precitec::storage::FilterInstanceModel;

class TestFilterInstanceModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testGraphId();
    void testGraphModel();
    void testSubGraphModel();
    void testLoadGraph();
    void testLoadSubGraph();
};

void TestFilterInstanceModel::testCtor()
{
    FilterInstanceModel model;
    QVERIFY(!model.graphModel());
    QVERIFY(!model.subGraphModel());
    QCOMPARE(model.graphId(), QUuid());
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.nullUuid(), QUuid());
    QCOMPARE(model.index(0, 0).isValid(), false);
    QCOMPARE(model.data(model.index(0, 0)).isValid(), false);

    const auto roleNames = model.roleNames();
    QCOMPARE(roleNames.size(), 6);
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("display"));
    QCOMPARE(roleNames[Qt::UserRole], QByteArrayLiteral("uuid"));
    QCOMPARE(roleNames[Qt::UserRole + 1], QByteArrayLiteral("group"));
    QCOMPARE(roleNames[Qt::UserRole + 2], QByteArrayLiteral("visibleAttributes"));
    QCOMPARE(roleNames[Qt::UserRole + 3], QByteArrayLiteral("maxVisibleUserLevel"));
    QCOMPARE(roleNames[Qt::UserRole + 4], QByteArrayLiteral("filterId"));
}

void TestFilterInstanceModel::testGraphId()
{
    FilterInstanceModel model;
    QSignalSpy graphIdChangedSpy{&model, &FilterInstanceModel::graphIdChanged};
    QVERIFY(graphIdChangedSpy.isValid());
    QSignalSpy modelResetSpy{&model, &FilterInstanceModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    QCOMPARE(model.graphId(), QUuid());

    const auto uuid = QUuid::createUuid();
    model.setGraphId(uuid);
    QCOMPARE(model.graphId(), uuid);
    QCOMPARE(graphIdChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);
    // no graph model, so init should not add entries
    QCOMPARE(model.rowCount(), 0);

    // setting same uuid should not change
    QCOMPARE(model.graphId(), uuid);
    QCOMPARE(graphIdChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);
}

void TestFilterInstanceModel::testGraphModel()
{
    auto graphModel = std::make_unique<GraphModel>();
    FilterInstanceModel model;
    QSignalSpy graphModelChangedSpy{&model, &FilterInstanceModel::graphModelChanged};
    QVERIFY(graphModelChangedSpy.isValid());
    QSignalSpy modelResetSpy{&model, &FilterInstanceModel::modelReset};
    QVERIFY(modelResetSpy.isValid());

    model.setGraphModel(graphModel.get());
    QCOMPARE(model.graphModel(), graphModel.get());

    QCOMPARE(graphModelChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);
    // no graphs in the model and no graph uuid set, so init should not add entries
    QCOMPARE(model.rowCount(), 0);

    // setting same should not change
    model.setGraphModel(graphModel.get());
    QCOMPARE(graphModelChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    // delete graph model should result in it being unset
    graphModel.reset();
    QCOMPARE(graphModelChangedSpy.count(), 2);
    QCOMPARE(modelResetSpy.count(), 2);
    QVERIFY(!model.graphModel());
}

void TestFilterInstanceModel::testSubGraphModel()
{
    auto graphModel = std::make_unique<SubGraphModel>();
    FilterInstanceModel model;
    QSignalSpy graphModelChangedSpy{&model, &FilterInstanceModel::subGraphModelChanged};
    QVERIFY(graphModelChangedSpy.isValid());
    QSignalSpy modelResetSpy{&model, &FilterInstanceModel::modelReset};
    QVERIFY(modelResetSpy.isValid());

    model.setSubGraphModel(graphModel.get());
    QCOMPARE(model.subGraphModel(), graphModel.get());

    QCOMPARE(graphModelChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);
    // no graphs in the model and no graph uuid set, so init should not add entries
    QCOMPARE(model.rowCount(), 0);

    // setting same should not change
    model.setSubGraphModel(graphModel.get());
    QCOMPARE(graphModelChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    // delete graph model should result in it being unset
    graphModel.reset();
    QCOMPARE(graphModelChangedSpy.count(), 2);
    QCOMPARE(modelResetSpy.count(), 2);
    QVERIFY(!model.subGraphModel());
}

void TestFilterInstanceModel::testLoadGraph()
{
    auto graphModel = std::make_unique<GraphModel>();
    FilterInstanceModel model;
    QSignalSpy modelResetSpy{&model, &FilterInstanceModel::modelReset};
    QVERIFY(modelResetSpy.isValid());

    model.setGraphId(QUuid{QStringLiteral("e58abf42-77a6-4456-9f78-56e002b38549")});
    QCOMPARE(modelResetSpy.count(), 1);
    model.setGraphModel(graphModel.get());
    QCOMPARE(model.graphModel(), graphModel.get());
    QCOMPARE(modelResetSpy.count(), 2);
    // no graphs in the model and no graph uuid set, so init should not add entries
    QCOMPARE(model.rowCount(), 0);

    graphModel->loadGraphs(QFINDTESTDATA("testdata/graphs"));
    QVERIFY(modelResetSpy.wait());

    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.index(0, 0).isValid(), true);
    QCOMPARE(model.rowCount(model.index(0, 0)), 0);

    QCOMPARE(model.index(0, 0).data(Qt::DisplayRole).toString(), QStringLiteral("A Filter"));
    QCOMPARE(model.index(0, 0).data(Qt::UserRole).toUuid(), QUuid(QStringLiteral("cb9f223c-6762-4d8e-9faa-bca71f951444")));
    QCOMPARE(model.index(0, 0).data(Qt::UserRole+1).toInt(), -1);
    QCOMPARE(model.index(0, 0).data(Qt::UserRole+2).toBool(), true);
    QCOMPARE(model.index(0, 0).data(Qt::UserRole+3).toInt(), 3);
    QCOMPARE(model.index(0, 0).data(Qt::UserRole+4).toString(), QStringLiteral("be7d83e3-9b98-41f2-a484-d0e74e5e8ea2"));
    QCOMPARE(model.index(0, 0).data(Qt::UserRole+5).isValid(), false);
}

void TestFilterInstanceModel::testLoadSubGraph()
{
    auto graphModel = std::make_unique<SubGraphModel>();
    graphModel->loadSubGraphs(QFINDTESTDATA("testdata/subgraphs"));
    QTRY_COMPARE(graphModel->rowCount(), 5);

    FilterInstanceModel model;
    QSignalSpy modelResetSpy{&model, &FilterInstanceModel::modelReset};
    QVERIFY(modelResetSpy.isValid());

    model.setSubGraphModel(graphModel.get());
    QCOMPARE(model.subGraphModel(), graphModel.get());
    QCOMPARE(modelResetSpy.count(), 1);
    // no graphs in the model and no graph uuid set, so init should not add entries
    QCOMPARE(model.rowCount(), 0);

    const auto graphId = graphModel->generateGraphId({{QByteArrayLiteral("6932b184-42b5-4a42-8417-7ff81d066cab"), QByteArrayLiteral("7772eaab-acf4-47cd-86dc-02affc8c68c0"), QByteArrayLiteral("ca23f1a1-874d-4383-95fb-f22b65c513e9")}});

    model.setGraphId(graphId);
    QCOMPARE(modelResetSpy.count(), 2);

    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(model.index(0, 0).isValid(), true);
    QCOMPARE(model.rowCount(model.index(0, 0)), 0);

    QCOMPARE(model.index(0, 0).data(Qt::DisplayRole).toString(), QStringLiteral("Bildquelle"));
    QCOMPARE(model.index(0, 0).data(Qt::UserRole).toUuid(), QUuid(QStringLiteral("6aeced48-9684-468d-93bc-ed006289f57c")));
    QCOMPARE(model.index(0, 0).data(Qt::UserRole+1).toInt(), 0);

    QCOMPARE(model.index(1, 0).data(Qt::DisplayRole).toString(), QStringLiteral("ROI Auswahl"));
    QCOMPARE(model.index(1, 0).data(Qt::UserRole).toUuid(), QUuid(QStringLiteral("50b6f7d1-4604-43f9-ab9d-e7a96bffc6cf")));
    QCOMPARE(model.index(1, 0).data(Qt::UserRole+1).toInt(), 1);

    QCOMPARE(model.index(2, 0).data(Qt::DisplayRole).toString(), QStringLiteral("Line Tracking"));
    QCOMPARE(model.index(2, 0).data(Qt::UserRole).toUuid(), QUuid(QStringLiteral("d2e82f6f-a239-45a2-b8be-f13229713b05")));
    QCOMPARE(model.index(2, 0).data(Qt::UserRole+1).toInt(), 1);

    QCOMPARE(model.index(3, 0).data(Qt::DisplayRole).toString(), QStringLiteral("Profildaten"));
    QCOMPARE(model.index(3, 0).data(Qt::UserRole).toUuid(), QUuid(QStringLiteral("73c825fa-a99b-45e4-9501-818a0f7d24b8")));
    QCOMPARE(model.index(3, 0).data(Qt::UserRole+1).toInt(), 2);
}

QTEST_GUILESS_MAIN(TestFilterInstanceModel)
#include "testFilterInstanceModel.moc"
