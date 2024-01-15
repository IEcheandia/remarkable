#include <QTest>
#include <QSignalSpy>
#include <QDebug>

#include "../src/WobbleFigureEditor.h"
#include "../src/fileType.h"
#include "../src/commandManager.h"

using precitec::scantracker::components::wobbleFigureEditor::WobbleFigureEditor;
using precitec::scantracker::components::wobbleFigureEditor::FigureCreator;
using precitec::scantracker::components::wobbleFigureEditor::CommandManager;

using precitec::scantracker::components::wobbleFigureEditor::FileType;
using precitec::scanmaster::components::wobbleFigureEditor::powerModulationMode::PowerModulationMode;

class WobbleFigureEditorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testFileType();
    void testSeamPoints();
    void testRamps();
    void testNewWobbleFigure();
    //void testCommandManager();

private:
    QTemporaryDir m_dir;
};

void WobbleFigureEditorTest::initTestCase()
{
    QVERIFY(m_dir.isValid());
    QVERIFY(QDir{}.mkpath(m_dir.filePath(QStringLiteral("config"))));
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());
}

void WobbleFigureEditorTest::testCtor()
{
    WobbleFigureEditor figureEditor;
    CommandManager commandManager;
    figureEditor.setCommandManager(&commandManager);
    QSignalSpy fileTypeChanged{&figureEditor, &WobbleFigureEditor::fileTypeChanged};
    figureEditor.setFileType(FileType::Seam);
    QCOMPARE(figureEditor.fileType(), FileType::Seam);
    QCOMPARE(fileTypeChanged.count(), 1);

    QCOMPARE(figureEditor.figure(), nullptr);
    QCOMPARE(figureEditor.fileModel(), nullptr);
    QCOMPARE(figureEditor.figureCreator(), nullptr);
    QCOMPARE(figureEditor.fileType(), FileType::Seam);
    QCOMPARE(figureEditor.commandManager(), &commandManager);
    QCOMPARE(figureEditor.m_oldFigureScale, 1);
    QCOMPARE(figureEditor.figureScale(), 1000);
    QCOMPARE(figureEditor.numberOfPoints(), 0);
    QCOMPARE(figureEditor.offset(), QPoint(0, 0));
}

void WobbleFigureEditorTest::testFileType()
{
    WobbleFigureEditor figureEditor;
    QSignalSpy fileTypeChanged{&figureEditor, &WobbleFigureEditor::fileTypeChanged};
    QVERIFY(fileTypeChanged.isValid());
    QCOMPARE(figureEditor.fileType(), FileType::None);
    QCOMPARE(fileTypeChanged.count(), 0);

    figureEditor.setFileType(FileType::Seam);
    QCOMPARE(figureEditor.fileType(), FileType::Seam);
    QCOMPARE(fileTypeChanged.count(), 1);

    figureEditor.setFileType(FileType::Wobble);
    QCOMPARE(figureEditor.fileType(), FileType::Wobble);
    QCOMPARE(fileTypeChanged.count(), 2);

    figureEditor.setFileType(FileType::Overlay);
    QCOMPARE(figureEditor.fileType(), FileType::Overlay);
    QCOMPARE(fileTypeChanged.count(), 3);

    figureEditor.setFileType(FileType::Seam);
    QCOMPARE(figureEditor.fileType(), FileType::Seam);
    QCOMPARE(fileTypeChanged.count(), 4);
}

void WobbleFigureEditorTest::testNewWobbleFigure()
{
    WobbleFigureEditor figureEditor;
    FigureCreator figureCreator;

    figureCreator.setFigureShape(FigureCreator::Circle);
    figureCreator.createFigure();

    figureEditor.setFigureCreator(&figureCreator);
    figureEditor.setFileType(FileType::Wobble);
}

void WobbleFigureEditorTest::testSeamPoints()
{
    WobbleFigureEditor figureEditor;
    CommandManager commandManager;
    figureEditor.setCommandManager(&commandManager);
    QSignalSpy fileTypeChanged{&figureEditor, &WobbleFigureEditor::fileTypeChanged};
    figureEditor.setFileType(FileType::Seam);
    QCOMPARE(figureEditor.fileType(), FileType::Seam);
    QCOMPARE(fileTypeChanged.count(), 1);

    QSignalSpy seamPointsChanged{&figureEditor, &WobbleFigureEditor::seamPointsChanged};
    QVERIFY(seamPointsChanged.isValid());
    QCOMPARE(seamPointsChanged.count(), 0);

    RTC6::seamFigure::SeamFigure seam;
    RTC6::seamFigure::command::Order order;
    order.endPosition = std::make_pair(1.0, 0.5);
    order.power = 0.2;
    order.ringPower = 0.3;
    order.velocity = 100.0;
    seam.figure.push_back(order);

    figureEditor.setSeam(seam);
    QCOMPARE(seamPointsChanged.count(), 1);
    QCOMPARE(figureEditor.seamPoints(), seam.figure);
}

void WobbleFigureEditorTest::testRamps()
{
    WobbleFigureEditor figureEditor;
    CommandManager commandManager;
    figureEditor.setCommandManager(&commandManager);
    QSignalSpy fileTypeChanged{&figureEditor, &WobbleFigureEditor::fileTypeChanged};
    figureEditor.setFileType(FileType::Seam);
    QCOMPARE(figureEditor.fileType(), FileType::Seam);
    QCOMPARE(fileTypeChanged.count(), 1);

    QSignalSpy rampsChanged{&figureEditor, &WobbleFigureEditor::rampsChanged};
    QVERIFY(rampsChanged.isValid());
    QCOMPARE(rampsChanged.count(), 0);

    RTC6::seamFigure::SeamFigure seam;
    RTC6::seamFigure::Ramp ramp;
    ramp.startPointID = 0;
    ramp.length = 1.0;
    ramp.startPower = 0.5;
    ramp.endPower = 1.0;
    ramp.startPowerRing = 0.25;
    ramp.endPowerRing = 0.95;
    seam.ramps.push_back(ramp);

    figureEditor.setRamps(seam.ramps);
    QCOMPARE(rampsChanged.count(), 1);
    figureEditor.setRamps(seam.ramps);
    QCOMPARE(rampsChanged.count(), 1);

    QCOMPARE(figureEditor.ramps(), seam.ramps);
}

//void WobbleFigureEditorTest::testCommandManager()
//{
//    CommandManager commandManager;
//    WobbleFigureEditor figureEditor;
//    QSignalSpy commandManagerChanged{&figureEditor, &WobbleFigureEditor::commandManagerChanged};
//    QVERIFY(commandManagerChanged.isValid());
//    QCOMPARE(commandManagerChanged.count(), 0);

//    figureEditor.setCommandManager(&commandManager);
//    QCOMPARE(figureEditor.commandManager(), commandManager);
//    QCOMPARE(commandManagerChanged.count(), 1);

//    figureEditor.setCommandManager(nullptr);
//    QCOMPARE(figureEditor.commandManager(), nullptr);
//    QCOMPARE(commandManagerChanged.count(), 2);
//}

QTEST_GUILESS_MAIN(WobbleFigureEditorTest)
#include "wobbleFigureEditorTest.moc"
