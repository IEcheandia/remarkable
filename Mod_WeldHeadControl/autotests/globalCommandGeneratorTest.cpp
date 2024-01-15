#include <QTest>

#include "../include/viWeldHead/Scanlab/globalCommandGenerator.h"

using precitec::hardware::GlobalCommandGenerator;

/**
 *  This class is used to check the global command generator.
 *  Global commands are used to generate scanner specific commands
 *  to execute specific tasks like moving or configure the scanner.
 **/
class GlobalCommandGeneratorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testAddProcessJob();
    void testAddJobSelect();
    void testAddSystemOperationMode();
    void testAddInputMode();
    void testAddAlign();
    void testAddLaserOnForAlignment();
    void testAddCalibrationFilename();
    void testAddFocalLength();
    void testAddScanfieldSize();
    void testAddJumpSpeed();
    void testAddMarkSpeed();
    void testAddPositionFeedBack();
    void testAddPositionCommand();
    void testReset();
    void testJumpToPosition();
};

void GlobalCommandGeneratorTest::testCtor()
{
    GlobalCommandGenerator generator;

    QVERIFY(generator.empty());
    const auto& generatedGlobalCommands = generator.generatedGlobalCommands();
    QVERIFY(generatedGlobalCommands.empty());
}

void GlobalCommandGeneratorTest::testAddProcessJob()
{
    GlobalCommandGenerator generator;

    QVERIFY(generator.empty());
    generator.addProcessJob(precitec::smartMove::JobRepeats::Infinite);
    QVERIFY(!generator.empty());
    generator.addProcessJob(precitec::smartMove::JobRepeats::SingleShot);
    generator.addProcessJob(precitec::smartMove::JobRepeats::SingleShot);

    QVERIFY(!generator.empty());
    QCOMPARE(generator.generatedGlobalCommands().size(), 3);

    const auto& generatedGlobalCommands = generator.generatedGlobalCommands();
    const auto& firstCommand = std::dynamic_pointer_cast<precitec::hardware::ProcessJob>(generatedGlobalCommands.at(0));
    QVERIFY(firstCommand->set);
    QCOMPARE(firstCommand->repeats, -1);
    const auto& secondCommand = std::dynamic_pointer_cast<precitec::hardware::ProcessJob>(generatedGlobalCommands.at(1));
    QVERIFY(secondCommand->set);
    QCOMPARE(secondCommand->repeats, 1);
    const auto& thirdCommand = std::dynamic_pointer_cast<precitec::hardware::ProcessJob>(generatedGlobalCommands.at(2));
    QVERIFY(thirdCommand->set);
    QCOMPARE(thirdCommand->repeats, 1);
}

void GlobalCommandGeneratorTest::testAddJobSelect()
{
    GlobalCommandGenerator generator;

    QVERIFY(generator.empty());
    generator.addJobSelect(1);
    QVERIFY(!generator.empty());

    generator.addJobSelect(10);
    generator.addJobSelect(15);

    QVERIFY(!generator.empty());
    QCOMPARE(generator.generatedGlobalCommands().size(), 3);

    const auto& generatedGlobalCommands = generator.generatedGlobalCommands();
    const auto& firstCommand = std::dynamic_pointer_cast<precitec::hardware::JobSelect>(generatedGlobalCommands.at(0));
    QVERIFY(firstCommand->set);
    QCOMPARE(firstCommand->jobID, 1);
    const auto& secondCommand = std::dynamic_pointer_cast<precitec::hardware::JobSelect>(generatedGlobalCommands.at(1));
    QVERIFY(secondCommand->set);
    QCOMPARE(secondCommand->jobID, 10);
    const auto& thirdCommand = std::dynamic_pointer_cast<precitec::hardware::JobSelect>(generatedGlobalCommands.at(2));
    QVERIFY(thirdCommand->set);
    QCOMPARE(thirdCommand->jobID, 15);
}

void GlobalCommandGeneratorTest::testAddSystemOperationMode()
{
    GlobalCommandGenerator generator;

    QVERIFY(generator.empty());
    generator.addSystemOperationMode(SystemOperationModes::Service, true);
    QVERIFY(!generator.empty());

    generator.addSystemOperationMode(SystemOperationModes::Marking, true);
    generator.addSystemOperationMode(SystemOperationModes::Marking, false);

    QVERIFY(!generator.empty());
    QCOMPARE(generator.generatedGlobalCommands().size(), 3);

    const auto& generatedGlobalCommands = generator.generatedGlobalCommands();
    const auto& firstCommand = std::dynamic_pointer_cast<precitec::hardware::SystemOperationMode>(generatedGlobalCommands.at(0));
    QVERIFY(firstCommand->set);
    QCOMPARE(static_cast<int>(firstCommand->sysOpMode), static_cast<int>(SystemOperationModes::Service));
    const auto& secondCommand = std::dynamic_pointer_cast<precitec::hardware::SystemOperationMode>(generatedGlobalCommands.at(1));
    QVERIFY(secondCommand->set);
    QCOMPARE(static_cast<int>(secondCommand->sysOpMode), static_cast<int>(SystemOperationModes::Marking));
    const auto& thirdCommand = std::dynamic_pointer_cast<precitec::hardware::SystemOperationMode>(generatedGlobalCommands.at(2));
    QVERIFY(!thirdCommand->set);
    QCOMPARE(static_cast<int>(thirdCommand->sysOpMode), static_cast<int>(SystemOperationModes::Marking));
}

void GlobalCommandGeneratorTest::testAddInputMode()
{
    GlobalCommandGenerator generator;

    QVERIFY(generator.empty());
    generator.addInputMode(InputModes::Alignment, true);
    QVERIFY(!generator.empty());

    generator.addInputMode(InputModes::WaveformGenerator, true);
    generator.addInputMode(InputModes::Calibration, false);

    QVERIFY(!generator.empty());
    QCOMPARE(generator.generatedGlobalCommands().size(), 3);

    const auto& generatedGlobalCommands = generator.generatedGlobalCommands();
    const auto& firstCommand = std::dynamic_pointer_cast<precitec::hardware::InputMode>(generatedGlobalCommands.at(0));
    QVERIFY(firstCommand->set);
    QCOMPARE(static_cast<int>(firstCommand->inputMode), static_cast<int>(InputModes::Alignment));
    const auto& secondCommand = std::dynamic_pointer_cast<precitec::hardware::InputMode>(generatedGlobalCommands.at(1));
    QVERIFY(secondCommand->set);
    QCOMPARE(static_cast<int>(secondCommand->inputMode), static_cast<int>(InputModes::WaveformGenerator));
    const auto& thirdCommand = std::dynamic_pointer_cast<precitec::hardware::InputMode>(generatedGlobalCommands.at(2));
    QVERIFY(!thirdCommand->set);
    QCOMPARE(static_cast<int>(thirdCommand->inputMode), static_cast<int>(InputModes::Calibration));
}

void GlobalCommandGeneratorTest::testAddAlign()
{
    GlobalCommandGenerator generator;

    QVERIFY(generator.empty());
    generator.addAlign(std::make_pair(0, 0), true);
    QVERIFY(!generator.empty());

    generator.addAlign(std::make_pair(1000, 1000), true);
    generator.addAlign(std::make_pair(-1000, -1000), false);

    QVERIFY(!generator.empty());
    QCOMPARE(generator.generatedGlobalCommands().size(), 3);

    const auto& generatedGlobalCommands = generator.generatedGlobalCommands();
    const auto& firstCommand = std::dynamic_pointer_cast<precitec::hardware::Align>(generatedGlobalCommands.at(0));
    QVERIFY(firstCommand->set);
    QCOMPARE(firstCommand->x, 0);
    QCOMPARE(firstCommand->y, 0);
    const auto& secondCommand = std::dynamic_pointer_cast<precitec::hardware::Align>(generatedGlobalCommands.at(1));
    QVERIFY(secondCommand->set);
    QCOMPARE(secondCommand->x, 1000);
    QCOMPARE(secondCommand->y, 1000);
    const auto& thirdCommand = std::dynamic_pointer_cast<precitec::hardware::Align>(generatedGlobalCommands.at(2));
    QVERIFY(!thirdCommand->set);
    QCOMPARE(thirdCommand->x, -1000);
    QCOMPARE(thirdCommand->y, -1000);
}

void GlobalCommandGeneratorTest::testAddLaserOnForAlignment()
{
    GlobalCommandGenerator generator;

    QVERIFY(generator.empty());
    generator.addLaserOnForAlignment(std::make_pair(0, 0), true);
    QVERIFY(!generator.empty());

    generator.addLaserOnForAlignment(std::make_pair(1000, 1000), true);
    generator.addLaserOnForAlignment(std::make_pair(200, 200), false);

    QVERIFY(!generator.empty());
    QCOMPARE(generator.generatedGlobalCommands().size(), 3);

    const auto& generatedGlobalCommands = generator.generatedGlobalCommands();
    const auto& firstCommand = std::dynamic_pointer_cast<precitec::hardware::LaserOnForAlignment>(generatedGlobalCommands.at(0));
    QVERIFY(firstCommand->set);
    QCOMPARE(firstCommand->laserOn, 0);
    QCOMPARE(firstCommand->laserOnMax, 0);
    const auto& secondCommand = std::dynamic_pointer_cast<precitec::hardware::LaserOnForAlignment>(generatedGlobalCommands.at(1));
    QVERIFY(secondCommand->set);
    QCOMPARE(secondCommand->laserOn, 1000);
    QCOMPARE(secondCommand->laserOnMax, 1000);
    const auto& thirdCommand = std::dynamic_pointer_cast<precitec::hardware::LaserOnForAlignment>(generatedGlobalCommands.at(2));
    QVERIFY(!thirdCommand->set);
    QCOMPARE(thirdCommand->laserOn, 200);
    QCOMPARE(thirdCommand->laserOnMax, 200);
}

void GlobalCommandGeneratorTest::testAddCalibrationFilename()
{
    GlobalCommandGenerator generator;

    QVERIFY(generator.empty());
    generator.addCalibrationFilename();
    QVERIFY(!generator.empty());

    generator.addCalibrationFilename();
    generator.addCalibrationFilename();

    QVERIFY(!generator.empty());
    QCOMPARE(generator.generatedGlobalCommands().size(), 3);

    const auto& generatedGlobalCommands = generator.generatedGlobalCommands();
    const auto& firstCommand = std::dynamic_pointer_cast<precitec::hardware::CalibrationFilename>(generatedGlobalCommands.at(0));
    QVERIFY(!firstCommand->set);
    const auto& secondCommand = std::dynamic_pointer_cast<precitec::hardware::CalibrationFilename>(generatedGlobalCommands.at(1));
    QVERIFY(!secondCommand->set);
    const auto& thirdCommand = std::dynamic_pointer_cast<precitec::hardware::CalibrationFilename>(generatedGlobalCommands.at(2));
    QVERIFY(!thirdCommand->set);
}

void GlobalCommandGeneratorTest::testAddFocalLength()
{
    GlobalCommandGenerator generator;

    QVERIFY(generator.empty());
    generator.addFocalLength(10, true);
    QVERIFY(!generator.empty());

    generator.addFocalLength(0.340, false);
    generator.addFocalLength(0.460, true);

    QVERIFY(!generator.empty());
    QCOMPARE(generator.generatedGlobalCommands().size(), 3);

    const auto& generatedGlobalCommands = generator.generatedGlobalCommands();
    const auto& firstCommand = std::dynamic_pointer_cast<precitec::hardware::FocalLength>(generatedGlobalCommands.at(0));
    QVERIFY(firstCommand->set);
    QCOMPARE(firstCommand->focalLength, 10);
    const auto& secondCommand = std::dynamic_pointer_cast<precitec::hardware::FocalLength>(generatedGlobalCommands.at(1));
    QVERIFY(!secondCommand->set);
    QCOMPARE(secondCommand->focalLength, 0.340);
    const auto& thirdCommand = std::dynamic_pointer_cast<precitec::hardware::FocalLength>(generatedGlobalCommands.at(2));
    QVERIFY(thirdCommand->set);
    QCOMPARE(thirdCommand->focalLength, 0.460);
}

void GlobalCommandGeneratorTest::testAddScanfieldSize()
{
    GlobalCommandGenerator generator;

    QVERIFY(generator.empty());
    generator.addScanfieldSize(10, true);
    QVERIFY(!generator.empty());

    generator.addScanfieldSize(0.5, true);
    generator.addScanfieldSize(1.125, false);

    QVERIFY(!generator.empty());
    QCOMPARE(generator.generatedGlobalCommands().size(), 3);

    const auto& generatedGlobalCommands = generator.generatedGlobalCommands();
    const auto& firstCommand = std::dynamic_pointer_cast<precitec::hardware::ScanfieldSize>(generatedGlobalCommands.at(0));
    QVERIFY(firstCommand->set);
    QCOMPARE(firstCommand->scanfieldSize, 10);
    const auto& secondCommand = std::dynamic_pointer_cast<precitec::hardware::ScanfieldSize>(generatedGlobalCommands.at(1));
    QVERIFY(secondCommand->set);
    QCOMPARE(secondCommand->scanfieldSize, 0.5);
    const auto& thirdCommand = std::dynamic_pointer_cast<precitec::hardware::ScanfieldSize>(generatedGlobalCommands.at(2));
    QVERIFY(!thirdCommand->set);
    QCOMPARE(thirdCommand->scanfieldSize, 1.125);
}

void GlobalCommandGeneratorTest::testAddJumpSpeed()
{
    GlobalCommandGenerator generator;

    QVERIFY(generator.empty());
    generator.addJumpSpeed(12.0, true);
    QVERIFY(!generator.empty());

    generator.addJumpSpeed(7.5, true);
    generator.addJumpSpeed(2.05, false);

    QVERIFY(!generator.empty());
    QCOMPARE(generator.generatedGlobalCommands().size(), 3);

    const auto& generatedGlobalCommands = generator.generatedGlobalCommands();
    const auto& firstCommand = std::dynamic_pointer_cast<precitec::hardware::JumpSpeed>(generatedGlobalCommands.at(0));
    QVERIFY(firstCommand->set);
    QCOMPARE(firstCommand->speed, 12.0);
    const auto& secondCommand = std::dynamic_pointer_cast<precitec::hardware::JumpSpeed>(generatedGlobalCommands.at(1));
    QVERIFY(secondCommand->set);
    QCOMPARE(secondCommand->speed, 7.5);
    const auto& thirdCommand = std::dynamic_pointer_cast<precitec::hardware::JumpSpeed>(generatedGlobalCommands.at(2));
    QVERIFY(!thirdCommand->set);
    QCOMPARE(thirdCommand->speed, 2.05);
}

void GlobalCommandGeneratorTest::testAddMarkSpeed()
{
    GlobalCommandGenerator generator;

    QVERIFY(generator.empty());
    generator.addMarkSpeed(12.0, true);
    QVERIFY(!generator.empty());

    generator.addMarkSpeed(7.5, true);
    generator.addMarkSpeed(2.05, false);

    QVERIFY(!generator.empty());
    QCOMPARE(generator.generatedGlobalCommands().size(), 3);

    const auto& generatedGlobalCommands = generator.generatedGlobalCommands();
    const auto& firstCommand = std::dynamic_pointer_cast<precitec::hardware::MarkSpeed>(generatedGlobalCommands.front());
    QVERIFY(firstCommand->set);
    QCOMPARE(firstCommand->speed, 12.0);
    const auto& secondCommand = std::dynamic_pointer_cast<precitec::hardware::MarkSpeed>(generatedGlobalCommands.at(1));
    QVERIFY(secondCommand->set);
    QCOMPARE(secondCommand->speed, 7.5);
    const auto& thirdCommand = std::dynamic_pointer_cast<precitec::hardware::MarkSpeed>(generatedGlobalCommands.back());
    QVERIFY(!thirdCommand->set);
    QCOMPARE(thirdCommand->speed, 2.05);
}

void GlobalCommandGeneratorTest::testAddPositionFeedBack()
{
    GlobalCommandGenerator generator;

    QVERIFY(generator.empty());
    generator.addPositionFeedback();
    QVERIFY(!generator.empty());

    generator.addPositionFeedback();
    generator.addPositionFeedback();

    QVERIFY(!generator.empty());
    QCOMPARE(generator.generatedGlobalCommands().size(), 3);

    const auto& generatedGlobalCommands = generator.generatedGlobalCommands();
    const auto& firstCommand = std::dynamic_pointer_cast<precitec::hardware::PositionFeedbackBits>(generatedGlobalCommands.front());
    QVERIFY(firstCommand);
    QVERIFY(firstCommand->set);
    const auto& secondCommand = std::dynamic_pointer_cast<precitec::hardware::PositionFeedbackBits>(generatedGlobalCommands.at(1));
    QVERIFY(secondCommand);
    QVERIFY(secondCommand->set);
    const auto& thirdCommand = std::dynamic_pointer_cast<precitec::hardware::PositionFeedbackBits>(generatedGlobalCommands.back());
    QVERIFY(thirdCommand);
    QVERIFY(thirdCommand->set);
}

void GlobalCommandGeneratorTest::testAddPositionCommand()
{
    GlobalCommandGenerator generator;

    QVERIFY(generator.empty());
    generator.addPositionCommand();
    QVERIFY(!generator.empty());

    generator.addPositionCommand();
    generator.addPositionCommand();

    QVERIFY(!generator.empty());
    QCOMPARE(generator.generatedGlobalCommands().size(), 3);

    const auto& generatedGlobalCommands = generator.generatedGlobalCommands();
    const auto& firstCommand = std::dynamic_pointer_cast<precitec::hardware::PositionCommandBits>(generatedGlobalCommands.front());
    QVERIFY(firstCommand);
    QVERIFY(firstCommand->set);
    const auto& secondCommand = std::dynamic_pointer_cast<precitec::hardware::PositionCommandBits>(generatedGlobalCommands.at(1));
    QVERIFY(secondCommand);
    QVERIFY(secondCommand->set);
    const auto& thirdCommand = std::dynamic_pointer_cast<precitec::hardware::PositionCommandBits>(generatedGlobalCommands.back());
    QVERIFY(thirdCommand);
    QVERIFY(thirdCommand->set);
}

void GlobalCommandGeneratorTest::testReset()
{
    GlobalCommandGenerator generator;

    QVERIFY(generator.empty());
    QCOMPARE(generator.generatedGlobalCommands().size(), 0);

    generator.addSystemOperationMode(SystemOperationModes::Service, true);
    generator.addInputMode(InputModes::Alignment, true);
    generator.addAlign(std::make_pair(0, 0), true);                 //Jump to position 0,0
    generator.addLaserOnForAlignment(std::make_pair(0, 0), true);

    QVERIFY(!generator.empty());
    QCOMPARE(generator.generatedGlobalCommands().size(), 4);

    generator.reset();
    QVERIFY(generator.empty());
    QCOMPARE(generator.generatedGlobalCommands().size(), 0);
}

void GlobalCommandGeneratorTest::testJumpToPosition()
{
    GlobalCommandGenerator generator;

    QVERIFY(generator.empty());
    generator.addSystemOperationMode(SystemOperationModes::Service, true);
    QVERIFY(!generator.empty());

    generator.addInputMode(InputModes::Alignment, true);
    generator.addAlign(std::make_pair(0, 0), true);                 //Jump to position 0,0
    generator.addLaserOnForAlignment(std::make_pair(0, 0), true);

    QVERIFY(!generator.empty());
    QCOMPARE(generator.generatedGlobalCommands().size(), 4);

    const auto& generatedGlobalCommands = generator.generatedGlobalCommands();
    const auto& firstCommand = std::dynamic_pointer_cast<precitec::hardware::SystemOperationMode>(generatedGlobalCommands.at(0));
    QVERIFY(firstCommand->set);
    QCOMPARE(static_cast<int>(firstCommand->sysOpMode), static_cast<int>(SystemOperationModes::Service));

    const auto& secondCommand = std::dynamic_pointer_cast<precitec::hardware::InputMode>(generatedGlobalCommands.at(1));
    QVERIFY(secondCommand->set);
    QCOMPARE(static_cast<int>(secondCommand->inputMode), static_cast<int>(InputModes::Alignment));

    const auto& thirdCommand = std::dynamic_pointer_cast<precitec::hardware::Align>(generatedGlobalCommands.at(2));
    QVERIFY(thirdCommand->set);
    QCOMPARE(thirdCommand->x, 0);
    QCOMPARE(thirdCommand->y, 0);

    const auto& fourthCommand = std::dynamic_pointer_cast<precitec::hardware::LaserOnForAlignment>(generatedGlobalCommands.at(3));
    QVERIFY(fourthCommand->set);
    QCOMPARE(fourthCommand->laserOn, 0);
    QCOMPARE(fourthCommand->laserOnMax, 0);

    generator.reset();
    QVERIFY(generator.empty());

    generator.addSystemOperationMode(SystemOperationModes::Service, true);
    generator.addInputMode(InputModes::Alignment, true);
    generator.addAlign(std::make_pair(-100, 100), true);                 //Jump to position -100,100 virtual bits, so the position depends on the scanfield size
    generator.addLaserOnForAlignment(std::make_pair(0, 0), true);

    QVERIFY(!generator.empty());
    QCOMPARE(generator.generatedGlobalCommands().size(), 4);

    const auto& generatedGlobalCommands2 = generator.generatedGlobalCommands();
    const auto& firstCommand2 = std::dynamic_pointer_cast<precitec::hardware::SystemOperationMode>(generatedGlobalCommands2.at(0));
    QVERIFY(firstCommand2->set);
    QCOMPARE(static_cast<int>(firstCommand2->sysOpMode), static_cast<int>(SystemOperationModes::Service));

    const auto& secondCommand2 = std::dynamic_pointer_cast<precitec::hardware::InputMode>(generatedGlobalCommands2.at(1));
    QVERIFY(secondCommand2->set);
    QCOMPARE(static_cast<int>(secondCommand2->inputMode), static_cast<int>(InputModes::Alignment));

    const auto& thirdCommand2 = std::dynamic_pointer_cast<precitec::hardware::Align>(generatedGlobalCommands2.at(2));
    QVERIFY(thirdCommand2->set);
    QCOMPARE(thirdCommand2->x, -100);
    QCOMPARE(thirdCommand2->y, 100);

    const auto& fourthCommand2 = std::dynamic_pointer_cast<precitec::hardware::LaserOnForAlignment>(generatedGlobalCommands2.at(3));
    QVERIFY(fourthCommand2->set);
    QCOMPARE(fourthCommand2->laserOn, 0);
    QCOMPARE(fourthCommand2->laserOnMax, 0);
}

QTEST_GUILESS_MAIN(GlobalCommandGeneratorTest)
#include "globalCommandGeneratorTest.moc"
