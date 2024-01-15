#include <QTest>

#include "viWeldHead/Scanlab/scannerLogger.h"

using precitec::hardware::logger::ScannerLogger;

class ScannerLoggerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testCorrectionFileErrorMessage();
    void testCorrectionFileModeSelectionFailed();
    void testLogCorrectionFileNumber();
    void testLogCorrectionFileMode();
};

void ScannerLoggerTest::testCtor()
{
    ScannerLogger logger;
}

void ScannerLoggerTest::testCorrectionFileErrorMessage()
{
    ScannerLogger logger;

    QVERIFY(!logger.correctionFileErrorMessage(CorrectionFileErrorCode::Success));
    QVERIFY(logger.correctionFileErrorMessage(CorrectionFileErrorCode::FileCorrupt));
    QVERIFY(logger.correctionFileErrorMessage(CorrectionFileErrorCode::Memory));
    QVERIFY(logger.correctionFileErrorMessage(CorrectionFileErrorCode::FileNotFound));
    QVERIFY(logger.correctionFileErrorMessage(CorrectionFileErrorCode::DSPMemory));
    QVERIFY(logger.correctionFileErrorMessage(CorrectionFileErrorCode::PCIDownload));
    QVERIFY(logger.correctionFileErrorMessage(CorrectionFileErrorCode::RTC6Driver));
    QVERIFY(logger.correctionFileErrorMessage(CorrectionFileErrorCode::CorrectionFileNumber));
    QVERIFY(logger.correctionFileErrorMessage(CorrectionFileErrorCode::Access));
    QVERIFY(logger.correctionFileErrorMessage(CorrectionFileErrorCode::Option3DUnlocked));
    QVERIFY(logger.correctionFileErrorMessage(CorrectionFileErrorCode::Busy));
    QVERIFY(logger.correctionFileErrorMessage(CorrectionFileErrorCode::PCIUpload));
    QVERIFY(logger.correctionFileErrorMessage(CorrectionFileErrorCode::Verify));
}

void ScannerLoggerTest::testCorrectionFileModeSelectionFailed()
{
    ScannerLogger logger;

    logger.logCorrectionFileModeSelectionFailed(CorrectionFileMode::Welding);
    logger.logCorrectionFileModeSelectionFailed(CorrectionFileMode::Pilot);
    logger.logCorrectionFileModeSelectionFailed(CorrectionFileMode::HeightMeasurement);
}

void ScannerLoggerTest::testLogCorrectionFileNumber()
{
    ScannerLogger logger;

    logger.logCorrectionFileNumber(0);
    logger.logCorrectionFileNumber(1);
    logger.logCorrectionFileNumber(2);
    logger.logCorrectionFileNumber(3);
}

void ScannerLoggerTest::testLogCorrectionFileMode()
{
    ScannerLogger logger;

    logger.logCorrectionFileMode(CorrectionFileMode::Pilot);
    logger.logCorrectionFileMode(CorrectionFileMode::Welding);
    logger.logCorrectionFileMode(CorrectionFileMode::HeightMeasurement);
    logger.logCorrectionFileMode(CorrectionFileMode::Welding);
}

QTEST_GUILESS_MAIN(ScannerLoggerTest)
#include "loggerTest.moc"
