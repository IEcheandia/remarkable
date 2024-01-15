/**
 * @file
 * @copyright	Precitec Vision GmbH & Co. KG
 * @author		LB
 * @date		2019
 * @brief	Calibration OCT - Container for the key-values necessary for the line calibration procedure
 */

#pragma once

#include "message/device.h"
#include "Poco/Util/AbstractConfiguration.h"

namespace precitec {
namespace calibration {

    struct CalibrationOCTLineParameters 
    {
        enum key_id{
            e_minNumValuesPerLayer = 0,
            e_maxRangePerLayer,
            e_minJumpPixel,
            e_maxWidthPixel,
            e_gapWidth,
            e_gapHeight,
            NUM_IDs
        };
        static const std::array<std::string, key_id::NUM_IDs> s_keys;
        
        int m_minNumValuesPerLayer = 10;
        double m_maxRangePerLayer = 10.0;
        int m_minJumpPixel = 30; 
        int m_maxWidthPixel = 200;
        double m_gapWidth = 1000; //mm
        double m_gapHeight = 1000; //mm
        
        interface::Configuration makeConfiguration() const;
        interface::SmpKeyValue getKeyValue(std::string p_key) const;
        interface::KeyHandle setKeyValue(interface::SmpKeyValue p_oSmpKeyValue);
        void load(const Poco::Util::AbstractConfiguration * pConfIn);
    };
    
}
}
