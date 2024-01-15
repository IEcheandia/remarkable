#include <QTest>

#include "../include/calibration/calibrationManager.h"
#include "../include/calibration/CalibrationOCTData.h"
#include "../include/calibration/CalibrationOCTLineParameters.h"

using precitec::calibration::CalibrationOCTData;
using precitec::calibration::CalibrationOCTMeasurementModel;
using precitec::calibration::CalibrationOCTConfigurationIDM;
using precitec::Types;

// dummy implementation of CalibrationManager
namespace precitec
{
namespace calibration
{

    interface::SmpKeyValue CalibrationManager::getIDMKeyValue(precitec::interface::Key p_key, int p_subDevice) 
    { 
        return precitec::interface::SmpKeyValue{};
    }
    interface::KeyHandle CalibrationManager::setIDMKeyValue ( interface::SmpKeyValue p_keyValue, int p_subDevice ) 
    {
        return interface::KeyHandle{};
    }

    math::CalibrationData& CalibrationManager::getCalibrationData(unsigned int p_oSensorId)
    {
        return m_oCalData[math::SensorId(p_oSensorId)];
    } 
    void CalibrationManager::sendCalibDataChangedSignal(int p_oSensorID, bool p_oInit)
    {

    }

}
}

class testCalibrationOCTData : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testDefaultCtor();
    void testNoConfigFile();
    void testReadWrite();
    void testLoadConfiguration();
};



void testCalibrationOCTData::testDefaultCtor()
{
    CalibrationOCTData calibData;
    QVERIFY(!calibData.isValid());
}

void testCalibrationOCTData::testNoConfigFile()
{
    CalibrationOCTData calibData;
    bool fileOK = calibData.load("InexistentFolder");
    QVERIFY(!fileOK);
    
    //we didn't provide a config file, we load the default values
    QVERIFY(calibData.isValid());
}



void testCalibrationOCTData::testReadWrite()
{
    QTemporaryDir dir;

    Poco::File oTestFolder(dir.path().toStdString() + "/testCalibrationOCTData");
    
    // folder does not exist, load default values
    QVERIFY(!oTestFolder.exists());
    CalibrationOCTData calibDataDefault;
    calibDataDefault.load(oTestFolder.path());
    QVERIFY(calibDataDefault.isValid());
    
    //write to empty directory
    bool ok = calibDataDefault.write(oTestFolder.path());
    QVERIFY(ok);
    QVERIFY(oTestFolder.exists());
    
    //load from a valid file
    CalibrationOCTData calibDataFromFile = calibDataDefault;
    calibDataFromFile.load(oTestFolder.path());
    QVERIFY(calibDataFromFile.isValid());
    
    auto expectedParameters  = calibDataDefault.getParametersFromMeasurementModel();
    auto actualParameters  = calibDataFromFile.getParametersFromMeasurementModel();

    QCOMPARE(actualParameters.size(), CalibrationOCTData::Parameter::NumParameters);
    QCOMPARE(expectedParameters.size(), CalibrationOCTData::Parameter::NumParameters);
    for (unsigned int i = 0; i < actualParameters.size() ; ++i)
    {
        QCOMPARE(actualParameters[i]->toString(),  expectedParameters[i]->toString());
    }
        
    //overwrite existing file
    bool overwriteOk = calibDataDefault.write(oTestFolder.path());
    QVERIFY(overwriteOk);
    
    //load the file again 
    CalibrationOCTData calibDataFromFile2;
    bool reload_ok = calibDataFromFile2.load(oTestFolder.path());
    QVERIFY(reload_ok);
    auto actualParameters2  = CalibrationOCTData::getParametersFromModel(calibDataFromFile.m_oOCTMeasurementModel);
    QCOMPARE(actualParameters2.size(), CalibrationOCTData::Parameter::NumParameters);
    for (unsigned int i = 0; i < actualParameters2.size() ; ++i)
    {
        QCOMPARE(actualParameters2[i]->toString(),  expectedParameters[i]->toString());
    }
    
}



void testCalibrationOCTData::testLoadConfiguration()
{
    Poco::AutoPtr<Poco::Util::XMLConfiguration> pConf {new Poco::Util::XMLConfiguration};
    pConf->loadEmpty("key_value_congfiguration");
    
    //this configuration is incomplete and corrupt
    pConf->setInt("OCT_Line_MinNumValuesPerLayer", 1);
    pConf->setDouble("OCT_Line_MaxRangePerLayer", 2.1);
    pConf->setDouble("OCT_Line_MinJumpPixel", 3.5); //should have been int
    pConf->setInt("OCT_Line_MaxWidthPixel", 4);
    pConf->setDouble("OCT_Line_GapWidth", 5.1);
    pConf->setDouble("OCT_Line_GapHeight", 6.1); 
    pConf->setInt( "Reference_Speed",7);
    pConf->setDouble("Reference_DepthResolution", 8.1);
    pConf->setDouble("X_TCP_Newson", 9.1); 
    pConf->setDouble("Y_TCP_Newson", 10.1); 
    pConf->setDouble("DesiredDistanceFromTCP", 11.1); 

        
    QTemporaryDir dir;
    Poco::File oTestFolder(dir.path().toStdString() + "/testCalibrationOCTData/");
    Poco::File oTestConfig(oTestFolder.path()+"/config/");
    Poco::File oTestXML(oTestConfig.path()+"/"+CalibrationOCTData::m_oConfigFilename);
    
    oTestConfig.createDirectories();
    QVERIFY(oTestConfig.exists());

    pConf->save(oTestXML.path());
    QVERIFY(oTestXML.exists());

    
    CalibrationOCTData oParams;
    bool read = oParams.load(oTestFolder.path());
    QVERIFY(read);
    QVERIFY(oParams.isValid());

    
    QCOMPARE(oParams.getKeyValue("OCT_Line_MinNumValuesPerLayer")->value<int>(), 1);
    QCOMPARE(oParams.getKeyValue("OCT_Line_MaxRangePerLayer")->value<double>(), 2.1);
    //MinJumpPixel was of the wrong type
    QCOMPARE(oParams.getKeyValue("OCT_Line_MaxWidthPixel")->value<int>(), 4);
    QCOMPARE(oParams.getKeyValue("OCT_Line_GapWidth")->value<double>(), 5.1);
    QCOMPARE(oParams.getKeyValue("OCT_Line_GapHeight")->value<double>(), 6.1);
    QCOMPARE(oParams.getKeyValue("Reference_Speed")->value<int>(), 7);
    QCOMPARE(oParams.getKeyValue("Reference_DepthResolution")->value<double>(), 8.1);
    QCOMPARE(oParams.getKeyValue("X_TCP_Newson")->value<double>(), 9.1);
    QCOMPARE(oParams.getKeyValue("Y_TCP_Newson")->value<double>(), 10.1);
    QCOMPARE(oParams.getKeyValue("DesiredDistanceFromTCP")->value<double>(), 11.1);

    
}


QTEST_GUILESS_MAIN(testCalibrationOCTData)

#include "testCalibrationOCTData.moc"
