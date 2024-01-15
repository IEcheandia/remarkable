#include <QTest>
#include <QSignalSpy>
#include <QDebug>

#include "../src/rampValidator.h"
#include "../src/commandManager.h"

using precitec::scanmaster::components::wobbleFigureEditor::RampValidator;
using precitec::scantracker::components::wobbleFigureEditor::WobbleFigureEditor;
using precitec::scantracker::components::wobbleFigureEditor::CommandManager;
using RTC6::seamFigure::Ramp;

namespace {
double limitTo4Digits(double value)
{
    return static_cast<double> (qRound(value * 10000)) / 10000;
}
}

class RampValidatorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testFigureEditor();
    void testStartPointID();
    void testIsPointAlreadyInRamp();
    void testIsPointAStartPoint();
    void testMaxRampLength();
    void testSeamPoints();
    void testRamps();
    void testEndPoints();
    void testTakeSeamPoints();
    void testCastOrderPositionToVector();
    void testTakeRamps();
    void testNecessaryInfoAvailable();
    void testClearEndPoints();
    void testCalculateEndPoint();
    void testCalculateEndPointPosition();
    void testCalculateEndPoints();
    void testHasRampOut();
    void testInitRamps();
    void testCheckPointAlreadyInRamp();
    void testCheckPointsIsStartPoint();
    void testFindPointInRamps();
    void testCheckPointIsInRampOut();
    void testCheckPointIsInRamp();
    void testUpdatemaxRampLength();
    void testSearchNextRamp();
    void testCheckIDBelongsToRampOut();
    void testLengthFromRamp();
    void testCalculateLengthPointToPoint();
    void testCalculateWholeLength();
    void testConvertLengthToDegree();
    void testConvertDegreeToLength();
};

void RampValidatorTest::testCtor()
{
    RampValidator validator;

    QCOMPARE(validator.figureEditor(), nullptr);
    QCOMPARE(validator.startPointID(), -1);
    QVERIFY(!validator.isPointAlreadyInRamp());
    QVERIFY(!validator.isPointAStartPoint());
    QCOMPARE(validator.maxRampLength(), 0.0);

    QVERIFY(validator.seamPoints().empty());
    QVERIFY(validator.ramps().empty());
    QVERIFY(validator.endPoints().empty());
}

void RampValidatorTest::testFigureEditor()
{
    RampValidator validator;
    WobbleFigureEditor figureEditor;

    QSignalSpy figureEditorChanged {&validator, &RampValidator::figureEditorChanged};
    QVERIFY(figureEditorChanged.isValid());
    QCOMPARE(figureEditorChanged.count(), 0);

    validator.setFigureEditor(&figureEditor);
    QCOMPARE(validator.figureEditor(), &figureEditor);
    QCOMPARE(figureEditorChanged.count(), 1);

    figureEditor.destroyed();
    QCOMPARE(validator.figureEditor(), nullptr);
    QCOMPARE(figureEditorChanged.count(), 2);

    validator.setFigureEditor(nullptr);
    QCOMPARE(validator.figureEditor(), nullptr);
    QCOMPARE(figureEditorChanged.count(), 2);
}

void RampValidatorTest::testStartPointID()
{
    RampValidator validator;

    QSignalSpy startPointIDChanged {&validator, &RampValidator::startPointIDChanged};
    QVERIFY(startPointIDChanged.isValid());
    QCOMPARE(startPointIDChanged.count(), 0);

    validator.setStartPointID(-1);
    QCOMPARE(validator.startPointID(), -1);
    QCOMPARE(startPointIDChanged.count(), 0);

    validator.setStartPointID(3);
    QCOMPARE(validator.startPointID(), 3);
    QCOMPARE(startPointIDChanged.count(), 1);
}

void RampValidatorTest::testIsPointAlreadyInRamp()
{
    RampValidator validator;

    QSignalSpy isPointAlreadyInRampChanged {&validator, &RampValidator::isPointAlreadyInRampChanged};
    QVERIFY(isPointAlreadyInRampChanged.isValid());
    QCOMPARE(isPointAlreadyInRampChanged.count(), 0);

    validator.setIsPointAlreadyInRamp(false);
    QCOMPARE(validator.isPointAlreadyInRamp(), false);
    QCOMPARE(isPointAlreadyInRampChanged.count(), 0);

    validator.setIsPointAlreadyInRamp(true);
    QCOMPARE(validator.isPointAlreadyInRamp(), true);
    QCOMPARE(isPointAlreadyInRampChanged.count(), 1);

    validator.setIsPointAlreadyInRamp(false);
    QCOMPARE(validator.isPointAlreadyInRamp(), false);
    QCOMPARE(isPointAlreadyInRampChanged.count(), 2);
}

void RampValidatorTest::testIsPointAStartPoint()
{
    RampValidator validator;

    QSignalSpy isPointAStartPointChanged {&validator, &RampValidator::isPointAStartPointChanged};
    QVERIFY(isPointAStartPointChanged.isValid());
    QCOMPARE(isPointAStartPointChanged.count(), 0);

    validator.setIsPointAStartPoint(false);
    QCOMPARE(validator.isPointAStartPoint(), false);
    QCOMPARE(isPointAStartPointChanged.count(), 0);

    validator.setIsPointAStartPoint(true);
    QCOMPARE(validator.isPointAStartPoint(), true);
    QCOMPARE(isPointAStartPointChanged.count(), 1);

    validator.setIsPointAStartPoint(false);
    QCOMPARE(validator.isPointAStartPoint(), false);
    QCOMPARE(isPointAStartPointChanged.count(), 2);
}

void RampValidatorTest::testMaxRampLength()
{
    RampValidator validator;

    QSignalSpy maxRampLengthChanged {&validator, &RampValidator::maxRampLengthChanged};
    QVERIFY(maxRampLengthChanged.isValid());
    QCOMPARE(maxRampLengthChanged.count(), 0);

    validator.setMaxRampLength(0.0);
    QCOMPARE(validator.maxRampLength(), 0.0);
    QCOMPARE(maxRampLengthChanged.count(), 0);

    validator.setMaxRampLength(0.5);
    QCOMPARE(validator.maxRampLength(), 0.5);
    QCOMPARE(maxRampLengthChanged.count(), 1);

    validator.setMaxRampLength(3.2);
    QCOMPARE(validator.maxRampLength(), 3.2);
    QCOMPARE(maxRampLengthChanged.count(), 2);
}

void RampValidatorTest::testSeamPoints()
{
    RampValidator validator;

    QSignalSpy seamPointsChanged {&validator, &RampValidator::seamPointsChanged};
    QVERIFY(seamPointsChanged.isValid());
    QCOMPARE(seamPointsChanged.count(), 0);

    std::vector<QVector2D> points;

    validator.setSeamPoints(points);
    QCOMPARE(validator.seamPoints(), points);
    QCOMPARE(seamPointsChanged.count(), 0);

    points = {{1.0, -0.5}, {-2.0, 0.25}, {0.0, 0.0}};
    validator.setSeamPoints(points);
    QCOMPARE(validator.seamPoints(), points);
    QCOMPARE(seamPointsChanged.count(), 1);

    points = {{1.0, -0.75}, {2.0, 0.25}, {0.0, 0.0}};
    validator.setSeamPoints(points);
    QCOMPARE(validator.seamPoints(), points);
    QCOMPARE(seamPointsChanged.count(), 2);
}

void RampValidatorTest::testRamps()
{
    RampValidator validator;

    QSignalSpy rampsChanged {&validator, &RampValidator::rampsChanged};
    QVERIFY(rampsChanged.isValid());
    QCOMPARE(rampsChanged.count(), 0);

    std::vector<Ramp> ramps;

    validator.setRamps(ramps);
    std::size_t i = 0;
    for (const auto& element : validator.ramps())
    {
        QCOMPARE(element.startPointID, ramps.at(i).startPointID);
        QCOMPARE(element.length, ramps.at(i).length);
        QCOMPARE(element.startPower, ramps.at(i).startPower);
        QCOMPARE(element.endPower, ramps.at(i).endPower);
        QCOMPARE(element.startPowerRing, ramps.at(i).startPowerRing);
        QCOMPARE(element.endPowerRing, ramps.at(i).endPowerRing);
        i++;
    }
    QCOMPARE(rampsChanged.count(), 0);

    ramps = {{1, 0.5, 0.1, 1.0, 0.25, 0.75}, {1, 0.5, 0.3, 1.0, 0.45, 0.75}};
    validator.setRamps(ramps);
    i = 0;
    for (const auto& element : validator.ramps())
    {
        QCOMPARE(element.startPointID, ramps.at(i).startPointID);
        QCOMPARE(element.length, ramps.at(i).length);
        QCOMPARE(element.startPower, ramps.at(i).startPower);
        QCOMPARE(element.endPower, ramps.at(i).endPower);
        QCOMPARE(element.startPowerRing, ramps.at(i).startPowerRing);
        QCOMPARE(element.endPowerRing, ramps.at(i).endPowerRing);
        i++;
    }
    QCOMPARE(rampsChanged.count(), 1);

    ramps = {Ramp{1, 0.75, 0.1, 1.0, 0.25, 0.75}, Ramp{1, 8.5, 0.3, 1.0, 0.35, 0.95}};
    validator.setRamps(ramps);
    i = 0;
    for (const auto& element : validator.ramps())
    {
        QCOMPARE(element.startPointID, ramps.at(i).startPointID);
        QCOMPARE(element.length, ramps.at(i).length);
        QCOMPARE(element.startPower, ramps.at(i).startPower);
        QCOMPARE(element.endPower, ramps.at(i).endPower);
        QCOMPARE(element.startPowerRing, ramps.at(i).startPowerRing);
        QCOMPARE(element.endPowerRing, ramps.at(i).endPowerRing);
        i++;
    }
    QCOMPARE(rampsChanged.count(), 2);
}

void RampValidatorTest::testEndPoints()
{
    RampValidator validator;

    std::vector<QVector3D> points {{1.0, -0.5, 0}, {-2.0, 0.25, 1}, {0.0, 0.0, 2}};
    validator.m_endPoints = points;
    QCOMPARE(validator.endPoints(), points);
}

void RampValidatorTest::testTakeSeamPoints()
{
    RampValidator validator;
    WobbleFigureEditor figureEditor;

    QVERIFY(validator.seamPoints().empty());
    validator.takeSeamPoints();
    QVERIFY(validator.seamPoints().empty());

    RTC6::seamFigure::SeamFigure seam;
    std::vector<RTC6::seamFigure::command::Order> seamPoints;
    RTC6::seamFigure::command::Order order;
    order.endPosition = std::make_pair(0.5, 1.0);
    order.power = 0.5;
    order.ringPower = 0.25;
    order.velocity = -1.0;
    seamPoints.push_back(order);
    order.endPosition = std::make_pair(1.0, 1.0);
    seamPoints.push_back(order);
    seam.figure = seamPoints;

    validator.setFigureEditor(&figureEditor);
    figureEditor.setSeam(seam);

    QVERIFY(!validator.seamPoints().empty());
    QCOMPARE(validator.seamPoints().size(), seamPoints.size());
    for (std::size_t i = 0; i < seamPoints.size(); i++)
    {
        QCOMPARE(validator.seamPoints().at(i).x(), seamPoints.at(i).endPosition.first);
        QCOMPARE(validator.seamPoints().at(i).y(), seamPoints.at(i).endPosition.second);
    }
}

void RampValidatorTest::testCastOrderPositionToVector()
{
    RampValidator validator;

    std::vector<RTC6::seamFigure::command::Order> seamPoints;
    RTC6::seamFigure::command::Order order;
    order.endPosition = std::make_pair(0.5, 1.0);
    order.power = 0.5;
    order.ringPower = 0.25;
    order.velocity = -1.0;
    seamPoints.push_back(order);
    order.endPosition = std::make_pair(1.0, 1.0);
    seamPoints.push_back(order);

    const auto& points = validator.castOrderPositionToVector(seamPoints);

    QCOMPARE(points.size(), seamPoints.size());
    for (std::size_t i = 0; i < seamPoints.size(); i++)
    {
        QCOMPARE(points.at(i).x(), seamPoints.at(i).endPosition.first);
        QCOMPARE(points.at(i).y(), seamPoints.at(i).endPosition.second);
    }
}

void RampValidatorTest::testTakeRamps()
{
    RampValidator validator;
    WobbleFigureEditor figureEditor;
    CommandManager commandManager;
    figureEditor.setFileType(FileType::Seam);
    figureEditor.setCommandManager(&commandManager);

    QVERIFY(validator.ramps().empty());
    validator.takeRamps();
    QVERIFY(validator.ramps().empty());

    validator.setFigureEditor(&figureEditor);
    std::vector<Ramp> ramps {{0, 0.25, 0.0, 1.0, 0.0, 1.0}, {4, 0.3, 0.25, 0.75, 0.3, 0.8}};

    figureEditor.setRamps(ramps);
    QVERIFY(!validator.ramps().empty());
    QCOMPARE(validator.ramps().size(), ramps.size());
    QCOMPARE(validator.ramps(), ramps);
}

void RampValidatorTest::testNecessaryInfoAvailable()
{
    RampValidator validator;

    QVERIFY(!validator.necessaryInfoAvailable());

    std::vector<Ramp> ramps {{0, 0.25, 0.0, 1.0, 0.0, 1.0}, {4, 0.3, 0.25, 0.75, 0.3, 0.8}};
    validator.setRamps(ramps);

    QVERIFY(!validator.necessaryInfoAvailable());

    std::vector<RTC6::seamFigure::command::Order> seamPoints;
    RTC6::seamFigure::command::Order order;
    order.endPosition = std::make_pair(0.5, 1.0);
    order.power = 0.5;
    order.ringPower = 0.25;
    order.velocity = -1.0;
    seamPoints.push_back(order);
    order.endPosition = std::make_pair(1.0, 1.0);
    seamPoints.push_back(order);

    const auto& points = validator.castOrderPositionToVector(seamPoints);

    validator.setSeamPoints(points);
    QVERIFY(validator.necessaryInfoAvailable());
}

void RampValidatorTest::testClearEndPoints()
{
    RampValidator validator;

    QSignalSpy endPointsChanged{&validator, &RampValidator::endPointsChanged};
    QVERIFY(endPointsChanged.isValid());
    QCOMPARE(endPointsChanged.count(), 0);

    validator.clearEndPoints();
    QCOMPARE(endPointsChanged.count(), 1);

    std::vector<QVector3D> endPoints {{1.0, 1.0, 1}, {0.75, 0.8, 2}};
    validator.m_endPoints = endPoints;
    QVERIFY(!validator.endPoints().empty());

    validator.clearEndPoints();
    QVERIFY(validator.endPoints().empty());
    QCOMPARE(endPointsChanged.count(), 2);
}

void RampValidatorTest::testCalculateEndPoint()
{
    RampValidator validator;
    std::vector<QVector2D> points{{-0.5, 0.0}, {-0.25, 0.0}, {0.5, 0.0}, {1.0, 0.0}, {1.2, 0.0}};

    validator.setSeamPoints(points);
    auto startPointID = 0;
    auto length = 0.1;

    auto endPoint = validator.calculateEndPoint(startPointID, length);
    QCOMPARE(endPoint, QVector3D(-0.4, 0.0, 0));
    length = 0.5;
    endPoint = validator.calculateEndPoint(startPointID, length);
    QCOMPARE(endPoint, QVector3D(0.0, 0.0, 0));

    startPointID = 1;
    endPoint = validator.calculateEndPoint(startPointID, length);
    QCOMPARE(endPoint, QVector3D(0.25, 0.0, 1));

    startPointID = 3;
    length = 0.1;
    endPoint = validator.calculateEndPoint(startPointID, length);
    QCOMPARE(endPoint, QVector3D(1.1, 0.0, 3));

    startPointID = 4;
    length = 0.2;
    endPoint = validator.calculateEndPoint(startPointID, length);
    QCOMPARE(endPoint, QVector3D(1.0, 0.0, 4));
}

void RampValidatorTest::testCalculateEndPointPosition()
{
    RampValidator validator;
    std::vector<QVector2D> points{{-0.5, 0.0}, {-0.25, 0.0}, {0.5, 0.0}, {1.0, 0.0}, {1.2, 0.0}};

    validator.setSeamPoints(points);
    auto startPointID = 0;
    auto length = 0.2;

    auto endPoint = validator.calculateEndPointPosition(startPointID, length);
    QCOMPARE(endPoint, QVector3D(-0.3, 0.0, 0));
    length = 0.5;
    endPoint = validator.calculateEndPointPosition(startPointID, length);
    QCOMPARE(endPoint, QVector3D(0.0, 0.0, 0));

    startPointID = 1;
    endPoint = validator.calculateEndPointPosition(startPointID, length);
    QCOMPARE(endPoint, QVector3D(0.25, 0.0, 1));

    startPointID = 3;
    length = 0.1;
    endPoint = validator.calculateEndPointPosition(startPointID, length);
    QCOMPARE(endPoint, QVector3D(1.1, 0.0, 3));
}

void RampValidatorTest::testCalculateEndPoints()
{
    RampValidator validator;
    QSignalSpy endPointsChanged{&validator, &RampValidator::endPointsChanged};
    QVERIFY(endPointsChanged.isValid());
    QCOMPARE(endPointsChanged.count(), 0);

    validator.calculateEndPoints();
    QVERIFY(validator.endPoints().empty());
    QCOMPARE(endPointsChanged.count(), 1);

    std::vector<QVector2D> points{{-0.5, 0.0}, {-0.25, 0.0}, {0.5, 0.0}, {1.0, 0.0}, {1.2, 0.0}};
    std::vector<Ramp> ramps{{0, 0.25, 0.1, 0.9, 0.0, 1.0}};
    std::vector<QVector3D> expectedEndPoints {{-0.25, 0.0, 0}};

    validator.setSeamPoints(points);
    validator.setRamps(ramps);
    validator.calculateEndPoints();

    QVERIFY(!validator.endPoints().empty());
    QCOMPARE(validator.endPoints(), expectedEndPoints);
    QCOMPARE(endPointsChanged.count(), 2);

    ramps.front().startPointID = 4;
    expectedEndPoints.push_back({0.95, 0.0, 4});

    validator.setRamps(ramps);
    validator.hasRampOut();
    validator.calculateEndPoints();

    QVERIFY(!validator.endPoints().empty());
    QCOMPARE(validator.endPoints().size(), expectedEndPoints.size());
    for (std::size_t i = 0; i < validator.endPoints().size(); i++)
    {
        QCOMPARE(validator.endPoints().at(i).x(), expectedEndPoints.at(i).x());
        QCOMPARE(validator.endPoints().at(i).y(), expectedEndPoints.at(i).y());
        QCOMPARE(validator.endPoints().at(i).z(), expectedEndPoints.at(i).z());
    }
    QCOMPARE(validator.seamPoints(), points);
}

void RampValidatorTest::testHasRampOut()
{
    RampValidator validator;
    std::vector<QVector2D> points{{-0.5, 0.0}, {-0.25, 0.0}, {0.5, 0.0}, {1.0, 0.0}, {1.2, 0.0}};
    std::vector<Ramp> ramps{{0, 0.25, 0.1, 0.9, 0.0, 1.0}};

    validator.hasRampOut();
    QVERIFY(!validator.m_rampsHasRampOut);

    validator.setSeamPoints(points);
    validator.hasRampOut();
    QVERIFY(!validator.m_rampsHasRampOut);

    validator.setRamps(ramps);
    validator.hasRampOut();
    QVERIFY(!validator.m_rampsHasRampOut);

    ramps.emplace_back(Ramp{4, 0.1, 0.1, 0.8, 0.0, 0.75});
    validator.setRamps(ramps);
    validator.hasRampOut();
    QVERIFY(validator.m_rampsHasRampOut);
}

void RampValidatorTest::testInitRamps()
{
    RampValidator validator;
    WobbleFigureEditor figureEditor;
    CommandManager commandManager;
    figureEditor.setFileType(FileType::Seam);
    figureEditor.setCommandManager(&commandManager);
    std::vector<Ramp> ramps {{0, 0.25, 0.0, 1.0, 0.0, 1.0}, {4, 0.3, 0.25, 0.75, 0.3, 0.8}};

    QSignalSpy endPointsChanged{&validator, &RampValidator::endPointsChanged};
    QVERIFY(endPointsChanged.isValid());
    QCOMPARE(endPointsChanged.count(), 0);

    validator.setFigureEditor(&figureEditor);
    figureEditor.setRamps(ramps);

    QCOMPARE(endPointsChanged.count(), 1);
    QVERIFY(!validator.ramps().empty());
    QVERIFY(!validator.m_rampsHasRampOut);

    std::vector<QVector2D> points{{-0.5, 0.0}, {-0.25, 0.0}, {0.5, 0.0}, {1.0, 0.0}, {1.2, 0.0}};
    std::vector<QVector3D> expectedOutPoints{{-0.25, 0.0, 0}, {0.9, 0.0, 4}};
    validator.setSeamPoints(points);
    validator.initRamps();

    QCOMPARE(endPointsChanged.count(), 3);
    QCOMPARE(validator.endPoints().size(), expectedOutPoints.size());

    for (std::size_t i = 0; i < validator.endPoints().size(); i++)
    {
        QCOMPARE(validator.endPoints().at(i).x(), expectedOutPoints.at(i).x());
        QCOMPARE(validator.endPoints().at(i).y(), expectedOutPoints.at(i).y());
        QCOMPARE(validator.endPoints().at(i).z(), expectedOutPoints.at(i).z());
    }
}

void RampValidatorTest::testCheckPointAlreadyInRamp()
{
    RampValidator validator;

    validator.checkPointAlreadyInRamp();
    QVERIFY(!validator.isPointAlreadyInRamp());
    QVERIFY(!validator.isPointAStartPoint());

    std::vector<QVector2D> points{{-0.5, 0.0}, {-0.25, 0.0}, {0.5, 0.0}, {1.0, 0.0}, {1.2, 0.0}};
    validator.setSeamPoints(points);

    validator.checkPointAlreadyInRamp();
    QVERIFY(!validator.isPointAlreadyInRamp());
    QVERIFY(!validator.isPointAStartPoint());

    std::vector<Ramp> ramps {{1, 0.8, 0.0, 1.0, 0.0, 1.0}, {3, 0.1, 0.0, 1.0, 0.0, 1.0}};
    validator.setRamps(ramps);

    validator.setStartPointID(0);
    validator.checkPointAlreadyInRamp();
    QVERIFY(!validator.isPointAlreadyInRamp());
    QVERIFY(!validator.isPointAStartPoint());

    validator.setStartPointID(1);
    validator.checkPointAlreadyInRamp();
    QVERIFY(validator.isPointAlreadyInRamp());
    QVERIFY(validator.isPointAStartPoint());

    validator.setStartPointID(2);
    validator.checkPointAlreadyInRamp();
    QVERIFY(validator.isPointAlreadyInRamp());
    QVERIFY(!validator.isPointAStartPoint());

    validator.setStartPointID(3);
    validator.checkPointAlreadyInRamp();
    QVERIFY(validator.isPointAlreadyInRamp());
    QVERIFY(validator.isPointAStartPoint());

    validator.setStartPointID(4);
    validator.checkPointAlreadyInRamp();
    QVERIFY(!validator.isPointAlreadyInRamp());
    QVERIFY(!validator.isPointAStartPoint());
}

void RampValidatorTest::testCheckPointsIsStartPoint()
{
    RampValidator validator;

    std::vector<Ramp> ramps {{1, 0.5, 0.0, 1.0, 0.0, 1.0}, {4, 0.5, 0.0, 1.0, 0.0, 1.0}, {5, 0.5, 0.0, 1.0, 0.0, 1.0}};

    QVERIFY(!validator.checkPointIsStartPoint(1));
    QVERIFY(!validator.checkPointIsStartPoint(3));

    validator.setRamps(ramps);
    QVERIFY(!validator.checkPointIsStartPoint(2));
    QVERIFY(!validator.checkPointIsStartPoint(6));

    QVERIFY(validator.checkPointIsStartPoint(1));
    QVERIFY(validator.checkPointIsStartPoint(4));
    QVERIFY(validator.checkPointIsStartPoint(5));
}

void RampValidatorTest::testFindPointInRamps()
{
    RampValidator validator;
    std::vector<QVector2D> points{{-0.5, 0.0}, {-0.25, 0.0}, {0.5, 0.0}, {1.0, 0.0}, {1.2, 0.0}};
    std::vector<Ramp> ramps {{4, 0.5, 0.0, 1.0, 0.0, 1.0}};

    validator.setSeamPoints(points);

    QVERIFY(!validator.findPointInRamps());

    validator.setRamps(ramps);
    validator.setStartPointID(0);

    QVERIFY(!validator.findPointInRamps());

    validator.m_rampsHasRampOut = true;
    QVERIFY(!validator.findPointInRamps());

    ramps.front().length = 2.0;
    validator.setRamps(ramps);

    QVERIFY(validator.findPointInRamps());

    ramps.front().startPointID = 2;
    ramps.front().length = 0.6;

    validator.setRamps(ramps);
    validator.setStartPointID(3);
    validator.m_rampsHasRampOut = false;

    QVERIFY(validator.findPointInRamps());

    ramps.front().length = 0.2;
    validator.setRamps(ramps);

    QVERIFY(!validator.findPointInRamps());

    ramps.front().startPointID = 4;
    ramps.front().length = 0.9;

    validator.setRamps(ramps);
    validator.setStartPointID(2);
    validator.m_rampsHasRampOut = true;

    QVERIFY(validator.findPointInRamps());

    ramps.front().startPointID = 4;
    ramps.front().length = 0.6;

    validator.setRamps(ramps);

    QVERIFY(!validator.findPointInRamps());
}

void RampValidatorTest::testCheckPointIsInRampOut()
{
    RampValidator validator;
    std::vector<QVector2D> points{{-0.5, 0.0}, {-0.25, 0.0}, {0.5, 0.0}, {1.0, 0.0}, {1.2, 0.0}};
    std::vector<Ramp> ramps {{4, 0.5, 0.0, 1.0, 0.0, 1.0}};

    validator.setSeamPoints(points);
    validator.setRamps(ramps);
    validator.setStartPointID(0);

    QVERIFY(!validator.checkPointIsInRampOut());
    QCOMPARE(validator.seamPoints(), points);

    ramps.front().length = 2.0;
    validator.setRamps(ramps);

    QVERIFY(validator.checkPointIsInRampOut());
    QCOMPARE(validator.seamPoints(), points);

    validator.setStartPointID(2);
    QVERIFY(validator.checkPointIsInRampOut());
    QCOMPARE(validator.seamPoints(), points);

    ramps.front().length = 0.2;
    validator.setRamps(ramps);

    QVERIFY(!validator.checkPointIsInRampOut());
    QCOMPARE(validator.seamPoints(), points);

    ramps.front().length = 0.6;
    validator.setRamps(ramps);

    QVERIFY(!validator.checkPointIsInRampOut());
    QCOMPARE(validator.seamPoints(), points);

    ramps.front().length = 0.701;
    validator.setRamps(ramps);

    QVERIFY(validator.checkPointIsInRampOut());
    QCOMPARE(validator.seamPoints(), points);

    ramps.front().length = 0.9;
    validator.setRamps(ramps);

    QVERIFY(validator.checkPointIsInRampOut());
    QCOMPARE(validator.seamPoints(), points);
}

void RampValidatorTest::testCheckPointIsInRamp()
{
    RampValidator validator;
    std::vector<QVector2D> points{{-0.5, 0.0}, {-0.25, 0.0}, {0.5, 0.0}, {1.0, 0.0}, {1.2, 0.0}};
    std::vector<Ramp> ramps {{2, 0.6, 0.0, 1.0, 0.0, 1.0}};

    validator.setSeamPoints(points);
    validator.setRamps(ramps);
    validator.setStartPointID(3);

    QVERIFY(validator.checkPointIsInRamp());

    ramps.front().length = 0.5;
    validator.setRamps(ramps);

    QVERIFY(validator.checkPointIsInRamp());

    ramps.front().length = 0.2;
    validator.setRamps(ramps);

    QVERIFY(!validator.checkPointIsInRamp());
}

void RampValidatorTest::testUpdatemaxRampLength()
{
    RampValidator validator;
    std::vector<QVector2D> points{{-0.5, 0.0}, {-0.25, 0.0}, {0.5, 0.0}, {1.0, 0.0}, {1.2, 0.0}};

    validator.updateMaxRampLength();
    QCOMPARE(validator.maxRampLength(), 0.0);

    validator.setSeamPoints(points);

    validator.updateMaxRampLength();
    QCOMPARE(validator.maxRampLength(), 0.0);

    validator.setStartPointID(0);
    QCOMPARE(limitTo4Digits(validator.maxRampLength()), 1.7);
    validator.setStartPointID(1);
    QCOMPARE(limitTo4Digits(validator.maxRampLength()), 1.45);
    validator.setStartPointID(2);
    QCOMPARE(limitTo4Digits(validator.maxRampLength()), 0.7);
    validator.setStartPointID(3);
    QCOMPARE(limitTo4Digits(validator.maxRampLength()), 0.2);
    validator.setStartPointID(4);
    QCOMPARE(limitTo4Digits(validator.maxRampLength()), 1.7);

    std::vector<Ramp> ramps {{1, 0.15, 0.0, 1.0, 0.0, 1.0}, {3, 0.1, 0.0, 1.0, 0.0, 1.0}};
    validator.setRamps(ramps);

    validator.setStartPointID(0);
    QCOMPARE(limitTo4Digits(validator.maxRampLength()), 0.25);
    validator.setStartPointID(1);
    QCOMPARE(limitTo4Digits(validator.maxRampLength()), 1.25);
    validator.setStartPointID(2);
    QCOMPARE(limitTo4Digits(validator.maxRampLength()), 0.5);
    validator.setStartPointID(3);
    QCOMPARE(limitTo4Digits(validator.maxRampLength()), 0.2);
    validator.setStartPointID(4);
    QCOMPARE(limitTo4Digits(validator.maxRampLength()), 0.1);

    ramps = {{1, 0.15, 0.0, 1.0, 0.0, 1.0}, {4, 0.2, 0.0, 1.0, 0.0, 1.0}};
    validator.setRamps(ramps);

    validator.setStartPointID(0);
    QCOMPARE(limitTo4Digits(validator.maxRampLength()), 0.25);
    validator.setStartPointID(1);
    QCOMPARE(limitTo4Digits(validator.maxRampLength()), 1.25);
    validator.setStartPointID(2);
    QCOMPARE(limitTo4Digits(validator.maxRampLength()), 0.5);
    validator.setStartPointID(3);
    QCOMPARE(limitTo4Digits(validator.maxRampLength()), 0.0);
    validator.setStartPointID(4);
    QCOMPARE(limitTo4Digits(validator.maxRampLength()), 1.3);
}

void RampValidatorTest::testSearchNextRamp()
{
    RampValidator validator;
    std::vector<QVector2D> points{{-0.5, 0.0}, {-0.25, 0.0}, {0.5, 0.0}, {1.0, 0.0}, {1.2, 0.0}};
    std::vector<Ramp> ramps {{1, 0.5, 0.0, 1.0, 0.0, 1.0}, {2, 0.5, 0.0, 1.0, 0.0, 1.0}, {4, 0.5, 0.0, 1.0, 0.0, 1.0}};

    validator.setSeamPoints(points);
    validator.setRamps(ramps);

    auto startPointID = 0;
    QCOMPARE(validator.searchNextRamp(startPointID), 1);
    startPointID++;
    QCOMPARE(validator.searchNextRamp(startPointID), 2);
    startPointID++;
    QCOMPARE(validator.searchNextRamp(startPointID), 4);
    startPointID++;
    QCOMPARE(validator.searchNextRamp(startPointID), 4);
    startPointID++;
    QCOMPARE(validator.searchNextRamp(startPointID), points.size() - 1);
}

void RampValidatorTest::testCheckIDBelongsToRampOut()
{
    RampValidator validator;
    std::vector<QVector2D> points{{-0.5, 0.0}, {-0.25, 0.0}, {0.5, 0.0}, {1.0, 0.0}, {1.2, 0.0}};

    QVERIFY(!validator.checkIDBelongsToRampOut(0));
    validator.setSeamPoints(points);
    QVERIFY(!validator.checkIDBelongsToRampOut(0));
    QVERIFY(!validator.checkIDBelongsToRampOut(3));

    QVERIFY(validator.checkIDBelongsToRampOut(4));
}

void RampValidatorTest::testLengthFromRamp()
{
    RampValidator validator;
    std::vector<Ramp> ramps {{1, 0.5, 0.0, 1.0, 0.0, 1.0}, {2, 0.6, 0.0, 1.0, 0.0, 1.0}, {4, 1.5, 0.0, 1.0, 0.0, 1.0}};

    QCOMPARE(validator.lengthFromRamp(0), 0.0);
    validator.setRamps(ramps);
    QCOMPARE(validator.lengthFromRamp(0), 0.0);
    QCOMPARE(validator.lengthFromRamp(1), 0.5);
    QCOMPARE(validator.lengthFromRamp(2), 0.6);
    QCOMPARE(validator.lengthFromRamp(4), 1.5);
}

void RampValidatorTest::testCalculateLengthPointToPoint()
{
    RampValidator validator;

    std::vector<QVector2D> points{{-0.5, 0.0}, {-0.25, 0.0}, {0.5, 0.0}, {1.0, 0.0}, {1.2, 0.0}, {-0.2, 1.0}};

    QCOMPARE(validator.calculateLengthPointToPoint(0, 1), 0.0);
    validator.setSeamPoints(points);

    QCOMPARE(validator.calculateLengthPointToPoint(1, 0), 0.0);
    QCOMPARE(validator.calculateLengthPointToPoint(points.size(), points.size() + 1), 0.0);
    QCOMPARE(validator.calculateLengthPointToPoint(points.size() - 1, points.size() + 1), 0.0);
    QCOMPARE(validator.calculateLengthPointToPoint(0, points.size()), 0.0);

    QCOMPARE(limitTo4Digits(validator.calculateLengthPointToPoint(0, points.size() - 1)), 3.4205);

    QCOMPARE(limitTo4Digits(validator.calculateLengthPointToPoint(1, points.size() - 1)), 3.1705);
    QCOMPARE(limitTo4Digits(validator.calculateLengthPointToPoint(1, 2)), 0.75);
}

void RampValidatorTest::testCalculateWholeLength()
{
    RampValidator validator;

    std::vector<QVector2D> points{{-0.5, 0.0}, {-0.25, 0.0}, {0.5, 0.0}, {1.0, 0.0}, {1.2, 0.0}};

    validator.calculateWholeLength();
    QCOMPARE(validator.m_wholeLength, 0.0);

    validator.setSeamPoints(points);
    validator.calculateWholeLength();
    QCOMPARE(limitTo4Digits(validator.m_wholeLength), 1.7);
}

void RampValidatorTest::testConvertLengthToDegree()
{
    RampValidator validator;

    QCOMPARE(validator.convertLengthToDegree(5.0), 5.0);

    std::vector<QVector2D> points{{-0.5, 0.0}, {-0.25, 0.0}, {0.5, 0.0}, {1.0, 0.0}, {1.2, 0.0}};
    validator.setSeamPoints(points);
    validator.calculateWholeLength();
    QCOMPARE(limitTo4Digits(validator.m_wholeLength), 1.7);

    QCOMPARE(limitTo4Digits(validator.convertLengthToDegree(1.7 * 0.25)), 90.0);
    QCOMPARE(limitTo4Digits(validator.convertLengthToDegree(1.7)), 360.0);
    QCOMPARE(limitTo4Digits(validator.convertLengthToDegree(1.7 * 0.75)), 270.0);
    QCOMPARE(limitTo4Digits(validator.convertLengthToDegree(1.7 * 0.0)), 0.0);
    QCOMPARE(limitTo4Digits(validator.convertLengthToDegree(1.7 * 0.5)), 180.0);
}

void RampValidatorTest::testConvertDegreeToLength()
{
    RampValidator validator;

    QCOMPARE(validator.convertDegreeToLength(15.0), 0.0);

    std::vector<QVector2D> points{{-0.5, 0.0}, {-0.25, 0.0}, {0.5, 0.0}, {1.0, 0.0}, {1.2, 0.0}};
    validator.setSeamPoints(points);
    validator.calculateWholeLength();
    QCOMPARE(limitTo4Digits(validator.m_wholeLength), 1.7);

    QCOMPARE(limitTo4Digits(validator.convertDegreeToLength(90.0)), 1.7 * 0.25);
    QCOMPARE(limitTo4Digits(validator.convertDegreeToLength(360.0)), 1.7);
    QCOMPARE(limitTo4Digits(validator.convertDegreeToLength(270.0)), 1.7 * 0.75);
    QCOMPARE(limitTo4Digits(validator.convertDegreeToLength(0.0)), 0.0);
    QCOMPARE(limitTo4Digits(validator.convertDegreeToLength(180.0)), 1.7 * 0.5);
}


QTEST_GUILESS_MAIN(RampValidatorTest)
#include "rampValidatorTest.moc"

