#include <QTest>
#include <QVector>

#include "../src/FigureAnalyzer.h"
#include "../src/WobbleFigureEditor.h"
#include "../src/simulationController.h"
#include "../src/figureEditorSettings.h"

#include "../src/editorDataTypes.h"

using precitec::scantracker::components::wobbleFigureEditor::FigureAnalyzer;
using precitec::scantracker::components::wobbleFigureEditor::WobbleFigureEditor;
using precitec::scanmaster::components::wobbleFigureEditor::SimulationController;
using precitec::scanmaster::components::wobbleFigureEditor::FigureEditorSettings;

namespace testFunction
{
    double limitPrecisionToThreeDigits(double value)
    {
        int copy = static_cast<int> (value * 1000);
        return static_cast<double> (copy) / 1000;
    }
}

class FigureAnalyzerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testResetInformation();
    void testCastSeam();
    void testCastWobble();
    void testCastOverlay();
    void testGetInformationFromFigureEditor();
    void testCalcEuclidieanDistance_data();
    void testCalcEuclidieanDistance();
    void testUpdateRouteProperties_data();
    void testUpdateRouteProperties();
    void testRouteParts_data();
    void testRouteParts();
    void testUpdateProperties_data();
    void testUpdateProperties();
    void testUpdateWobblePointDistance_data();
    void testUpdateWobblePointDistance();
    void testGetInformationFromSimulationController_data();
    void testGetInformationFromSimulationController();
    void testUpdateSimulationProperties();
    void testUpdateFocusSpeed();

private:
    QTemporaryDir m_dir;
};

void FigureAnalyzerTest::initTestCase()
{
    QVERIFY(m_dir.isValid());
    QVERIFY(QDir{}.mkpath(m_dir.filePath(QStringLiteral("config"))));
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());
}

void FigureAnalyzerTest::testCtor()
{
    FigureAnalyzer analyzer;

    QCOMPARE(analyzer.figureEditor(), nullptr);
    QCOMPARE(analyzer.pointCount(), 0);
    QCOMPARE(analyzer.routeLength(), 0.0);
    QCOMPARE(analyzer.startPoint(), QPointF{});
    QCOMPARE(analyzer.endPoint(), QPointF{});
    QCOMPARE(analyzer.figureWidth(), 0.0);
    QCOMPARE(analyzer.figureHeight(), 0.0);
    QCOMPARE(analyzer.figureTime(), 0.0);
    QCOMPARE(analyzer.m_isWobbleFigure, false);
    QVERIFY(analyzer.m_routeParts.empty());
    QVERIFY(analyzer.m_routeInformation.empty());
}

void FigureAnalyzerTest::testResetInformation()
{
    FigureAnalyzer analyzer;

    analyzer.m_pointCount = 2;
    analyzer.m_routeLength = 4.75;
    analyzer.m_startPoint = QPointF{1.0, 0.5};
    analyzer.m_endPoint = QPointF{-0.5, 1.5};
    analyzer.m_figureWidth = 6.0;
    analyzer.m_figureHeight = 8.25;
    analyzer.m_figureTime = 9.5;
    analyzer.m_isWobbleFigure = true;

    QCOMPARE(analyzer.pointCount(), 2);
    QCOMPARE(analyzer.routeLength(), 4.75);
    QCOMPARE(analyzer.startPoint(), QPointF(1.0, 0.5));
    QCOMPARE(analyzer.endPoint(), QPointF(-0.5, 1.5));
    QCOMPARE(analyzer.figureWidth(), 6.0);
    QCOMPARE(analyzer.figureHeight(), 8.25);
    QCOMPARE(analyzer.figureTime(), 9.5);
    QCOMPARE(analyzer.m_isWobbleFigure, true);

    analyzer.resetInformation();
    QCOMPARE(analyzer.pointCount(), 0);
    QCOMPARE(analyzer.routeLength(), 0.0);
    QCOMPARE(analyzer.startPoint(), QPointF{});
    QCOMPARE(analyzer.endPoint(), QPointF{});
    QCOMPARE(analyzer.figureWidth(), 0.0);
    QCOMPARE(analyzer.figureHeight(), 0.0);
    QCOMPARE(analyzer.figureTime(), 0.0);
    QCOMPARE(analyzer.m_isWobbleFigure, false);
}

void FigureAnalyzerTest::testCastSeam()
{
    FigureAnalyzer analyzer;

    std::vector<RTC6::seamFigure::command::Order> seamFigure;

    for (int i = 0; i < 10; i++)
    {
        RTC6::seamFigure::command::Order newOrder;
        newOrder.endPosition.first = i * 0.25;
        newOrder.endPosition.second = i * 0.3;
        newOrder.power = i * 0.05;
        newOrder.ringPower = i * 0.02;
        newOrder.velocity = i * 0.09;

        seamFigure.push_back(newOrder);
    }

    analyzer.castSeamToRouteInformation(seamFigure);
    QCOMPARE(analyzer.m_routeInformation.size(), seamFigure.size());

    for (std::size_t i = 0; i < analyzer.m_routeInformation.size(); i++)
    {
        QCOMPARE(analyzer.m_routeInformation.at(i).coordinates, seamFigure.at(i).endPosition);
        QCOMPARE(analyzer.m_routeInformation.at(i).speed, seamFigure.at(i).velocity);
    }
}

void FigureAnalyzerTest::testCastWobble()
{
    FigureAnalyzer analyzer;

    std::vector<RTC6::wobbleFigure::command::Order> wobbleFigure;

    for (int i = 0; i < 10; i++)
    {
        RTC6::wobbleFigure::command::Order newOrder;
        newOrder.endPosition.first = i * 0.25;
        newOrder.endPosition.second = i * 0.3;
        newOrder.power = i * 0.05;
        newOrder.ringPower = i * 0.02;

        wobbleFigure.push_back(newOrder);
    }

    analyzer.castWobbleToRouteInformation(wobbleFigure);
    QCOMPARE(analyzer.m_routeInformation.size(), wobbleFigure.size());

    for (std::size_t i = 0; i < analyzer.m_routeInformation.size(); i++)
    {
        QCOMPARE(analyzer.m_routeInformation.at(i).coordinates, wobbleFigure.at(i).endPosition);
    }
}

void FigureAnalyzerTest::testCastOverlay()
{
    FigureAnalyzer analyzer;

    std::vector<std::pair<double, double>> overlayFigure;

    for (int i = 0; i < 10; i++)
    {
        std::pair<double, double> newOrder;
        newOrder.first = i * 0.25;
        newOrder.second = i * 0.3;

        overlayFigure.push_back(newOrder);
    }

    analyzer.castOverlayToRouteInformation(overlayFigure);
    QCOMPARE(analyzer.m_routeInformation.size(), overlayFigure.size());

    for (std::size_t i = 0; i < analyzer.m_routeInformation.size(); i++)
    {
        QCOMPARE(analyzer.m_routeInformation.at(i).coordinates, overlayFigure.at(i));
    }
}

void FigureAnalyzerTest::testGetInformationFromFigureEditor()
{
    FigureAnalyzer analyzer;

    QVERIFY(!analyzer.getInformationFromFigureEditor());

    WobbleFigureEditor figureEditor;
    analyzer.setFigureEditor(&figureEditor);

    figureEditor.setFileType(FileType::Seam);
    QVERIFY(!analyzer.getInformationFromFigureEditor());
    QVERIFY(!analyzer.m_isWobbleFigure);
    figureEditor.setFileType(FileType::Wobble);
    QVERIFY(!analyzer.getInformationFromFigureEditor());
    QVERIFY(!analyzer.m_isWobbleFigure);
    figureEditor.setFileType(FileType::Overlay);
    QVERIFY(!analyzer.getInformationFromFigureEditor());
    QVERIFY(!analyzer.m_isWobbleFigure);

    std::vector<RTC6::seamFigure::command::Order> seamFigure;

    for (int i = 0; i < 10; i++)
    {
        RTC6::seamFigure::command::Order newOrder;
        newOrder.endPosition.first = i * 0.25;
        newOrder.endPosition.second = i * 0.3;
        newOrder.power = i * 0.05;
        newOrder.ringPower = i * 0.02;
        newOrder.velocity = i * 0.09;

        seamFigure.push_back(newOrder);
    }

    figureEditor.m_seamFigure.figure = seamFigure;
    QVERIFY(!figureEditor.m_seamFigure.figure.empty());
    figureEditor.setFileType(FileType::Seam);
    QVERIFY(analyzer.getInformationFromFigureEditor());
    QVERIFY(!analyzer.m_isWobbleFigure);
    figureEditor.setFileType(FileType::Wobble);
    QVERIFY(!analyzer.getInformationFromFigureEditor());
    QVERIFY(!analyzer.m_isWobbleFigure);
    figureEditor.setFileType(FileType::Overlay);
    QVERIFY(!analyzer.getInformationFromFigureEditor());
    QVERIFY(!analyzer.m_isWobbleFigure);
    QCOMPARE(analyzer.m_routeInformation.size(), seamFigure.size());
    for (std::size_t i = 0; i < analyzer.m_routeInformation.size(); i++)
    {
        QCOMPARE(analyzer.m_routeInformation.at(i).coordinates, seamFigure.at(i).endPosition);
        QCOMPARE(analyzer.m_routeInformation.at(i).speed, seamFigure.at(i).velocity);
    }
    figureEditor.m_seamFigure.figure = {};
    QVERIFY(figureEditor.m_seamFigure.figure.empty());
    figureEditor.setFileType(FileType::Seam);
    QVERIFY(!analyzer.getInformationFromFigureEditor());
    QVERIFY(!analyzer.m_isWobbleFigure);
    figureEditor.setFileType(FileType::Wobble);
    QVERIFY(!analyzer.getInformationFromFigureEditor());
    QVERIFY(!analyzer.m_isWobbleFigure);
    figureEditor.setFileType(FileType::Overlay);
    QVERIFY(!analyzer.getInformationFromFigureEditor());
    QVERIFY(!analyzer.m_isWobbleFigure);

    figureEditor.setFileType(FileType::Seam);
    QVERIFY(!analyzer.getInformationFromFigureEditor());
    QVERIFY(!analyzer.m_isWobbleFigure);
    figureEditor.setFileType(FileType::Wobble);
    std::vector<RTC6::wobbleFigure::command::Order> wobbleFigure;

    for (int i = 0; i < 10; i++)
    {
        RTC6::wobbleFigure::command::Order newOrder;
        newOrder.endPosition.first = i * 0.3;
        newOrder.endPosition.second = i * 0.25;
        newOrder.power = i * 0.05;
        newOrder.ringPower = i * 0.02;

        wobbleFigure.push_back(newOrder);
    }
    figureEditor.m_wobbelFigure.figure = wobbleFigure;
    QVERIFY(!figureEditor.m_wobbelFigure.figure.empty());
    QVERIFY(analyzer.getInformationFromFigureEditor());
    QVERIFY(analyzer.m_isWobbleFigure);
    figureEditor.setFileType(FileType::Overlay);
    QVERIFY(!analyzer.getInformationFromFigureEditor());
    QCOMPARE(analyzer.m_routeInformation.size(), wobbleFigure.size());
    for (std::size_t i = 0; i < analyzer.m_routeInformation.size(); i++)
    {
        QCOMPARE(analyzer.m_routeInformation.at(i).coordinates, wobbleFigure.at(i).endPosition);
    }
    figureEditor.m_wobbelFigure.figure = {};
    QVERIFY(figureEditor.m_wobbelFigure.figure.empty());
    figureEditor.setFileType(FileType::Seam);
    QVERIFY(!analyzer.getInformationFromFigureEditor());
    figureEditor.setFileType(FileType::Wobble);
    QVERIFY(!analyzer.getInformationFromFigureEditor());
    figureEditor.setFileType(FileType::Overlay);
    QVERIFY(!analyzer.getInformationFromFigureEditor());

    analyzer.m_isWobbleFigure = false;
    figureEditor.setFileType(FileType::Seam);
    QVERIFY(!analyzer.getInformationFromFigureEditor());
    QVERIFY(!analyzer.m_isWobbleFigure);
    figureEditor.setFileType(FileType::Wobble);
    QVERIFY(!analyzer.getInformationFromFigureEditor());
    QVERIFY(!analyzer.m_isWobbleFigure);
    figureEditor.setFileType(FileType::Overlay);
    std::vector<std::pair<double, double>> overlayFigure;

    for (int i = 0; i < 10; i++)
    {
        std::pair<double, double> newOrder;
        newOrder.first = i * 0.25;
        newOrder.second = i * 0.3;

        overlayFigure.push_back(newOrder);
    }
    figureEditor.m_overlayFigure.functionValues = overlayFigure;
    QVERIFY(!figureEditor.m_overlayFigure.functionValues.empty());
    QVERIFY(analyzer.getInformationFromFigureEditor());
    QVERIFY(!analyzer.m_isWobbleFigure);
    QCOMPARE(analyzer.m_routeInformation.size(), overlayFigure.size());
    for (std::size_t i = 0; i < analyzer.m_routeInformation.size(); i++)
    {
        QCOMPARE(analyzer.m_routeInformation.at(i).coordinates, overlayFigure.at(i));
    }
    figureEditor.m_overlayFigure.functionValues = {};
    QVERIFY(figureEditor.m_overlayFigure.functionValues.empty());
    figureEditor.setFileType(FileType::Seam);
    QVERIFY(!analyzer.getInformationFromFigureEditor());
    QVERIFY(!analyzer.m_isWobbleFigure);
    figureEditor.setFileType(FileType::Wobble);
    QVERIFY(!analyzer.getInformationFromFigureEditor());
    QVERIFY(!analyzer.m_isWobbleFigure);
    figureEditor.setFileType(FileType::Overlay);
    QVERIFY(!analyzer.getInformationFromFigureEditor());
    QVERIFY(!analyzer.m_isWobbleFigure);
}

void FigureAnalyzerTest::testCalcEuclidieanDistance_data()
{
    QTest::addColumn<QPointF>("firstPoint");
    QTest::addColumn<QPointF>("secondPoint");
    QTest::addColumn<double>("euclDistance");

    QTest::newRow("") << QPointF{0, 0} << QPointF{1, 0} << 1.0;
    QTest::newRow("") << QPointF{0, 1} << QPointF{0, 0} << 1.0;
    QTest::newRow("") << QPointF{0, 0} << QPointF{0, 1} << 1.0;
    QTest::newRow("") << QPointF{0, 1} << QPointF{0, 0} << 1.0;
    QTest::newRow("") << QPointF{-1, 0} << QPointF{1, 0} << 2.0;
    QTest::newRow("") << QPointF{1, 0} << QPointF{-1, 0} << 2.0;
    QTest::newRow("") << QPointF{0, 1} << QPointF{0, -1} << 2.0;
    QTest::newRow("") << QPointF{0, -1} << QPointF{0, 1} << 2.0;
}

void FigureAnalyzerTest::testCalcEuclidieanDistance()
{
    FigureAnalyzer analyzer;

    QFETCH(QPointF, firstPoint);
    QFETCH(QPointF, secondPoint);
    QTEST(analyzer.calculateEuclideanDistance(firstPoint, secondPoint), "euclDistance");
    QTEST(analyzer.calculateEuclideanDistance(secondPoint, firstPoint), "euclDistance");
}

typedef QVector<FigureAnalyzer::FigureInformation> myImprovedFigure;
void FigureAnalyzerTest::testUpdateRouteProperties_data()
{
    QTest::addColumn<myImprovedFigure>("figure");
    QTest::addColumn<double>("routeLength");
    QTest::addColumn<QPointF>("startPoint");
    QTest::addColumn<QPointF>("endPoint");
    QTest::addColumn<double>("figureWidth");
    QTest::addColumn<double>("figureHeight");
    QTest::addColumn<double>("figureTime");

    myImprovedFigure figure = {FigureAnalyzer::FigureInformation{{-1.5, -1.0}, 100.0},
    FigureAnalyzer::FigureInformation{{0, 1.0}, 1000.0},
    FigureAnalyzer::FigureInformation{{1.5, -1.0}, 500.0}};
    QTest::addRow("triangle") << figure << 5.0 << QPointF{-1.5, -1.0} << QPointF{1.5, -1.0} << 3.0 << 2.0 << 0.027;

    figure = {FigureAnalyzer::FigureInformation{{-1.5, -1.0}, 0.0},
    FigureAnalyzer::FigureInformation{{0, 1.0}, 0.0},
    FigureAnalyzer::FigureInformation{{1.5, -1.0}, 0.0}};
    QTest::addRow("triangleGlobalSpeed") << figure << 5.0 << QPointF{-1.5, -1.0} << QPointF{1.5, -1.0} << 3.0 << 2.0 << 5.0;

    figure = {FigureAnalyzer::FigureInformation{{0, 1.5}, 1000.0},
    FigureAnalyzer::FigureInformation{{0, -2.0}, 100.0},
    FigureAnalyzer::FigureInformation{{3, -2.0}, 250.0},
    FigureAnalyzer::FigureInformation{{3, 1.5}, 400.0},
    FigureAnalyzer::FigureInformation{{0, 1.5}, 200.0}};
    QTest::addRow("rectangle") << figure << 13.0 << QPointF{0.0, 1.5} << QPointF{0.0, 1.5} << 3.0 << 3.5 << 0.055;

    figure = {FigureAnalyzer::FigureInformation{{-3.0, 0.0}, 150.0}, FigureAnalyzer::FigureInformation{{-2.44, -0.29}, 300.0},
    FigureAnalyzer::FigureInformation{{-1.89, -0.48}, 600.0},
    FigureAnalyzer::FigureInformation{{-1.33, -0.48}, 100.0},
    FigureAnalyzer::FigureInformation{{-0.78, -0.29}, 100.0},
    FigureAnalyzer::FigureInformation{{-0.22, 0.0}, 200.0},
    FigureAnalyzer::FigureInformation{{0.33, 0.29}, 400.0},
    FigureAnalyzer::FigureInformation{{0.89, 0.48}, 800.0},
    FigureAnalyzer::FigureInformation{{1.44, 0.48}, 100.0},
    FigureAnalyzer::FigureInformation{{2.0, 0.29}, 300.0}};
    QTest::addRow("sinus") << figure << 5.339 << QPointF{-3.0, 0.0} << QPointF{2.0, 0.29} << 5.0 << 0.96 << 0.03;

    figure = {FigureAnalyzer::FigureInformation{{0.0, 0.0}, 100.0}, FigureAnalyzer::FigureInformation{{0.06, 0.16}, 100.0},
    FigureAnalyzer::FigureInformation{{0.21, 0.25}, 100.0},
    FigureAnalyzer::FigureInformation{{0.38, 0.22}, 100.0},
    FigureAnalyzer::FigureInformation{{0.49, 0.09}, 100.0},
    FigureAnalyzer::FigureInformation{{0.49, -0.09}, 100.0},
    FigureAnalyzer::FigureInformation{{0.38, -0.22}, 100.0},
    FigureAnalyzer::FigureInformation{{0.21, -0.25}, 100.0},
    FigureAnalyzer::FigureInformation{{0.06, -0.16}, 100.0},
    FigureAnalyzer::FigureInformation{{0.0, 0.0}, 100.0}};
    QTest::addRow("circle") << figure << 1.557 << QPointF{0.0, 0.0} << QPointF{0.0, 0.0} << 0.49 << 0.5 << 0.015;

    figure = {FigureAnalyzer::FigureInformation{{-0.25, 0.0}, 250.0},
    FigureAnalyzer::FigureInformation{{-0.23, 0.09}, 250.0},
    FigureAnalyzer::FigureInformation{{-0.19, 0.16}, 500.0},
    FigureAnalyzer::FigureInformation{{-0.12, 0.22}, 500.0},
    FigureAnalyzer::FigureInformation{{-0.04, 0.25}, 100.0},
    FigureAnalyzer::FigureInformation{{0.04, 0.25}, 100.0},
    FigureAnalyzer::FigureInformation{{0.13, 0.22}, 100.0},
    FigureAnalyzer::FigureInformation{{0.19, 0.16}, 300.0},
    FigureAnalyzer::FigureInformation{{0.24, 0.09}, 300.0},
    FigureAnalyzer::FigureInformation{{0.25, 0.0}, 300.0}};
    QTest::addRow("halfCircle") << figure << 0.786 << QPointF{-0.25, 0.0} << QPointF{0.25, 0.0} << 0.5 << 0.25 << 0.004;
}

void FigureAnalyzerTest::testUpdateRouteProperties()
{
    FigureAnalyzer analyzer;

    QFETCH(myImprovedFigure, figure);
    for (const auto &information : figure)
    {
        analyzer.m_routeInformation.push_back(FigureAnalyzer::FigureInformation{{information.coordinates.first, information.coordinates.second}, information.speed});
    }
    analyzer.updatePointCount();
    analyzer.updateStartPoint();
    analyzer.updateEndPoint();
    analyzer.updateFigureDimensions();
    analyzer.updateRouteProperties();

    QCOMPARE(analyzer.pointCount(), figure.size());
    QTEST(analyzer.startPoint(), "startPoint");
    QTEST(analyzer.endPoint(), "endPoint");
    QTEST(analyzer.figureWidth(), "figureWidth");
    QTEST(analyzer.figureHeight(), "figureHeight");
    QTEST(testFunction::limitPrecisionToThreeDigits(analyzer.routeLength()), "routeLength");
    QTEST(testFunction::limitPrecisionToThreeDigits(analyzer.figureTime()), "figureTime");
}

void FigureAnalyzerTest::testRouteParts_data()
{
    QTest::addColumn<myImprovedFigure>("figure");
    QTest::addColumn<bool>("isWobbleFigure");
    QTest::addColumn<unsigned int>("microVectorFactor");
    QTest::addColumn<QVector<double>>("partLength");
    QTest::addColumn<QVector<double>>("partSpeed");
    QTest::addColumn<QVector<double>>("partTime");

    myImprovedFigure figure = {FigureAnalyzer::FigureInformation{{-1.5, -1.0}, 100.0},
    FigureAnalyzer::FigureInformation{{0, 1.0}, 1000.0},
    FigureAnalyzer::FigureInformation{{1.5, -1.0}, 500.0}};
    QVector<double> partLength = {2.5, 2.5};
    QVector<double> partSpeed = {100.0, 1000.0};
    QVector<double> partTime = {0.025, 0.0025};
    QTest::addRow("triangle") << figure << false << 0u << partLength << partSpeed << partTime;

    figure = {FigureAnalyzer::FigureInformation{{-1.5, -1.0}, 0.0},
    FigureAnalyzer::FigureInformation{{0, 1.0}, 0.0},
    FigureAnalyzer::FigureInformation{{1.5, -1.0}, 0.0}};
    partLength = {2.5, 2.5};
    partSpeed = {1.0, 1.0};
    partTime = {2.5, 2.5};
    QTest::addRow("triangle2") << figure << false << 10u << partLength << partSpeed << partTime;

    figure = {FigureAnalyzer::FigureInformation{{-1.5, -1.0}, 0.0},
    FigureAnalyzer::FigureInformation{{0, 1.0}, 200.0},
    FigureAnalyzer::FigureInformation{{1.5, -1.0}, 0.0}};
    partLength = {0.05, 0.05};
    partSpeed = {5000.0, 5000.0};
    partTime = {0.00001, 0.00001};
    QTest::addRow("wobbleTriangle") << figure << true << 50u << partLength << partSpeed << partTime;

    figure = {FigureAnalyzer::FigureInformation{{-1.5, -1.0}, 0.0},
    FigureAnalyzer::FigureInformation{{0, 1.0}, 200.0},
    FigureAnalyzer::FigureInformation{{1.5, -1.0}, 0.0}};
    partLength = {0.025, 0.025};
    partSpeed = {2500.0, 2500.0};
    partTime = {0.00001, 0.00001};
    QTest::addRow("wobbleTriangle2") << figure << true << 100u << partLength << partSpeed << partTime;
}

void FigureAnalyzerTest::testRouteParts()
{
    FigureAnalyzer analyzer;

    QFETCH(myImprovedFigure, figure);
    for (const auto &information : figure)
    {
        analyzer.m_routeInformation.push_back(FigureAnalyzer::FigureInformation{{information.coordinates.first, information.coordinates.second}, information.speed});
    }

    QFETCH(bool, isWobbleFigure);
    QFETCH(unsigned int, microVectorFactor);
    analyzer.m_isWobbleFigure = isWobbleFigure;
    analyzer.m_microVectorFactor = microVectorFactor;
    analyzer.updateRouteProperties();

    const auto &partInformation = analyzer.routeParts();

    QCOMPARE(partInformation.size(), figure.size() - 1);
    QFETCH(QVector<double>, partLength);
    QFETCH(QVector<double>, partSpeed);
    QFETCH(QVector<double>, partTime);
    for (std::size_t i = 0; i < partInformation.size(); i++)
    {
        QCOMPARE(partInformation.at(i).length, partLength.at(i));
        QCOMPARE(partInformation.at(i).speed, partSpeed.at(i));
        QCOMPARE(partInformation.at(i).time, partTime.at(i));
    }
}

typedef QVector<QPair<double, double>> myFigure;
void FigureAnalyzerTest::testUpdateProperties_data()
{
    QTest::addColumn<myFigure>("figure");
    QTest::addColumn<double>("routeLength");
    QTest::addColumn<QPointF>("startPoint");
    QTest::addColumn<QPointF>("endPoint");
    QTest::addColumn<double>("figureWidth");
    QTest::addColumn<double>("figureHeight");
    QTest::addColumn<double>("figureTime");

    myFigure figure = {{-1.5, -1.0}, {0, 1.0}, {1.5, -1.0}};

    QTest::addRow("triangle") << figure << 5.0 << QPointF{-1.5, -1.0} << QPointF{1.5, -1.0} << 3.0 << 2.0 << 5.0;

    figure = {{0, 1.5}, {0, -2.0}, {3, -2.0}, {3, 1.5}, {0, 1.5}};
    QTest::addRow("rectangle") << figure << 13.0 << QPointF{0.0, 1.5} << QPointF{0.0, 1.5} << 3.0 << 3.5 << 13.0;

    figure = {{-3.0, 0.0}, {-2.44, -0.29}, {-1.89, -0.48}, {-1.33, -0.48}, {-0.78, -0.29}, {-0.22, 0.0}, {0.33, 0.29}, {0.89, 0.48}, {1.44, 0.48}, {2.0, 0.29}};
    QTest::addRow("sinus") << figure << 5.339 << QPointF{-3.0, 0.0} << QPointF{2.0, 0.29} << 5.0 << 0.96 << 5.339;

    figure = {{0.0, 0.0}, {0.06, 0.16}, {0.21, 0.25}, {0.38, 0.22}, {0.49, 0.09}, {0.49, -0.09}, {0.38, -0.22}, {0.21, -0.25}, {0.06, -0.16}, {0.0, 0.0}};
    QTest::addRow("circle") << figure << 1.557 << QPointF{0.0, 0.0} << QPointF{0.0, 0.0} << 0.49 << 0.5 << 1.557;

    figure = {{-0.25, 0.0}, {-0.23, 0.09}, {-0.19, 0.16}, {-0.12, 0.22}, {-0.04, 0.25}, {0.04, 0.25}, {0.13, 0.22}, {0.19, 0.16}, {0.24, 0.09}, {0.25, 0.0}};
    QTest::addRow("halfCircle") << figure << 0.786 << QPointF{-0.25, 0.0} << QPointF{0.25, 0.0} << 0.5 << 0.25 << 0.786;
}


void FigureAnalyzerTest::testUpdateProperties()
{
    FigureAnalyzer analyzer;

    QFETCH(myFigure, figure);
    for (const auto &point : figure)
    {
        const std::pair<double, double> &pair = {point.first, point.second};
        analyzer.m_routeInformation.push_back(FigureAnalyzer::FigureInformation{pair, 0.0});
    }
    analyzer.updatePointCount();
    analyzer.updateStartPoint();
    analyzer.updateEndPoint();
    analyzer.updateFigureDimensions();
    analyzer.updateRouteProperties();

    QCOMPARE(analyzer.pointCount(), figure.size());
    QTEST(testFunction::limitPrecisionToThreeDigits(analyzer.routeLength()), "routeLength");
    QTEST(analyzer.startPoint(), "startPoint");
    QTEST(analyzer.endPoint(), "endPoint");
    QTEST(analyzer.figureWidth(), "figureWidth");
    QTEST(analyzer.figureHeight(), "figureHeight");
    QTEST(testFunction::limitPrecisionToThreeDigits(analyzer.figureTime()), "figureTime");
}

void FigureAnalyzerTest::testUpdateWobblePointDistance_data()
{
    QTest::addColumn<int>("wobbleFigurePointSize");
    QTest::addColumn<int>("microVectorFactor");
    QTest::addColumn<double>("speed");
    QTest::addColumn<double>("wobbleGap");

    QTest::addRow("840 Hz") << 8 << 17 << 500.0 << 0.595;
    QTest::addRow("500 Hz") << 5 << 50 << 500.0 << 1.0;
    QTest::addRow("500 Hz") << 9 << 25 << 500.0 << 1.0;
    QTest::addRow("521 Hz") << 17 << 12 << 500.0 << 0.96;

}

void FigureAnalyzerTest::testUpdateWobblePointDistance()
{
    FigureAnalyzer analyzer;

    QFETCH(double, speed);
    FigureEditorSettings::instance()->setScannerSpeed(speed);
    QCOMPARE(FigureEditorSettings::instance()->scannerSpeed(), speed);

    QFETCH(int, wobbleFigurePointSize);
    QFETCH(int, microVectorFactor);

    analyzer.m_microVectorFactor = microVectorFactor;
    analyzer.m_wobbleFigurePointCount = wobbleFigurePointSize;
    analyzer.updateWobblePointDistance();

    QFETCH(double, wobbleGap);
    QCOMPARE(analyzer.m_wobblePointDistance, wobbleGap);
}

void FigureAnalyzerTest::testGetInformationFromSimulationController_data()
{
    QTest::addColumn<myFigure>("figure");
    QTest::addColumn<int>("microVectorFactor");

    QTest::addRow("2Circles") << myFigure {
        {0.97, 0.0},
        {0.68, 0.71},
        {-0.02, 1.0},
        {-0.73, 0.71},
        {-1.02, 0.0},
        {-0.72, -0.71},
        {-0.01, -1.0},
        {0.7, 0.71},
        {0.99, 0.0},
        {0.7, -0.71},
        {0.0, 1.0},
        {-0.7, 0.71},
        {-0.99, 0.0},
        {-0.7, -0.71},
        {0.01, -1.0},
        {0.72, -0.71},
        {1.02, 0.0}
    } << 1;
}

void FigureAnalyzerTest::testGetInformationFromSimulationController()
{
    FigureAnalyzer analyzer;
    SimulationController simulationController;

    QVERIFY(!analyzer.getInformationFromSimulationController());
    QVERIFY(analyzer.m_routeInformation.empty());

    analyzer.setSimulationController(&simulationController);
    QVERIFY(!analyzer.getInformationFromSimulationController());

    QFETCH(myFigure, figure);
    QFETCH(int, microVectorFactor);
    RTC6::wobbleFigure::Figure wobbleFigure;
    wobbleFigure.microVectorFactor = microVectorFactor;
    for (const auto& point : figure)
    {
        RTC6::wobbleFigure::command::Order newOrder;
        newOrder.endPosition = std::make_pair(point.first, point.second);
        wobbleFigure.figure.push_back(newOrder);
    }

    simulationController.m_wobbleFigure = wobbleFigure;
    QVERIFY(!analyzer.getInformationFromSimulationController());

    RTC6::seamFigure::SeamFigure seamFigure;
    for (const auto& point : figure)
    {
        RTC6::seamFigure::command::Order newOrder;
        newOrder.endPosition = std::make_pair(point.first, point.second);
        seamFigure.figure.emplace_back(newOrder);
    }
    simulationController.m_simulatedSeamFigure = seamFigure;

    QVERIFY(analyzer.getInformationFromSimulationController());
    QVERIFY(!analyzer.m_routeInformation.empty());
}

void FigureAnalyzerTest::testUpdateSimulationProperties()
{
    FigureAnalyzer analyzer;
    SimulationController simulationController;

    analyzer.m_routeInformation.emplace_back(FigureAnalyzer::FigureInformation{{1.5, 0.3}, 0.0});
    analyzer.m_wobbleFigurePointSize = 5;
    analyzer.m_wobblePointDistance = 2;
    QVERIFY(!analyzer.m_routeInformation.empty());
    QCOMPARE(analyzer.m_wobbleFigurePointSize, 5);
    QCOMPARE(analyzer.m_wobblePointDistance, 2);

    analyzer.updateSimulationProperties();
    QVERIFY(!analyzer.m_routeInformation.empty());
    QCOMPARE(analyzer.m_wobbleFigurePointSize, 0);
    QCOMPARE(analyzer.wobblePointDistance(), 0);

    analyzer.setSimulationController(&simulationController);
    analyzer.updateSimulationProperties();
    QVERIFY(!analyzer.m_routeInformation.empty());
    QCOMPARE(analyzer.m_wobbleFigurePointSize, 0);
    QCOMPARE(analyzer.wobblePointDistance(), 0);

    simulationController.setReady(true);
    analyzer.updateSimulationProperties();
    QVERIFY(!analyzer.m_routeInformation.empty());
    QCOMPARE(analyzer.m_wobbleFigurePointSize, 0);
    QCOMPARE(analyzer.wobblePointDistance(), 0);
}

void FigureAnalyzerTest::testUpdateFocusSpeed()
{
    FigureAnalyzer analyzer;
    SimulationController simulationController;

    simulationController.m_focusSpeed = std::vector<double> {100.0, 200.0, 300.0, 400.0, 500.0};
    analyzer.setSimulationController(&simulationController);
    analyzer.updateFocusSpeed();

    QCOMPARE(analyzer.minFocusSpeed(), 100.0);
    QCOMPARE(analyzer.maxFocusSpeed(), 500.0);
    QCOMPARE(analyzer.averageFocusSpeed(), 300.0);

    simulationController.m_focusSpeed = std::vector<double> {500.0, 750.0, 250.0, 500.0};
    analyzer.updateFocusSpeed();

    QCOMPARE(analyzer.minFocusSpeed(), 250.0);
    QCOMPARE(analyzer.maxFocusSpeed(), 750.0);
    QCOMPARE(analyzer.averageFocusSpeed(), 500.0);
}

QTEST_GUILESS_MAIN(FigureAnalyzerTest)
#include "figureAnalyzerTest.moc"
