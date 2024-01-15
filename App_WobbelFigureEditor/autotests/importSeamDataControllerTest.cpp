#include <QTest>
#include <QSignalSpy>

#include "../src/importSeamDataController.h"
#include "../src/figureEditorSettings.h"

#include "../../Mod_Storage/src/product.h"
#include "../../Mod_Storage/src/seam.h"
#include "../../Mod_Storage/src/seamSeries.h"

using precitec::scanmaster::components::wobbleFigureEditor::ImportSeamDataController;
using precitec::scanmaster::components::wobbleFigureEditor::FigureEditorSettings;

using precitec::storage::ProductModel;
using precitec::storage::Product;
using precitec::storage::Seam;
using precitec::storage::SeamSeries;

class ImportSeamDataControllerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testVelocity();
    void testReset();
    void testUMPerSecondToMMPerSecond();
    void testImportData();

private:
    QTemporaryDir m_dir;
};

void ImportSeamDataControllerTest::initTestCase()
{
    QVERIFY(m_dir.isValid());
    QVERIFY(QDir{}.mkpath(m_dir.filePath(QStringLiteral("config"))));
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());
}

void ImportSeamDataControllerTest::testCtor()
{
    ImportSeamDataController importSeamDataController;

    QCOMPARE(importSeamDataController.velocity(), 0.0);
}

void ImportSeamDataControllerTest::testVelocity()
{
    ImportSeamDataController importSeamDataController;

    Product product(QUuid::createUuid());
    SeamSeries seamSeries(QUuid::createUuid(), &product);
    Seam seam1(QUuid::createUuid(), &seamSeries);
    Seam seam2(QUuid::createUuid(), &seamSeries);
    seam1.setVelocity(100000);
    seam2.setVelocity(123000);

    importSeamDataController.importVelocityFromSeam();
    QCOMPARE(importSeamDataController.velocity(), 0.0);

    importSeamDataController.setSeam(&seam1);
    QCOMPARE(importSeamDataController.seam(), &seam1);

    importSeamDataController.importVelocityFromSeam();
    QCOMPARE(importSeamDataController.velocity(), 100.0);

    importSeamDataController.setSeam(&seam2);
    QCOMPARE(importSeamDataController.seam(), &seam2);

    importSeamDataController.importVelocityFromSeam();
    QCOMPARE(importSeamDataController.velocity(), 123.0);
}

void ImportSeamDataControllerTest::testReset()
{
    ImportSeamDataController importSeamDataController;

    importSeamDataController.m_velocity = 1000.0;
    QCOMPARE(importSeamDataController.velocity(), 1000.0);

    importSeamDataController.reset();
    QCOMPARE(importSeamDataController.velocity(), 0.0);

    importSeamDataController.m_velocity = 500.0;
    QCOMPARE(importSeamDataController.velocity(), 500.0);

    importSeamDataController.productModelIndexChanged();
    QCOMPARE(importSeamDataController.velocity(), 0.0);
}

void ImportSeamDataControllerTest::testUMPerSecondToMMPerSecond()
{
    ImportSeamDataController importSeamDataController;

    double uMPerSecond{500000};
    QCOMPARE(importSeamDataController.umPerSecondToMMPerSecond(uMPerSecond), 500);
}

void ImportSeamDataControllerTest::testImportData()
{
    ImportSeamDataController importSeamDataController;

    QCOMPARE(FigureEditorSettings::instance()->scannerSpeed(), 1.0);

    importSeamDataController.importData();

    QCOMPARE(FigureEditorSettings::instance()->scannerSpeed(), 0.0);

    Product product(QUuid::createUuid());
    SeamSeries seamSeries(QUuid::createUuid(), &product);
    Seam seam1(QUuid::createUuid(), &seamSeries);
    seam1.setVelocity(123000);

    importSeamDataController.setSeam(&seam1);
    QCOMPARE(importSeamDataController.seam(), &seam1);

    importSeamDataController.importVelocityFromSeam();
    QCOMPARE(importSeamDataController.velocity(), 123.0);

    importSeamDataController.importData();

    QCOMPARE(FigureEditorSettings::instance()->scannerSpeed(), 123.0);
}


QTEST_GUILESS_MAIN(ImportSeamDataControllerTest)
#include "importSeamDataControllerTest.moc"
