/**
 * @file
 * @copyright Precitec Vision GmbH & Co. KG
 * @author LB
 * @date 2019
 * @brief Calibration OCT - Subset of IDM device key-values necessary for calibration
 */

#pragma once

#include <message/device.h> //for interface::Key
#include <geo/array.h>
#include <array>

namespace precitec
{
namespace calibration
{

class CalibrationManager;

enum  IDM_Parameter {
    eJumpSpeed = 0,
    eMarkSpeed,
    eScanWidth,
    eSamplingFrequency,
    eRescaleIDMValue,
    eNumParameters
};

typedef std::array<int, IDM_Parameter::eNumParameters> IDM_Values_t;

class CalibrationOCTConfigurationIDM
{
public:

    static const std::array<interface::Key, IDM_Parameter::eNumParameters> IDM_Keys;
    CalibrationOCTConfigurationIDM ( IDM_Values_t p_oConfiguration, geo2d::DPoint p_oOCTScanCenter);
    CalibrationOCTConfigurationIDM();

    static int validateConfiguration ( IDM_Values_t & p_rConfig );

    bool loadCurrentIDMConfiguration ( CalibrationManager & p_rCalibMgr, bool setToValid = true );
    bool configureIDM ( CalibrationManager & p_rCalibMgr ) const;
    bool configureIDM ( CalibrationManager & p_rCalibMgr, const CalibrationOCTConfigurationIDM & cachedConfiguration ) const;
    int get ( IDM_Parameter i ) const;
    const IDM_Values_t & getConfiguration() const;
    geo2d::DPoint getOCTScanCenter() const;
    void updateOCTScanCenterInConfiguration(geo2d::DPoint p_oScanCenter);
    void updateParameterInConfiguration(IDM_Parameter p_parameter, int p_value);

private:

    static void setIDM_IntValue ( CalibrationManager & p_rCalibMgr, IDM_Parameter p_parameter, int p_value );
    static void setIDMScanCenterX( CalibrationManager & p_rCalibMgr, double p_value );
    static void setIDMScanCenterY( CalibrationManager & p_rCalibMgr, double p_value );

    IDM_Values_t m_oConfiguration;
    geo2d::DPoint m_oOCTScanCenter = {0.0, 0.0}; //not in IDM_Values_t because it's a double, not an int value

};

}// calibration
}// precitec
