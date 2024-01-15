#include <QTest>
#include <QSignalSpy>

#include "../src/rampModel.h"

#include "../src/WobbleFigureEditor.h"
#include "../src/fileType.h"
#include "../src/commandManager.h"

using precitec::scanmaster::components::wobbleFigureEditor::RampModel;
using precitec::scantracker::components::wobbleFigureEditor::WobbleFigureEditor;
using RTC6::seamFigure::SeamFigure;
using RTC6::seamFigure::Ramp;
using precitec::scantracker::components::wobbleFigureEditor::FileType;

class RampModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testFigureEditor();
    void testStartPointID();
    void testRampLength();
    void testStartPower();
    void testEndPower();
    void testStartPowerRing();
    void testEndPowerRing();
    void testRamps_data();
    void testRamps();
    void testUpdateRamps();
    void testEraseRamp();
    void testTakeRamps();
    void testGiveBackRamps();
    void testReset();
    void testUpdateRampModel();
    void testSearchRamp_data();
    void testSearchRamp();
    void testSearchRampIt_data();
    void testSearchRampIt();
    void testFoundRamp_data();
    void testFoundRamp();
    void testUpdateRampModelProperties();
    void testAppendRamp();
    void testSortRamps();
    void testUpdateFoundRamp_data();
    void testUpdateFoundRamp();
    void testIsStartPointIDValid();
};

void RampModelTest::testCtor()
{
    RampModel model;

    QCOMPARE(model.figureEditor(), nullptr);
    QCOMPARE(model.startPointID(), -1);
    QCOMPARE(model.rampLength(), 0.0);
    QCOMPARE(model.startPower(), 0.0);
    QCOMPARE(model.endPower(), 0.0);
    QCOMPARE(model.startPowerRing(), 0.0);
    QCOMPARE(model.endPowerRing(), 0.0);
    QVERIFY(model.ramps().empty());
}

void RampModelTest::testFigureEditor()
{
    RampModel model;

    QSignalSpy figureEditorChanged {&model, &RampModel::figureEditorChanged};
    QVERIFY(figureEditorChanged.isValid());
    QCOMPARE(figureEditorChanged.count(), 0);

    model.setFigureEditor(nullptr);
    QCOMPARE(figureEditorChanged.count(), 0);

    WobbleFigureEditor figureEditor;
    model.setFigureEditor(&figureEditor);
    QCOMPARE(figureEditorChanged.count(), 1);
    QCOMPARE(model.figureEditor(), &figureEditor);

    figureEditor.destroyed();
    QCOMPARE(figureEditorChanged.count(), 2);
    QCOMPARE(model.figureEditor(), nullptr);
}

void RampModelTest::testStartPointID()
{
    RampModel model;

    QSignalSpy startPointIDChanged {&model, &RampModel::startPointIDChanged};
    QVERIFY(startPointIDChanged.isValid());
    QCOMPARE(startPointIDChanged.count(), 0);

    int id = 5;
    model.setStartPointID(id);
    QCOMPARE(model.startPointID(), id);
    QCOMPARE(startPointIDChanged.count(), 1);

    model.setStartPointID(id);
    QCOMPARE(model.startPointID(), id);
    QCOMPARE(startPointIDChanged.count(), 1);

    id = 10;
    model.setStartPointID(id);
    QCOMPARE(model.startPointID(), id);
    QCOMPARE(startPointIDChanged.count(), 2);
}

void RampModelTest::testRampLength()
{
    RampModel model;

    QSignalSpy rampLengthChanged {&model, &RampModel::rampLengthChanged};
    QVERIFY(rampLengthChanged.isValid());
    QCOMPARE(rampLengthChanged.count(), 0);

    double length = 10.5;
    model.setRampLength(length);
    QCOMPARE(model.rampLength(), length);
    QCOMPARE(rampLengthChanged.count(), 1);

    model.setRampLength(length);
    QCOMPARE(model.rampLength(), length);
    QCOMPARE(rampLengthChanged.count(), 1);

    length = 0.5;
    model.setRampLength(length);
    QCOMPARE(model.rampLength(), length);
    QCOMPARE(rampLengthChanged.count(), 2);
}

void RampModelTest::testStartPower()
{
    RampModel model;

    QSignalSpy startPowerChanged {&model, &RampModel::startPowerChanged};
    QVERIFY(startPowerChanged.isValid());
    QCOMPARE(startPowerChanged.count(), 0);

    double power = 10.5;
    model.setStartPower(power);
    QCOMPARE(model.startPower(), power);
    QCOMPARE(startPowerChanged.count(), 1);

    model.setStartPower(power);
    QCOMPARE(model.startPower(), power);
    QCOMPARE(startPowerChanged.count(), 1);


    power = 0.5;
    model.setStartPower(power);
    QCOMPARE(model.startPower(), power);
    QCOMPARE(startPowerChanged.count(), 2);
}

void RampModelTest::testEndPower()
{
    RampModel model;

    QSignalSpy endPowerChanged {&model, &RampModel::endPowerChanged};
    QVERIFY(endPowerChanged.isValid());
    QCOMPARE(endPowerChanged.count(), 0);

    double power = 10.5;
    model.setEndPower(power);
    QCOMPARE(model.endPower(), power);
    QCOMPARE(endPowerChanged.count(), 1);

    model.setEndPower(power);
    QCOMPARE(model.endPower(), power);
    QCOMPARE(endPowerChanged.count(), 1);

    power = 0.5;
    model.setEndPower(power);
    QCOMPARE(model.endPower(), power);
    QCOMPARE(endPowerChanged.count(), 2);
}

void RampModelTest::testStartPowerRing()
{
    RampModel model;

    QSignalSpy startPowerRingChanged {&model, &RampModel::startPowerRingChanged};
    QVERIFY(startPowerRingChanged.isValid());
    QCOMPARE(startPowerRingChanged.count(), 0);

    double power = 10.5;
    model.setStartPowerRing(power);
    QCOMPARE(model.startPowerRing(), power);
    QCOMPARE(startPowerRingChanged.count(), 1);

    model.setStartPowerRing(power);
    QCOMPARE(model.startPowerRing(), power);
    QCOMPARE(startPowerRingChanged.count(), 1);

    power = 0.5;
    model.setStartPowerRing(power);
    QCOMPARE(model.startPowerRing(), power);
    QCOMPARE(startPowerRingChanged.count(), 2);
}

void RampModelTest::testEndPowerRing()
{
    RampModel model;

    QSignalSpy endPowerRingChanged {&model, &RampModel::endPowerRingChanged};
    QVERIFY(endPowerRingChanged.isValid());
    QCOMPARE(endPowerRingChanged.count(), 0);

    double power = 10.5;
    model.setEndPowerRing(power);
    QCOMPARE(model.endPowerRing(), power);
    QCOMPARE(endPowerRingChanged.count(), 1);

    model.setEndPowerRing(power);
    QCOMPARE(model.endPowerRing(), power);
    QCOMPARE(endPowerRingChanged.count(), 1);

    power = 0.5;
    model.setEndPowerRing(power);
    QCOMPARE(model.endPowerRing(), power);
    QCOMPARE(endPowerRingChanged.count(), 2);
}

void RampModelTest::testRamps_data()
{
    QTest::addColumn<QVector<Ramp>>("availableRamps");
    QTest::addColumn<int>("numberOfRamps");
    QTest::addColumn<int>("changeCounter");

    QVector<Ramp> rampOne;
    QVector<Ramp> rampTwo;

    Ramp ramp;
    ramp.startPointID = 0;
    ramp.length = 0.25;
    ramp.startPower = 0.0;
    ramp.endPower = 1.0;
    ramp.startPowerRing = 0.25;
    ramp.endPowerRing = 0.75;

    rampOne.push_back(ramp);
    rampTwo.push_back(ramp);

    ramp.startPointID = 4;
    ramp.length = 1.5;
    ramp.startPower = 1.0;
    ramp.endPower = 0.5;
    ramp.startPowerRing = 0.25;
    ramp.endPowerRing = 0.5;

    rampTwo.push_back(ramp);

    QTest::addRow("RampOne") << rampOne << 1 << 1;
    QTest::addRow("RampTwo") << rampTwo << 2 << 1;
}

void RampModelTest::testRamps()
{
    RampModel model;

    QSignalSpy rampsChanged {&model, &RampModel::rampsChanged};
    QVERIFY(rampsChanged.isValid());
    QCOMPARE(rampsChanged.count(), 0);

    model.setRamps({});
    QVERIFY(model.ramps().empty());
    QCOMPARE(rampsChanged.count(), 0);

    QFETCH(QVector<Ramp>, availableRamps);
    QFETCH(int, numberOfRamps);
    QFETCH(int, changeCounter);

    std::vector<Ramp> ramps {availableRamps.begin(), availableRamps.end()};
    model.setRamps(ramps);
    QVERIFY(!model.ramps().empty());
    QCOMPARE(model.ramps().size(), numberOfRamps);
    QCOMPARE(rampsChanged.count(), changeCounter);
}

void RampModelTest::testUpdateRamps()
{
    RampModel model;
    WobbleFigureEditor figureEditor;
    CommandManager commandManager;
    figureEditor.setCommandManager(&commandManager);
    QVERIFY(figureEditor.ramps().empty());

    QVERIFY(model.m_ramps.empty());

    model.setStartPointID(1);
    model.setRampLength(0.6);
    model.setStartPower(0.25);
    model.setEndPower(0.75);
    model.setStartPowerRing(1.0);
    model.setEndPowerRing(0.0);

    model.updateRamps();

    QCOMPARE(model.m_ramps.size(), 1);
    QCOMPARE(model.m_ramps.back().startPointID, 1);
    QCOMPARE(model.m_ramps.back().length, 0.6);
    QCOMPARE(model.m_ramps.back().startPower, 0.25);
    QCOMPARE(model.m_ramps.back().endPower, 0.75);
    QCOMPARE(model.m_ramps.back().startPowerRing, 1.0);
    QCOMPARE(model.m_ramps.back().endPowerRing, 0.0);

    model.setRampLength(1.0);
    model.setStartPower(0.35);
    model.setEndPower(0.85);
    model.setStartPowerRing(0.9);
    model.setEndPowerRing(0.2);

    model.updateRamps();
    QCOMPARE(model.m_ramps.size(), 1);
    QCOMPARE(model.m_ramps.back().startPointID, 1);
    QCOMPARE(model.m_ramps.back().length, 1.0);
    QCOMPARE(model.m_ramps.back().startPower, 0.35);
    QCOMPARE(model.m_ramps.back().endPower, 0.85);
    QCOMPARE(model.m_ramps.back().startPowerRing, 0.9);
    QCOMPARE(model.m_ramps.back().endPowerRing, 0.2);

    model.setStartPointID(5);
    model.setRampLength(1.0);
    model.setStartPower(0.35);
    model.setEndPower(0.85);
    model.setStartPowerRing(0.9);
    model.setEndPowerRing(0.2);
    model.updateRamps();
    QCOMPARE(model.m_ramps.size(), 2);
    QCOMPARE(model.m_ramps.back().startPointID, 5);
    QCOMPARE(model.m_ramps.back().length, 1.0);
    QCOMPARE(model.m_ramps.back().startPower, 0.35);
    QCOMPARE(model.m_ramps.back().endPower, 0.85);
    QCOMPARE(model.m_ramps.back().startPowerRing, 0.9);
    QCOMPARE(model.m_ramps.back().endPowerRing, 0.2);

    model.setStartPointID(2);
    model.setRampLength(1.0);
    model.setStartPower(0.35);
    model.setEndPower(0.85);
    model.setStartPowerRing(0.9);
    model.setEndPowerRing(0.2);
    model.updateRamps();
    QCOMPARE(model.m_ramps.size(), 3);
    QCOMPARE(model.m_ramps.front().startPointID, 1);
    QCOMPARE(model.m_ramps.at(1).startPointID, 2);
    QCOMPARE(model.m_ramps.back().startPointID, 5);
    QCOMPARE(model.m_ramps.at(1).startPointID, 2);
    QCOMPARE(model.m_ramps.at(1).length, 1.0);
    QCOMPARE(model.m_ramps.at(1).startPower, 0.35);
    QCOMPARE(model.m_ramps.at(1).endPower, 0.85);
    QCOMPARE(model.m_ramps.at(1).startPowerRing, 0.9);
    QCOMPARE(model.m_ramps.at(1).endPowerRing, 0.2);

    model.setRampLength(2.0);
    model.setStartPower(0.39);
    model.setEndPower(0.89);
    model.setStartPowerRing(0.95);
    model.setEndPowerRing(0.25);

    model.updateRamps();
    QCOMPARE(model.m_ramps.size(), 3);
    QCOMPARE(model.m_ramps.at(1).startPointID, 2);
    QCOMPARE(model.m_ramps.at(1).length, 2.0);
    QCOMPARE(model.m_ramps.at(1).startPower, 0.39);
    QCOMPARE(model.m_ramps.at(1).endPower, 0.89);
    QCOMPARE(model.m_ramps.at(1).startPowerRing, 0.95);
    QCOMPARE(model.m_ramps.at(1).endPowerRing, 0.25);

    model.setStartPointID(-1);
    QVERIFY(figureEditor.ramps().empty());
    model.setFigureEditor(&figureEditor);

    figureEditor.setFileType(FileType::Seam);

    model.setStartPointID(2);
    QVERIFY(model.ramps().empty());
    model.setRampLength(1.0);
    model.setStartPower(0.35);
    model.setEndPower(0.85);
    model.setStartPowerRing(0.9);
    model.setEndPowerRing(0.2);
    model.updateRamps();

    QCOMPARE(model.ramps().size(), 1);
    QCOMPARE(model.ramps().front().startPointID, 2);
    QCOMPARE(model.ramps().front().length, 1.0);
    QCOMPARE(model.ramps().front().startPower, 0.35);
    QCOMPARE(model.ramps().front().endPower, 0.85);
    QCOMPARE(model.ramps().front().startPowerRing, 0.9);
    QCOMPARE(model.ramps().front().endPowerRing, 0.2);

    QVERIFY(!figureEditor.ramps().empty());
    QCOMPARE(figureEditor.ramps(), model.ramps());

    model.setRampLength(2.0);
    model.setStartPower(0.39);
    model.setEndPower(0.89);
    model.setStartPowerRing(0.95);
    model.setEndPowerRing(0.25);

    model.updateRamps();
    QCOMPARE(model.ramps().size(), 1);
    QCOMPARE(model.ramps().front().startPointID, 2);
    QCOMPARE(model.ramps().front().length, 2.0);
    QCOMPARE(model.ramps().front().startPower, 0.39);
    QCOMPARE(model.ramps().front().endPower, 0.89);
    QCOMPARE(model.ramps().front().startPowerRing, 0.95);
    QCOMPARE(model.ramps().front().endPowerRing, 0.25);

    QVERIFY(!figureEditor.ramps().empty());
    QCOMPARE(figureEditor.ramps(), model.ramps());

    model.setStartPointID(2);
    model.setRampLength(0.0);
    model.updateRamps();

    QVERIFY(figureEditor.ramps().empty());
    QVERIFY(model.ramps().empty());

    model.setStartPointID(3);
    model.setRampLength(0.0);
    model.setStartPower(0.1);
    model.setEndPower(0.2);
    model.setStartPowerRing(0.05);
    model.setEndPowerRing(0.2);
    model.updateRamps();

    QVERIFY(figureEditor.ramps().empty());
    QVERIFY(model.ramps().empty());
}

void RampModelTest::testEraseRamp()
{
    RampModel model;

    std::vector<Ramp> ramps;
    Ramp ramp;
    ramp.startPointID = 2;
    ramp.length = 1.2;
    ramp.startPower = 0.3;
    ramp.endPower = 0.8;
    ramp.startPowerRing = 0.1;
    ramp.endPowerRing = 1.0;
    ramps.push_back(ramp);

    ramp.startPointID = 4;
    ramp.length = 1.5;
    ramp.startPower = 0.25;
    ramp.endPower = 0.75;
    ramp.startPowerRing = 0.0;
    ramp.endPowerRing = 1.0;
    ramps.push_back(ramp);

    model.m_ramps = ramps;

    model.setStartPointID(1);
    model.eraseRamp();
    QCOMPARE(model.m_ramps.size(), 2);

    model.setStartPointID(4);
    model.eraseRamp();
    QCOMPARE(model.m_ramps.size(), 1);

    model.setStartPointID(2);
    model.eraseRamp();
    QCOMPARE(model.m_ramps.size(), 0);

    ramps = {{0, 0.25, 0.0, 1.0, 0.0, 1.0}, {4, 0.3, 0.25, 0.75, 0.3, 0.8}};
    model.setRamps(ramps);
    QVERIFY(!model.ramps().empty());
    model.setStartPointID(4);

    WobbleFigureEditor figureEditor;
    CommandManager commandManager;
    figureEditor.setFileType(FileType::Seam);
    figureEditor.setCommandManager(&commandManager);
    QVERIFY(figureEditor.ramps().empty());
    model.setFigureEditor(&figureEditor);

    model.eraseRamp();
    QVERIFY(!model.ramps().empty());
    QVERIFY(!figureEditor.ramps().empty());

    QCOMPARE(model.ramps().front(), ramps.front());
    QCOMPARE(model.ramps(), figureEditor.ramps());
}

void RampModelTest::testTakeRamps()
{
    RampModel model;
    WobbleFigureEditor figureEditor;

    SeamFigure seam;
    std::vector<Ramp> ramps {{0, 0.25, 0.0, 1.0, 0.0, 1.0}, {4, 0.3, 0.25, 0.75, 0.3, 0.8}};
    seam.ramps = ramps;

    figureEditor.setSeam(seam);

    QVERIFY(model.ramps().empty());

    model.takeRamps();
    QVERIFY(model.ramps().empty());

    model.setFigureEditor(&figureEditor);
    model.takeRamps();
    QVERIFY(!model.ramps().empty());
    for (std::size_t i = 0; i < model.ramps().size(); i++)
    {
        QCOMPARE(model.ramps().at(i), ramps.at(i));
    }
}

void RampModelTest::testGiveBackRamps()
{
    RampModel model;
    WobbleFigureEditor figureEditor;
    CommandManager commandManager;
    figureEditor.setFileType(FileType::Seam);
    figureEditor.setCommandManager(&commandManager);

    SeamFigure seam;
    figureEditor.setSeam(seam);
    QVERIFY(figureEditor.ramps().empty());

    std::vector<Ramp> ramps {{0, 0.25, 0.0, 1.0, 0.0, 1.0}, {4, 0.3, 0.25, 0.75, 0.3, 0.8}};

    QVERIFY(model.ramps().empty());
    model.setRamps(ramps);
    QVERIFY(!model.ramps().empty());

    model.giveBackRamps();
    QVERIFY(figureEditor.ramps().empty());

    model.setFigureEditor(&figureEditor);
    model.giveBackRamps();
    QVERIFY(!figureEditor.ramps().empty());

    for (std::size_t i = 0; i < figureEditor.ramps().size(); i++)
    {
        QCOMPARE(figureEditor.ramps().at(i), ramps.at(i));
    }
}

void RampModelTest::testReset()
{
    RampModel model;

    model.setStartPointID(5);
    QCOMPARE(model.startPointID(), 5);
    model.setRampLength(0.6);
    QCOMPARE(model.rampLength(), 0.6);
    model.setStartPower(0.25);
    QCOMPARE(model.startPower(), 0.25);
    model.setEndPower(0.75);
    QCOMPARE(model.endPower(), 0.75);
    model.setStartPowerRing(1.0);
    QCOMPARE(model.startPowerRing(), 1.0);
    model.setEndPowerRing(0.0);
    QCOMPARE(model.endPowerRing(), 0.0);

    model.reset();

    QCOMPARE(model.startPointID(), 5);      //Start point ID is set by attribute controller. Reset is applied after the start point ID is set and would erase the important start point ID before using it.
    QCOMPARE(model.rampLength(), 0.0);
    QCOMPARE(model.startPower(), 0.0);
    QCOMPARE(model.endPower(), 0.0);
    QCOMPARE(model.startPowerRing(), 0.0);
    QCOMPARE(model.endPowerRing(), 0.0);
}

void RampModelTest::testUpdateRampModel()
{
    RampModel model;

    model.setRampLength(0.6);
    QCOMPARE(model.rampLength(), 0.6);
    model.setStartPower(0.25);
    QCOMPARE(model.startPower(), 0.25);
    model.setEndPower(0.75);
    QCOMPARE(model.endPower(), 0.75);
    model.setStartPowerRing(1.0);
    QCOMPARE(model.startPowerRing(), 1.0);
    model.setEndPowerRing(0.0);
    QCOMPARE(model.endPowerRing(), 0.0);

    model.updateRampModel();

    QCOMPARE(model.startPointID(), -1);      //Start point ID is set by attribute controller. Reset is applied after the start point ID is set and would erase the important start point ID before using it.
    QCOMPARE(model.rampLength(), 0.0);
    QCOMPARE(model.startPower(), 0.0);
    QCOMPARE(model.endPower(), 0.0);
    QCOMPARE(model.startPowerRing(), 0.0);
    QCOMPARE(model.endPowerRing(), 0.0);

    model.setStartPointID(4);
    QCOMPARE(model.startPointID(), 4);
    model.setRampLength(0.6);
    QCOMPARE(model.rampLength(), 0.6);
    model.setStartPower(0.25);
    QCOMPARE(model.startPower(), 0.25);
    model.setEndPower(0.75);
    QCOMPARE(model.endPower(), 0.75);
    model.setStartPowerRing(1.0);
    QCOMPARE(model.startPowerRing(), 1.0);
    model.setEndPowerRing(0.0);
    QCOMPARE(model.endPowerRing(), 0.0);

    model.updateRampModel();

    QCOMPARE(model.startPointID(), 4);
    QCOMPARE(model.rampLength(), 0.0);
    QCOMPARE(model.startPower(), 0.0);
    QCOMPARE(model.endPower(), 0.0);
    QCOMPARE(model.startPowerRing(), 0.0);
    QCOMPARE(model.endPowerRing(), 0.0);

    Ramp ramp;
    ramp.startPointID = 4;
    ramp.length = 1.5;
    ramp.startPower = 0.25;
    ramp.endPower = 0.75;
    ramp.startPowerRing = 0.0;
    ramp.endPowerRing = 1.0;
    std::vector<Ramp> ramps;
    ramps.push_back(ramp);

    model.m_ramps = ramps;

    model.updateRampModel();

    QCOMPARE(model.startPointID(), 4);
    QCOMPARE(model.rampLength(), 1.5);
    QCOMPARE(model.startPower(), 0.25);
    QCOMPARE(model.endPower(), 0.75);
    QCOMPARE(model.startPowerRing(), 0.0);
    QCOMPARE(model.endPowerRing(), 1.0);

    model.setStartPointID(5);
    QCOMPARE(model.startPointID(), 5);

    model.updateRampModel();

    QCOMPARE(model.startPointID(), 5);
    QCOMPARE(model.rampLength(), 0.0);
    QCOMPARE(model.startPower(), 0.0);
    QCOMPARE(model.endPower(), 0.0);
    QCOMPARE(model.startPowerRing(), 0.0);
    QCOMPARE(model.endPowerRing(), 0.0);

    model.m_ramps = ramps;

    QVERIFY(!model.ramps().empty());

    WobbleFigureEditor figureEditor;
    CommandManager commandManager;
    figureEditor.setFileType(FileType::Seam);
    figureEditor.setCommandManager(&commandManager);
    model.setFigureEditor(&figureEditor);
    model.updateRampModel();
    QVERIFY(model.ramps().empty());
    model.setFigureEditor(nullptr);

    SeamFigure seam;
    ramps = {{0, 0.25, 0.0, 1.0, 0.0, 1.0}, {4, 0.3, 0.25, 0.75, 0.3, 0.8}};
    seam.ramps = ramps;

    figureEditor.setSeam(seam);

    model.updateRampModel();
    QVERIFY(model.ramps().empty());

    QCOMPARE(model.startPointID(), 5);
    QCOMPARE(model.rampLength(), 0.0);
    QCOMPARE(model.startPower(), 0.0);
    QCOMPARE(model.endPower(), 0.0);
    QCOMPARE(model.startPowerRing(), 0.0);
    QCOMPARE(model.endPowerRing(), 0.0);

    model.setFigureEditor(&figureEditor);
    model.setStartPointID(4);

    QCOMPARE(model.startPointID(), 4);
    QCOMPARE(model.rampLength(), 0.3);
    QCOMPARE(model.startPower(), 0.25);
    QCOMPARE(model.endPower(), 0.75);
    QCOMPARE(model.startPowerRing(), 0.3);
    QCOMPARE(model.endPowerRing(), 0.8);

    model.reset();
    ramps = {{4, 0.3, 0.25, 0.75, 0.3, 0.8}};
    figureEditor.setRamps(ramps);

    QCOMPARE(model.startPointID(), 4);
    QCOMPARE(model.rampLength(), 0.3);
    QCOMPARE(model.startPower(), 0.25);
    QCOMPARE(model.endPower(), 0.75);
    QCOMPARE(model.startPowerRing(), 0.3);
    QCOMPARE(model.endPowerRing(), 0.8);
}

void RampModelTest::testSearchRamp_data()
{
    QTest::addColumn<QVector<Ramp>>("availableRamps");

    QVector<Ramp> ramps;

    Ramp ramp;
    ramp.startPointID = 0;
    ramp.length = 0.25;
    ramp.startPower = 0.0;
    ramp.endPower = 1.0;
    ramp.startPowerRing = 0.25;
    ramp.endPowerRing = 0.75;

    ramps.push_back(ramp);

    ramp.startPointID = 1;
    ramp.length = 1.5;
    ramp.startPower = 1.0;
    ramp.endPower = 0.5;
    ramp.startPowerRing = 0.25;
    ramp.endPowerRing = 0.5;

    ramps.push_back(ramp);

    ramp.startPointID = 3;
    ramp.length = 1.5;
    ramp.startPower = 1.0;
    ramp.endPower = 0.5;
    ramp.startPowerRing = 0.25;
    ramp.endPowerRing = 0.5;

    ramps.push_back(ramp);

    ramp.startPointID = 6;
    ramp.length = 1.5;
    ramp.startPower = 1.0;
    ramp.endPower = 0.5;
    ramp.startPowerRing = 0.25;
    ramp.endPowerRing = 0.5;

    ramps.push_back(ramp);

    ramp.startPointID = 7;
    ramp.length = 1.5;
    ramp.startPower = 1.0;
    ramp.endPower = 0.5;
    ramp.startPowerRing = 0.25;
    ramp.endPowerRing = 0.5;

    ramps.push_back(ramp);

    QTest::addRow("Ramps") << ramps;
}

void RampModelTest::testSearchRamp()
{
    RampModel model;

    QFETCH(QVector<Ramp>, availableRamps);
    std::vector<Ramp> ramps {availableRamps.begin(), availableRamps.end()};
    model.setRamps(ramps);

    auto noRampFound = model.searchRamp();

    QVERIFY(!model.foundRamp(noRampFound));

    auto id = 0;
    model.setStartPointID(id);
    auto rampFound = model.searchRamp();
    QVERIFY(model.foundRamp(rampFound));

    QCOMPARE(rampFound->startPointID, availableRamps.at(id).startPointID);
    QCOMPARE(rampFound->length, availableRamps.at(id).length);
    QCOMPARE(rampFound->startPower, availableRamps.at(id).startPower);
    QCOMPARE(rampFound->endPower, availableRamps.at(id).endPower);
    QCOMPARE(rampFound->startPowerRing, availableRamps.at(id).startPowerRing);
    QCOMPARE(rampFound->endPowerRing, availableRamps.at(id).endPowerRing);

    id = 1;
    model.setStartPointID(id);
    rampFound = model.searchRamp();

    QVERIFY(model.foundRamp(rampFound));

    QCOMPARE(rampFound->startPointID, availableRamps.at(id).startPointID);
    QCOMPARE(rampFound->length, availableRamps.at(id).length);
    QCOMPARE(rampFound->startPower, availableRamps.at(id).startPower);
    QCOMPARE(rampFound->endPower, availableRamps.at(id).endPower);
    QCOMPARE(rampFound->startPowerRing, availableRamps.at(id).startPowerRing);
    QCOMPARE(rampFound->endPowerRing, availableRamps.at(id).endPowerRing);

    id = 2;
    model.setStartPointID(id);
    noRampFound = model.searchRamp();

    QVERIFY(!model.foundRamp(noRampFound));

    id = 3;
    model.setStartPointID(id);
    rampFound = model.searchRamp();

    QVERIFY(model.foundRamp(rampFound));

    QCOMPARE(rampFound->startPointID, availableRamps.at(id - 1).startPointID);
    QCOMPARE(rampFound->length, availableRamps.at(id - 1).length);
    QCOMPARE(rampFound->startPower, availableRamps.at(id - 1).startPower);
    QCOMPARE(rampFound->endPower, availableRamps.at(id - 1).endPower);
    QCOMPARE(rampFound->startPowerRing, availableRamps.at(id - 1).startPowerRing);
    QCOMPARE(rampFound->endPowerRing, availableRamps.at(id - 1).endPowerRing);

    id = 4;
    model.setStartPointID(id);
    noRampFound = model.searchRamp();

    QVERIFY(!model.foundRamp(noRampFound));

    id = 5;
    model.setStartPointID(id);
    noRampFound = model.searchRamp();

    QVERIFY(!model.foundRamp(noRampFound));

    id = 6;
    model.setStartPointID(id);
    rampFound = model.searchRamp();

    QVERIFY(model.foundRamp(rampFound));

    QCOMPARE(rampFound->startPointID, availableRamps.at(id - 3).startPointID);
    QCOMPARE(rampFound->length, availableRamps.at(id - 3).length);
    QCOMPARE(rampFound->startPower, availableRamps.at(id - 3).startPower);
    QCOMPARE(rampFound->endPower, availableRamps.at(id - 3).endPower);
    QCOMPARE(rampFound->startPowerRing, availableRamps.at(id - 3).startPowerRing);
    QCOMPARE(rampFound->endPowerRing, availableRamps.at(id - 3).endPowerRing);

    id = 7;
    model.setStartPointID(id);
    rampFound = model.searchRamp();

    QVERIFY(model.foundRamp(rampFound));

    QCOMPARE(rampFound->startPointID, availableRamps.at(id - 3).startPointID);
    QCOMPARE(rampFound->length, availableRamps.at(id - 3).length);
    QCOMPARE(rampFound->startPower, availableRamps.at(id - 3).startPower);
    QCOMPARE(rampFound->endPower, availableRamps.at(id - 3).endPower);
    QCOMPARE(rampFound->startPowerRing, availableRamps.at(id - 3).startPowerRing);
    QCOMPARE(rampFound->endPowerRing, availableRamps.at(id - 3).endPowerRing);
}

void RampModelTest::testSearchRampIt_data()
{
    QTest::addColumn<QVector<Ramp>>("availableRamps");

    QVector<Ramp> ramps;

    Ramp ramp;
    ramp.startPointID = 0;
    ramp.length = 0.25;
    ramp.startPower = 0.0;
    ramp.endPower = 1.0;
    ramp.startPowerRing = 0.25;
    ramp.endPowerRing = 0.75;

    ramps.push_back(ramp);

    ramp.startPointID = 1;
    ramp.length = 1.5;
    ramp.startPower = 1.0;
    ramp.endPower = 0.5;
    ramp.startPowerRing = 0.25;
    ramp.endPowerRing = 0.5;

    ramps.push_back(ramp);

    ramp.startPointID = 3;
    ramp.length = 1.5;
    ramp.startPower = 1.0;
    ramp.endPower = 0.5;
    ramp.startPowerRing = 0.25;
    ramp.endPowerRing = 0.5;

    ramps.push_back(ramp);

    ramp.startPointID = 6;
    ramp.length = 1.5;
    ramp.startPower = 1.0;
    ramp.endPower = 0.5;
    ramp.startPowerRing = 0.25;
    ramp.endPowerRing = 0.5;

    ramps.push_back(ramp);

    ramp.startPointID = 7;
    ramp.length = 1.5;
    ramp.startPower = 1.0;
    ramp.endPower = 0.5;
    ramp.startPowerRing = 0.25;
    ramp.endPowerRing = 0.5;

    ramps.push_back(ramp);

    QTest::addRow("Ramps") << ramps;
}

void RampModelTest::testSearchRampIt()
{
    RampModel model;

    QFETCH(QVector<Ramp>, availableRamps);
    std::vector<Ramp> ramps {availableRamps.begin(), availableRamps.end()};
    model.setRamps(ramps);

    auto noRampFound = model.searchRamp();

    QVERIFY(noRampFound == model.m_ramps.end());

    auto id = 0;
    model.setStartPointID(id);
    auto rampFound = model.searchRamp();

    QCOMPARE(rampFound->startPointID, availableRamps.at(id).startPointID);
    QCOMPARE(rampFound->length, availableRamps.at(id).length);
    QCOMPARE(rampFound->startPower, availableRamps.at(id).startPower);
    QCOMPARE(rampFound->endPower, availableRamps.at(id).endPower);
    QCOMPARE(rampFound->startPowerRing, availableRamps.at(id).startPowerRing);
    QCOMPARE(rampFound->endPowerRing, availableRamps.at(id).endPowerRing);

    id = 1;
    model.setStartPointID(id);
    rampFound = model.searchRamp();

    QCOMPARE(rampFound->startPointID, availableRamps.at(id).startPointID);
    QCOMPARE(rampFound->length, availableRamps.at(id).length);
    QCOMPARE(rampFound->startPower, availableRamps.at(id).startPower);
    QCOMPARE(rampFound->endPower, availableRamps.at(id).endPower);
    QCOMPARE(rampFound->startPowerRing, availableRamps.at(id).startPowerRing);
    QCOMPARE(rampFound->endPowerRing, availableRamps.at(id).endPowerRing);

    id = 2;
    model.setStartPointID(id);
    noRampFound = model.searchRamp();

    QVERIFY(noRampFound == model.m_ramps.end());

    id = 3;
    model.setStartPointID(id);
    rampFound = model.searchRamp();

    QCOMPARE(rampFound->startPointID, availableRamps.at(id - 1).startPointID);
    QCOMPARE(rampFound->length, availableRamps.at(id - 1).length);
    QCOMPARE(rampFound->startPower, availableRamps.at(id - 1).startPower);
    QCOMPARE(rampFound->endPower, availableRamps.at(id - 1).endPower);
    QCOMPARE(rampFound->startPowerRing, availableRamps.at(id - 1).startPowerRing);
    QCOMPARE(rampFound->endPowerRing, availableRamps.at(id - 1).endPowerRing);

    id = 4;
    model.setStartPointID(id);
    noRampFound = model.searchRamp();

    QVERIFY(noRampFound == model.m_ramps.end());

    id = 5;
    model.setStartPointID(id);
    noRampFound = model.searchRamp();

    QVERIFY(noRampFound == model.m_ramps.end());

    id = 6;
    model.setStartPointID(id);
    rampFound = model.searchRamp();

    QCOMPARE(rampFound->startPointID, availableRamps.at(id - 3).startPointID);
    QCOMPARE(rampFound->length, availableRamps.at(id - 3).length);
    QCOMPARE(rampFound->startPower, availableRamps.at(id - 3).startPower);
    QCOMPARE(rampFound->endPower, availableRamps.at(id - 3).endPower);
    QCOMPARE(rampFound->startPowerRing, availableRamps.at(id - 3).startPowerRing);
    QCOMPARE(rampFound->endPowerRing, availableRamps.at(id - 3).endPowerRing);

    id = 7;
    model.setStartPointID(id);
    rampFound = model.searchRamp();

    QCOMPARE(rampFound->startPointID, availableRamps.at(id - 3).startPointID);
    QCOMPARE(rampFound->length, availableRamps.at(id - 3).length);
    QCOMPARE(rampFound->startPower, availableRamps.at(id - 3).startPower);
    QCOMPARE(rampFound->endPower, availableRamps.at(id - 3).endPower);
    QCOMPARE(rampFound->startPowerRing, availableRamps.at(id - 3).startPowerRing);
    QCOMPARE(rampFound->endPowerRing, availableRamps.at(id - 3).endPowerRing);
}

void RampModelTest::testFoundRamp_data()
{
    QTest::addColumn<QVector<Ramp>>("availableRamps");

    QVector<Ramp> ramps;

    Ramp ramp;
    ramp.startPointID = 0;
    ramp.length = 0.25;
    ramp.startPower = 0.0;
    ramp.endPower = 1.0;
    ramp.startPowerRing = 0.25;
    ramp.endPowerRing = 0.75;

    ramps.push_back(ramp);

    ramp.startPointID = 1;
    ramp.length = 1.5;
    ramp.startPower = 1.0;
    ramp.endPower = 0.5;
    ramp.startPowerRing = 0.25;
    ramp.endPowerRing = 0.5;

    ramps.push_back(ramp);

    ramp.startPointID = 3;
    ramp.length = 1.5;
    ramp.startPower = 1.0;
    ramp.endPower = 0.5;
    ramp.startPowerRing = 0.25;
    ramp.endPowerRing = 0.5;

    ramps.push_back(ramp);

    ramp.startPointID = 6;
    ramp.length = 1.5;
    ramp.startPower = 1.0;
    ramp.endPower = 0.5;
    ramp.startPowerRing = 0.25;
    ramp.endPowerRing = 0.5;

    ramps.push_back(ramp);

    ramp.startPointID = 7;
    ramp.length = 1.5;
    ramp.startPower = 1.0;
    ramp.endPower = 0.5;
    ramp.startPowerRing = 0.25;
    ramp.endPowerRing = 0.5;

    ramps.push_back(ramp);

    QTest::addRow("Ramps") << ramps;
}

void RampModelTest::testFoundRamp()
{
    RampModel model;

    QFETCH(QVector<Ramp>, availableRamps);
    std::vector<Ramp> ramps {availableRamps.begin(), availableRamps.end()};
    model.setRamps(ramps);

    auto noRampFound = model.searchRamp();

    QVERIFY(!model.foundRamp(noRampFound));

    auto id = 0;
    model.setStartPointID(id);
    auto rampFound = model.searchRamp();

    QVERIFY(model.foundRamp(rampFound));
    QCOMPARE(rampFound->startPointID, availableRamps.at(id).startPointID);
    QCOMPARE(rampFound->length, availableRamps.at(id).length);
    QCOMPARE(rampFound->startPower, availableRamps.at(id).startPower);
    QCOMPARE(rampFound->endPower, availableRamps.at(id).endPower);
    QCOMPARE(rampFound->startPowerRing, availableRamps.at(id).startPowerRing);
    QCOMPARE(rampFound->endPowerRing, availableRamps.at(id).endPowerRing);

    id = 1;
    model.setStartPointID(id);
    rampFound = model.searchRamp();

    QVERIFY(model.foundRamp(rampFound));
    QCOMPARE(rampFound->startPointID, availableRamps.at(id).startPointID);
    QCOMPARE(rampFound->length, availableRamps.at(id).length);
    QCOMPARE(rampFound->startPower, availableRamps.at(id).startPower);
    QCOMPARE(rampFound->endPower, availableRamps.at(id).endPower);
    QCOMPARE(rampFound->startPowerRing, availableRamps.at(id).startPowerRing);
    QCOMPARE(rampFound->endPowerRing, availableRamps.at(id).endPowerRing);

    id = 2;
    model.setStartPointID(id);
    noRampFound = model.searchRamp();

    QVERIFY(!model.foundRamp(noRampFound));

    id = 3;
    model.setStartPointID(id);
    rampFound = model.searchRamp();

    QVERIFY(model.foundRamp(rampFound));
    QCOMPARE(rampFound->startPointID, availableRamps.at(id - 1).startPointID);
    QCOMPARE(rampFound->length, availableRamps.at(id - 1).length);
    QCOMPARE(rampFound->startPower, availableRamps.at(id - 1).startPower);
    QCOMPARE(rampFound->endPower, availableRamps.at(id - 1).endPower);
    QCOMPARE(rampFound->startPowerRing, availableRamps.at(id - 1).startPowerRing);
    QCOMPARE(rampFound->endPowerRing, availableRamps.at(id - 1).endPowerRing);

    id = 4;
    model.setStartPointID(id);
    noRampFound = model.searchRamp();

    QVERIFY(!model.foundRamp(noRampFound));

    id = 5;
    model.setStartPointID(id);
    noRampFound = model.searchRamp();

    QVERIFY(!model.foundRamp(noRampFound));

    id = 6;
    model.setStartPointID(id);
    rampFound = model.searchRamp();

    QVERIFY(model.foundRamp(rampFound));
    QCOMPARE(rampFound->startPointID, availableRamps.at(id - 3).startPointID);
    QCOMPARE(rampFound->length, availableRamps.at(id - 3).length);
    QCOMPARE(rampFound->startPower, availableRamps.at(id - 3).startPower);
    QCOMPARE(rampFound->endPower, availableRamps.at(id - 3).endPower);
    QCOMPARE(rampFound->startPowerRing, availableRamps.at(id - 3).startPowerRing);
    QCOMPARE(rampFound->endPowerRing, availableRamps.at(id - 3).endPowerRing);

    id = 7;
    model.setStartPointID(id);
    rampFound = model.searchRamp();

    QVERIFY(model.foundRamp(rampFound));
    QCOMPARE(rampFound->startPointID, availableRamps.at(id - 3).startPointID);
    QCOMPARE(rampFound->length, availableRamps.at(id - 3).length);
    QCOMPARE(rampFound->startPower, availableRamps.at(id - 3).startPower);
    QCOMPARE(rampFound->endPower, availableRamps.at(id - 3).endPower);
    QCOMPARE(rampFound->startPowerRing, availableRamps.at(id - 3).startPowerRing);
    QCOMPARE(rampFound->endPowerRing, availableRamps.at(id - 3).endPowerRing);
}

void RampModelTest::testUpdateRampModelProperties()
{
    RampModel model;

    std::vector<Ramp> ramps;

    Ramp ramp;
    ramp.startPointID = 0;
    ramp.length = 0.25;
    ramp.startPower = 0.0;
    ramp.endPower = 1.0;
    ramp.startPowerRing = 0.25;
    ramp.endPowerRing = 0.75;

    ramps.push_back(ramp);

    model.setRamps(ramps);

    auto id = 0;
    model.setStartPointID(id);

    const auto& rampIt = model.searchRamp();
    model.updateRampModelProperties(*rampIt);

    QVERIFY(model.foundRamp(rampIt));

    QCOMPARE(model.startPointID(), ramp.startPointID);
    QCOMPARE(model.rampLength(), ramp.length);
    QCOMPARE(model.startPower(), ramp.startPower);
    QCOMPARE(model.endPower(), ramp.endPower);
    QCOMPARE(model.startPowerRing(), ramp.startPowerRing);
    QCOMPARE(model.endPowerRing(), ramp.endPowerRing);
}

void RampModelTest::testAppendRamp()
{
    RampModel model;

    auto startPointID = 0;
    auto rampLength = 0.5;
    auto startPower = 0.25;
    auto endPower = 0.75;
    auto startPowerRing = 1.0;
    auto endPowerRing = 0.0;

    model.setStartPointID(startPointID);
    model.setRampLength(rampLength);
    model.setStartPower(startPower);
    model.setEndPower(endPower);
    model.setStartPowerRing(startPowerRing);
    model.setEndPowerRing(endPowerRing);

    model.appendRamp();

    auto ramps = model.ramps();

    QCOMPARE(ramps.back().startPointID, startPointID);
    QCOMPARE(ramps.back().length, rampLength);
    QCOMPARE(ramps.back().startPower, startPower);
    QCOMPARE(ramps.back().endPower, endPower);
    QCOMPARE(ramps.back().startPowerRing, startPowerRing);
    QCOMPARE(ramps.back().endPowerRing, endPowerRing);
}

void RampModelTest::testSortRamps()
{
    RampModel model;

    auto startPointID = 0;
    model.setStartPointID(startPointID);
    model.appendRamp();

    startPointID = 4;
    model.setStartPointID(startPointID);
    model.appendRamp();

    startPointID = 2;
    model.setStartPointID(startPointID);
    model.appendRamp();

    startPointID = 8;
    model.setStartPointID(startPointID);
    model.appendRamp();

    model.sortRamps();

    std::array<int, 4> expectedStartPointIDs {0, 2, 4, 8};
    auto arrayIndex = 0;
    for (const auto& element : model.ramps())
    {
        QCOMPARE(element.startPointID, expectedStartPointIDs.at(arrayIndex));
        arrayIndex++;
    }
}

void RampModelTest::testUpdateFoundRamp_data()
{
    QTest::addColumn<QVector<Ramp>>("availableRamps");

    QVector<Ramp> ramps;

    Ramp ramp;
    ramp.startPointID = 0;
    ramp.length = 0.25;
    ramp.startPower = 0.0;
    ramp.endPower = 1.0;
    ramp.startPowerRing = 0.25;
    ramp.endPowerRing = 0.75;

    ramps.push_back(ramp);

    ramp.startPointID = 1;
    ramp.length = 1.5;
    ramp.startPower = 1.0;
    ramp.endPower = 0.5;
    ramp.startPowerRing = 0.25;
    ramp.endPowerRing = 0.5;

    ramps.push_back(ramp);

    ramp.startPointID = 3;
    ramp.length = 1.5;
    ramp.startPower = 1.0;
    ramp.endPower = 0.5;
    ramp.startPowerRing = 0.25;
    ramp.endPowerRing = 0.5;

    ramps.push_back(ramp);

    ramp.startPointID = 6;
    ramp.length = 1.5;
    ramp.startPower = 1.0;
    ramp.endPower = 0.5;
    ramp.startPowerRing = 0.25;
    ramp.endPowerRing = 0.5;

    ramps.push_back(ramp);

    ramp.startPointID = 7;
    ramp.length = 1.5;
    ramp.startPower = 1.0;
    ramp.endPower = 0.5;
    ramp.startPowerRing = 0.25;
    ramp.endPowerRing = 0.5;

    ramps.push_back(ramp);

    QTest::addRow("Ramps") << ramps;
}

void RampModelTest::testUpdateFoundRamp()
{
    RampModel model;

    QFETCH(QVector<Ramp>, availableRamps);
    std::vector<Ramp> ramps {availableRamps.begin(), availableRamps.end()};
    model.setRamps(ramps);

    auto noRampFound = model.searchRamp();

    QVERIFY(noRampFound == model.m_ramps.end());

    auto id = 1;
    model.setStartPointID(id);
    auto rampFound = model.searchRamp();

    QCOMPARE(rampFound->startPointID, availableRamps.at(id).startPointID);
    QCOMPARE(rampFound->length, availableRamps.at(id).length);
    QCOMPARE(rampFound->startPower, availableRamps.at(id).startPower);
    QCOMPARE(rampFound->endPower, availableRamps.at(id).endPower);
    QCOMPARE(rampFound->startPowerRing, availableRamps.at(id).startPowerRing);
    QCOMPARE(rampFound->endPowerRing, availableRamps.at(id).endPowerRing);

    model.setRampLength(2.0);
    model.setStartPower(0.3);
    model.setEndPower(0.6);
    model.setStartPowerRing(0.7);
    model.setEndPowerRing(0.5);

    model.updateFoundRamp(rampFound);

    auto rampFoundAfterUpdate = model.searchRamp();

    QCOMPARE(rampFoundAfterUpdate->startPointID, 1);
    QCOMPARE(rampFoundAfterUpdate->length, 2.0);
    QCOMPARE(rampFoundAfterUpdate->startPower, 0.3);
    QCOMPARE(rampFoundAfterUpdate->endPower, 0.6);
    QCOMPARE(rampFoundAfterUpdate->startPowerRing, 0.7);
    QCOMPARE(rampFoundAfterUpdate->endPowerRing, 0.5);
}

void RampModelTest::testIsStartPointIDValid()
{
    RampModel model;

    QVERIFY(!model.isStartPointIDValid());          //Normally the start point id validation is done by the attribute controller.

    int id = 0;
    model.setStartPointID(id);
    QVERIFY(model.isStartPointIDValid());
    id++;

    model.setStartPointID(id);
    QVERIFY(model.isStartPointIDValid());
}

QTEST_GUILESS_MAIN(RampModelTest)
#include "rampModelTest.moc"
