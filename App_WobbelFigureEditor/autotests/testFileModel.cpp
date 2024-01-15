#include <QTest>
#include <QSignalSpy>

#include "../src/FileModel.h"

using precitec::scantracker::components::wobbleFigureEditor::FileModel;
using precitec::scantracker::components::wobbleFigureEditor::FileType;

class FileModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testRoleNames();
    void testLoadingFiles_data();
    void testLoadingFiles();
    void testAddFile();
    void testFileType();
    void testLoadSeamFigure_data();
    void testLoadSeamFigure();
    void testLoadWobbleFigure_data();
    void testLoadWobbleFigure();
    void testLoadOverlayFunction_data();
    void testLoadOverlayFunction();
    void testLoadBasicFigure_data();
    void testLoadBasicFigure();
    void testSaveSeamFigure_data();
    void testSaveSeamFigure();
    void testSaveWobbleFigure_data();
    void testSaveWobbleFigure();
    void testSaveOverlayFigure_data();
    void testSaveOverlayFigure();
    void testSaveBasicFigure_data();
    void testSaveBasicFigure();

private:
    QTemporaryDir m_dir;
};

void FileModelTest::initTestCase()
{
    QVERIFY(m_dir.isValid());
    QVERIFY(QDir{}.mkpath(m_dir.filePath(QStringLiteral("config"))));
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());
    qRegisterMetaType<precitec::scantracker::components::wobbleFigureEditor::FileType>();
}

void FileModelTest::testCtor()
{
    FileModel fileModel;

    QCOMPARE(fileModel.files().size(), 0);
    QVERIFY(!fileModel.loading());
    QCOMPARE(fileModel.fileType(), FileType::None);
    QCOMPARE(fileModel.filename(), QStringLiteral(""));
}

void FileModelTest::testRoleNames()
{
    FileModel fileModel;
    const auto roleNames = fileModel.roleNames();
    QCOMPARE(roleNames.size(), 7);
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("name"));
    QCOMPARE(roleNames[Qt::UserRole], QByteArrayLiteral("path"));
    QCOMPARE(roleNames[Qt::UserRole + 1], QByteArrayLiteral("type"));
    QCOMPARE(roleNames[Qt::UserRole + 2], QByteArrayLiteral("typeName"));
    QCOMPARE(roleNames[Qt::UserRole + 3], QByteArrayLiteral("id"));
    QCOMPARE(roleNames[Qt::UserRole + 4], QByteArrayLiteral("fileName"));
    QCOMPARE(roleNames[Qt::UserRole + 5], QByteArrayLiteral("visibleName"));
}

void FileModelTest::testLoadingFiles_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QString>("name");
    QTest::addColumn<FileType>("type");
    QTest::addColumn<QString>("typeName");
    QTest::addColumn<QString>("id");
    QTest::addColumn<QString>("fileName");

    QTest::newRow("Overlayfile2") << 0 << QStringLiteral("Testfile A (2)") << FileType::Overlay << QStringLiteral("Overlay function") << QStringLiteral("2") << QStringLiteral("overlayFunction2.json");
    QTest::newRow("Overlayfile3") << 1 << QStringLiteral("Circle (2)") << FileType::Overlay << QStringLiteral("Overlay function") << QStringLiteral("2") << QStringLiteral("overlayFunction3.json");
    QTest::newRow("Overlayfile4") << 2 << QStringLiteral("Rectangle (1)") << FileType::Overlay << QStringLiteral("Overlay function") << QStringLiteral("1") << QStringLiteral("overlayFunction4.json");
    QTest::newRow("Seamfile1") << 3 << QStringLiteral("Test (1)") << FileType::Seam << QStringLiteral("Seam figure") << QStringLiteral("1") << QStringLiteral("weldingSeam1.json");
    QTest::newRow("Seamfile2") << 4 << QStringLiteral("Second test (2)") << FileType::Seam << QStringLiteral("Seam figure") << QStringLiteral("2") << QStringLiteral("weldingSeam2.json");
    QTest::newRow("Seamfile3") << 5 << QStringLiteral("Test (3)") << FileType::Seam << QStringLiteral("Seam figure") << QStringLiteral("3") << QStringLiteral("weldingSeam3.json");
    QTest::newRow("Wobblefile2") << 6 << QStringLiteral("Test (2)") << FileType::Wobble << QStringLiteral("Wobble figure") << QStringLiteral("2") << QStringLiteral("figureWobble2.json");
}

void FileModelTest::testLoadingFiles()
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
    auto testWeldingSeamV3 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam3.json"));
    QVERIFY( QFile::copy(testWeldingSeamV3, tmp.filePath(QStringLiteral("config/weld_figure/weldingSeam3.json"))));

    auto testOverlayFunctionV1 = QFINDTESTDATA(QStringLiteral("testData/overlayFunction1.json"));
    QVERIFY( QFile::copy(testOverlayFunctionV1, tmp.filePath(QStringLiteral("config/weld_figure/overlayFunction1.json"))));
    auto testOverlayFunctionV2 = QFINDTESTDATA(QStringLiteral("testData/overlayFunction2.json"));
    QVERIFY( QFile::copy(testOverlayFunctionV2, tmp.filePath(QStringLiteral("config/weld_figure/overlayFunction2.json"))));
    auto testOverlayFunctionV3 = QFINDTESTDATA(QStringLiteral("testData/overlayFunction3.json"));
    QVERIFY( QFile::copy(testOverlayFunctionV3, tmp.filePath(QStringLiteral("config/weld_figure/overlayFunction3.json"))));
    auto testOverlayFunctionV4 = QFINDTESTDATA(QStringLiteral("testData/overlayFunction4.json"));
    QVERIFY( QFile::copy(testOverlayFunctionV4, tmp.filePath(QStringLiteral("config/weld_figure/overlayFunction4.json"))));

    QDir wobbleDir{tmp.path()};
    QVERIFY(wobbleDir.exists());
    wobbleDir.mkpath(QStringLiteral("config/laser_controls/"));
    QVERIFY(wobbleDir.exists("config/laser_controls/"));
    QVERIFY(wobbleDir.cd("config/laser_controls/"));

    auto testWobbleFileV1 = QFINDTESTDATA(QStringLiteral("testData/figureWobble1.json"));
    QVERIFY( QFile::copy(testWobbleFileV1, tmp.filePath(QStringLiteral("config/laser_controls/figureWobble1.json"))));
    auto testWobbleFileV2 = QFINDTESTDATA(QStringLiteral("testData/figureWobble2.json"));
    QVERIFY( QFile::copy(testWobbleFileV2, tmp.filePath(QStringLiteral("config/laser_controls/figureWobble2.json"))));
    auto testWobbleFileV3 = QFINDTESTDATA(QStringLiteral("testData/figureWobble3.json"));
    QVERIFY( QFile::copy(testWobbleFileV3, tmp.filePath(QStringLiteral("config/laser_controls/figureWobble3.json"))));

    qputenv("WM_BASE_DIR", tmp.path().toLocal8Bit());

    FileModel fileModel;

    QSignalSpy modelResetSpy{&fileModel, &FileModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    QCOMPARE(modelResetSpy.count(), 0);
    QSignalSpy loadingChangedSpy{&fileModel, &FileModel::loadingChanged};
    QVERIFY(loadingChangedSpy.isValid());
    QCOMPARE(loadingChangedSpy.count(), 0);

    fileModel.loadFiles();
    QCOMPARE(loadingChangedSpy.count(), 1);
    QTRY_COMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(modelResetSpy.count(), 1);

    const auto &files = fileModel.files();
    QVERIFY(!files.empty());
    QCOMPARE(files.size(), 7);

    QFETCH(int, row);
    QTEST(fileModel.data(fileModel.index(row, 0), Qt::DisplayRole).toString(), "name");
    QTEST(static_cast<FileType>(fileModel.data(fileModel.index(row, 0), Qt::UserRole + 1).toInt()), "type");
    QTEST(fileModel.data(fileModel.index(row, 0), Qt::UserRole + 2).toString(), "typeName");
    QTEST(fileModel.data(fileModel.index(row, 0), Qt::UserRole + 4).toString(), "fileName");
    if (static_cast<FileType>(fileModel.data(fileModel.index(row, 0), Qt::UserRole + 1).toInt()) == FileType::Wobble)
    {
        QCOMPARE(fileModel.data(fileModel.index(row, 0), Qt::UserRole).toString(), fileModel.m_wobbleFilePath + "/" + fileModel.data(fileModel.index(row, 0), Qt::UserRole + 4).toString());
    }
    else
    {
        QCOMPARE(fileModel.data(fileModel.index(row, 0), Qt::UserRole).toString(), fileModel.m_seamAndOverlayFilePath + "/" + fileModel.data(fileModel.index(row, 0), Qt::UserRole + 4).toString());
    }
    QTEST(fileModel.data(fileModel.index(row, 0), Qt::UserRole + 3).toString(), "id");
}

void FileModelTest::testAddFile()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    QVERIFY(QDir{tmp.path()}.mkpath(QStringLiteral("config/weld_figure/")));
    QDir dir{tmp.path()};
    QVERIFY(dir.exists());
    dir.mkpath(QStringLiteral("config/weld_figure/"));
    QVERIFY(dir.exists("config/weld_figure/"));
    QVERIFY(dir.cd("config/weld_figure/"));

    qputenv("WM_BASE_DIR", tmp.path().toLocal8Bit());

    FileModel fileModel;

    QSignalSpy modelResetSpy{&fileModel, &FileModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    QCOMPARE(modelResetSpy.count(), 0);
    QSignalSpy loadingChangedSpy{&fileModel, &FileModel::loadingChanged};
    QVERIFY(loadingChangedSpy.isValid());
    QCOMPARE(loadingChangedSpy.count(), 0);

    fileModel.loadFiles();
    QCOMPARE(loadingChangedSpy.count(), 1);
    QTRY_COMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(modelResetSpy.count(), 1);

    auto files = fileModel.files();
    QVERIFY(files.empty());
    QVERIFY(!fileModel.indexForSeamFigure(1).isValid());
    QVERIFY(!fileModel.indexForWobbleFigure(1).isValid());

    auto testWeldingSeamV1 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam1.json"));
    QVERIFY( QFile::copy(testWeldingSeamV1, tmp.filePath(QStringLiteral("config/weld_figure/weldingSeam1.json"))));
    QVERIFY(loadingChangedSpy.wait());
    QTRY_COMPARE(modelResetSpy.count(), 2);

    QVERIFY(fileModel.indexForSeamFigure(1).isValid());
    QVERIFY(!fileModel.indexForWobbleFigure(1).isValid());

    files = fileModel.files();
    QVERIFY(!files.empty());
}

void FileModelTest::testFileType()
{
    FileModel fileModel;

    QCOMPARE(fileModel.fileType(), FileType::None);

    QSignalSpy fileTypeChanged{&fileModel, &FileModel::fileTypeChanged};
    QVERIFY(fileTypeChanged.isValid());
    QCOMPARE(fileTypeChanged.count(), 0);

    fileModel.setFileType(FileType::Seam);
    QCOMPARE(fileTypeChanged.count(), 1);
    QCOMPARE(fileModel.fileType(), FileType::Seam);

    fileModel.setFileType(FileType::Wobble);
    QCOMPARE(fileTypeChanged.count(), 2);
    QCOMPARE(fileModel.fileType(), FileType::Wobble);

    fileModel.setFileType(FileType::Overlay);
    QCOMPARE(fileTypeChanged.count(), 3);
    QCOMPARE(fileModel.fileType(), FileType::Overlay);

    fileModel.setFileType(FileType::Basic);
    QCOMPARE(fileTypeChanged.count(), 4);
    QCOMPARE(fileModel.fileType(), FileType::Basic);
}

Q_DECLARE_METATYPE(RTC6::seamFigure::command::Order)
void FileModelTest::testLoadSeamFigure_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<std::vector<RTC6::seamFigure::command::Order>>("figure");
    QTest::addColumn<std::vector<RTC6::seamFigure::command::Order>>("figure2");
    QTest::addColumn<std::vector<RTC6::seamFigure::command::Order>>("figure3");
    QTest::addColumn<std::vector<RTC6::seamFigure::command::Order>>("figure4");

    std::vector<RTC6::seamFigure::command::Order> figure1;
    std::vector<RTC6::seamFigure::command::Order> figure2;
    std::vector<RTC6::seamFigure::command::Order> figure3;
    std::vector<RTC6::seamFigure::command::Order> figure4;
    RTC6::seamFigure::command::Order order;

    order.endPosition = std::make_pair(0.0, 0.0);
    order.power = 25.0;
    order.ringPower = -1.0;
    order.velocity = 100.0;
    figure1.push_back(order);
    order.endPosition = std::make_pair(-2.5, 0.0);
    order.power = 30.0;
    order.velocity = 200.0;
    figure1.push_back(order);
    order.endPosition = std::make_pair(0.0, 0.0);
    order.power = 55.0;
    order.velocity = 130.0;
    figure1.push_back(order);
    order.endPosition = std::make_pair(2.5, 0);
    order.power = 2.0;
    order.velocity = 150.5;
    figure1.push_back(order);
    order.endPosition = std::make_pair(0.0, 0.0);
    order.power = -1.0;
    order.velocity = -1.0;
    figure1.push_back(order);
    order.endPosition = std::make_pair(-2.0, 0.0);
    order.power = 25.2;
    order.ringPower = -1.0;
    order.velocity = -1.0;
    figure2.push_back(order);
    order.endPosition = std::make_pair(-2.500, 1);
    order.power = 32.0;
    figure2.push_back(order);
    order.endPosition = std::make_pair(0.0, 0.0);
    order.power = 55.2;
    figure2.push_back(order);
    order.endPosition = std::make_pair(2.5, 0.0);
    order.power = 2.2;
    figure2.push_back(order);
    order.endPosition = std::make_pair(0.0, 0.0);
    order.power = 3.2;
    figure2.push_back(order);
    order.endPosition = std::make_pair(0.0, 0.0);
    order.power = 25.0;
    order.ringPower = 12.5;
    order.velocity = 100.0;
    figure3.push_back(order);
    order.endPosition = std::make_pair(-2.5, 0.0);
    order.power = 30.0;
    order.ringPower = 15.0;
    order.velocity = 200.0;
    figure3.push_back(order);
    order.endPosition = std::make_pair(0.0, 0.0);
    order.power = 55.0;
    order.ringPower = 27.5;
    order.velocity = 130.0;
    figure3.push_back(order);
    order.endPosition = std::make_pair(2.5, 0.0);
    order.power = 2.0;
    order.ringPower = 1.0;
    order.velocity = 150.5;
    figure3.push_back(order);
    order.endPosition = std::make_pair(0.0, 0.0);
    order.power = 3.0;
    order.ringPower = 60.0;
    order.velocity = 250.0;
    figure3.push_back(order);
    figure4 = figure3;

    QTest::newRow("SeamFiles") << 1 << figure1 << figure2 << figure3 << figure4;
}

void FileModelTest::testLoadSeamFigure()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    QVERIFY(QDir{tmp.path()}.mkpath(QStringLiteral("config/weld_figure/")));
    QDir dir{tmp.path()};
    QVERIFY(dir.exists());
    dir.mkpath(QStringLiteral("config/weld_figure/"));
    QVERIFY(dir.exists("config/weld_figure/"));
    QVERIFY(dir.cd("config/weld_figure/"));

    auto testWeldingSeamV0 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam0.json"));
    QVERIFY( QFile::copy(testWeldingSeamV0, tmp.filePath(QStringLiteral("config/weld_figure/weldingSeam0.json"))));
    auto testWeldingSeamV1 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam1.json"));
    QVERIFY( QFile::copy(testWeldingSeamV1, tmp.filePath(QStringLiteral("config/weld_figure/weldingSeam1.json"))));
    auto testWeldingSeamV2 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam2.json"));
    QVERIFY( QFile::copy(testWeldingSeamV2, tmp.filePath(QStringLiteral("config/weld_figure/weldingSeam2.json"))));
    auto testWeldingSeamV3 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam3.json"));
    QVERIFY( QFile::copy(testWeldingSeamV3, tmp.filePath(QStringLiteral("config/weld_figure/weldingSeam3.json"))));
    auto testWeldingSeamV4 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam4.json"));
    QVERIFY( QFile::copy(testWeldingSeamV4, tmp.filePath(QStringLiteral("config/weld_figure/weldingSeam4.json"))));
    auto testWeldingSeamV5 = QFINDTESTDATA(QStringLiteral("testData/weldingSeam5.json"));
    QVERIFY( QFile::copy(testWeldingSeamV5, tmp.filePath(QStringLiteral("config/weld_figure/weldingSeam5.json"))));

    qputenv("WM_BASE_DIR", tmp.path().toLocal8Bit());

    FileModel fileModel;

    QSignalSpy figureLoaded{&fileModel, &FileModel::figureLoaded};
    QVERIFY(figureLoaded.isValid());
    QCOMPARE(figureLoaded.count(), 0);

    QSignalSpy filenameChanged{&fileModel, &FileModel::filenameChanged};
    QVERIFY(filenameChanged.isValid());
    QCOMPARE(filenameChanged.count(), 0);

    QSignalSpy loadingChangedSpy{&fileModel, &FileModel::loadingChanged};
    QVERIFY(loadingChangedSpy.isValid());
    QCOMPARE(loadingChangedSpy.count(), 0);

    fileModel.loadFiles();
    QCOMPARE(loadingChangedSpy.count(), 1);
    QTRY_COMPARE(loadingChangedSpy.count(), 2);
    const auto &files = fileModel.files();
    QVERIFY(!files.empty());
    QCOMPARE(files.size(), 4);

    fileModel.setFileType(FileType::Seam);
    QCOMPARE(fileModel.loadJsonFromFile(QStringLiteral("")), FileModel::LoadingFeedback::NameOrTypeError);
    QVERIFY(fileModel.filename().isEmpty());
    QCOMPARE(filenameChanged.count(), 0);
    QCOMPARE(figureLoaded.count(), 0);

    QCOMPARE(fileModel.loadJsonFromFile(QStringLiteral("abc0.json")), FileModel::LoadingFeedback::FileNotFound);
    QVERIFY(fileModel.filename().isEmpty());
    QCOMPARE(filenameChanged.count(), 0);
    QCOMPARE(figureLoaded.count(), 0);

    QCOMPARE(fileModel.loadJsonFromFile(QStringLiteral("weldingSeam0.json")), FileModel::LoadingFeedback::FileNotFound);
    QCOMPARE(std::get<0>(fileModel.loadJsonFromFile(QFileInfo{tmp.filePath(QStringLiteral("config/weld_figure/weldingSeam0.json"))}, FileType::Seam)), FileModel::LoadingFeedback::EmptyFile);
    QVERIFY(fileModel.filename().isEmpty());
    QCOMPARE(filenameChanged.count(), 0);
    QCOMPARE(figureLoaded.count(), 0);

    QCOMPARE(fileModel.loadJsonFromFile(QStringLiteral("weldingSeam4.json")), FileModel::LoadingFeedback::FileNotFound);
    QCOMPARE(std::get<0>(fileModel.loadJsonFromFile(QFileInfo{tmp.filePath(QStringLiteral("config/weld_figure/weldingSeam4.json"))}, FileType::Seam)), FileModel::LoadingFeedback::CorruptedFile);
    QVERIFY(fileModel.filename().isEmpty());
    QCOMPARE(filenameChanged.count(), 0);
    QCOMPARE(figureLoaded.count(), 0);

    QCOMPARE(fileModel.loadJsonFromFile(QStringLiteral("weldingSeam1.json")), FileModel::LoadingFeedback::NoErrors);
    QVERIFY(!fileModel.filename().isEmpty());
    QCOMPARE(fileModel.filename(), QStringLiteral("weldingSeam1.json"));
    QCOMPARE(filenameChanged.count(), 2);
    QCOMPARE(figureLoaded.count(), 1);
    QCOMPARE(static_cast<FileType>(figureLoaded.takeFirst().at(0).toInt()), FileType::Seam);
    QCOMPARE(fileModel.indexForSeamFigure(1).data(Qt::UserRole + 4).toString(), QStringLiteral("weldingSeam1.json"));

    auto seamFigure = fileModel.seamFigure();
    QCOMPARE(QString::fromStdString(seamFigure.name), QStringLiteral("Test"));
    QCOMPARE(QString::fromStdString(seamFigure.ID), QStringLiteral("1"));
    QCOMPARE(QString::fromStdString(seamFigure.description), QStringLiteral("Test file without ring power"));
    auto currentfigure = seamFigure.figure;
    QFETCH(std::vector<RTC6::seamFigure::command::Order>, figure);
    QCOMPARE(currentfigure.size(), figure.size());

    for (std::size_t i = 0; i < currentfigure.size(); i++)
    {
        const auto &currentOrder = currentfigure.at(i);
        const auto &order = figure.at(i);
        QCOMPARE(currentOrder.endPosition, order.endPosition);
        QCOMPARE(currentOrder.power, order.power);
        QCOMPARE(currentOrder.ringPower, order.ringPower);
        QCOMPARE(currentOrder.velocity, order.velocity);
    }

    QCOMPARE(fileModel.loadJsonFromFile(QStringLiteral("weldingSeam2.json")), FileModel::LoadingFeedback::NoErrors);
    QVERIFY(!fileModel.filename().isEmpty());
    QCOMPARE(fileModel.filename(), QStringLiteral("weldingSeam2.json"));
    QCOMPARE(filenameChanged.count(), 4);
    QCOMPARE(figureLoaded.count(), 1);
    QCOMPARE(static_cast<FileType>(figureLoaded.takeLast().at(0).toInt()), FileType::Seam);
    QCOMPARE(fileModel.indexForSeamFigure(2).data(Qt::UserRole + 4).toString(), QStringLiteral("weldingSeam2.json"));

    seamFigure = fileModel.seamFigure();
    QCOMPARE(QString::fromStdString(seamFigure.name), QStringLiteral("Second test"));
    QCOMPARE(QString::fromStdString(seamFigure.ID), QStringLiteral("2"));
    QCOMPARE(QString::fromStdString(seamFigure.description), QStringLiteral("Test file without velocity and ring power"));
    currentfigure = seamFigure.figure;
    QFETCH(std::vector<RTC6::seamFigure::command::Order>, figure2);
    QCOMPARE(currentfigure.size(), figure2.size());

    for (std::size_t i = 0; i < currentfigure.size(); i++)
    {
        const auto &currentOrder = currentfigure.at(i);
        const auto &order = figure2.at(i);
        QCOMPARE(currentOrder.endPosition, order.endPosition);
        QCOMPARE(currentOrder.power, order.power);
        QCOMPARE(currentOrder.ringPower, order.ringPower);
        QCOMPARE(currentOrder.velocity, order.velocity);
    }

    QCOMPARE(fileModel.loadJsonFromFile(QStringLiteral("weldingSeam3.json")), FileModel::LoadingFeedback::NoErrors);
    QVERIFY(!fileModel.filename().isEmpty());
    QCOMPARE(fileModel.filename(), QStringLiteral("weldingSeam3.json"));
    QCOMPARE(filenameChanged.count(), 6);
    QCOMPARE(figureLoaded.count(), 1);
    QCOMPARE(static_cast<FileType>(figureLoaded.takeLast().at(0).toInt()), FileType::Seam);
    QCOMPARE(fileModel.indexForSeamFigure(3).data(Qt::UserRole + 4).toString(), QStringLiteral("weldingSeam3.json"));

    seamFigure = fileModel.seamFigure();
    QCOMPARE(QString::fromStdString(seamFigure.name), QStringLiteral("Test"));
    QCOMPARE(QString::fromStdString(seamFigure.ID), QStringLiteral("3"));
    QCOMPARE(QString::fromStdString(seamFigure.description), QStringLiteral("Test file with all information"));
    currentfigure = seamFigure.figure;
    QFETCH(std::vector<RTC6::seamFigure::command::Order>, figure3);
    QCOMPARE(currentfigure.size(), figure3.size());

    for (std::size_t i = 0; i < currentfigure.size(); i++)
    {
        const auto &currentOrder = currentfigure.at(i);
        const auto &order = figure3.at(i);
        QCOMPARE(currentOrder.endPosition, order.endPosition);
        QCOMPARE(currentOrder.power, order.power);
        QCOMPARE(currentOrder.ringPower, order.ringPower);
        QCOMPARE(currentOrder.velocity, order.velocity);
    }

    QCOMPARE(fileModel.loadJsonFromFile(QStringLiteral("weldingSeam5.json")), FileModel::LoadingFeedback::NoErrors);
    QVERIFY(!fileModel.filename().isEmpty());
    QCOMPARE(fileModel.filename(), QStringLiteral("weldingSeam5.json"));
    QCOMPARE(filenameChanged.count(), 8);
    QCOMPARE(figureLoaded.count(), 1);
    QCOMPARE(static_cast<FileType>(figureLoaded.takeLast().at(0).toInt()), FileType::Seam);
    QCOMPARE(fileModel.indexForSeamFigure(3).data(Qt::UserRole + 4).toString(), QStringLiteral("weldingSeam3.json"));

    seamFigure = fileModel.seamFigure();
    QCOMPARE(QString::fromStdString(seamFigure.name), QStringLiteral("Test"));
    QCOMPARE(QString::fromStdString(seamFigure.ID), QStringLiteral("5"));
    QCOMPARE(QString::fromStdString(seamFigure.description), QStringLiteral("Test file with all information"));
    currentfigure = seamFigure.figure;
    QFETCH(std::vector<RTC6::seamFigure::command::Order>, figure4);
    QCOMPARE(currentfigure.size(), figure4.size());

    for (std::size_t i = 0; i < currentfigure.size(); i++)
    {
        const auto &currentOrder = currentfigure.at(i);
        const auto &order = figure4.at(i);
        QCOMPARE(currentOrder.endPosition, order.endPosition);
        QCOMPARE(currentOrder.power, order.power);
        QCOMPARE(currentOrder.ringPower, order.ringPower);
        QCOMPARE(currentOrder.velocity, order.velocity);
    }

    RTC6::seamFigure::Ramp ramp;
    ramp.startPointID = 0;
    ramp.length = 0.05;
    ramp.startPower = 1.0;
    ramp.endPower = 0.0;
    ramp.startPowerRing = 0.0;
    ramp.endPowerRing = 1.0;
    QCOMPARE(seamFigure.ramps.front(), ramp);

    seamFigure = fileModel.loadSeamFigure("");
    QVERIFY(seamFigure.figure.empty());

    seamFigure = fileModel.loadSeamFigure("weldingSeam3.json");
    currentfigure = seamFigure.figure;
    QCOMPARE(currentfigure.size(), figure3.size());

    for (std::size_t i = 0; i < currentfigure.size(); i++)
    {
        const auto &currentOrder = currentfigure.at(i);
        const auto &order = figure3.at(i);
        QCOMPARE(currentOrder.endPosition, order.endPosition);
        QCOMPARE(currentOrder.power, order.power);
        QCOMPARE(currentOrder.ringPower, order.ringPower);
        QCOMPARE(currentOrder.velocity, order.velocity);
    }
}

Q_DECLARE_METATYPE(RTC6::wobbleFigure::command::Order)
void FileModelTest::testLoadWobbleFigure_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<std::vector<RTC6::wobbleFigure::command::Order>>("figure");

    std::vector<RTC6::wobbleFigure::command::Order> figure1;
    RTC6::wobbleFigure::command::Order order;

    order.endPosition = std::make_pair(0.0, 0.0);
    order.power = 0.0;
    figure1.push_back(order);
    order.endPosition = std::make_pair(3.0, 0.0);
    order.power = 0.15;
    figure1.push_back(order);
    order.endPosition = std::make_pair(-3.0, 0.0);
    order.power = 0.5;
    figure1.push_back(order);
    order.endPosition = std::make_pair(0.0, 0.0);
    order.power = -0.65;
    figure1.push_back(order);

    QTest::newRow("WobbleFile") << 1 << figure1;
}

void FileModelTest::testLoadWobbleFigure()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    QVERIFY(QDir{tmp.path()}.mkpath(QStringLiteral("config/laser_controls/")));
    QDir dir{tmp.path()};
    QVERIFY(dir.exists());
    dir.mkpath(QStringLiteral("config/laser_controls/"));
    QVERIFY(dir.exists("config/laser_controls/"));
    QVERIFY(dir.cd("config/laser_controls/"));

    auto testWobbleFile = QFINDTESTDATA(QStringLiteral("testData/figureWobble0.json"));
    QVERIFY( QFile::copy(testWobbleFile, tmp.filePath(QStringLiteral("config/laser_controls/figureWobble0.json"))));
    auto testWobbleFileV1 = QFINDTESTDATA(QStringLiteral("testData/figureWobble1.json"));
    QVERIFY( QFile::copy(testWobbleFileV1, tmp.filePath(QStringLiteral("config/laser_controls/figureWobble1.json"))));
    auto testWobbleFileV2 = QFINDTESTDATA(QStringLiteral("testData/figureWobble2.json"));
    QVERIFY( QFile::copy(testWobbleFileV2, tmp.filePath(QStringLiteral("config/laser_controls/figureWobble2.json"))));
    auto testWobbleFileV3 = QFINDTESTDATA(QStringLiteral("testData/figureWobble3.json"));
    QVERIFY( QFile::copy(testWobbleFileV3, tmp.filePath(QStringLiteral("config/laser_controls/figureWobble3.json"))));
    auto testWobbleFileV4 = QFINDTESTDATA(QStringLiteral("testData/figureWobble4.json"));
    QVERIFY( QFile::copy(testWobbleFileV4, tmp.filePath(QStringLiteral("config/laser_controls/figureWobble4.json"))));

    qputenv("WM_BASE_DIR", tmp.path().toLocal8Bit());

    FileModel fileModel;

    QSignalSpy figureLoaded{&fileModel, &FileModel::figureLoaded};
    QVERIFY(figureLoaded.isValid());
    QCOMPARE(figureLoaded.count(), 0);

    QSignalSpy filenameChanged{&fileModel, &FileModel::filenameChanged};
    QVERIFY(filenameChanged.isValid());
    QCOMPARE(filenameChanged.count(), 0);

    QCOMPARE(fileModel.loadJsonFromFile(QStringLiteral("")), FileModel::LoadingFeedback::NameOrTypeError);
    QVERIFY(fileModel.filename().isEmpty());
    QCOMPARE(filenameChanged.count(), 0);
    QCOMPARE(figureLoaded.count(), 0);

    QSignalSpy loadingChangedSpy{&fileModel, &FileModel::loadingChanged};
    QVERIFY(loadingChangedSpy.isValid());
    QCOMPARE(loadingChangedSpy.count(), 0);

    fileModel.loadFiles();
    QCOMPARE(loadingChangedSpy.count(), 1);
    QTRY_COMPARE(loadingChangedSpy.count(), 2);

    const auto &files = fileModel.files();
    QVERIFY(!files.empty());
    QCOMPARE(files.size(), 1);

    fileModel.setFileType(FileType::Wobble);
    QCOMPARE(fileModel.loadJsonFromFile(QStringLiteral("")), FileModel::LoadingFeedback::NameOrTypeError);
    QVERIFY(fileModel.filename().isEmpty());
    QCOMPARE(filenameChanged.count(), 0);
    QCOMPARE(figureLoaded.count(), 0);

    QCOMPARE(fileModel.loadJsonFromFile(QStringLiteral("abc0.json")), FileModel::LoadingFeedback::FileNotFound);
    QVERIFY(fileModel.filename().isEmpty());
    QCOMPARE(filenameChanged.count(), 0);
    QCOMPARE(figureLoaded.count(), 0);

    QCOMPARE(fileModel.loadJsonFromFile(QStringLiteral("figureWobble0.json")), FileModel::LoadingFeedback::FileNotFound);
    QCOMPARE(std::get<0>(fileModel.loadJsonFromFile(QFileInfo{tmp.filePath(QStringLiteral("config/laser_controls/figureWobble0.json"))}, FileType::Wobble)), FileModel::LoadingFeedback::EmptyFile);
    QVERIFY(fileModel.filename().isEmpty());
    QCOMPARE(filenameChanged.count(), 0);
    QCOMPARE(figureLoaded.count(), 0);

    QCOMPARE(fileModel.loadJsonFromFile(QStringLiteral("figureWobble1.json")), FileModel::LoadingFeedback::FileNotFound);
    QCOMPARE(std::get<0>(fileModel.loadJsonFromFile(QFileInfo{tmp.filePath(QStringLiteral("config/laser_controls/figureWobble1.json"))}, FileType::Wobble)), FileModel::LoadingFeedback::CorruptedFile);
    QVERIFY(fileModel.filename().isEmpty());
    QCOMPARE(filenameChanged.count(), 0);
    QCOMPARE(figureLoaded.count(), 0);

    QCOMPARE(fileModel.loadJsonFromFile(QStringLiteral("figureWobble2.json")), FileModel::LoadingFeedback::NoErrors);
    QVERIFY(!fileModel.filename().isEmpty());
    QCOMPARE(fileModel.filename(), QStringLiteral("figureWobble2.json"));
    QCOMPARE(filenameChanged.count(), 2);
    QCOMPARE(figureLoaded.count(), 1);
    QCOMPARE(static_cast<FileType>(figureLoaded.takeFirst().at(0).toInt()), FileType::Wobble);
    QCOMPARE(fileModel.indexForWobbleFigure(2).data(Qt::UserRole + 4).toString(), QStringLiteral("figureWobble2.json"));

    auto wobbleFigure = fileModel.wobbleFigure();
    QCOMPARE(QString::fromStdString(wobbleFigure.name), QStringLiteral("Test"));
    QCOMPARE(QString::fromStdString(wobbleFigure.ID), QStringLiteral("2"));
    QCOMPARE(QString::fromStdString(wobbleFigure.description), QStringLiteral("Test wobble figure"));
    auto currentfigure = wobbleFigure.figure;
    QFETCH(std::vector<RTC6::wobbleFigure::command::Order>, figure);
    QCOMPARE(currentfigure.size(), figure.size());

    for (std::size_t i = 0; i < currentfigure.size(); i++)
    {
        const auto &currentOrder = currentfigure.at(i);
        const auto &order = figure.at(i);
        QCOMPARE(currentOrder.endPosition, order.endPosition);
        QCOMPARE(currentOrder.power, order.power);
    }

    wobbleFigure = fileModel.loadWobbleFigure("Test");
    QVERIFY(wobbleFigure.figure.empty());

    wobbleFigure = fileModel.loadWobbleFigure("figureWobble2.json");
    currentfigure = wobbleFigure.figure;
    QCOMPARE(currentfigure.size(), figure.size());

    for (std::size_t i = 0; i < currentfigure.size(); i++)
    {
        const auto &currentOrder = currentfigure.at(i);
        const auto &order = figure.at(i);
        QCOMPARE(currentOrder.endPosition, order.endPosition);
        QCOMPARE(currentOrder.power, order.power);
    }
}

typedef std::vector< std::pair<double, double>> FunctionValues;
Q_DECLARE_METATYPE(FunctionValues)
void FileModelTest::testLoadOverlayFunction_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<FunctionValues>("function");

    FunctionValues figure1;
    figure1.emplace_back(-1.0, 0.0);
    figure1.emplace_back(1.0, 0.0);

    QTest::newRow("Seamfile1") << 1 << figure1;
}

void FileModelTest::testLoadOverlayFunction()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    QVERIFY(QDir{tmp.path()}.mkpath(QStringLiteral("config/weld_figure/")));
    QDir dir{tmp.path()};
    QVERIFY(dir.exists());
    dir.mkpath(QStringLiteral("config/weld_figure/"));
    QVERIFY(dir.exists("config/weld_figure/"));
    QVERIFY(dir.cd("config/weld_figure/"));

    auto testOverlayFunction = QFINDTESTDATA(QStringLiteral("testData/overlayFunction0.json"));
    QVERIFY( QFile::copy(testOverlayFunction, tmp.filePath(QStringLiteral("config/weld_figure/overlayFunction0.json"))));
    auto testOverlayFunctionV1 = QFINDTESTDATA(QStringLiteral("testData/overlayFunction1.json"));
    QVERIFY( QFile::copy(testOverlayFunctionV1, tmp.filePath(QStringLiteral("config/weld_figure/overlayFunction1.json"))));
    auto testOverlayFunctionV2 = QFINDTESTDATA(QStringLiteral("testData/overlayFunction2.json"));
    QVERIFY( QFile::copy(testOverlayFunctionV2, tmp.filePath(QStringLiteral("config/weld_figure/overlayFunction2.json"))));
    auto testOverlayFunctionV3 = QFINDTESTDATA(QStringLiteral("testData/overlayFunction3.json"));
    QVERIFY( QFile::copy(testOverlayFunctionV3, tmp.filePath(QStringLiteral("config/weld_figure/overlayFunction3.json"))));
    auto testOverlayFunctionV4 = QFINDTESTDATA(QStringLiteral("testData/overlayFunction4.json"));
    QVERIFY( QFile::copy(testOverlayFunctionV4, tmp.filePath(QStringLiteral("config/weld_figure/overlayFunction4.json"))));

    qputenv("WM_BASE_DIR", tmp.path().toLocal8Bit());

    FileModel fileModel;

    QSignalSpy figureLoaded{&fileModel, &FileModel::figureLoaded};
    QVERIFY(figureLoaded.isValid());
    QCOMPARE(figureLoaded.count(), 0);

    QSignalSpy filenameChanged{&fileModel, &FileModel::filenameChanged};
    QVERIFY(filenameChanged.isValid());
    QCOMPARE(filenameChanged.count(), 0);

    QCOMPARE(fileModel.loadJsonFromFile(QStringLiteral("")), FileModel::LoadingFeedback::NameOrTypeError);
    QVERIFY(fileModel.filename().isEmpty());
    QCOMPARE(filenameChanged.count(), 0);
    QCOMPARE(figureLoaded.count(), 0);

    QSignalSpy loadingChangedSpy{&fileModel, &FileModel::loadingChanged};
    QVERIFY(loadingChangedSpy.isValid());
    QCOMPARE(loadingChangedSpy.count(), 0);

    fileModel.loadFiles();
    QCOMPARE(loadingChangedSpy.count(), 1);
    QTRY_COMPARE(loadingChangedSpy.count(), 2);
    const auto &files = fileModel.files();
    QVERIFY(!files.empty());
    QCOMPARE(files.size(), 3);

    fileModel.setFileType(FileType::Overlay);
    QCOMPARE(fileModel.loadJsonFromFile(QStringLiteral("")), FileModel::LoadingFeedback::NameOrTypeError);
    QVERIFY(fileModel.filename().isEmpty());
    QCOMPARE(filenameChanged.count(), 0);
    QCOMPARE(figureLoaded.count(), 0);

    QCOMPARE(fileModel.loadJsonFromFile(QStringLiteral("abc0.json")), FileModel::LoadingFeedback::FileNotFound);
    QVERIFY(fileModel.filename().isEmpty());
    QCOMPARE(filenameChanged.count(), 0);
    QCOMPARE(figureLoaded.count(), 0);

    QCOMPARE(fileModel.loadJsonFromFile(QStringLiteral("overlayFunction0.json")), FileModel::LoadingFeedback::FileNotFound);
    QCOMPARE(std::get<0>(fileModel.loadJsonFromFile(QFileInfo{tmp.filePath(QStringLiteral("config/weld_figure/overlayFunction0.json"))}, FileType::Overlay)), FileModel::LoadingFeedback::EmptyFile);
    QVERIFY(fileModel.filename().isEmpty());
    QCOMPARE(filenameChanged.count(), 0);
    QCOMPARE(figureLoaded.count(), 0);

    QCOMPARE(fileModel.loadJsonFromFile(QStringLiteral("overlayFunction1.json")), FileModel::LoadingFeedback::FileNotFound);
    QCOMPARE(std::get<0>(fileModel.loadJsonFromFile(QFileInfo{tmp.filePath(QStringLiteral("config/weld_figure/overlayFunction1.json"))}, FileType::Overlay)), FileModel::LoadingFeedback::CorruptedFile);
    QVERIFY(fileModel.filename().isEmpty());
    QCOMPARE(filenameChanged.count(), 0);
    QCOMPARE(figureLoaded.count(), 0);

    QCOMPARE(fileModel.loadJsonFromFile(QStringLiteral("overlayFunction2.json")), FileModel::LoadingFeedback::NoErrors);
    QVERIFY(!fileModel.filename().isEmpty());
    QCOMPARE(fileModel.filename(), QStringLiteral("overlayFunction2.json"));
    QCOMPARE(filenameChanged.count(), 2);
    QCOMPARE(figureLoaded.count(), 1);
    QCOMPARE(static_cast<FileType>(figureLoaded.takeFirst().at(0).toInt()), FileType::Overlay);

    const auto &overlayFunction = fileModel.overlayFigure();
    QCOMPARE(QString::fromStdString(overlayFunction.name), QStringLiteral("Testfile A"));
    QCOMPARE(QString::fromStdString(overlayFunction.ID), QStringLiteral("2"));
    QCOMPARE(QString::fromStdString(overlayFunction.description), QStringLiteral("This is a simple and short test file."));
    QTEST(overlayFunction.functionValues, "function");

    auto overlayForFigureCreator = fileModel.loadOverlayFunction("");
    QVERIFY(QString::fromStdString(overlayForFigureCreator.name).isEmpty());
    QVERIFY(QString::fromStdString(overlayForFigureCreator.ID).isEmpty());
    QVERIFY(QString::fromStdString(overlayForFigureCreator.description).isEmpty());
    QVERIFY(overlayForFigureCreator.functionValues.empty());

    overlayForFigureCreator = fileModel.loadOverlayFunction("abc.json");
    QVERIFY(QString::fromStdString(overlayForFigureCreator.name).isEmpty());
    QVERIFY(QString::fromStdString(overlayForFigureCreator.ID).isEmpty());
    QVERIFY(QString::fromStdString(overlayForFigureCreator.description).isEmpty());
    QVERIFY(overlayForFigureCreator.functionValues.empty());

    overlayForFigureCreator = fileModel.loadOverlayFunction("overlayFunction0.json");
    QVERIFY(QString::fromStdString(overlayForFigureCreator.name).isEmpty());
    QVERIFY(QString::fromStdString(overlayForFigureCreator.ID).isEmpty());
    QVERIFY(QString::fromStdString(overlayForFigureCreator.description).isEmpty());
    QVERIFY(overlayForFigureCreator.functionValues.empty());

    overlayForFigureCreator = fileModel.loadOverlayFunction("overlayFunction1.json");
    QVERIFY(QString::fromStdString(overlayForFigureCreator.name).isEmpty());
    QVERIFY(QString::fromStdString(overlayForFigureCreator.ID).isEmpty());
    QVERIFY(QString::fromStdString(overlayForFigureCreator.description).isEmpty());
    QVERIFY(overlayForFigureCreator.functionValues.empty());

    overlayForFigureCreator = fileModel.loadOverlayFunction("overlayFunction2.json");
    QCOMPARE(QString::fromStdString(overlayForFigureCreator.name), QStringLiteral("Testfile A"));
    QCOMPARE(QString::fromStdString(overlayForFigureCreator.ID), QStringLiteral("2"));
    QCOMPARE(QString::fromStdString(overlayForFigureCreator.description), QStringLiteral("This is a simple and short test file."));
    QTEST(overlayForFigureCreator.functionValues, "function");
}

void FileModelTest::testLoadBasicFigure_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<std::vector<RTC6::wobbleFigure::command::Order>>("basicFigure");

    std::vector<RTC6::wobbleFigure::command::Order> figureLine;
    RTC6::wobbleFigure::command::Order order;

    order.endPosition = std::make_pair(0.0, 0.0);
    order.power = 0.5;
    order.ringPower = 0.5;
    figureLine.push_back(order);
    order.endPosition = std::make_pair(0.0, 0.5);
    order.power = 0.75;
    order.ringPower = 0.75;
    figureLine.push_back(order);
    order.endPosition = std::make_pair(0.0, 0.0);
    order.power = 0.5;
    order.ringPower = 0.5;
    figureLine.push_back(order);
    order.endPosition = std::make_pair(0.0, -0.5);
    order.power = 0.35;
    order.ringPower = 0.45;
    figureLine.push_back(order);
    order.endPosition = std::make_pair(0.0, 0.0);
    order.power = 0.5;
    order.ringPower = 0.5;
    figureLine.push_back(order);

    QTest::newRow("WobbleFile") << 1 << figureLine;
}

void FileModelTest::testLoadBasicFigure()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    QVERIFY(QDir{tmp.path()}.mkpath(QStringLiteral("system_graphs/basic_figure/")));
    QDir dir{tmp.path()};
    QVERIFY(dir.exists());
    dir.mkpath(QStringLiteral("system_graphs/basic_figure/"));
    QVERIFY(dir.exists("system_graphs/basic_figure/"));
    QVERIFY(dir.cd("system_graphs/basic_figure/"));

    auto testBasicFigure = QFINDTESTDATA(QStringLiteral("testData/basicCircle.json"));
    QVERIFY( QFile::copy(testBasicFigure, tmp.filePath(QStringLiteral("system_graphs/basic_figure/basicCircle.json"))));
    auto testBasicFigure2 = QFINDTESTDATA(QStringLiteral("testData/basicEight.json"));
    QVERIFY( QFile::copy(testBasicFigure2, tmp.filePath(QStringLiteral("system_graphs/basic_figure/basicEight.json"))));
    auto testBasicFigure3 = QFINDTESTDATA(QStringLiteral("testData/basicInfinity.json"));
    QVERIFY( QFile::copy(testBasicFigure3, tmp.filePath(QStringLiteral("system_graphs/basic_figure/basicInfinity.json"))));
    auto testBasicFigure4 = QFINDTESTDATA(QStringLiteral("testData/basicLine.json"));
    QVERIFY( QFile::copy(testBasicFigure4, tmp.filePath(QStringLiteral("system_graphs/basic_figure/basicLine.json"))));
    auto testOverlayFunctionV4 = QFINDTESTDATA(QStringLiteral("testData/basicSine.json"));
    QVERIFY( QFile::copy(testOverlayFunctionV4, tmp.filePath(QStringLiteral("system_graphs/basic_figure/basicSine.json"))));

    qputenv("WM_BASE_DIR", tmp.path().toLocal8Bit());

    FileModel fileModel;

    QSignalSpy figureLoaded{&fileModel, &FileModel::figureLoaded};
    QVERIFY(figureLoaded.isValid());
    QCOMPARE(figureLoaded.count(), 0);

    QSignalSpy filenameChanged{&fileModel, &FileModel::filenameChanged};
    QVERIFY(filenameChanged.isValid());
    QCOMPARE(filenameChanged.count(), 0);

    QCOMPARE(fileModel.loadJsonFromFile(QStringLiteral("")), FileModel::LoadingFeedback::NameOrTypeError);
    QVERIFY(fileModel.filename().isEmpty());
    QCOMPARE(filenameChanged.count(), 0);
    QCOMPARE(figureLoaded.count(), 0);

    QSignalSpy loadingChangedSpy{&fileModel, &FileModel::loadingChanged};
    QVERIFY(loadingChangedSpy.isValid());
    QCOMPARE(loadingChangedSpy.count(), 0);

    fileModel.loadFiles();
    QCOMPARE(loadingChangedSpy.count(), 1);
    QTRY_COMPARE(loadingChangedSpy.count(), 2);
    const auto &files = fileModel.files();
    QVERIFY(!files.empty());
    QCOMPARE(files.size(), 5);

    fileModel.setFileType(FileType::Basic);
    QCOMPARE(fileModel.loadJsonFromFile(QStringLiteral("")), FileModel::LoadingFeedback::NameOrTypeError);
    QVERIFY(fileModel.filename().isEmpty());
    QCOMPARE(filenameChanged.count(), 0);
    QCOMPARE(figureLoaded.count(), 0);

    QCOMPARE(fileModel.loadJsonFromFile(QStringLiteral("abc0.json")), FileModel::LoadingFeedback::FileNotFound);
    QVERIFY(fileModel.filename().isEmpty());
    QCOMPARE(filenameChanged.count(), 0);
    QCOMPARE(figureLoaded.count(), 0);

    QCOMPARE(fileModel.loadJsonFromFile(QStringLiteral("basicLine.json")), FileModel::LoadingFeedback::NoErrors);
    QVERIFY(!fileModel.filename().isEmpty());
    QCOMPARE(fileModel.filename(), QStringLiteral("basicLine.json"));
    QCOMPARE(filenameChanged.count(), 2);
    QCOMPARE(figureLoaded.count(), 1);
    QCOMPARE(static_cast<FileType>(figureLoaded.takeFirst().at(0).toInt()), FileType::Basic);

    const auto& basicLine = fileModel.wobbleFigure();
    QCOMPARE(QString::fromStdString(basicLine.name), QStringLiteral("Basic figure line"));
    QCOMPARE(QString::fromStdString(basicLine.ID), QStringLiteral("2"));
    QCOMPARE(QString::fromStdString(basicLine.description), QStringLiteral("Line with 4 points"));
    QFETCH(std::vector<RTC6::wobbleFigure::command::Order>, basicFigure);

    for (std::size_t i = 0; i < basicLine.figure.size(); i++)
    {
        QCOMPARE(basicLine.figure.at(i).endPosition.first, basicFigure.at(i).endPosition.first);
        QCOMPARE(basicLine.figure.at(i).endPosition.second, basicFigure.at(i).endPosition.second);
        QCOMPARE(basicLine.figure.at(i).power, basicFigure.at(i).power);
        QCOMPARE(basicLine.figure.at(i).ringPower, basicFigure.at(i).ringPower);
    }

    auto basicFigureLoading = fileModel.loadBasicFigure("");
    QVERIFY(QString::fromStdString(basicFigureLoading.name).isEmpty());
    QVERIFY(QString::fromStdString(basicFigureLoading.ID).isEmpty());
    QVERIFY(QString::fromStdString(basicFigureLoading.description).isEmpty());
    QVERIFY(basicFigureLoading.figure.empty());

    basicFigureLoading = fileModel.loadBasicFigure("abc.json");
    QVERIFY(QString::fromStdString(basicFigureLoading.name).isEmpty());
    QVERIFY(QString::fromStdString(basicFigureLoading.ID).isEmpty());
    QVERIFY(QString::fromStdString(basicFigureLoading.description).isEmpty());
    QVERIFY(basicFigureLoading.figure.empty());

    basicFigureLoading = fileModel.loadBasicFigure("basicLine.json");
    QVERIFY(!QString::fromStdString(basicFigureLoading.name).isEmpty());
    QVERIFY(!QString::fromStdString(basicFigureLoading.ID).isEmpty());
    QVERIFY(!QString::fromStdString(basicFigureLoading.description).isEmpty());
    QVERIFY(!basicFigureLoading.figure.empty());

    for (std::size_t i = 0; i < basicFigureLoading.figure.size(); i++)
    {
        QCOMPARE(basicFigureLoading.figure.at(i).endPosition.first, basicFigure.at(i).endPosition.first);
        QCOMPARE(basicFigureLoading.figure.at(i).endPosition.second, basicFigure.at(i).endPosition.second);
        QCOMPARE(basicFigureLoading.figure.at(i).power, basicFigure.at(i).power);
        QCOMPARE(basicFigureLoading.figure.at(i).ringPower, basicFigure.at(i).ringPower);
    }
}

void FileModelTest::testSaveSeamFigure_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<std::vector<RTC6::seamFigure::command::Order>>("figure");
    QTest::addColumn<std::vector<RTC6::seamFigure::command::Order>>("figure2");

    std::vector<RTC6::seamFigure::command::Order> figure1;
    std::vector<RTC6::seamFigure::command::Order> figure2;
    RTC6::seamFigure::command::Order order;

    order.endPosition = std::make_pair(0.0, 0.0);
    order.power = 25.0;
    order.ringPower = -1.0;
    order.velocity = 100.0;
    figure1.push_back(order);
    order.endPosition = std::make_pair(-2.5, 0.0);
    order.power = 30.0;
    order.velocity = 200.0;
    figure1.push_back(order);
    order.endPosition = std::make_pair(0.0, 0.0);
    order.power = 55.0;
    order.velocity = 130.0;
    figure1.push_back(order);
    order.endPosition = std::make_pair(2.5, 0);
    order.power = 2.0;
    order.velocity = 150.5;
    figure1.push_back(order);
    order.endPosition = std::make_pair(0.0, 0.0);
    order.power = -1.0;
    order.velocity = -1.0;
    figure1.push_back(order);
    order.endPosition = std::make_pair(-2.0, 0.0);
    order.power = 25.2;
    order.ringPower = -1.0;
    order.velocity = -1.0;
    figure2 = figure1;
    figure2.at(0).endPosition = std::make_pair(-5.0, 7.5);
    order.endPosition = std::make_pair(15.0, -10.0);
    order.power = -1.0;
    figure2.push_back(order);

    QTest::newRow("SeamFiles") << 1 << figure1 << figure2;
}

void FileModelTest::testSaveSeamFigure()
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

    qputenv("WM_BASE_DIR", tmp.path().toLocal8Bit());

    FileModel fileModel;
    QSignalSpy loadingChangedSpy{&fileModel, &FileModel::loadingChanged};
    QVERIFY(loadingChangedSpy.isValid());
    QCOMPARE(loadingChangedSpy.count(), 0);

    fileModel.loadFiles();
    QCOMPARE(loadingChangedSpy.count(), 1);
    QTRY_COMPARE(loadingChangedSpy.count(), 2);

    fileModel.setFileType(FileType::Seam);
    fileModel.loadJsonFromFile("weldingSeam1.json");

    auto seamFigure = fileModel.seamFigure();
    QCOMPARE(QString::fromStdString(seamFigure.name), "Test");
    QCOMPARE(QString::fromStdString(seamFigure.ID), "1");
    QCOMPARE(QString::fromStdString(seamFigure.description), "Test file without ring power");

    QFETCH(std::vector<RTC6::seamFigure::command::Order>, figure);
    QCOMPARE(seamFigure.figure.size(), figure.size());
    for (std::size_t i = 0; i < seamFigure.figure.size(); i++)
    {
        const auto &currentPoint = seamFigure.figure.at(i);
        const auto &referencePoint = figure.at(i);
        QCOMPARE(currentPoint.endPosition, referencePoint.endPosition);
        QCOMPARE(currentPoint.power, referencePoint.power);
        QCOMPARE(currentPoint.ringPower, referencePoint.ringPower);
        QCOMPARE(currentPoint.velocity, referencePoint.velocity);
    }

    seamFigure.name = std::string("This is a better name");
    seamFigure.figure.at(0).endPosition = std::make_pair(-5.0, 7.5);
    RTC6::seamFigure::command::Order newOrder;
    newOrder.endPosition = std::make_pair(15.0, -10.0);
    newOrder.power = -1.0;
    newOrder.ringPower = -1.0;
    newOrder.velocity = -1.0;
    seamFigure.figure.push_back(newOrder);
    RTC6::seamFigure::Ramp ramp;
    ramp.startPointID = 0;
    ramp.length = 0.05;
    ramp.startPower = 1.0;
    ramp.endPower = 0.0;
    ramp.startPowerRing = 0.0;
    ramp.endPowerRing = 1.0;
    seamFigure.ramps.push_back(ramp);
    QVERIFY(fileModel.saveFigure(seamFigure));
    QCOMPARE(loadingChangedSpy.count(), 2);

    QFETCH(std::vector<RTC6::seamFigure::command::Order>, figure2);
    QCOMPARE(seamFigure.figure.size(), figure2.size());

    fileModel.loadJsonFromFile("weldingSeam1.json");
    seamFigure = fileModel.seamFigure();
    QCOMPARE(QString::fromStdString(seamFigure.name), "This is a better name");
    QCOMPARE(QString::fromStdString(seamFigure.ID), "1");
    QCOMPARE(QString::fromStdString(seamFigure.description), "Test file without ring power");
    QCOMPARE(seamFigure.figure.size(), figure2.size());
    QCOMPARE(seamFigure.ramps.front(), ramp);

    for (std::size_t i = 0; i < seamFigure.figure.size(); i++)
    {
        const auto &currentPoint = seamFigure.figure.at(i);
        const auto &referencePoint = figure2.at(i);
        QCOMPARE(currentPoint.endPosition, referencePoint.endPosition);
        QCOMPARE(currentPoint.power, referencePoint.power);
        QCOMPARE(currentPoint.ringPower, referencePoint.ringPower);
        QCOMPARE(currentPoint.velocity, referencePoint.velocity);
    }
}

void FileModelTest::testSaveWobbleFigure_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<std::vector<RTC6::wobbleFigure::command::Order>>("figure");
    QTest::addColumn<std::vector<RTC6::wobbleFigure::command::Order>>("figure2");

    std::vector<RTC6::wobbleFigure::command::Order> figure1;
    std::vector<RTC6::wobbleFigure::command::Order> figure2;
    RTC6::wobbleFigure::command::Order order;

    order.endPosition = std::make_pair(0.0, 0.0);
    order.power = 0.0;
    figure1.push_back(order);
    order.endPosition = std::make_pair(3.0, 0.0);
    order.power = 0.15;
    figure1.push_back(order);
    order.endPosition = std::make_pair(-3.0, 0.0);
    order.power = 0.5;
    figure1.push_back(order);
    order.endPosition = std::make_pair(0.0, 0);
    order.power = -0.65;
    figure1.push_back(order);
    figure2 = figure1;
    figure2.at(3).endPosition = std::make_pair(1.0, 0.0);
    order.endPosition = std::make_pair(0.0, 0.0);
    order.power = 0.0;
    figure2.push_back(order);

    QTest::newRow("WobbleFiles") << 1 << figure1 << figure2;
}

void FileModelTest::testSaveWobbleFigure()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    QVERIFY(QDir{tmp.path()}.mkpath(QStringLiteral("config/laser_controls/")));
    QDir dir{tmp.path()};
    QVERIFY(dir.exists());
    dir.mkpath(QStringLiteral("config/laser_controls/"));
    QVERIFY(dir.exists("config/laser_controls/"));
    QVERIFY(dir.cd("config/laser_controls/"));

    auto testWobbleFile = QFINDTESTDATA(QStringLiteral("testData/figureWobble2.json"));
    QVERIFY( QFile::copy(testWobbleFile, tmp.filePath(QStringLiteral("config/laser_controls/figureWobble2.json"))));

    qputenv("WM_BASE_DIR", tmp.path().toLocal8Bit());

    FileModel fileModel;
    QSignalSpy loadingChangedSpy{&fileModel, &FileModel::loadingChanged};
    QVERIFY(loadingChangedSpy.isValid());
    QCOMPARE(loadingChangedSpy.count(), 0);

    fileModel.loadFiles();
    QCOMPARE(loadingChangedSpy.count(), 1);
    QTRY_COMPARE(loadingChangedSpy.count(), 2);

    fileModel.setFileType(FileType::Wobble);
    fileModel.loadJsonFromFile("figureWobble2.json");

    auto wobbleFigure = fileModel.wobbleFigure();
    QCOMPARE(QString::fromStdString(wobbleFigure.name), "Test");
    QCOMPARE(QString::fromStdString(wobbleFigure.ID), "2");
    QCOMPARE(QString::fromStdString(wobbleFigure.description), "Test wobble figure");

    QFETCH(std::vector<RTC6::wobbleFigure::command::Order>, figure);
    QCOMPARE(wobbleFigure.figure.size(), figure.size());
    for (std::size_t i = 0; i < wobbleFigure.figure.size(); i++)
    {
        const auto &currentPoint = wobbleFigure.figure.at(i);
        const auto &referencePoint = figure.at(i);
        QCOMPARE(currentPoint.endPosition, referencePoint.endPosition);
        QCOMPARE(currentPoint.power, referencePoint.power);
    }

    wobbleFigure.name = std::string("This is a better name");
    wobbleFigure.figure.at(3).endPosition = std::make_pair(1.0, 0.0);
    RTC6::wobbleFigure::command::Order newOrder;
    newOrder.endPosition = std::make_pair(0.0, 0.0);
    newOrder.power = 0.0;
    wobbleFigure.figure.push_back(newOrder);
    QVERIFY(fileModel.saveFigure(wobbleFigure));
    QCOMPARE(loadingChangedSpy.count(), 2);

    QFETCH(std::vector<RTC6::wobbleFigure::command::Order>, figure2);
    QCOMPARE(wobbleFigure.figure.size(), figure2.size());

    fileModel.loadJsonFromFile("figureWobble2.json");
    wobbleFigure = fileModel.wobbleFigure();
    QCOMPARE(QString::fromStdString(wobbleFigure.name), "This is a better name");
    QCOMPARE(QString::fromStdString(wobbleFigure.ID), "2");
    QCOMPARE(QString::fromStdString(wobbleFigure.description), "Test wobble figure");
    QCOMPARE(wobbleFigure.figure.size(), figure2.size());

    for (std::size_t i = 0; i < wobbleFigure.figure.size(); i++)
    {
        const auto &currentPoint = wobbleFigure.figure.at(i);
        const auto &referencePoint = figure2.at(i);
        QCOMPARE(currentPoint.endPosition, referencePoint.endPosition);
        QCOMPARE(currentPoint.power, referencePoint.power);
    }
}

void FileModelTest::testSaveOverlayFigure_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<FunctionValues>("figure");
    QTest::addColumn<FunctionValues>("figure2");

    FunctionValues figure1;
    FunctionValues figure2;

    figure1.push_back(std::make_pair(-1.0, 0.0));
    figure1.push_back(std::make_pair(1.0, 0.0));
    figure2 = figure1;
    figure2.at(0) = std::make_pair(-2.5, 1.0);
    figure2.push_back(std::make_pair(0.0, 0.0));

    QTest::newRow("OverlayFigures") << 1 << figure1 << figure2;
}

void FileModelTest::testSaveOverlayFigure()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    QVERIFY(QDir{tmp.path()}.mkpath(QStringLiteral("config/weld_figure/")));
    QDir dir{tmp.path()};
    QVERIFY(dir.exists());
    dir.mkpath(QStringLiteral("config/weld_figure/"));
    QVERIFY(dir.exists("config/weld_figure/"));
    QVERIFY(dir.cd("config/weld_figure/"));

    auto testOverlayFunction = QFINDTESTDATA(QStringLiteral("testData/overlayFunction2.json"));
    QVERIFY( QFile::copy(testOverlayFunction, tmp.filePath(QStringLiteral("config/weld_figure/overlayFunction2.json"))));

    qputenv("WM_BASE_DIR", tmp.path().toLocal8Bit());

    FileModel fileModel;
    QSignalSpy loadingChangedSpy{&fileModel, &FileModel::loadingChanged};
    QVERIFY(loadingChangedSpy.isValid());
    QCOMPARE(loadingChangedSpy.count(), 0);

    fileModel.loadFiles();
    QCOMPARE(loadingChangedSpy.count(), 1);
    QTRY_COMPARE(loadingChangedSpy.count(), 2);

    fileModel.setFileType(FileType::Overlay);
    fileModel.loadJsonFromFile("overlayFunction2.json");

    auto overlayFigure = fileModel.overlayFigure();
    QCOMPARE(QString::fromStdString(overlayFigure.name), "Testfile A");
    QCOMPARE(QString::fromStdString(overlayFigure.ID), "2");
    QCOMPARE(QString::fromStdString(overlayFigure.description), "This is a simple and short test file.");

    QFETCH(FunctionValues, figure);
    QCOMPARE(overlayFigure.functionValues.size(), figure.size());
    for (std::size_t i = 0; i < overlayFigure.functionValues.size(); i++)
    {
        const auto &currentPoint = overlayFigure.functionValues.at(i);
        const auto &referencePoint = figure.at(i);
        QCOMPARE(currentPoint, referencePoint);
    }

    overlayFigure.name = std::string("This is a better name");
    overlayFigure.functionValues.at(0) = std::make_pair(-2.5, 1.0);
    overlayFigure.functionValues.push_back(std::make_pair(0.0, 0.0));
    QVERIFY(fileModel.saveFigure(overlayFigure));
    QCOMPARE(loadingChangedSpy.count(), 2);

    QFETCH(FunctionValues, figure2);
    QCOMPARE(overlayFigure.functionValues.size(), figure2.size());

    fileModel.loadJsonFromFile("overlayFunction2.json");
    overlayFigure = fileModel.overlayFigure();
    QCOMPARE(QString::fromStdString(overlayFigure.name), "This is a better name");
    QCOMPARE(QString::fromStdString(overlayFigure.ID), "2");
    QCOMPARE(QString::fromStdString(overlayFigure.description), "This is a simple and short test file.");
    QCOMPARE(overlayFigure.functionValues.size(), figure2.size());

    for (std::size_t i = 0; i < overlayFigure.functionValues.size(); i++)
    {
        const auto &currentPoint = overlayFigure.functionValues.at(i);
        const auto &referencePoint = figure2.at(i);
        QCOMPARE(currentPoint, referencePoint);
    }

}

void FileModelTest::testSaveBasicFigure_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<std::vector<RTC6::wobbleFigure::command::Order>>("figure");

    std::vector<RTC6::wobbleFigure::command::Order> figureLine;
    RTC6::wobbleFigure::command::Order order;

    order.endPosition = std::make_pair(0.5, 0.0);
    order.power = 0.0;
    order.ringPower = 0.0;
    figureLine.push_back(order);
    order.endPosition = std::make_pair(0.4729086208503173, -0.16234973460234173);
    order.power = 0.0;
    order.ringPower = 0.0;
    figureLine.push_back(order);
    order.endPosition = std::make_pair(0.3945702546981968, -0.3071063563448339);
    order.power = 0.0;
    order.ringPower = 0.0;
    figureLine.push_back(order);
    order.endPosition = std::make_pair(0.2734740790612135, -0.4185832391312643);
    order.power = 0.0;
    order.ringPower = 0.0;
    figureLine.push_back(order);
    order.endPosition = std::make_pair(0.12274274357039962, -0.4847001329696652);
    order.power = 0.0;
    order.ringPower = 0.0;
    figureLine.push_back(order);
    order.endPosition = std::make_pair(-0.041289672736166134, -0.4982922465033349);
    order.power = 0.0;
    order.ringPower = 0.0;
    figureLine.push_back(order);
    order.endPosition = std::make_pair(-0.20084771232648463, -0.45788666332752875);
    order.power = 0.0;
    order.ringPower = 0.0;
    figureLine.push_back(order);
    order.endPosition = std::make_pair(-0.33864078581287044, -0.3678619553365659);
    order.power = 0.0;
    order.ringPower = 0.0;
    figureLine.push_back(order);
    order.endPosition = std::make_pair(-0.4397368756032445, -0.23797369651853684);
    order.power = 0.0;
    order.ringPower = 0.0;
    figureLine.push_back(order);
    order.endPosition = std::make_pair(-0.49318065170136116, -0.08229729514036702);
    order.power = 0.0;
    order.ringPower = 0.0;
    figureLine.push_back(order);
    order.endPosition = std::make_pair(-0.4931806517013612, 0.08229729514036689);
    order.power = 0.0;
    order.ringPower = 0.0;
    figureLine.push_back(order);
    order.endPosition = std::make_pair(-0.43973687560324454, 0.23797369651853675);
    order.power = 0.0;
    order.ringPower = 0.0;
    figureLine.push_back(order);
    order.endPosition = std::make_pair(-0.3386407858128707, 0.3678619553365656);
    order.power = 0.0;
    order.ringPower = 0.0;
    figureLine.push_back(order);
    order.endPosition = std::make_pair(-0.20084771232648493, 0.45788666332752864);
    order.power = 0.0;
    order.ringPower = 0.0;
    figureLine.push_back(order);
    order.endPosition = std::make_pair(-0.04128967273616637, 0.4982922465033349);
    order.power = 0.0;
    order.ringPower = 0.0;
    figureLine.push_back(order);
    order.endPosition = std::make_pair(0.1227427435703994, 0.48470013296966524);
    order.power = 0.0;
    order.ringPower = 0.0;
    figureLine.push_back(order);
    order.endPosition = std::make_pair(0.2734740790612133, 0.4185832391312644);
    order.power = 0.0;
    order.ringPower = 0.0;
    figureLine.push_back(order);
    order.endPosition = std::make_pair(0.3945702546981967, 0.307106356344834);
    order.power = 0.0;
    order.ringPower = 0.0;
    figureLine.push_back(order);
    order.endPosition = std::make_pair(0.4729086208503173, 0.16234973460234187);
    order.power = 0.0;
    order.ringPower = 0.0;
    figureLine.push_back(order);
    order.endPosition = std::make_pair(0.5, 0.0);
    order.power = 0.0;
    order.ringPower = 0.0;
    figureLine.push_back(order);

    QTest::newRow("WobbleFile") << 1 << figureLine;
}

void FileModelTest::testSaveBasicFigure()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    QVERIFY(QDir{tmp.path()}.mkpath(QStringLiteral("system_graphs/basic_figure/")));
    QDir dir{tmp.path()};
    QVERIFY(dir.exists());
    dir.mkpath(QStringLiteral("system_graphs/basic_figure/"));
    QVERIFY(dir.exists("system_graphs/basic_figure/"));
    QVERIFY(dir.cd("system_graphs/basic_figure/"));

    auto testBasicFigure = QFINDTESTDATA(QStringLiteral("testData/basicCircle.json"));
    QVERIFY( QFile::copy(testBasicFigure, tmp.filePath(QStringLiteral("system_graphs/basic_figure/basicCircle.json"))));

    QVERIFY(QDir{tmp.path()}.mkpath(QStringLiteral("config/laser_controls/")));
    QDir saveAsDir{tmp.path()};
    QVERIFY(saveAsDir.exists());
    saveAsDir.mkpath(QStringLiteral("config/laser_controls/"));
    QVERIFY(saveAsDir.exists("config/laser_controls/"));
    QVERIFY(saveAsDir.cd("config/laser_controls/"));

    auto testWobbleFile = QFINDTESTDATA(QStringLiteral("testData/figureWobble0.json"));
    QVERIFY( QFile::copy(testWobbleFile, tmp.filePath(QStringLiteral("config/laser_controls/figureWobble0.json"))));

    qputenv("WM_BASE_DIR", tmp.path().toLocal8Bit());

    FileModel fileModel;
    QSignalSpy loadingChangedSpy{&fileModel, &FileModel::loadingChanged};
    QVERIFY(loadingChangedSpy.isValid());
    QCOMPARE(loadingChangedSpy.count(), 0);

    fileModel.loadFiles();
    QCOMPARE(loadingChangedSpy.count(), 1);
    QTRY_COMPARE(loadingChangedSpy.count(), 2);

    fileModel.setFileType(FileType::Basic);
    fileModel.loadJsonFromFile("basicCircle.json");

    auto basicFigure = fileModel.wobbleFigure();
    QCOMPARE(QString::fromStdString(basicFigure.name), "Basic figure circle");
    QCOMPARE(QString::fromStdString(basicFigure.ID), "3");
    QCOMPARE(QString::fromStdString(basicFigure.description), "Circle with 19 vectors");

    QFETCH(std::vector<RTC6::wobbleFigure::command::Order>, figure);
    QCOMPARE(basicFigure.figure.size(), figure.size());
    for (std::size_t i = 0; i < basicFigure.figure.size(); i++)
    {
        const auto &currentPoint = basicFigure.figure.at(i);
        const auto &referencePoint = figure.at(i);
        QCOMPARE(currentPoint.endPosition.first, referencePoint.endPosition.first);
        QCOMPARE(currentPoint.endPosition.second, referencePoint.endPosition.second);
        QCOMPARE(currentPoint.power, referencePoint.power);
        QCOMPARE(currentPoint.ringPower, referencePoint.ringPower);
    }
}

QTEST_GUILESS_MAIN(FileModelTest)
#include "testFileModel.moc"
