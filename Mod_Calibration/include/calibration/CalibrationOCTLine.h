/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		LB
 * 	@date		2019
 * 	@brief	Calibration OCT - Single scan line
 */

#ifndef CALIBRATIONOCTLINE_H
#define CALIBRATIONOCTLINE_H

#include <message/calibration.interface.h>
#include <calibration/calibrationProcedure.h>
#include <calibration/CalibrationOCTConfigurationIDM.h>
#include "calibration/CalibrationOCTMeasurementModel.h"
#include "calibration/CalibrationOCTLineParameters.h"
#include <image/image.h>
#include <overlay/color.h>


namespace precitec
{
namespace calibration
{


class CalibrationOCTLine : public CalibrationProcedure
{
public:
    
    /**
    * @brief CTor.
    * @param p_rCalibrationManager Reference to the calibration manager.
    */
    CalibrationOCTLine ( CalibrationManager& p_rCalibrationManager );

    ~ CalibrationOCTLine();

    /**
    * Execute calibration procedure
    */
    virtual bool calibrate();
    
private:
    struct ProfileFeatures {
        ////////////////////////////////////////////
        // debug variables
        ///////////////////////////////////////////
        int m_oCandidateXLeft;
        int m_oCandidateXRight;


        ///////////////////////////////////////////////////////////
        // results
        ///////////////////////////////////////////////////////////
        bool valid;
        int m_oXLeft;
        int m_oXGap;
        int m_oZTop;
        int m_oZGap;
        int numMeasurements;
    };
    
    
    static ProfileFeatures computeProfileFeatures ( const precitec::image::Sample & rSample, const CalibrationOCTLineParameters & rParameters );
    static CalibrationOCTMeasurementModel computeModel ( const std::vector<CalibrationOCTConfigurationIDM> & r_configurations, const std::vector<ProfileFeatures> & r_profileFeatures,
            double p_oGapWidth_mm, double p_oGapHeigth_mm );


    geo2d::Point coordinatesForPaint ( int x, int y, int x_offset, int index_subplot, int num_subplot );

    image::Color chooseColor ( int index ) const;
    void showInfoConfiguration ( const CalibrationOCTConfigurationIDM & rConfiguration,  int index, int total_subplots );
    void showInfoProfile ( const precitec::image::Sample & rSample, const ProfileFeatures & rProfileFeatures,  int index, int total_subplots );

    int m_oRepetitions;
    int m_yMagnify;
    
    CalibrationOCTLineParameters m_profileRecognitionParameters;

};

} // namespace calibration
} // namespace precitec

#endif 
