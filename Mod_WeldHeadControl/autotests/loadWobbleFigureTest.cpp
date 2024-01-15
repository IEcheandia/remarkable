#include <QTest>

#include "../include/viWeldHead/Scanlab/FigureWeldingFunctions.h"
#include "../include/viWeldHead/Scanlab/Scanlab.h"

using namespace precitec::FigureWeldingFunctions;

namespace testHelper
{
struct TestHelper
{
    double limitPrecisionToSixDigits(const double& value)
    {
        auto valueCopy = static_cast<int> (value * 1000000);
        return valueCopy / 1000000.0;
    }
};
}

/*
 * This class is used to check the loading of a wobble figure in the scanlab class.
 */
class LoadWobbleFigureTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testWobbleFigureLoading_data();
    void testWobbleFigureLoading();
    void testWobbleFigureDeltaPower();
    void testDefineWobbleFigure_data();
    void testDefineWobbleFigure();

    void testLaserPowerDelayCompensation();
    void testADCValue();
    void testSetNominalPowers();
    void testConvertPowerToBits();
    void testFindDivisor();
    void testCalculateShiftResolution();
    void testCalculateShiftToCompensateDelay();
    void testShiftPowerToCompensateDelay();
    void testInterpolate();
    void testInterpolateMicroPoints();
    void testShiftedPowerValues();
    void testDefineWobbleFigureWithPowerDelayCompensation();

private:
    QTemporaryDir m_dir;
};

void LoadWobbleFigureTest::initTestCase()
{
    QVERIFY(m_dir.isValid());
    QVERIFY(QDir{}.mkpath(m_dir.filePath(QStringLiteral("config"))));
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());
}

void LoadWobbleFigureTest::testCtor()
{
    auto scanlabClass = new precitec::hardware::Scanlab();
    QCOMPARE(scanlabClass->GetWobbelFigureNumber(), -1);
    QCOMPARE(scanlabClass->GetWobbleMode(), -2);

    delete scanlabClass;
}

typedef std::vector< std::pair<double, double> > position;
void LoadWobbleFigureTest::testWobbleFigureLoading_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("ID");
    QTest::addColumn<QString>("description");
    QTest::addColumn<unsigned int>("microVectorFactor");
    QTest::addColumn<unsigned int>("powerModulationMode");
    QTest::addColumn<position>("endPosition");
    QTest::addColumn<std::vector<double>>("relativePower");
    QTest::addColumn<std::vector<double>>("relativeRingPower");

    position endPosition {{0.0, 0.0}, {0.0, 3.0}, {0.0, -3.0}, {0.0, 0.0}};
    std::vector<double> relativePower {0.0, 0.15, -0.75, 0.6};
    std::vector<double> relativeRingPower {0.0, 0.0, 0.0, 0.0};

    position endPosition2 {{0.0, 0.0}, {3.0, 3.0}, {-3.0, -3.0}, {0.0, 0.0}};
    std::vector<double> relativePower2 {0.0, 0.15, 0.6, -.75};
    std::vector<double> relativeRingPower2 {0.0, 0.3, -0.5, 0.2};

    QTest::newRow("1") << 2 << QStringLiteral("Another test figure") << QStringLiteral("2") << QStringLiteral("A short description") << 100u << 1u << endPosition << relativePower << relativeRingPower;
    QTest::newRow("2") << 3 << QStringLiteral("New figure") << QStringLiteral("3") << QStringLiteral("No description") << 500u << 8u << endPosition2 << relativePower2 << relativeRingPower2;
}

void LoadWobbleFigureTest::testWobbleFigureLoading()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    QVERIFY(QDir{tmp.path()}.mkpath(QStringLiteral("config/laser_controls/")));
    QDir dir{tmp.path()};
    QVERIFY(dir.exists());
    dir.mkpath(QStringLiteral("config/laser_controls/"));
    QVERIFY(dir.exists("config/laser_controls/"));
    QVERIFY(dir.cd("config/laser_controls/"));

    auto testWobbleFigureV1 = QFINDTESTDATA(QStringLiteral("testData/figureWobble1.json"));
    QVERIFY( QFile::copy(testWobbleFigureV1, dir.filePath(QStringLiteral("figureWobble1.json"))));
    auto testWobbleFigureV2 = QFINDTESTDATA(QStringLiteral("testData/figureWobble2.json"));
    QVERIFY( QFile::copy(testWobbleFigureV2, dir.filePath(QStringLiteral("figureWobble2.json"))));
    auto testWobbleFigureV3 = QFINDTESTDATA(QStringLiteral("testData/figureWobble3.json"));
    QVERIFY( QFile::copy(testWobbleFigureV3, dir.filePath(QStringLiteral("figureWobble3.json"))));

    qputenv("WM_BASE_DIR", tmp.path().toLocal8Bit());

    auto scanlabClass = new precitec::hardware::Scanlab();
    auto& figureWelding = scanlabClass->rtc6FigureWelding();

    QCOMPARE(QString::fromStdString(scanlabClass->GetFigureFilePath()), dir.path());

    int fileNumber = 0;
    std::string figureFile = scanlabClass->GetFigureFilePath() + scanlabClass->GetFigureFileName() + std::to_string(fileNumber) + scanlabClass->GetFigureFileEnding();

    QCOMPARE(figureWelding.readFigureFromFile(figureFile, precitec::FigureFileType::WobbleFigureType), false);

    fileNumber = 1;
    figureFile = scanlabClass->GetFigureFilePath() + scanlabClass->GetFigureFileName() + std::to_string(fileNumber) + scanlabClass->GetFigureFileEnding();

    QCOMPARE(figureWelding.readFigureFromFile(figureFile, precitec::FigureFileType::WobbleFigureType), false);

    QFETCH(int, row);
    fileNumber = row;
    figureFile = scanlabClass->GetFigureFilePath() + scanlabClass->GetFigureFileName() + std::to_string(fileNumber) + scanlabClass->GetFigureFileEnding();

    QCOMPARE(figureWelding.readFigureFromFile(figureFile, precitec::FigureFileType::WobbleFigureType), true);
    const auto wobbleFigure = figureWelding.getFigure(precitec::FigureFileType::WobbleFigureType);

    QTEST(QString::fromStdString(wobbleFigure->name), "name");
    QTEST(QString::fromStdString(wobbleFigure->ID), "ID");
    QTEST(QString::fromStdString(wobbleFigure->description), "description");
    QTEST(wobbleFigure->microVectorFactor, "microVectorFactor");
    QTEST(wobbleFigure->powerModulationMode, "powerModulationMode");

    const auto figure = wobbleFigure->figure;

    for (std::size_t i = 0; i < figure.size(); i++)
    {
        QFETCH(position, endPosition);
        QCOMPARE(figure.at(i).endPosition.first, endPosition.at(i).first);
        QCOMPARE(figure.at(i).endPosition.second, endPosition.at(i).second);
        QFETCH(std::vector<double>, relativePower);
        QCOMPARE(figure.at(i).relativePower, relativePower.at(i));
        QFETCH(std::vector<double>, relativeRingPower);
        QCOMPARE(figure.at(i).relativeRingPower, relativeRingPower.at(i));
    }

    delete scanlabClass;
}

void LoadWobbleFigureTest::testWobbleFigureDeltaPower()
{
    auto scanlabClass = new precitec::hardware::Scanlab();
    auto& figureWelding = scanlabClass->rtc6FigureWelding();

    auto nominalPower = 0.2;
    figureWelding.setNominalPower(nominalPower);
    QCOMPARE(figureWelding.nominalPower(), nominalPower);

    auto newPower = 0.35;
    auto oldPower = 0.2;
    unsigned int microVector = 1;

    QCOMPARE(calculateRelativeLaserPower(nominalPower, newPower, oldPower, microVector), 0.75);

    oldPower = newPower;
    newPower = 0.175;

    QCOMPARE(calculateRelativeLaserPower(nominalPower, newPower, oldPower, microVector), -0.875);

    nominalPower = 0.5;
    oldPower = 0.5;
    newPower = 0.75;
    microVector = 4;

    QCOMPARE(calculateRelativeLaserPower(nominalPower, newPower, oldPower, microVector), 0.125);

    oldPower = newPower;
    newPower = 0.1;

    QCOMPARE(calculateRelativeLaserPower(nominalPower, newPower, oldPower, microVector), -0.325);

    nominalPower = 0.1;
    oldPower = 0.2;
    newPower = 0.8;
    microVector = 10;

    QCOMPARE(calculateRelativeLaserPower(nominalPower, newPower, oldPower, microVector), 0.6);

    oldPower = newPower;
    newPower = 0.15;

    QCOMPARE(calculateRelativeLaserPower(nominalPower, newPower, oldPower, microVector), -0.65);

    //Limits
    nominalPower = 0.1;
    oldPower = 0.2;
    newPower = 0.8;
    microVector = 2;

    QCOMPARE(calculateRelativeLaserPower(nominalPower, newPower, oldPower, microVector), 1.0);

    oldPower = newPower;
    newPower = 0.15;

    QCOMPARE(calculateRelativeLaserPower(nominalPower, newPower, oldPower, microVector), -1.0);

    delete scanlabClass;
}

void LoadWobbleFigureTest::testDefineWobbleFigure_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<std::vector<double>>("wobbleTransVector");
    QTest::addColumn<std::vector<double>>("wobbleLongVector");
    QTest::addColumn<std::vector<double>>("deltaPower");
    QTest::addColumn<std::vector<double>>("deltaRingPower");
    QTest::addColumn<std::vector<double>>("wobbleTransVector2");
    QTest::addColumn<std::vector<double>>("wobbleLongVector2");
    QTest::addColumn<std::vector<double>>("deltaPower2");
    QTest::addColumn<std::vector<double>>("deltaRingPower2");
    QTest::addColumn<std::vector<double>>("wobbleTransVector3");
    QTest::addColumn<std::vector<double>>("wobbleLongVector3");
    QTest::addColumn<std::vector<double>>("deltaPower3");
    QTest::addColumn<std::vector<double>>("deltaRingPower3");
    QTest::addColumn<std::vector<double>>("wobbleTransVector4");
    QTest::addColumn<std::vector<double>>("wobbleLongVector4");
    QTest::addColumn<std::vector<double>>("deltaPower4");
    QTest::addColumn<std::vector<double>>("deltaRingPower4");

    std::vector<double> wobbleTransVector {-0.71, -0.29, 0.29, 0.71, 0.71, 0.29, -0.29, -0.71};
    std::vector<double> wobbleLongVector {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    std::vector<double> deltaPower {-0.25, 0.75, -0.60, 0.25, 0.5, 0.6, -0.75, -0.5};
    std::vector<double> deltaRingPower {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

    std::vector<double> wobbleTransVector2 {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    std::vector<double> wobbleLongVector2 {0.71, 0.29, -0.29, -0.71, -0.71, -0.29, 0.29, 0.71};
    std::vector<double> deltaPower2 {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    std::vector<double> deltaRingPower2 {0.016666, 0.033333, -0.05, -0.033333, 0.016666, 0.008333, 0.018333, -0.01};

    std::vector<double> wobbleTransVector3 {-0.71, -0.29, 0.29, 0.71, 0.71, 0.29, -0.29, -0.71};
    std::vector<double> wobbleLongVector3 {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    std::vector<double> deltaPower3 {0.1, 0.399999, -0.5, -0.199999, 0.199999, -0.5, 1.0, -0.6};
    std::vector<double> deltaRingPower3 {0.05, 0.199999, -0.35, -0.1, 0.1, 0.099999, 0.3, -0.3};

    std::vector<double> wobbleTransVector4 {-0.71, -0.29, 0.29, 0.71, 0.71, 0.29, -0.29, -0.71};
    std::vector<double> wobbleLongVector4 {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    std::vector<double> deltaPower4 {-0.111111, -0.044444, -0.088888, 0.066666, 0.044444, 0.066666, 0.111111, -0.044444};
    std::vector<double> deltaRingPower4 {-0.041666, 0.020833, -0.104166, 0.041666, 0.041666, 0.041666, 0.0625, -0.0625};

    QTest::newRow("1") << 1 << wobbleTransVector << wobbleLongVector << deltaPower << deltaRingPower << wobbleTransVector2 << wobbleLongVector2 << deltaPower2 << deltaRingPower2 << wobbleTransVector3 << wobbleLongVector3 << deltaPower3 << deltaRingPower3 << wobbleTransVector4 << wobbleLongVector4 << deltaPower4 << deltaRingPower4;
}

void LoadWobbleFigureTest::testDefineWobbleFigure()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    QVERIFY(QDir{tmp.path()}.mkpath(QStringLiteral("config/laser_controls/")));
    QDir dir{tmp.path()};
    QVERIFY(dir.exists());
    dir.mkpath(QStringLiteral("config/laser_controls/"));
    QVERIFY(dir.exists("config/laser_controls/"));
    QVERIFY(dir.cd("config/laser_controls/"));

    auto testWobbleFigure = QFINDTESTDATA(QStringLiteral("testData/figureWobble4.json"));
    QVERIFY( QFile::copy(testWobbleFigure, dir.filePath(QStringLiteral("figureWobble4.json"))));
    auto testWobbleFigure2 = QFINDTESTDATA(QStringLiteral("testData/figureWobble5.json"));
    QVERIFY( QFile::copy(testWobbleFigure2, dir.filePath(QStringLiteral("figureWobble5.json"))));
    auto testWobbleFigure3 = QFINDTESTDATA(QStringLiteral("testData/figureWobble6.json"));
    QVERIFY( QFile::copy(testWobbleFigure3, dir.filePath(QStringLiteral("figureWobble6.json"))));
    auto testWobbleFigure4 = QFINDTESTDATA(QStringLiteral("testData/figureWobble7.json"));
    QVERIFY( QFile::copy(testWobbleFigure4, dir.filePath(QStringLiteral("figureWobble7.json"))));
    auto testWobbleFigure5 = QFINDTESTDATA(QStringLiteral("testData/figureWobble8.json"));
    QVERIFY( QFile::copy(testWobbleFigure5, dir.filePath(QStringLiteral("figureWobble8.json"))));
    auto testWobbleFigure6 = QFINDTESTDATA(QStringLiteral("testData/figureWobble9.json"));
    QVERIFY( QFile::copy(testWobbleFigure6, dir.filePath(QStringLiteral("figureWobble9.json"))));

    qputenv("WM_BASE_DIR", tmp.path().toLocal8Bit());

    auto scanlabClass = new precitec::hardware::Scanlab();
    auto& figureWelding = scanlabClass->rtc6FigureWelding();
    testHelper::TestHelper helper;

    QCOMPARE(QString::fromStdString(scanlabClass->GetFigureFilePath()), dir.path());

    QCOMPARE(figureWelding.define_WobbleFigure(precitec::WobbleMode::NoWobbling), RTC6::FigureWelding::WobbleReturn::NoWobbling);
    QCOMPARE(figureWelding.define_WobbleFigure(precitec::WobbleMode::StandingEight), RTC6::FigureWelding::WobbleReturn::BasicWobbling);
    QCOMPARE(figureWelding.define_WobbleFigure(precitec::WobbleMode::Ellipse), RTC6::FigureWelding::WobbleReturn::BasicWobbling);
    QCOMPARE(figureWelding.define_WobbleFigure(precitec::WobbleMode::LyingEight), RTC6::FigureWelding::WobbleReturn::BasicWobbling);
    QVERIFY(figureWelding.getFigure(precitec::FigureFileType::WobbleFigureType)->figure.empty());
    QCOMPARE(figureWelding.define_WobbleFigure(precitec::WobbleMode::Free), RTC6::FigureWelding::WobbleReturn::InvalidWobbleFigure);

    int fileNumber = 4;
    std::string figureFile = scanlabClass->GetFigureFilePath() + scanlabClass->GetFigureFileName() + std::to_string(fileNumber) + scanlabClass->GetFigureFileEnding();

    QVERIFY(figureWelding.readFigureFromFile(figureFile, precitec::FigureFileType::WobbleFigureType));

    auto figure = figureWelding.getFigure(precitec::FigureFileType::WobbleFigureType);
    QVERIFY(!figure->figure.empty());
    figureWelding.setNominalPower(figure->figure.front().relativePower);
    QCOMPARE(figure->powerModulationMode, 1);                                   //Laser power modulation at analog port 1

    QCOMPARE(figureWelding.define_WobbleFigure(precitec::WobbleMode::Free), RTC6::FigureWelding::WobbleReturn::OnePortWobbling);
    QFETCH(std::vector<double>, wobbleTransVector);
    QFETCH(std::vector<double>, wobbleLongVector);
    QFETCH(std::vector<double>, deltaPower);
    QFETCH(std::vector<double>, deltaRingPower);
    QCOMPARE(figure->figure.size() - 1, figureWelding.m_wobbelTransVector.size());

    for (std::size_t i = 0; i < figureWelding.m_wobbelTransVector.size(); i++)
    {
        QCOMPARE(figureWelding.m_wobbelTransVector.at(i), wobbleTransVector.at(i));
        QCOMPARE(figureWelding.m_wobbelLongVector.at(i), wobbleLongVector.at(i));
        QCOMPARE(figureWelding.m_deltaPower.at(i), deltaPower.at(i));
        QVERIFY(figureWelding.m_deltaRingPower.empty());
    }

    fileNumber = 5;
    figureFile = scanlabClass->GetFigureFilePath() + scanlabClass->GetFigureFileName() + std::to_string(fileNumber) + scanlabClass->GetFigureFileEnding();

    QVERIFY(figureWelding.readFigureFromFile(figureFile, precitec::FigureFileType::WobbleFigureType));

    figure = figureWelding.getFigure(precitec::FigureFileType::WobbleFigureType);
    QVERIFY(!figure->figure.empty());
    figureWelding.setNominalRingPower(figure->figure.front().relativeRingPower);
    QCOMPARE(figure->powerModulationMode, 2);                                   //Laser power modulation at analog port 2

    QCOMPARE(figureWelding.define_WobbleFigure(precitec::WobbleMode::Free), RTC6::FigureWelding::WobbleReturn::OnePortWobbling);
    QFETCH(std::vector<double>, wobbleTransVector2);
    QFETCH(std::vector<double>, wobbleLongVector2);
    QFETCH(std::vector<double>, deltaPower2);
    QFETCH(std::vector<double>, deltaRingPower2);
    QCOMPARE(figure->figure.size() - 1, figureWelding.m_wobbelTransVector.size());

    for (std::size_t i = 0; i < figureWelding.m_wobbelTransVector.size(); i++)
    {
        QCOMPARE(figureWelding.m_wobbelTransVector.at(i), wobbleTransVector2.at(i));
        QCOMPARE(figureWelding.m_wobbelLongVector.at(i), wobbleLongVector2.at(i));
        QCOMPARE(helper.limitPrecisionToSixDigits(figureWelding.m_deltaPower.at(i)), deltaRingPower2.at(i));
        QVERIFY(figureWelding.m_deltaRingPower.empty());
    }

    fileNumber = 6;
    figureFile = scanlabClass->GetFigureFilePath() + scanlabClass->GetFigureFileName() + std::to_string(fileNumber) + scanlabClass->GetFigureFileEnding();

    QVERIFY(figureWelding.readFigureFromFile(figureFile, precitec::FigureFileType::WobbleFigureType));

    figure = figureWelding.getFigure(precitec::FigureFileType::WobbleFigureType);
    QVERIFY(!figure->figure.empty());
    QCOMPARE(figure->powerModulationMode, 8);
    QCOMPARE(figure->microVectorFactor, 500);
    figureWelding.setNominalPowers();

    QCOMPARE(figureWelding.define_WobbleFigure(precitec::WobbleMode::Free), RTC6::FigureWelding::WobbleReturn::TooManySegments);

    fileNumber = 7;
    figureFile = scanlabClass->GetFigureFilePath() + scanlabClass->GetFigureFileName() + std::to_string(fileNumber) + scanlabClass->GetFigureFileEnding();

    QVERIFY(figureWelding.readFigureFromFile(figureFile, precitec::FigureFileType::WobbleFigureType));

    figure = figureWelding.getFigure(precitec::FigureFileType::WobbleFigureType);
    QVERIFY(!figure->figure.empty());
    figureWelding.setNominalPower(figure->figure.front().relativePower);
    figureWelding.setNominalRingPower(figure->figure.front().relativeRingPower);
    QCOMPARE(figure->powerModulationMode, 8);                                   //Laser power modulation at analog port 1 and 2
    QCOMPARE(figure->microVectorFactor, 2);

    QCOMPARE(figureWelding.define_WobbleFigure(precitec::WobbleMode::Free), RTC6::FigureWelding::WobbleReturn::TwoPortWobbling);
    QCOMPARE(figureWelding.m_wobbelTransVector.size(), figure->figure.size() - 1);
    QCOMPARE(figureWelding.m_wobbelLongVector.size(), figure->figure.size() - 1);
    QCOMPARE(figureWelding.m_deltaPower.size(), figure->figure.size() - 1);
    QCOMPARE(figureWelding.m_deltaRingPower.size(), figure->figure.size() - 1);

    QFETCH(std::vector<double>, wobbleTransVector3);
    QFETCH(std::vector<double>, wobbleLongVector3);
    QFETCH(std::vector<double>, deltaPower3);
    QFETCH(std::vector<double>, deltaRingPower3);

    QCOMPARE(wobbleTransVector3.size(), figureWelding.m_wobbelTransVector.size());
    QCOMPARE(wobbleLongVector3.size(), figureWelding.m_wobbelTransVector.size());
    QCOMPARE(deltaPower3.size(), figureWelding.m_wobbelTransVector.size());
    QCOMPARE(deltaRingPower3.size(), figureWelding.m_wobbelTransVector.size());

    for (std::size_t i = 0; i < figureWelding.m_wobbelTransVector.size(); i++)
    {
        QCOMPARE(figureWelding.m_wobbelTransVector.at(i), wobbleTransVector3.at(i));
        QCOMPARE(figureWelding.m_wobbelLongVector.at(i), wobbleLongVector3.at(i));
        QCOMPARE(helper.limitPrecisionToSixDigits(figureWelding.m_deltaPower.at(i)), deltaPower3.at(i));
        QCOMPARE(helper.limitPrecisionToSixDigits(figureWelding.m_deltaRingPower.at(i)), deltaRingPower3.at(i));
    }

    fileNumber = 8;
    figureFile = scanlabClass->GetFigureFilePath() + scanlabClass->GetFigureFileName() + std::to_string(fileNumber) + scanlabClass->GetFigureFileEnding();

    QVERIFY(figureWelding.readFigureFromFile(figureFile, precitec::FigureFileType::WobbleFigureType));

    figure = figureWelding.getFigure(precitec::FigureFileType::WobbleFigureType);
    QVERIFY(!figure->figure.empty());
    figureWelding.setNominalPower(figure->figure.front().relativePower);
    figureWelding.setNominalRingPower(figure->figure.front().relativeRingPower);
    QCOMPARE(figure->powerModulationMode, 8);                                   //Laser power modulation at analog port 1 and 2
    QCOMPARE(figure->microVectorFactor, 4);

    QCOMPARE(figureWelding.define_WobbleFigure(precitec::WobbleMode::Free), RTC6::FigureWelding::WobbleReturn::TwoPortWobbling);
    QCOMPARE(figureWelding.m_wobbelTransVector.size(), figure->figure.size() - 1);
    QCOMPARE(figureWelding.m_wobbelLongVector.size(), figure->figure.size() - 1);
    QCOMPARE(figureWelding.m_deltaPower.size(), figure->figure.size() - 1);
    QCOMPARE(figureWelding.m_deltaRingPower.size(), figure->figure.size() - 1);

    QFETCH(std::vector<double>, wobbleTransVector4);
    QFETCH(std::vector<double>, wobbleLongVector4);
    QFETCH(std::vector<double>, deltaPower4);
    QFETCH(std::vector<double>, deltaRingPower4);

    QCOMPARE(wobbleTransVector4.size(), figureWelding.m_wobbelTransVector.size());
    QCOMPARE(wobbleLongVector4.size(), figureWelding.m_wobbelTransVector.size());
    QCOMPARE(deltaPower4.size(), figureWelding.m_wobbelTransVector.size());
    QCOMPARE(deltaRingPower4.size(), figureWelding.m_wobbelTransVector.size());

    for (std::size_t i = 0; i < figureWelding.m_wobbelTransVector.size(); i++)
    {
        QCOMPARE(figureWelding.m_wobbelTransVector.at(i), wobbleTransVector4.at(i));
        QCOMPARE(figureWelding.m_wobbelLongVector.at(i), wobbleLongVector4.at(i));
        QCOMPARE(helper.limitPrecisionToSixDigits(figureWelding.m_deltaPower.at(i)), deltaPower4.at(i));
        QCOMPARE(helper.limitPrecisionToSixDigits(figureWelding.m_deltaRingPower.at(i)), deltaRingPower4.at(i));
    }

    fileNumber = 9;
    figureFile = scanlabClass->GetFigureFilePath() + scanlabClass->GetFigureFileName() + std::to_string(fileNumber) + scanlabClass->GetFigureFileEnding();

    QVERIFY(figureWelding.readFigureFromFile(figureFile, precitec::FigureFileType::WobbleFigureType));

    figure = figureWelding.getFigure(precitec::FigureFileType::WobbleFigureType);
    QVERIFY(!figure->figure.empty());
    QCOMPARE(figure->powerModulationMode, 8);
    QCOMPARE(figure->microVectorFactor, 3);
    figureWelding.setNominalPowers();

    QCOMPARE(figureWelding.define_WobbleFigure(precitec::WobbleMode::Free), RTC6::FigureWelding::WobbleReturn::InvalidMicroVectorFactor);

    delete scanlabClass;
}

void LoadWobbleFigureTest::testLaserPowerDelayCompensation()
{
    auto scanlabClass = new precitec::hardware::Scanlab();
    auto& figureWelding = scanlabClass->rtc6FigureWelding();

    figureWelding.setLaserPowerDelayCompensation(10);
    QCOMPARE(figureWelding.laserPowerDelayCompensation(), 10);

    figureWelding.setLaserPowerDelayCompensation(5);
    QCOMPARE(figureWelding.laserPowerDelayCompensation(), 5);

    figureWelding.setLaserPowerDelayCompensation(20);
    QCOMPARE(figureWelding.laserPowerDelayCompensation(), 20);

    delete scanlabClass;
}

void LoadWobbleFigureTest::testADCValue()
{
    auto scanlabClass = new precitec::hardware::Scanlab();
    auto& figureWelding = scanlabClass->rtc6FigureWelding();

    QCOMPARE(figureWelding.getADCValue(), 4095);

    figureWelding.setADCValue(10);
    QCOMPARE(figureWelding.getADCValue(), 10);

    figureWelding.setADCValue(2047);
    QCOMPARE(figureWelding.getADCValue(), 2047);

    figureWelding.setADCValue(1023);
    QCOMPARE(figureWelding.getADCValue(), 1023);

    delete scanlabClass;
}

void LoadWobbleFigureTest::testSetNominalPowers()
{
    auto scanlabClass = new precitec::hardware::Scanlab();
    auto& figureWelding = scanlabClass->rtc6FigureWelding();

    QVERIFY(!figureWelding.setNominalPowers());

    RTC6::Figure newFigure;

    figureWelding.m_freeFigure = newFigure;
    QVERIFY(!figureWelding.setNominalPowers());

    RTC6::command::Order newOrder {std::make_pair(0.4, 0.2), 0.3, 0.2, 100.0};
    newFigure.figure.emplace_back(newOrder);

    figureWelding.m_freeFigure = newFigure;
    QVERIFY(figureWelding.setNominalPowers());
    QCOMPARE(figureWelding.nominalPower(), 0.3);
    QCOMPARE(figureWelding.nominalRingPower(), 0.2);

    newFigure.figure.clear();
    RTC6::command::Order newOrder2 {std::make_pair(0.34, 0.12), 0.5, 0.75, 150.0};
    newFigure.figure.emplace_back(newOrder2);
    newFigure.figure.emplace_back(newOrder);
    figureWelding.m_freeFigure = newFigure;

    QVERIFY(figureWelding.setNominalPowers());
    QCOMPARE(figureWelding.nominalPower(), 0.5);
    QCOMPARE(figureWelding.nominalRingPower(), 0.75);

    delete scanlabClass;
}

void LoadWobbleFigureTest::testConvertPowerToBits()
{
    auto scanlabClass = new precitec::hardware::Scanlab();
    auto& figureWelding = scanlabClass->rtc6FigureWelding();

    QCOMPARE(figureWelding.getADCValue(), 4095);

    QCOMPARE(figureWelding.convertPowerToBits(0.0), 0);
    QCOMPARE(figureWelding.convertPowerToBits(0.1), 409);
    QCOMPARE(figureWelding.convertPowerToBits(0.25), 1023);
    QCOMPARE(figureWelding.convertPowerToBits(0.5), 2047);
    QCOMPARE(figureWelding.convertPowerToBits(0.6), 2457);
    QCOMPARE(figureWelding.convertPowerToBits(0.75), 3071);
    QCOMPARE(figureWelding.convertPowerToBits(1.0), 4095);

    delete scanlabClass;
}

void LoadWobbleFigureTest::testFindDivisor()
{
    QCOMPARE(findDivisor(4, 25, false), 1);                 //1 kHz
    QCOMPARE(findDivisor(4, 50, false), 1);                 //500 Hz
    QCOMPARE(findDivisor(4, 100, false), 1);                //250 Hz
    QCOMPARE(findDivisor(4, 250, false), 1);                //100 Hz
    QCOMPARE(findDivisor(4, 500, false), 2);                //50 Hz
    QCOMPARE(findDivisor(4, 1000, false), 4);               //25 Hz
    QCOMPARE(findDivisor(4, 2500, false), 10);              //10 Hz

    QCOMPARE(findDivisor(8, 12, false), 1);
    QCOMPARE(findDivisor(8, 25, false), 1);
    QCOMPARE(findDivisor(8, 50, false), 1);
    QCOMPARE(findDivisor(8, 125, false), 1);
    QCOMPARE(findDivisor(8, 250, false), 2);
    QCOMPARE(findDivisor(8, 500, false), 4);
    QCOMPARE(findDivisor(8, 1250, false), 10);

    QCOMPARE(findDivisor(16, 6, false), 1);
    QCOMPARE(findDivisor(16, 12, false), 1);
    QCOMPARE(findDivisor(16, 25, false), 1);
    QCOMPARE(findDivisor(16, 50, false), 1);
    QCOMPARE(findDivisor(16, 125, false), 5);
    QCOMPARE(findDivisor(16, 250, false), 5);
    QCOMPARE(findDivisor(16, 625, false), 25);
}

void LoadWobbleFigureTest::testCalculateShiftResolution()
{
    QCOMPARE(calculateShiftResolution(0), 0);
    QCOMPARE(calculateShiftResolution(1), 10);
    QCOMPARE(calculateShiftResolution(2), 20);
    QCOMPARE(calculateShiftResolution(3), 30);
    QCOMPARE(calculateShiftResolution(5), 50);
    QCOMPARE(calculateShiftResolution(10), 100);
}

void LoadWobbleFigureTest::testCalculateShiftToCompensateDelay()
{
    QCOMPARE(calculateShiftToCompensateDelay(100, 10), 10);
    QCOMPARE(calculateShiftToCompensateDelay(200, 10), 20);
    QCOMPARE(calculateShiftToCompensateDelay(300, 10), 30);
    QCOMPARE(calculateShiftToCompensateDelay(100, 20), 5);
    QCOMPARE(calculateShiftToCompensateDelay(200, 20), 10);
    QCOMPARE(calculateShiftToCompensateDelay(300, 20), 15);
    QCOMPARE(calculateShiftToCompensateDelay(100, 50), 2);
    QCOMPARE(calculateShiftToCompensateDelay(200, 50), 4);
    QCOMPARE(calculateShiftToCompensateDelay(300, 50), 6);

    QCOMPARE(calculateShiftToCompensateDelay(-100, 10), -10);
    QCOMPARE(calculateShiftToCompensateDelay(-200, 10), -20);
    QCOMPARE(calculateShiftToCompensateDelay(-300, 10), -30);
    QCOMPARE(calculateShiftToCompensateDelay(-100, 20), -5);
    QCOMPARE(calculateShiftToCompensateDelay(-200, 20), -10);
    QCOMPARE(calculateShiftToCompensateDelay(-300, 20), -15);
    QCOMPARE(calculateShiftToCompensateDelay(-100, 50), -2);
    QCOMPARE(calculateShiftToCompensateDelay(-200, 50), -4);
    QCOMPARE(calculateShiftToCompensateDelay(-300, 50), -6);
}

void LoadWobbleFigureTest::testShiftPowerToCompensateDelay()
{
    std::vector<double> powerValues {0.1, 0.4, 0.5, 0.2, 0.1};
    std::vector<double> expectedPowerValues {0.4, 0.5, 0.2, 0.1, 0.1};

    //Left shifting
    auto resultedPowerValues = shiftPowerToCompensateDelay(powerValues, 1);
    QCOMPARE(resultedPowerValues, expectedPowerValues);

    expectedPowerValues = {0.2, 0.1, 0.1, 0.4, 0.5};
    resultedPowerValues = shiftPowerToCompensateDelay(powerValues, 3);
    QCOMPARE(resultedPowerValues, expectedPowerValues);

    //Right shifting
    expectedPowerValues = {0.1, 0.1, 0.4, 0.5, 0.2};
    resultedPowerValues = shiftPowerToCompensateDelay(powerValues, -1);
    QCOMPARE(resultedPowerValues, expectedPowerValues);

    expectedPowerValues = {0.2, 0.1, 0.1, 0.4, 0.5};
    resultedPowerValues = shiftPowerToCompensateDelay(powerValues, -2);
    QCOMPARE(resultedPowerValues, expectedPowerValues);
}

void LoadWobbleFigureTest::testInterpolate()
{
    QCOMPARE(precitec::math::interpolate(0.0, 1.0, 2, 0), 0.0);
    QCOMPARE(precitec::math::interpolate(0.0, 1.0, 2, 1), 0.5);
    QCOMPARE(precitec::math::interpolate(0.0, 1.0, 2, 2), 1.0);
}

void LoadWobbleFigureTest::testInterpolateMicroPoints()
{
    std::vector<RTC6::command::Order> points;
    RTC6::command::Order newOrder = {std::make_pair(0.0, 0.0), 0.5, 0.2};
    points.emplace_back(newOrder);
    newOrder = {std::make_pair(0.0, 10.0), 0.6, 0.3};
    points.emplace_back(newOrder);
    newOrder = {std::make_pair(10.0, 10.0), 0.5, 0.2};
    points.emplace_back(newOrder);
    newOrder = {std::make_pair(10.0, 0.0), 0.4, 0.1};
    points.emplace_back(newOrder);
    newOrder = {std::make_pair(0.0, 0.0), 0.5, 0.2};
    points.emplace_back(newOrder);

    std::vector<RTC6::command::Order> expectedPoints;
    newOrder = {std::make_pair(0.0, 0.0), 0.5, 0.2};
    expectedPoints.emplace_back(newOrder);
    newOrder = {std::make_pair(0.0, 5.0), 0.55, 0.25};
    expectedPoints.emplace_back(newOrder);
    newOrder = {std::make_pair(0.0, 10.0), 0.6, 0.3};
    expectedPoints.emplace_back(newOrder);
    newOrder = {std::make_pair(5.0, 10.0), 0.55, 0.25};
    expectedPoints.emplace_back(newOrder);
    newOrder = {std::make_pair(10.0, 10.0), 0.5, 0.2};
    expectedPoints.emplace_back(newOrder);
    newOrder = {std::make_pair(10.0, 5.0), 0.45, 0.15};
    expectedPoints.emplace_back(newOrder);
    newOrder = {std::make_pair(10.0, 0.0), 0.4, 0.1};
    expectedPoints.emplace_back(newOrder);
    newOrder = {std::make_pair(5.0, 0.0), 0.45, 0.15};
    expectedPoints.emplace_back(newOrder);
    newOrder = {std::make_pair(0.0, 0.0), 0.5, 0.2};
    expectedPoints.emplace_back(newOrder);

    unsigned int numberOfInterpolations = 2;
    QCOMPARE(points.size(), (expectedPoints.size() + 1) / numberOfInterpolations);

    const auto resultedPoints = interpolateMicroPoints(points, numberOfInterpolations);

    QCOMPARE(resultedPoints.size(), expectedPoints.size());
    for (std::size_t i = 0; i < resultedPoints.size(); i++)
    {
        QCOMPARE(resultedPoints.at(i).endPosition.first, expectedPoints.at(i).endPosition.first);
        QCOMPARE(resultedPoints.at(i).endPosition.second, expectedPoints.at(i).endPosition.second);
        QCOMPARE(resultedPoints.at(i).relativePower, expectedPoints.at(i).relativePower);
        QCOMPARE(resultedPoints.at(i).relativeRingPower, expectedPoints.at(i).relativeRingPower);
        QCOMPARE(resultedPoints.at(i).velocity, 0);
    }
}

void LoadWobbleFigureTest::testShiftedPowerValues()
{
    std::vector<RTC6::command::Order> figure;
    RTC6::command::Order newOrder;
    newOrder.endPosition = std::make_pair(-5.0, 0.0);
    newOrder.relativePower = 0.5;
    newOrder.relativeRingPower = 0.45;
    figure.emplace_back(newOrder);
    newOrder.endPosition = std::make_pair(-2.5, 0.0);
    newOrder.relativePower = 0.6;
    newOrder.relativeRingPower = 0.35;
    figure.emplace_back(newOrder);
    newOrder.endPosition = std::make_pair(0.0, 0.0);
    newOrder.relativePower = 0.5;
    newOrder.relativeRingPower = 0.45;
    figure.emplace_back(newOrder);
    newOrder.endPosition = std::make_pair(2.5, 0.0);
    newOrder.relativePower = 0.4;
    newOrder.relativeRingPower = 0.55;
    figure.emplace_back(newOrder);
    newOrder.endPosition = std::make_pair(5.0, 0.0);
    newOrder.relativePower = 0.5;
    newOrder.relativeRingPower = 0.45;
    figure.emplace_back(newOrder);

    QVERIFY(shiftedPowerValues(precitec::WobbleControl::AnalogOut1Variation, {}, 0).empty());

    const int zeroShift = 0;
    const auto& analogOut1PowerValues = shiftedPowerValues(precitec::WobbleControl::AnalogOut1Variation, figure, zeroShift);
    QVERIFY(!analogOut1PowerValues.empty());
    QCOMPARE(analogOut1PowerValues.front(), analogOut1PowerValues.back());
    QCOMPARE(analogOut1PowerValues.size(), figure.size());

    for (std::size_t i = 0; i < figure.size(); i++)
    {
        QCOMPARE(analogOut1PowerValues.at(i), figure.at(i).relativePower);
    }

    const auto& analogOut2PowerValues = shiftedPowerValues(precitec::WobbleControl::AnalogOut2Variation, figure, zeroShift);
    QVERIFY(!analogOut2PowerValues.empty());
    QCOMPARE(analogOut2PowerValues.front(), analogOut2PowerValues.back());
    QCOMPARE(analogOut2PowerValues.size(), figure.size());

    for (std::size_t i = 0; i < figure.size(); i++)
    {
        QCOMPARE(analogOut2PowerValues.at(i), figure.at(i).relativeRingPower);
    }

    const int shift = 2;
    const auto& analogOut1PowerValuesShifted = shiftedPowerValues(precitec::WobbleControl::AnalogOut1Variation, figure, shift);
    QVERIFY(!analogOut1PowerValuesShifted.empty());
    QCOMPARE(analogOut1PowerValuesShifted.front(), analogOut1PowerValuesShifted.back());
    QCOMPARE(analogOut1PowerValuesShifted.size(), figure.size());
    std::vector<double> expectedPowerValuesShifted {0.5, 0.4, 0.5, 0.6, 0.5};

    for (std::size_t i = 0; i < figure.size(); i++)
    {
        QCOMPARE(analogOut1PowerValuesShifted.at(i), expectedPowerValuesShifted.at(i));
    }

    const auto& analogOut2PowerValuesShifted = shiftedPowerValues(precitec::WobbleControl::AnalogOut2Variation, figure, shift);
    QVERIFY(!analogOut2PowerValuesShifted.empty());
    QCOMPARE(analogOut2PowerValuesShifted.front(), analogOut2PowerValuesShifted.back());
    QCOMPARE(analogOut2PowerValuesShifted.size(), figure.size());
    expectedPowerValuesShifted = {0.45, 0.55, 0.45, 0.35, 0.45};

    for (std::size_t i = 0; i < figure.size(); i++)
    {
        QCOMPARE(analogOut2PowerValuesShifted.at(i), expectedPowerValuesShifted.at(i));
    }

    //TODO Shift in other direction!
}

void LoadWobbleFigureTest::testDefineWobbleFigureWithPowerDelayCompensation()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    QVERIFY(QDir{tmp.path()}.mkpath(QStringLiteral("config/laser_controls/")));
    QDir dir{tmp.path()};
    QVERIFY(dir.exists());
    dir.mkpath(QStringLiteral("config/laser_controls/"));
    QVERIFY(dir.exists("config/laser_controls/"));
    QVERIFY(dir.cd("config/laser_controls/"));

    auto testWobbleFigure = QFINDTESTDATA(QStringLiteral("testData/figureWobble4.json"));
    QVERIFY( QFile::copy(testWobbleFigure, dir.filePath(QStringLiteral("figureWobble4.json"))));

    qputenv("WM_BASE_DIR", tmp.path().toLocal8Bit());

    auto scanlabClass = new precitec::hardware::Scanlab();
    /*auto seamWobbel = scanlabClass->seamWobbel();

    int fileNumber = 4;
    std::string figureFile = scanlabClass->GetFigureFilePath() + scanlabClass->GetFigureFileName() + std::to_string(fileNumber) + scanlabClass->GetFigureFileEnding();

    QVERIFY(scanlabClass->loadWobbleFigure(figureFile, precitec::FigureFileType::WobbleFigureType));

    seamWobbel = scanlabClass->seamWobbel();
    seamWobbel.setLaserPowerDelayCompensation(0);
    auto figure = seamWobbel.getFigure(precitec::FigureFileType::WobbleFigureType);
    QVERIFY(!figure->figure.empty());
    figure->microVectorFactor = 124;

    seamWobbel.setNominalPowers();
    QCOMPARE(seamWobbel.define_WobbleFigure(precitec::WobbleMode::Free), SeamWobbel::WobbleReturn::OnePortWobbling);*/

    delete scanlabClass;
}

QTEST_GUILESS_MAIN(LoadWobbleFigureTest)
#include "loadWobbleFigureTest.moc"
