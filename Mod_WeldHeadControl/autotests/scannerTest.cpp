#include <QTest>
#include <QDebug>

#include "viWeldHead/Scanlab/RTC6Scanner.h"

using RTC6::Scanner;

/**
 * Unit test of RTC6 scanner.
 **/
class ScannerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testLensType();
    void testEnums();
    void testCheckCustomCorrectionFileSpecified();
};

void ScannerTest::testCtor()
{
    Scanner scanner;

    QCOMPARE(scanner.scannerModel(), ScannerModel::ScanlabScanner);
    QCOMPARE(scanner.get_CalibValueBitsPerMM(), 4000.0);
    long int xOffset;
    long int yOffset;
    double angle;
    scanner.get_ScannerOffset(xOffset, yOffset, angle);
    QCOMPARE(xOffset, 0);
    QCOMPARE(yOffset, 0);
    QCOMPARE(angle, 0.0);
    long int jumpDelay;
    long int markDelay;
    long int polygonDelay;
    scanner.get_ScannerDelays(jumpDelay, markDelay, polygonDelay);
    QCOMPARE(jumpDelay, 0);
    QCOMPARE(markDelay, 0);
    QCOMPARE(polygonDelay, 0);
}

void ScannerTest::testLensType()
{
    Scanner scanner;

    QCOMPARE(scanner.m_lensType, LensType::F_Theta_340);
    scanner.setLensType(LensType::F_Theta_460);
    QCOMPARE(scanner.m_lensType, LensType::F_Theta_460);
    scanner.setLensType(LensType::F_Theta_340);
    QCOMPARE(scanner.m_lensType, LensType::F_Theta_340);
    scanner.setLensType(LensType::F_Theta_255);
    QCOMPARE(scanner.m_lensType, LensType::F_Theta_255);
}

void ScannerTest::testEnums()
{
    Scanner scanner;

    QCOMPARE(static_cast<int>(Scanner::CorrectionFileType::systemSpecific), 0);
    QCOMPARE(static_cast<int>(Scanner::CorrectionFileType::lenseSpecific), 1);

    QCOMPARE(static_cast<int>(precitec::interface::ScannerModel::ScanlabScanner), 0);
    QCOMPARE(static_cast<int>(precitec::interface::ScannerModel::SmartMoveScanner), 1);

    QCOMPARE(static_cast<int>(precitec::interface::LensType::F_Theta_340), 1);
    QCOMPARE(static_cast<int>(precitec::interface::LensType::F_Theta_460), 2);
    QCOMPARE(static_cast<int>(precitec::interface::LensType::F_Theta_255), 3);

    QCOMPARE(static_cast<int>(precitec::interface::CorrectionFileMode::Welding), 1);
    QCOMPARE(static_cast<int>(precitec::interface::CorrectionFileMode::Pilot), 2);
    QCOMPARE(static_cast<int>(precitec::interface::CorrectionFileMode::HeightMeasurement), 3);
}

void ScannerTest::testCheckCustomCorrectionFileSpecified()
{
    Scanner scanner;

    QCOMPARE(scanner.m_correctionFileType, Scanner::CorrectionFileType::lenseSpecific);
    QVERIFY(scanner.m_oCorrectionFile.empty());
    QVERIFY(scanner.m_customCorrectionFile.empty());

    scanner.checkCustomCorrectionFileSpecified();

    QCOMPARE(scanner.m_correctionFileType, Scanner::CorrectionFileType::lenseSpecific);
    QCOMPARE(scanner.m_oCorrectionFile, "IntelliScanIII30_F_Theta_340");

    scanner.setLensType(LensType::F_Theta_460);

    scanner.checkCustomCorrectionFileSpecified();
    QCOMPARE(scanner.m_correctionFileType, Scanner::CorrectionFileType::lenseSpecific);
    QCOMPARE(scanner.m_oCorrectionFile, "IntelliScanIII30_F_Theta_460");

    scanner.setLensType(LensType::F_Theta_255);

    scanner.checkCustomCorrectionFileSpecified();
    QCOMPARE(scanner.m_correctionFileType, Scanner::CorrectionFileType::lenseSpecific);
    QCOMPARE(scanner.m_oCorrectionFile, "IntelliScanIII30_F_Theta_255");

    scanner.m_customCorrectionFile = std::string("customCorrectionFile");

    scanner.checkCustomCorrectionFileSpecified();
    QVERIFY(!scanner.m_customCorrectionFile.empty());
    QCOMPARE(scanner.m_correctionFileType, Scanner::CorrectionFileType::systemSpecific);
    QCOMPARE(scanner.m_oCorrectionFile, "customCorrectionFile");

    scanner.m_customCorrectionFile = "";

    scanner.checkCustomCorrectionFileSpecified();
    QVERIFY(scanner.m_customCorrectionFile.empty());
    QCOMPARE(scanner.m_correctionFileType, Scanner::CorrectionFileType::lenseSpecific);
    QCOMPARE(scanner.m_oCorrectionFile, "IntelliScanIII30_F_Theta_255");
}

QTEST_GUILESS_MAIN(ScannerTest)
#include "scannerTest.moc"
