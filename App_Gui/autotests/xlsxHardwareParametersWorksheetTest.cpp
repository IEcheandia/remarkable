#include <QTest>
#include <QObject>
#include <QTemporaryDir>

#include <memory>
#include "product.h"
#include "parameterSet.h"
#include "seamSeries.h"
#include "seam.h"

#include "../src/xlsxWorkbook.h"
#include "../src/xlsxHardwareParametersWorksheet.h"

#include <xlsxwriter/workbook.h>
#include <xlsxwriter/worksheet.h>

using precitec::gui::HardwareParametersWorksheet;
using precitec::gui::Workbook;
using precitec::storage::Parameter;
using precitec::storage::ParameterSet;
using precitec::storage::Product;
using precitec::storage::Seam;
using precitec::storage::SeamSeries;

class XlsxHardwareParametersWorksheetTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void xlsxFileWithoutSheet();
    void xlsxFileWithSheet();
};

void XlsxHardwareParametersWorksheetTest::xlsxFileWithoutSheet()
{
    Product *product = nullptr;
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());
    const auto fileName = QStringLiteral("product_parameters_text.xlsx");
    auto workbook = new Workbook(this);
    workbook->prepareWorkbook(temporaryDir.path() + QStringLiteral("/"), fileName, "test");
    auto hardwareParametersWorksheet = std::make_unique<HardwareParametersWorksheet>();
    hardwareParametersWorksheet->setProduct(product);
    workbook->addWorksheet(std::move(hardwareParametersWorksheet), QString());
    workbook->save();
    QVERIFY(QFile::remove(temporaryDir.filePath(fileName)));
}

void XlsxHardwareParametersWorksheetTest::xlsxFileWithSheet()
{
    const auto productFileName = QFINDTESTDATA("testdata/product_data/product_with_parameters.json");
    QFile file(productFileName);
    QVERIFY(file.open(QIODevice::ReadOnly));
    auto data = file.readAll();
    QVERIFY(!data.isEmpty());
    std::unique_ptr<Product> product{Product::fromJson(productFileName)};

    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const auto fileName = QStringLiteral("product_parameters_text.xlsx");
    auto workbook = new Workbook(this);
    workbook->prepareWorkbook(temporaryDir.path() + QStringLiteral("/"), fileName, "test");
    auto hardwareParametersWorksheet = std::make_unique<HardwareParametersWorksheet>();
    hardwareParametersWorksheet->setProduct(product.get());

    workbook->addWorksheet(std::move(hardwareParametersWorksheet), QStringLiteral("huha"));
    const auto &worksheets = workbook->worksheets();

    QVERIFY(worksheets.size() == 1);
    QCOMPARE(worksheets.front()->readFromCellValue(1, 0).toString(), QStringLiteral("Seam_Series_(number)"));
    QCOMPARE(worksheets.front()->readFromCellValue(1, 3).toByteArray(), QStringLiteral("_(1)"));
    QCOMPARE(worksheets.front()->readFromCellValue(6, 6).toString(), QStringLiteral("25"));

    workbook->save();
    QVERIFY(QFile::remove(temporaryDir.filePath(fileName)));
}

QTEST_GUILESS_MAIN(XlsxHardwareParametersWorksheetTest)
#include "xlsxHardwareParametersWorksheetTest.moc"
