#include <QTest>
#include <QSignalSpy>

#include "../hardwareConfigurationBackupModel.h"

using precitec::gui::HardwareConfigurationBackupModel;

class HardwareConfigurationBackupModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testSetConfigDir();
    void testSetBackupDir();
    void testBackupNoFiles();
    void testBackupLinkFiles();
    void testBackupCalibrationGridFiles();
    void testBackupScanmasterFiles();
    void testBackupFiles();
    void testRestoreFailsInvalidRow();
    void testRestoreFailsInvalidConfigDir();
};

void HardwareConfigurationBackupModelTest::initTestCase()
{
    qputenv("WM_BASE_DIR", QFINDTESTDATA("testdata").toUtf8());
}

void HardwareConfigurationBackupModelTest::testCtor()
{
    HardwareConfigurationBackupModel model;
    QCOMPARE(model.configDir(), QString{});
    QCOMPARE(model.backupDir(), QString{});
    QCOMPARE(model.rowCount(), 0);
}

void HardwareConfigurationBackupModelTest::testSetConfigDir()
{
    HardwareConfigurationBackupModel model;
    QCOMPARE(model.configDir(), QString{});
    QSignalSpy configDirChangedSpy{&model, &HardwareConfigurationBackupModel::configDirChanged};
    QVERIFY(configDirChangedSpy.isValid());

    QTemporaryDir dir;
    model.setConfigDir(dir.path());
    QCOMPARE(model.configDir(), dir.path());
    QCOMPARE(configDirChangedSpy.count(), 1);

    // setting again should not change
    model.setConfigDir(dir.path());
    QCOMPARE(configDirChangedSpy.count(), 1);

    // backup should fail as backup dir is not defined
    QSignalSpy backupFailedSpy{&model, &HardwareConfigurationBackupModel::backupFailed};
    QVERIFY(backupFailedSpy.isValid());
    model.createBackup();
    QCOMPARE(backupFailedSpy.count(), 1);
}

void HardwareConfigurationBackupModelTest::testSetBackupDir()
{
    HardwareConfigurationBackupModel model;
    QCOMPARE(model.backupDir(), QString{});
    QSignalSpy backupDirChangedSpy{&model, &HardwareConfigurationBackupModel::backupDirChanged};
    QVERIFY(backupDirChangedSpy.isValid());

    QTemporaryDir dir;
    model.setBackupDir(dir.path());
    QCOMPARE(model.backupDir(), dir.path());
    QCOMPARE(backupDirChangedSpy.count(), 1);

    // setting again should not change
    model.setBackupDir(dir.path());
    QCOMPARE(backupDirChangedSpy.count(), 1);

    // backup should fail as config dir is not defined
    QSignalSpy backupFailedSpy{&model, &HardwareConfigurationBackupModel::backupFailed};
    QVERIFY(backupFailedSpy.isValid());
    model.createBackup();
    QCOMPARE(backupFailedSpy.count(), 1);
}

void HardwareConfigurationBackupModelTest::testBackupNoFiles()
{
    HardwareConfigurationBackupModel model;
    QTemporaryDir backupDir;
    QTemporaryDir configDir;
    model.setBackupDir(backupDir.path());
    model.setConfigDir(configDir.path());

    QSignalSpy backupFailedSpy{&model, &HardwareConfigurationBackupModel::backupFailed};
    QVERIFY(backupFailedSpy.isValid());
    model.createBackup();
    QCOMPARE(backupFailedSpy.count(), 1);

    // sub directory should not be created
    QCOMPARE(QDir{backupDir.path()}.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot).size(), 0);
    QVERIFY(QDir{backupDir.path()}.exists());
}

void HardwareConfigurationBackupModelTest::testBackupLinkFiles()
{
    HardwareConfigurationBackupModel model;
    QTemporaryDir backupDir;
    QTemporaryDir configDir;
    model.setBackupDir(backupDir.path());
    model.setConfigDir(configDir.path());

    const QString sourceViConfig{QFINDTESTDATA("../../../../wm_inst/config_templates/VI_Config.xml")};
    const QString sourceSystemConfig{QFINDTESTDATA("../../../../wm_inst/config_templates/track_compact/SystemConfig.xml")};
    const QString sourceCalibrationConfig{QFINDTESTDATA("../../../../Analyzer_Interface/autotests/testdata/coax/config/calibrationData0.xml")};

    QVERIFY(QFile::link(sourceViConfig, configDir.filePath(QStringLiteral("VI_Config.xml"))));
    QVERIFY(QFile::link(sourceSystemConfig, configDir.filePath(QStringLiteral("SystemConfig.xml"))));
    QVERIFY(QFile::link(sourceCalibrationConfig, configDir.filePath(QStringLiteral("calibrationData0.xml"))));

    QSignalSpy backupSucceededSpy{&model, &HardwareConfigurationBackupModel::backupSucceeded};
    QVERIFY(backupSucceededSpy.isValid());
    QCOMPARE(model.rowCount(), 0);
    model.createBackup();
    QCOMPARE(backupSucceededSpy.count(), 1);
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.rowCount(model.index(0, 0)), 0);

    const auto archives = QDir{backupDir.path()}.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    QCOMPARE(archives.size(), 1);
    QCOMPARE(model.index(0, 0).data().toDateTime(), QDateTime::fromString(archives.first().fileName(), QStringLiteral("yyyyMMdd-HHmm")));
    const auto files = QDir{archives.first().filePath()}.entryInfoList(QDir::Files, QDir::Name);
    QCOMPARE(files.size(), 4);
    QCOMPARE(files.at(0).fileName(), QStringLiteral("SystemConfig.xml"));
    QCOMPARE(files.at(1).fileName(), QStringLiteral("VI_Config.xml"));
    QCOMPARE(files.at(2).fileName(), QStringLiteral("calibrationData0.xml"));
    QCOMPARE(files.at(3).fileName(), QStringLiteral("systemHardwareConfiguration.json"));
    QCOMPARE(files.at(0).isFile(), true);
    QCOMPARE(files.at(0).isSymLink(), false);
    QCOMPARE(files.at(1).isFile(), true);
    QCOMPARE(files.at(1).isSymLink(), false);
    QCOMPARE(files.at(2).isFile(), true);
    QCOMPARE(files.at(2).isSymLink(), false);
    QCOMPARE(files.at(3).isFile(), true);
    QCOMPARE(files.at(3).isSymLink(), false);

    QFile sourceSystemFile{sourceSystemConfig};
    QVERIFY(sourceSystemFile.open(QIODevice::ReadOnly));
    QFile targetSystemConfigFile{files.at(0).absoluteFilePath()};
    QVERIFY(targetSystemConfigFile.open(QIODevice::ReadOnly));
    const auto sourceSystemFileData{sourceSystemFile.readAll()};
    QCOMPARE(sourceSystemFileData, targetSystemConfigFile.readAll());

    QFile sourceViConfigFile{sourceViConfig};
    QVERIFY(sourceViConfigFile.open(QIODevice::ReadOnly));
    QFile targetViConfigFile{files.at(1).absoluteFilePath()};
    QVERIFY(targetViConfigFile.open(QIODevice::ReadOnly));
    const auto sourceViConfigData{sourceViConfigFile.readAll()};
    QCOMPARE(sourceViConfigData, targetViConfigFile.readAll());

    // and restore
    QSignalSpy restoreSucceededSpy{&model, &HardwareConfigurationBackupModel::restoreSucceeded};
    QVERIFY(restoreSucceededSpy.isValid());
    model.restore(0);
    QCOMPARE(restoreSucceededSpy.count(), 1);

    const auto configDirFiles = QDir{configDir.path()}.entryInfoList(QDir::Files, QDir::Name);
    QCOMPARE(configDirFiles.size(), 8);
    QCOMPARE(configDirFiles.at(0).fileName(), QStringLiteral("SystemConfig.xml"));
    QVERIFY(configDirFiles.at(1).fileName().startsWith(QStringLiteral("SystemConfig.xml")));
    QVERIFY(configDirFiles.at(1).fileName().endsWith(QStringLiteral(".bak")));
    QCOMPARE(configDirFiles.at(2).fileName(), QStringLiteral("VI_Config.xml"));
    QVERIFY(configDirFiles.at(3).fileName().startsWith(QStringLiteral("VI_Config.xml")));
    QVERIFY(configDirFiles.at(3).fileName().endsWith(QStringLiteral(".bak")));
    QCOMPARE(configDirFiles.at(4).fileName(), QStringLiteral("calibrationData0.xml"));
    QVERIFY(configDirFiles.at(5).fileName().startsWith(QStringLiteral("calibrationData0.xml")));
    QVERIFY(configDirFiles.at(5).fileName().endsWith(QStringLiteral(".bak")));
    QCOMPARE(configDirFiles.at(6).fileName(), QStringLiteral("systemHardwareConfiguration.json"));
    QVERIFY(configDirFiles.at(7).fileName().startsWith(QStringLiteral("systemHardwareConfiguration.json")));
    QVERIFY(configDirFiles.at(7).fileName().endsWith(QStringLiteral(".bak")));
    for (int i = 0; i < 3; i++)
    {
        QCOMPARE(configDirFiles.at(2*i).isFile(), true);
        QCOMPARE(configDirFiles.at(2*i).isSymLink(), false);
        QCOMPARE(configDirFiles.at(2*i+1).isFile(), true);
        QCOMPARE(configDirFiles.at(2*i+1).isSymLink(), true);
    }

    QFile backupSystemConfigFile{configDirFiles.at(0).absoluteFilePath()};
    QVERIFY(backupSystemConfigFile.open(QIODevice::ReadOnly));
    QCOMPARE(sourceSystemFileData, backupSystemConfigFile.readAll());

    QFile backupViConfigFile{configDirFiles.at(2).absoluteFilePath()};
    QVERIFY(backupViConfigFile.open(QIODevice::ReadOnly));
    QCOMPARE(sourceViConfigData, backupViConfigFile.readAll());
}

void HardwareConfigurationBackupModelTest::testBackupFiles()
{
    HardwareConfigurationBackupModel model;
    QTemporaryDir backupDir;
    QTemporaryDir configDir;
    model.setBackupDir(backupDir.path());
    model.setConfigDir(configDir.path());

    const QString sourceViConfig{QFINDTESTDATA("../../../../wm_inst/config_templates/VI_Config.xml")};
    const QString sourceSystemConfig{QFINDTESTDATA("../../../../wm_inst/config_templates/track_compact/SystemConfig.xml")};
    const QString sourceCalibrationConfig{QFINDTESTDATA("../../../../Analyzer_Interface/autotests/testdata/coax/config/calibrationData0.xml")};


    QVERIFY(QFile::copy(sourceViConfig, configDir.filePath(QStringLiteral("VI_Config.xml"))));
    QVERIFY(QFile::copy(sourceSystemConfig, configDir.filePath(QStringLiteral("SystemConfig.xml"))));
    QVERIFY(QFile::copy(sourceCalibrationConfig, configDir.filePath(QStringLiteral("calibrationData0.xml"))));

    // create camera.xml
    QFile camera{configDir.filePath(QStringLiteral("camera.xml"))};
    QVERIFY(camera.open(QIODevice::WriteOnly));
    camera.close();

    QFile cameraMV4{configDir.filePath(QStringLiteral("camera-PF-MV4.xml"))};
    QVERIFY(cameraMV4.open(QIODevice::WriteOnly));
    cameraMV4.close();

    QSignalSpy backupSucceededSpy{&model, &HardwareConfigurationBackupModel::backupSucceeded};
    QVERIFY(backupSucceededSpy.isValid());
    QCOMPARE(model.rowCount(), 0);
    model.createBackup();
    QCOMPARE(backupSucceededSpy.count(), 1);
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.rowCount(model.index(0, 0)), 0);

    const auto archives = QDir{backupDir.path()}.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    QCOMPARE(archives.size(), 1);
    QCOMPARE(model.index(0, 0).data().toDateTime(), QDateTime::fromString(archives.first().fileName(), QStringLiteral("yyyyMMdd-HHmm")));
    const auto files = QDir{archives.first().filePath()}.entryInfoList(QDir::Files, QDir::Name);
    QCOMPARE(files.size(), 6);
    QCOMPARE(files.at(0).fileName(), QStringLiteral("SystemConfig.xml"));
    QCOMPARE(files.at(1).fileName(), QStringLiteral("VI_Config.xml"));
    QCOMPARE(files.at(2).fileName(), QStringLiteral("calibrationData0.xml"));
    QCOMPARE(files.at(3).fileName(), QStringLiteral("camera-PF-MV4.xml"));
    QCOMPARE(files.at(4).fileName(), QStringLiteral("camera.xml"));
    QCOMPARE(files.at(0).isFile(), true);
    QCOMPARE(files.at(0).isSymLink(), false);
    QCOMPARE(files.at(1).isFile(), true);
    QCOMPARE(files.at(1).isSymLink(), false);
    QCOMPARE(files.at(2).isFile(), true);
    QCOMPARE(files.at(2).isSymLink(), false);
    QCOMPARE(files.at(3).isFile(), true);
    QCOMPARE(files.at(3).isSymLink(), false);

    QFile sourceSystemFile{sourceSystemConfig};
    QVERIFY(sourceSystemFile.open(QIODevice::ReadOnly));
    QFile targetSystemConfigFile{files.at(0).absoluteFilePath()};
    QVERIFY(targetSystemConfigFile.open(QIODevice::ReadOnly));
    QCOMPARE(sourceSystemFile.readAll(), targetSystemConfigFile.readAll());

    QFile sourceViConfigFile{sourceViConfig};
    QVERIFY(sourceViConfigFile.open(QIODevice::ReadOnly));
    QFile targetViConfigFile{files.at(1).absoluteFilePath()};
    QVERIFY(targetViConfigFile.open(QIODevice::ReadOnly));
    QCOMPARE(sourceViConfigFile.readAll(), targetViConfigFile.readAll());
}

void HardwareConfigurationBackupModelTest::testBackupCalibrationGridFiles()
{
    HardwareConfigurationBackupModel model;
    QTemporaryDir backupDir;
    QTemporaryDir configDir;
    model.setBackupDir(backupDir.path());
    model.setConfigDir(configDir.path());

    const QString sourceViConfig{QFINDTESTDATA("../../../../wm_inst/config_templates/VI_Config.xml")};
    const QString sourceSystemConfig{QFINDTESTDATA("../../../../wm_inst/config_templates/track_compact/SystemConfig.xml")};
    const QString sourceCalibrationConfig{QFINDTESTDATA("../../../../Analyzer_Interface/autotests/testdata/coax/config/calibrationData0.xml")};
    const QString sourceCalibrationGrid{QFINDTESTDATA("../../../../Filtertest/graphs/calibImgData0fallback.csv")};

    
    QVERIFY(QFile::copy(sourceViConfig, configDir.filePath(QStringLiteral("VI_Config.xml"))));
    QVERIFY(QFile::copy(sourceSystemConfig, configDir.filePath(QStringLiteral("SystemConfig.xml"))));
    QVERIFY(QFile::copy(sourceCalibrationConfig, configDir.filePath(QStringLiteral("calibrationData0.xml"))));
    QVERIFY(QFile::copy(sourceCalibrationGrid, configDir.filePath(QStringLiteral("calibImgData0fallback.csv"))));

    QSignalSpy backupSucceededSpy{&model, &HardwareConfigurationBackupModel::backupSucceeded};
    QVERIFY(backupSucceededSpy.isValid());
    QCOMPARE(model.rowCount(), 0);
    model.createBackup();
    QCOMPARE(backupSucceededSpy.count(), 1);
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.rowCount(model.index(0, 0)), 0);

    const auto archives = QDir{backupDir.path()}.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    QCOMPARE(archives.size(), 1);
    QCOMPARE(model.index(0, 0).data().toDateTime(), QDateTime::fromString(archives.first().fileName(), QStringLiteral("yyyyMMdd-HHmm")));
    const auto files = QDir{archives.first().filePath()}.entryInfoList(QDir::Files, QDir::Name);
    QCOMPARE(files.size(), 5);
    QCOMPARE(files.at(0).fileName(), QStringLiteral("SystemConfig.xml"));
    QCOMPARE(files.at(1).fileName(), QStringLiteral("VI_Config.xml"));
    QCOMPARE(files.at(2).fileName(), QStringLiteral("calibImgData0fallback.csv"));
    QCOMPARE(files.at(3).fileName(), QStringLiteral("calibrationData0.xml"));
    QCOMPARE(files.at(4).fileName(), QStringLiteral("systemHardwareConfiguration.json"));
    QCOMPARE(files.at(0).isFile(), true);
    QCOMPARE(files.at(0).isSymLink(), false);
    QCOMPARE(files.at(1).isFile(), true);
    QCOMPARE(files.at(1).isSymLink(), false);
    QCOMPARE(files.at(2).isFile(), true);
    QCOMPARE(files.at(2).isSymLink(), false);

    QFile sourceSystemFile{sourceSystemConfig};
    QVERIFY(sourceSystemFile.open(QIODevice::ReadOnly));
    QFile targetSystemConfigFile{files.at(0).absoluteFilePath()};
    QVERIFY(targetSystemConfigFile.open(QIODevice::ReadOnly));
    QCOMPARE(sourceSystemFile.readAll(), targetSystemConfigFile.readAll());

    QFile sourceViConfigFile{sourceViConfig};
    QVERIFY(sourceViConfigFile.open(QIODevice::ReadOnly));
    QFile targetViConfigFile{files.at(1).absoluteFilePath()};
    QVERIFY(targetViConfigFile.open(QIODevice::ReadOnly));
    QCOMPARE(sourceViConfigFile.readAll(), targetViConfigFile.readAll());
    
    QFile sourceCalibrationGridFile{sourceCalibrationGrid};
    QVERIFY(sourceCalibrationGridFile.open(QIODevice::ReadOnly));
    QFile targetCalibrationGridFile{files.at(2).absoluteFilePath()};
    QVERIFY(targetCalibrationGridFile.open(QIODevice::ReadOnly));
    QCOMPARE(sourceCalibrationGridFile.readAll(), targetCalibrationGridFile.readAll());
}


void HardwareConfigurationBackupModelTest::testBackupScanmasterFiles()
{
    HardwareConfigurationBackupModel model;
    QTemporaryDir backupDir;
    QTemporaryDir configDir;
    model.setBackupDir(backupDir.path());
    model.setConfigDir(configDir.path());

    const QString sourceViConfig{QFINDTESTDATA("../../../../wm_inst/config_templates/VI_Config.xml")};
    const QString sourceSystemConfig{QFINDTESTDATA("../../../../wm_inst/config_templates/scanmaster/SystemConfig.xml")};
    const QString sourceCalibrationConfig{QFINDTESTDATA("../../../../Analyzer_Interface/autotests/testdata/coax/config/calibrationData0.xml")};
    const QString sourceCameraCorrectionGrid{QFINDTESTDATA("testdata/config/correctionGrid0.csv")};
    const QString sourceIDMCorrectionGrid{QFINDTESTDATA("testdata/config/IDM_correctionGrid0.csv")};
    const QString sourceIDMConfig{QFINDTESTDATA("testdata/config/OCT.xml")};

    std::vector<QString> sourceConfigFiles {
        sourceIDMCorrectionGrid,
        sourceIDMConfig,
        sourceSystemConfig,
        sourceViConfig,
        sourceCalibrationConfig,
        sourceCameraCorrectionGrid};

    std::vector<QString> targetFilenames {
        "IDM_correctionGrid0.csv",
        "OCT.xml",
        "SystemConfig.xml",
        "VI_Config.xml",
        "calibrationData0.xml",
        "correctionGrid0.csv"};
    QCOMPARE(targetFilenames.size(), sourceConfigFiles.size());

    for (unsigned int i = 0; i < sourceConfigFiles.size(); i++)
    {
        QVERIFY(QFile::copy(sourceConfigFiles.at(i), configDir.filePath(targetFilenames.at(i))));
    }

    QSignalSpy backupSucceededSpy{&model, &HardwareConfigurationBackupModel::backupSucceeded};
    QVERIFY(backupSucceededSpy.isValid());
    QCOMPARE(model.rowCount(), 0);
    model.createBackup();
    QCOMPARE(backupSucceededSpy.count(), 1);
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.rowCount(model.index(0, 0)), 0);

    const auto archives = QDir{backupDir.path()}.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    QCOMPARE(archives.size(), 1);
    QCOMPARE(model.index(0, 0).data().toDateTime(), QDateTime::fromString(archives.first().fileName(), QStringLiteral("yyyyMMdd-HHmm")));
    const auto files = QDir{archives.first().filePath()}.entryInfoList(QDir::Files, QDir::Name);
    QCOMPARE(files.size(), 7);

    for (int i = 0; i < files.size() - 1; i++) //systemHardwareConfiguration check separately
    {
        auto & file = files.at(i);
        QCOMPARE(file.fileName(), targetFilenames.at(i));
        QCOMPARE(file.isFile(), true);
        QCOMPARE(file.isSymLink(), false);
    }
    QCOMPARE(files.back().fileName(), "systemHardwareConfiguration.json");
    QCOMPARE(files.back().isFile(), true);
    QCOMPARE(files.back().isSymLink(), false);

    QCOMPARE( sourceConfigFiles.size(), files.size() -1 ); //do not check systemHardwareConfiguration
    for (unsigned int i = 0; i < sourceConfigFiles.size(); i++)
    {
        auto & sourceConfig = sourceConfigFiles[i];
        QFile sourceConfigFile{sourceConfig};
        QVERIFY(sourceConfigFile.open(QIODevice::ReadOnly));
        QFile targetConfigFile{files.at(i).absoluteFilePath()};
        QVERIFY(targetConfigFile.open(QIODevice::ReadOnly));
        QCOMPARE(sourceConfigFile.readAll(), targetConfigFile.readAll());
    }
}

void HardwareConfigurationBackupModelTest::testRestoreFailsInvalidRow()
{
    HardwareConfigurationBackupModel model;
    QTemporaryDir backupDir;
    QVERIFY(QDir{backupDir.path()}.mkdir(QStringLiteral("20190701-1200")));
    model.setBackupDir(backupDir.path());
    QCOMPARE(model.rowCount(), 1);
    QTemporaryDir configDir;
    model.setConfigDir(configDir.path());

    QSignalSpy restoreFailedSpy{&model, &HardwareConfigurationBackupModel::restoreFailed};
    QVERIFY(restoreFailedSpy.isValid());
    model.restore(1);
    QCOMPARE(restoreFailedSpy.count(), 1);
}

void HardwareConfigurationBackupModelTest::testRestoreFailsInvalidConfigDir()
{
    HardwareConfigurationBackupModel model;
    QTemporaryDir backupDir;
    QVERIFY(QDir{backupDir.path()}.mkdir(QStringLiteral("20190701-1200")));
    model.setBackupDir(backupDir.path());
    QCOMPARE(model.rowCount(), 1);

    QSignalSpy restoreFailedSpy{&model, &HardwareConfigurationBackupModel::restoreFailed};
    QVERIFY(restoreFailedSpy.isValid());
    // no config dir yet
    model.restore(0);
    QCOMPARE(restoreFailedSpy.count(), 1);

    // config dir doesn't exist
    model.setConfigDir(QStringLiteral("invalid"));
    model.restore(0);
    QCOMPARE(restoreFailedSpy.count(), 2);
}

QTEST_GUILESS_MAIN(HardwareConfigurationBackupModelTest)
#include "hardwareConfigurationBackupModelTest.moc"
