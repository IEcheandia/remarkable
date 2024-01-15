/**
 * @file
 * @copyright Precitec Vision GmbH & Co. KG
 * @author LB
 * @date 2019
 * @brief Calibration OCT Data- Container of the key-values necessary for calibration procedures and measurements
 */

#pragma once
#include "calibration/CalibrationOCTMeasurementModel.h"
#include "calibration/CalibrationOCTLineParameters.h"
#include "math/calibrationData.h"
#include "Poco/Util/XMLConfiguration.h"

class testCalibrationOCTData;
class testOCTCoordinates;
class testOCTParameters;

namespace precitec
{
namespace calibration
{

class CalibrationOCTData
{    
    
public:
    static const std::string m_oConfigFilename;
    static const std::string s_x_tcp_newson;
    static const std::string s_y_tcp_newson;
    static const std::string s_distanceTCP;
    
    CalibrationOCTData();
    CalibrationOCTData ( CalibrationOCTMeasurementModel p_OCTMeasurementModel, CalibrationOCTConfigurationIDM p_currentConfiguration );
    void updateCurrentConfiguration ( CalibrationOCTConfigurationIDM p_currentConfiguration );
    void updateMeasurementModel ( CalibrationOCTMeasurementModel p_OCTMeasurementModel);
    
    bool write ( std::string pHomeDir );
    bool load ( std::string pHomeDir );
    bool initializeSystem ( CalibrationManager & p_rCalibMgr ) ;
    bool isValid() const;
    bool equivalentCoaxCalibModified() const { return m_oEquivalentCoaxCalibModified;}
    void computeEquivalentCoaxCalibrationData(math::CalibrationData & rCalibrationData);
    const CalibrationOCTMeasurementModel & getMeasurementModel() const;
    const CalibrationOCTLineParameters & getCalibrationOCTLineParameters() const {return m_oCalibrationOCTLineParameters;}

    interface::Configuration makeConfiguration(bool withReferenceValues=false) const;
    interface::SmpKeyValue getKeyValue(std::string p_key) const;
    interface::KeyHandle setKeyValue(interface::SmpKeyValue p_oSmpKeyValue);
    
 private:
   
    enum Parameter {
        Speed = 0,
        Width,
        SHZ,
        Scale,
        X,
        Y,
        NumMeasurement,
        LateralResolution,
        DepthResolution,
        NumParameters
    };
    
    typedef std::array<interface::SmpKeyValue, int ( Parameter::NumParameters ) > parameters_t;
    static const parameters_t s_DefaultParameters;
    static CalibrationOCTMeasurementModel getModelFromParameters ( parameters_t oCurrentModelConfiguration );;
    static parameters_t getParametersFromModel ( const CalibrationOCTMeasurementModel & oCurrentModel );
    static parameters_t getParametersFromXMLConfiguration(const Poco::Util::XMLConfiguration * pConfIn);

    parameters_t getParametersFromMeasurementModel() const;
    
    CalibrationOCTMeasurementModel m_oOCTMeasurementModel;
    CalibrationOCTConfigurationIDM m_oCurrentConfiguration;
    
    // points position respect to scanner center (same unit as Newson Parameter LineWidth)
    std::array<double,2> m_oTCPNewson = {0.0, 0.0};
    double m_oDesiredDistanceFromTCP = 0.0; //mm
    
    CalibrationOCTLineParameters m_oCalibrationOCTLineParameters;
    bool m_oEquivalentCoaxCalibModified = true;
    
    
    friend testCalibrationOCTData;
    friend testOCTCoordinates;
    friend testOCTParameters;
};

}
}
