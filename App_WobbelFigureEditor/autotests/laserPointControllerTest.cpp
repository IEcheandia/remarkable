#include <QTest>
#include <QSignalSpy>
#include <QVector>

#include "../src/laserPointController.h"
#include "../src/WobbleFigure.h"

#include "../src/LaserPoint.h"
#include "../src/LaserTrajectory.h"

#include "../src/fileType.h"
#include "../src/editorDataTypes.h"

using precitec::scanmaster::components::wobbleFigureEditor::LaserPointController;
using precitec::scantracker::components::wobbleFigureEditor::WobbleFigure;

using precitec::scantracker::components::wobbleFigureEditor::LaserPoint;
using precitec::scantracker::components::wobbleFigureEditor::LaserTrajectory;

using precitec::scantracker::components::wobbleFigureEditor::FileType;
using RTC6::seamFigure::SeamFigure;
using RTC6::wobbleFigure::Figure;
using RTC6::function::OverlayFunction;

namespace testFunction
{

double calculatePowerFromQMLPercentToCpp(double value)
{
    return value * 0.01;
}

SeamFigure createSeamFigureFromQPointFs(const QVector<QPointF>& points)
{
    SeamFigure seam;
    seam.name = std::string("Test");
    seam.ID = std::string("1");
    seam.description = std::string("Test description!");

    int i = 0;
    for (const auto &point : points)
    {
        RTC6::seamFigure::command::Order newOrder;
        newOrder.endPosition = std::make_pair(point.x(), point.y());
        if (i == points.size())
        {
            newOrder.power = -1.0;
            newOrder.ringPower = -1.0;
            newOrder.velocity = -1.0;
        }
        else
        {
            newOrder.power = i * 0.075;
            newOrder.ringPower = i * 0.03;
            newOrder.velocity = i * 1.5;
        }
        seam.figure.push_back(newOrder);
        i++;
    }
    return seam;
}

Figure createWobbleFigureFromQPointFs(const QVector<QPointF>& points)
{
    Figure wobble;
    wobble.name = std::string("Wobble test");
    wobble.ID = std::string("10");
    wobble.description = std::string("I don't know what to write!");
    wobble.microVectorFactor = 50;
    wobble.powerModulationMode = 8;

    int i = 0;
    for (const auto &point : points)
    {
        RTC6::wobbleFigure::command::Order newOrder;
        newOrder.endPosition = std::make_pair(point.x(), point.y());
        if (i == points.size())
        {
            newOrder.power = -1.0;
            newOrder.ringPower = -1.0;
        }
        else
        {
            newOrder.power = i * 0.05;
            newOrder.ringPower = i * 0.10;
        }
        wobble.figure.push_back(newOrder);
        i++;
    }
    return wobble;
}

OverlayFunction createOverlayFigureFromQPointFs(const QVector<QPointF>& points)
{
    OverlayFunction overlay;
    overlay.name = std::string("Overlay test");
    overlay.ID = std::string("5");
    overlay.description = std::string("This is an overlay function, seems to be useless...");

    for (const auto &point : points)
    {
        overlay.functionValues.emplace_back(point.x(), point.y());
    }
    return overlay;
}

}

class LaserPointControllerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testFigure();
    void testFigureScale();
    void testFileType();
    void testPowerAnalog();
    void testDrawSeamFigure_data();
    void testDrawSeamFigure();
    void testDrawWobbleFigure_data();
    void testDrawWobbleFigure();
    void testDrawOverlayFunction_data();
    void testDrawOverlayFunction();
    void testDrawBasicFigure_data();
    void testDrawBasicFigure();
    void testDrawSimulatedFigure_data();
    void testDrawSimulatedFigure();
    void testDrawFigureFromCreator_data();
    void testDrawFigureFromCreator();
    void testApplyOffset_data();
    void testApplyOffset();
    void testDeletePoint_data();
    void testDeletePoint();
    void testSetNewIDs_data();
    void testSetNewIDs();
    void testCloseCreatedGap_data();
    void testCloseCreatedGap();
    void testNewStartPoint_data();
    void testNewStartPoint();
    void testMirrorYAxis_data();
    void testMirrorYAxis();
    void testClearPoints();
    void testClearFigure();
    void testChangeFigureScale_data();
    void testChangeFigureScale();
    void testCalculateFigureCenter_data();
    void testCalculateFigureCenter();
    void testDeleteAllEdges_data();
    void testDeleteAllEdges();
    void testDrawRamps();
    void testChangeRampVisibility();
    void testCreateRampPoint();
    void testChangeRampPointVisibility();
    void testCreateRampTrajectory();
    void testChangeRampTrajectoryVisibility();
    void testDeleteRampPoints();
    void testInvertY();

private:
    QTemporaryDir m_dir;
    QQmlEngine *m_engine = nullptr;
    WobbleFigure *m_wobbleFigure = nullptr;
    std::unique_ptr<QQmlComponent> m_laserPointComponent;
    std::unique_ptr<QQmlComponent> m_laserTrajectoryComponent;
};

void LaserPointControllerTest::initTestCase()
{
    QVERIFY(m_dir.isValid());
    QVERIFY(QDir{}.mkpath(m_dir.filePath(QStringLiteral("config"))));
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());

    m_engine = new QQmlEngine{this};
    QStringList paths = m_engine->importPathList();
    paths << QStringLiteral("/usr/lib/x86_64-linux-gnu/qt5/qml");
    m_engine->setImportPathList(paths);

    qmlRegisterType<precitec::scantracker::components::wobbleFigureEditor::LaserPoint>("figureeditor.components", 1, 0, "LaserPoint");
    qmlRegisterType<precitec::scantracker::components::wobbleFigureEditor::LaserTrajectory>("figureeditor.components", 1, 0, "LaserTrajectory");

    m_wobbleFigure = new WobbleFigure{};
    m_engine->setContextForObject(m_wobbleFigure, m_engine->rootContext());

    QQuickItem figureContainer;
    m_wobbleFigure->setContainerItem(new QQuickItem{});
    m_wobbleFigure->getContainerItem()->setX(0);
    m_wobbleFigure->getContainerItem()->setY(0);
    m_wobbleFigure->getContainerItem()->setWidth(10000);
    m_wobbleFigure->getContainerItem()->setHeight(10000);

    m_laserPointComponent = std::make_unique<QQmlComponent>(m_engine);
    m_laserPointComponent->setData(QByteArrayLiteral("import QtQuick 2.7; import figureeditor.components 1.0; LaserPoint {}"), {});
    m_laserTrajectoryComponent = std::make_unique<QQmlComponent>(m_engine);
    m_laserTrajectoryComponent->setData(QByteArrayLiteral("import QtQuick 2.7; import figureeditor.components 1.0; LaserTrajectory {}"), {});

    m_wobbleFigure->setProperty("nodeDelegate", QVariant::fromValue(m_laserPointComponent.get()));
    m_wobbleFigure->setProperty("edgeDelegate", QVariant::fromValue(m_laserTrajectoryComponent.get()));
}

void LaserPointControllerTest::testCtor()
{
    LaserPointController laserPointController;

    QCOMPARE(laserPointController.figure(), nullptr);
    QCOMPARE(laserPointController.figureScale(), 1000);
    QCOMPARE(laserPointController.fileType(), FileType::Seam);
    QVERIFY(laserPointController.powerAnalog());
}

void LaserPointControllerTest::testFigure()
{
    LaserPointController laserPointController;
    WobbleFigure figure;

    QSignalSpy figureChanged{&laserPointController, &LaserPointController::figureChanged};
    QVERIFY(figureChanged.isValid());
    QCOMPARE(figureChanged.count(), 0);

    laserPointController.setFigure(&figure);
    QCOMPARE(figureChanged.count(), 1);
    QCOMPARE(laserPointController.figure(), &figure);

    laserPointController.setFigure(&figure);
    QCOMPARE(figureChanged.count(), 1);
    QCOMPARE(laserPointController.figure(), &figure);

    laserPointController.setFigure(nullptr);
    QCOMPARE(figureChanged.count(), 2);
    QCOMPARE(laserPointController.figure(), nullptr);
}

void LaserPointControllerTest::testFigureScale()
{
    LaserPointController laserPointController;

    QSignalSpy figureScaleChanged{&laserPointController, &LaserPointController::figureScaleChanged};
    QVERIFY(figureScaleChanged.isValid());
    QCOMPARE(figureScaleChanged.count(), 0);

    laserPointController.setFigureScale(1000);
    QCOMPARE(figureScaleChanged.count(), 0);
    QCOMPARE(laserPointController.figureScale(), 1000);
    QCOMPARE(laserPointController.m_oldFigureScale, 1);

    laserPointController.setFigureScale(100);
    QCOMPARE(figureScaleChanged.count(), 1);
    QCOMPARE(laserPointController.figureScale(), 100);
    QCOMPARE(laserPointController.m_oldFigureScale, 1000);

    laserPointController.setFigureScale(10000);
    QCOMPARE(figureScaleChanged.count(), 2);
    QCOMPARE(laserPointController.figureScale(), 10000);
    QCOMPARE(laserPointController.m_oldFigureScale, 100);
}

void LaserPointControllerTest::testFileType()
{
    LaserPointController laserPointController;

    QSignalSpy fileTypeChanged{&laserPointController, &LaserPointController::fileTypeChanged};
    QVERIFY(fileTypeChanged.isValid());
    QCOMPARE(fileTypeChanged.count(), 0);

    laserPointController.setFileType(FileType::Seam);
    QCOMPARE(fileTypeChanged.count(), 0);
    QCOMPARE(laserPointController.fileType(), FileType::Seam);

    laserPointController.setFileType(FileType::Wobble);
    QCOMPARE(fileTypeChanged.count(), 1);
    QCOMPARE(laserPointController.fileType(), FileType::Wobble);

    laserPointController.setFileType(FileType::Seam);
    QCOMPARE(fileTypeChanged.count(), 2);
    QCOMPARE(laserPointController.fileType(), FileType::Seam);

    laserPointController.setFileType(FileType::Overlay);
    QCOMPARE(fileTypeChanged.count(), 3);
    QCOMPARE(laserPointController.fileType(), FileType::Overlay);

    laserPointController.setFileType(FileType::None);
    QCOMPARE(fileTypeChanged.count(), 4);
    QCOMPARE(laserPointController.fileType(), FileType::None);
}

void LaserPointControllerTest::testPowerAnalog()
{
    LaserPointController laserPointController;

    QSignalSpy powerAnalogChanged{&laserPointController, &LaserPointController::powerAnalogChanged};
    QVERIFY(powerAnalogChanged.isValid());
    QCOMPARE(powerAnalogChanged.count(), 0);

    laserPointController.setPowerAnalog(true);
    QCOMPARE(powerAnalogChanged.count(), 0);
    QCOMPARE(laserPointController.powerAnalog(), true);

    laserPointController.setPowerAnalog(false);
    QCOMPARE(powerAnalogChanged.count(), 1);
    QCOMPARE(laserPointController.powerAnalog(), false);

    laserPointController.setPowerAnalog(true);
    QCOMPARE(powerAnalogChanged.count(), 2);
    QCOMPARE(laserPointController.powerAnalog(), true);
}

void LaserPointControllerTest::testDrawSeamFigure_data()
{
    QTest::addColumn<QVector<QPointF>>("figure");

    QTest::newRow("LineX") << QVector<QPointF> {
        QPointF{0.0, 0.0},
        QPointF{0.5, 0.0},
        QPointF{0.75, 0.0},
        QPointF{2.5, 0.0},
        QPointF{11.342, 0.0},
        QPointF{14.025, 0.0},
        QPointF{15.0, 0.0}
    };
}

void LaserPointControllerTest::testDrawSeamFigure()
{
    QVERIFY(m_engine);
    QVERIFY(m_wobbleFigure);
    m_wobbleFigure->clear();

    LaserPointController laserPointController;
    laserPointController.m_qmlComponent = qan::Node::delegate(*m_engine);
    laserPointController.m_qmlComponentEdge = qan::Edge::delegate(*m_engine);
    laserPointController.setFigure(m_wobbleFigure);
    laserPointController.setFileType(FileType::Seam);

    QFETCH(QVector<QPointF>, figure);
    QVERIFY(!figure.empty());

    SeamFigure newSeamFigure{testFunction::createSeamFigureFromQPointFs(figure)};
    QVERIFY(!newSeamFigure.figure.empty());

    laserPointController.drawSeamFigure(newSeamFigure);

    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 1);

    for (int i = 0; i < figure.size(); i++)
    {
        const auto &expectedPoint = figure.at(i);
        const auto &createdPoint = qobject_cast<LaserPoint*> (m_wobbleFigure->get_nodes().at(i));
        QCOMPARE(createdPoint->center().x() / laserPointController.m_figureScale, expectedPoint.x());
        QCOMPARE(createdPoint->center().y() / laserPointController.m_figureScale, expectedPoint.y());
        QCOMPARE(createdPoint->getItem()->width(), laserPointController.m_laserPointSize);
        QCOMPARE(createdPoint->getItem()->height(), laserPointController.m_laserPointSize);
        QCOMPARE(createdPoint->ID(), i);
        QCOMPARE(createdPoint->type(), FileType::Seam);
        QCOMPARE(createdPoint->type(), laserPointController.m_fileType);
        if (i == figure.size())
        {
            QCOMPARE(createdPoint->laserPower(), -1.0);
            QCOMPARE(createdPoint->ringPower(), -1.0);
            QCOMPARE(createdPoint->velocity(), -1.0);
        }
        else
        {
            QCOMPARE(testFunction::calculatePowerFromQMLPercentToCpp(createdPoint->laserPower()), i * 0.075);
            QCOMPARE(testFunction::calculatePowerFromQMLPercentToCpp(createdPoint->ringPower()), i * 0.03);
            QCOMPARE(createdPoint->velocity(), i * 1.5);
        }
    }

    QCOMPARE(laserPointController.figure()->name(), "Test");
    QCOMPARE(laserPointController.figure()->ID(), 1);
    QCOMPARE(laserPointController.figure()->description(), "Test description!");

    laserPointController.drawSeamFigure(newSeamFigure);

    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 1);

    for (int i = 0; i < figure.size(); i++)
    {
        const auto &expectedPoint = figure.at(i);
        const auto &createdPoint = qobject_cast<LaserPoint*> (m_wobbleFigure->get_nodes().at(i));
        QCOMPARE(createdPoint->center().x() / laserPointController.m_figureScale, expectedPoint.x());
        QCOMPARE(createdPoint->center().y() / laserPointController.m_figureScale, expectedPoint.y());
        QCOMPARE(createdPoint->getItem()->width(), laserPointController.m_laserPointSize);
        QCOMPARE(createdPoint->getItem()->height(), laserPointController.m_laserPointSize);
        QCOMPARE(createdPoint->ID(), i);
        QCOMPARE(createdPoint->type(), FileType::Seam);
        QCOMPARE(createdPoint->type(), laserPointController.m_fileType);
        if (i == figure.size())
        {
            QCOMPARE(createdPoint->laserPower(), -1.0);
            QCOMPARE(createdPoint->ringPower(), -1.0);
            QCOMPARE(createdPoint->velocity(), -1.0);
        }
        else
        {
            QCOMPARE(testFunction::calculatePowerFromQMLPercentToCpp(createdPoint->laserPower()), i * 0.075);
            QCOMPARE(testFunction::calculatePowerFromQMLPercentToCpp(createdPoint->ringPower()), i * 0.03);
            QCOMPARE(createdPoint->velocity(), i * 1.5);
        }
    }

    QCOMPARE(laserPointController.figure()->name(), "Test");
    QCOMPARE(laserPointController.figure()->ID(), 1);
    QCOMPARE(laserPointController.figure()->description(), "Test description!");
}

void LaserPointControllerTest::testDrawWobbleFigure_data()
{
    QTest::addColumn<QVector<QPointF>>("figure");

    QTest::newRow("LineX") << QVector<QPointF> {
        QPointF{0.0, 0.0},
        QPointF{0.5, 0.0},
        QPointF{0.75, 0.0},
        QPointF{2.5, 0.0},
        QPointF{11.342, 0.0},
        QPointF{14.025, 0.0},
        QPointF{15.0, 0.0}
    };
}

void LaserPointControllerTest::testDrawWobbleFigure()
{
    QVERIFY(m_engine);
    QVERIFY(m_wobbleFigure);
    m_wobbleFigure->clear();

    LaserPointController laserPointController;
    laserPointController.m_qmlComponent = qan::Node::delegate(*m_engine);
    laserPointController.m_qmlComponentEdge = qan::Edge::delegate(*m_engine);
    laserPointController.setFigure(m_wobbleFigure);
    laserPointController.setFileType(FileType::Wobble);

    QFETCH(QVector<QPointF>, figure);
    QVERIFY(!figure.empty());

    Figure newWobbleFigure{testFunction::createWobbleFigureFromQPointFs(figure)};
    QVERIFY(!newWobbleFigure.figure.empty());

    laserPointController.drawWobbleFigure(newWobbleFigure);

    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 1);

    for (int i = 0; i < figure.size(); i++)
    {
        const auto &expectedPoint = figure.at(i);
        const auto &createdPoint = qobject_cast<LaserPoint*> (m_wobbleFigure->get_nodes().at(i));
        QCOMPARE(createdPoint->center().x() / laserPointController.m_figureScale, expectedPoint.x());
        QCOMPARE(createdPoint->center().y() / laserPointController.m_figureScale, expectedPoint.y());
        QCOMPARE(createdPoint->getItem()->width(), laserPointController.m_laserPointSize);
        QCOMPARE(createdPoint->getItem()->height(), laserPointController.m_laserPointSize);
        QCOMPARE(createdPoint->ID(), i);
        QCOMPARE(createdPoint->type(), FileType::Wobble);
        QCOMPARE(createdPoint->type(), laserPointController.m_fileType);
        if (i == figure.size())
        {
            QCOMPARE(createdPoint->laserPower(), -1.0);
            QCOMPARE(createdPoint->ringPower(), -1.0);
        }
        else
        {
            QCOMPARE(testFunction::calculatePowerFromQMLPercentToCpp(createdPoint->laserPower()), i * 0.05);
            QCOMPARE(testFunction::calculatePowerFromQMLPercentToCpp(createdPoint->ringPower()), i * 0.10);
        }
    }

    QCOMPARE(laserPointController.figure()->name(), "Wobble test");
    QCOMPARE(laserPointController.figure()->ID(), 10);
    QCOMPARE(laserPointController.figure()->description(), "I don't know what to write!");

    laserPointController.drawWobbleFigure(newWobbleFigure);

    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 1);

    for (int i = 0; i < figure.size(); i++)
    {
        const auto &expectedPoint = figure.at(i);
        const auto &createdPoint = qobject_cast<LaserPoint*> (m_wobbleFigure->get_nodes().at(i));
        QCOMPARE(createdPoint->center().x() / laserPointController.m_figureScale, expectedPoint.x());
        QCOMPARE(createdPoint->center().y() / laserPointController.m_figureScale, expectedPoint.y());
        QCOMPARE(createdPoint->getItem()->width(), laserPointController.m_laserPointSize);
        QCOMPARE(createdPoint->getItem()->height(), laserPointController.m_laserPointSize);
        QCOMPARE(createdPoint->ID(), i);
        QCOMPARE(createdPoint->type(), FileType::Wobble);
        QCOMPARE(createdPoint->type(), laserPointController.m_fileType);
        if (i == figure.size())
        {
            QCOMPARE(createdPoint->laserPower(), -1.0);
            QCOMPARE(createdPoint->ringPower(), -1.0);
        }
        else
        {
            QCOMPARE(testFunction::calculatePowerFromQMLPercentToCpp(createdPoint->laserPower()), i * 0.05);
            QCOMPARE(testFunction::calculatePowerFromQMLPercentToCpp(createdPoint->ringPower()), i * 0.10);
        }
    }

    QCOMPARE(laserPointController.figure()->name(), "Wobble test");
    QCOMPARE(laserPointController.figure()->ID(), 10);
    QCOMPARE(laserPointController.figure()->description(), "I don't know what to write!");
}

void LaserPointControllerTest::testDrawOverlayFunction_data()
{
    QTest::addColumn<QVector<QPointF>>("figure");

    QTest::newRow("LineX") << QVector<QPointF> {
        QPointF{0.0, 0.0},
        QPointF{0.5, 0.0},
        QPointF{0.75, 0.0},
        QPointF{2.5, 0.0},
        QPointF{11.342, 0.0},
        QPointF{14.025, 0.0},
        QPointF{15.0, 0.0}
    };
}

void LaserPointControllerTest::testDrawOverlayFunction()
{
    QVERIFY(m_engine);
    QVERIFY(m_wobbleFigure);
    m_wobbleFigure->clear();

    LaserPointController laserPointController;
    laserPointController.m_qmlComponent = qan::Node::delegate(*m_engine);
    laserPointController.m_qmlComponentEdge = qan::Edge::delegate(*m_engine);
    laserPointController.setFigure(m_wobbleFigure);
    laserPointController.setFileType(FileType::Overlay);

    QFETCH(QVector<QPointF>, figure);
    QVERIFY(!figure.empty());

    OverlayFunction newOverlayFunction{testFunction::createOverlayFigureFromQPointFs(figure)};
    QVERIFY(!newOverlayFunction.functionValues.empty());

    laserPointController.drawOverlayFigure(newOverlayFunction);

    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 1);

    for (int i = 0; i < figure.size(); i++)
    {
        const auto &expectedPoint = figure.at(i);
        const auto &createdPoint = qobject_cast<LaserPoint*> (m_wobbleFigure->get_nodes().at(i));
        QCOMPARE(createdPoint->center().x() / laserPointController.m_figureScale, expectedPoint.x());
        QCOMPARE(createdPoint->center().y() / laserPointController.m_figureScale, expectedPoint.y());
        QCOMPARE(createdPoint->getItem()->width(), laserPointController.m_laserPointSize);
        QCOMPARE(createdPoint->getItem()->height(), laserPointController.m_laserPointSize);
        QCOMPARE(createdPoint->ID(), i);
        QCOMPARE(createdPoint->type(), FileType::Overlay);
        QCOMPARE(createdPoint->type(), laserPointController.m_fileType);
    }

    QCOMPARE(laserPointController.figure()->name(), "Overlay test");
    QCOMPARE(laserPointController.figure()->ID(), 5);
    QCOMPARE(laserPointController.figure()->description(), "This is an overlay function, seems to be useless...");

    laserPointController.drawOverlayFigure(newOverlayFunction);

    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 1);

    for (int i = 0; i < figure.size(); i++)
    {
        const auto &expectedPoint = figure.at(i);
        const auto &createdPoint = qobject_cast<LaserPoint*> (m_wobbleFigure->get_nodes().at(i));
        QCOMPARE(createdPoint->center().x() / laserPointController.m_figureScale, expectedPoint.x());
        QCOMPARE(createdPoint->center().y() / laserPointController.m_figureScale, expectedPoint.y());
        QCOMPARE(createdPoint->getItem()->width(), laserPointController.m_laserPointSize);
        QCOMPARE(createdPoint->getItem()->height(), laserPointController.m_laserPointSize);
        QCOMPARE(createdPoint->ID(), i);
        QCOMPARE(createdPoint->type(), FileType::Overlay);
        QCOMPARE(createdPoint->type(), laserPointController.m_fileType);
    }

    QCOMPARE(laserPointController.figure()->name(), "Overlay test");
    QCOMPARE(laserPointController.figure()->ID(), 5);
    QCOMPARE(laserPointController.figure()->description(), "This is an overlay function, seems to be useless...");
}

void LaserPointControllerTest::testDrawBasicFigure_data()
{
    QTest::addColumn<QVector<QPointF>>("figure");

    QTest::newRow("LineX") << QVector<QPointF> {
        QPointF{0.0, 0.0},
        QPointF{0.5, 0.0},
        QPointF{0.75, 0.0},
        QPointF{2.5, 0.0},
        QPointF{11.342, 0.0},
        QPointF{14.025, 0.0},
        QPointF{15.0, 0.0}
    };
}

void LaserPointControllerTest::testDrawBasicFigure()
{
    QVERIFY(m_engine);
    QVERIFY(m_wobbleFigure);
    m_wobbleFigure->clear();

    LaserPointController laserPointController;
    laserPointController.m_qmlComponent = qan::Node::delegate(*m_engine);
    laserPointController.m_qmlComponentEdge = qan::Edge::delegate(*m_engine);
    laserPointController.setFigure(m_wobbleFigure);
    laserPointController.setFileType(FileType::Basic);

    QFETCH(QVector<QPointF>, figure);
    QVERIFY(!figure.empty());

    Figure newBasicFigure{testFunction::createWobbleFigureFromQPointFs(figure)};
    QVERIFY(!newBasicFigure.figure.empty());

    laserPointController.drawBasicFigure(newBasicFigure);

    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 1);

    for (int i = 0; i < figure.size(); i++)
    {
        const auto &expectedPoint = figure.at(i);
        const auto &createdPoint = qobject_cast<LaserPoint*> (m_wobbleFigure->get_nodes().at(i));
        QCOMPARE(createdPoint->center().x() / laserPointController.m_figureScale, expectedPoint.x());
        QCOMPARE(createdPoint->center().y() / laserPointController.m_figureScale, expectedPoint.y());
        QCOMPARE(createdPoint->getItem()->width(), laserPointController.m_laserPointSize);
        QCOMPARE(createdPoint->getItem()->height(), laserPointController.m_laserPointSize);
        QCOMPARE(createdPoint->ID(), i);
        QCOMPARE(createdPoint->type(), FileType::Basic);
        QCOMPARE(createdPoint->type(), laserPointController.m_fileType);
        //Check basic point properties
        QVERIFY(!createdPoint->getItem()->getSelectable());
        QVERIFY(!createdPoint->getItem()->getDraggable());
        QVERIFY(!createdPoint->getItem()->getResizable());
        QCOMPARE(createdPoint->laserPower(), -1.0);
        QCOMPARE(createdPoint->ringPower(), -1.0);
    }

    QCOMPARE(laserPointController.figure()->name(), "Wobble test");
    QCOMPARE(laserPointController.figure()->ID(), 10);
    QCOMPARE(laserPointController.figure()->description(), "I don't know what to write!");

    //Check if points are deleted at the beginning of a new drawing.
    laserPointController.drawBasicFigure(newBasicFigure);

    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 1);

    for (int i = 0; i < figure.size(); i++)
    {
        const auto &expectedPoint = figure.at(i);
        const auto &createdPoint = qobject_cast<LaserPoint*> (m_wobbleFigure->get_nodes().at(i));
        QCOMPARE(createdPoint->center().x() / laserPointController.m_figureScale, expectedPoint.x());
        QCOMPARE(createdPoint->center().y() / laserPointController.m_figureScale, expectedPoint.y());
        QCOMPARE(createdPoint->getItem()->width(), laserPointController.m_laserPointSize);
        QCOMPARE(createdPoint->getItem()->height(), laserPointController.m_laserPointSize);
        QCOMPARE(createdPoint->ID(), i);
        QCOMPARE(createdPoint->type(), FileType::Basic);
        QCOMPARE(createdPoint->type(), laserPointController.m_fileType);
        //Check basic point properties
        QVERIFY(!createdPoint->getItem()->getSelectable());
        QVERIFY(!createdPoint->getItem()->getDraggable());
        QVERIFY(!createdPoint->getItem()->getResizable());
        QCOMPARE(createdPoint->laserPower(), -1.0);
        QCOMPARE(createdPoint->ringPower(), -1.0);
    }

    QCOMPARE(laserPointController.figure()->name(), "Wobble test");
    QCOMPARE(laserPointController.figure()->ID(), 10);
    QCOMPARE(laserPointController.figure()->description(), "I don't know what to write!");
}

void LaserPointControllerTest::testDrawSimulatedFigure_data()
{
    QTest::addColumn<QVector<QPointF>>("figure");
    QTest::addColumn<unsigned int>("loopCountInPoints");
    QTest::addColumn<int>("deltaTFactorOf10us");
    QTest::addColumn<QPointF>("offset");

    QTest::newRow("LineX") << QVector<QPointF> {
        QPointF{0.0, 0.0},
        QPointF{0.5, 0.0},
        QPointF{0.75, 0.0},
        QPointF{1.0, 0.0},
        QPointF{1.342, 0.0},
        QPointF{1.525, 0.0},
        QPointF{2.0, 0.0},
        QPointF{3.342, 0.0},
        QPointF{4.025, 0.0},
        QPointF{5.5, 0.0},
        QPointF{6.342, 0.0},
        QPointF{7.025, 0.0},
        QPointF{8.5, 0.0},
        QPointF{11.342, 0.0},
        QPointF{14.025, 0.0},
        QPointF{15.0, 0.0}
    } << 0u << 1 << QPointF{0.0, 0.0};
    QTest::newRow("LineX2") << QVector<QPointF> {
        QPointF{0.0, 0.0},
        QPointF{0.5, 0.0},
        QPointF{0.75, 0.0},
        QPointF{1.0, 0.0},
        QPointF{1.342, 0.0},
        QPointF{1.525, 0.0},
        QPointF{2.0, 0.0},
        QPointF{3.342, 0.0},
        QPointF{4.025, 0.0},
        QPointF{5.5, 0.0},
        QPointF{6.342, 0.0},
        QPointF{7.025, 0.0},
        QPointF{8.5, 0.0},
        QPointF{11.342, 0.0},
        QPointF{14.025, 0.0},
        QPointF{15.0, 0.0}
    } << 0u << 5 << QPointF{0.0, 0.0};
    QTest::newRow("LineX3") << QVector<QPointF> {
        QPointF{0.0, 0.0},
        QPointF{0.5, 0.0},
        QPointF{0.75, 0.0},
        QPointF{1.0, 0.0},
        QPointF{1.342, 0.0},
        QPointF{1.525, 0.0},
        QPointF{2.0, 0.0},
        QPointF{3.342, 0.0},
        QPointF{4.025, 0.0},
        QPointF{5.5, 0.0},
        QPointF{6.342, 0.0},
        QPointF{7.025, 0.0},
        QPointF{8.5, 0.0},
        QPointF{11.342, 0.0},
        QPointF{14.025, 0.0},
        QPointF{15.0, 0.0}
    } << 7u << 1 << QPointF{1.671, 0.0};
    QTest::newRow("LineX4") << QVector<QPointF> {
        QPointF{0.0, 0.0},
        QPointF{0.5, 0.0},
        QPointF{0.75, 0.0},
        QPointF{1.0, 0.0},
        QPointF{1.342, 0.0},
        QPointF{1.525, 0.0},
        QPointF{2.0, 0.0},
        QPointF{3.342, 0.0},
        QPointF{4.025, 0.0},
        QPointF{5.5, 0.0},
        QPointF{6.342, 0.0},
        QPointF{7.025, 0.0},
        QPointF{8.5, 0.0},
        QPointF{11.342, 0.0},
        QPointF{14.025, 0.0},
        QPointF{15.0, 0.0}
    } << 7u << 2 << QPointF{2.0125, 0.0};
}

void LaserPointControllerTest::testDrawSimulatedFigure()
{
    QVERIFY(m_engine);
    QVERIFY(m_wobbleFigure);
    m_wobbleFigure->clear();

    LaserPointController laserPointController;
    laserPointController.m_qmlComponent = qan::Node::delegate(*m_engine);
    laserPointController.m_qmlComponentEdge = qan::Edge::delegate(*m_engine);
    laserPointController.setFigure(m_wobbleFigure);

    QFETCH(QVector<QPointF>, figure);
    QFETCH(unsigned int, loopCountInPoints);
    QFETCH(int, deltaTFactorOf10us);

    const auto& simulatedFigure = testFunction::createSeamFigureFromQPointFs(figure);
    std::pair<unsigned int, int> visualization = std::make_pair(loopCountInPoints, deltaTFactorOf10us);
    laserPointController.drawSimulatedFigure(simulatedFigure, visualization);

    if (loopCountInPoints == 0 && deltaTFactorOf10us == 1)
    {
        QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
        for (int i = 0; i < figure.size(); i++)
        {
            const auto &expectedPoint = figure.at(i);
            const auto &createdPoint = qobject_cast<LaserPoint*> (m_wobbleFigure->get_nodes().at(i));
            QCOMPARE(createdPoint->center().x() / laserPointController.m_figureScale, expectedPoint.x());
            QCOMPARE(createdPoint->center().y() / laserPointController.m_figureScale, expectedPoint.y());
            QCOMPARE(createdPoint->getItem()->width(), laserPointController.m_laserPointSize);
            QCOMPARE(createdPoint->getItem()->height(), laserPointController.m_laserPointSize);
            QCOMPARE(createdPoint->ID(), i);
            QCOMPARE(createdPoint->type(), laserPointController.m_fileType);
        }
    }
    else if (loopCountInPoints == 0 && deltaTFactorOf10us != 1)
    {
        QCOMPARE(laserPointController.figure()->get_node_count(), (figure.size() / deltaTFactorOf10us) + 1);
        for (int i = 0; i < (figure.size() / deltaTFactorOf10us) + 1; i++)
        {
            const auto &expectedPoint = figure.at(i * deltaTFactorOf10us);
            const auto &createdPoint = qobject_cast<LaserPoint*> (m_wobbleFigure->get_nodes().at(i));
            QCOMPARE(createdPoint->center().x() / laserPointController.m_figureScale, expectedPoint.x());
            QCOMPARE(createdPoint->center().y() / laserPointController.m_figureScale, expectedPoint.y());
            QCOMPARE(createdPoint->getItem()->width(), laserPointController.m_laserPointSize);
            QCOMPARE(createdPoint->getItem()->height(), laserPointController.m_laserPointSize);
            QCOMPARE(createdPoint->ID(), i);
            QCOMPARE(createdPoint->type(), laserPointController.m_fileType);
        }
    }
    else if (loopCountInPoints != 0 && deltaTFactorOf10us == 1)
    {
        QCOMPARE(laserPointController.figure()->get_node_count(), loopCountInPoints + 1);
        QFETCH(QPointF, offset);
        for (std::size_t i = 0u; i < laserPointController.figure()->get_node_count(); i++)
        {
            const auto &expectedPoint = figure.at(i);
            const auto &createdPoint = qobject_cast<LaserPoint*> (m_wobbleFigure->get_nodes().at(i));
            QCOMPARE(createdPoint->center().x() / laserPointController.m_figureScale, expectedPoint.x() - offset.x());
            QCOMPARE(createdPoint->center().y() / laserPointController.m_figureScale, expectedPoint.y() + offset.y());
            QCOMPARE(createdPoint->getItem()->width(), laserPointController.m_laserPointSize);
            QCOMPARE(createdPoint->getItem()->height(), laserPointController.m_laserPointSize);
            QCOMPARE(createdPoint->ID(), i);
            QCOMPARE(createdPoint->type(), laserPointController.m_fileType);
        }
    }
    else
    {
        QCOMPARE(laserPointController.figure()->get_node_count(), ((loopCountInPoints + 1) / deltaTFactorOf10us) + 1);
        QFETCH(QPointF, offset);
        for (std::size_t i = 0u; i < laserPointController.figure()->get_node_count(); i++)
        {
            const auto &expectedPoint = figure.at(i * deltaTFactorOf10us);
            const auto &createdPoint = qobject_cast<LaserPoint*> (m_wobbleFigure->get_nodes().at(i));
            QCOMPARE(createdPoint->center().x() / laserPointController.m_figureScale, expectedPoint.x() - offset.x());
            QCOMPARE(createdPoint->center().y() / laserPointController.m_figureScale, expectedPoint.y() + offset.y());
            QCOMPARE(createdPoint->getItem()->width(), laserPointController.m_laserPointSize);
            QCOMPARE(createdPoint->getItem()->height(), laserPointController.m_laserPointSize);
            QCOMPARE(createdPoint->ID(), i);
            QCOMPARE(createdPoint->type(), laserPointController.m_fileType);
        }
    }
}

void LaserPointControllerTest::testDrawFigureFromCreator_data()
{
    QTest::addColumn<QVector<QPointF>>("figure");

    QTest::newRow("LineX") << QVector<QPointF> {
        QPointF{0.0, 0.0},
        QPointF{0.5, 0.0},
        QPointF{0.75, 0.0},
        QPointF{2.5, 0.0},
        QPointF{11.342, 0.0},
        QPointF{14.025, 0.0},
        QPointF{15.0, 0.0}
    };
}

void LaserPointControllerTest::testDrawFigureFromCreator()
{
    QVERIFY(m_engine);
    QVERIFY(m_wobbleFigure);
    m_wobbleFigure->clear();

    LaserPointController laserPointController;
    laserPointController.m_qmlComponent = qan::Node::delegate(*m_engine);
    laserPointController.m_qmlComponentEdge = qan::Edge::delegate(*m_engine);
    laserPointController.setFigure(m_wobbleFigure);

    QFETCH(QVector<QPointF>, figure);

    laserPointController.drawFigureFromCreator(std::vector<QPointF>(figure.begin(), figure.end()));

    QCOMPARE(figure.size(), laserPointController.figure()->get_node_count());
    QCOMPARE(figure.size() - 1, laserPointController.figure()->get_edge_count());

    for (int i = 0; i < figure.size(); i++)
    {
        const auto &expectedPoint = figure.at(i);
        const auto &createdPoint = qobject_cast<LaserPoint*> (m_wobbleFigure->get_nodes().at(i));
        QCOMPARE(createdPoint->center().x() / laserPointController.m_figureScale, expectedPoint.x());
        QCOMPARE(createdPoint->center().y() / laserPointController.m_figureScale, expectedPoint.y());
        QCOMPARE(createdPoint->getItem()->width(), laserPointController.m_laserPointSize);
        QCOMPARE(createdPoint->getItem()->height(), laserPointController.m_laserPointSize);
        QCOMPARE(createdPoint->ID(), i);
        QCOMPARE(createdPoint->type(), laserPointController.m_fileType);
    }

    laserPointController.drawFigureFromCreator(std::vector<QPointF>(figure.begin(), figure.end()));

    QCOMPARE(2 * figure.size(), laserPointController.figure()->get_node_count());
    QCOMPARE((2 * figure.size()) - 1, laserPointController.figure()->get_edge_count());

    for (std::size_t i = 0u; i < laserPointController.figure()->get_node_count(); i++)
    {
        const auto &expectedPoint = figure.at(i % figure.size());
        const auto &createdPoint = qobject_cast<LaserPoint*> (m_wobbleFigure->get_nodes().at(i));
        QCOMPARE(createdPoint->center().x() / laserPointController.m_figureScale, expectedPoint.x());
        QCOMPARE(createdPoint->center().y() / laserPointController.m_figureScale, expectedPoint.y());
        QCOMPARE(createdPoint->getItem()->width(), laserPointController.m_laserPointSize);
        QCOMPARE(createdPoint->getItem()->height(), laserPointController.m_laserPointSize);
        QCOMPARE(createdPoint->ID(), i);
        QCOMPARE(createdPoint->type(), laserPointController.m_fileType);
    }
}

void LaserPointControllerTest::testApplyOffset_data()
{
    QTest::addColumn<QVector<QPointF>>("figure");
    QTest::addColumn<QPointF>("offset");

    QTest::newRow("LineX") << QVector<QPointF> {
        QPointF{0.0, 0.0},
        QPointF{0.5, 0.0},
        QPointF{0.75, 0.0},
        QPointF{2.5, 0.0},
        QPointF{11.342, 0.0},
        QPointF{14.025, 0.0},
        QPointF{15.0, 0.0}
    }
    << QPointF{-5.0, 4.95};
}

void LaserPointControllerTest::testApplyOffset()
{
    QVERIFY(m_engine);
    QVERIFY(m_wobbleFigure);
    m_wobbleFigure->clear();

    LaserPointController laserPointController;
    laserPointController.m_qmlComponent = qan::Node::delegate(*m_engine);
    laserPointController.m_qmlComponentEdge = qan::Edge::delegate(*m_engine);
    laserPointController.setFigure(m_wobbleFigure);
    laserPointController.setFileType(FileType::Seam);

    QFETCH(QVector<QPointF>, figure);
    QVERIFY(!figure.empty());

    SeamFigure newSeamFigure{testFunction::createSeamFigureFromQPointFs(figure)};
    QVERIFY(!newSeamFigure.figure.empty());

    laserPointController.drawSeamFigure(newSeamFigure);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());

    QFETCH(QPointF, offset);
    laserPointController.applyOffset(offset);

    for (std::size_t i = 0u; i < laserPointController.figure()->get_node_count(); i++)
    {
        const auto &laserPoint = qobject_cast<LaserPoint*> (laserPointController.figure()->get_nodes().at(i));
        QCOMPARE(laserPoint->center() / laserPointController.m_figureScale, figure.at(i) + offset);
    }
}

void LaserPointControllerTest::testDeletePoint_data()
{
    QTest::addColumn<QVector<QPointF>>("figure");

    QTest::newRow("LineX") << QVector<QPointF> {
        QPointF{0.0, 0.0},
        QPointF{0.5, 0.0},
        QPointF{0.75, 0.0},
        QPointF{2.5, 0.0},
        QPointF{11.342, 0.0},
        QPointF{14.025, 0.0},
        QPointF{15.0, 0.0}
    };
}

void LaserPointControllerTest::testDeletePoint()
{
    QVERIFY(m_engine);
    QVERIFY(m_wobbleFigure);
    m_wobbleFigure->clear();

    LaserPointController laserPointController;
    laserPointController.m_qmlComponent = qan::Node::delegate(*m_engine);
    laserPointController.m_qmlComponentEdge = qan::Edge::delegate(*m_engine);
    laserPointController.setFigure(m_wobbleFigure);
    laserPointController.setFileType(FileType::Seam);

    QFETCH(QVector<QPointF>, figure);
    QVERIFY(!figure.empty());

    SeamFigure newSeamFigure{testFunction::createSeamFigureFromQPointFs(figure)};
    QVERIFY(!newSeamFigure.figure.empty());

    laserPointController.drawSeamFigure(newSeamFigure);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 1);

    laserPointController.deletePoint(0);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size() - 1);
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 2);

    laserPointController.deletePoint(figure.size() - 1);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size() - 2);
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 3);

    laserPointController.deletePoint(3);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size() - 3);
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 5);

    laserPointController.deletePoint(figure.size());
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size() - 3);
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 5);
}

void LaserPointControllerTest::testSetNewIDs_data()
{
    QTest::addColumn<QVector<QPointF>>("figure");

    QTest::newRow("LineX") << QVector<QPointF> {
        QPointF{0.0, 0.0},
        QPointF{0.5, 0.0},
        QPointF{0.75, 0.0},
        QPointF{2.5, 0.0},
        QPointF{11.342, 0.0},
        QPointF{14.025, 0.0},
        QPointF{15.0, 0.0}
    };
}

void LaserPointControllerTest::testSetNewIDs()
{
    QVERIFY(m_engine);
    QVERIFY(m_wobbleFigure);
    m_wobbleFigure->clear();

    LaserPointController laserPointController;
    laserPointController.m_qmlComponent = qan::Node::delegate(*m_engine);
    laserPointController.m_qmlComponentEdge = qan::Edge::delegate(*m_engine);
    laserPointController.setFigure(m_wobbleFigure);
    laserPointController.setFileType(FileType::Seam);

    QFETCH(QVector<QPointF>, figure);
    QVERIFY(!figure.empty());

    SeamFigure newSeamFigure{testFunction::createSeamFigureFromQPointFs(figure)};
    QVERIFY(!newSeamFigure.figure.empty());

    laserPointController.drawSeamFigure(newSeamFigure);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());

    for (std::size_t i = 0u; i < laserPointController.figure()->get_node_count(); i++)
    {
        const auto &laserPoint = qobject_cast<LaserPoint*> (laserPointController.figure()->get_nodes().at(i));
        QCOMPARE(laserPoint->ID(), i);
    }

    laserPointController.deletePoint(0);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size() - 1);
    laserPointController.setNewIDs(0);
    for (std::size_t i = 0u; i < laserPointController.figure()->get_node_count(); i++)
    {
        const auto &laserPoint = qobject_cast<LaserPoint*> (laserPointController.figure()->get_nodes().at(i));
        QCOMPARE(laserPoint->ID(), i);
    }

    laserPointController.drawSeamFigure(newSeamFigure);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());

    for (std::size_t i = 0u; i < laserPointController.figure()->get_node_count(); i++)
    {
        const auto &laserPoint = qobject_cast<LaserPoint*> (laserPointController.figure()->get_nodes().at(i));
        QCOMPARE(laserPoint->ID(), i);
    }

    laserPointController.deletePoint(figure.size() - 1);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size() - 1);
    laserPointController.setNewIDs(figure.size() - 1);
    for (std::size_t i = 0u; i < laserPointController.figure()->get_node_count(); i++)
    {
        const auto &laserPoint = qobject_cast<LaserPoint*> (laserPointController.figure()->get_nodes().at(i));
        QCOMPARE(laserPoint->ID(), i);
    }

    laserPointController.drawSeamFigure(newSeamFigure);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());

    for (std::size_t i = 0u; i < laserPointController.figure()->get_node_count(); i++)
    {
        const auto &laserPoint = qobject_cast<LaserPoint*> (laserPointController.figure()->get_nodes().at(i));
        QCOMPARE(laserPoint->ID(), i);
    }

    laserPointController.deletePoint(3);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size() - 1);
    laserPointController.setNewIDs(3);
    for (std::size_t i = 0u; i < laserPointController.figure()->get_node_count(); i++)
    {
        const auto &laserPoint = qobject_cast<LaserPoint*> (laserPointController.figure()->get_nodes().at(i));
        QCOMPARE(laserPoint->ID(), i);
    }

    laserPointController.drawSeamFigure(newSeamFigure);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());

    for (std::size_t i = 0u; i < laserPointController.figure()->get_node_count(); i++)
    {
        const auto &laserPoint = qobject_cast<LaserPoint*> (laserPointController.figure()->get_nodes().at(i));
        QCOMPARE(laserPoint->ID(), i);
    }

    laserPointController.deletePoint(figure.size());
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    laserPointController.setNewIDs(figure.size());
    for (std::size_t i = 0u; i < laserPointController.figure()->get_node_count(); i++)
    {
        const auto &laserPoint = qobject_cast<LaserPoint*> (laserPointController.figure()->get_nodes().at(i));
        QCOMPARE(laserPoint->ID(), i);
    }
}

void LaserPointControllerTest::testCloseCreatedGap_data()
{
    QTest::addColumn<QVector<QPointF>>("figure");

    QTest::newRow("LineX") << QVector<QPointF> {
        QPointF{0.0, 0.0},
        QPointF{0.5, 0.0},
        QPointF{0.75, 0.0},
        QPointF{2.5, 0.0},
        QPointF{11.342, 0.0},
        QPointF{14.025, 0.0},
        QPointF{15.0, 0.0}
    };
}

void LaserPointControllerTest::testCloseCreatedGap()
{
    QVERIFY(m_engine);
    QVERIFY(m_wobbleFigure);
    m_wobbleFigure->clear();

    LaserPointController laserPointController;
    laserPointController.m_qmlComponent = qan::Node::delegate(*m_engine);
    laserPointController.m_qmlComponentEdge = qan::Edge::delegate(*m_engine);
    laserPointController.setFigure(m_wobbleFigure);
    laserPointController.setFileType(FileType::Seam);

    QFETCH(QVector<QPointF>, figure);
    QVERIFY(!figure.empty());

    SeamFigure newSeamFigure{testFunction::createSeamFigureFromQPointFs(figure)};
    QVERIFY(!newSeamFigure.figure.empty());

    laserPointController.drawSeamFigure(newSeamFigure);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 1);

    for (std::size_t i = 0; i < laserPointController.figure()->get_edge_count(); i++)
    {
        const auto &edge = qobject_cast<LaserTrajectory*> (laserPointController.figure()->get_edges().at(i));
        QCOMPARE(edge->ID(), i);
    }

    auto id = static_cast<int> (figure.size() * 0.5);
    laserPointController.deletePoint(id);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size() - 1);
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 3);
    laserPointController.closeCreatedGap(id);
    laserPointController.setNewIDs(id - 1);
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 2);
}

void LaserPointControllerTest::testNewStartPoint_data()
{
    QTest::addColumn<QVector<QPointF>>("figure");

    QTest::newRow("LineX") << QVector<QPointF> {
        QPointF{0.0, 0.0},
        QPointF{0.5, 0.0},
        QPointF{0.75, 0.0},
        QPointF{2.5, 0.0},
        QPointF{11.342, 0.0},
        QPointF{14.025, 0.0},
        QPointF{15.0, 0.0}
    };
    QTest::newRow("Circle") << QVector<QPointF> {
        QPointF{1.0, 0.0},
        QPointF{0.62, -0.78},
        QPointF{-0.22, -0.98},
        QPointF{-0.90, -0.43},
        QPointF{-0.90, 0.43},
        QPointF{-0.22, 0.97},
        QPointF{0.62, 0.78},
        QPointF{1.0, 0.0}
    };
}

void LaserPointControllerTest::testNewStartPoint()
{
    QVERIFY(m_engine);
    QVERIFY(m_wobbleFigure);
    m_wobbleFigure->clear();

    LaserPointController laserPointController;
    laserPointController.m_qmlComponent = qan::Node::delegate(*m_engine);
    laserPointController.m_qmlComponentEdge = qan::Edge::delegate(*m_engine);
    laserPointController.setFigure(m_wobbleFigure);
    laserPointController.setFileType(FileType::Seam);

    QFETCH(QVector<QPointF>, figure);
    QVERIFY(!figure.empty());

    SeamFigure newSeamFigure{testFunction::createSeamFigureFromQPointFs(figure)};
    QVERIFY(!newSeamFigure.figure.empty());

    laserPointController.drawSeamFigure(newSeamFigure);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 1);

    laserPointController.newStartPoint(3, true);
    QCOMPARE(laserPointController.figure()->get_node_count(), 0);
    QCOMPARE(laserPointController.figure()->get_edge_count(), 0);

    laserPointController.drawSeamFigure(newSeamFigure);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 1);

    laserPointController.newStartPoint(2, false);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size() - 2);
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 3);

    for (std::size_t i = 0u; i < laserPointController.figure()->get_node_count(); i++)
    {
        const auto &laserPoint = qobject_cast<LaserPoint*> (laserPointController.figure()->get_nodes().at(i));
        QCOMPARE(laserPoint->ID(), i);
    }

    for (std::size_t i = 0; i < laserPointController.figure()->get_edge_count(); i++)
    {
        const auto &edge = qobject_cast<LaserTrajectory*> (laserPointController.figure()->get_edges().at(i));
        QCOMPARE(edge->ID(), i);
    }
}

void LaserPointControllerTest::testMirrorYAxis_data()
{
    QTest::addColumn<QVector<QPointF>>("figure");

    QTest::newRow("LineX") << QVector<QPointF> {
        QPointF{0.0, 0.0},
        QPointF{0.5, 0.0},
        QPointF{0.75, 0.0},
        QPointF{2.5, 0.0},
        QPointF{11.342, 0.0},
        QPointF{14.025, 0.0},
        QPointF{15.0, 0.0}
    };
}

void LaserPointControllerTest::testMirrorYAxis()
{
    QVERIFY(m_engine);
    QVERIFY(m_wobbleFigure);
    m_wobbleFigure->clear();

    LaserPointController laserPointController;
    laserPointController.m_qmlComponent = qan::Node::delegate(*m_engine);
    laserPointController.m_qmlComponentEdge = qan::Edge::delegate(*m_engine);
    laserPointController.setFigure(m_wobbleFigure);
    laserPointController.setFileType(FileType::Seam);

    QFETCH(QVector<QPointF>, figure);
    QVERIFY(!figure.empty());

    SeamFigure newSeamFigure{testFunction::createSeamFigureFromQPointFs(figure)};
    QVERIFY(!newSeamFigure.figure.empty());

    laserPointController.drawSeamFigure(newSeamFigure);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 1);

    laserPointController.mirrorYAxis();
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 1);

    for (std::size_t i = 0u; i < laserPointController.figure()->get_node_count(); i++)
    {
        const auto &laserPoint = qobject_cast<LaserPoint*> (laserPointController.figure()->get_nodes().at(i));
        QCOMPARE(laserPoint->ID(), i);
        QCOMPARE(laserPoint->center().x() / laserPointController.m_figureScale, figure.at(i).x());
        QCOMPARE(laserPoint->center().y() / laserPointController.m_figureScale, -1.0 * figure.at(i).y());
    }
}

void LaserPointControllerTest::testClearPoints()
{
    QVERIFY(m_engine);
    QVERIFY(m_wobbleFigure);
    m_wobbleFigure->clear();

    LaserPointController laserPointController;
    laserPointController.m_qmlComponent = qan::Node::delegate(*m_engine);
    laserPointController.m_qmlComponentEdge = qan::Edge::delegate(*m_engine);
    laserPointController.setFigure(m_wobbleFigure);

    QVector<QPointF> figure {
        QPointF{0.0, 0.0},
        QPointF{0.5, 0.0},
        QPointF{0.75, 0.0},
        QPointF{2.5, 0.0},
        QPointF{11.342, 0.0},
        QPointF{14.025, 0.0},
        QPointF{15.0, 0.0}
    };

    laserPointController.drawFigureFromCreator(std::vector<QPointF>(figure.begin(), figure.end()));

    QCOMPARE(figure.size(), laserPointController.figure()->get_node_count());
    QCOMPARE(figure.size() - 1, laserPointController.figure()->get_edge_count());

    laserPointController.clearPoints();

    QCOMPARE(laserPointController.figure()->get_node_count(), 0);
    QCOMPARE(laserPointController.figure()->get_edge_count(), 0);
}

void LaserPointControllerTest::testClearFigure()
{
    QVERIFY(m_wobbleFigure);

    LaserPointController laserPointController;

    laserPointController.clearFigure();

    laserPointController.setFigure(m_wobbleFigure);

    m_wobbleFigure->setName(QStringLiteral("A new name!"));
    m_wobbleFigure->setID(10);
    m_wobbleFigure->setDescription("I have no idea what to write here!");
    m_wobbleFigure->setAnalogPower(false);

    QVERIFY(!m_wobbleFigure->name().isEmpty());
    QCOMPARE(m_wobbleFigure->ID(), 10);
    QVERIFY(!m_wobbleFigure->description().isEmpty());
    QVERIFY(!m_wobbleFigure->analogPower());

    laserPointController.clearFigure();
    QCOMPARE(m_wobbleFigure->name(), "Figure name");
    QCOMPARE(m_wobbleFigure->ID(), 0);
    QVERIFY(m_wobbleFigure->description().isEmpty());
    QVERIFY(m_wobbleFigure->analogPower());
}

void LaserPointControllerTest::testChangeFigureScale_data()
{
    QTest::addColumn<QVector<QPointF>>("figure");

    QTest::newRow("LineX") << QVector<QPointF> {
        QPointF{0.0, 0.0},
        QPointF{0.5, 0.0},
        QPointF{0.75, 0.0},
        QPointF{2.5, 0.0},
        QPointF{11.342, 0.0},
        QPointF{14.025, 0.0},
        QPointF{15.0, 0.0}
    };
}

void LaserPointControllerTest::testChangeFigureScale()
{
    QVERIFY(m_engine);
    QVERIFY(m_wobbleFigure);
    m_wobbleFigure->clear();

    LaserPointController laserPointController;
    laserPointController.m_qmlComponent = qan::Node::delegate(*m_engine);
    laserPointController.m_qmlComponentEdge = qan::Edge::delegate(*m_engine);
    laserPointController.setFigure(m_wobbleFigure);
    laserPointController.setFileType(FileType::Seam);

    QFETCH(QVector<QPointF>, figure);
    QVERIFY(!figure.empty());

    SeamFigure newSeamFigure{testFunction::createSeamFigureFromQPointFs(figure)};
    QVERIFY(!newSeamFigure.figure.empty());

    laserPointController.drawSeamFigure(newSeamFigure);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 1);

    for (std::size_t i = 0u; i < laserPointController.figure()->get_node_count(); i++)
    {
        const auto &laserPoint = qobject_cast<LaserPoint*> (laserPointController.figure()->get_nodes().at(i));
        QCOMPARE(laserPoint->ID(), i);
        QCOMPARE(laserPoint->center().x() / laserPointController.m_figureScale, figure.at(i).x());
        QCOMPARE(laserPoint->center().y() / laserPointController.m_figureScale, -1.0 * figure.at(i).y());
    }

    laserPointController.setFigureScale(100);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 1);

    for (std::size_t i = 0u; i < laserPointController.figure()->get_node_count(); i++)
    {
        const auto &laserPoint = qobject_cast<LaserPoint*> (laserPointController.figure()->get_nodes().at(i));
        QCOMPARE(laserPoint->ID(), i);
        QCOMPARE(laserPoint->center().x() / laserPointController.m_figureScale, figure.at(i).x());
        QCOMPARE(laserPoint->center().y() / laserPointController.m_figureScale, -1.0 * figure.at(i).y());
    }
}

void LaserPointControllerTest::testCalculateFigureCenter_data()
{
    QTest::addColumn<QPointF>("center");
    QTest::addColumn<QVector<QPointF>>("figure");

    QTest::newRow("LineX") << QPointF{7.5, 0.0}
    << QVector<QPointF> {
        QPointF{0.0, 0.0},
        QPointF{0.5, 0.0},
        QPointF{0.75, 0.0},
        QPointF{2.5, 0.0},
        QPointF{11.342, 0.0},
        QPointF{14.025, 0.0},
        QPointF{15.0, 0.0}
    };
    QTest::newRow("LineY") << QPointF{0.0, -0.5}
    << QVector<QPointF> {
        QPointF{0.0, 0.0},
        QPointF{0.0, 0.5},
        QPointF{0.0, 1.0}
    };
    QTest::newRow("Circle") << QPointF{0.05, 0.005}
    << QVector<QPointF> {
        QPointF{1.0, 0.0},
        QPointF{0.62, -0.78},
        QPointF{-0.22, -0.98},
        QPointF{-0.90, -0.43},
        QPointF{-0.90, 0.43},
        QPointF{-0.22, 0.97},
        QPointF{0.62, 0.78},
        QPointF{1.0, 0.0}
    };
    QTest::newRow("Rectangle") << QPointF{-0.5, -0.25}
    << QVector<QPointF> {
        QPointF{0.0, 0.0},
        QPointF{0.0, 0.5},
        QPointF{-1.0, 0.5},
        QPointF{-1.0, 0.0},
        QPointF{0.0, 0.0}
    };
}

void LaserPointControllerTest::testCalculateFigureCenter()
{
    QVERIFY(m_engine);
    QVERIFY(m_wobbleFigure);
    m_wobbleFigure->clear();

    LaserPointController laserPointController;
    laserPointController.m_qmlComponent = qan::Node::delegate(*m_engine);
    laserPointController.m_qmlComponentEdge = qan::Edge::delegate(*m_engine);
    laserPointController.setFigure(m_wobbleFigure);
    laserPointController.setFileType(FileType::Seam);

    QFETCH(QVector<QPointF>, figure);
    QVERIFY(!figure.empty());

    SeamFigure newSeamFigure{testFunction::createSeamFigureFromQPointFs(figure)};
    QVERIFY(!newSeamFigure.figure.empty());

    laserPointController.drawSeamFigure(newSeamFigure);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 1);

    QFETCH(QPointF, center);
    QCOMPARE(laserPointController.calculateFigureCenter() / laserPointController.m_figureScale, center);
}

void LaserPointControllerTest::testDeleteAllEdges_data()
{
    QTest::addColumn<QVector<QPointF>>("figure");

    QTest::newRow("LineX") << QVector<QPointF> {
        QPointF{0.0, 0.0},
        QPointF{0.5, 0.0},
        QPointF{0.75, 0.0},
        QPointF{2.5, 0.0},
        QPointF{11.342, 0.0},
        QPointF{14.025, 0.0},
        QPointF{15.0, 0.0}
    };
}

void LaserPointControllerTest::testDeleteAllEdges()
{
    QVERIFY(m_engine);
    QVERIFY(m_wobbleFigure);
    m_wobbleFigure->clear();

    LaserPointController laserPointController;
    laserPointController.m_qmlComponent = qan::Node::delegate(*m_engine);
    laserPointController.m_qmlComponentEdge = qan::Edge::delegate(*m_engine);
    laserPointController.setFigure(m_wobbleFigure);
    laserPointController.setFileType(FileType::Seam);

    QFETCH(QVector<QPointF>, figure);
    QVERIFY(!figure.empty());

    SeamFigure newSeamFigure{testFunction::createSeamFigureFromQPointFs(figure)};
    QVERIFY(!newSeamFigure.figure.empty());

    laserPointController.drawSeamFigure(newSeamFigure);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 1);
    laserPointController.deleteAllEdges();
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    QCOMPARE(laserPointController.figure()->get_edge_count(), 0);
}

void LaserPointControllerTest::testDrawRamps()
{
    QVERIFY(m_engine);
    QVERIFY(m_wobbleFigure);
    m_wobbleFigure->clear();

    LaserPointController laserPointController;
    laserPointController.m_qmlComponent = qan::Node::delegate(*m_engine);
    laserPointController.m_qmlComponentEdge = qan::Edge::delegate(*m_engine);
    laserPointController.setFigure(m_wobbleFigure);
    laserPointController.setFileType(FileType::Seam);

    QVector<QPointF> figure {
        QPointF{0.0, 0.0},
        QPointF{0.0, 0.5},
        QPointF{-1.0, 0.5},
        QPointF{-1.0, 0.0},
        QPointF{0.0, 0.0}
    };

    SeamFigure newSeamFigure{testFunction::createSeamFigureFromQPointFs(figure)};

    laserPointController.drawSeamFigure(newSeamFigure);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 1);

    std::vector<QVector3D> ramps {
        QVector3D{0.0, 0.25, 0},
        QVector3D{-1.0, 0.25, 2}
    };

    laserPointController.drawRamps(ramps);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size() + 2);
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() + 1);

    std::vector<QPointF> expectedPoints {{0.0, 0.0}, {0.0, -500.0}, {-1000.0, -500.0}, {-1000.0, 0.0}, {0.0, 0.0}, {0.0, -250.0}, {-1000.0, -250.0}};
    int pointCounter = 0;
    for (const auto& node : laserPointController.figure()->get_nodes())
    {
        if (auto laserPoint = qobject_cast<LaserPoint*> (node))
        {
            QCOMPARE(laserPoint->center(), expectedPoints.at(pointCounter));
            pointCounter++;
        }
    }
}

void LaserPointControllerTest::testChangeRampVisibility()
{
    QVERIFY(m_engine);
    QVERIFY(m_wobbleFigure);
    m_wobbleFigure->clear();

    LaserPointController laserPointController;
    laserPointController.m_qmlComponent = qan::Node::delegate(*m_engine);
    laserPointController.m_qmlComponentEdge = qan::Edge::delegate(*m_engine);
    laserPointController.setFigure(m_wobbleFigure);
    laserPointController.setFileType(FileType::Seam);

    QVector<QPointF> figure {
        QPointF{0.0, 0.0},
        QPointF{0.0, 0.5},
        QPointF{-1.0, 0.5},
        QPointF{-1.0, 0.0},
        QPointF{0.0, 0.0}
    };

    SeamFigure newSeamFigure{testFunction::createSeamFigureFromQPointFs(figure)};

    laserPointController.drawSeamFigure(newSeamFigure);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 1);

    std::vector<QVector3D> ramps {
        QVector3D{0.0, 0.25, 0},
        QVector3D{-1.0, 0.25, 2}
    };

    laserPointController.drawRamps(ramps);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size() + 2);
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() + 1);

    laserPointController.m_visualizeRamps = false;
    laserPointController.changeRampVisibility();

    for (const auto& node : laserPointController.figure()->get_nodes())
    {
        if (auto laserPoint = qobject_cast<LaserPoint*> (node))
        {
            if (laserPoint->isRampPoint())
            {
                QVERIFY(!laserPoint->getItem()->isVisible());
            }
            else
            {
                QVERIFY(laserPoint->getItem()->isVisible());
            }
        }
    }

    for (const auto& edge : laserPointController.figure()->get_edges())
    {
        if (auto trajectory = qobject_cast<LaserTrajectory*> (edge))
        {
            if (trajectory->isRampEdge())
            {
                QVERIFY(!trajectory->getItem()->isVisible());
            }
            else
            {
                QVERIFY(trajectory->getItem()->isVisible());
            }
        }
    }
}

void LaserPointControllerTest::testCreateRampPoint()
{
    QVERIFY(m_engine);
    QVERIFY(m_wobbleFigure);
    m_wobbleFigure->clear();

    LaserPointController laserPointController;
    laserPointController.m_qmlComponent = qan::Node::delegate(*m_engine);
    laserPointController.m_qmlComponentEdge = qan::Edge::delegate(*m_engine);
    laserPointController.setFigure(m_wobbleFigure);
    laserPointController.setFileType(FileType::Seam);

    QVector<QPointF> figure {
        QPointF{0.0, 0.0},
        QPointF{0.0, 0.5},
        QPointF{-1.0, 0.5},
        QPointF{-1.0, 0.0},
        QPointF{0.0, 0.0}
    };

    SeamFigure newSeamFigure{testFunction::createSeamFigureFromQPointFs(figure)};

    laserPointController.drawSeamFigure(newSeamFigure);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 1);

    QVector3D rampEndPoint {0.0, 0.25, 0};

    auto laserPoint = laserPointController.createRampPoint(rampEndPoint);

    QVERIFY(laserPoint);
    QVERIFY(laserPoint->isRampPoint());
    QCOMPARE(laserPoint->center(), QPointF(rampEndPoint.x(), -1.0 * rampEndPoint.y() * laserPointController.figureScale()));
    QVERIFY(!laserPoint->getItem()->getSelectable());
    QVERIFY(!laserPoint->getItem()->getDraggable());
    QVERIFY(!laserPoint->getItem()->getResizable());
    QVERIFY(laserPoint->getItem()->isVisible());
}

void LaserPointControllerTest::testChangeRampPointVisibility()
{
    QVERIFY(m_engine);
    QVERIFY(m_wobbleFigure);
    m_wobbleFigure->clear();

    LaserPointController laserPointController;
    laserPointController.m_qmlComponent = qan::Node::delegate(*m_engine);
    laserPointController.m_qmlComponentEdge = qan::Edge::delegate(*m_engine);
    laserPointController.setFigure(m_wobbleFigure);
    laserPointController.setFileType(FileType::Seam);

    QVector<QPointF> figure {
        QPointF{0.0, 0.0},
        QPointF{0.0, 0.5},
        QPointF{-1.0, 0.5},
        QPointF{-1.0, 0.0},
        QPointF{0.0, 0.0}
    };

    SeamFigure newSeamFigure{testFunction::createSeamFigureFromQPointFs(figure)};

    laserPointController.drawSeamFigure(newSeamFigure);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 1);

    std::vector<QVector3D> ramps {
        QVector3D{0.0, 0.25, 0},
        QVector3D{-1.0, 0.25, 2}
    };

    laserPointController.drawRamps(ramps);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size() + 2);
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() + 1);

    laserPointController.m_visualizeRamps = false;
    laserPointController.changeRampPointVisibility();

    for (const auto& node : laserPointController.figure()->get_nodes())
    {
        if (auto laserPoint = qobject_cast<LaserPoint*> (node))
        {
            if (laserPoint->isRampPoint())
            {
                QVERIFY(!laserPoint->getItem()->isVisible());
            }
            else
            {
                QVERIFY(laserPoint->getItem()->isVisible());
            }
        }
    }
}

void LaserPointControllerTest::testCreateRampTrajectory()
{
    QVERIFY(m_engine);
    QVERIFY(m_wobbleFigure);
    m_wobbleFigure->clear();

    LaserPointController laserPointController;
    laserPointController.m_qmlComponent = qan::Node::delegate(*m_engine);
    laserPointController.m_qmlComponentEdge = qan::Edge::delegate(*m_engine);
    laserPointController.setFigure(m_wobbleFigure);
    laserPointController.setFileType(FileType::Seam);

    QVector<QPointF> figure {
        QPointF{0.0, 0.0},
        QPointF{0.0, 0.5},
        QPointF{-1.0, 0.5},
        QPointF{-1.0, 0.0},
        QPointF{0.0, 0.0}
    };

    SeamFigure newSeamFigure{testFunction::createSeamFigureFromQPointFs(figure)};

    laserPointController.drawSeamFigure(newSeamFigure);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 1);

    laserPointController.createRampTrajectory(qobject_cast<LaserPoint*>(laserPointController.figure()->get_nodes().at(0)), qobject_cast<LaserPoint*>(laserPointController.figure()->get_nodes().at(1)));

    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size());
    int foundRampTrajectories = 0;
    for (const auto& edge : laserPointController.figure()->get_edges())
    {
        if (auto trajectory = qobject_cast<LaserTrajectory*> (edge))
        {
            if (trajectory->isRampEdge())
            {
                foundRampTrajectories++;
                QCOMPARE(trajectory->ID(), qobject_cast<LaserPoint*>(laserPointController.figure()->get_nodes().at(0))->ID());
                QCOMPARE(trajectory->getItem()->getSrcShape(), qan::EdgeStyle::ArrowShape::Rect);
                QCOMPARE(trajectory->getItem()->getDstShape(), qan::EdgeStyle::ArrowShape::Rect);
            }
        }
    }
    QCOMPARE(foundRampTrajectories, 1);
}

void LaserPointControllerTest::testChangeRampTrajectoryVisibility()
{
    QVERIFY(m_engine);
    QVERIFY(m_wobbleFigure);
    m_wobbleFigure->clear();

    LaserPointController laserPointController;
    laserPointController.m_qmlComponent = qan::Node::delegate(*m_engine);
    laserPointController.m_qmlComponentEdge = qan::Edge::delegate(*m_engine);
    laserPointController.setFigure(m_wobbleFigure);
    laserPointController.setFileType(FileType::Seam);

    QVector<QPointF> figure {
        QPointF{0.0, 0.0},
        QPointF{0.0, 0.5},
        QPointF{-1.0, 0.5},
        QPointF{-1.0, 0.0},
        QPointF{0.0, 0.0}
    };

    SeamFigure newSeamFigure{testFunction::createSeamFigureFromQPointFs(figure)};

    laserPointController.drawSeamFigure(newSeamFigure);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 1);

    std::vector<QVector3D> ramps {
        QVector3D{0.0, 0.25, 0},
        QVector3D{-1.0, 0.25, 2}
    };

    laserPointController.drawRamps(ramps);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size() + 2);
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() + 1);

    laserPointController.m_visualizeRamps = false;
    laserPointController.changeRampTrajectoryVisibility();

    for (const auto& edge : laserPointController.figure()->get_edges())
    {
        if (auto trajectory = qobject_cast<LaserTrajectory*> (edge))
        {
            if (trajectory->isRampEdge())
            {
                QVERIFY(!trajectory->getItem()->isVisible());
            }
            else
            {
                QVERIFY(trajectory->getItem()->isVisible());
            }
        }
    }
}

void LaserPointControllerTest::testDeleteRampPoints()
{
    QVERIFY(m_engine);
    QVERIFY(m_wobbleFigure);
    m_wobbleFigure->clear();

    LaserPointController laserPointController;
    laserPointController.m_qmlComponent = qan::Node::delegate(*m_engine);
    laserPointController.m_qmlComponentEdge = qan::Edge::delegate(*m_engine);
    laserPointController.setFigure(m_wobbleFigure);
    laserPointController.setFileType(FileType::Seam);

    QVector<QPointF> figure {
        QPointF{0.0, 0.0},
        QPointF{0.0, 0.5},
        QPointF{-1.0, 0.5},
        QPointF{-1.0, 0.0},
        QPointF{0.0, 0.0}
    };

    SeamFigure newSeamFigure{testFunction::createSeamFigureFromQPointFs(figure)};

    laserPointController.drawSeamFigure(newSeamFigure);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 1);

    std::vector<QVector3D> ramps {
        QVector3D{0.0, 0.25, 0},
        QVector3D{-1.0, 0.25, 2}
    };

    laserPointController.drawRamps(ramps);
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size() + 2);
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() + 1);

    laserPointController.deleteRampPoints();
    QCOMPARE(laserPointController.figure()->get_node_count(), figure.size());
    QCOMPARE(laserPointController.figure()->get_edge_count(), figure.size() - 1);
}

void LaserPointControllerTest::testInvertY()
{
    QVERIFY(m_engine);
    QVERIFY(m_wobbleFigure);
    m_wobbleFigure->clear();

    LaserPointController laserPointController;
    laserPointController.m_qmlComponent = qan::Node::delegate(*m_engine);
    laserPointController.m_qmlComponentEdge = qan::Edge::delegate(*m_engine);
    laserPointController.setFigure(m_wobbleFigure);
    laserPointController.setFileType(FileType::Seam);

    QPointF testPoint {0.0, 0.0};
    QPointF expectedPoint {0.0, 0.0};

    QCOMPARE(laserPointController.invertY(testPoint), expectedPoint);

    testPoint.setX(5.2);
    testPoint.setY(-0.5);
    expectedPoint.setX(5.2);
    expectedPoint.setY(0.5);
    QCOMPARE(laserPointController.invertY(testPoint), expectedPoint);

    testPoint.setX(2.2);
    testPoint.setY(2.5);
    expectedPoint.setX(2.2);
    expectedPoint.setY(-2.5);
    QCOMPARE(laserPointController.invertY(testPoint), expectedPoint);
}

QTEST_MAIN(LaserPointControllerTest)
#include "laserPointControllerTest.moc"
