#include <calibration/CalibrationOCTConfigurationIDM.h>
#include "calibration/calibrationManager.h"
#include <cassert>

namespace precitec {
namespace calibration {

const IDM_Values_t defaultIDMValues {3018, 3018, 30, 70000, 10};

//compare OCTDeviceConfiguration::s_KeyList
const std::array<interface::Key, IDM_Parameter::eNumParameters> CalibrationOCTConfigurationIDM::IDM_Keys =
{
    "JumpSpeed",
    "MarkSpeed",
    "LineWidth",
    "SampleFrequency",
    "Rescale IDM Line"
};



CalibrationOCTConfigurationIDM::CalibrationOCTConfigurationIDM ( std::array< int, IDM_Parameter::eNumParameters > p_oConfiguration, geo2d::DPoint p_oOCTScanCenter)
        : m_oConfiguration ( std::move ( p_oConfiguration ) ), 
        m_oOCTScanCenter(p_oOCTScanCenter)
{
    validateConfiguration ( m_oConfiguration );
}

CalibrationOCTConfigurationIDM::CalibrationOCTConfigurationIDM()
: CalibrationOCTConfigurationIDM(defaultIDMValues, geo2d::DPoint{0.0,0.0}) 
{
}



int CalibrationOCTConfigurationIDM::validateConfiguration ( IDM_Values_t& p_rConfig )
{
    int invalidFields = false;
    if ( p_rConfig[eJumpSpeed] != p_rConfig [eMarkSpeed] )
    {
        wmLog ( eWarning, "Setting MarkSpeed equal to JumpSpeed" );
        p_rConfig[eMarkSpeed] = p_rConfig[eJumpSpeed];
        invalidFields++;
    }
    return invalidFields;
}

bool CalibrationOCTConfigurationIDM::loadCurrentIDMConfiguration ( CalibrationManager& p_rCalibMgr, bool setToValid )
{
    if ( !p_rCalibMgr.isOCTTrackApplication() )
    {
        return false;
    }
    static_assert ( std::tuple_size<decltype(m_oConfiguration)>::value == IDM_Parameter::eNumParameters, "");
    static_assert ( std::tuple_size<decltype(IDM_Keys)>::value == IDM_Parameter::eNumParameters,"" );

    for ( int i = 0; i < IDM_Parameter::eNumParameters; ++i )
    {
        interface::SmpKeyValue pKeyValue =  p_rCalibMgr.getIDMKeyValue ( IDM_Keys[i] );
        if ( pKeyValue.isNull() )
        {
            wmLog ( eWarning, "Could not read IDM parameter %s  \n", IDM_Keys[i].c_str() );
            return false;
        }
        if ( pKeyValue->type() != TInt )
        {
            wmLog ( eWarning, "Unexpected type for IDM parameter %s (%d)  \n", IDM_Keys[i].c_str(), pKeyValue->type() );
            return false;
        }
        m_oConfiguration[i] = pKeyValue->value<int>();
    }
    
    //get ScanCenter
    {
        auto pKv  = p_rCalibMgr.getIDMKeyValue ( "ScanCenterX" );
        if (pKv.isNull())
        {
            wmLog(eWarning, "Error when reading scan center \n");
            return false;
        }
        m_oOCTScanCenter.x = pKv->value<double>();
        pKv  = p_rCalibMgr.getIDMKeyValue ( "ScanCenterY" );
        if (pKv.isNull())
        {
            wmLog(eWarning, "Error when reading scan center \n");
            return false;
        }
        m_oOCTScanCenter.y = pKv->value<double>();
    }
    if ( setToValid )
    {
        int invalidFields = validateConfiguration ( m_oConfiguration );
        if ( invalidFields > 0 )
        {
            bool ok = configureIDM ( p_rCalibMgr, *this );
            if ( !ok )
            {
                return false;
            }
        }// invalid fields
    } // setToValid
    return true;
}

bool CalibrationOCTConfigurationIDM::configureIDM ( CalibrationManager& p_rCalibMgr ) const
{
    if ( !p_rCalibMgr.isOCTTrackApplication() )
    {
        return false;
    }
    assert ( m_oConfiguration.size() == IDM_Parameter::eNumParameters );
    assert ( IDM_Keys.size() == IDM_Parameter::eNumParameters );
    for ( int i = 0; i < IDM_Parameter::eNumParameters; ++i )
    {
        setIDM_IntValue ( p_rCalibMgr, static_cast<IDM_Parameter> ( i ), m_oConfiguration[i] );
        usleep(30 * 1000);
    }
    
    setIDMScanCenterX(p_rCalibMgr, m_oOCTScanCenter.x);
    usleep(30*1000);
    setIDMScanCenterY(p_rCalibMgr, m_oOCTScanCenter.y);
    usleep(30*1000);
    
    return true;
}

bool CalibrationOCTConfigurationIDM::configureIDM ( CalibrationManager& p_rCalibMgr, const calibration::CalibrationOCTConfigurationIDM& cachedConfiguration ) const
{
    if ( !p_rCalibMgr.isOCTTrackApplication() )
    {
        return false;
    }
    assert ( m_oConfiguration.size() == IDM_Parameter::eNumParameters );
    assert ( IDM_Keys.size() == IDM_Parameter::eNumParameters );
    assert ( cachedConfiguration.m_oConfiguration.size() == m_oConfiguration.size() );

    for ( int i = 0; i < IDM_Parameter::eNumParameters; ++i )
    {
        if ( cachedConfiguration.m_oConfiguration[i] != m_oConfiguration[i] )
        {
            setIDM_IntValue ( p_rCalibMgr, static_cast<IDM_Parameter> ( i ), m_oConfiguration[i] );
            usleep(30 * 1000);
        }
    }
    
    if (cachedConfiguration.m_oOCTScanCenter.x != m_oOCTScanCenter.x)
    {
        setIDMScanCenterX(p_rCalibMgr, m_oOCTScanCenter.x);
        usleep(30*1000);
    }
    if (cachedConfiguration.m_oOCTScanCenter.y != m_oOCTScanCenter.y)
    {
        setIDMScanCenterY(p_rCalibMgr, m_oOCTScanCenter.y);
        usleep(30*1000);
    }
    return true;
}

int CalibrationOCTConfigurationIDM::get ( IDM_Parameter i ) const
{
    return m_oConfiguration[i];
}

void CalibrationOCTConfigurationIDM::setIDM_IntValue ( CalibrationManager& p_rCalibMgr, IDM_Parameter p_parameter, int p_value ) 
{
    interface::SmpKeyValue pKeyValue = new interface::TKeyValue<int> ( IDM_Keys[p_parameter], p_value );
    p_rCalibMgr.setIDMKeyValue ( pKeyValue );
}

void CalibrationOCTConfigurationIDM::setIDMScanCenterX( CalibrationManager & p_rCalibMgr, double p_value )
{
    interface::SmpKeyValue pKeyValue = new interface::TKeyValue<double>("ScanCenterX", p_value );
    p_rCalibMgr.setIDMKeyValue ( pKeyValue );
}

void CalibrationOCTConfigurationIDM::setIDMScanCenterY( CalibrationManager & p_rCalibMgr, double p_value )
{
    interface::SmpKeyValue pKeyValue = new interface::TKeyValue<double>("ScanCenterY", p_value );
    p_rCalibMgr.setIDMKeyValue ( pKeyValue );
}
    

const IDM_Values_t& CalibrationOCTConfigurationIDM::getConfiguration() const
{
    return m_oConfiguration;
}

geo2d::DPoint CalibrationOCTConfigurationIDM::getOCTScanCenter() const
{
    return m_oOCTScanCenter;
}


void CalibrationOCTConfigurationIDM::updateOCTScanCenterInConfiguration ( geo2d::DPoint p_oScanCenter )
{
    m_oOCTScanCenter = p_oScanCenter;
}


void CalibrationOCTConfigurationIDM::updateParameterInConfiguration ( IDM_Parameter p_parameter, int p_value )
{
    m_oConfiguration[p_parameter] = p_value;
}

}
}
