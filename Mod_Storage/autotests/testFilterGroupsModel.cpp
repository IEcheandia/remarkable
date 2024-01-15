#include <QTest>
#include <QSignalSpy>

#include "../src/filterGroupsModel.h"
#include "../src/graphModel.h"

using precitec::storage::FilterGroupsModel;
using precitec::storage::GraphModel;

class TestFilterGroupsModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testLoadGraph();
};

void TestFilterGroupsModel::testCtor()
{
    FilterGroupsModel model;
    QVERIFY(!model.graphModel());
    QVERIFY(!model.subGraphModel());
    QCOMPARE(model.graphId(), QUuid());
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.nullUuid(), QUuid());
    QCOMPARE(model.index(0, 0).isValid(), false);
    QCOMPARE(model.data(model.index(0, 0)).isValid(), false);

    const auto roleNames = model.roleNames();
    QCOMPARE(roleNames.size(), 4);
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("name"));
    QCOMPARE(roleNames[Qt::UserRole], QByteArrayLiteral("number"));
    QCOMPARE(roleNames[Qt::UserRole + 2], QByteArrayLiteral("visibleAttributes"));
    QCOMPARE(roleNames[Qt::UserRole + 3], QByteArrayLiteral("maxVisibleUserLevel"));
}

void TestFilterGroupsModel::testLoadGraph()
{
    auto graphModel = std::make_unique<GraphModel>();
    FilterGroupsModel model;
    QSignalSpy modelResetSpy{&model, &FilterGroupsModel::modelReset};
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

    QCOMPARE(model.index(0, 0).data(Qt::DisplayRole).toString(), QStringLiteral("Not grouped"));
    QCOMPARE(model.index(0, 0).data(Qt::UserRole).toInt(), -1);
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 2).toBool(), true);
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 3).toInt(), 3);

    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 1).isValid(), false);
}

QTEST_GUILESS_MAIN(TestFilterGroupsModel)
#include "testFilterGroupsModel.moc"
