#include <QTest>
#include <QObject>
#include <QTemporaryDir>
#include <QSignalSpy>

#include <memory>

#include "parameterSet.h"
#include "product.h"
#include "seamSeries.h"
#include "seam.h"

#include "../src/xlsxWorkbook.h"
#include "../src/xlsxProductSeamGraphWorksheets.h"
#include "../src/xlsxSeamGraphParametersWorksheet.h"

#include <xlsxwriter/workbook.h>
#include <xlsxwriter/worksheet.h>

#include "../src/graphModel.h"
#include "../src/subGraphModel.h"
#include "../src/attributeModel.h"

using precitec::gui::ProductSeamGraphWorksheets;
using precitec::gui::Workbook;
using precitec::storage::AttributeModel;
using precitec::storage::GraphModel;
using precitec::storage::Parameter;
using precitec::storage::ParameterSet;
using precitec::storage::Product;
using precitec::storage::Seam;
using precitec::storage::SeamSeries;
using precitec::storage::SubGraphModel;

class XlsxProductSeamGraphWorksheetsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void xlsxFileWithGraphModel();
};

void XlsxProductSeamGraphWorksheetsTest::xlsxFileWithGraphModel()
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
    QSignalSpy graphModelLoadingChanged{graphModel.get(), &precitec::storage::GraphModel::loadingChanged};
    QVERIFY(graphModelLoadingChanged.isValid());
    graphModel->loadGraphs(QFINDTESTDATA("testdata/graphs/graphs"));
    QTRY_COMPARE(graphModelLoadingChanged.size(), 2);
    QCOMPARE(graphModel->rowCount(), 1);

    // prepare workbook
    const auto fileName = QStringLiteral("product_test_1.xlsx");
    auto workbook = new Workbook(this);
    workbook->prepareWorkbook(temporaryDir.path() + QStringLiteral("/"), fileName, "test");

    // initialisation of graph parameters worksheet
    auto productSeamGraphWorksheets =
        std::make_unique<ProductSeamGraphWorksheets>();
    productSeamGraphWorksheets->setProduct(product.get());
    productSeamGraphWorksheets->setGraphModel(graphModel.get());
    productSeamGraphWorksheets->setSubGraphModel(subGraphModel.get());
    productSeamGraphWorksheets->setAttributeModel(attributeModel.get());
    productSeamGraphWorksheets->setTargetWorkbook(workbook);
    workbook->addWorksheet(std::move(productSeamGraphWorksheets), QString("Graph parameters overview"));

    // check number of worksheets after initialization
    const auto &worksheets = workbook->worksheets();
    QCOMPARE(worksheets.size(), 4);

    // check content
    QCOMPARE(worksheets.back()->readFromCellValue(0, 0).toString(), QString("Seam_name"));
    QCOMPARE(worksheets.back()->readFromCellValue(3, 3).toString(), QString("ParameterSetTestGraphrSet Test Graph"));
    QCOMPARE(worksheets.back()->readFromCellValue(4, 1).toString(), QString("Graph_parameters"));
    // check save
    workbook->save();
    QVERIFY(QFile::remove(temporaryDir.filePath(fileName)));
}

QTEST_GUILESS_MAIN(XlsxProductSeamGraphWorksheetsTest)
#include "xlsxProductSeamGraphWorksheetsTest.moc"