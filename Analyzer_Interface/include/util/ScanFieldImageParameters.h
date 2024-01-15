#ifndef SCANFIELDIMAGEPARAMETERS_H
#define SCANFIELDIMAGEPARAMETERS_H

#include "image/image.h"
#include "module/moduleLogger.h"
#include "util/CalibrationScanMasterData.h"

#include "message/device.h"
#include "Poco/Util/AbstractConfiguration.h"

namespace precitec{
namespace calibration{    

struct ScanFieldGridParameters
{
    struct ScannerPosition 
    {
        double x; double y; int row; int column;
    };
    typedef std::vector<ScannerPosition> ScannerPositions;
    
    double xMin;
    double xMax;
    double yMin;
    double yMax;
    double deltaX;
    double deltaY;
    
    bool includesOrigin() const;
    void forceDelta(double delta);
    void forceSymmetry();
    void forceAcquireOnOrigin();
    int numCols() const;
    int numRows() const;
    double xRange() const;
    double yRange() const;
    ScannerPositions computeScannerPositions(bool minimizeJump) const;
};

struct ScanFieldImageParameters
{       
    enum key_id{
        e_xMinLeft_mm, 
        e_yMinTop_mm, 
        e_Width,
        e_Height, 
        NUM_IDs
    };     
    static const std::array<std::string, key_id::NUM_IDs> s_keys;
    
    double m_xMinLeft_mm = 0.0;
    double m_yMinTop_mm = 0.0;
    image::Size2d m_scanfieldimageSize = {0, 0};
    ScanMasterCalibrationData m_ScanMasterData;
    
    interface::Configuration makeConfiguration() const;
    bool saveToIni(std::string filename) const;;
    interface::SmpKeyValue getKeyValue(std::string p_key) const;
    interface::KeyHandle setKeyValue(interface::SmpKeyValue p_oSmpKeyValue);
    void load(const Poco::Util::AbstractConfiguration * pConfIn);

    
    static ScanFieldImageParameters computeParameters(const ScanMasterCalibrationData & rScanMasterCalibrationData,
        image::Size2d imageSize, ScanFieldGridParameters gridParameters);
    static ScanFieldImageParameters computeParameters(const ScanMasterCalibrationData & rScanMasterCalibrationData,
        int imageWidth, int imageHeight,  double xMin, double xMax, double yMin, double yMax);
    static ScanFieldImageParameters computeParameters(const math::CalibrationParamMap & rCalibrationParameters);
    static ScanFieldImageParameters loadConfiguration(interface::Configuration p_configuration);
    
    // scanner -> scanfield image
    geo2d::DPoint getCenterInScanFieldImage(double xCenter_mm, double yCenter_mm) const;
    geo2d::DPoint getCenterInScanFieldImageBeforeMirroring(double xCenter_mm, double yCenter_mm) const;
    geo2d::Point getTopLeftCornerInScanFieldImageBeforeMirroring(const precitec::image::Size2d & rLocalImageSize, double xCenter_mm, double yCenter_mm) const;

    // scanfield image -> scanner
    geo2d::DPoint getScannerPositionFromScanFieldImage(double xCenter_pix, double yCenter_pix) const;
    
    friend std::ostream& operator << ( std::ostream& os, const ScanFieldImageParameters& rValue )        
    {
        os << "Min x " << rValue.m_xMinLeft_mm << ", " << rValue.m_yMinTop_mm << 
        " Xmm_to_Pixel " << rValue.m_ScanMasterData.m_XmmToPixel << 
        " Ymm_to_Pixel " << rValue.m_ScanMasterData.m_YmmToPixel << 
        " slope " << rValue.m_ScanMasterData.m_Slope << 
        " scanfieldimagesize " <<  rValue.m_scanfieldimageSize;
        return os;
    }
    
};


}
}
#endif
