#pragma once
#include <array>
#include <string>
#include "message/device.h"
#include "math/CalibrationParamMap.h"
#include "Poco/Util/AbstractConfiguration.h"
#include "image/image.h"

namespace precitec {


namespace calibration {

struct ScanMasterCalibrationData
{
    enum key_id{
        e_XmmToPixel, 
        e_YmmToPixel, 
        e_Slope,
        e_MirrorX,
        e_MirrorY,
        NUM_IDs
    };
    static const std::array<std::string, key_id::NUM_IDs> s_keys;
    
    double m_XmmToPixel = -27.0;
    double m_YmmToPixel = 27.0;
    double m_Slope = 0.0;
    bool m_mirrorX = false;
    bool m_mirrorY = false;
    static bool hasMirrorX(image::ImageFillMode mode);
    static bool hasMirrorY(image::ImageFillMode mode) ;
    image::ImageFillMode getImageFillMode() const;
    
    interface::Configuration makeConfiguration() const;
    interface::SmpKeyValue getKeyValue(std::string p_key) const;
    interface::KeyHandle setKeyValue(interface::SmpKeyValue p_oSmpKeyValue);
    void load(const Poco::Util::AbstractConfiguration * pConfIn);
    
    static ScanMasterCalibrationData load(const math::CalibrationParamMap & rCalibrationParamMap);
    

};

}
}
