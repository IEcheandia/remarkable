#include <QTest>

#include "../include/viWeldHead/Scanlab/lensModel.h"

using precitec::hardware::LensModel;

/**
 *  This class is used to test the lens model class.
 **/
class LensModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testSetType();
    void testSetScannerController();
    void testCurrentLensInformation_data();
    void testCurrentLensInformation();
    void testLensData_data();
    void testLensData();
    void testCorrectionFile();
    void testCorrectionFilePrefix();
    void testCorrectionFileLens();
    void testCorrectionFileLensScanlab();
    void testCorrectionFileLensSmartMove();
    void testCorrectionFileMode();
    void testCorrectionFileSuffix();
};

void LensModelTest::testCtor()
{
    LensModel model;

    QCOMPARE(model.type(), LensType::F_Theta_340);
}

void LensModelTest::testSetType()
{
    LensModel model;

    model.setType(LensType::F_Theta_255);
    QCOMPARE(model.type(), LensType::F_Theta_255);
    model.setType(LensType::F_Theta_340);
    QCOMPARE(model.type(), LensType::F_Theta_340);
    model.setType(LensType::F_Theta_460);
    QCOMPARE(model.type(), LensType::F_Theta_460);
}

void LensModelTest::testSetScannerController()
{
    LensModel model;

    model.setScannerController(ScannerModel::ScanlabScanner);
    QCOMPARE(model.scannerController(), ScannerModel::ScanlabScanner);
    model.setScannerController(ScannerModel::SmartMoveScanner);
    QCOMPARE(model.scannerController(), ScannerModel::SmartMoveScanner);
    model.setScannerController(ScannerModel::ScanlabScanner);
}

Q_DECLARE_METATYPE(precitec::interface::LensType);
Q_DECLARE_METATYPE(precitec::interface::ScannerModel);
void LensModelTest::testCurrentLensInformation_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<ScannerModel>("scannerModel");
    QTest::addColumn<LensType>("lensType");
    QTest::addColumn<int>("focalLength");
    QTest::addColumn<std::pair<int, int>>("scanFieldSize");
    QTest::addColumn<int>("scanFieldSquare");
    QTest::addColumn<QString>("correctionFile");
    QTest::addColumn<QString>("correctionFilePilot");
    QTest::addColumn<QString>("correctionFileHeightMeasurement");

    QTest::newRow("F_Theta_340_Scanlab") << 1 << ScannerModel::ScanlabScanner << LensType::F_Theta_340 << 340 << std::make_pair(220, 104) << 52 <<
    QStringLiteral("IntelliScanIII30_F_Theta_340.ct5") <<
    QStringLiteral("IntelliScanIII30_F_Theta_340Pilot.ct5") <<
    QStringLiteral("IntelliScanIII30_F_Theta_340HeightMeasurement.ct5");
    QTest::newRow("F_Theta_460_Scanlab") << 2 << ScannerModel::ScanlabScanner << LensType::F_Theta_460 << 460 << std::make_pair(380, 290) << 145 <<
    QStringLiteral("IntelliScanIII30_F_Theta_460.ct5") <<
    QStringLiteral("IntelliScanIII30_F_Theta_460Pilot.ct5") <<
    QStringLiteral("IntelliScanIII30_F_Theta_460HeightMeasurement.ct5");
    QTest::newRow("F_Theta_255_Scanlab") << 3 << ScannerModel::ScanlabScanner << LensType::F_Theta_255 << 255 << std::make_pair(170, 100) << 50 <<
    QStringLiteral("IntelliScanIII30_F_Theta_255.ct5") <<
    QStringLiteral("IntelliScanIII30_F_Theta_255Pilot.ct5") <<
    QStringLiteral("IntelliScanIII30_F_Theta_255HeightMeasurement.ct5");
    QTest::newRow("F_Theta_340_SmartMove") << 4 << ScannerModel::SmartMoveScanner << LensType::F_Theta_340 << 340 << std::make_pair(220, 104) << 52 <<
    QStringLiteral("ME_F340.sbd") <<
    QStringLiteral("ME_F340.sbd") <<
    QStringLiteral("ME_F340.sbd");
    QTest::newRow("F_Theta_460_SmartMove") << 2 << ScannerModel::SmartMoveScanner << LensType::F_Theta_460 << 460 << std::make_pair(380, 290) << 145 <<
    QStringLiteral("ME_F460.sbd") <<
    QStringLiteral("ME_F460.sbd") <<
    QStringLiteral("ME_F460.sbd");
    QTest::newRow("F_Theta_255_SmartMove") << 3 << ScannerModel::SmartMoveScanner << LensType::F_Theta_255 << 255 << std::make_pair(170, 100) << 50 <<
    QStringLiteral("ME_F255.sbd") <<
    QStringLiteral("ME_F255.sbd") <<
    QStringLiteral("ME_F255.sbd");
}

void LensModelTest::testCurrentLensInformation()
{
    LensModel model;

    QFETCH(ScannerModel, scannerModel);
    QFETCH(LensType, lensType);

    model.setScannerController(scannerModel);
    QCOMPARE(model.scannerController(), scannerModel);
    model.setType(lensType);
    QCOMPARE(model.type(), lensType);

    auto lensData = model.lensData();
    QTEST(lensData.type, "lensType");
    QTEST(lensData.focalLength, "focalLength");
    QTEST(lensData.scanFieldSize, "scanFieldSize");
    QTEST(lensData.scanFieldSquare, "scanFieldSquare");
    QTEST(QString::fromStdString(lensData.calibrationFile), "correctionFile");
    QTEST(QString::fromStdString(lensData.calibrationFilePilot), "correctionFilePilot");
    QTEST(QString::fromStdString(lensData.calibrationFileZMeasurement), "correctionFileHeightMeasurement");
}

void LensModelTest::testLensData_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<ScannerModel>("scannerModel");
    QTest::addColumn<LensType>("lensType");
    QTest::addColumn<int>("focalLength");
    QTest::addColumn<std::pair<int, int>>("scanFieldSize");
    QTest::addColumn<int>("scanFieldSquare");
    QTest::addColumn<QString>("correctionFile");
    QTest::addColumn<QString>("correctionFilePilot");
    QTest::addColumn<QString>("correctionFileHeightMeasurement");

    QTest::newRow("F_Theta_340_Scanlab") << 1 << ScannerModel::ScanlabScanner << LensType::F_Theta_340 << 340 << std::make_pair(220, 104) << 52 <<
    QStringLiteral("IntelliScanIII30_F_Theta_340.ct5") <<
    QStringLiteral("IntelliScanIII30_F_Theta_340Pilot.ct5") <<
    QStringLiteral("IntelliScanIII30_F_Theta_340HeightMeasurement.ct5");
    QTest::newRow("F_Theta_460_Scanlab") << 2 << ScannerModel::ScanlabScanner << LensType::F_Theta_460 << 460 << std::make_pair(380, 290) << 145 <<
    QStringLiteral("IntelliScanIII30_F_Theta_460.ct5") <<
    QStringLiteral("IntelliScanIII30_F_Theta_460Pilot.ct5") <<
    QStringLiteral("IntelliScanIII30_F_Theta_460HeightMeasurement.ct5");
    QTest::newRow("F_Theta_255_Scanlab") << 3 << ScannerModel::ScanlabScanner << LensType::F_Theta_255 << 255 << std::make_pair(170, 100) << 50 <<
    QStringLiteral("IntelliScanIII30_F_Theta_255.ct5") <<
    QStringLiteral("IntelliScanIII30_F_Theta_255Pilot.ct5") <<
    QStringLiteral("IntelliScanIII30_F_Theta_255HeightMeasurement.ct5");
    QTest::newRow("F_Theta_340_SmartMove") << 4 << ScannerModel::SmartMoveScanner << LensType::F_Theta_340 << 340 << std::make_pair(220, 104) << 52 <<
    QStringLiteral("ME_F340.sbd") <<
    QStringLiteral("ME_F340.sbd") <<
    QStringLiteral("ME_F340.sbd");
    QTest::newRow("F_Theta_460_SmartMove") << 2 << ScannerModel::SmartMoveScanner << LensType::F_Theta_460 << 460 << std::make_pair(380, 290) << 145 <<
    QStringLiteral("ME_F460.sbd") <<
    QStringLiteral("ME_F460.sbd") <<
    QStringLiteral("ME_F460.sbd");
    QTest::newRow("F_Theta_255_SmartMove") << 3 << ScannerModel::SmartMoveScanner << LensType::F_Theta_255 << 255 << std::make_pair(170, 100) << 50 <<
    QStringLiteral("ME_F255.sbd") <<
    QStringLiteral("ME_F255.sbd") <<
    QStringLiteral("ME_F255.sbd");
}

void LensModelTest::testLensData()
{
    LensModel model;

    QFETCH(ScannerModel, scannerModel);
    QFETCH(LensType, lensType);

    model.setScannerController(scannerModel);
    QCOMPARE(model.scannerController(), scannerModel);
    model.setType(lensType);
    QCOMPARE(model.type(), lensType);

    auto lensData = model.lensData();
    QTEST(lensData.type, "lensType");
    QTEST(lensData.focalLength, "focalLength");
    QTEST(lensData.scanFieldSize, "scanFieldSize");
    QTEST(lensData.scanFieldSquare, "scanFieldSquare");
    QTEST(QString::fromStdString(lensData.calibrationFile), "correctionFile");
    QTEST(QString::fromStdString(lensData.calibrationFilePilot), "correctionFilePilot");
    QTEST(QString::fromStdString(lensData.calibrationFileZMeasurement), "correctionFileHeightMeasurement");
}

void LensModelTest::testCorrectionFile()
{
    LensModel model;

    QCOMPARE(model.correctionFile(CorrectionFileMode::Welding), "IntelliScanIII30_F_Theta_340.ct5");
    QCOMPARE(model.correctionFile(CorrectionFileMode::Pilot), "IntelliScanIII30_F_Theta_340Pilot.ct5");
    QCOMPARE(model.correctionFile(CorrectionFileMode::HeightMeasurement), "IntelliScanIII30_F_Theta_340HeightMeasurement.ct5");

    model.setType(LensType::F_Theta_460);
    QCOMPARE(model.correctionFile(CorrectionFileMode::Welding), "IntelliScanIII30_F_Theta_460.ct5");
    QCOMPARE(model.correctionFile(CorrectionFileMode::Pilot), "IntelliScanIII30_F_Theta_460Pilot.ct5");
    QCOMPARE(model.correctionFile(CorrectionFileMode::HeightMeasurement), "IntelliScanIII30_F_Theta_460HeightMeasurement.ct5");

    model.setType(LensType::F_Theta_255);
    QCOMPARE(model.correctionFile(CorrectionFileMode::Welding), "IntelliScanIII30_F_Theta_255.ct5");
    QCOMPARE(model.correctionFile(CorrectionFileMode::Pilot), "IntelliScanIII30_F_Theta_255Pilot.ct5");
    QCOMPARE(model.correctionFile(CorrectionFileMode::HeightMeasurement), "IntelliScanIII30_F_Theta_255HeightMeasurement.ct5");

    model.setScannerController(ScannerModel::SmartMoveScanner);
    QCOMPARE(model.correctionFile(CorrectionFileMode::Welding), "ME_F255.sbd");
    QCOMPARE(model.correctionFile(CorrectionFileMode::Pilot), "ME_F255.sbd");
    QCOMPARE(model.correctionFile(CorrectionFileMode::HeightMeasurement), "ME_F255.sbd");

    model.setType(LensType::F_Theta_340);
    QCOMPARE(model.correctionFile(CorrectionFileMode::Welding), "ME_F340.sbd");
    QCOMPARE(model.correctionFile(CorrectionFileMode::Pilot), "ME_F340.sbd");
    QCOMPARE(model.correctionFile(CorrectionFileMode::HeightMeasurement), "ME_F340.sbd");

    model.setType(LensType::F_Theta_460);
    QCOMPARE(model.correctionFile(CorrectionFileMode::Welding), "ME_F460.sbd");
    QCOMPARE(model.correctionFile(CorrectionFileMode::Pilot), "ME_F460.sbd");
    QCOMPARE(model.correctionFile(CorrectionFileMode::HeightMeasurement), "ME_F460.sbd");

    model.setScannerController(ScannerModel::ScanlabScanner);
    QCOMPARE(model.correctionFile(CorrectionFileMode::Welding), "IntelliScanIII30_F_Theta_460.ct5");
    QCOMPARE(model.correctionFile(CorrectionFileMode::Pilot), "IntelliScanIII30_F_Theta_460Pilot.ct5");
    QCOMPARE(model.correctionFile(CorrectionFileMode::HeightMeasurement), "IntelliScanIII30_F_Theta_460HeightMeasurement.ct5");

    model.setType(LensType::F_Theta_255);
    QCOMPARE(model.correctionFile(CorrectionFileMode::Welding), "IntelliScanIII30_F_Theta_255.ct5");
    QCOMPARE(model.correctionFile(CorrectionFileMode::Pilot), "IntelliScanIII30_F_Theta_255Pilot.ct5");
    QCOMPARE(model.correctionFile(CorrectionFileMode::HeightMeasurement), "IntelliScanIII30_F_Theta_255HeightMeasurement.ct5");

    model.setType(LensType::F_Theta_340);
    QCOMPARE(model.correctionFile(CorrectionFileMode::Welding), "IntelliScanIII30_F_Theta_340.ct5");
    QCOMPARE(model.correctionFile(CorrectionFileMode::Pilot), "IntelliScanIII30_F_Theta_340Pilot.ct5");
    QCOMPARE(model.correctionFile(CorrectionFileMode::HeightMeasurement), "IntelliScanIII30_F_Theta_340HeightMeasurement.ct5");
}

void LensModelTest::testCorrectionFilePrefix()
{
    LensModel model;

    QCOMPARE(model.correctionFilePrefix(), "IntelliScanIII30");
    model.setScannerController(ScannerModel::SmartMoveScanner);
    QCOMPARE(model.correctionFilePrefix(), "ME");
    model.setScannerController(ScannerModel::ScanlabScanner);
    QCOMPARE(model.correctionFilePrefix(), "IntelliScanIII30");
}

void LensModelTest::testCorrectionFileLens()
{
    LensModel model;

    model.setScannerController(ScannerModel::SmartMoveScanner);
    QCOMPARE(model.correctionFileLens(), "_F340");
    model.setType(LensType::F_Theta_460);
    QCOMPARE(model.correctionFileLens(), "_F460");
    model.setType(LensType::F_Theta_255);
    QCOMPARE(model.correctionFileLens(), "_F255");
    model.setType(LensType::F_Theta_340);
    QCOMPARE(model.correctionFileLens(), "_F340");

    model.setScannerController(ScannerModel::ScanlabScanner);

    QCOMPARE(model.correctionFileLens(), "_F_Theta_340");
    model.setType(LensType::F_Theta_460);
    QCOMPARE(model.correctionFileLens(), "_F_Theta_460");
    model.setType(LensType::F_Theta_255);
    QCOMPARE(model.correctionFileLens(), "_F_Theta_255");
    model.setType(LensType::F_Theta_340);
    QCOMPARE(model.correctionFileLens(), "_F_Theta_340");

    model.setScannerController(ScannerModel::SmartMoveScanner);

    QCOMPARE(model.correctionFileLens(), "_F340");
    model.setType(LensType::F_Theta_460);
    QCOMPARE(model.correctionFileLens(), "_F460");
    model.setType(LensType::F_Theta_255);
    QCOMPARE(model.correctionFileLens(), "_F255");
    model.setType(LensType::F_Theta_340);
    QCOMPARE(model.correctionFileLens(), "_F340");
}

void LensModelTest::testCorrectionFileLensScanlab()
{
    LensModel model;

    QCOMPARE(model.correctionFileLensScanlab(), "_F_Theta_340");
    model.setType(LensType::F_Theta_460);
    QCOMPARE(model.correctionFileLensScanlab(), "_F_Theta_460");
    model.setType(LensType::F_Theta_255);
    QCOMPARE(model.correctionFileLensScanlab(), "_F_Theta_255");
    model.setType(LensType::F_Theta_340);
    QCOMPARE(model.correctionFileLensScanlab(), "_F_Theta_340");

    model.setScannerController(ScannerModel::ScanlabScanner);

    QCOMPARE(model.correctionFileLensScanlab(), "_F_Theta_340");
    model.setType(LensType::F_Theta_460);
    QCOMPARE(model.correctionFileLensScanlab(), "_F_Theta_460");
    model.setType(LensType::F_Theta_255);
    QCOMPARE(model.correctionFileLensScanlab(), "_F_Theta_255");
    model.setType(LensType::F_Theta_340);
    QCOMPARE(model.correctionFileLensScanlab(), "_F_Theta_340");
}

void LensModelTest::testCorrectionFileLensSmartMove()
{
    LensModel model;

    model.setScannerController(ScannerModel::SmartMoveScanner);

    QCOMPARE(model.correctionFileLensSmartMove(), "_F340");
    model.setType(LensType::F_Theta_460);
    QCOMPARE(model.correctionFileLensSmartMove(), "_F460");
    model.setType(LensType::F_Theta_255);
    QCOMPARE(model.correctionFileLensSmartMove(), "_F255");
    model.setType(LensType::F_Theta_340);
    QCOMPARE(model.correctionFileLensSmartMove(), "_F340");
}

void LensModelTest::testCorrectionFileMode()
{
    LensModel model;

    QCOMPARE(model.correctionFileMode(CorrectionFileMode::Welding), "");
    QCOMPARE(model.correctionFileMode(CorrectionFileMode::Pilot), "Pilot");
    QCOMPARE(model.correctionFileMode(CorrectionFileMode::HeightMeasurement), "HeightMeasurement");
    QCOMPARE(model.correctionFileMode(CorrectionFileMode::Welding), "");

    model.setScannerController(ScannerModel::SmartMoveScanner);
    QCOMPARE(model.correctionFileMode(CorrectionFileMode::Welding), "");
    QCOMPARE(model.correctionFileMode(CorrectionFileMode::Pilot), "");
    QCOMPARE(model.correctionFileMode(CorrectionFileMode::HeightMeasurement), "");
    QCOMPARE(model.correctionFileMode(CorrectionFileMode::Welding), "");
}

void LensModelTest::testCorrectionFileSuffix()
{
    LensModel model;

    QCOMPARE(model.correctionFileSuffix(), ".ct5");
    model.setScannerController(ScannerModel::SmartMoveScanner);
    QCOMPARE(model.correctionFileSuffix(), ".sbd");
    model.setScannerController(ScannerModel::ScanlabScanner);
    QCOMPARE(model.correctionFileSuffix(), ".ct5");
}

QTEST_GUILESS_MAIN(LensModelTest)
#include "lensModelTest.moc"
