#include <QTest>
#include <QSignalSpy>

#include "../src/gridController.h"
#include "../src/figureEditorSettings.h"

using precitec::scanmaster::components::wobbleFigureEditor::GridController;

class GridControllerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testShowCoordinateSystem();
    void testShowScanField();
    void testScale();

private:
    QTemporaryDir m_dir;
};

void GridControllerTest::initTestCase()
{
    QVERIFY(m_dir.isValid());
    QVERIFY(QDir{}.mkpath(m_dir.filePath(QStringLiteral("config"))));
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());
}

void GridControllerTest::testCtor()
{
    GridController gridController;

    QVERIFY(gridController.showCoordinateSystem());
    QVERIFY(!gridController.showScanField());
    QCOMPARE(gridController.origin(), QRect{});
    QCOMPARE(gridController.scanField(), QRectF{});
    QCOMPARE(gridController.scale(), 1000);
}

void GridControllerTest::testShowCoordinateSystem()
{
    GridController gridController;

    QSignalSpy showCoordinateSystemChanged{&gridController, &GridController::showCoordinateSystemChanged};
    QVERIFY(showCoordinateSystemChanged.isValid());
    QCOMPARE(showCoordinateSystemChanged.count(), 0);

    QCOMPARE(gridController.showCoordinateSystem(), true);
    gridController.setShowCoordinateSystem(true);
    QCOMPARE(gridController.showCoordinateSystem(), true);
    QCOMPARE(showCoordinateSystemChanged.count(), 0);

    gridController.setShowCoordinateSystem(false);
    QCOMPARE(gridController.showCoordinateSystem(), false);
    QCOMPARE(showCoordinateSystemChanged.count(), 1);
}

void GridControllerTest::testShowScanField()
{
    GridController gridController;

    QSignalSpy showScanFieldChanged{&gridController, &GridController::showScanFieldChanged};
    QVERIFY(showScanFieldChanged.isValid());
    QCOMPARE(showScanFieldChanged.count(), 0);

    QCOMPARE(gridController.showScanField(), false);
    gridController.setShowScanField(false);
    QCOMPARE(gridController.showScanField(), false);
    QCOMPARE(showScanFieldChanged.count(), 0);

    gridController.setShowScanField(true);
    QCOMPARE(gridController.showScanField(), true);
    QCOMPARE(showScanFieldChanged.count(), 1);
}

void GridControllerTest::testScale()
{
    GridController gridController;

    QSignalSpy scaleChanged{&gridController, &GridController::scaleChanged};
    QVERIFY(scaleChanged.isValid());
    QCOMPARE(scaleChanged.count(), 0);

    QCOMPARE(gridController.scale(), 1000);
    gridController.setScale(1000);
    QCOMPARE(gridController.scale(), 1000);
    QCOMPARE(scaleChanged.count(), 0);

    gridController.setScale(10);
    QCOMPARE(gridController.scale(), 10);
    QCOMPARE(scaleChanged.count(), 1);
}

QTEST_GUILESS_MAIN(GridControllerTest)
#include "gridControllerTest.moc"
