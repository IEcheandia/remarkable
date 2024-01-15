#include <QTest>
#include "../include/viWeldHead/Scanlab/smartMoveInterpreter.h"
#include "memory/memoryFile.h"

#include <QFile>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <unistd.h>

namespace
{
bool compareFiles(const std::string& file1, const std::string& file2)
{
    std::ifstream streamFile1(file1, std::ifstream::binary | std::ifstream::ate);
    std::ifstream streamFile2(file2, std::ifstream::binary | std::ifstream::ate);

    if (streamFile1.fail() || streamFile2.fail())
    {
        return false;
    }

    if (streamFile1.tellg() != streamFile2.tellg())
    {
        return false;
    }

    streamFile1.seekg(0, std::ifstream::beg);
    streamFile2.seekg(0, std::ifstream::beg);
    return std::equal(std::istreambuf_iterator<char>(streamFile1.rdbuf()), std::istreambuf_iterator<char>(), std::istreambuf_iterator<char>(streamFile2.rdbuf()));
}

}

using precitec::hardware::SmartMoveInterpreter;

/**
* Tests the code of the smart move interpreter.
**/
class SmartMoveInterpreterTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testCreateContourFile();
    void testFilename();
    void testFileDescriptor();
    void testDebug();
    void testTranslateInitialize();
    void testTranslateMark();
    void testTranslateJump();
    void testTranslateLaserPower();
    void testTranslateMarkSpeed();
    void testTranslateContour();
    void testNewContour();
    void testSaveHPGL2Contour();
    void testWriteAnonymousFile();
};

void SmartMoveInterpreterTest::testCtor()
{
    SmartMoveInterpreter interpreter;
    QVERIFY(interpreter.isCurrentContourEmpty());
    const auto& contour = interpreter.currentContour();
    QVERIFY(contour.empty());
    QCOMPARE(interpreter.fileDescriptor(), -1);
    QCOMPARE(interpreter.m_contourFileSize, 0);
}

void SmartMoveInterpreterTest::testFilename()
{
    SmartMoveInterpreter interpreter;
    QVERIFY(interpreter.filename().empty());
    std::string newFilename{"scannerJobID0.txt"};
    interpreter.setFilename(newFilename);
    QVERIFY(!interpreter.filename().empty());
    QCOMPARE(interpreter.filename(), newFilename);
}

void SmartMoveInterpreterTest::testCreateContourFile()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    QVERIFY(QDir{tmp.path()}.mkpath(QStringLiteral("config/hpgl2File/")));
    QDir dir{tmp.path()};
    QVERIFY(dir.exists());
    dir.mkpath(QStringLiteral("config/hpgl2File/"));
    QVERIFY(dir.exists("config/hpgl2File/"));
    QVERIFY(dir.cd("config/hpgl2File/"));

    auto testHPGL2File = QFINDTESTDATA(QStringLiteral("testData/HPGL2TestFile.txt"));
    QVERIFY( QFile::copy(testHPGL2File, dir.filePath(QStringLiteral("HPGL2TestFile.txt"))));

    qputenv("WM_BASE_DIR", tmp.path().toLocal8Bit());

    SmartMoveInterpreter interpreter;

    std::vector<std::shared_ptr<precitec::hardware::contour::Command>> contour;
    contour.reserve(7);
    std::shared_ptr<precitec::hardware::contour::Initialize> initialize(new precitec::hardware::contour::Initialize);
    contour.push_back(std::move(initialize));
    std::shared_ptr<precitec::hardware::contour::Jump> jump(new precitec::hardware::contour::Jump);
    jump->x = 150.5;
    jump->y = 50.0;
    contour.push_back(std::move(jump));
    std::shared_ptr<precitec::hardware::contour::LaserPower> laserPower(new precitec::hardware::contour::LaserPower);
    laserPower->power = 0.55;
    contour.push_back(std::move(laserPower));
    std::shared_ptr<precitec::hardware::contour::Mark> mark(new precitec::hardware::contour::Mark);
    mark->x = 175;
    mark->y = 75;
    contour.push_back(std::move(mark));
    std::shared_ptr<precitec::hardware::contour::Mark> mark2(new precitec::hardware::contour::Mark);
    mark2->x = 200;
    mark2->y = 50;
    contour.push_back(std::move(mark2));
    std::shared_ptr<precitec::hardware::contour::LaserPower> laserPower2(new precitec::hardware::contour::LaserPower);
    laserPower2->power = 0.75;
    contour.push_back(std::move(laserPower2));
    std::shared_ptr<precitec::hardware::contour::Mark> mark3(new precitec::hardware::contour::Mark);
    mark3->x = 150;
    mark3->y = 100;
    contour.push_back(std::move(mark3));

    QVERIFY(interpreter.isCurrentContourEmpty());
    QVERIFY(!interpreter.debug());
    interpreter.createContourFile(contour);
    QVERIFY(!interpreter.isCurrentContourEmpty());

    auto fd = interpreter.fileDescriptor();
    QVERIFY(fd != -1);
    std::string data;
    for (const auto& element : interpreter.currentContour())
    {
        std::size_t returnValue{0};
        data.resize(element.size());

        returnValue = read(fd, data.data(), element.size());
        QCOMPARE(returnValue, element.size());
        QCOMPARE(data, element);
    }

    dir.setFilter(QDir::Files);
    const auto& listBeforeDebug = dir.entryInfoList();
    QCOMPARE(listBeforeDebug.size(), 1);

    interpreter.newContour();
    QVERIFY(interpreter.isCurrentContourEmpty());
    QVERIFY(interpreter.fileDescriptor() == -1);

    interpreter.setDebug(true);
    QVERIFY(interpreter.debug());

    interpreter.createContourFile(contour);
    QVERIFY(!interpreter.isCurrentContourEmpty());

    fd = interpreter.fileDescriptor();
    QVERIFY(fd != -1);
    for (const auto& element : interpreter.currentContour())
    {
        std::size_t returnValue{0};
        data.resize(element.size());

        returnValue = read(fd, data.data(), element.size());
        QCOMPARE(returnValue, element.size());
        QCOMPARE(data, element);
    }

    dir.setFilter(QDir::Files);
    const auto& listWithDebug = dir.entryInfoList();
    QCOMPARE(listWithDebug.size(), 1);

    interpreter.newContour();
    QVERIFY(interpreter.isCurrentContourEmpty());
    QVERIFY(interpreter.fileDescriptor() == -1);

    interpreter.setFilename(std::string(getenv("WM_BASE_DIR")) + "/config/hpgl2File/" + "createdHPGL2File.txt");

    interpreter.createContourFile(contour);
    QVERIFY(!interpreter.isCurrentContourEmpty());

    fd = interpreter.fileDescriptor();
    QVERIFY(fd != -1);
    for (const auto& element : interpreter.currentContour())
    {
        std::size_t returnValue{0};
        data.resize(element.size());

        returnValue = read(fd, data.data(), element.size());
        QCOMPARE(returnValue, element.size());
        QCOMPARE(data, element);
    }

    dir.setFilter(QDir::Files);
    const auto& list = dir.entryInfoList();
    QCOMPARE(list.size(), 2);

    QVERIFY(compareFiles(list.at(0).absoluteFilePath().toStdString(), list.at(1).absoluteFilePath().toStdString()));
}

void SmartMoveInterpreterTest::testFileDescriptor()
{
    SmartMoveInterpreter interpreter;
    QCOMPARE(interpreter.fileDescriptor(), -1);

    interpreter.writeAnonymousFile();
    QCOMPARE(interpreter.fileDescriptor(), -1);
}

void SmartMoveInterpreterTest::testDebug()
{
    SmartMoveInterpreter interpreter;
    QVERIFY(!interpreter.debug());

    interpreter.setDebug(false);
    QVERIFY(!interpreter.debug());
    interpreter.setDebug(true);
    QVERIFY(interpreter.debug());
    interpreter.setDebug(false);
    QVERIFY(!interpreter.debug());
    interpreter.setDebug(true);
    QVERIFY(interpreter.debug());
}

void SmartMoveInterpreterTest::testTranslateInitialize()
{
    SmartMoveInterpreter interpreter;

    std::string initialize;
    initialize = interpreter.translateInitialize();
    QCOMPARE(initialize, "IN;");
    interpreter.m_stringstream.str("");
    initialize = interpreter.translateInitialize();
    QCOMPARE(initialize, "IN;");
}

void SmartMoveInterpreterTest::testTranslateMark()
{
    SmartMoveInterpreter interpreter;

    std::string mark;
    mark = interpreter.translateMark(1.0, -2.4);
    QCOMPARE(mark, "PD1,-2.4;");
    interpreter.m_stringstream.str("");
    mark = interpreter.translateMark(0.25, 1.333);
    QCOMPARE(mark, "PD0.25,1.333;");
    interpreter.m_stringstream.str("");
    mark = interpreter.translateMark(-0.001, -1.099);
    QCOMPARE(mark, "PD-0.001,-1.099;");
}

void SmartMoveInterpreterTest::testTranslateJump()
{
    SmartMoveInterpreter interpreter;

    std::string jump;
    jump = interpreter.translateJump(1.0, -2.4);
    QCOMPARE(jump, "PU1,-2.4;");
    interpreter.m_stringstream.str("");
    jump = interpreter.translateJump(0.25, 1.333);
    QCOMPARE(jump, "PU0.25,1.333;");
    interpreter.m_stringstream.str("");
    jump = interpreter.translateJump(-0.001, -1.099);
    QCOMPARE(jump, "PU-0.001,-1.099;");
}

void SmartMoveInterpreterTest::testTranslateLaserPower()
{
    SmartMoveInterpreter interpreter;

    std::string laserPower;
    laserPower = interpreter.translateLaserPower(1.0);
    QCOMPARE(laserPower, "PW1;");
    interpreter.m_stringstream.str("");
    laserPower = interpreter.translateLaserPower(0.45);
    QCOMPARE(laserPower, "PW0.45;");
    interpreter.m_stringstream.str("");
    laserPower = interpreter.translateLaserPower(0.005);
    QCOMPARE(laserPower, "PW0.005;");
}

void SmartMoveInterpreterTest::testTranslateMarkSpeed()
{
    SmartMoveInterpreter interpreter;

    std::string markSpeed;
    markSpeed = interpreter.translateMarkSpeed(500.0);
    QCOMPARE(markSpeed, "VS500;");
    interpreter.m_stringstream.str("");
    markSpeed = interpreter.translateMarkSpeed(250.0);
    QCOMPARE(markSpeed, "VS250;");
    interpreter.m_stringstream.str("");
    markSpeed = interpreter.translateMarkSpeed(1000.0);
    QCOMPARE(markSpeed, "VS1000;");
}

void SmartMoveInterpreterTest::testTranslateContour()
{
    SmartMoveInterpreter interpreter;

    std::vector<std::shared_ptr<precitec::hardware::contour::Command>> contour;
    contour.reserve(7);
    std::shared_ptr<precitec::hardware::contour::Initialize> initialize(new precitec::hardware::contour::Initialize);
    contour.push_back(std::move(initialize));
    std::shared_ptr<precitec::hardware::contour::Jump> jump(new precitec::hardware::contour::Jump);
    jump->x = 150.5;
    jump->y = 50.0;
    contour.push_back(std::move(jump));
    std::shared_ptr<precitec::hardware::contour::LaserPower> laserPower(new precitec::hardware::contour::LaserPower);
    laserPower->power = 0.55;
    contour.push_back(std::move(laserPower));
    std::shared_ptr<precitec::hardware::contour::Mark> mark(new precitec::hardware::contour::Mark);
    mark->x = 175;
    mark->y = 75;
    contour.push_back(std::move(mark));
    std::shared_ptr<precitec::hardware::contour::Mark> mark2(new precitec::hardware::contour::Mark);
    mark2->x = 200;
    mark2->y = 50;
    contour.push_back(std::move(mark2));
    std::shared_ptr<precitec::hardware::contour::LaserPower> laserPower2(new precitec::hardware::contour::LaserPower);
    laserPower2->power = 0.75;
    contour.push_back(std::move(laserPower2));
    std::shared_ptr<precitec::hardware::contour::Mark> mark3(new precitec::hardware::contour::Mark);
    mark3->x = 150;
    mark3->y = 100;
    contour.push_back(std::move(mark3));

    QVERIFY(interpreter.isCurrentContourEmpty());
    interpreter.translateContour(contour);
    QVERIFY(!interpreter.isCurrentContourEmpty());

    const auto& hpgl2Contour = interpreter.currentContour();
    QCOMPARE(hpgl2Contour.size(), contour.size());

    QCOMPARE(hpgl2Contour.at(0), "IN;");
    QCOMPARE(hpgl2Contour.at(1), "PU150.500,50.000;");
    QCOMPARE(hpgl2Contour.at(2), "PW0.550;");
    QCOMPARE(hpgl2Contour.at(3), "PD175.000,75.000;");
    QCOMPARE(hpgl2Contour.at(4), "PD200.000,50.000;");
    QCOMPARE(hpgl2Contour.at(5), "PW0.750;");
    QCOMPARE(hpgl2Contour.at(6), "PD150.000,100.000;");
}

void SmartMoveInterpreterTest::testNewContour()
{
    SmartMoveInterpreter interpreter;

    std::vector<std::shared_ptr<precitec::hardware::contour::Command>> contour;
    std::unique_ptr<precitec::hardware::contour::Initialize> initialize(new precitec::hardware::contour::Initialize);
    contour.push_back(std::move(initialize));

    QVERIFY(interpreter.isCurrentContourEmpty());
    interpreter.translateContour(contour);
    interpreter.writeAnonymousFile();
    QVERIFY(!interpreter.isCurrentContourEmpty());
    QVERIFY(interpreter.fileDescriptor() != -1);
    QCOMPARE(interpreter.m_contourFileSize, 3);
    interpreter.newContour();
    QVERIFY(interpreter.isCurrentContourEmpty());
    QCOMPARE(interpreter.m_contourFileSize, 0);
    QCOMPARE(interpreter.fileDescriptor(), -1);
}

void SmartMoveInterpreterTest::testSaveHPGL2Contour()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    QVERIFY(QDir{tmp.path()}.mkpath(QStringLiteral("config/hpgl2File/")));
    QDir dir{tmp.path()};
    QVERIFY(dir.exists());
    dir.mkpath(QStringLiteral("config/hpgl2File/"));
    QVERIFY(dir.exists("config/hpgl2File/"));
    QVERIFY(dir.cd("config/hpgl2File/"));

    auto testHPGL2File = QFINDTESTDATA(QStringLiteral("testData/HPGL2TestFile.txt"));
    QVERIFY( QFile::copy(testHPGL2File, dir.filePath(QStringLiteral("HPGL2TestFile.txt"))));

    qputenv("WM_BASE_DIR", tmp.path().toLocal8Bit());

    SmartMoveInterpreter interpreter;

    std::vector<std::shared_ptr<precitec::hardware::contour::Command>> contour;
    contour.reserve(7);
    std::shared_ptr<precitec::hardware::contour::Initialize> initialize(new precitec::hardware::contour::Initialize);
    contour.push_back(std::move(initialize));
    std::shared_ptr<precitec::hardware::contour::Jump> jump(new precitec::hardware::contour::Jump);
    jump->x = 150.5;
    jump->y = 50.0;
    contour.push_back(std::move(jump));
    std::shared_ptr<precitec::hardware::contour::LaserPower> laserPower(new precitec::hardware::contour::LaserPower);
    laserPower->power = 0.55;
    contour.push_back(std::move(laserPower));
    std::shared_ptr<precitec::hardware::contour::Mark> mark(new precitec::hardware::contour::Mark);
    mark->x = 175;
    mark->y = 75;
    contour.push_back(std::move(mark));
    std::shared_ptr<precitec::hardware::contour::Mark> mark2(new precitec::hardware::contour::Mark);
    mark2->x = 200;
    mark2->y = 50;
    contour.push_back(std::move(mark2));
    std::shared_ptr<precitec::hardware::contour::LaserPower> laserPower2(new precitec::hardware::contour::LaserPower);
    laserPower2->power = 0.75;
    contour.push_back(std::move(laserPower2));
    std::shared_ptr<precitec::hardware::contour::Mark> mark3(new precitec::hardware::contour::Mark);
    mark3->x = 150;
    mark3->y = 100;
    contour.push_back(std::move(mark3));

    QVERIFY(interpreter.isCurrentContourEmpty());
    interpreter.translateContour(contour);
    QVERIFY(!interpreter.isCurrentContourEmpty());

    interpreter.setFilename(std::string(getenv("WM_BASE_DIR")) + "/config/hpgl2File/" + "createdHPGL2File.txt");
    interpreter.saveHPGL2Contour();

    dir.setFilter(QDir::Files);
    const auto& list = dir.entryInfoList();
    QCOMPARE(list.size(), 2);
    QVERIFY(compareFiles(list.at(0).absoluteFilePath().toStdString(), list.at(1).absoluteFilePath().toStdString()));
}

void SmartMoveInterpreterTest::testWriteAnonymousFile()
{
    SmartMoveInterpreter interpreter;

    std::vector<std::shared_ptr<precitec::hardware::contour::Command>> contour;
    contour.reserve(7);
    std::shared_ptr<precitec::hardware::contour::Initialize> initialize(new precitec::hardware::contour::Initialize);
    contour.push_back(std::move(initialize));
    std::shared_ptr<precitec::hardware::contour::Jump> jump(new precitec::hardware::contour::Jump);
    jump->x = 150.5;
    jump->y = 50.0;
    contour.push_back(std::move(jump));
    std::shared_ptr<precitec::hardware::contour::LaserPower> laserPower(new precitec::hardware::contour::LaserPower);
    laserPower->power = 0.55;
    contour.push_back(std::move(laserPower));
    std::shared_ptr<precitec::hardware::contour::Mark> mark(new precitec::hardware::contour::Mark);
    mark->x = 175;
    mark->y = 75;
    contour.push_back(std::move(mark));
    std::shared_ptr<precitec::hardware::contour::Mark> mark2(new precitec::hardware::contour::Mark);
    mark2->x = 200;
    mark2->y = 50;
    contour.push_back(std::move(mark2));
    std::shared_ptr<precitec::hardware::contour::LaserPower> laserPower2(new precitec::hardware::contour::LaserPower);
    laserPower2->power = 0.75;
    contour.push_back(std::move(laserPower2));
    std::shared_ptr<precitec::hardware::contour::Mark> mark3(new precitec::hardware::contour::Mark);
    mark3->x = 150;
    mark3->y = 100;
    contour.push_back(std::move(mark3));

    QVERIFY(interpreter.isCurrentContourEmpty());
    interpreter.translateContour(contour);
    QVERIFY(!interpreter.isCurrentContourEmpty());

    interpreter.writeAnonymousFile();
    auto fd = interpreter.fileDescriptor();
    QVERIFY(fd != -1);

    std::string data;
    for (const auto& element : interpreter.currentContour())
    {
        std::size_t returnValue{0};
        data.resize(element.size());

        returnValue = read(fd, data.data(), element.size());
        QCOMPARE(returnValue, element.size());
        QCOMPARE(data, element);
    }
}

QTEST_GUILESS_MAIN(SmartMoveInterpreterTest)
#include "smartMoveInterpreterTest.moc"
