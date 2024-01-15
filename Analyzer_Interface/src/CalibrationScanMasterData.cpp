#include "util/CalibrationScanMasterData.h"
#include <assert.h>


namespace precitec {
namespace calibration {

const std::array<std::string, ScanMasterCalibrationData::key_id::NUM_IDs> ScanMasterCalibrationData::s_keys ={
    "SM_scanXToPixel",  "SM_scanYToPixel",  "SM_slopePixel", "SM_mirrorX", "SM_mirrorY"
};


/*static*/ ScanMasterCalibrationData ScanMasterCalibrationData::load(const math::CalibrationParamMap & rCalibrationParamMap)
{
    return 
        {
        rCalibrationParamMap.getDouble(s_keys[e_XmmToPixel]), 
        rCalibrationParamMap.getDouble(s_keys[e_YmmToPixel]), 
        rCalibrationParamMap.getDouble(s_keys[e_Slope]),
        rCalibrationParamMap.getBool(s_keys[e_MirrorX]),
        rCalibrationParamMap.getBool(s_keys[e_MirrorY])
    };
}

image::ImageFillMode ScanMasterCalibrationData::getImageFillMode() const
{
    return m_mirrorX ? 
                m_mirrorY ? 
                    image::ImageFillMode::Reverse 
                    : 
                    image::ImageFillMode::FlippedHorizontal
            : 
                m_mirrorY ? 
                    image::ImageFillMode::FlippedVertical 
                    : 
                    image::ImageFillMode::Direct
            ;
}

bool ScanMasterCalibrationData::hasMirrorX(image::ImageFillMode mode)
{
    switch (mode)
    {
        case image::ImageFillMode::Direct:
        case image::ImageFillMode::FlippedVertical:
            return false;
        case image::ImageFillMode::FlippedHorizontal:
        case image::ImageFillMode::Reverse:
            return true;
    }
    assert(false && "case not handled");
    return false;
}

bool ScanMasterCalibrationData::hasMirrorY(image::ImageFillMode mode)
{
    switch (mode)
    {
        case image::ImageFillMode::Direct:
        case image::ImageFillMode::FlippedHorizontal:
            return false;
        case image::ImageFillMode::FlippedVertical:
        case image::ImageFillMode::Reverse:
            return true;
    }
    assert(false && "case not handled");
    return false;
}


interface::Configuration ScanMasterCalibrationData::makeConfiguration() const
{
    interface::Configuration oConfig;
    for (auto & key: s_keys)
    {
        oConfig.push_back(getKeyValue(key));
    }
    return oConfig;
}
 
interface::SmpKeyValue ScanMasterCalibrationData::getKeyValue ( std::string p_key ) const
{
    // TODO: ensure that the default values are the same as in CalibrationParamMap (AnalyzerInterface)
    auto it = std::find(s_keys.begin(), s_keys.end(), p_key);
    if (it == s_keys.end())
    {
        return {};
    }
    
    key_id id = static_cast<key_id>(std::distance(s_keys.begin(), it));
    
    switch (id)
    {
        case (e_XmmToPixel ):
            return interface::SmpKeyValue { new interface::TKeyValue<double> ( p_key, m_XmmToPixel, -1000, +1000, -27.0, 6)};
        case (e_YmmToPixel ) :
            return interface::SmpKeyValue {new interface::TKeyValue<double> ( p_key, m_YmmToPixel, -1000, +1000, 27.0, 6)};
        case (e_Slope ) :
            return interface::SmpKeyValue {new interface::TKeyValue<double> ( p_key, m_Slope, -1000, +1000, 0.0, 6)};
        case (e_MirrorX):
            return interface::SmpKeyValue {new interface::TKeyValue<bool> ( p_key, m_mirrorX, false, true, false)};
        case (e_MirrorY):
            return interface::SmpKeyValue {new interface::TKeyValue<bool> ( p_key, m_mirrorY, false, true, false)};
        case (NUM_IDs):
            //key not found
            return {};
    }
    return {};
}

interface::KeyHandle ScanMasterCalibrationData::setKeyValue ( interface::SmpKeyValue p_oSmpKeyValue )
{
    assert(!p_oSmpKeyValue.isNull());
    auto it = std::find(s_keys.begin(), s_keys.end(), p_oSmpKeyValue->key());
    
    if (it == s_keys.end())
    {
        wmLog(eDebug, "ScanMasterCalibrationData: Unknown key  %s \n",  p_oSmpKeyValue->key().c_str());
        return {-1};
    }
    
    key_id id = static_cast<key_id>(std::distance(s_keys.begin(), it));
    
    switch(p_oSmpKeyValue->type())
    {
        case(TDouble ): 
        {
            double value = p_oSmpKeyValue->value<double>();
            if ( id == e_XmmToPixel ) 
            {
                m_XmmToPixel = value;
                return {1};
            }
            if ( id == e_YmmToPixel ) 
            {
                m_YmmToPixel = value;
                return {1};
            }
            if ( id == e_Slope ) 
            {
                m_Slope = value;
                return {1};
            }
        }
            break;
        case(TBool):
        {
            bool value = p_oSmpKeyValue->value<bool>();
            if ( id == e_MirrorX)
            {
                m_mirrorX = value;
                return {1};
            }
            if ( id == e_MirrorY)
            {
                m_mirrorY = value;
                return {1};
            }
        }
            break;
        default:
            wmLog(eDebug, "CalibrationOCTLineParameters: Unkwown key  %s of type %d \n",  p_oSmpKeyValue->key().c_str(), int(p_oSmpKeyValue->type()) );
    }
    return {-1};
}
void ScanMasterCalibrationData::load ( const Poco::Util::AbstractConfiguration* pConfIn )
{
    assert ( pConfIn );

    for (auto & key: s_keys)
    {
        auto smpKeyValue = getKeyValue(key);
        
        try 
        {
            switch ( smpKeyValue->type() ) 
            {
                case TDouble:
                    smpKeyValue->setValue ( pConfIn->getDouble ( key, smpKeyValue->defValue<double>() ) );
                    break;
                case TBool:
                    smpKeyValue->setValue ( pConfIn->getBool( key, smpKeyValue->defValue<bool>() ) );
                    break;
                default:
                    std::ostringstream oMsg;
                    oMsg << "Invalid value type: '" << smpKeyValue->type() << "'\n";
                    wmLogTr ( eWarning, "QnxMsg.Vdr.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str() );
                    
                    break;
                    
            } // switch
        } 
        catch ( const Poco::Exception &p_rException ) 
        {

            wmLog ( eDebug, "%s - '%s': %s\n", __FUNCTION__, p_rException.name(), p_rException.message().c_str() );
            std::ostringstream oMsg;
            oMsg << "Parameter '" << key.c_str() << "' of type '" << smpKeyValue->type() << "' could not be converted. Reset to default value.\n";
            wmLog ( eDebug, oMsg.str() );
            wmLogTr ( eWarning, "QnxMsg.Vdr.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str() );
            
            smpKeyValue->resetToDefault();

        } // catch
        catch (...)
        {
            std::cout << "Uncaught Exception " << std::endl;
            std::ostringstream oMsg;
            oMsg << smpKeyValue->toString();
            wmLogTr ( eWarning, "QnxMsg.Vdr.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str() );
            
            smpKeyValue->resetToDefault();
        }
        setKeyValue ( smpKeyValue );


    }
}



}
}
