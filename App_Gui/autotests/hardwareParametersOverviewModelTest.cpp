#include <QTest>
#include <QSignalSpy>

#include "../src/hardwareParametersOverviewModel.h"
#include "guiConfiguration.h"

#include "attribute.h"
#include "attributeModel.h"
#include "product.h"
#include "parameter.h"
#include "parameterSet.h"
#include "seamSeries.h"
#include "seam.h"

using precitec::gui::GuiConfiguration;
using precitec::gui::HardwareParametersOverviewModel;
using precitec::storage::Attribute;
using precitec::storage::AttributeModel;
using precitec::storage::Product;
using precitec::storage::Parameter;

class HardwareParametersOverviewModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testSetProduct();
    void testHardwareParameters();

private:
    QTemporaryDir m_dir;
};

void HardwareParametersOverviewModelTest::initTestCase()
{
    QVERIFY(m_dir.isValid());
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());
}

void HardwareParametersOverviewModelTest::testCtor()
{
    HardwareParametersOverviewModel model;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.columnCount(), 0);

    QVERIFY(!model.product());
    QVERIFY(!model.keyValueAttributeModel());
    QCOMPARE(model.columnWidth(0), 1);
    QCOMPARE(model.headerData(0, Qt::Horizontal), QVariant{});
    QCOMPARE(model.headerData(0, Qt::Vertical), QVariant{});
}

void HardwareParametersOverviewModelTest::testSetProduct()
{
    Product *p = new Product{QUuid::createUuid(), this};
    p->createFirstSeamSeries();
    auto *seam = p->createSeam();
    p->seamSeries().at(0)->createSeamLink(seam, QString::number(2));

    HardwareParametersOverviewModel model;
    QSignalSpy productChangedSpy{&model, &HardwareParametersOverviewModel::productChanged};
    QVERIFY(productChangedSpy.isValid());
    QSignalSpy modelResetSpy{&model, &HardwareParametersOverviewModel::modelReset};
    QVERIFY(modelResetSpy.isValid());

    GuiConfiguration::instance()->setSeamSeriesOnProductStructure(false);

    // set product
    model.setProduct(p);
    QCOMPARE(model.product(), p);
    QCOMPARE(productChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    // setting same product should not change
    model.setProduct(p);
    QCOMPARE(model.product(), p);
    QCOMPARE(productChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    // now we should have two columns, but no rows
    QCOMPARE(model.columnCount(), 2);
    QCOMPARE(model.rowCount(), 0);

    // we should have header data
    QCOMPARE(model.headerData(0, Qt::Horizontal).toString(), QStringLiteral("P"));
    QCOMPARE(model.headerData(1, Qt::Horizontal).toString(), QStringLiteral("1"));
    QCOMPARE(model.headerData(2, Qt::Horizontal), QVariant{});
    QCOMPARE(model.headerData(0, Qt::Horizontal, Qt::UserRole), QVariant{});
    QCOMPARE(model.headerData(0, Qt::Vertical), QVariant{});

    // data for index should be invalid
    QCOMPARE(model.data(model.index(0, 0)), QVariant{});

    // change to support for seam series
    GuiConfiguration::instance()->setSeamSeriesOnProductStructure(true);
    QCOMPARE(modelResetSpy.count(), 2);

    // now we should have 3 columns
    QCOMPARE(model.columnCount(), 3);
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.headerData(0, Qt::Horizontal).toString(), QStringLiteral("P"));
    QCOMPARE(model.headerData(1, Qt::Horizontal).toString(), QStringLiteral("1"));
    QCOMPARE(model.headerData(2, Qt::Horizontal).toString(), QStringLiteral("1\n1"));

    QVERIFY(model.columnWidth(0) > 1);
    QVERIFY(model.columnWidth(1) > 1);
    QVERIFY(model.columnWidth(2) > 1);
    QCOMPARE(model.columnWidth(3), 1);
    QVERIFY(model.columnWidth(0) != model.columnWidth(2));

    // and deleting the product should clear the model
    p->deleteLater();
    QVERIFY(productChangedSpy.wait());
    QCOMPARE(productChangedSpy.count(), 2);
    QCOMPARE(modelResetSpy.count(), 3);
    QVERIFY(!model.product());

    QCOMPARE(model.rowCount(), 0);
}

void HardwareParametersOverviewModelTest::testHardwareParameters()
{
    Product *p = new Product{QUuid::createUuid(), this};
    p->createFirstSeamSeries();
    p->createSeam();

    AttributeModel *attributeModel = new AttributeModel{this};
    QSignalSpy attributeModelResetSpy{attributeModel, &AttributeModel::modelReset};
    QVERIFY(attributeModelResetSpy.isValid());

    attributeModel->load(QFINDTESTDATA("../../wm_inst/system_graphs/keyValueAttributes.json"));
    QVERIFY(attributeModelResetSpy.wait());

    p->createHardwareParameters();
    p->hardwareParameters()->createParameter(QUuid::createUuid(), attributeModel->findAttributeByName(QStringLiteral("LEDSendData")), QUuid{}, true);
    auto *seam = p->seamSeries().front()->seams().front();
    QVERIFY(seam);
    seam->createHardwareParameters();
    seam->hardwareParameters()->createParameter(QUuid::createUuid(), attributeModel->findAttributeByName(QStringLiteral("LEDSendData")), QUuid{}, true);
    seam->hardwareParameters()->createParameter(QUuid::createUuid(), attributeModel->findAttributeByName(QStringLiteral("ExposureTime")), QUuid{}, 2.5);

    GuiConfiguration::instance()->setSeamSeriesOnProductStructure(true);
    HardwareParametersOverviewModel model;
    model.setProduct(p);
    QCOMPARE(model.columnCount(), 3);
    QCOMPARE(model.rowCount(), 2);

    QCOMPARE(model.columnCount(model.index(0, 0)), 0);
    QCOMPARE(model.rowCount(model.index(0, 0)), 0);

    // row 0 is ExposureTime
    QCOMPARE(model.data(model.index(0, 0)), QVariant{});
    QCOMPARE(model.data(model.index(0, 1)), QVariant{});
    QCOMPARE(QLocale{}.toDouble(model.data(model.index(0, 2)).toString()), 2.5);

    // row 1 is LEDSendData
    QCOMPARE(model.data(model.index(1, 0)).toBool(), true);
    QCOMPARE(model.data(model.index(1, 1)), QVariant{});
    QCOMPARE(model.data(model.index(1, 2)).toBool(), true);

    // column width should be determined by the values
    QVERIFY(model.columnWidth(0) > 1);
    QVERIFY(model.columnWidth(1) > 1);
    QVERIFY(model.columnWidth(2) > 1);
    QCOMPARE(model.columnWidth(3), 1);

    QCOMPARE(model.headerData(0, Qt::Horizontal).toString(), QStringLiteral("P"));
    QCOMPARE(model.headerData(1, Qt::Horizontal).toString(), QStringLiteral("1"));
    QCOMPARE(model.headerData(2, Qt::Horizontal).toString(), QStringLiteral("1\n1"));

    QCOMPARE(model.headerData(0, Qt::Vertical).toString(), QStringLiteral("ExposureTime"));
    QCOMPARE(model.headerData(1, Qt::Vertical).toString(), QStringLiteral("LEDSendData"));
    QCOMPARE(model.headerData(2, Qt::Vertical), QVariant{});

    QSignalSpy attributesModelChangedSpy{&model, &HardwareParametersOverviewModel::keyValueAttributeModelChanged};
    QVERIFY(attributesModelChangedSpy.isValid());
    model.setKeyValueAttributeModel(attributeModel);
    QCOMPARE(model.keyValueAttributeModel(), attributeModel);
    QCOMPARE(attributesModelChangedSpy.count(), 1);
    // setting same should not chage
    model.setKeyValueAttributeModel(attributeModel);
    QCOMPARE(attributesModelChangedSpy.count(), 1);

    // the header data for exposure time should have changed
    QCOMPARE(model.headerData(0, Qt::Vertical).toString(), QStringLiteral("Exposure time [ms]"));

    // test delete of attributeModel
    attributeModel->deleteLater();
    QVERIFY(attributesModelChangedSpy.wait());
    QVERIFY(!model.keyValueAttributeModel());
}

QTEST_MAIN(HardwareParametersOverviewModelTest)
#include "hardwareParametersOverviewModelTest.moc"
