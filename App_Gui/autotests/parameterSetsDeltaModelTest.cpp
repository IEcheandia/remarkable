#include <QTest>
#include <QSignalSpy>

#include "../src/parameterSetsDeltaModel.h"

#include "attribute.h"
#include "attributeModel.h"
#include "graphModel.h"
#include "subGraphModel.h"
#include "parameter.h"
#include "product.h"
#include "seamSeries.h"
#include "seam.h"
#include "parameterSet.h"

#include "guiConfiguration.h"

using precitec::gui::ParameterSetsDeltaModel;
using precitec::gui::GuiConfiguration;
using precitec::storage::AttributeModel;
using precitec::storage::GraphModel;
using precitec::storage::SubGraphModel;
using precitec::storage::Parameter;
using precitec::storage::Product;

class ParameterSetsDeltaModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testSetAttributeModel();
    void testSetGraphModel();
    void testSetSubGraphModel();
    void testSeam();
};

void ParameterSetsDeltaModelTest::testCtor()
{
    ParameterSetsDeltaModel model;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.columnCount(), 0);
    QVERIFY(!model.seam());
    QVERIFY(!model.attributeModel());
    QVERIFY(!model.graphModel());
    QVERIFY(!model.subGraphModel());

    QCOMPARE(model.headerData(0, Qt::Horizontal, Qt::DisplayRole), QVariant{});
    QCOMPARE(model.headerData(0, Qt::Horizontal, Qt::UserRole), QVariant{});
    QCOMPARE(model.headerData(0, Qt::Horizontal, Qt::UserRole + 1), QVariant{});
    QCOMPARE(model.headerData(0, Qt::Horizontal, Qt::UserRole + 2), QVariant{});
    QCOMPARE(model.headerData(0, Qt::Vertical, Qt::DisplayRole), QVariant{});
    QCOMPARE(model.headerData(0, Qt::Vertical, Qt::UserRole), QVariant{});
    QCOMPARE(model.headerData(0, Qt::Vertical, Qt::UserRole + 1), QVariant{});
    QCOMPARE(model.headerData(0, Qt::Vertical, Qt::UserRole + 2), QVariant{});
}

void ParameterSetsDeltaModelTest::testSetAttributeModel()
{
    ParameterSetsDeltaModel model;
    QSignalSpy attributeModelChangedSpy{&model, &ParameterSetsDeltaModel::attributeModelChanged};
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

void ParameterSetsDeltaModelTest::testSetGraphModel()
{
    ParameterSetsDeltaModel model;
    QSignalSpy graphModelChangedSpy{&model, &ParameterSetsDeltaModel::graphModelChanged};
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

void ParameterSetsDeltaModelTest::testSetSubGraphModel()
{
    ParameterSetsDeltaModel model;
    QSignalSpy subGraphModelChangedSpy{&model, &ParameterSetsDeltaModel::subGraphModelChanged};
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

void ParameterSetsDeltaModelTest::testSeam()
{
    ParameterSetsDeltaModel model;
    QSignalSpy modelResetSpy{&model, &ParameterSetsDeltaModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    QSignalSpy seamChangedSpy{&model, &ParameterSetsDeltaModel::seamChanged};
    QVERIFY(seamChangedSpy.isValid());

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
    // setting same should not change
    model.setSeam(seam);
    QCOMPARE(seamChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    QCOMPARE(model.columnCount(), 3);
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.columnCount(model.index(0, 0)), 0);
    QCOMPARE(model.rowCount(model.index(0, 0)), 0);

    QCOMPARE(model.data(model.index(0, 0)).value<Parameter*>()->value(), 500);
    QCOMPARE(model.data(model.index(0, 1)).value<Parameter*>()->value(), 100);
    QCOMPARE(model.data(model.index(0, 2)).value<Parameter*>()->value(), 200);
    QCOMPARE(model.data(model.index(1, 0)).value<Parameter*>()->value(), 600);
    QCOMPARE(model.data(model.index(1, 1)).value<Parameter*>()->value(), 10);
    QCOMPARE(model.data(model.index(1, 2)).value<Parameter*>()->value(), 50);
    QCOMPARE(model.data(model.index(2, 0)).value<Parameter*>()->value(), 0);
    QCOMPARE(model.data(model.index(2, 1)).value<Parameter*>()->value(), 255);
    QCOMPARE(model.data(model.index(2, 2)).value<Parameter*>()->value(), 255);

    // header data
    QCOMPARE(model.headerData(-1, Qt::Vertical), QVariant{});
    QCOMPARE(model.headerData(3, Qt::Vertical), QVariant{});
    QCOMPARE(model.headerData(0, Qt::Vertical), QStringLiteral("X"));
    QCOMPARE(model.headerData(1, Qt::Vertical), QStringLiteral("ParameterFilter"));
    QCOMPARE(model.headerData(2, Qt::Vertical), QStringLiteral("PosDisplay"));

    QCOMPARE(model.headerData(0, Qt::Vertical, Qt::UserRole).toString(), QStringLiteral("Not grouped"));
    QCOMPARE(model.headerData(1, Qt::Vertical, Qt::UserRole).toString(), QStringLiteral("Not grouped"));
    QCOMPARE(model.headerData(2, Qt::Vertical, Qt::UserRole).toString(), QStringLiteral("Not grouped"));

    QCOMPARE(model.headerData(0, Qt::Vertical, Qt::UserRole + 1).toString(), QStringLiteral("1a7f9c78-ff0a-4574-ad2d-a587f0561e13"));
    QCOMPARE(model.headerData(1, Qt::Vertical, Qt::UserRole + 1).toString(), QStringLiteral("1a7f9c78-ff0a-4574-ad2d-a587f0561e13"));
    QCOMPARE(model.headerData(2, Qt::Vertical, Qt::UserRole + 1).toString(), QStringLiteral("c5a7c583-06af-43dd-8045-c404f8fe89c7"));

    // Qt::UserRole + 3 should always be false as UserManagement is not mocked
    QVERIFY(!model.headerData(0, Qt::Vertical, Qt::UserRole + 3).toBool());
    QVERIFY(!model.headerData(1, Qt::Vertical, Qt::UserRole + 3).toBool());
    QVERIFY(!model.headerData(2, Qt::Vertical, Qt::UserRole + 3).toBool());
    // default values from Graph
    QCOMPARE(model.headerData(0, Qt::Vertical, Qt::UserRole + 4).toInt(), 0);
    QCOMPARE(model.headerData(1, Qt::Vertical, Qt::UserRole + 4).toInt(), 0);
    QCOMPARE(model.headerData(2, Qt::Vertical, Qt::UserRole + 4).toInt(), 255);

    // horizontal
    GuiConfiguration::instance()->setSeamSeriesOnProductStructure(true);
    QCOMPARE(model.headerData(0, Qt::Horizontal).toString(), QStringLiteral("1\n3"));
    QCOMPARE(model.headerData(1, Qt::Horizontal).toString(), QStringLiteral("1\n1"));
    QCOMPARE(model.headerData(2, Qt::Horizontal).toString(), QStringLiteral("1\n2"));
    GuiConfiguration::instance()->setSeamSeriesOnProductStructure(false);
    QCOMPARE(model.headerData(0, Qt::Horizontal).toString(), QStringLiteral("3"));
    QCOMPARE(model.headerData(1, Qt::Horizontal).toString(), QStringLiteral("1"));
    QCOMPARE(model.headerData(2, Qt::Horizontal).toString(), QStringLiteral("2"));

    QCOMPARE(model.headerData(0, Qt::Horizontal, Qt::UserRole).toString(), QStringLiteral("500"));
    QCOMPARE(model.headerData(1, Qt::Horizontal, Qt::UserRole).toString(), QStringLiteral("100"));
    QCOMPARE(model.headerData(2, Qt::Horizontal, Qt::UserRole).toString(), QStringLiteral("200"));

    QCOMPARE(model.headerData(0, Qt::Horizontal, Qt::UserRole + 1).value<precitec::storage::Seam*>(), seam);
    QCOMPARE(model.headerData(1, Qt::Horizontal, Qt::UserRole + 1).value<precitec::storage::Seam*>(), p->seamSeries().at(0)->seams().at(0));
    QCOMPARE(model.headerData(2, Qt::Horizontal, Qt::UserRole + 1).value<precitec::storage::Seam*>(), p->seamSeries().at(0)->seams().at(1));

    auto *attribute = model.headerData(0, Qt::Vertical, Qt::UserRole + 2).value<precitec::storage::Attribute*>();
    QVERIFY(attribute);
    QCOMPARE(attribute->uuid(), QUuid{QByteArrayLiteral("95B2BA6A-B1EA-40CF-A91F-76EB2F0B7064")});
    attribute = model.headerData(1, Qt::Vertical, Qt::UserRole + 2).value<precitec::storage::Attribute*>();
    QVERIFY(attribute);
    QCOMPARE(attribute->uuid(), QUuid{QByteArrayLiteral("95B2BA6A-B1EA-40CF-A91F-76EB2F0B7064")});
    attribute = model.headerData(2, Qt::Vertical, Qt::UserRole + 2).value<precitec::storage::Attribute*>();
    QVERIFY(attribute);
    QCOMPARE(attribute->uuid(), QUuid{QByteArrayLiteral("AD4AF47E-598C-4044-B4EA-46B3F4548DEE")});

    // test data change
    QSignalSpy dataChangedSpy{&model, &QAbstractItemModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());
    model.updateFilterParameter(model.index(2, 0), 200);
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.at(0).at(0).toModelIndex(), model.index(2, 0));
    QCOMPARE(dataChangedSpy.at(0).at(1).toModelIndex(), model.index(2, 0));
    QCOMPARE(dataChangedSpy.at(0).at(2).value<QVector<int>>(), QVector<int>({Qt::DisplayRole}));
    QCOMPARE(model.data(model.index(2, 0)).value<Parameter*>()->value().toInt(), 200);
    auto *ps = p->filterParameterSet(seam->graphParamSet());
    QVERIFY(ps);
    auto *parameter = ps->findById(model.data(model.index(2, 0)).value<Parameter*>()->uuid());
    QVERIFY(parameter);
    QCOMPARE(parameter->value().toInt(), 200);

    seam->deleteLater();
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 2);
    QCOMPARE(seamChangedSpy.count(), 2);
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.columnCount(), 0);
    QVERIFY(!model.seam());
}

QTEST_GUILESS_MAIN(ParameterSetsDeltaModelTest)
#include "parameterSetsDeltaModelTest.moc"
