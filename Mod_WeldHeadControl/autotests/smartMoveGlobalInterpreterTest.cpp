#include <QTest>

#include "../include/viWeldHead/Scanlab/smartMoveGlobalInterpreter.h"

using precitec::hardware::SmartMoveGlobalInterpreter;

/**
 *  This class is used to check the translation of global commands.
 *  The translated commands are scanner specific commands which are used
 *  to execute specific tasks like moving or configure the scanner.
 **/
class SmartMoveGlobalInterpreterTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testTranslateGlobalCommands_data();
    void testTranslateGlobalCommands();         //Also tests functions "currentCommandSeries()", "isCurrentCommandSeriesEmpty()" and "reset()"
    void testTranslateInit_data();
    void testTranslateInit();
    void testMergeMultipleSetsToOneMultiSet_data();
    void testMergeMultipleSetsToOneMultiSet();
    void testTranslateProcessJob();
    void testTranslateJobSelect();
    void testTranslateSystemOperationMode();
    void testTranslateInputMode();
    void testTranslateAlign();
    void testTranslateLaserOnMaxForAlignment();
    void testTranslateCalibrationFileName();
    void testTranslateFocalLength();
    void testTranslateScanfieldSize();
    void testTranslateJumpSpeed();
    void testTranslateMarkSpeed();
    void testTranslatePositionFeedbackX();
    void testTranslatePositionFeedbackY();
    void testTranslatePositionCommandX();
    void testTranslatePositionCommandY();
    void testTranslatePrefixSetOrGet();
    void testCheckIfOnlySetCommands_data();
    void testCheckIfOnlySetCommands();
    void testCheckCreateMultiSetCommand_data();
    void testCheckCreateMultiSetCommand();
};

void SmartMoveGlobalInterpreterTest::testCtor()
{
    SmartMoveGlobalInterpreter globalInterpreter;

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());
    const auto& globalInterpretedCommands = globalInterpreter.currentCommandSeries();
    QVERIFY(globalInterpretedCommands.empty());
}

void SmartMoveGlobalInterpreterTest::testTranslateGlobalCommands_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<std::vector<QString>>("driveToZero");
    QTest::addColumn<std::vector<QString>>("driveToPosition1");

    QTest::newRow("DriveToZero") << 0 << std::vector<QString> {"S SYS_OPMODE 0",
                                                     "S INPUT_MODE 10",
                                                     "S ALIGN_X 0",
                                                     "S ALIGN_Y 0",
                                                     "S ALIGN_LASERON 0",
                                                     "S ALIGN_LASERON_MAX 0"} <<
                                         std::vector<QString> {"S SYS_OPMODE 0",
                                                     "S INPUT_MODE 10",
                                                     "S ALIGN_X -100",
                                                     "S ALIGN_Y 100",
                                                     "S ALIGN_LASERON 0",
                                                     "S ALIGN_LASERON_MAX 0"};
}

void SmartMoveGlobalInterpreterTest::testTranslateGlobalCommands()
{
    SmartMoveGlobalInterpreter globalInterpreter;
    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    //Drive to position
    std::vector<std::shared_ptr<precitec::hardware::GlobalCommand>> driveToPosition;
    std::shared_ptr<precitec::hardware::SystemOperationMode> sysOpMode(new precitec::hardware::SystemOperationMode);
    driveToPosition.push_back(std::move(sysOpMode));
    std::shared_ptr<precitec::hardware::InputMode> inputMode(new precitec::hardware::InputMode);
    driveToPosition.push_back(std::move(inputMode));
    std::shared_ptr<precitec::hardware::Align> align(new precitec::hardware::Align);    //Drive to origin
    driveToPosition.push_back(std::move(align));
    std::shared_ptr<precitec::hardware::LaserOnForAlignment> laserAlignment(new precitec::hardware::LaserOnForAlignment);
    driveToPosition.push_back(std::move(laserAlignment));

    globalInterpreter.translateGlobalCommands(driveToPosition);
    QVERIFY(!globalInterpreter.isCurrentCommandSeriesEmpty());

    QFETCH(std::vector<QString>, driveToZero);
    const auto& driveToZeroCommands = globalInterpreter.currentCommandSeries();
    for (std::size_t i = 0; i < driveToZeroCommands.size(); i++)
    {
        QCOMPARE(QString::fromStdString(driveToZeroCommands.at(i)), driveToZero.at(i));
    }

    globalInterpreter.reset();
    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    //Drive to position
    driveToPosition.clear();
    std::shared_ptr<precitec::hardware::SystemOperationMode> sysOpMode2(new precitec::hardware::SystemOperationMode);
    driveToPosition.push_back(std::move(sysOpMode2));
    std::shared_ptr<precitec::hardware::InputMode> inputMode2(new precitec::hardware::InputMode);
    driveToPosition.push_back(std::move(inputMode2));
    std::shared_ptr<precitec::hardware::Align> align2(new precitec::hardware::Align);    ////Drive to virtual -100,100
    align2->x = -100;
    align2->y = 100;
    driveToPosition.push_back(std::move(align2));
    std::shared_ptr<precitec::hardware::LaserOnForAlignment> laserAlignment2(new precitec::hardware::LaserOnForAlignment);
    driveToPosition.push_back(std::move(laserAlignment2));

    globalInterpreter.translateGlobalCommands(driveToPosition);
    QVERIFY(!globalInterpreter.isCurrentCommandSeriesEmpty());

    QFETCH(std::vector<QString>, driveToPosition1);
    const auto& driveToPositionCommands = globalInterpreter.currentCommandSeries();
    for (std::size_t i = 0; i < driveToPositionCommands.size(); i++)
    {
        QCOMPARE(QString::fromStdString(driveToPositionCommands.at(i)), driveToPosition1.at(i));
    }

}

void SmartMoveGlobalInterpreterTest::testTranslateInit_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<std::vector<QString>>("init");

    QTest::newRow("Init") << 0 << std::vector<QString> {"S SFC_FOCALLEN 0.340000",
                                                     "S FIELD_SIZE 50.000000",
                                                     "G SFC_NAME"};
}

void SmartMoveGlobalInterpreterTest::testTranslateInit()
{
    SmartMoveGlobalInterpreter globalInterpreter;
    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    //Init
    std::vector<std::shared_ptr<precitec::hardware::GlobalCommand>> initGlobalCommands;
    std::shared_ptr<precitec::hardware::FocalLength> focalLength(new precitec::hardware::FocalLength);
    focalLength->focalLength = 0.340;
    initGlobalCommands.push_back(std::move(focalLength));
    std::shared_ptr<precitec::hardware::ScanfieldSize> scanfieldSize(new precitec::hardware::ScanfieldSize);
    scanfieldSize->scanfieldSize = 50;
    initGlobalCommands.push_back(std::move(scanfieldSize));
    std::shared_ptr<precitec::hardware::CalibrationFilename> calibrationFilename(new precitec::hardware::CalibrationFilename);
    initGlobalCommands.push_back(std::move(calibrationFilename));

    globalInterpreter.translateGlobalCommands(initGlobalCommands);
    QVERIFY(!globalInterpreter.isCurrentCommandSeriesEmpty());

    QFETCH(std::vector<QString>, init);
    const auto& initCommands = globalInterpreter.currentCommandSeries();
    for (std::size_t i = 0; i < init.size(); i++)
    {
        QCOMPARE(QString::fromStdString(initCommands.at(i)), init.at(i));
    }

    globalInterpreter.reset();
    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());
}

void SmartMoveGlobalInterpreterTest::testMergeMultipleSetsToOneMultiSet_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<std::vector<QString>>("multipleSetCommands");
    QTest::addColumn<int>("charsCountForReservation");
    QTest::addColumn<int>("resultSize");
    QTest::addColumn<QString>("multiSetCommand");

    QTest::newRow("MultiSetInit") << 0 << std::vector<QString> {"S SFC_FOCALLEN 0.460",
                                                     "S FIELD_SIZE 50"} << 14 << 34 << "S SFC_FOCALLEN 0.460 FIELD_SIZE 50";
    QTest::newRow("MultiSetAlignment") << 1 << std::vector<QString> {"S SYS_OPMODE 0",
                                                     "S INPUT_MODE 10",
                                                     "S ALIGN_X -30000",
                                                     "S ALIGN_Y 30000"} << 43 << 57 << "S SYS_OPMODE 0 INPUT_MODE 10 ALIGN_X -30000 ALIGN_Y 30000";
    QTest::newRow("MultiSetTooLong") << 2 << std::vector<QString> {"S SYS_OPMODE 0",
                                                     "S INPUT_MODE 10",
                                                     "S ALIGN_X -30000",
                                                     "S ALIGN_Y 30000",
                                                     "S ALIGN_LASERON 8388607",
                                                     "S ALIGN_LASERON_MAX 8388607"} << 91 << 0 << "";
    QTest::newRow("MultiSetSpeeds") << 3 << std::vector<QString> {"S MV_JUMPSPEED 2500",
                                                     "S MV_MARKSPEED 1200"} << 18 << 37 << "S MV_JUMPSPEED 2500 MV_MARKSPEED 1200";
    QTest::newRow("MultiSetOnlyGets") << 4 << std::vector<QString> {"G RT_CMD_X",
                                                     "G RT_CMD_Y"} << 0 << 0 << "";
    QTest::newRow("MultiSetGetsBeforeSets") << 5 << std::vector<QString> {"G RT_CMD_X", "S SYS_OPMODE 0", "S INPUT_MODE 10", "S ALIGN_X 0", "S ALIGN_Y 0"} << 0 << 0 << "";
    QTest::newRow("MultiSetSetsBeforeGets") << 6 << std::vector<QString> {"S SYS_OPMODE 0", "S INPUT_MODE 10", "S ALIGN_X 0", "S ALIGN_Y 0", "G RT_CMD_X"} << 51 << 0 << "";
}

void SmartMoveGlobalInterpreterTest::testMergeMultipleSetsToOneMultiSet()
{
    SmartMoveGlobalInterpreter globalInterpreter;

    QCOMPARE(globalInterpreter.m_sizeForMultiSetMerge, 0);

    QFETCH(std::vector<QString>, multipleSetCommands);
    QFETCH(int, charsCountForReservation);
    QFETCH(QString, multiSetCommand);
    QFETCH(int, resultSize);

    std::vector<std::string> multipleSets;
    multipleSets.reserve(multipleSetCommands.size());

    for (const auto& setCommand : multipleSetCommands)
    {
        multipleSets.push_back(setCommand.toStdString());
    }

    const auto& multiSetString = globalInterpreter.mergeMultipleSetsToOneMultiSet(multipleSets);
    QCOMPARE(QString::fromStdString(multiSetString), multiSetCommand);
    QCOMPARE(globalInterpreter.m_sizeForMultiSetMerge, charsCountForReservation);
    QCOMPARE(multiSetString.size(), resultSize);
}

void SmartMoveGlobalInterpreterTest::testTranslateProcessJob()
{
    SmartMoveGlobalInterpreter globalInterpreter;

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    std::shared_ptr<precitec::hardware::ProcessJob> processJob(new precitec::hardware::ProcessJob);
    processJob->set = precitec::smartMove::TCPCommand::Set;
    processJob->repeats = static_cast<int>(precitec::smartMove::JobRepeats::Infinite);
    auto returnValue = globalInterpreter.translateProcessJob(processJob);
    QCOMPARE(returnValue, "P -1");
    processJob->set = precitec::smartMove::TCPCommand::Get;
    processJob->repeats = static_cast<int>(precitec::smartMove::JobRepeats::SingleShot);
    returnValue = globalInterpreter.translateProcessJob(processJob);
    QCOMPARE(returnValue, "P 1");
    processJob->set = precitec::smartMove::TCPCommand::Get;
    processJob->repeats = static_cast<int>(precitec::smartMove::JobRepeats::Infinite);
    returnValue = globalInterpreter.translateProcessJob(processJob);
    QCOMPARE(returnValue, "P -1");
    processJob->repeats = static_cast<int>(precitec::smartMove::JobRepeats::SingleShot);

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    std::vector<std::shared_ptr<precitec::hardware::GlobalCommand>> globalCommandContainer;
    globalCommandContainer.push_back(std::move(processJob));

    globalInterpreter.translateGlobalCommands(globalCommandContainer);

    QVERIFY(!globalInterpreter.isCurrentCommandSeriesEmpty());

    QCOMPARE(globalInterpreter.currentCommandSeries().front(), "P 1");
}

void SmartMoveGlobalInterpreterTest::testTranslateJobSelect()
{
    SmartMoveGlobalInterpreter globalInterpreter;

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    std::shared_ptr<precitec::hardware::JobSelect> jobSelect(new precitec::hardware::JobSelect);
    jobSelect->set = precitec::smartMove::TCPCommand::Set;
    jobSelect->jobID = 10;
    auto returnValue = globalInterpreter.translateJobSelect(jobSelect);
    QCOMPARE(returnValue, "J id 10");
    jobSelect->set = precitec::smartMove::TCPCommand::Get;
    jobSelect->jobID = 1;
    returnValue = globalInterpreter.translateJobSelect(jobSelect);
    QCOMPARE(returnValue, "J id 1");
    jobSelect->set = precitec::smartMove::TCPCommand::Set;
    jobSelect->jobID = 150;
    returnValue = globalInterpreter.translateJobSelect(jobSelect);
    QCOMPARE(returnValue, "J id 150");

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    std::vector<std::shared_ptr<precitec::hardware::GlobalCommand>> globalCommandContainer;
    globalCommandContainer.push_back(std::move(jobSelect));

    globalInterpreter.translateGlobalCommands(globalCommandContainer);

    QVERIFY(!globalInterpreter.isCurrentCommandSeriesEmpty());

    QCOMPARE(globalInterpreter.currentCommandSeries().front(), "J id 150");
}

void SmartMoveGlobalInterpreterTest::testTranslateSystemOperationMode()
{
    SmartMoveGlobalInterpreter globalInterpreter;

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    std::shared_ptr<precitec::hardware::SystemOperationMode> sysOpMode(new precitec::hardware::SystemOperationMode);
    sysOpMode->set = true;
    sysOpMode->sysOpMode = SystemOperationModes::Marking;
    auto returnValue = globalInterpreter.translateGlobalCommand(sysOpMode);
    QCOMPARE(returnValue.front(), "S SYS_OPMODE 1");
    sysOpMode->set = false;
    returnValue = globalInterpreter.translateGlobalCommand(sysOpMode);
    QCOMPARE(returnValue.front(), "G SYS_OPMODE ");
    sysOpMode->sysOpMode = SystemOperationModes::Service;
    returnValue = globalInterpreter.translateGlobalCommand(sysOpMode);
    QCOMPARE(returnValue.front(), "G SYS_OPMODE ");
    sysOpMode->set = true;
    returnValue = globalInterpreter.translateGlobalCommand(sysOpMode);
    QCOMPARE(returnValue.front(), "S SYS_OPMODE 0");

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    std::vector<std::shared_ptr<precitec::hardware::GlobalCommand>> globalCommandContainer;
    globalCommandContainer.push_back(std::move(sysOpMode));

    globalInterpreter.translateGlobalCommands(globalCommandContainer);

    QVERIFY(!globalInterpreter.isCurrentCommandSeriesEmpty());

    QCOMPARE(globalInterpreter.currentCommandSeries().front(), "S SYS_OPMODE 0");
}

void SmartMoveGlobalInterpreterTest::testTranslateInputMode()
{
    SmartMoveGlobalInterpreter globalInterpreter;

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    std::shared_ptr<precitec::hardware::InputMode> inputMode(new precitec::hardware::InputMode);
    inputMode->set = true;
    inputMode->inputMode = InputModes::Alignment;
    auto returnValue = globalInterpreter.translateGlobalCommand(inputMode);
    QCOMPARE(returnValue.front(), "S INPUT_MODE 10");
    inputMode->set = false;
    returnValue = globalInterpreter.translateGlobalCommand(inputMode);
    QCOMPARE(returnValue.front(), "G INPUT_MODE ");
    inputMode->inputMode = InputModes::Calibration;
    returnValue = globalInterpreter.translateGlobalCommand(inputMode);
    QCOMPARE(returnValue.front(), "G INPUT_MODE ");
    inputMode->set = true;
    returnValue = globalInterpreter.translateGlobalCommand(inputMode);
    QCOMPARE(returnValue.front(), "S INPUT_MODE 1");
    inputMode->inputMode = InputModes::WaveformGenerator;
    returnValue = globalInterpreter.translateGlobalCommand(inputMode);
    QCOMPARE(returnValue.front(), "S INPUT_MODE 0");
    inputMode->set = false;
    returnValue = globalInterpreter.translateGlobalCommand(inputMode);
    QCOMPARE(returnValue.front(), "G INPUT_MODE ");

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    std::vector<std::shared_ptr<precitec::hardware::GlobalCommand>> globalCommandContainer;
    globalCommandContainer.push_back(std::move(inputMode));

    globalInterpreter.translateGlobalCommands(globalCommandContainer);

    QVERIFY(!globalInterpreter.isCurrentCommandSeriesEmpty());

    QCOMPARE(globalInterpreter.currentCommandSeries().front(), "G INPUT_MODE ");
}

void SmartMoveGlobalInterpreterTest::testTranslateAlign()
{
    SmartMoveGlobalInterpreter globalInterpreter;

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    std::shared_ptr<precitec::hardware::Align> align(new precitec::hardware::Align);
    align->set = true;
    align->x = 10;
    align->y = 0;
    auto returnValue = globalInterpreter.translateGlobalCommand(align);
    QCOMPARE(returnValue.front(), "S ALIGN_X 10");
    QCOMPARE(returnValue.back(), "S ALIGN_Y 0");
    align->set = false;
    returnValue = globalInterpreter.translateGlobalCommand(align);
    QCOMPARE(returnValue.front(), "G ALIGN_X ");
    QCOMPARE(returnValue.back(), "G ALIGN_Y ");
    align->x = -1000;
    align->y = -5;
    returnValue = globalInterpreter.translateGlobalCommand(align);
    QCOMPARE(returnValue.front(), "G ALIGN_X ");
    QCOMPARE(returnValue.back(), "G ALIGN_Y ");
    align->set = true;
    returnValue = globalInterpreter.translateGlobalCommand(align);
    QCOMPARE(returnValue.front(), "S ALIGN_X -1000");
    QCOMPARE(returnValue.back(), "S ALIGN_Y -5");
    align->x = 3000;
    returnValue = globalInterpreter.translateGlobalCommand(align);
    QCOMPARE(returnValue.front(), "S ALIGN_X 3000");
    QCOMPARE(returnValue.back(), "S ALIGN_Y -5");

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    std::vector<std::shared_ptr<precitec::hardware::GlobalCommand>> globalCommandContainer;
    globalCommandContainer.push_back(std::move(align));

    globalInterpreter.translateGlobalCommands(globalCommandContainer);

    QVERIFY(!globalInterpreter.isCurrentCommandSeriesEmpty());

    QCOMPARE(globalInterpreter.currentCommandSeries().front(), "S ALIGN_X 3000");
    QCOMPARE(globalInterpreter.currentCommandSeries().back(), "S ALIGN_Y -5");
}

void SmartMoveGlobalInterpreterTest::testTranslateLaserOnMaxForAlignment()
{
    SmartMoveGlobalInterpreter globalInterpreter;

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    std::shared_ptr<precitec::hardware::LaserOnForAlignment> laserAlignment(new precitec::hardware::LaserOnForAlignment);
    laserAlignment->set = true;
    laserAlignment->laserOn = 0;
    laserAlignment->laserOnMax = 0;
    auto returnValue = globalInterpreter.translateGlobalCommand(laserAlignment);
    QCOMPARE(returnValue.front(), "S ALIGN_LASERON 0");
    QCOMPARE(returnValue.back(), "S ALIGN_LASERON_MAX 0");
    laserAlignment->set = false;
    returnValue = globalInterpreter.translateGlobalCommand(laserAlignment);
    QCOMPARE(returnValue.front(), "G ALIGN_LASERON ");
    QCOMPARE(returnValue.back(), "G ALIGN_LASERON_MAX ");
    laserAlignment->laserOn = 1000;
    laserAlignment->laserOnMax = 100;
    returnValue = globalInterpreter.translateGlobalCommand(laserAlignment);
    QCOMPARE(returnValue.front(), "G ALIGN_LASERON ");
    QCOMPARE(returnValue.back(), "G ALIGN_LASERON_MAX ");
    laserAlignment->set = true;
    returnValue = globalInterpreter.translateGlobalCommand(laserAlignment);
    QCOMPARE(returnValue.front(), "S ALIGN_LASERON 1000");
    QCOMPARE(returnValue.back(), "S ALIGN_LASERON_MAX 100");
    laserAlignment->laserOn = 3000;
    returnValue = globalInterpreter.translateGlobalCommand(laserAlignment);
    QCOMPARE(returnValue.front(), "S ALIGN_LASERON 3000");
    QCOMPARE(returnValue.back(), "S ALIGN_LASERON_MAX 100");

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    std::vector<std::shared_ptr<precitec::hardware::GlobalCommand>> globalCommandContainer;
    globalCommandContainer.push_back(std::move(laserAlignment));

    globalInterpreter.translateGlobalCommands(globalCommandContainer);

    QVERIFY(!globalInterpreter.isCurrentCommandSeriesEmpty());

    QCOMPARE(globalInterpreter.currentCommandSeries().front(), "S ALIGN_LASERON 3000");
    QCOMPARE(globalInterpreter.currentCommandSeries().back(), "S ALIGN_LASERON_MAX 100");
}

void SmartMoveGlobalInterpreterTest::testTranslateCalibrationFileName()
{
    SmartMoveGlobalInterpreter globalInterpreter;

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    std::shared_ptr<precitec::hardware::CalibrationFilename> calibrationFile(new precitec::hardware::CalibrationFilename);
    auto returnValue = globalInterpreter.translateCalibrationFileName();
    QCOMPARE(returnValue, "G SFC_NAME");
    returnValue = globalInterpreter.translateCalibrationFileName();
    QCOMPARE(returnValue, "G SFC_NAME");
    returnValue = globalInterpreter.translateCalibrationFileName();
    QCOMPARE(returnValue, "G SFC_NAME");

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    std::vector<std::shared_ptr<precitec::hardware::GlobalCommand>> globalCommandContainer;
    globalCommandContainer.push_back(std::move(calibrationFile));

    globalInterpreter.translateGlobalCommands(globalCommandContainer);

    QVERIFY(!globalInterpreter.isCurrentCommandSeriesEmpty());

    QCOMPARE(globalInterpreter.currentCommandSeries().front(), "G SFC_NAME");
}

void SmartMoveGlobalInterpreterTest::testTranslateFocalLength()
{
    SmartMoveGlobalInterpreter globalInterpreter;

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    std::shared_ptr<precitec::hardware::FocalLength> focalLength(new precitec::hardware::FocalLength);
    focalLength->set = true;
    focalLength->focalLength = 10;
    auto returnValue = globalInterpreter.translateGlobalCommand(focalLength);
    QCOMPARE(returnValue.front(), "S SFC_FOCALLEN 10.000000");
    focalLength->set = false;
    focalLength->focalLength = 50;
    returnValue = globalInterpreter.translateGlobalCommand(focalLength);
    QCOMPARE(returnValue.front(), "G SFC_FOCALLEN ");
    focalLength->set = true;
    focalLength->focalLength = 0.255;
    returnValue = globalInterpreter.translateGlobalCommand(focalLength);
    QCOMPARE(returnValue.front(), "S SFC_FOCALLEN 0.255000");

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    std::vector<std::shared_ptr<precitec::hardware::GlobalCommand>> globalCommandContainer;
    globalCommandContainer.push_back(std::move(focalLength));

    globalInterpreter.translateGlobalCommands(globalCommandContainer);

    QVERIFY(!globalInterpreter.isCurrentCommandSeriesEmpty());

    QCOMPARE(globalInterpreter.currentCommandSeries().front(), "S SFC_FOCALLEN 0.255000");
}

void SmartMoveGlobalInterpreterTest::testTranslateScanfieldSize()
{
    SmartMoveGlobalInterpreter globalInterpreter;

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    std::shared_ptr<precitec::hardware::ScanfieldSize> scanfield(new precitec::hardware::ScanfieldSize);
    scanfield->set = true;
    scanfield->scanfieldSize = 0.5;
    auto returnValue = globalInterpreter.translateGlobalCommand(scanfield);
    QCOMPARE(returnValue.front(), "S FIELD_SIZE 0.500000");
    scanfield->set = false;
    scanfield->scanfieldSize = 100;
    returnValue = globalInterpreter.translateGlobalCommand(scanfield);
    QCOMPARE(returnValue.front(), "G FIELD_SIZE ");
    scanfield->set = true;
    scanfield->scanfieldSize = 1.125;
    returnValue = globalInterpreter.translateGlobalCommand(scanfield);
    QCOMPARE(returnValue.front(), "S FIELD_SIZE 1.125000");

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    std::vector<std::shared_ptr<precitec::hardware::GlobalCommand>> globalCommandContainer;
    globalCommandContainer.push_back(std::move(scanfield));

    globalInterpreter.translateGlobalCommands(globalCommandContainer);

    QVERIFY(!globalInterpreter.isCurrentCommandSeriesEmpty());

    QCOMPARE(globalInterpreter.currentCommandSeries().front(), "S FIELD_SIZE 1.125000");
}

void SmartMoveGlobalInterpreterTest::testTranslateJumpSpeed()
{
    SmartMoveGlobalInterpreter globalInterpreter;

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    std::shared_ptr<precitec::hardware::JumpSpeed> jumpSpeed(new precitec::hardware::JumpSpeed);
    jumpSpeed->set = true;
    jumpSpeed->speed = 5.0;
    auto returnValue = globalInterpreter.translateGlobalCommand(jumpSpeed);
    QCOMPARE(returnValue.front(), "S MV_JUMPSPEED 5.000000");
    jumpSpeed->set = false;
    jumpSpeed->speed = 10.25;
    returnValue = globalInterpreter.translateGlobalCommand(jumpSpeed);
    QCOMPARE(returnValue.front(), "G MV_JUMPSPEED ");
    jumpSpeed->set = true;
    jumpSpeed->speed = 1925.5;
    returnValue = globalInterpreter.translateGlobalCommand(jumpSpeed);
    QCOMPARE(returnValue.front(), "S MV_JUMPSPEED 1925.500000");

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    std::vector<std::shared_ptr<precitec::hardware::GlobalCommand>> globalCommandContainer;
    globalCommandContainer.push_back(std::move(jumpSpeed));

    globalInterpreter.translateGlobalCommands(globalCommandContainer);

    QVERIFY(!globalInterpreter.isCurrentCommandSeriesEmpty());

    QCOMPARE(globalInterpreter.currentCommandSeries().front(), "S MV_JUMPSPEED 1925.500000");
}

void SmartMoveGlobalInterpreterTest::testTranslateMarkSpeed()
{
    SmartMoveGlobalInterpreter globalInterpreter;

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    std::shared_ptr<precitec::hardware::MarkSpeed> markSpeed(new precitec::hardware::MarkSpeed);
    markSpeed->set = true;
    markSpeed->speed = 250.01;
    auto returnValue = globalInterpreter.translateGlobalCommand(markSpeed);
    QCOMPARE(returnValue.front(), "S MV_MARKSPEED 250.010000");
    markSpeed->set = false;
    markSpeed->speed = 10.25;
    returnValue = globalInterpreter.translateGlobalCommand(markSpeed);
    QCOMPARE(returnValue.front(), "G MV_MARKSPEED ");
    markSpeed->set = true;
    markSpeed->speed = 1900;
    returnValue = globalInterpreter.translateGlobalCommand(markSpeed);
    QCOMPARE(returnValue.front(), "S MV_MARKSPEED 1900.000000");

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    std::vector<std::shared_ptr<precitec::hardware::GlobalCommand>> globalCommandContainer;
    globalCommandContainer.push_back(std::move(markSpeed));

    globalInterpreter.translateGlobalCommands(globalCommandContainer);

    QVERIFY(!globalInterpreter.isCurrentCommandSeriesEmpty());

    QCOMPARE(globalInterpreter.currentCommandSeries().front(), "S MV_MARKSPEED 1900.000000");
}

void SmartMoveGlobalInterpreterTest::testTranslatePositionFeedbackX()
{
    SmartMoveGlobalInterpreter globalInterpreter;

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    QCOMPARE(globalInterpreter.translatePositionFeedbackX(), "G RT_PFB_X");

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    std::vector<std::shared_ptr<precitec::hardware::GlobalCommand>> globalCommandContainer;
    std::shared_ptr<precitec::hardware::PositionFeedbackBits> positionFeedback(new precitec::hardware::PositionFeedbackBits);

    globalCommandContainer.push_back(std::move(positionFeedback));

    globalInterpreter.translateGlobalCommands(globalCommandContainer);

    QVERIFY(!globalInterpreter.isCurrentCommandSeriesEmpty());

    QCOMPARE(globalInterpreter.currentCommandSeries().front(), "G RT_PFB_X");
    QCOMPARE(globalInterpreter.currentCommandSeries().size(), 2);
}

void SmartMoveGlobalInterpreterTest::testTranslatePositionFeedbackY()
{
    SmartMoveGlobalInterpreter globalInterpreter;

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    QCOMPARE(globalInterpreter.translatePositionFeedbackY(), "G RT_PFB_Y");

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    std::vector<std::shared_ptr<precitec::hardware::GlobalCommand>> globalCommandContainer;
    std::shared_ptr<precitec::hardware::PositionFeedbackBits> positionFeedback(new precitec::hardware::PositionFeedbackBits);

    globalCommandContainer.push_back(std::move(positionFeedback));

    globalInterpreter.translateGlobalCommands(globalCommandContainer);

    QVERIFY(!globalInterpreter.isCurrentCommandSeriesEmpty());

    QCOMPARE(globalInterpreter.currentCommandSeries().back(), "G RT_PFB_Y");
    QCOMPARE(globalInterpreter.currentCommandSeries().size(), 2);
}

void SmartMoveGlobalInterpreterTest::testTranslatePositionCommandX()
{
    SmartMoveGlobalInterpreter globalInterpreter;

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    QCOMPARE(globalInterpreter.translatePositionCommandX(), "G RT_CMD_X");

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    std::vector<std::shared_ptr<precitec::hardware::GlobalCommand>> globalCommandContainer;
    std::shared_ptr<precitec::hardware::PositionCommandBits> positionCommand(new precitec::hardware::PositionCommandBits);

    globalCommandContainer.push_back(std::move(positionCommand));

    globalInterpreter.translateGlobalCommands(globalCommandContainer);

    QVERIFY(!globalInterpreter.isCurrentCommandSeriesEmpty());

    QCOMPARE(globalInterpreter.currentCommandSeries().front(), "G RT_CMD_X");
    QCOMPARE(globalInterpreter.currentCommandSeries().size(), 2);
}

void SmartMoveGlobalInterpreterTest::testTranslatePositionCommandY()
{
    SmartMoveGlobalInterpreter globalInterpreter;

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    QCOMPARE(globalInterpreter.translatePositionCommandY(), "G RT_CMD_Y");

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    std::vector<std::shared_ptr<precitec::hardware::GlobalCommand>> globalCommandContainer;
    std::shared_ptr<precitec::hardware::PositionCommandBits> positionCommand(new precitec::hardware::PositionCommandBits);

    globalCommandContainer.push_back(std::move(positionCommand));

    globalInterpreter.translateGlobalCommands(globalCommandContainer);

    QVERIFY(!globalInterpreter.isCurrentCommandSeriesEmpty());

    QCOMPARE(globalInterpreter.currentCommandSeries().back(), "G RT_CMD_Y");
    QCOMPARE(globalInterpreter.currentCommandSeries().size(), 2);
}

void SmartMoveGlobalInterpreterTest::testTranslatePrefixSetOrGet()
{
    SmartMoveGlobalInterpreter globalInterpreter;

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());

    auto returnValue = globalInterpreter.translatePrefixSetOrGet(true);
    QCOMPARE(returnValue, "S ");
    returnValue = globalInterpreter.translatePrefixSetOrGet(false);
    QCOMPARE(returnValue, "G ");
    returnValue = globalInterpreter.translatePrefixSetOrGet(true);
    QCOMPARE(returnValue, "S ");
    returnValue = globalInterpreter.translatePrefixSetOrGet(false);
    QCOMPARE(returnValue, "G ");

    QVERIFY(globalInterpreter.isCurrentCommandSeriesEmpty());
}

void SmartMoveGlobalInterpreterTest::testCheckIfOnlySetCommands_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<std::vector<QString>>("multipleSetCommands");
    QTest::addColumn<int>("numberOfChars");
    QTest::addColumn<bool>("onlySetterCommands");

    QTest::newRow("OnlySets") << 0 << std::vector<QString> {"S SYS_OPMODE 0",
                                                     "S INPUT_MODE 10",
                                                     "S ALIGN_X 0",
                                                     "S ALIGN_Y 0"} << 51 << true;
    QTest::newRow("OnlyGets") << 1 << std::vector<QString> {"G RT_CMD_X",
                                                     "G RT_CMD_Y"} << 0 << false;
    QTest::newRow("OneGetAndSets") << 2 << std::vector<QString> {"G RT_CMD_X", "S SYS_OPMODE 0", "S INPUT_MODE 10", "S ALIGN_X 0", "S ALIGN_Y 0"} << 0 << false;
    QTest::newRow("SetsAndOneGet") << 3 << std::vector<QString> {"S SYS_OPMODE 0", "S INPUT_MODE 10", "S ALIGN_X 0", "S ALIGN_Y 0", "G RT_CMD_X"} << 51 << false;
}

void SmartMoveGlobalInterpreterTest::testCheckIfOnlySetCommands()
{
    SmartMoveGlobalInterpreter globalInterpreter;

    QCOMPARE(globalInterpreter.m_sizeForMultiSetMerge, 0);

    QFETCH(std::vector<QString>, multipleSetCommands);
    QFETCH(bool, onlySetterCommands);
    QFETCH(int, numberOfChars);

    std::vector<std::string> multipleSets;
    multipleSets.reserve(multipleSetCommands.size());

    for (const auto& setCommand : multipleSetCommands)
    {
        multipleSets.push_back(setCommand.toStdString());
    }

    QCOMPARE(globalInterpreter.checkIfOnlySetCommands(multipleSets), onlySetterCommands);
    QCOMPARE(globalInterpreter.m_sizeForMultiSetMerge, numberOfChars);
}

void SmartMoveGlobalInterpreterTest::testCheckCreateMultiSetCommand_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<std::vector<QString>>("multipleSetCommands");
    QTest::addColumn<int>("numberOfChars");
    QTest::addColumn<QString>("multiSetCommand");

    QTest::newRow("MultiSetAlignment") << 0 << std::vector<QString> {"S SYS_OPMODE 0",
                                                     "S INPUT_MODE 10",
                                                     "S ALIGN_X -30000",
                                                     "S ALIGN_Y 30000"} << 60 << "S SYS_OPMODE 0 INPUT_MODE 10 ALIGN_X -30000 ALIGN_Y 30000";
    QTest::newRow("MultiSetTooLong") << 1 << std::vector<QString> {"S SYS_OPMODE 1000000",
                                                     "S INPUT_MODE 10000000",
                                                     "S ALIGN_X -3000000000",
                                                     "S ALIGN_Y 30000000000"} << 80 << "";
    QTest::newRow("MultiSetTooMuchCommands") << 2 << std::vector<QString> {"S SYS_OPMODE 0",
                                                     "S INPUT_MODE 10",
                                                     "S ALIGN_X -30000",
                                                     "S ALIGN_Y 30000",
                                                     "S ALIGN_LASERON 8388607",
                                                     "S ALIGN_LASERON_MAX 8388607"} << 105 << "";
    QTest::newRow("MultiSetSpeeds") << 0 << std::vector<QString> {"S MV_JUMPSPEED 2500",
                                                     "S MV_MARKSPEED 1200"} << 37 << "S MV_JUMPSPEED 2500 MV_MARKSPEED 1200";
    QTest::newRow("MultiSetInit") << 0 << std::vector<QString> {"S SFC_FOCALLEN 460",
                                                     "S FIELD_SIZE 50"} << 32 << "S SFC_FOCALLEN 460 FIELD_SIZE 50";
}

void SmartMoveGlobalInterpreterTest::testCheckCreateMultiSetCommand()
{
    SmartMoveGlobalInterpreter globalInterpreter;

    QCOMPARE(globalInterpreter.m_sizeForMultiSetMerge, 0);

    QFETCH(std::vector<QString>, multipleSetCommands);
    QFETCH(QString, multiSetCommand);
    QFETCH(int, numberOfChars);

    globalInterpreter.m_sizeForMultiSetMerge = numberOfChars;

    std::vector<std::string> multipleSets;
    multipleSets.reserve(multipleSetCommands.size());

    for (const auto& setCommand : multipleSetCommands)
    {
        multipleSets.push_back(setCommand.toStdString());
    }

    QCOMPARE(QString::fromStdString(globalInterpreter.createMultiSetCommand(multipleSets)), multiSetCommand);
}

QTEST_GUILESS_MAIN(SmartMoveGlobalInterpreterTest)
#include "smartMoveGlobalInterpreterTest.moc"
