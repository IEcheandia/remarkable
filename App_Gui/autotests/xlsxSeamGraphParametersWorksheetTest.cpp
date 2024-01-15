#include <QTest>
#include <QObject>
#include <QTemporaryDir>
#include <QSignalSpy>
#include <QSettings>

#include <memory>

#include "graphModel.h"
#include "subGraphModel.h"
#include "attributeModel.h"
#include "parameterSet.h"
#include "seamSeries.h"
#include "product.h"
#include "seam.h"

#include "../src/xlsxWorkbook.h"
#include "../src/xlsxProductSeamGraphWorksheets.h"
#include "../src/xlsxSeamGraphParametersWorksheet.h"
#include "../src/permissions.h"

#include <precitec/userManagement.h>
#include <precitec/permission.h>
#include <precitec/user.h>

#include <xlsxwriter/workbook.h>
#include <xlsxwriter/worksheet.h>

using precitec::gui::ProductSeamGraphWorksheets;
using precitec::gui::SeamGraphParametersWorksheet;
using precitec::gui::Workbook;
using precitec::storage::AttributeModel;
using precitec::storage::GraphModel;
using precitec::storage::Parameter;
using precitec::storage::ParameterSet;
using precitec::storage::Product;
using precitec::storage::Seam;
using precitec::storage::SeamSeries;
using precitec::storage::SubGraphModel;

using precitec::gui::components::user::UserManagement;
using precitec::gui::components::user::Permission;

class XlsxSeamGraphParameterWorksheetTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void xlsxAdminRightsAndGraphModel();
    void xlsxOperatorRightsAndGraphModel();
};

void XlsxSeamGraphParameterWorksheetTest::initTestCase()
{
    UserManagement::instance()->installPermissions(
        {
            {int(precitec::gui::Permission::ViewFilterParameterOperator), QLatin1String("")},
            {int(precitec::gui::Permission::ViewFilterParameterGroupLeader), QLatin1String("")},
            {int(precitec::gui::Permission::ViewFilterParameterSuperUser), QLatin1String("")},
            {int(precitec::gui::Permission::ViewFilterParameterOperator), QLatin1String("")}
        }
    );
    QSettings roleSettings(QFINDTESTDATA("testdata/user_data/role.ini"), QSettings::IniFormat);
    UserManagement::instance()->loadRoles(&roleSettings);
    QSettings userSettings(QFINDTESTDATA("testdata/user_data/user.ini"), QSettings::IniFormat);
    UserManagement::instance()->loadUsers(&userSettings);
}

void XlsxSeamGraphParameterWorksheetTest::xlsxAdminRightsAndGraphModel()
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

    // set admin permissions rights (all parameters)
    QVERIFY(UserManagement::instance()->authenticate(1, QStringLiteral("1234")));

    // prepare workbook
    const auto fileName = QStringLiteral("product_test_3_1.xlsx");
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

    // check content
    QCOMPARE(worksheets.size(), 4);
    QCOMPARE(worksheets.front()->name(), QStringLiteral("seam_1_of_seam_series_1"));
    QCOMPARE(worksheets.front()->readFromCellValue(1, 0).toString(), QStringLiteral("Filter_group"));
    QCOMPARE(worksheets.front()->readFromCellValue(1, 4).toString(), QStringLiteral("Parameter"));

    QCOMPARE(worksheets.front()->readFromCellValue(2, 1).toString(), QStringLiteral("ImageSource"));

    QCOMPARE(worksheets.front()->readFromCellValue(3, 0).toString(), QStringLiteral("Not grouped"));
    QCOMPARE(worksheets.front()->readFromCellValue(5, 4).toString(), QStringLiteral("Verbosity"));
    QCOMPARE(worksheets.front()->readFromCellValue(6, 5).toString(), QStringLiteral("10"));
    QCOMPARE(worksheets.front()->readFromCellValue(6, 6).toString(), QStringLiteral("1"));

    QCOMPARE(worksheets.front()->readFromCellValue(7, 4).toString(), QStringLiteral("Blue"));
    QCOMPARE(worksheets.front()->readFromCellValue(8, 5).toString(), QStringLiteral("255"));
    QCOMPARE(worksheets.front()->readFromCellValue(9, 3).toString(), QStringLiteral("b149cc19-66c4-4c5a-93c8-0f3a6876ffd2"));

    // save check
    workbook->save();
    QVERIFY(QFile::remove(temporaryDir.filePath(fileName)));
}

void XlsxSeamGraphParameterWorksheetTest::xlsxOperatorRightsAndGraphModel()
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

    // set permissions rights
    QVERIFY(UserManagement::instance()->authenticate(2, QStringLiteral("1234")));
    auto currentUser = UserManagement::instance()->currentUser();
    QCOMPARE(currentUser->name(), QString("operator"));

    // prepare workbook
    const auto fileName = QStringLiteral("product_test_3_2.xlsx");
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

    // check content
    QCOMPARE(worksheets.size(), 4);
    QCOMPARE(worksheets.front()->name(), QStringLiteral("seam_1_of_seam_series_1"));
    QCOMPARE(worksheets.front()->readFromCellValue(1, 0).toString(), QStringLiteral("Filter_group"));
    QCOMPARE(worksheets.front()->readFromCellValue(1, 4).toString(), QStringLiteral("Parameter"));
    QCOMPARE(worksheets.front()->readFromCellValue(2, 1).toString(), QLatin1String(""));
    QCOMPARE(worksheets.front()->readFromCellValue(1, 6).toString(), QStringLiteral("User_level"));

    //save check
    workbook->save();
    QVERIFY(QFile::remove(temporaryDir.filePath(fileName)));
}

QTEST_GUILESS_MAIN(XlsxSeamGraphParameterWorksheetTest)
#include "xlsxSeamGraphParametersWorksheetTest.moc"