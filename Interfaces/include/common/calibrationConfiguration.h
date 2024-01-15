/*!
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			LB
 *  @date			2018
 *  @file
 *  @brief			Common definition for calibration coordinates
 */

#ifndef CALIBRATIONCONFIGURATION_H
#define CALIBRATIONCONFIGURATION_H

#include "InterfacesManifest.h"
#include <string>
#include <vector>
#include <cassert>

namespace precitec {
namespace coordinates{

/**
 * @brief   Generates and returns calibration image filenames for transmission to windows.
 */


class INTERFACES_API CalibrationConfiguration
{

public:
	static std::string toWMCalibFile(const std::string p_oFile, const std::string p_oWMBaseDir);
	static std::string getCSVFallbackBasename(int p_oSensorID){ return "calibImgData" + std::to_string(p_oSensorID)+ "fallback.csv";}

	CalibrationConfiguration(const int p_oSensorID,  const std::string p_oWMBaseDir);

	const std::string & getHomeDir() const {return m_oHomeDir;}
	const std::string & getConfigFilename() const { return m_oConfigFile;}
	std::string getCalibFolder() const;

	//get paths  associated to camgriddata (see getCamGridDataFromCamera for their usage)
	std::string getCamGridDataBinaryFilename() const;
	std::string getCopyCSVFilename() const;
	std::string getCSVFallbackFilename() const;
	std::string getCSVFallbackFilenameInCalibFolder() const; //back-compatibility

	//get paths  associated to scanmaster correctiongrid
	std::string getCameraCorrectionGridFilename() const;
    std::string getIDMCorrectionGridFilename() const;

    //get paths associate to the calibration procedure debug
    std::string getLastFailedLineCalibrationFilename() const;

private:
    static std::string generateImgFilename(const int p_oSensorID, const std::string p_oPrefix, const int p_oNum);
    static std::string getConfigFilename(const int p_oSensorID, const std::string p_oWMBaseDir);
    ///< XML configuration file, where all the parameters are stored permanently.
    static std::string configFilename(const int p_oSensorID, const std::string extension, const std::string p_oWMBaseDir);

    int m_oSensorID;
    std::string m_oHomeDir;
    std::string m_oConfigFile;
};

} //coordinates
} //precitec
#endif
