#include <QTest>
#include <QSignalSpy>

#include "../src/fileSaveHandler.h"
#include "../src/FileModel.h"

using precitec::scantracker::components::wobbleFigureEditor::FileSaveHandler;
using precitec::scantracker::components::wobbleFigureEditor::FileType;
using precitec::scantracker::components::wobbleFigureEditor::FileModel;

class FileSaveHandlerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testType();
    void testNumber();
    void testNumberAlreadyUsed();
    void testSearchLowerNumber();
    void testFileModel();
    void testSearchAvailableNumber();
    void testCheckIfNumberIsAlreadyUsed();
    void testSetTypeAndPrefix();
    void testGetNumberFromFileName();

private:
    QTemporaryDir m_dir;
};

void FileSaveHandlerTest::initTestCase()
{
    QVERIFY(m_dir.isValid());
    QVERIFY(QDir{}.mkpath(m_dir.filePath(QStringLiteral("config"))));
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());
}

void FileSaveHandlerTest::testCtor()
{
    FileSaveHandler saveHandler;

    QCOMPARE(saveHandler.type(), FileType::None);
    QVERIFY(!saveHandler.fileModel());
    QCOMPARE(saveHandler.number(), 0);
    QVERIFY(!saveHandler.numberAlreadyUsed());
    QVERIFY(!saveHandler.searchLowerNumber());
    QCOMPARE(saveHandler.filePrefix(), "");
    QCOMPARE(saveHandler.fileTypeLabel(), "");
}

void FileSaveHandlerTest::testType()
{
    FileSaveHandler saveHandler;

    QSignalSpy typeChanged {&saveHandler, &FileSaveHandler::typeChanged};
    QVERIFY(typeChanged.isValid());
    QCOMPARE(typeChanged.count(), 0);

    saveHandler.setType(FileType::Seam);
    QCOMPARE(typeChanged.count(), 1);
    QCOMPARE(saveHandler.type(), FileType::Seam);

    saveHandler.setType(FileType::Seam);
    QCOMPARE(typeChanged.count(), 1);
    QCOMPARE(saveHandler.type(), FileType::Seam);

    saveHandler.setType(FileType::Wobble);
    QCOMPARE(typeChanged.count(), 2);
    QCOMPARE(saveHandler.type(), FileType::Wobble);

    saveHandler.setType(FileType::Overlay);
    QCOMPARE(typeChanged.count(), 3);
    QCOMPARE(saveHandler.type(), FileType::Overlay);

    saveHandler.setType(FileType::Basic);
    QCOMPARE(typeChanged.count(), 4);
    QCOMPARE(saveHandler.type(), FileType::Basic);
}

void FileSaveHandlerTest::testNumber()
{
    FileSaveHandler saveHandler;

    QSignalSpy numberChanged {&saveHandler, &FileSaveHandler::numberChanged};
    QVERIFY(numberChanged.isValid());
    QCOMPARE(numberChanged.count(), 0);

    saveHandler.setNumber(10);
    QCOMPARE(numberChanged.count(), 1);
    QCOMPARE(saveHandler.number(), 10);

    saveHandler.setNumber(10);
    QCOMPARE(numberChanged.count(), 1);
    QCOMPARE(saveHandler.number(), 10);

    saveHandler.setNumber(5);
    QCOMPARE(numberChanged.count(), 2);
    QCOMPARE(saveHandler.number(), 5);
}


void FileSaveHandlerTest::testNumberAlreadyUsed()
{
    FileSaveHandler saveHandler;

    QSignalSpy numberAlreadyUsedChanged {&saveHandler, &FileSaveHandler::numberAlreadyUsedChanged};
    QVERIFY(numberAlreadyUsedChanged.isValid());
    QCOMPARE(numberAlreadyUsedChanged.count(), 0);

    saveHandler.setNumberAlreadyUsed(false);
    QCOMPARE(numberAlreadyUsedChanged.count(), 0);
    QVERIFY(!saveHandler.numberAlreadyUsed());

    saveHandler.setNumberAlreadyUsed(true);
    QCOMPARE(numberAlreadyUsedChanged.count(), 1);
    QVERIFY(saveHandler.numberAlreadyUsed());
}

void FileSaveHandlerTest::testSearchLowerNumber()
{
    FileSaveHandler saveHandler;

    QSignalSpy searchLowerNumberChanged {&saveHandler, &FileSaveHandler::searchLowerNumberChanged};
    QVERIFY(searchLowerNumberChanged.isValid());
    QCOMPARE(searchLowerNumberChanged.count(), 0);

    saveHandler.setSearchLowerNumber(false);
    QCOMPARE(searchLowerNumberChanged.count(), 0);
    QVERIFY(!saveHandler.searchLowerNumber());

    saveHandler.setSearchLowerNumber(true);
    QCOMPARE(searchLowerNumberChanged.count(), 1);
    QVERIFY(saveHandler.searchLowerNumber());
}

void FileSaveHandlerTest::testFileModel()
{
    FileSaveHandler saveHandler;
    FileModel model;

    QSignalSpy fileModelChanged {&saveHandler, &FileSaveHandler::fileModelChanged};
    QVERIFY(fileModelChanged.isValid());
    QCOMPARE(fileModelChanged.count(), 0);

    saveHandler.setFileModel(&model);
    QCOMPARE(fileModelChanged.count(), 1);
    QCOMPARE(saveHandler.fileModel(), &model);

    saveHandler.setFileModel(&model);
    QCOMPARE(fileModelChanged.count(), 1);
    QCOMPARE(saveHandler.fileModel(), &model);

    saveHandler.setFileModel(nullptr);
    QCOMPARE(fileModelChanged.count(), 2);
    QCOMPARE(saveHandler.fileModel(), nullptr);
}

void FileSaveHandlerTest::testSearchAvailableNumber()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    QVERIFY(QDir{tmp.path()}.mkpath(QStringLiteral("config/weld_figure/")));
    QDir dir{tmp.path()};
    QVERIFY(dir.exists());
    dir.mkpath(QStringLiteral("config/weld_figure/"));
    QVERIFY(dir.exists("config/weld_figure/"));
    QVERIFY(dir.cd("config/weld_figure/"));

    auto testWeldingSeam = QFINDTESTDATA(QStringLiteral("testData/weldingSeam1.json"));
    QVERIFY( QFile::copy(testWeldingSeam, tmp.filePath(QStringLiteral("config/weld_figure/weldingSeam1.json"))));
    auto testWeldingSeamV2 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam2.json"));
    QVERIFY( QFile::copy(testWeldingSeamV2, tmp.filePath(QStringLiteral("config/weld_figure/weldingSeam2.json"))));
    auto testWeldingSeamV3 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam3.json"));
    QVERIFY( QFile::copy(testWeldingSeamV3, tmp.filePath(QStringLiteral("config/weld_figure/weldingSeam3.json"))));

    QVERIFY(tmp.isValid());
    QVERIFY(QDir{tmp.path()}.mkpath(QStringLiteral("config/laser_controls/")));
    QDir dir2{tmp.path()};
    QVERIFY(dir2.exists());
    dir2.mkpath(QStringLiteral("config/laser_controls/"));
    QVERIFY(dir2.exists("config/laser_controls/"));
    QVERIFY(dir2.cd("config/laser_controls/"));

    auto testWobbleFile = QFINDTESTDATA(QStringLiteral("testData/figureWobble2.json"));
    QVERIFY( QFile::copy(testWobbleFile, tmp.filePath(QStringLiteral("config/laser_controls/figureWobble2.json"))));

    auto testOverlayFunction = QFINDTESTDATA(QStringLiteral("testData/overlayFunction2.json"));
    QVERIFY( QFile::copy(testOverlayFunction, tmp.filePath(QStringLiteral("config/weld_figure/overlayFunction2.json"))));
    auto testOverlayFunctionV2 = QFINDTESTDATA(QStringLiteral("testData/overlayFunction3.json"));
    QVERIFY( QFile::copy(testOverlayFunctionV2, tmp.filePath(QStringLiteral("config/weld_figure/overlayFunction3.json"))));
    auto testOverlayFunctionV3 = QFINDTESTDATA(QStringLiteral("testData/overlayFunction4.json"));
    QVERIFY( QFile::copy(testOverlayFunctionV3, tmp.filePath(QStringLiteral("config/weld_figure/overlayFunction4.json"))));

    qputenv("WM_BASE_DIR", tmp.path().toLocal8Bit());

    FileSaveHandler saveHandler;
    FileModel model;

    saveHandler.searchAvailableNumber();
    saveHandler.setFileModel(&model);
    saveHandler.searchAvailableNumber();
    model.loadFiles();
    QTRY_VERIFY_WITH_TIMEOUT(!model.loading(), 500);               //wait 5s

    saveHandler.setType(FileType::Seam);
    saveHandler.setNumber(2);

    saveHandler.searchAvailableNumber();
    QCOMPARE(saveHandler.number(), 4);
    saveHandler.setSearchLowerNumber(true);
    saveHandler.searchAvailableNumber();
    QCOMPARE(saveHandler.number(), 0);
    saveHandler.setNumber(2);
    saveHandler.searchAvailableNumber();
    QCOMPARE(saveHandler.number(), 0);

    saveHandler.setType(FileType::Wobble);
    saveHandler.searchAvailableNumber();
    QCOMPARE(saveHandler.number(), 0);
    saveHandler.setSearchLowerNumber(false);
    saveHandler.searchAvailableNumber();
    QCOMPARE(saveHandler.number(), 1);
    saveHandler.searchAvailableNumber();
    QCOMPARE(saveHandler.number(), 3);
    saveHandler.searchAvailableNumber();
    QCOMPARE(saveHandler.number(), 4);

    saveHandler.setType(FileType::Overlay);
    saveHandler.setSearchLowerNumber(true);
    saveHandler.searchAvailableNumber();
    QCOMPARE(saveHandler.number(), 1);
    saveHandler.setSearchLowerNumber(false);
    saveHandler.searchAvailableNumber();
    QCOMPARE(saveHandler.number(), 5);

    saveHandler.setType(FileType::Wobble);
    saveHandler.setNumber(5);
    saveHandler.searchAvailableNumber();
    QCOMPARE(saveHandler.number(), 6);
    saveHandler.setNumber(4);
    saveHandler.setSearchLowerNumber(true);
    saveHandler.searchAvailableNumber();
    QCOMPARE(saveHandler.number(), 3);
    saveHandler.searchAvailableNumber();
    QCOMPARE(saveHandler.number(), 1);
    saveHandler.setSearchLowerNumber(false);
    saveHandler.searchAvailableNumber();
    QCOMPARE(saveHandler.number(), 3);
}

void FileSaveHandlerTest::testCheckIfNumberIsAlreadyUsed()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    QVERIFY(QDir{tmp.path()}.mkpath(QStringLiteral("config/weld_figure/")));
    QDir dir{tmp.path()};
    QVERIFY(dir.exists());
    dir.mkpath(QStringLiteral("config/weld_figure/"));
    QVERIFY(dir.exists("config/weld_figure/"));
    QVERIFY(dir.cd("config/weld_figure/"));

    auto testWeldingSeam = QFINDTESTDATA(QStringLiteral("testData/weldingSeam1.json"));
    QVERIFY( QFile::copy(testWeldingSeam, tmp.filePath(QStringLiteral("config/weld_figure/weldingSeam1.json"))));
    auto testWeldingSeamV2 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam2.json"));
    QVERIFY( QFile::copy(testWeldingSeamV2, tmp.filePath(QStringLiteral("config/weld_figure/weldingSeam2.json"))));
    auto testWeldingSeamV3 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam3.json"));
    QVERIFY( QFile::copy(testWeldingSeamV3, tmp.filePath(QStringLiteral("config/weld_figure/weldingSeam3.json"))));

    QVERIFY(tmp.isValid());
    QVERIFY(QDir{tmp.path()}.mkpath(QStringLiteral("config/laser_controls/")));
    QDir dir2{tmp.path()};
    QVERIFY(dir2.exists());
    dir2.mkpath(QStringLiteral("config/laser_controls/"));
    QVERIFY(dir2.exists("config/laser_controls/"));
    QVERIFY(dir2.cd("config/laser_controls/"));

    auto testWobbleFile = QFINDTESTDATA(QStringLiteral("testData/figureWobble2.json"));
    QVERIFY( QFile::copy(testWobbleFile, tmp.filePath(QStringLiteral("config/laser_controls/figureWobble2.json"))));

    auto testOverlayFunction = QFINDTESTDATA(QStringLiteral("testData/overlayFunction2.json"));
    QVERIFY( QFile::copy(testOverlayFunction, tmp.filePath(QStringLiteral("config/weld_figure/overlayFunction2.json"))));
    auto testOverlayFunctionV2 = QFINDTESTDATA(QStringLiteral("testData/overlayFunction3.json"));
    QVERIFY( QFile::copy(testOverlayFunctionV2, tmp.filePath(QStringLiteral("config/weld_figure/overlayFunction3.json"))));
    auto testOverlayFunctionV3 = QFINDTESTDATA(QStringLiteral("testData/overlayFunction4.json"));
    QVERIFY( QFile::copy(testOverlayFunctionV3, tmp.filePath(QStringLiteral("config/weld_figure/overlayFunction4.json"))));

    qputenv("WM_BASE_DIR", tmp.path().toLocal8Bit());

    FileSaveHandler saveHandler;
    FileModel model;

    QVERIFY(!saveHandler.numberAlreadyUsed());
    saveHandler.checkIfNumberIsAlreadyUsed();
    QVERIFY(!saveHandler.numberAlreadyUsed());
    saveHandler.setFileModel(&model);
    saveHandler.checkIfNumberIsAlreadyUsed();
    QVERIFY(!saveHandler.numberAlreadyUsed());
    model.loadFiles();
    QTRY_VERIFY_WITH_TIMEOUT(!model.loading(), 500);               //wait 5s

    saveHandler.setType(FileType::Seam);
    saveHandler.setNumber(1);
    saveHandler.checkIfNumberIsAlreadyUsed();
    QVERIFY(saveHandler.numberAlreadyUsed());
    saveHandler.setNumber(2);
    saveHandler.checkIfNumberIsAlreadyUsed();
    QVERIFY(saveHandler.numberAlreadyUsed());
    saveHandler.setNumber(3);
    saveHandler.checkIfNumberIsAlreadyUsed();
    QVERIFY(saveHandler.numberAlreadyUsed());
    saveHandler.setNumber(4);
    saveHandler.checkIfNumberIsAlreadyUsed();
    QVERIFY(!saveHandler.numberAlreadyUsed());
    saveHandler.setNumber(10);
    saveHandler.checkIfNumberIsAlreadyUsed();
    QVERIFY(!saveHandler.numberAlreadyUsed());

    saveHandler.setType(FileType::Wobble);
    saveHandler.setNumber(1);
    saveHandler.checkIfNumberIsAlreadyUsed();
    QVERIFY(!saveHandler.numberAlreadyUsed());
    saveHandler.setNumber(2);
    saveHandler.checkIfNumberIsAlreadyUsed();
    QVERIFY(saveHandler.numberAlreadyUsed());
    saveHandler.setNumber(3);
    saveHandler.checkIfNumberIsAlreadyUsed();
    QVERIFY(!saveHandler.numberAlreadyUsed());
    saveHandler.setNumber(4);
    saveHandler.checkIfNumberIsAlreadyUsed();
    QVERIFY(!saveHandler.numberAlreadyUsed());
    saveHandler.setNumber(10);
    saveHandler.checkIfNumberIsAlreadyUsed();
    QVERIFY(!saveHandler.numberAlreadyUsed());

    saveHandler.setType(FileType::Overlay);
    saveHandler.setNumber(1);
    saveHandler.checkIfNumberIsAlreadyUsed();
    QVERIFY(!saveHandler.numberAlreadyUsed());
    saveHandler.setNumber(2);
    saveHandler.checkIfNumberIsAlreadyUsed();
    QVERIFY(saveHandler.numberAlreadyUsed());
    saveHandler.setNumber(3);
    saveHandler.checkIfNumberIsAlreadyUsed();
    QVERIFY(saveHandler.numberAlreadyUsed());
    saveHandler.setNumber(4);
    saveHandler.checkIfNumberIsAlreadyUsed();
    QVERIFY(saveHandler.numberAlreadyUsed());
    saveHandler.setNumber(10);
    saveHandler.checkIfNumberIsAlreadyUsed();
    QVERIFY(!saveHandler.numberAlreadyUsed());

    saveHandler.setType(FileType::Basic);
    saveHandler.setNumber(1);
    saveHandler.checkIfNumberIsAlreadyUsed();
    QVERIFY(!saveHandler.numberAlreadyUsed());
    saveHandler.setNumber(2);
    saveHandler.checkIfNumberIsAlreadyUsed();
    QVERIFY(saveHandler.numberAlreadyUsed());
    saveHandler.setNumber(3);
    saveHandler.checkIfNumberIsAlreadyUsed();
    QVERIFY(!saveHandler.numberAlreadyUsed());
    saveHandler.setNumber(4);
    saveHandler.checkIfNumberIsAlreadyUsed();
    QVERIFY(!saveHandler.numberAlreadyUsed());
    saveHandler.setNumber(10);
    saveHandler.checkIfNumberIsAlreadyUsed();
    QVERIFY(!saveHandler.numberAlreadyUsed());
}

void FileSaveHandlerTest::testSetTypeAndPrefix()
{
    FileSaveHandler saveHandler;

    QSignalSpy filePrefixChanged {&saveHandler, &FileSaveHandler::filePrefixChanged};
    QVERIFY(filePrefixChanged.isValid());
    QCOMPARE(filePrefixChanged.count(), 0);

    QSignalSpy fileTypeLabelChanged {&saveHandler, &FileSaveHandler::fileTypeLabelChanged};
    QVERIFY(fileTypeLabelChanged.isValid());
    QCOMPARE(fileTypeLabelChanged.count(), 0);

    saveHandler.setTypeAndPrefix();
    QCOMPARE(filePrefixChanged.count(), 0);
    QCOMPARE(fileTypeLabelChanged.count(), 0);
    QCOMPARE(saveHandler.filePrefix(), "");
    QCOMPARE(saveHandler.fileTypeLabel(), "");

    saveHandler.setType(FileType::Seam);
    saveHandler.setTypeAndPrefix();
    QCOMPARE(filePrefixChanged.count(), 1);
    QCOMPARE(fileTypeLabelChanged.count(), 1);
    QCOMPARE(saveHandler.filePrefix(), "weldingSeam");
    QCOMPARE(saveHandler.fileTypeLabel(), "Seam figure");

    saveHandler.setType(FileType::Wobble);
    saveHandler.setTypeAndPrefix();
    QCOMPARE(filePrefixChanged.count(), 2);
    QCOMPARE(fileTypeLabelChanged.count(), 2);
    QCOMPARE(saveHandler.filePrefix(), "figureWobble");
    QCOMPARE(saveHandler.fileTypeLabel(), "Wobble figure");

    saveHandler.setType(FileType::Overlay);
    saveHandler.setTypeAndPrefix();
    QCOMPARE(filePrefixChanged.count(), 3);
    QCOMPARE(fileTypeLabelChanged.count(), 3);
    QCOMPARE(saveHandler.filePrefix(), "overlayFunction");
    QCOMPARE(saveHandler.fileTypeLabel(), "Overlay figure");

    saveHandler.setType(FileType::Basic);
    saveHandler.setTypeAndPrefix();
    QCOMPARE(filePrefixChanged.count(), 4);
    QCOMPARE(fileTypeLabelChanged.count(), 4);
    QCOMPARE(saveHandler.filePrefix(), "figureWobble");
    QCOMPARE(saveHandler.fileTypeLabel(), "Wobble figure");

    saveHandler.setType(FileType::Basic);
    saveHandler.setTypeAndPrefix();
    QCOMPARE(filePrefixChanged.count(), 5);
    QCOMPARE(fileTypeLabelChanged.count(), 5);
    QCOMPARE(saveHandler.filePrefix(), "figureWobble");
    QCOMPARE(saveHandler.fileTypeLabel(), "Wobble figure");
}

#include<QDebug>
void FileSaveHandlerTest::testGetNumberFromFileName()
{
    FileSaveHandler saveHandler;

    saveHandler.setType(FileType::Seam);
    saveHandler.setTypeAndPrefix();
    QCOMPARE(saveHandler.getNumberFromFileName(QStringLiteral("weldingSeam1.json")), 1);
    QCOMPARE(saveHandler.getNumberFromFileName(QStringLiteral("weldingSeam502.json")), 502);

    QCOMPARE(saveHandler.getNumberFromFileName(QStringLiteral("figureWobble5.json")), 0);

    saveHandler.setType(FileType::Wobble);
    saveHandler.setTypeAndPrefix();

    QCOMPARE(saveHandler.getNumberFromFileName(QStringLiteral("figureWobble5.json")), 5);
    QCOMPARE(saveHandler.getNumberFromFileName(QStringLiteral("figureWobble0.json")), 0);

    saveHandler.setType(FileType::Overlay);
    saveHandler.setTypeAndPrefix();

    QCOMPARE(saveHandler.getNumberFromFileName(QStringLiteral("overlayFunction4.json")), 4);
    QCOMPARE(saveHandler.getNumberFromFileName(QStringLiteral("overlayFunction400.json")), 400);

    saveHandler.setType(FileType::Basic);
    saveHandler.setTypeAndPrefix();

    QCOMPARE(saveHandler.getNumberFromFileName(QStringLiteral("figureWobble51.json")), 51);
    QCOMPARE(saveHandler.getNumberFromFileName(QStringLiteral("figureWobble1340.json")), 1340);
}

QTEST_GUILESS_MAIN(FileSaveHandlerTest)
#include "fileSaveHandlerTest.moc"
