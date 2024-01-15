#include "calibration/CalibrationOCTLineParameters.h"
#include <cassert>
#include <array>

namespace precitec {
namespace calibration {
    
const std::array<std::string, CalibrationOCTLineParameters::key_id::NUM_IDs> CalibrationOCTLineParameters::s_keys = 
{
    "OCT_Line_MinNumValuesPerLayer",
    "OCT_Line_MaxRangePerLayer",
    "OCT_Line_MinJumpPixel",
    "OCT_Line_MaxWidthPixel",
    "OCT_Line_GapWidth",
    "OCT_Line_GapHeight",
};
    
interface::Configuration CalibrationOCTLineParameters::makeConfiguration() const
{
    interface::Configuration oConfig;
    for (auto & key: s_keys)
    {
        oConfig.push_back(getKeyValue(key));
    }
    return oConfig;
}

interface::SmpKeyValue CalibrationOCTLineParameters::getKeyValue ( std::string p_key ) const
{
    auto it = std::find(s_keys.begin(), s_keys.end(), p_key);
    if (it == s_keys.end())
    {
        return {};
    }
    
    key_id id = static_cast<key_id>(std::distance(s_keys.begin(), it));
    
    switch (id)
    {
        case (NUM_IDs):
            //key not found
            return {};
        case (e_minNumValuesPerLayer ):
            return interface::SmpKeyValue { new interface::TKeyValue<int> ( p_key, m_minNumValuesPerLayer, 0, 100, 10 )};
        case (e_maxRangePerLayer ) :
            return interface::SmpKeyValue {new interface::TKeyValue<double> ( p_key, m_maxRangePerLayer, 0, 100, 10 )};
        case (e_minJumpPixel ) :
            return interface::SmpKeyValue {new interface::TKeyValue<int> ( p_key, m_minJumpPixel, 0, 100, 30 )};
        case (e_maxWidthPixel ): 
            return interface::SmpKeyValue { new interface::TKeyValue<int> ( p_key, m_maxWidthPixel, 0, 1000, 200 )};
        case (e_gapWidth):
            return interface::SmpKeyValue { new interface::TKeyValue<double> ( p_key, m_gapWidth, 0, 10000, 1000 )};
        case (e_gapHeight):
            return interface::SmpKeyValue { new interface::TKeyValue<double> ( p_key, m_gapHeight, 0, 10000, 1000 )};
    }
    return {};
}


interface::KeyHandle CalibrationOCTLineParameters::setKeyValue ( interface::SmpKeyValue p_oSmpKeyValue )
{
    assert(!p_oSmpKeyValue.isNull());
    auto it = std::find(s_keys.begin(), s_keys.end(), p_oSmpKeyValue->key());
    
    if (it == s_keys.end())
    {
        wmLog(eDebug, "CalibrationOCTLineParameters: Unknown key  %s \n",  p_oSmpKeyValue->key().c_str());
        return {-1};
    }
    
    key_id id = static_cast<key_id>(std::distance(s_keys.begin(), it));
    
    switch(p_oSmpKeyValue->type())
    {
        case(TDouble ): 
            if ( id == e_maxRangePerLayer ) 
            {
                m_maxRangePerLayer = p_oSmpKeyValue->value<double>();
                return {1};
            }
            if ( id == e_gapWidth ) 
            {
                m_gapWidth = p_oSmpKeyValue->value<double>();
                return {1};
            }
            if ( id == e_gapHeight ) 
            {
                m_gapHeight = p_oSmpKeyValue->value<double>();
                return {1};
            }
            break;
        case (TInt ):
            if ( id == e_minNumValuesPerLayer ) 
            {
                m_minNumValuesPerLayer = p_oSmpKeyValue->value<int>();
                return {1};
            }
            if ( id == e_minJumpPixel ) 
            {
                m_minJumpPixel = p_oSmpKeyValue->value<int>();
                return {1};
            }
            if ( id == e_maxWidthPixel ) 
            {
                m_maxWidthPixel = p_oSmpKeyValue->value<int>();
                return {1};
            }
            break;
        default:
            wmLog(eDebug, "CalibrationOCTLineParameters: Unkwown key  %s of type %d \n",  p_oSmpKeyValue->key().c_str(), int(p_oSmpKeyValue->type()) );
    }
    return {-1};
}

void CalibrationOCTLineParameters::load ( const Poco::Util::AbstractConfiguration* pConfIn )
{
    assert ( pConfIn );

    for (auto & key: s_keys)
    {
        auto smpKeyValue = getKeyValue(key);
        
        try 
        {
            switch ( smpKeyValue->type() ) 
            {
                case TBool:
                    smpKeyValue->setValue ( pConfIn->getBool ( key, smpKeyValue->defValue<bool>() ) );
                    break;
                    
                case TInt:
                    smpKeyValue->setValue ( pConfIn->getInt ( key, smpKeyValue->defValue<int>() ) );
                    break;
                    
                case TUInt: 
                    { // NOTE: theres is no 'setUInt()' or 'getUInt()', thus needs cast, because written with 'setInt()'
                        auto defValue = smpKeyValue->defValue<uint32_t>();
                        smpKeyValue->setValue ( static_cast<uint32_t> ( pConfIn->getInt ( key, defValue ) ) );
                    }
                    break;
                    
                case TString:
                    smpKeyValue->setValue ( pConfIn->getString ( key, smpKeyValue->defValue<std::string>() ) );
                    break;
                    
                case TDouble:
                    smpKeyValue->setValue ( pConfIn->getDouble ( key, smpKeyValue->defValue<double>() ) );
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



} //calibration    
} //precitec
