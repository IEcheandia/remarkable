#include "common/calibrationConfiguration.h"

namespace {
    void formatFolderFilename(std::string & p_oName)
    {
        if (p_oName.empty())
        {
            return;
        }
        if (p_oName.substr(p_oName.length()-1, 1) != "/")
        {
            p_oName += "/";
        }
        return;
    }
}

namespace precitec {
namespace coordinates{
    

CalibrationConfiguration::CalibrationConfiguration(const int p_oSensorID,  const std::string p_oWMBaseDir)
: m_oSensorID(p_oSensorID) 
, m_oHomeDir(p_oWMBaseDir)
, m_oConfigFile(getConfigFilename(m_oSensorID,p_oWMBaseDir))
{
    formatFolderFilename(m_oHomeDir);
}
    
// Generates calibration filename for coax/fallback and/or Scheimpflulg data
std::string CalibrationConfiguration::generateImgFilename(const int p_oSensorID, const std::string p_oPrefix, const int p_oNum)
{
	std::string oC("XYZ"); 
	std::string oNum("0123"); 
	std::string oSen("012");
	std::string oFilename("");
	oFilename = p_oPrefix + oSen.substr(p_oSensorID, 1) + oC.substr(p_oNum/4, 1) + oNum.substr(p_oNum%4, 1) + std::string(".bmp");  // todo: encode sensor in name
	return oFilename;
}

std::string CalibrationConfiguration::toWMCalibFile(const std::string p_oFile, const std::string p_oWMBaseDir)
{
	std::string oTheFile = p_oWMBaseDir; //pwdToStr();
	formatFolderFilename(oTheFile);
	oTheFile += "calib/";
	oTheFile += p_oFile;
	return oTheFile;
}

std::string CalibrationConfiguration::configFilename(const int p_oSensorID, const std::string extension, const std::string p_oWMBaseDir)
{
    // build location of configuration file. e.g  configFilename(0,".xml","/opt/wm_inst") ->  /opt/wm_inst/config/calibrationData0.xml
    // location of configuration file
	std::string oConfigFile = p_oWMBaseDir;
    formatFolderFilename(oConfigFile);
	oConfigFile += std::string("config/calibrationData");
    oConfigFile += std::to_string(p_oSensorID);
    oConfigFile += extension;
    return oConfigFile;

}

std::string CalibrationConfiguration::getConfigFilename(const int p_oSensorID, const std::string p_oWMBaseDir)
{
    return configFilename(p_oSensorID, ".xml", p_oWMBaseDir);
    
}

std::string CalibrationConfiguration::getCamGridDataBinaryFilename() const
{
    // e.g /opt/wm_inst/calib/calibImgData0.dat
    // (NB this file is in the calib folder because it's not meant for backup, it will be overwritten when the camera changes)
    return toWMCalibFile( "calibImgData" + std::to_string(m_oSensorID)+".dat" , m_oHomeDir);
}

std::string CalibrationConfiguration::getCopyCSVFilename() const
{
    
    return  m_oHomeDir + "/config/calibImgData" + std::to_string(m_oSensorID)+ "copy.csv" , m_oHomeDir;
}


std::string CalibrationConfiguration::getCSVFallbackFilename() const
{
    return  m_oHomeDir + "/config/" + getCSVFallbackBasename(m_oSensorID);
}

std::string CalibrationConfiguration::getCSVFallbackFilenameInCalibFolder() const
{
    //e.g.  /opt/wm_inst/calibImgData0fallback.csv
    return toWMCalibFile( "calibImgData" + std::to_string( m_oSensorID )+"fallback.csv", m_oHomeDir );
}


std::string CalibrationConfiguration::getCalibFolder() const
{
    return toWMCalibFile ( "",m_oHomeDir );
}

std::string CalibrationConfiguration::getCameraCorrectionGridFilename() const
{
    return  m_oHomeDir + "/config/correctionGrid" + std::to_string(m_oSensorID)+ ".csv";
}

std::string CalibrationConfiguration::getIDMCorrectionGridFilename() const
{
    return  m_oHomeDir + "/config/IDM_correctionGrid" + std::to_string(m_oSensorID)+ ".csv";
}

std::string CalibrationConfiguration::getLastFailedLineCalibrationFilename() const
{
    return  m_oHomeDir + "/config/lastFailedLineCalibration" + std::to_string(m_oSensorID)+ ".bmp";
}



}

}
