/**
 * @file
 * @copyright Precitec Vision GmbH & Co. KG
 * @author LB
 * @date 2019
 * @brief Calibration OCT - 2D scan for TCP search
 */

#ifndef CALIBRATIONOCT_TCP_H
#define CALIBRATIONOCT_TCP_H

#include <message/calibration.interface.h>
#include <calibration/calibrationProcedure.h>
#include <calibration/CalibrationOCTConfigurationIDM.h>
#include "calibration/CalibrationOCTMeasurementModel.h"
#include <image/image.h>
#include <overlay/color.h>

//forward declaration
class TestCalibrationOCT_TCP; 

namespace precitec
{
namespace calibration
{

class CalibrationOCT_TCP : public CalibrationProcedure
{
public:

    CalibrationOCT_TCP ( CalibrationManager& p_rCalibrationManager );
    ~ CalibrationOCT_TCP();

    bool calibrate() override;

    geo2d::DPoint newsonToPixelScan2D(geo2d::DPoint p_PositionNewson) const;
    geo2d::DPoint pixelScan2DtoNewson(geo2d::DPoint p_PositionImage) const;
    double scanWidth_CanvasImage( CalibrationOCTConfigurationIDM p_oOCTConfigurationScan1D ) const;
    
    bool m_oScanWideArea;

private:
    struct NormalizedImage
    {
        image::BImage image;
        int leftBorder;
        int topBorder;
        int validWidth;
        int validHeight;
        double scaleX = 1.0;
        double scaleY;
        bool normalize(const image::BImage & r_ScanImage, unsigned int lengthScanLine, unsigned int numScanLines, geo2d::Size outputSize);
        bool normalize(const std::vector<image::Sample> & r_samples, double scaleY, unsigned int numScanLines, int minIntensity, int maxIntensity, geo2d::Size outputSize);

    };
    
    
    // IDM device 
    void setTCPSearchLoopCount( int p_oValue);
    int getTCPSearchLoopCount() const;
    void setRescaleFactor( int p_oValue);
    int getRescaleFactor() const;
    double getScanCenterX() const;
    double getScanCenterY() const;
    void setScanCenter(double x, double y);
    double getDesiredDistanceFromTCP() const;
    geo2d::DPoint getTCPNewson() const;
    
    
    NormalizedImage m_oCanvasImage;
    CalibrationOCTConfigurationIDM m_oScanOCTConfiguration;
    unsigned int m_numPointsScanLine = 0;
    unsigned int m_numScanLines = 0;
    double m_NewsonToPixel2D = 0;
    geo2d::DPoint m_CenterCanvasImage {0,0};
    
};

} // namespace calibration
} // namespace precitec


#endif // CALIBRATIONOCT_TCP_H
