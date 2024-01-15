#include <QTest>
#include <QSignalSpy>

#include "../src/simulationController.h"
#include "../src/FileModel.h"
#include "../src/laserPointController.h"
#include "../src/WobbleFigure.h"
#include "../src/figureEditorSettings.h"

#include "../src/editorDataTypes.h"

using precitec::scanmaster::components::wobbleFigureEditor::SimulationController;
using precitec::scantracker::components::wobbleFigureEditor::FileModel;
using precitec::scanmaster::components::wobbleFigureEditor::LaserPointController;
using precitec::scantracker::components::wobbleFigureEditor::WobbleFigure;
using precitec::scanmaster::components::wobbleFigureEditor::FigureEditorSettings;

using precitec::scantracker::components::wobbleFigureEditor::FileType;
using RTC6::seamFigure::SeamFigure;
using RTC6::wobbleFigure::Figure;

namespace testFunction
{
QVector2D limitPrecisionToSixDigits(QVector2D value)
{
    QVector2D copy = value * 1000000;
    copy.setX(qRound(copy.x()));
    copy.setY(qRound(copy.y()));
    return copy / 1000000;
}

double limitPrecisionToSixDigits(double value)
{
    auto copy = value * 1000000;
    int copyInt = qRound(copy);
    return static_cast<double> (copyInt) / 1000000.0;
}

double limitPowerPrecisionToSixDigits(double value)
{
    auto copy = value * 10;
    int copyInt = qRound(copy) * 100000;
    return static_cast<double> (copyInt) / 1000000.0;
}

double limitPrecisionToOneDigit(double value)
{
    auto valueCopy = value * 10;
    int copyInt = qRound(valueCopy);
    return static_cast<double> (copyInt) / 10.0;
}

double limitPrecisionToTwoDigits(double value)
{
    auto valueCopy = value * 100;
    int copyInt = qRound(valueCopy);
    return static_cast<double> (copyInt) / 100.0;
}

SeamFigure getSeamFigureFromVectors(const QVector<QVector2D>& position, double seamSpeed)
{
    SeamFigure seam;

    for (const auto& element : position)
    {
        RTC6::seamFigure::command::Order newOrder;
        newOrder.endPosition = std::make_pair(element.x(), element.y());
        newOrder.velocity = seamSpeed;
        seam.figure.push_back(newOrder);
    }

    return seam;
}

Figure getWobbleFigureFromVectors(const QVector<QVector2D>& position, int frequency)
{
    Figure wobble;
    wobble.microVectorFactor = 1000000 / (10 * frequency * (position.size() - 1));

    for (const auto& element : position)
    {
        RTC6::wobbleFigure::command::Order newOrder;
        newOrder.endPosition = std::make_pair(element.x(), element.y());
        wobble.figure.push_back(newOrder);
    }

    return wobble;
}
}

class SimulationControllerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testFileModel();
    void testLaserPointController();
    void testSimulationMode();
    void testLoopCount();
    void testTenMicroSecondsFactor();
    void testReady();
    void testGetFigureForSimulation();
    void testShowSimulatedFigure();
    void testClear();
    void testWobbleFigurePointSize_data();
    void testWobbleFigurePointSize();
    void testSimulatedSeamFigure();
    void testStartSimulationFigure();
    void testCheckOneFigureMissing();
    void testClearSimulationFigure();
    void testFirstValidSpeed();
    void testCreateSeamVelocities_data();
    void testCreateSeamVelocities();
    void testCreateWobbleVelocities_data();
    void testCreateWobbleVelocities();
    void testCalculateVisualizationInformation_data();
    void testCalculateVisualizationInformation();
    void testCheckTooManyPoints_data();
    void testCheckTooManyPoints();
    void testCheckSimulatedFigureReady_data();
    void testCheckSimulatedFigureReady();
    void testCalcFocusSpeed();
    void testCalculateSimulationFigure_data();
    void testCalculateSimulationFigure();
    void testAngleRadFromXAxis_data();
    void testAngleRadFromXAxis();
    void testRotateVectorRad_data();
    void testRotateVectorRad();

private:
    QTemporaryDir m_dir;
};

void SimulationControllerTest::initTestCase()
{
    QVERIFY(m_dir.isValid());
    QVERIFY(QDir{}.mkpath(m_dir.filePath(QStringLiteral("config"))));
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());
}

void SimulationControllerTest::testCtor()
{
    SimulationController simulationController;

    QCOMPARE(simulationController.fileModel(), nullptr);
    QCOMPARE(simulationController.laserPointController(), nullptr);
    QVERIFY(!simulationController.simulationMode());
    QCOMPARE(simulationController.loopCount(), 0);
    QCOMPARE(simulationController.tenMicroSecondsFactor(), 1);
    QVERIFY(!simulationController.ready());
}

void SimulationControllerTest::testFileModel()
{
    SimulationController simulationController;

    QSignalSpy filemodelChanged{&simulationController, &SimulationController::fileModelChanged};
    QVERIFY(filemodelChanged.isValid());
    QCOMPARE(filemodelChanged.count(), 0);

    simulationController.setFileModel(nullptr);
    QCOMPARE(simulationController.fileModel(), nullptr);
    QCOMPARE(filemodelChanged.count(), 0);

    FileModel fileModel;

    simulationController.setFileModel(&fileModel);
    QCOMPARE(simulationController.fileModel(), &fileModel);
    QCOMPARE(filemodelChanged.count(), 1);

    simulationController.setFileModel(nullptr);
    QCOMPARE(simulationController.fileModel(), nullptr);
    QCOMPARE(filemodelChanged.count(), 2);
}

void SimulationControllerTest::testLaserPointController()
{
    SimulationController simulationController;

    QSignalSpy laserPointControllerChanged{&simulationController, &SimulationController::laserPointControllerChanged};
    QVERIFY(laserPointControllerChanged.isValid());
    QCOMPARE(laserPointControllerChanged.count(), 0);

    simulationController.setLaserPointController(nullptr);
    QCOMPARE(simulationController.laserPointController(), nullptr);
    QCOMPARE(laserPointControllerChanged.count(), 0);

    LaserPointController laserPointController;

    simulationController.setLaserPointController(&laserPointController);
    QCOMPARE(simulationController.laserPointController(), &laserPointController);
    QCOMPARE(laserPointControllerChanged.count(), 1);

    simulationController.setLaserPointController(nullptr);
    QCOMPARE(simulationController.laserPointController(), nullptr);
    QCOMPARE(laserPointControllerChanged.count(), 2);
}

void SimulationControllerTest::testSimulationMode()
{
    SimulationController simulationController;

    QSignalSpy simulationModeChanged{&simulationController, &SimulationController::simulationModeChanged};
    QVERIFY(simulationModeChanged.isValid());
    QCOMPARE(simulationModeChanged.count(), 0);

    simulationController.setSimulationMode(false);
    QVERIFY(!simulationController.simulationMode());
    QCOMPARE(simulationModeChanged.count(), 0);

    simulationController.setSimulationMode(true);
    QVERIFY(simulationController.simulationMode());
    QCOMPARE(simulationModeChanged.count(), 1);

    simulationController.setSimulationMode(false);
    QVERIFY(!simulationController.simulationMode());
    QCOMPARE(simulationModeChanged.count(), 2);
}

void SimulationControllerTest::testLoopCount()
{
    SimulationController simulationController;

    QSignalSpy loopCountChanged{&simulationController, &SimulationController::loopCountChanged};
    QVERIFY(loopCountChanged.isValid());
    QCOMPARE(loopCountChanged.count(), 0);

    simulationController.setLoopCount(0);
    QCOMPARE(simulationController.loopCount(), 0);
    QCOMPARE(loopCountChanged.count(), 0);

    simulationController.setLoopCount(10);
    QCOMPARE(simulationController.loopCount(), 10);
    QCOMPARE(loopCountChanged.count(), 1);
}

void SimulationControllerTest::testTenMicroSecondsFactor()
{
    SimulationController simulationController;

    QSignalSpy tenMicroSecondsFactorChanged{&simulationController, &SimulationController::tenMicroSecondsFactorChanged};
    QVERIFY(tenMicroSecondsFactorChanged.isValid());
    QCOMPARE(tenMicroSecondsFactorChanged.count(), 0);

    simulationController.setTenMicroSecondsFactor(1);
    QCOMPARE(simulationController.tenMicroSecondsFactor(), 1);
    QCOMPARE(tenMicroSecondsFactorChanged.count(), 0);

    simulationController.setTenMicroSecondsFactor(10);
    QCOMPARE(simulationController.tenMicroSecondsFactor(), 10);
    QCOMPARE(tenMicroSecondsFactorChanged.count(), 1);

    simulationController.setTenMicroSecondsFactor(500);
    QCOMPARE(simulationController.tenMicroSecondsFactor(), 500);
    QCOMPARE(tenMicroSecondsFactorChanged.count(), 2);
}

void SimulationControllerTest::testReady()
{
    SimulationController simulationController;

    QSignalSpy readyChanged{&simulationController, &SimulationController::readyChanged};
    QVERIFY(readyChanged.isValid());
    QCOMPARE(readyChanged.count(), 0);

    simulationController.setReady(false);
    QVERIFY(!simulationController.ready());
    QCOMPARE(readyChanged.count(), 0);

    simulationController.setReady(true);
    QVERIFY(simulationController.ready());
    QCOMPARE(readyChanged.count(), 1);

    simulationController.setReady(false);
    QVERIFY(!simulationController.ready());
    QCOMPARE(readyChanged.count(), 2);
}

void SimulationControllerTest::testGetFigureForSimulation()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    QVERIFY(QDir{tmp.path()}.mkpath(QStringLiteral("config/weld_figure/")));
    QDir dir{tmp.path()};
    QVERIFY(dir.exists());
    dir.mkpath(QStringLiteral("config/weld_figure/"));
    QVERIFY(dir.exists("config/weld_figure/"));
    QVERIFY(dir.cd("config/weld_figure/"));

    auto testWeldingSeamV1 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam1.json"));
    QVERIFY( QFile::copy(testWeldingSeamV1, tmp.filePath(QStringLiteral("config/weld_figure/weldingSeam1.json"))));
    auto testWeldingSeamV2 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam2.json"));
    QVERIFY( QFile::copy(testWeldingSeamV2, tmp.filePath(QStringLiteral("config/weld_figure/weldingSeam2.json"))));

    QDir wobbleDir{tmp.path()};
    QVERIFY(wobbleDir.exists());
    wobbleDir.mkpath(QStringLiteral("config/laser_controls/"));
    QVERIFY(wobbleDir.exists("config/laser_controls/"));
    QVERIFY(wobbleDir.cd("config/laser_controls/"));

    auto testWobbleFileV1 = QFINDTESTDATA(QStringLiteral("testData/figureWobble2.json"));
    QVERIFY( QFile::copy(testWobbleFileV1, tmp.filePath(QStringLiteral("config/laser_controls/figureWobble1.json"))));
    auto testWobbleFileV2 = QFINDTESTDATA(QStringLiteral("testData/figureWobble3.json"));
    QVERIFY( QFile::copy(testWobbleFileV2, tmp.filePath(QStringLiteral("config/laser_controls/figureWobble2.json"))));

    qputenv("WM_BASE_DIR", tmp.path().toLocal8Bit());

    FileModel fileModel;

    fileModel.loadFiles();
    QTRY_COMPARE(fileModel.files().size(), 3);

    SimulationController simulationController;
    QVERIFY(simulationController.m_seamFigure.figure.empty());

    QSignalSpy figureForSimulationChanged{&simulationController, &SimulationController::figureForSimulationChanged};
    QVERIFY(figureForSimulationChanged.isValid());
    QCOMPARE(figureForSimulationChanged.count(), 0);

    simulationController.getFigureForSimulation("weldingSeam1.json", FileType::Seam);
    QVERIFY(simulationController.m_seamFigure.figure.empty());
    QCOMPARE(figureForSimulationChanged.count(), 0);
    simulationController.getFigureForSimulation("figureWobble1.json", FileType::Wobble);
    QVERIFY(simulationController.m_wobbleFigure.figure.empty());
    QCOMPARE(figureForSimulationChanged.count(), 0);

    simulationController.setFileModel(&fileModel);
    simulationController.getFigureForSimulation("", FileType::Seam);
    QVERIFY(simulationController.m_seamFigure.figure.empty());
    QCOMPARE(figureForSimulationChanged.count(), 0);
    simulationController.getFigureForSimulation("", FileType::Wobble);
    QVERIFY(simulationController.m_wobbleFigure.figure.empty());
    QCOMPARE(figureForSimulationChanged.count(), 0);

    simulationController.getFigureForSimulation("weldingSeam5.json", FileType::Overlay);
    QVERIFY(simulationController.m_seamFigure.figure.empty());
    QCOMPARE(figureForSimulationChanged.count(), 0);
    simulationController.getFigureForSimulation("figureWobble5.json", FileType::None);
    QVERIFY(simulationController.m_wobbleFigure.figure.empty());
    QCOMPARE(figureForSimulationChanged.count(), 0);

    simulationController.getFigureForSimulation("weldingSeam5.json", FileType::Seam);
    QVERIFY(simulationController.m_seamFigure.figure.empty());
    QCOMPARE(figureForSimulationChanged.count(), 1);
    simulationController.getFigureForSimulation("figureWobble5.json", FileType::Wobble);
    QVERIFY(simulationController.m_wobbleFigure.figure.empty());
    QCOMPARE(figureForSimulationChanged.count(), 2);

    simulationController.getFigureForSimulation("weldingSeam1.json", FileType::Seam);
    QVERIFY(!simulationController.m_seamFigure.figure.empty());
    QCOMPARE(figureForSimulationChanged.count(), 3);

    QCOMPARE(QString::fromStdString(simulationController.m_seamFigure.name), "Test");
    QCOMPARE(QString::fromStdString(simulationController.m_seamFigure.ID), "1");
    QCOMPARE(QString::fromStdString(simulationController.m_seamFigure.description), "Test file without ring power");
    QCOMPARE(simulationController.m_seamFigure.figure.size(), 5);

    simulationController.getFigureForSimulation("figureWobble1.json", FileType::Wobble);
    QVERIFY(!simulationController.m_wobbleFigure.figure.empty());
    QCOMPARE(figureForSimulationChanged.count(), 4);

    QCOMPARE(QString::fromStdString(simulationController.m_wobbleFigure.name), "Test");
    QCOMPARE(QString::fromStdString(simulationController.m_wobbleFigure.ID), "2");
    QCOMPARE(QString::fromStdString(simulationController.m_wobbleFigure.description), "Test wobble figure");
    QCOMPARE(simulationController.m_wobbleFigure.microVectorFactor, 1);
    QCOMPARE(simulationController.m_wobbleFigure.figure.size(), 4);
}

void SimulationControllerTest::testShowSimulatedFigure()
{
    SimulationController simulationController;
    WobbleFigure figure;
    LaserPointController laserPointController;
    laserPointController.setFigure(&figure);

    RTC6::seamFigure::command::Order newOrder;
    newOrder.endPosition = std::make_pair(1.0, 0.5);
    newOrder.power = -1.0;
    newOrder.ringPower = -1.0;
    newOrder.velocity = -1.0;
    simulationController.m_simulatedSeamFigure.figure.push_back(newOrder);
    newOrder.endPosition = std::make_pair(2.0, 1.0);
    simulationController.m_simulatedSeamFigure.figure.push_back(newOrder);

    simulationController.setReady(true);
    simulationController.showSimulatedFigure();
    QCOMPARE(laserPointController.figure()->get_node_count(), 0);

    simulationController.setReady(false);
    simulationController.showSimulatedFigure();
    QCOMPARE(laserPointController.figure()->get_node_count(), 0);

    simulationController.setLaserPointController(&laserPointController);
    simulationController.showSimulatedFigure();
    QCOMPARE(laserPointController.figure()->get_node_count(), 0);
}

void SimulationControllerTest::testClear()
{
    SimulationController simulationController;

    QSignalSpy pointCountSimulationFigureChanged{&simulationController, &SimulationController::pointCountSimulationFigureChanged};
    QVERIFY(pointCountSimulationFigureChanged.isValid());
    QCOMPARE(pointCountSimulationFigureChanged.count(), 0);

    simulationController.m_simulatedSeamFigure.name = std::string("Test");
    simulationController.m_simulatedSeamFigure.ID = std::string("0");
    simulationController.m_simulatedSeamFigure.description = std::string("Description");

    RTC6::seamFigure::command::Order simulationOrder;
    simulationOrder.endPosition = std::make_pair(-1.0, 0.5);
    simulationOrder.power = -1.0;
    simulationOrder.ringPower = -1.0;
    simulationOrder.velocity = -1.0;
    simulationController.m_simulatedSeamFigure.figure.push_back(simulationOrder);
    simulationOrder.endPosition = std::make_pair(1.0, -0.5);
    simulationController.m_simulatedSeamFigure.figure.push_back(simulationOrder);

    simulationController.clear();
    QCOMPARE(pointCountSimulationFigureChanged.count(), 1);

    QCOMPARE(simulationController.m_simulatedSeamFigure.name, "Simulation");
    QCOMPARE(simulationController.m_simulatedSeamFigure.ID, "0");
    QCOMPARE(simulationController.m_simulatedSeamFigure.description, "Simulated seam superimposed with a wobble figure!");
    QVERIFY(simulationController.m_simulatedSeamFigure.figure.empty());

    simulationController.m_simulatedSeamFigure.name = std::string("Test");
    simulationController.m_simulatedSeamFigure.ID = std::string("0");
    simulationController.m_simulatedSeamFigure.description = std::string("Description");

    simulationOrder.endPosition = std::make_pair(-1.0, 0.5);
    simulationOrder.power = -1.0;
    simulationOrder.ringPower = -1.0;
    simulationOrder.velocity = -1.0;
    simulationController.m_simulatedSeamFigure.figure.push_back(simulationOrder);
    simulationController.m_seamFigure.figure.push_back(simulationOrder);
    simulationOrder.endPosition = std::make_pair(1.0, -0.5);
    simulationController.m_simulatedSeamFigure.figure.push_back(simulationOrder);
    simulationController.m_seamFigure.figure.push_back(simulationOrder);

    RTC6::wobbleFigure::command::Order wobbleOrder;
    wobbleOrder.endPosition = std::make_pair(0.5, 6.0);
    simulationController.m_wobbleFigure.figure.push_back(wobbleOrder);
    simulationController.m_wobbleFigure.figure.push_back(wobbleOrder);
    simulationController.m_wobbleFigure.figure.push_back(wobbleOrder);

    QCOMPARE(simulationController.m_simulatedSeamFigure.figure.size(), 2);
    QCOMPARE(simulationController.m_seamFigure.figure.size(), 2);
    QCOMPARE(simulationController.m_wobbleFigure.figure.size(), 3);

    simulationController.setSimulationMode(true);

    QCOMPARE(simulationController.m_simulatedSeamFigure.figure.size(), 0);
    QCOMPARE(simulationController.m_seamFigure.figure.size(), 0);
    QCOMPARE(simulationController.m_wobbleFigure.figure.size(), 0);

    QCOMPARE(simulationController.m_simulatedSeamFigure.name, "Simulation");
    QCOMPARE(simulationController.m_simulatedSeamFigure.ID, "0");
    QCOMPARE(simulationController.m_simulatedSeamFigure.description, "Simulated seam superimposed with a wobble figure!");
}

void SimulationControllerTest::testWobbleFigurePointSize_data()
{
    QTest::addColumn<QVector<QVector2D>>("position");
    QTest::addColumn<int>("microVectorFactor");
    QTest::addColumn<int>("result");

    QTest::newRow("LineX") << QVector<QVector2D> {
        QVector2D{0.0, 0.0},
        QVector2D{0.15, 0.0},
        QVector2D{0.0, 0.0},
        QVector2D{-0.15, 0.0},
        QVector2D{0.0, 0.0}
    } << 10 << 50;
    QTest::newRow("LineY") << QVector<QVector2D> {
        QVector2D{0.0, 0.0},
        QVector2D{0.0, 0.15},
        QVector2D{0.0, 0.0},
    } << 5 << 15;
}

void SimulationControllerTest::testWobbleFigurePointSize()
{
    SimulationController simulationController;

    QFETCH(QVector<QVector2D>, position);
    QFETCH(int, microVectorFactor);

    Figure wobble;
    wobble.microVectorFactor = microVectorFactor;
    RTC6::wobbleFigure::command::Order newWobblePoint;
    for (int i = 0; i < position.size(); i++)
    {
        const auto &point = position.at(i);
        newWobblePoint.endPosition = std::make_pair(point.x(), point.y());
        wobble.figure.push_back(newWobblePoint);
    }

    simulationController.m_wobbleFigure = wobble;

    QFETCH(int, result);
    QCOMPARE(simulationController.wobbleFigurePointSize(), result);
}

void SimulationControllerTest::testSimulatedSeamFigure()
{
    SimulationController simulationController;

    SeamFigure seamFigure;
    RTC6::seamFigure::command::Order seamOrder;
    seamOrder.endPosition = std::make_pair(-1.0, 0.5);
    seamOrder.power = -1.0;
    seamOrder.ringPower = -1.0;
    seamOrder.velocity = -1.0;
    seamFigure.figure.push_back(seamOrder);
    seamOrder.endPosition = std::make_pair(1.0, -0.5);
    seamFigure.figure.push_back(seamOrder);
    simulationController.m_simulatedSeamFigure = seamFigure;

    const auto& simulatedSeamFigure = simulationController.simulatedSeamFigure();
    QCOMPARE(simulatedSeamFigure.front().endPosition.first, -1.0);
    QCOMPARE(simulatedSeamFigure.front().endPosition.second, 0.5);
    QCOMPARE(simulatedSeamFigure.back().endPosition.first, 1.0);
    QCOMPARE(simulatedSeamFigure.back().endPosition.second, -0.5);
}

void SimulationControllerTest::testStartSimulationFigure()
{
    SimulationController simulationController;

    QSignalSpy readyChanged{&simulationController, &SimulationController::readyChanged};
    QVERIFY(readyChanged.isValid());
    QCOMPARE(readyChanged.count(), 0);

    simulationController.setReady(true);
    QCOMPARE(readyChanged.count(), 1);

    simulationController.startSimulationFigure();
    QVERIFY(!simulationController.ready());
    QCOMPARE(readyChanged.count(), 2);
}

void SimulationControllerTest::testCheckOneFigureMissing()
{
    SimulationController simulationController;

    QVERIFY(simulationController.checkOneFigureMissing());
    SeamFigure seamFigure;
    RTC6::seamFigure::command::Order seamOrder;
    seamOrder.endPosition = std::make_pair(-1.0, 0.5);
    seamOrder.power = -1.0;
    seamOrder.ringPower = -1.0;
    seamOrder.velocity = -1.0;
    seamFigure.figure.push_back(seamOrder);
    seamOrder.endPosition = std::make_pair(1.0, -0.5);
    seamFigure.figure.push_back(seamOrder);
    simulationController.m_seamFigure = seamFigure;

    QVERIFY(simulationController.checkOneFigureMissing());
    Figure wobbleFigure;
    RTC6::wobbleFigure::command::Order wobbleOrder;
    wobbleOrder.endPosition = std::make_pair(0.25, 0.0);
    wobbleOrder.power = -1.0;
    wobbleOrder.ringPower = -1.0;
    wobbleFigure.figure.push_back(wobbleOrder);
    wobbleOrder.endPosition = std::make_pair(0.0, 0.0);
    wobbleFigure.figure.push_back(wobbleOrder);
    simulationController.m_seamFigure = SeamFigure{};
    simulationController.m_wobbleFigure = wobbleFigure;
    QVERIFY(simulationController.checkOneFigureMissing());

    simulationController.m_seamFigure = seamFigure;
    QVERIFY(!simulationController.checkOneFigureMissing());
}

void SimulationControllerTest::testClearSimulationFigure()
{
    SimulationController simulationController;

    simulationController.m_simulatedSeamFigure.name = std::string("Test");
    simulationController.m_simulatedSeamFigure.ID = std::string("0");
    simulationController.m_simulatedSeamFigure.description = std::string("Description");

    RTC6::seamFigure::command::Order simulationOrder;
    simulationOrder.endPosition = std::make_pair(-1.0, 0.5);
    simulationOrder.power = -1.0;
    simulationOrder.ringPower = -1.0;
    simulationOrder.velocity = -1.0;
    simulationController.m_simulatedSeamFigure.figure.push_back(simulationOrder);
    simulationOrder.endPosition = std::make_pair(1.0, -0.5);
    simulationController.m_simulatedSeamFigure.figure.push_back(simulationOrder);

    simulationController.calculateSimulationFigure();
    QCOMPARE(simulationController.m_simulatedSeamFigure.name, "Simulation");
    QCOMPARE(simulationController.m_simulatedSeamFigure.ID, "0");
    QCOMPARE(simulationController.m_simulatedSeamFigure.description, "Simulated seam superimposed with a wobble figure!");
    QVERIFY(simulationController.m_simulatedSeamFigure.figure.empty());
}

void SimulationControllerTest::testFirstValidSpeed()
{
    SimulationController simulationController;

    QCOMPARE(simulationController.firstValidSpeed(), 1.0);

    FigureEditorSettings::instance()->setScannerSpeed(1000.0);

    QCOMPARE(simulationController.firstValidSpeed(), 1000.0);
}

void SimulationControllerTest::testCreateSeamVelocities_data()
{
    QTest::addColumn<QVector<QVector2D>>("position");
    QTest::addColumn<QVector<double>>("velocity");
    QTest::addColumn<int>("countSeam10usParts");

    QTest::newRow("LineX") << QVector<QVector2D> {
        QVector2D{0.0f, 0.0f},
        QVector2D{0.25f, 0.0f},
        QVector2D{0.5f, 0.0f},
        QVector2D{0.75f, 0.0f},
        QVector2D{1.0f, 0.0f}
    } << QVector<double> {
        -1.0,
        -1.0,
        -1.0,
        -1.0,
        -1.0
    } << 100;
}

void SimulationControllerTest::testCreateSeamVelocities()
{
    SimulationController simulationController;
    FigureEditorSettings::instance()->setScannerSpeed(1000.0);

    QFETCH(QVector<QVector2D>, position);
    QFETCH(QVector<double>, velocity);
    QCOMPARE(position.size(), velocity.size());

    SeamFigure seam;
    RTC6::seamFigure::command::Order newSeamPoint;
    for (int i = 0; i < position.size(); i++)
    {
        const auto &point = position.at(i);
        newSeamPoint.endPosition = std::make_pair(point.x(), point.y());
        newSeamPoint.velocity = velocity.at(i);
        seam.figure.push_back(newSeamPoint);
    }

    simulationController.m_seamFigure = seam;
    const auto &seamPartsIn10usParts = simulationController.createSeamVelocities();
    QFETCH(int, countSeam10usParts);
    QCOMPARE(seamPartsIn10usParts.size(), countSeam10usParts);

    auto startPoint = position.front();
    int counter = 0;
    int deltaCompareValue = countSeam10usParts / (position.size() - 1);
    int compareValue = 0;

    for (const auto& seamPart : seamPartsIn10usParts)
    {
        if (counter == compareValue)
        {
            QCOMPARE(testFunction::limitPrecisionToSixDigits(startPoint), position.at(counter / deltaCompareValue));
            compareValue += deltaCompareValue;
        }
        startPoint += seamPart;
        counter++;
    }
}


void SimulationControllerTest::testCreateWobbleVelocities_data()
{
    QTest::addColumn<QVector<QVector2D>>("position");
    QTest::addColumn<int>("microVectorFactor");
    QTest::addColumn<QVector<double>>("power");

    QTest::newRow("LineX") << QVector<QVector2D> {
        QVector2D{0.0, 0.0},
        QVector2D{0.15, 0.0},
        QVector2D{0.0, 0.0},
        QVector2D{-0.15, 0.0},
        QVector2D{0.0, 0.0}
    } << 10 << QVector<double> {
        0.1,
        0.3,
        0.1,
        0.1,
        0.1
    };
    QTest::newRow("LineY") << QVector<QVector2D> {
        QVector2D{0.0, 0.0},
        QVector2D{0.0, 0.15},
        QVector2D{0.0, 0.0},
        QVector2D{0.0, -0.15},
        QVector2D{0.0, 0.0}
    } << 5 << QVector<double> {
        0.5,
        0.3,
        0.4,
        0.5,
        0.5
    };
}

void SimulationControllerTest::testCreateWobbleVelocities()
{
    SimulationController simulationController;

    QFETCH(QVector<QVector2D>, position);
    QFETCH(int, microVectorFactor);
    QFETCH(QVector<double>, power);

    Figure wobble;
    wobble.microVectorFactor = microVectorFactor;
    RTC6::wobbleFigure::command::Order newWobblePoint;
    for (int i = 0; i < position.size(); i++)
    {
        const auto &point = position.at(i);
        newWobblePoint.endPosition = std::make_pair(point.x(), point.y());
        newWobblePoint.power = power.at(i);
        wobble.figure.push_back(newWobblePoint);
    }

    simulationController.m_wobbleFigure = wobble;
    const auto &wobbleParts = simulationController.createWobbleVelocities();
    QCOMPARE(wobbleParts.size(), (wobble.figure.size() - 1) * wobble.microVectorFactor);

    auto startPoint = position.front();
    int counter = 0;
    int deltaCompareValue = wobble.microVectorFactor;
    int compareValue = deltaCompareValue;
    auto startPower = power.front();

    for (const auto& wobblePart : wobbleParts)
    {
        if (counter == compareValue)
        {
            QCOMPARE(startPoint, position.at(counter / deltaCompareValue));
            compareValue += deltaCompareValue;
            QCOMPARE(testFunction::limitPowerPrecisionToSixDigits(startPower), power.at(counter / deltaCompareValue));
        }
        startPoint += wobblePart.toVector2D();
        startPower += wobblePart.z();
        counter++;
    }
}

void SimulationControllerTest::testCalculateVisualizationInformation_data()
{
    QTest::addColumn<QVector<QVector2D>>("position");
    QTest::addColumn<int>("microVectorFactor");
    QTest::addColumn<int>("loopCount");
    QTest::addColumn<int>("tenMicroSecondsFactor");

    QTest::newRow("LineX") << QVector<QVector2D> {
        QVector2D{0.0, 0.0},
        QVector2D{0.15, 0.0},
        QVector2D{0.0, 0.0},
        QVector2D{-0.15, 0.0},
        QVector2D{0.0, 0.0}
    } << 10 << 10 << 1;
    QTest::newRow("LineY") << QVector<QVector2D> {
        QVector2D{0.0, 0.0},
        QVector2D{0.0, 0.15},
        QVector2D{0.0, 0.0},
        QVector2D{0.0, -0.15},
        QVector2D{0.0, 0.0}
    } << 5 << 10 << 50;
}

void SimulationControllerTest::testCalculateVisualizationInformation()
{
    SimulationController simulationController;

    QFETCH(QVector<QVector2D>, position);
    QFETCH(int, microVectorFactor);
    QFETCH(int, loopCount);
    QFETCH(int, tenMicroSecondsFactor);

    Figure wobble;
    wobble.microVectorFactor = microVectorFactor;
    RTC6::wobbleFigure::command::Order newWobblePoint;
    for (int i = 0; i < position.size(); i++)
    {
        const auto &point = position.at(i);
        newWobblePoint.endPosition = std::make_pair(point.x(), point.y());
        wobble.figure.push_back(newWobblePoint);
    }

    simulationController.m_wobbleFigure = wobble;
    simulationController.setLoopCount(loopCount);
    simulationController.setTenMicroSecondsFactor(tenMicroSecondsFactor);

    const auto& visualizationInformation = simulationController.calculateVisualizationInformation();
    QCOMPARE(visualizationInformation.first, (wobble.figure.size() * wobble.microVectorFactor * loopCount) - 1);
    QCOMPARE(visualizationInformation.second, tenMicroSecondsFactor);
}

void SimulationControllerTest::testCheckTooManyPoints_data()
{
    QTest::addColumn<QVector<QVector2D>>("position");

    QTest::newRow("LineX") << QVector<QVector2D> {
        QVector2D{0.0, 0.0},
        QVector2D{0.05, 0.0},
        QVector2D{0.10, 0.0},
        QVector2D{0.15, 0.0},
        QVector2D{0.20, 0.0},
        QVector2D{0.25, 0.0},
        QVector2D{0.30, 0.0},
        QVector2D{0.35, 0.0},
        QVector2D{0.40, 0.0},
        QVector2D{0.45, 0.0},
        QVector2D{0.50, 0.0},
        QVector2D{0.55, 0.0},
        QVector2D{0.60, 0.0}
    };
}

void SimulationControllerTest::testCheckTooManyPoints()
{
    SimulationController simulationController;

    QFETCH(QVector<QVector2D>, position);

    SeamFigure seam;
    RTC6::seamFigure::command::Order newSeamPoint;
    for (int i = 0; i < position.size(); i++)
    {
        const auto &point = position.at(i);
        newSeamPoint.endPosition = std::make_pair(point.x(), point.y());
        seam.figure.push_back(newSeamPoint);
    }

    simulationController.m_simulatedSeamFigure = seam;
    simulationController.setTenMicroSecondsFactor(10);
    QVERIFY(!simulationController.checkTooManyPoints());

    simulationController.setTenMicroSecondsFactor(1);
    QVERIFY(!simulationController.checkTooManyPoints());

    simulationController.m_pointCountVisualizationLimit = 10;
    QVERIFY(simulationController.checkTooManyPoints());

    simulationController.setTenMicroSecondsFactor(20);
    QVERIFY(!simulationController.checkTooManyPoints());
}

void SimulationControllerTest::testCheckSimulatedFigureReady_data()
{
    QTest::addColumn<QVector<QVector2D>>("position");

    QTest::newRow("LineX") << QVector<QVector2D> {
        QVector2D{0.0, 0.0},
        QVector2D{0.05, 0.0},
        QVector2D{0.10, 0.0},
        QVector2D{0.15, 0.0},
        QVector2D{0.20, 0.0},
        QVector2D{0.25, 0.0},
        QVector2D{0.30, 0.0},
        QVector2D{0.35, 0.0},
        QVector2D{0.40, 0.0},
        QVector2D{0.45, 0.0},
        QVector2D{0.50, 0.0},
        QVector2D{0.55, 0.0},
        QVector2D{0.60, 0.0}
    };
}

void SimulationControllerTest::testCheckSimulatedFigureReady()
{
    SimulationController simulationController;

    simulationController.checkSimulatedFigureReady();
    QVERIFY(!simulationController.ready());

    QFETCH(QVector<QVector2D>, position);

    SeamFigure seam;
    RTC6::seamFigure::command::Order newSeamPoint;
    for (int i = 0; i < position.size(); i++)
    {
        const auto &point = position.at(i);
        newSeamPoint.endPosition = std::make_pair(point.x(), point.y());
        seam.figure.push_back(newSeamPoint);
    }
    simulationController.m_simulatedSeamFigure = seam;

    simulationController.checkSimulatedFigureReady();
    QVERIFY(simulationController.ready());

    Figure wobble;
    wobble.microVectorFactor = 5;
    RTC6::wobbleFigure::command::Order newWobblePoint;
    for (int i = 0; i < 5; i++)
    {
        newWobblePoint.endPosition = std::make_pair(i * 0.2, i * 0.01);
        wobble.figure.push_back(newWobblePoint);
    }
    simulationController.m_wobbleFigure = wobble;

    simulationController.checkSimulatedFigureReady();
    QVERIFY(simulationController.ready());

    simulationController.setLoopCount(1);
    simulationController.checkSimulatedFigureReady();
    QVERIFY(simulationController.ready());
}

void SimulationControllerTest::testCalcFocusSpeed()
{
    SimulationController simulationController;

    QVector2D movement{-0.02, 0.05};

    QCOMPARE(testFunction::limitPrecisionToTwoDigits(simulationController.calculateFocusSpeed(movement)), 5385.16);
}

void SimulationControllerTest::testCalculateSimulationFigure_data()
{
    QTest::addColumn<QVector<QVector2D>>("seamPosition");
    QTest::addColumn<QVector<QVector2D>>("wobblePosition");
    QTest::addColumn<QVector<QVector2D>>("simulationPosition");
    QTest::addColumn<double>("seamSpeed");
    QTest::addColumn<int>("wobbleFrequency");
    QTest::addColumn<QVector<double>>("focusSpeed");

    QTest::newRow("SeamLineWobbleCircle") << QVector<QVector2D> {           //Line l = 5 mm, horizontal, x axis
        QVector2D{-0.1, 0.0},
        QVector2D{-0.02999999970197678, 0.0},
        QVector2D{0.029999998658895494, 0.0},
        QVector2D{0.10000000029802322, 0.0}
    } << QVector<QVector2D> {                                               //Circle r = 1mm, d = 2mm
        QVector2D{1.0, 0.0},
        QVector2D{0.6234898018587336, 0.7818314824680298},
        QVector2D{-0.22252093395631434, 0.9749279121818236},
        QVector2D{-0.900968867902419, 0.43388373911755823},
        QVector2D{-0.9009688679024191, -0.433883739117558},
        QVector2D{-0.2225209339563146, -0.9749279121818236},
        QVector2D{0.6234898018587334, -0.7818314824680299},
        QVector2D{1.0, 0.0}
    } << QVector<QVector2D> {
        QVector2D{-0.1, 0.0},
        QVector2D{-0.117148, 0.0459901},
        QVector2D{-0.134295, 0.0919802},
        QVector2D{-0.151443, 0.13797},
        QVector2D{-0.168591, 0.18396},
        QVector2D{-0.185738, 0.22995},
        QVector2D{-0.202886, 0.275941},
        QVector2D{-0.220034, 0.321931},
        QVector2D{-0.237181, 0.367921},
        QVector2D{-0.254329, 0.413911},
        QVector2D{-0.271477, 0.459901},
        QVector2D{-0.288624, 0.505891},
        QVector2D{-0.305772, 0.551881},
        QVector2D{-0.32292, 0.597871},
        QVector2D{-0.340067, 0.643861},
        QVector2D{-0.357215, 0.689851},
        QVector2D{-0.374363, 0.735841},
        QVector2D{-0.39151, 0.781831},
        QVector2D{-0.436276, 0.79319},
        QVector2D{-0.481041, 0.804549},
        QVector2D{-0.525806, 0.815907},
        QVector2D{-0.570572, 0.827266},
        QVector2D{-0.615337, 0.838625},
        QVector2D{-0.660102, 0.849983},
        QVector2D{-0.704868, 0.861342},
        QVector2D{-0.749633, 0.8727},
        QVector2D{-0.794398, 0.884059},
        QVector2D{-0.839164, 0.895418},
        QVector2D{-0.883929, 0.906776},
        QVector2D{-0.928694, 0.918135},
        QVector2D{-0.97346, 0.929493},
        QVector2D{-1.018225, 0.940852},
        QVector2D{-1.06299, 0.952211},
        QVector2D{-1.107756, 0.963569},
        QVector2D{-1.152521, 0.974928},
        QVector2D{-1.18743, 0.943102},
        QVector2D{-1.222338, 0.911276},
        QVector2D{-1.257247, 0.879449},
        QVector2D{-1.292156, 0.847623},
        QVector2D{-1.327064, 0.815797}
    } << 500.0 << 840 << QVector<double> {
        4908.3,
        4908.3,
        4908.3,
        4908.3,
        4908.3,
        4908.3,
        4908.3,
        4908.3,
        4908.3,
        4908.3,
        4908.3,
        4908.3,
        4908.3,
        4908.3,
        4908.3,
        4908.3,
        4908.3,
        4618.4,
        4618.4,
        4618.4,
        4618.4,
        4618.4,
        4618.4,
        4618.4,
        4618.4,
        4618.4,
        4618.4,
        4618.4,
        4618.4,
        4618.4,
        4618.4,
        4618.4,
        4618.4,
        4618.4,
        4723.9,
        4723.9,
        4723.9,
        4723.9,
        4723.9,
        4723.9,
    };
}

void SimulationControllerTest::testCalculateSimulationFigure()
{
    SimulationController simulationController;
    simulationController.m_simulatedSeamFigure.name = std::string("Test");
    simulationController.m_simulatedSeamFigure.ID = std::string("0");
    simulationController.m_simulatedSeamFigure.description = std::string("Description");

    RTC6::seamFigure::command::Order simulationOrder;
    simulationOrder.endPosition = std::make_pair(-1.0, 0.5);
    simulationOrder.power = -1.0;
    simulationOrder.ringPower = -1.0;
    simulationOrder.velocity = -1.0;
    simulationController.m_simulatedSeamFigure.figure.push_back(simulationOrder);
    simulationOrder.endPosition = std::make_pair(1.0, -0.5);
    simulationController.m_simulatedSeamFigure.figure.push_back(simulationOrder);

    simulationController.calculateSimulationFigure();
    QCOMPARE(simulationController.m_simulatedSeamFigure.name, "Simulation");
    QCOMPARE(simulationController.m_simulatedSeamFigure.ID, "0");
    QCOMPARE(simulationController.m_simulatedSeamFigure.description, "Simulated seam superimposed with a wobble figure!");
    QVERIFY(simulationController.m_simulatedSeamFigure.figure.empty());

    QFETCH(QVector<QVector2D>, seamPosition);
    QFETCH(QVector<QVector2D>, wobblePosition);
    QFETCH(QVector<QVector2D>, simulationPosition);
    QFETCH(double, seamSpeed);
    QFETCH(int, wobbleFrequency);
    QFETCH(QVector<double>, focusSpeed);

    auto seamFigure = testFunction::getSeamFigureFromVectors(seamPosition, seamSpeed);
    auto wobbleFigure = testFunction::getWobbleFigureFromVectors(wobblePosition, wobbleFrequency);

    simulationController.m_wobbleFigure.microVectorFactor = wobbleFigure.microVectorFactor;
    for (const auto &element : seamFigure.figure)
    {
        simulationController.m_seamFigure.figure.push_back(element);
    }
    for (const auto &element : wobbleFigure.figure)
    {
        simulationController.m_wobbleFigure.figure.push_back(element);
    }

    simulationController.calculateSimulationFigure();

    QCOMPARE(simulationController.m_simulatedSeamFigure.figure.size() - 1, simulationPosition.size());

    for (int i = 0; i < simulationPosition.size(); i++)
    {
        const auto& calculatedSimulationPoint = simulationController.m_simulatedSeamFigure.figure.at(i);
        const auto& simulationPoint = simulationPosition.at(i);
        QCOMPARE(testFunction::limitPrecisionToSixDigits(calculatedSimulationPoint.endPosition.first), testFunction::limitPrecisionToSixDigits(simulationPoint.x()));
        QCOMPARE(testFunction::limitPrecisionToSixDigits(calculatedSimulationPoint.endPosition.second), testFunction::limitPrecisionToSixDigits(simulationPoint.y()));
    }

    QCOMPARE(simulationController.m_focusSpeed.size(), simulationPosition.size());

    for (std::size_t i = 0; i < simulationController.m_focusSpeed.size(); i++)
    {
        QCOMPARE(testFunction::limitPrecisionToOneDigit(simulationController.m_focusSpeed.at(i)), focusSpeed.at(i));
    }
}

void SimulationControllerTest::testAngleRadFromXAxis_data()
{
    QTest::addColumn<QVector2D>("point1");
    QTest::addColumn<QVector2D>("point2");
    QTest::addColumn<double>("angle");

    QTest::newRow("0°") << QVector2D{1.0, 1.0} << QVector2D{2.0, 1.0} << 0.0;
    QTest::newRow("45°") << QVector2D{1.0, 1.0} << QVector2D{2.0, 2.0} << 0.79;
    QTest::newRow("90°") << QVector2D{0.0, 0.0} << QVector2D{0.0, 2.0} << 1.57;
    QTest::newRow("135°") << QVector2D{-1.0, 1.0} << QVector2D{-2.0, 2.0} << 2.36;
    QTest::newRow("180°") << QVector2D{1.0, 0.0} << QVector2D{-2.0, 0.0} << 3.14;
    QTest::newRow("225°") << QVector2D{-1.0, -1.0} << QVector2D{-2.0, -2.0} << 3.93;
    QTest::newRow("270°") << QVector2D{-1.0, -1.0} << QVector2D{-1.0, -2.0} << 4.71;
    QTest::newRow("315°") << QVector2D{1.0, -1.0} << QVector2D{2.0, -2.0} << 5.5;
    QTest::newRow("360°") << QVector2D{1.0, -1.0} << QVector2D{2.0, -1.0} << 0.0;
}

void SimulationControllerTest::testAngleRadFromXAxis()
{
    SimulationController simulationController;

    QFETCH(QVector2D, point1);
    QFETCH(QVector2D, point2);

    const auto& vector = point2 - point1;
    QTEST(testFunction::limitPrecisionToTwoDigits(simulationController.angleRadFromXAxis(vector)), "angle");
}

void SimulationControllerTest::testRotateVectorRad_data()
{
    QTest::addColumn<QVector2D>("vector");
    QTest::addColumn<double>("angle");
    QTest::addColumn<QVector2D>("rotatedVector");

    QTest::newRow("0°") << QVector2D{1.0, 1.0} << 0.0 << QVector2D{1.0, 1.0};
    QTest::newRow("45°") << QVector2D{1.0, 1.0} << 0.79 << QVector2D{-0.006508, 1.414199};
    QTest::newRow("90°") << QVector2D{1.0, 1.0} << 1.57 << QVector2D{-0.999203, 1.000796};
    QTest::newRow("135°") << QVector2D{1.0, 1.0} << 2.36 << QVector2D{-1.414203, -0.005382};
    QTest::newRow("180°") << QVector2D{1.0, 1.0} << 3.14 << QVector2D{-1.001591, -0.998406};
    QTest::newRow("225°") << QVector2D{1.0, 1.0} << 3.93 << QVector2D{0.004256, -1.414207};
    QTest::newRow("270°") << QVector2D{1.0, 1.0} << 4.71 << QVector2D{0.997608, -1.002386};
    QTest::newRow("315°") << QVector2D{1.0, 1.0} << 5.5 << QVector2D{1.41421, 0.003129};
    QTest::newRow("360°") << QVector2D{1.0, 1.0} << 6.28 << QVector2D{1.00318, 0.99681};
}

void SimulationControllerTest::testRotateVectorRad()
{
    SimulationController simulationController;

    QFETCH(QVector2D, vector);
    QFETCH(double, angle);

    auto manipulatedVector = simulationController.rotateVectorRad(vector, angle);
    QFETCH(QVector2D, rotatedVector);
    QCOMPARE(testFunction::limitPrecisionToSixDigits(manipulatedVector.x()), testFunction::limitPrecisionToSixDigits(rotatedVector.x()));
    QCOMPARE(testFunction::limitPrecisionToSixDigits(manipulatedVector.y()), testFunction::limitPrecisionToSixDigits(rotatedVector.y()));
}

QTEST_GUILESS_MAIN(SimulationControllerTest)
#include "simulationControllerTest.moc"
