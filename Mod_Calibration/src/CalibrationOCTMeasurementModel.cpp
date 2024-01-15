#include "calibration/CalibrationOCTMeasurementModel.h"

namespace precitec {
namespace calibration {
    
CalibrationOCTMeasurementModel::CalibrationOCTMeasurementModel()
    : m_oInitialized(false)
    {}
    

CalibrationOCTMeasurementModel::CalibrationOCTMeasurementModel ( CalibrationOCTConfigurationIDM p_oReferenceConfiguration, geo2d::Point p_oReferencePosition, int p_oReferenceNumberMeasurements, double p_oReferenceLateralResolution, double p_oReferenceDepthResolution )
: m_oReferenceConfiguration(p_oReferenceConfiguration)
     ,m_oReferencePosition(p_oReferencePosition)
     ,m_oReferenceNumberMeasurements(p_oReferenceNumberMeasurements)
     ,m_oReferenceLateralResolution(p_oReferenceLateralResolution)
     ,m_oReferenceDepthResolution(p_oReferenceDepthResolution)
     ,m_oInitialized(true)
{}


double CalibrationOCTMeasurementModel::getLateralResolution ( const CalibrationOCTConfigurationIDM& r_configuration ) const
{
    //in this model lineWidth and reference position are not used
    return getNewsonSpeedCalibrated ( r_configuration.get ( eMarkSpeed ) ) / double ( r_configuration.get ( eSamplingFrequency ) );
}


double CalibrationOCTMeasurementModel::getDepthResolution() const
{
    //in this model lineWidth and reference position are not used
    return m_oReferenceDepthResolution;
}


int CalibrationOCTMeasurementModel::getNumberOfMeasurements ( const CalibrationOCTConfigurationIDM& r_configuration ) const
{
    return double ( r_configuration.get ( eScanWidth ) ) * m_oReferenceNumberMeasurements /  double ( m_oReferenceConfiguration.get ( eScanWidth ) );
}


double CalibrationOCTMeasurementModel::getLateralDistanceFromScanLineCenter ( int indexFromScanLineCenter, const CalibrationOCTConfigurationIDM& r_configuration ) const
{
    //in this model lineWidth and reference position are not used
    return indexFromScanLineCenter * getLateralResolution ( r_configuration );
}


double CalibrationOCTMeasurementModel::getDepth ( int pixelFromIDMProfile, const CalibrationOCTConfigurationIDM& r_configuration ) const
{
    //in this model lineWidth and reference position are not used
    return double ( pixelFromIDMProfile ) * getDepthResolution() * double ( r_configuration.get ( eRescaleIDMValue ) );
}


std::tuple< double, double > CalibrationOCTMeasurementModel::getEquivalentCoaxCalibration ( const CalibrationOCTConfigurationIDM& r_configuration, double p_dPixX, double p_dPixY ) const
{
    // coax parameters are defined as follows:
    // RatioHorizontal_pix_mm =  oCoaxData.m_oBeta0/oCoaxData.m_oDpixX
    // RatioVertical_pix_mm = oCoaxData.m_oBetaZ/oCoaxData.m_oDpixY
    
    // in this model the resolution is computed with um instead  of mm, so it need to be divided by 1000
    double beta0 =  p_dPixX * 1e3/ getLateralResolution ( r_configuration ) ;
    double betaZ = p_dPixY * 1e3/ double(r_configuration.get ( eRescaleIDMValue )*getDepthResolution());
    return {beta0, betaZ};
}


std::tuple< double, double > CalibrationOCTMeasurementModel::getEquivalentSampling ( const CalibrationOCTConfigurationIDM& r_configuration ) const
{
    return {1000 / getLateralResolution ( r_configuration ), 1000.0 * r_configuration.get ( eRescaleIDMValue ) / getDepthResolution()  };
}


geo2d::DPoint CalibrationOCTMeasurementModel::getEquivalentCoordinates ( int indexFromScanLineCenter, int pixelFromIDMProfile, const CalibrationOCTConfigurationIDM& r_configuration, 
                                                                         double p_Beta0, double p_BetaZ, double p_dPixX, double p_dPixY ) const
{
    double x_mm = getLateralDistanceFromScanLineCenter ( indexFromScanLineCenter, r_configuration );
    double z_mm = getDepth ( pixelFromIDMProfile, r_configuration );

    double oRatioHorizontal_pix_mm =  p_Beta0/p_dPixX;
    double oRatioVertical_pix_mm = p_BetaZ/p_dPixY;
    return { x_mm * oRatioHorizontal_pix_mm, z_mm * oRatioVertical_pix_mm};
}


bool CalibrationOCTMeasurementModel::isInitialized() const
{
    return m_oInitialized;
}


double CalibrationOCTMeasurementModel::getNewsonSpeedCalibrated ( int speed ) const
{
    assert ( m_oReferenceConfiguration.get ( eJumpSpeed ) == m_oReferenceConfiguration.get ( eMarkSpeed ) );

    // GapWidth[um] / (GapWidth[pix]/SampligFreq[Hz] ) = um*hz/pix
    double referenceSpeedCalibrated =  m_oReferenceLateralResolution * m_oReferenceConfiguration.get ( IDM_Parameter::eSamplingFrequency ) ;
    return speed * referenceSpeedCalibrated / double(m_oReferenceConfiguration.get ( IDM_Parameter::eJumpSpeed ));
}

double CalibrationOCTMeasurementModel::getNewsonLineWidthCalibrated ( double lineWidth ) const
{
    double reference_lineWidth_um = m_oReferenceLateralResolution * m_oReferenceNumberMeasurements;
    return reference_lineWidth_um * lineWidth / double ( m_oReferenceConfiguration.get ( eScanWidth ) );
}

double CalibrationOCTMeasurementModel::getNewsonPositionFromDistance (double mm ) const
{
    double reference_lineWidth_mm = m_oReferenceLateralResolution * m_oReferenceNumberMeasurements/1000.0;
    assert(reference_lineWidth_mm != 0);
    return  m_oReferenceConfiguration.get (eScanWidth)  * mm / reference_lineWidth_mm;
}



} //namespace calibration
} //namespace precitec
