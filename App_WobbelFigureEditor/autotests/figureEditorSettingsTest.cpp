#include <QTest>
#include <QSignalSpy>

#include "../src/figureEditorSettings.h"

using precitec::scanmaster::components::wobbleFigureEditor::FigureEditorSettings;
using precitec::scantracker::components::wobbleFigureEditor::FileType;

class FigureEditorSettingsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testDualChannelLaser();
    void testDigitalLaserPower();
    void testScanmasterMode();
    void testScannerSpeed();
    void testLaserMaxPower();
    void testColorFromValue();
    void testScale();
    void testFileType();
    void testIncreaseScaleByScaleFactor();
    void testDecreaseScaleByScaleFactor();

private:
    QTemporaryDir m_dir;
};

void FigureEditorSettingsTest::initTestCase()
{
    QVERIFY(m_dir.isValid());
    QVERIFY(QDir{}.mkpath(m_dir.filePath(QStringLiteral("config"))));
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());
}

void FigureEditorSettingsTest::testCtor()
{
    auto settings = FigureEditorSettings::instance();
    QCOMPARE(settings->dualChannelLaser(), false);
    QCOMPARE(settings->digitalLaserPower(), false);
    QCOMPARE(settings->scannerSpeed(), 1.0);
    QCOMPARE(settings->laserMaxPower(), 4000);
    QCOMPARE(settings->heatMap().empty(), false);
    QCOMPARE(settings->heatMap().size(), 5);
    QCOMPARE(settings->heatMap().at(0).threshold, 0.0);
    QCOMPARE(settings->heatMap().at(0).color, Qt::blue);
    QCOMPARE(settings->heatMap().at(1).threshold, 25.0);
    QCOMPARE(settings->heatMap().at(1).color, Qt::cyan);
    QCOMPARE(settings->heatMap().at(2).threshold, 50.0);
    QCOMPARE(settings->heatMap().at(2).color, Qt::darkGreen);
    QCOMPARE(settings->heatMap().at(3).threshold, 75.0);
    QCOMPARE(settings->heatMap().at(3).color, Qt::yellow);
    QCOMPARE(settings->heatMap().at(4).threshold, 100.0);
    QCOMPARE(settings->heatMap().at(4).color, Qt::red);
    QCOMPARE(settings->lensType(), precitec::interface::LensType::F_Theta_340);
    QCOMPARE(settings->scale(), 1000);
}

void FigureEditorSettingsTest::testDualChannelLaser()
{
    auto settings = FigureEditorSettings::instance();

    QVERIFY(!settings->dualChannelLaser());
}

void FigureEditorSettingsTest::testDigitalLaserPower()
{
    auto settings = FigureEditorSettings::instance();

    QVERIFY(!settings->digitalLaserPower());
}

void FigureEditorSettingsTest::testScanmasterMode()
{
    auto settings = FigureEditorSettings::instance();

    QVERIFY(settings->scanMasterMode());
}

void FigureEditorSettingsTest::testScannerSpeed()
{
    auto settings = FigureEditorSettings::instance();

    QSignalSpy scannerSpeedChanged{settings, &FigureEditorSettings::scannerSpeedChanged};
    QVERIFY(scannerSpeedChanged.isValid());
    QCOMPARE(scannerSpeedChanged.count(), 0);

    settings->setScannerSpeed(1000);
    QCOMPARE(settings->scannerSpeed(), 1000);
    QCOMPARE(scannerSpeedChanged.count(), 1);
}

void FigureEditorSettingsTest::testLaserMaxPower()
{
    auto settings = FigureEditorSettings::instance();

    QSignalSpy laserMaxPowerChanged{settings, &FigureEditorSettings::laserMaxPowerChanged};
    QVERIFY(laserMaxPowerChanged.isValid());
    QCOMPARE(laserMaxPowerChanged.count(), 0);

    settings->setLaserMaxPower(12000);
    QCOMPARE(settings->laserMaxPower(), 12000);
    QCOMPARE(laserMaxPowerChanged.count(), 1);
}

void FigureEditorSettingsTest::testColorFromValue()
{
    auto settings = FigureEditorSettings::instance();

    QCOMPARE(settings->colorFromValue(0.0), QColor(Qt::blue));
    QCOMPARE(settings->colorFromValue(12.5), QColor(0, 128, 255));
    QCOMPARE(settings->colorFromValue(25.0), QColor(Qt::cyan));
    QCOMPARE(settings->colorFromValue(37.5), QColor(0, 192, 128));
    QCOMPARE(settings->colorFromValue(50.0), QColor(Qt::darkGreen));
    QCOMPARE(settings->colorFromValue(62.5), QColor(128, 192, 0));
    QCOMPARE(settings->colorFromValue(75.0), QColor(Qt::yellow));
    QCOMPARE(settings->colorFromValue(87.5), QColor(255, 128, 0));
    QCOMPARE(settings->colorFromValue(100.0), QColor(Qt::red));
}

void FigureEditorSettingsTest::testScale()
{
    auto settings = FigureEditorSettings::instance();

    QSignalSpy scaleChanged{settings, &FigureEditorSettings::scaleChanged};
    QVERIFY(scaleChanged.isValid());
    QCOMPARE(scaleChanged.count(), 0);

    settings->setScale(500);
    QCOMPARE(settings->scale(), 500);
    QCOMPARE(scaleChanged.count(), 1);
    settings->setScale(1000);
    QCOMPARE(settings->scale(), 1000);
    QCOMPARE(scaleChanged.count(), 2);
}

void FigureEditorSettingsTest::testFileType()
{
    auto settings = FigureEditorSettings::instance();

    QSignalSpy fileTypeChanged{settings, &FigureEditorSettings::fileTypeChanged};
    QVERIFY(fileTypeChanged.isValid());
    QCOMPARE(fileTypeChanged.count(), 0);

    settings->setFileType(FileType::Seam);
    QCOMPARE(fileTypeChanged.count(), 0);
    settings->setFileType(FileType::Wobble);
    QCOMPARE(fileTypeChanged.count(), 1);
    QCOMPARE(settings->fileType(), FileType::Wobble);
    settings->setFileType(FileType::Overlay);
    QCOMPARE(fileTypeChanged.count(), 2);
    QCOMPARE(settings->fileType(), FileType::Overlay);
    settings->setFileType(FileType::Seam);
    QCOMPARE(fileTypeChanged.count(), 3);
    QCOMPARE(settings->fileType(), FileType::Seam);
}

void FigureEditorSettingsTest::testIncreaseScaleByScaleFactor()
{
    auto settings = FigureEditorSettings::instance();

    const int defaultScale(1000);
    const int minScale(10);
    const int scaleFactor(10);

    QSignalSpy scaleChanged{settings, &FigureEditorSettings::scaleChanged};
    QVERIFY(scaleChanged.isValid());
    QCOMPARE(scaleChanged.count(), 0);

    QCOMPARE(settings->scale(), defaultScale);

    settings->increaseScaleByScaleFactor();
    QCOMPARE(scaleChanged.count(), 0);

    settings->setScale(minScale);
    QCOMPARE(scaleChanged.count(), 1);

    settings->increaseScaleByScaleFactor();
    QCOMPARE(scaleChanged.count(), 2);
    QCOMPARE(settings->scale(), minScale * scaleFactor);

    settings->increaseScaleByScaleFactor();
    QCOMPARE(scaleChanged.count(), 3);
    QCOMPARE(settings->scale(), minScale * scaleFactor * scaleFactor);

    settings->increaseScaleByScaleFactor();
    QCOMPARE(scaleChanged.count(), 3);
    QCOMPARE(settings->scale(), minScale * scaleFactor * scaleFactor );
}

void FigureEditorSettingsTest::testDecreaseScaleByScaleFactor()
{
    auto settings = FigureEditorSettings::instance();

    const int defaultScale(1000);
    const int scaleFactor(10);

    QSignalSpy scaleChanged{settings, &FigureEditorSettings::scaleChanged};
    QVERIFY(scaleChanged.isValid());
    QCOMPARE(scaleChanged.count(), 0);

    QCOMPARE(settings->scale(), defaultScale);

    settings->decreaseScaleByScaleFactor();
    QCOMPARE(scaleChanged.count(), 1);
    QCOMPARE(settings->scale(), defaultScale / scaleFactor);

    settings->decreaseScaleByScaleFactor();
    QCOMPARE(scaleChanged.count(), 2);
    QCOMPARE(settings->scale(), defaultScale / scaleFactor / scaleFactor);

    settings->decreaseScaleByScaleFactor();
    QCOMPARE(scaleChanged.count(), 2);
    QCOMPARE(settings->scale(), defaultScale / scaleFactor / scaleFactor);
}

QTEST_GUILESS_MAIN(FigureEditorSettingsTest)
#include "figureEditorSettingsTest.moc"
