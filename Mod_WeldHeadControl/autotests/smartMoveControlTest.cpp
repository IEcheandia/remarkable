#include <QTest>
#include <memory>

#include "../include/viWeldHead/Scanlab/smartMoveControl.h"
#include "common/systemConfiguration.h"
#include "common/definesWeldingFigure.h"

#include "../include/viWeldHead/Scanlab/RTC6DataTypes.h"
#include "../include/viWeldHead/Scanlab/RTC6jsonSupport.h"
#include "../include/viWeldHead/Scanlab/lensData.h"
#include "module/moduleLogger.h"

namespace
{
RTC6::Figure figureFromFile(const std::string& filename)
{
    RTC6::Figure figure;
    try
    {
        nlohmann::ordered_json pointsFromFile;
        RTC6::readFromFile(pointsFromFile, filename);
        figure = pointsFromFile;
        return figure;
    }
    catch (...)
    {
        precitec::wmLog(precitec::LogType::eDebug, "File is missing\n");
        return {};
    }
}

std::vector<double> serializeFigurePoints(const RTC6::Figure& figure)
{
    std::vector<double> pointsSerialized;
    pointsSerialized.reserve(figure.figure.size() * precitec::weldingFigure::SEAMWELDING_RESULT_FIELDS_PER_POINT);
    for (const auto& point : figure.figure)
    {
        pointsSerialized.push_back(point.endPosition.first);
        pointsSerialized.push_back(point.endPosition.second);
        pointsSerialized.push_back(point.relativePower);
        pointsSerialized.push_back(point.relativeRingPower);
        pointsSerialized.push_back(point.velocity);
    }

    return pointsSerialized;
}

std::vector<double> createContourPointsFromFile(const std::string& filename)
{
    if (filename.empty())
    {
        return {};
    }

    const auto& figure = figureFromFile(filename);

    if (figure.figure.empty())
    {
        return {};
    }

    return serializeFigurePoints(figure);
}

std::string filePathWMBaseDir(const std::string& folderFromWeldmasterBaseDir)
{
    return std::string(getenv("WM_BASE_DIR")) + folderFromWeldmasterBaseDir;
}

std::vector<QString> convertVectorStdStringToQString(const std::vector<std::string>& vectorStdString)
{
    if (vectorStdString.empty())
    {
        return {};
    }
    std::vector<QString> qStringVector;
    qStringVector.reserve(vectorStdString.size());

    for (const auto& element : vectorStdString)
    {
        qStringVector.push_back(QString::fromStdString(element));
    }
    return qStringVector;
}
}

/**
 *  This class is used to check the SmartMove control class.
 **/
class SmartMoveControlTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testInit_data();
    void testInit();
    void testScannerDriveToPosition_data();
    void testScannerDriveToPosition();
    void testScannerDriveToZero_data();
    void testScannerDriveToZero();
    void testSetJumpSpeed_data();
    void testSetJumpSpeed();
    void testSetMarkSpeed_data();
    void testSetMarkSpeed();
    void testPrepareWeldingList_data();
    void testPrepareWeldingList();
    void testBuildPreviewList_data();
    void testBuildPreviewList();
    void testStartJob();
    void testTranslateSpecialValue();
    void testCorrectionFile();
    void testSendCorrectionFile();
    void testUploadCorrectionFile_data();
    void testUploadCorrectionFile();
private:
    QTemporaryDir m_dir;
};

void SmartMoveControlTest::initTestCase()
{
    QVERIFY(m_dir.isValid());
    QVERIFY(QDir{}.mkpath(m_dir.filePath(QStringLiteral("config"))));
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());
}

void SmartMoveControlTest::testCtor()
{
    std::unique_ptr<precitec::hardware::SmartMoveControl> m_markingEngineControl(new precitec::hardware::SmartMoveControl);
}

void SmartMoveControlTest::testInit_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QString>("correctionFile");
    QTest::addColumn<int>("lensType");
    QTest::addColumn<int>("scannerModel");
    QTest::addColumn<int>("scannerController");
    QTest::addColumn<int>("targetScanFieldSize");
    QTest::addColumn<std::vector<QString>>("targetCommandSeries");

    QTest::newRow("InitWithDefault340CorrectionFile") << 1 << QStringLiteral("") << 1 << 1 << 1 << 52 << std::vector<QString>{
                                                                                                    QStringLiteral("G SFC_NAME"),
                                                                                                    QStringLiteral("S FIELD_SIZE 0.052000"),
                                                                                                    QStringLiteral("S SFC_FOCALLEN 0.340000"),
                                                                                                    QStringLiteral("S FLOW_FORCEEN 1"),
                                                                                                    QStringLiteral("G SYS_TS")};
    QTest::newRow("InitWithDefault460CorrectionFile") << 2 << QStringLiteral("") << 2 << 1 << 1 << 145 << std::vector<QString>{
                                                                                                    QStringLiteral("G SFC_NAME"),
                                                                                                    QStringLiteral("S FIELD_SIZE 0.145000"),
                                                                                                    QStringLiteral("S SFC_FOCALLEN 0.460000"),
                                                                                                    QStringLiteral("S FLOW_FORCEEN 1"),
                                                                                                    QStringLiteral("G SYS_TS")};
    QTest::newRow("InitWithDefault255CorrectionFile") << 3 << QStringLiteral("") << 3 << 1 << 1 << 50 << std::vector<QString>{
                                                                                                    QStringLiteral("G SFC_NAME"),
                                                                                                    QStringLiteral("S FIELD_SIZE 0.050000"),
                                                                                                    QStringLiteral("S SFC_FOCALLEN 0.255000"),
                                                                                                    QStringLiteral("S FLOW_FORCEEN 1"),
                                                                                                    QStringLiteral("G SYS_TS")};
}

void SmartMoveControlTest::testInit()
{
    std::unique_ptr<precitec::hardware::SmartMoveControl> m_markingEngineControl(new precitec::hardware::SmartMoveControl);

    InitData data;

    QFETCH(QString, correctionFile);
    QFETCH(int, lensType);
    QFETCH(int, scannerModel);
    QFETCH(int, scannerController);

    data.ipAddress = "192.168.170.105";
    data.correctionFile = correctionFile.toStdString();
    data.lens = static_cast<precitec::interface::LensType>(lensType);
    data.scannerModel = static_cast<precitec::interface::ScannerModel>(scannerModel);
    data.scannerController = static_cast<precitec::interface::ScannerModel>(scannerController);

    QCOMPARE(m_markingEngineControl->init(data), -1);

    QTEST(m_markingEngineControl->m_calculator.scanfieldSize(), "targetScanFieldSize");

    const auto& commandSeries = m_markingEngineControl->m_globalInterpreter.currentCommandSeries();

    QCOMPARE(commandSeries.size(), 5);
    QTEST(convertVectorStdStringToQString(commandSeries), "targetCommandSeries");
}

void SmartMoveControlTest::testScannerDriveToPosition_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<double>("X");
    QTest::addColumn<double>("Y");
    QTest::addColumn<std::vector<QString>>("targetCommandSeries");

    QTest::newRow("DriveToPosition1") << 1 << 50.0 << 0.0 << std::vector<QString>{
                                                             QStringLiteral("S SYS_OPMODE 0"),
                                                             QStringLiteral("S INPUT_MODE 10"),
                                                             QStringLiteral("S ALIGN_X 32767"),
                                                             QStringLiteral("S ALIGN_Y 0")};
    QTest::newRow("DriveToPosition2") << 2 << 0.0 << 50.0 << std::vector<QString>{
                                                             QStringLiteral("S SYS_OPMODE 0"),
                                                             QStringLiteral("S INPUT_MODE 10"),
                                                             QStringLiteral("S ALIGN_X 0"),
                                                             QStringLiteral("S ALIGN_Y 32767")};
}

void SmartMoveControlTest::testScannerDriveToPosition()
{
    std::unique_ptr<precitec::hardware::SmartMoveControl> m_markingEngineControl(new precitec::hardware::SmartMoveControl);

    QFETCH(double, X);
    QFETCH(double, Y);
    m_markingEngineControl->scannerDriveToPosition(std::make_pair(X, Y));

    const auto& commandSeries = m_markingEngineControl->m_globalInterpreter.currentCommandSeries();

    QCOMPARE(commandSeries.size(), 4);
    QFETCH(std::vector<QString>, targetCommandSeries);

    const auto& commandSeriesQString = convertVectorStdStringToQString(commandSeries);
    QCOMPARE(commandSeriesQString, targetCommandSeries);
}

void SmartMoveControlTest::testScannerDriveToZero_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<std::vector<QString>>("targetCommandSeries");

    QTest::newRow("DriveToPosition1") << 1 << std::vector<QString>{
                                                             QStringLiteral("S SYS_OPMODE 0"),
                                                             QStringLiteral("S INPUT_MODE 10"),
                                                             QStringLiteral("S ALIGN_X 0"),
                                                             QStringLiteral("S ALIGN_Y 0")};
    QTest::newRow("DriveToPosition2") << 2 << std::vector<QString>{
                                                             QStringLiteral("S SYS_OPMODE 0"),
                                                             QStringLiteral("S INPUT_MODE 10"),
                                                             QStringLiteral("S ALIGN_X 0"),
                                                             QStringLiteral("S ALIGN_Y 0")};
}

void SmartMoveControlTest::testScannerDriveToZero()
{
    std::unique_ptr<precitec::hardware::SmartMoveControl> m_markingEngineControl(new precitec::hardware::SmartMoveControl);

    m_markingEngineControl->scannerDriveToZero();

    const auto& commandSeries = m_markingEngineControl->m_globalInterpreter.currentCommandSeries();

    QCOMPARE(commandSeries.size(), 4);
    QFETCH(std::vector<QString>, targetCommandSeries);

    const auto& commandSeriesQString = convertVectorStdStringToQString(commandSeries);
    QCOMPARE(commandSeriesQString, targetCommandSeries);
}

void SmartMoveControlTest::testSetJumpSpeed_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<double>("speedMMPerMS");
    QTest::addColumn<std::vector<QString>>("targetCommandSeries");

    QTest::newRow("SetJumpSpeed1") << 1 << 0.0 << std::vector<QString>{
                                                             QStringLiteral("S MV_JUMPSPEED 0.000000")};
}

void SmartMoveControlTest::testSetJumpSpeed()
{
    std::unique_ptr<precitec::hardware::SmartMoveControl> m_markingEngineControl(new precitec::hardware::SmartMoveControl);

    QFETCH(double, speedMMPerMS);
    m_markingEngineControl->setScannerJumpSpeed(speedMMPerMS);

    const auto& commandSeries = m_markingEngineControl->m_globalInterpreter.currentCommandSeries();

    QCOMPARE(commandSeries.size(), 1);
    QFETCH(std::vector<QString>, targetCommandSeries);

    const auto& commandSeriesQString = convertVectorStdStringToQString(commandSeries);
    QCOMPARE(commandSeriesQString, targetCommandSeries);
}

void SmartMoveControlTest::testSetMarkSpeed_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<double>("speedMMPerMS");
    QTest::addColumn<std::vector<QString>>("targetCommandSeries");

    QTest::newRow("SetMarkSpeed1") << 1 << 0.0 << std::vector<QString>{
                                                             QStringLiteral("S MV_MARKSPEED 0.000000")};
}

void SmartMoveControlTest::testSetMarkSpeed()
{
    std::unique_ptr<precitec::hardware::SmartMoveControl> m_markingEngineControl(new precitec::hardware::SmartMoveControl);

    QFETCH(double, speedMMPerMS);
    m_markingEngineControl->setScannerMarkSpeed(speedMMPerMS);

    const auto& commandSeries = m_markingEngineControl->m_globalInterpreter.currentCommandSeries();

    QCOMPARE(commandSeries.size(), 1);
    QFETCH(std::vector<QString>, targetCommandSeries);

    const auto& commandSeriesQString = convertVectorStdStringToQString(commandSeries);
    QCOMPARE(commandSeriesQString, targetCommandSeries);
}

void SmartMoveControlTest::testPrepareWeldingList_data()
{
    //TODO Add more cases to check.
    QTest::addColumn<int>("row");
    QTest::addColumn<std::vector<double>>("weldingData");

    QTest::newRow("RectangleAllValuesChange") << 1 << std::vector<double>{
                                                5.0, 2.5, 25.0, 30.0, 1000,
                                                7.5, 2.5, 30.0, 28.5, 1250,
                                                7.5, 5.0, 35.0, 25.0, 1300,
                                                5.0, 5.0, 40.0, 30.0, 1400,
                                                5.0, 2.5, 45.0, 20.0, 1500};
}

void SmartMoveControlTest::testPrepareWeldingList()
{
    std::unique_ptr<precitec::hardware::SmartMoveControl> m_markingEngineControl(new precitec::hardware::SmartMoveControl);

    precitec::weldingFigure::ProductValues defaultProductValues;
    defaultProductValues.laserPower = 25.0;
    defaultProductValues.laserPowerRing = 80.0;
    defaultProductValues.velocity = 1000;

    QVERIFY(m_markingEngineControl->m_currentPoints.empty());
    QFETCH(std::vector<double>, weldingData);

    m_markingEngineControl->prepareWeldingList(weldingData, defaultProductValues);

    QVERIFY(!m_markingEngineControl->m_currentPoints.empty());
    QCOMPARE(m_markingEngineControl->m_currentPoints.size(), weldingData.size() / precitec::weldingFigure::SEAMWELDING_RESULT_FIELDS_PER_POINT);

    for (std::size_t i = 0; i < m_markingEngineControl->m_currentPoints.size(); i++)
    {
        const auto& currentPoint = m_markingEngineControl->m_currentPoints.at(i);
        QCOMPARE(currentPoint.X, weldingData.at(i * precitec::weldingFigure::SEAMWELDING_RESULT_FIELDS_PER_POINT) + static_cast<int>(precitec::weldingFigure::SeamWeldingResultFields::X));
        QCOMPARE(currentPoint.Y, weldingData.at((i * precitec::weldingFigure::SEAMWELDING_RESULT_FIELDS_PER_POINT) + static_cast<int>(precitec::weldingFigure::SeamWeldingResultFields::Y)));
        QCOMPARE(currentPoint.laserPower, weldingData.at((i * precitec::weldingFigure::SEAMWELDING_RESULT_FIELDS_PER_POINT) + static_cast<int>(precitec::weldingFigure::SeamWeldingResultFields::Power)));
        QCOMPARE(currentPoint.laserPowerRing, weldingData.at((i * precitec::weldingFigure::SEAMWELDING_RESULT_FIELDS_PER_POINT) + static_cast<int>(precitec::weldingFigure::SeamWeldingResultFields::RingPower)));
        QCOMPARE(currentPoint.speed, weldingData.at((i * precitec::weldingFigure::SEAMWELDING_RESULT_FIELDS_PER_POINT) + static_cast<int>(precitec::weldingFigure::SeamWeldingResultFields::Velocity)));
    }
}

void SmartMoveControlTest::testBuildPreviewList_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("targetFilename");
    QTest::newRow("HorizontalLineWithTwoPoints") << 1 << QStringLiteral("weldingSeam1.json") << QStringLiteral("HPGL2WeldingSeam1.txt");
}

void SmartMoveControlTest::testBuildPreviewList()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    QVERIFY(QDir{tmp.path()}.mkpath(QStringLiteral("config/hpgl2File/")));
    QDir dir{tmp.path()};
    QVERIFY(dir.exists());
    dir.mkpath(QStringLiteral("config/hpgl2File/"));
    QVERIFY(dir.exists("config/hpgl2File/"));
    QVERIFY(dir.cd("config/hpgl2File/"));

    QFETCH(QString, filename);
    const auto& currentFigureFilename = QStringLiteral("testData/previewTestData/") + filename;
    auto currentFigureFile = QFINDTESTDATA(currentFigureFilename);
    QVERIFY(QFile::copy(currentFigureFile, dir.filePath(filename)));

    //NOTE Keep in mind that the target file has counts as unit for the position.
    QFETCH(QString, targetFilename);
    const auto& targetHPGL2Filename = QStringLiteral("testData/previewTestData/") + targetFilename;
    auto targetHPGL2File = QFINDTESTDATA(targetHPGL2Filename);
    QVERIFY(QFile::copy(targetHPGL2File, dir.filePath(targetFilename)));

    qputenv("WM_BASE_DIR", tmp.path().toLocal8Bit());

    std::unique_ptr<precitec::hardware::SmartMoveControl> m_markingEngineControl(new precitec::hardware::SmartMoveControl);

    precitec::weldingFigure::ProductValues defaultProductValues;
    defaultProductValues.laserPower = 25.0;
    defaultProductValues.laserPowerRing = 80.0;
    defaultProductValues.velocity = 1000;

    QVERIFY(m_markingEngineControl->m_currentPoints.empty());

    const auto& figureFileCopied = filePathWMBaseDir("/config/hpgl2File/") + filename.toStdString();
    const auto& serializedPoints = createContourPointsFromFile(figureFileCopied);
    m_markingEngineControl->prepareWeldingList(serializedPoints, defaultProductValues);
    QVERIFY(!m_markingEngineControl->m_currentPoints.empty());
    QCOMPARE(m_markingEngineControl->m_currentPoints.size(), serializedPoints.size() / precitec::weldingFigure::SEAMWELDING_RESULT_FIELDS_PER_POINT);

    const auto& pathForStoringGeneratedFile = filePathWMBaseDir("/config/hpgl2File/") + "currentHPGLFile.txt";
    m_markingEngineControl->m_interpreter.setFilename(pathForStoringGeneratedFile);
    m_markingEngineControl->m_interpreter.setDebug(true);
    m_markingEngineControl->buildPreviewList();
    QVERIFY(QFile::exists(QString::fromStdString(pathForStoringGeneratedFile)));

    const auto& filenameTargetFile = filePathWMBaseDir("/config/hpgl2File/") + "HPGL2WeldingSeam1.txt";
    QVERIFY(QFile::exists(QString::fromStdString(filenameTargetFile)));

    QFile currentFile(QString::fromStdString(pathForStoringGeneratedFile));
    QFile targetFile(QString::fromStdString(filenameTargetFile));
    if (currentFile.open(QIODevice::ReadOnly) && targetFile.open(QIODevice::ReadOnly))
    {
        QTextStream currentStream(&currentFile);
        QTextStream targetStream(&targetFile);
        while (!currentStream.atEnd() && !targetStream.atEnd())
        {
            const auto& currentLine = currentStream.readLine();
            const auto& targetLine = targetStream.readLine();
            QCOMPARE(currentLine, targetLine);
        }
        QVERIFY(currentStream.atEnd());
        QVERIFY(targetStream.atEnd());
        currentFile.close();
        targetFile.close();
    }
}

void SmartMoveControlTest::testStartJob()
{
    std::unique_ptr<precitec::hardware::SmartMoveControl> m_markingEngineControl(new precitec::hardware::SmartMoveControl);

    QVERIFY(m_markingEngineControl->m_globalGenerator.empty());
    QVERIFY(m_markingEngineControl->m_globalInterpreter.isCurrentCommandSeriesEmpty());

    m_markingEngineControl->startJob(JobRepeats::SingleShot);

    QVERIFY(!m_markingEngineControl->m_globalGenerator.empty());
    QVERIFY(!m_markingEngineControl->m_globalInterpreter.isCurrentCommandSeriesEmpty());

    auto currentCommandSeries = m_markingEngineControl->m_globalInterpreter.currentCommandSeries();

    QCOMPARE(currentCommandSeries.size(), 2);
    QCOMPARE(currentCommandSeries.front(), "S SYS_OPMODE 1");
    QCOMPARE(currentCommandSeries.back(), "P 1");

    m_markingEngineControl->startJob(JobRepeats::Stop);

    QVERIFY(!m_markingEngineControl->m_globalGenerator.empty());
    QVERIFY(!m_markingEngineControl->m_globalInterpreter.isCurrentCommandSeriesEmpty());

    currentCommandSeries = m_markingEngineControl->m_globalInterpreter.currentCommandSeries();

    QCOMPARE(currentCommandSeries.size(), 2);
    QCOMPARE(currentCommandSeries.front(), "S SYS_OPMODE 1");
    QCOMPARE(currentCommandSeries.back(), "P 0");

    m_markingEngineControl->startJob(JobRepeats::Infinite);

    QVERIFY(!m_markingEngineControl->m_globalGenerator.empty());
    QVERIFY(!m_markingEngineControl->m_globalInterpreter.isCurrentCommandSeriesEmpty());

    currentCommandSeries = m_markingEngineControl->m_globalInterpreter.currentCommandSeries();

    QCOMPARE(currentCommandSeries.size(), 2);
    QCOMPARE(currentCommandSeries.front(), "S SYS_OPMODE 1");
    QCOMPARE(currentCommandSeries.back(), "P -1");

    m_markingEngineControl->startJob(JobRepeats::Stop);

    QVERIFY(!m_markingEngineControl->m_globalGenerator.empty());
    QVERIFY(!m_markingEngineControl->m_globalInterpreter.isCurrentCommandSeriesEmpty());

    currentCommandSeries = m_markingEngineControl->m_globalInterpreter.currentCommandSeries();

    QCOMPARE(currentCommandSeries.size(), 2);
    QCOMPARE(currentCommandSeries.front(), "S SYS_OPMODE 1");
    QCOMPARE(currentCommandSeries.back(), "P 0");
}

void SmartMoveControlTest::testTranslateSpecialValue()
{
    std::unique_ptr<precitec::hardware::SmartMoveControl> m_markingEngineControl(new precitec::hardware::SmartMoveControl);

    precitec::weldingFigure::SpecialValueInformation specialValueInformation;
    //Test for first point is -1 --> Take product value
    specialValueInformation.currentPointValue = -1.0;
    specialValueInformation.pointValueBefore = -1.0;
    specialValueInformation.productValue = 50.0;

    QCOMPARE(m_markingEngineControl->translateSpecialValue(specialValueInformation), 50.0);

    //Test point value takes last valid --> Take pointValueBefore
    specialValueInformation.pointValueBefore = 75.0;
    QCOMPARE(m_markingEngineControl->translateSpecialValue(specialValueInformation), 75.0);

    //Test point value is valid --> No pointValueBefore and no product value.
    specialValueInformation.currentPointValue = 25.0;
    QCOMPARE(m_markingEngineControl->translateSpecialValue(specialValueInformation), 25.0);
}

void SmartMoveControlTest::testCorrectionFile()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    QVERIFY(QDir{tmp.path()}.mkpath(QStringLiteral("calib/")));
    QDir dir{tmp.path()};
    QVERIFY(dir.exists());
    dir.mkpath(QStringLiteral("calib/"));
    QVERIFY(dir.exists("calib/"));
    QVERIFY(dir.cd("calib/"));

    dir.cd("../");
    dir.mkpath(QStringLiteral("config/calib/"));
    QVERIFY(dir.exists("config/calib/"));
    QVERIFY(dir.cd("config/calib/"));

    qputenv("WM_BASE_DIR", tmp.path().toLocal8Bit());

    std::unique_ptr<precitec::hardware::SmartMoveControl> m_markingEngineControl(new precitec::hardware::SmartMoveControl);

    std::string defaultCorrectionFile {"defaultCorrectionFile"};
    std::string customCorrectionFile;
    precitec::hardware::LensData lensData;

    lensData.calibrationFile = defaultCorrectionFile;

    QCOMPARE(m_markingEngineControl->correctionFile(lensData, customCorrectionFile), tmp.path().toStdString() + "/calib/" + defaultCorrectionFile);

    customCorrectionFile = "customCorrectionFile";

    QCOMPARE(m_markingEngineControl->correctionFile(lensData, customCorrectionFile), tmp.path().toStdString() + "/config/calib/" + customCorrectionFile);
}

void SmartMoveControlTest::testSendCorrectionFile()
{
    std::unique_ptr<precitec::hardware::SmartMoveControl> m_markingEngineControl(new precitec::hardware::SmartMoveControl);

    std::string correctionFile {""};
    std::string currentCorrectionFile {"currentCorrectionFile"};

    QVERIFY(!m_markingEngineControl->sendCorrectionFile(correctionFile, currentCorrectionFile));

    correctionFile = "currentCorrectionFile";

    QVERIFY(m_markingEngineControl->sendCorrectionFile(correctionFile, currentCorrectionFile));

    correctionFile = "differentCorrectionFile";
    QVERIFY(!m_markingEngineControl->sendCorrectionFile(correctionFile, currentCorrectionFile));
}

void SmartMoveControlTest::testUploadCorrectionFile_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QString>("filename");
    QTest::newRow("F460TestCorrectionFile") << 1 << QStringLiteral("testF460CorrectionFile.sbd");
}

void SmartMoveControlTest::testUploadCorrectionFile()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    QVERIFY(QDir{tmp.path()}.mkpath(QStringLiteral("calib/")));
    QDir dir{tmp.path()};
    QVERIFY(dir.exists());
    dir.mkpath(QStringLiteral("calib/"));
    QVERIFY(dir.exists("calib/"));
    QVERIFY(dir.cd("calib/"));

    QFETCH(QString, filename);
    const auto& currentFigureFilename = QStringLiteral("testData/") + filename;
    auto currentFigureFile = QFINDTESTDATA(currentFigureFilename);
    QVERIFY(QFile::copy(currentFigureFile, dir.filePath(filename)));

    qputenv("WM_BASE_DIR", tmp.path().toLocal8Bit());

    std::unique_ptr<precitec::hardware::SmartMoveControl> m_markingEngineControl(new precitec::hardware::SmartMoveControl);

    std::string correctionFile {""};

    QVERIFY(!m_markingEngineControl->uploadCorrectionFile(correctionFile));

    correctionFile = "correctionFile";

    QVERIFY(!m_markingEngineControl->uploadCorrectionFile(correctionFile));

    correctionFile = dir.filePath(filename).toStdString();

    QVERIFY(!m_markingEngineControl->uploadCorrectionFile(correctionFile));
}

QTEST_GUILESS_MAIN(SmartMoveControlTest)
#include "smartMoveControlTest.moc"
