/**
 * @file
 * @copyright	Precitec Vision GmbH & Co. KG
 * @author		LB
 * @date		2019
 * @brief	CalibrationOCTMeasurementModel - Converts between mm and IDMProfileSource coordinates 
 */

#pragma once
#include "geo/point.h"
#include <calibration/CalibrationOCTConfigurationIDM.h>

namespace precitec
{
namespace calibration
{


class CalibrationOCTMeasurementModel
{
    friend class CalibrationOCTData; // getParametersFromModel
public:

    CalibrationOCTMeasurementModel();
    CalibrationOCTMeasurementModel ( CalibrationOCTConfigurationIDM p_oReferenceConfiguration,
                          geo2d::Point p_oReferencePosition,
                          int p_oReferenceNumberMeasurements,
                          double p_oReferenceLateralResolution,
                          double p_oReferenceDepthResolution
                        );
    
    //compute lateral resolution for the current configuration [um / (sample index from center)]
    double getLateralResolution ( const CalibrationOCTConfigurationIDM & r_configuration ) const;
    
    //compute lateral resolution for the current configuration [um / IDM value]
    double getDepthResolution () const;
    
    int getNumberOfMeasurements ( const CalibrationOCTConfigurationIDM & r_configuration ) const;
    
    double getLateralDistanceFromScanLineCenter ( int indexFromScanLineCenter, const CalibrationOCTConfigurationIDM & r_configuration ) const;
    double getDepth ( int pixelFromIDMProfile, const CalibrationOCTConfigurationIDM & r_configuration ) const;
    
    // get the coax calibration (beta0, betaz) that gives the correct measurements on the laser line from the IDM source profile
    std::tuple<double, double> getEquivalentCoaxCalibration ( const CalibrationOCTConfigurationIDM & r_configuration, double p_dPixX, double p_dPixY ) const;
    std::tuple<double, double> getEquivalentSampling ( const CalibrationOCTConfigurationIDM & r_configuration ) const;
    geo2d::DPoint getEquivalentCoordinates ( int indexFromScanLineCenter, int pixelFromIDMProfile,
            const CalibrationOCTConfigurationIDM & r_configuration,
            double p_Beta0, double p_BetaZ, double p_dPixX, double p_dPixY ) const;
    
    bool isInitialized() const;

    double getNewsonSpeedCalibrated ( int speed ) const; //from Newson unit to [um/s];
    double getNewsonLineWidthCalibrated ( double lineWidth ) const; //from Newson unit to [um/s];
    double getNewsonPositionFromDistance (double mm ) const; // from um to Newson unit
    
private:
    
    // configuration used for calibration
    CalibrationOCTConfigurationIDM m_oReferenceConfiguration;
    geo2d::Point m_oReferencePosition; // [index from scan center, IDM value]
    int m_oReferenceNumberMeasurements;
    double m_oReferenceLateralResolution;
    double m_oReferenceDepthResolution;
    bool m_oInitialized;
    
};


}// calibration
}// precitec
