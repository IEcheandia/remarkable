#include <QTest>
#include <QSignalSpy>

#include "../src/parameterSetToGraphDeltaModel.h"

#include "attribute.h"
#include "attributeModel.h"
#include "graphModel.h"
#include "subGraphModel.h"
#include "parameter.h"
#include "product.h"
#include "seamSeries.h"
#include "seam.h"

using precitec::gui::ParameterSetToGraphDeltaModel;
using precitec::storage::AttributeModel;
using precitec::storage::GraphModel;
using precitec::storage::SubGraphModel;
using precitec::storage::Parameter;
using precitec::storage::Product;

class ParameterSetToGraphDeltaModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testSetAttributeModel();
    void testSetGraphModel();
    void testSetSubGraphModel();
    void testSeam();
};

void ParameterSetToGraphDeltaModelTest::testCtor()
{
    ParameterSetToGraphDeltaModel model;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.columnCount(), 0);
    QVERIFY(!model.seam());
    QVERIFY(!model.attributeModel());
    QVERIFY(!model.graphModel());
    QVERIFY(!model.subGraphModel());
    QCOMPARE(model.longestValueGraph(), QString{});
    QCOMPARE(model.longestValueSeam(), QString{});

    QCOMPARE(model.headerData(0, Qt::Horizontal, Qt::DisplayRole), QVariant{});
    QCOMPARE(model.headerData(0, Qt::Horizontal, Qt::UserRole), QVariant{});
    QCOMPARE(model.headerData(0, Qt::Horizontal, Qt::UserRole + 1), QVariant{});
    QCOMPARE(model.headerData(0, Qt::Horizontal, Qt::UserRole + 2), QVariant{});
    QCOMPARE(model.headerData(0, Qt::Vertical, Qt::DisplayRole), QVariant{});
    QCOMPARE(model.headerData(0, Qt::Vertical, Qt::UserRole), QVariant{});
    QCOMPARE(model.headerData(0, Qt::Vertical, Qt::UserRole + 1), QVariant{});
    QCOMPARE(model.headerData(0, Qt::Vertical, Qt::UserRole + 2), QVariant{});
}

void ParameterSetToGraphDeltaModelTest::testSetAttributeModel()
{
    ParameterSetToGraphDeltaModel model;
    QSignalSpy attributeModelChangedSpy{&model, &ParameterSetToGraphDeltaModel::attributeModelChanged};
    QVERIFY(attributeModelChangedSpy.isValid());

    auto *attributeModel = new AttributeModel{&model};
    model.setAttributeModel(attributeModel);
    QCOMPARE(model.attributeModel(), attributeModel);
    QCOMPARE(attributeModelChangedSpy.count(), 1);
    // setting same should not emit signal
    model.setAttributeModel(attributeModel);
    QCOMPARE(attributeModelChangedSpy.count(), 1);

    attributeModel->deleteLater();
    QVERIFY(attributeModelChangedSpy.wait());
    QVERIFY(!model.attributeModel());
}

void ParameterSetToGraphDeltaModelTest::testSetGraphModel()
{
    ParameterSetToGraphDeltaModel model;
    QSignalSpy graphModelChangedSpy{&model, &ParameterSetToGraphDeltaModel::graphModelChanged};
    QVERIFY(graphModelChangedSpy.isValid());

    auto *graphModel = new GraphModel{&model};
    model.setGraphModel(graphModel);
    QCOMPARE(model.graphModel(), graphModel);
    QCOMPARE(graphModelChangedSpy.count(), 1);

    // setting same should not emit signal
    model.setGraphModel(graphModel);
    QCOMPARE(graphModelChangedSpy.count(), 1);

    graphModel->deleteLater();
    QVERIFY(graphModelChangedSpy.wait());
    QVERIFY(!model.graphModel());
}

void ParameterSetToGraphDeltaModelTest::testSetSubGraphModel()
{
    ParameterSetToGraphDeltaModel model;
    QSignalSpy subGraphModelChangedSpy{&model, &ParameterSetToGraphDeltaModel::subGraphModelChanged};
    QVERIFY(subGraphModelChangedSpy.isValid());

    auto *subGraphModel = new SubGraphModel{&model};
    model.setSubGraphModel(subGraphModel);
    QCOMPARE(model.subGraphModel(), subGraphModel);
    QCOMPARE(subGraphModelChangedSpy.count(), 1);

    // setting same should not emit signal
    model.setSubGraphModel(subGraphModel);
    QCOMPARE(subGraphModelChangedSpy.count(), 1);

    subGraphModel->deleteLater();
    QVERIFY(subGraphModelChangedSpy.wait());
    QVERIFY(!model.subGraphModel());
}

void ParameterSetToGraphDeltaModelTest::testSeam()
{
    ParameterSetToGraphDeltaModel model;
    QSignalSpy modelResetSpy{&model, &ParameterSetToGraphDeltaModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    QSignalSpy seamChangedSpy{&model, &ParameterSetToGraphDeltaModel::seamChanged};
    QVERIFY(seamChangedSpy.isValid());
    QSignalSpy longestValueSeamChanged{&model, &ParameterSetToGraphDeltaModel::longestValueSeamChanged};
    QVERIFY(longestValueSeamChanged.isValid());
    QSignalSpy longestValueGraphChanged{&model, &ParameterSetToGraphDeltaModel::longestValueGraphChanged};
    QVERIFY(longestValueGraphChanged.isValid());

    auto *attributeModel = new AttributeModel{&model};
    QSignalSpy attributeModelResetSpy{attributeModel, &AttributeModel::modelReset};
    QVERIFY(attributeModelResetSpy.isValid());

    attributeModel->load(QFINDTESTDATA("../../wm_inst/system_graphs/attributes.json"));
    QVERIFY(attributeModelResetSpy.wait());

    model.setAttributeModel(attributeModel);

    auto *subGraphModel = new SubGraphModel{&model};
    model.setSubGraphModel(subGraphModel);

    auto *graphModel = new GraphModel{&model};
    QSignalSpy graphModelLoadingChanged{graphModel, &GraphModel::loadingChanged};
    QVERIFY(graphModelLoadingChanged.isValid());
    graphModel->loadGraphs(QFINDTESTDATA("testdata/delta/graphs"));
    QTRY_COMPARE(graphModelLoadingChanged.size(), 2);
    QCOMPARE(graphModel->rowCount(), 1);

    model.setGraphModel(graphModel);
    QCOMPARE(modelResetSpy.count(), 0);

    auto *p = Product::fromJson(QFINDTESTDATA("testdata/delta/products/f34fdc27-2881-4bd0-b076-f08a4835dfce.json"), &model);
    QVERIFY(p);

    auto seam = p->seamSeries().front()->seams().back();
    model.setSeam(seam);
    QCOMPARE(model.seam(), seam);
    QCOMPARE(seamChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);
    QCOMPARE(longestValueSeamChanged.count(), 1);
    QCOMPARE(longestValueGraphChanged.count(), 1);
    // setting same should not change
    model.setSeam(seam);
    QCOMPARE(seamChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    QCOMPARE(model.columnCount(), 2);
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.columnCount(model.index(0, 0)), 0);
    QCOMPARE(model.rowCount(model.index(0, 0)), 0);

    QCOMPARE(model.data(model.index(0, 0)).value<Parameter*>()->value(), 500);
    QCOMPARE(model.data(model.index(0, 1)).value<Parameter*>()->value(), 0);
    QCOMPARE(model.data(model.index(1, 0)).value<Parameter*>()->value(), 600);
    QCOMPARE(model.data(model.index(1, 1)).value<Parameter*>()->value(), 0);
    QCOMPARE(model.data(model.index(2, 0)).value<Parameter*>()->value(), 0);
    QCOMPARE(model.data(model.index(2, 1)).value<Parameter*>()->value(), 255);

    QCOMPARE(model.longestValueSeam(), QStringLiteral("500"));
    QCOMPARE(model.longestValueGraph(), QStringLiteral("255"));

    // header data
    QCOMPARE(model.headerData(-1, Qt::Vertical), QVariant{});
    QCOMPARE(model.headerData(3, Qt::Vertical), QVariant{});
    QCOMPARE(model.headerData(0, Qt::Horizontal, Qt::DisplayRole), QVariant{});
    QCOMPARE(model.headerData(0, Qt::Horizontal, Qt::UserRole), QVariant{});
    QCOMPARE(model.headerData(0, Qt::Horizontal, Qt::UserRole + 1), QVariant{});
    QCOMPARE(model.headerData(0, Qt::Horizontal, Qt::UserRole + 2), QVariant{});
    QCOMPARE(model.headerData(0, Qt::Vertical), QStringLiteral("X"));
    QCOMPARE(model.headerData(1, Qt::Vertical), QStringLiteral("ParameterFilter"));
    QCOMPARE(model.headerData(2, Qt::Vertical), QStringLiteral("PosDisplay"));

    QCOMPARE(model.headerData(0, Qt::Vertical, Qt::UserRole).toString(), QStringLiteral("Not grouped"));
    QCOMPARE(model.headerData(1, Qt::Vertical, Qt::UserRole).toString(), QStringLiteral("Not grouped"));
    QCOMPARE(model.headerData(2, Qt::Vertical, Qt::UserRole).toString(), QStringLiteral("Not grouped"));

    QCOMPARE(model.headerData(0, Qt::Vertical, Qt::UserRole + 1).toString(), QStringLiteral("1a7f9c78-ff0a-4574-ad2d-a587f0561e13"));
    QCOMPARE(model.headerData(1, Qt::Vertical, Qt::UserRole + 1).toString(), QStringLiteral("1a7f9c78-ff0a-4574-ad2d-a587f0561e13"));
    QCOMPARE(model.headerData(2, Qt::Vertical, Qt::UserRole + 1).toString(), QStringLiteral("c5a7c583-06af-43dd-8045-c404f8fe89c7"));

    auto *attribute = model.headerData(0, Qt::Vertical, Qt::UserRole + 2).value<precitec::storage::Attribute*>();
    QVERIFY(attribute);
    QCOMPARE(attribute->uuid(), QUuid{QByteArrayLiteral("95B2BA6A-B1EA-40CF-A91F-76EB2F0B7064")});
    attribute = model.headerData(1, Qt::Vertical, Qt::UserRole + 2).value<precitec::storage::Attribute*>();
    QVERIFY(attribute);
    QCOMPARE(attribute->uuid(), QUuid{QByteArrayLiteral("95B2BA6A-B1EA-40CF-A91F-76EB2F0B7064")});
    attribute = model.headerData(2, Qt::Vertical, Qt::UserRole + 2).value<precitec::storage::Attribute*>();
    QVERIFY(attribute);
    QCOMPARE(attribute->uuid(), QUuid{QByteArrayLiteral("AD4AF47E-598C-4044-B4EA-46B3F4548DEE")});

    seam->deleteLater();
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 2);
    QCOMPARE(seamChangedSpy.count(), 2);
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.columnCount(), 0);
    QVERIFY(!model.seam());
}

QTEST_GUILESS_MAIN(ParameterSetToGraphDeltaModelTest)
#include "parameterSetToGraphDeltaModelTest.moc"
