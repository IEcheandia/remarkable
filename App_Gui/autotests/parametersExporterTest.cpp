#include <QTest>
#include <QObject>
#include <QTemporaryDir>
#include <QSignalSpy>

#include <memory>

#include "graphModel.h"
#include "subGraphModel.h"
#include "attributeModel.h"
#include "product.h"

#include "../src/parametersExporter.h"

using precitec::gui::ParametersExporter;
using precitec::storage::AttributeModel;
using precitec::storage::GraphModel;
using precitec::storage::Parameter;
using precitec::storage::Product;
using precitec::storage::SubGraphModel;

class ParametersExporterTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void checkSignals();
};

void ParametersExporterTest::checkSignals()
{
    // for test files
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    // prepare product
    std::unique_ptr<Product> product{Product::fromJson(QFINDTESTDATA("testdata/product_data/graphparameters.json"))};
    QVERIFY(product);

    // prepare graph and null subgraph
    auto graphModel = std::make_unique<GraphModel>();
    auto subGraphModel = std::make_unique<SubGraphModel>();
    auto attributeModel = std::make_unique<AttributeModel>();
    ParametersExporter parametersExporter;

    //check "Changed" signals
    QSignalSpy productChangedSpy(&parametersExporter, &ParametersExporter::productChanged);
    parametersExporter.setProduct(product.get());
    QCOMPARE(productChangedSpy.count(), 1);
    QSignalSpy graphModelChangedSpy(&parametersExporter, &ParametersExporter::graphModelChanged);
    parametersExporter.setGraphModel(graphModel.get());
    QCOMPARE(graphModelChangedSpy.count(), 1);
    QSignalSpy subGraphModelChangedSpy(&parametersExporter, &ParametersExporter::subGraphModelChanged);
    parametersExporter.setSubGraphModel(subGraphModel.get());
    QCOMPARE(subGraphModelChangedSpy.count(), 1);
}

QTEST_GUILESS_MAIN(ParametersExporterTest)
#include "parametersExporterTest.moc"