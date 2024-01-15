#include "calibration/CalibrationOCTData.h"

#include <calibration/calibrationManager.h>
#include "Poco/Util/XMLConfiguration.h"
#include "Poco/DOM/DOMParser.h"
#include "Poco/DOM/NodeList.h"
#include "Poco/XML/XMLWriter.h"
#include "Poco/File.h"

#include <common/systemConfiguration.h>

namespace precitec {
namespace calibration {

const std::string CalibrationOCTData::m_oConfigFilename =  "CalibrationDataOCT.xml";
const std::string CalibrationOCTData::s_x_tcp_newson =  "X_TCP_Newson";
const std::string CalibrationOCTData::s_y_tcp_newson =  "Y_TCP_Newson";
const std::string CalibrationOCTData::s_distanceTCP =  "DesiredDistanceFromTCP";

const CalibrationOCTData::parameters_t CalibrationOCTData::s_DefaultParameters = 
[]() {
    const CalibrationOCTConfigurationIDM oDefaultConfiguration;

    return CalibrationOCTData::parameters_t
    {
        interface::SmpKeyValue{ new interface::TKeyValue<int>( "Reference_Speed", oDefaultConfiguration.get(eMarkSpeed), 100, 20000, oDefaultConfiguration.get(eMarkSpeed) ) },
        interface::SmpKeyValue{ new interface::TKeyValue<int>( "Reference_Width", oDefaultConfiguration.get(eScanWidth), 10, 50, oDefaultConfiguration.get(eScanWidth) ) },
        interface::SmpKeyValue{ new interface::TKeyValue<int>( "Reference_SHZ", oDefaultConfiguration.get(eSamplingFrequency), 50, 70000, oDefaultConfiguration.get(eSamplingFrequency) ) },
        interface::SmpKeyValue{ new interface::TKeyValue<int>( "Reference_Scale", oDefaultConfiguration.get(eRescaleIDMValue), 1, 1000, oDefaultConfiguration.get(eRescaleIDMValue) ) },
        interface::SmpKeyValue{ new interface::TKeyValue<int>( "Reference_X", 0,-512,512,0) }, //IDM_DUMMYIMAGE_WIDTH
        interface::SmpKeyValue{ new interface::TKeyValue<int>( "Reference_Y", 0,0,512,0) }, //IDM_DUMMYIMAGE_HEIGHT
        interface::SmpKeyValue{ new interface::TKeyValue<int>( "Reference_NumMeasurement", 700,0,1024,700) },
        interface::SmpKeyValue{ new interface::TKeyValue<double>( "Reference_LateralResolution", 15.0,0.0001,100.0,15.0) }, 
        interface::SmpKeyValue{ new interface::TKeyValue<double>( "Reference_DepthResolution" ,   1.0,0.0001,100.0,1.0) }
    };
} ();

CalibrationOCTData::CalibrationOCTData()
: m_oOCTMeasurementModel()
, m_oCurrentConfiguration() // initialize with default values
{
    assert(!isValid());
};

CalibrationOCTData::CalibrationOCTData(CalibrationOCTMeasurementModel p_OCTMeasurementModel, CalibrationOCTConfigurationIDM p_currentConfiguration)
: m_oOCTMeasurementModel(p_OCTMeasurementModel)
, m_oCurrentConfiguration(std::move(p_currentConfiguration))
{   
};

bool CalibrationOCTData::initializeSystem( CalibrationManager & p_rCalibMgr ) 
{
    using interface::SystemConfiguration;
    using math::SensorModel;
    
    if (!p_rCalibMgr.isOCTTrackApplication() || !this->isValid())
    {
        return false;
    }
    
    CalibrationOCTConfigurationIDM oCurrentIDMConfiguration;
    bool validSystemConfig = oCurrentIDMConfiguration.loadCurrentIDMConfiguration(p_rCalibMgr, true);
    if (validSystemConfig)
    {
        updateCurrentConfiguration(oCurrentIDMConfiguration);
    }
    else
    {
        wmLog(eWarning, "Can't get current IDM configuration, using reference \n");
        updateCurrentConfiguration(m_oOCTMeasurementModel.m_oReferenceConfiguration);
    }
    
    //update the line center according to DesiredDistanceFromTCP

    geo2d::DPoint newScanCenter = {m_oTCPNewson[0],  m_oTCPNewson[1] - m_oOCTMeasurementModel.getNewsonPositionFromDistance(m_oDesiredDistanceFromTCP)};
    geo2d::DPoint currentScanCenter = oCurrentIDMConfiguration.getOCTScanCenter();
    if (newScanCenter != currentScanCenter)
    {
        wmLog(eInfo, "Set scan center with a distance of %f from TCP \n", m_oDesiredDistanceFromTCP);
        wmLog(eDebug, "Old scan center: %f, %f New scan center %f, %f \n",  currentScanCenter.x, currentScanCenter.y, newScanCenter.x, newScanCenter.y);
        wmLog(eDebug, "TCP position %f, %f [Newson]\n", m_oTCPNewson[0], m_oTCPNewson[1]);
        
        CalibrationOCTConfigurationIDM oNewOCTConfiguration(oCurrentIDMConfiguration);
        oNewOCTConfiguration.updateOCTScanCenterInConfiguration(newScanCenter);
        oNewOCTConfiguration.configureIDM(p_rCalibMgr, oCurrentIDMConfiguration); 
        oCurrentIDMConfiguration = oNewOCTConfiguration;
    }
    
    //just overwrite the coax calibration data of sensor 0  - in the future it could make sense to define a new type of sensor
    // compare CalibrationManager::getOSCalibrationDataFromHW, CalibrateIbOpticalSystem::startCalibration
    if (SystemConfiguration::instance().getInt("Type_of_Sensor", -1) != interface::TypeOfSensor::eCoax)
    {
        wmLog(eWarning, "IDM uses the coax calibration model: replacing System::Type_of_Sensor \n" );        
        interface::SystemConfiguration::instance().setInt("Type_of_Sensor", interface::TypeOfSensor::eCoax);
    }
    
    const auto oCalibSensorId = math::SensorId::eSensorId0;
    auto & rCalibrationData = p_rCalibMgr.getCalibrationData(oCalibSensorId); 
    computeEquivalentCoaxCalibrationData(rCalibrationData);
    
    p_rCalibMgr.sendCalibDataChangedSignal(oCalibSensorId , true); // transfer xml file & 3D coords
    if ( !p_rCalibMgr.getCalibrationData(oCalibSensorId).checkCalibrationValuesConsistency() )
    {
        wmLog(eDebug, "performMagnificationComputation: parameters updated, but inconsistencies found\n" );
    }
    assert(rCalibrationData.getSensorModel() == math::SensorModel::eLinearMagnification);
    assert(rCalibrationData.hasData());
    
    m_oEquivalentCoaxCalibModified = false;
    return true;

}
    

//update the model with the one from file, or with the default in case of errors
bool CalibrationOCTData::load(std::string pHomeDir)
 {
    using namespace Poco;
    
    bool validFile = false;
    
    //try reading values from config file
    std::string oConfigDir = pHomeDir + "/config";

    File oConfigFile( oConfigDir + "/" + m_oConfigFilename );
    AutoPtr<Util::XMLConfiguration> pConfIn = nullptr;
    if (oConfigFile.exists()) 
    { 
        try
        {// poco syntax exception might be thrown or sax parse excpetion
            pConfIn = new Util::XMLConfiguration(oConfigFile.path());
            validFile = true;
        }
        catch ( const Exception &p_rException )
        {
            validFile = false;
            std::cout << "XML configuration kann nicht erstellt werden... " << std::endl;
            wmLog(eDebug, "%s - '%s': %s\n", __FUNCTION__, p_rException.name(), p_rException.message().c_str());
            wmLog(eDebug, "Could not read parameters from file:\n");
            wmLog(eDebug, oConfigFile.path() + "\n" );
            wmLog(eWarning,"An error occurred in the procedure '%s'. Message: '%s'. Calibration file %s could not be read.\n", __FUNCTION__, p_rException.message().c_str(), oConfigFile.path());
        } // catch
    }
    else
    {
        validFile = false;
        std::ostringstream oMsg;
        oMsg << "OCT Calib file " << oConfigFile.path() << " does not exist, using default configuration \n" ;
        wmLog(eInfo, oMsg.str().c_str() );
    }
    
    if (!validFile)
    {
        //create an empty configuration, so that the load methods use the defaulf values
        pConfIn = new Util::XMLConfiguration();
    }
    
    assert (! pConfIn.isNull());
    
    m_oOCTMeasurementModel = getModelFromParameters(getParametersFromXMLConfiguration(pConfIn));        
    m_oTCPNewson[0] = pConfIn->getDouble(s_x_tcp_newson, 0.0);
    m_oTCPNewson[1] = pConfIn->getDouble(s_y_tcp_newson, 0.0); 
    m_oDesiredDistanceFromTCP = pConfIn->getDouble(s_distanceTCP, 0.0);
    m_oCalibrationOCTLineParameters.load(pConfIn);

    m_oEquivalentCoaxCalibModified = true;
    
    return validFile;
} //load
    


bool CalibrationOCTData::write ( std::string pHomeDir )
{
    std::string oConfigDir = pHomeDir + "/config/";
    Poco::File oConfigFileInstance ( oConfigDir + m_oConfigFilename );
    if (!m_oOCTMeasurementModel.isInitialized())
    {
        return false;
    }
    
    if (!Poco::File(oConfigDir).exists())
    {
        Poco::File(oConfigDir).createDirectories();
    }

    interface::writeToFile(oConfigFileInstance.path(), makeConfiguration(true));
    return oConfigFileInstance.exists();

}

void CalibrationOCTData::updateCurrentConfiguration ( CalibrationOCTConfigurationIDM p_currentConfiguration )
{
    m_oCurrentConfiguration = std::move ( p_currentConfiguration );
    m_oEquivalentCoaxCalibModified = true;
}

void CalibrationOCTData::updateMeasurementModel ( CalibrationOCTMeasurementModel p_OCTMeasurementModel)
{
    m_oOCTMeasurementModel = p_OCTMeasurementModel;
    m_oEquivalentCoaxCalibModified = true;
}

bool CalibrationOCTData::isValid() const
{
    return m_oOCTMeasurementModel.isInitialized();
}


CalibrationOCTMeasurementModel CalibrationOCTData::getModelFromParameters ( CalibrationOCTData::parameters_t oCurrentModelConfiguration )
{
    IDM_Values_t oIDM_Values {
        oCurrentModelConfiguration[Parameter::Speed]->value<int>(), //eJumpSpeed
        oCurrentModelConfiguration[Parameter::Speed]->value<int>(), //eMarkSpeed
        oCurrentModelConfiguration[Parameter::Width]->value<int>(),
        oCurrentModelConfiguration[Parameter::SHZ]->value<int>(),
        oCurrentModelConfiguration[Parameter::Scale]->value<int>()
    };
    geo2d::Point oReferencePosition ( oCurrentModelConfiguration[Parameter::X]->value<int>(), oCurrentModelConfiguration[Parameter::Y]->value<int>() );
    geo2d::DPoint dummyScanCenter {0.0, 0.0}; // the measurement model currently doesn't need the scan center, we use it only to construct the Configuration
    return { {oIDM_Values, dummyScanCenter}, 
             oReferencePosition,
             oCurrentModelConfiguration[Parameter::NumMeasurement]->value<int>(),
             oCurrentModelConfiguration[Parameter::LateralResolution]->value<double>(),
             oCurrentModelConfiguration[Parameter::DepthResolution]->value<double>()
           };
}

CalibrationOCTData::parameters_t CalibrationOCTData::getParametersFromModel(const CalibrationOCTMeasurementModel & oCurrentModel)
    {
        parameters_t oCurrentModelConfiguration;
        //copy key, min, max, default from DefaultParameters
        for(unsigned int i= 0; i <oCurrentModelConfiguration.size(); ++i) 
        {
            oCurrentModelConfiguration[i] = (s_DefaultParameters[i]->clone());
        }
        
        oCurrentModelConfiguration[Parameter::Speed]->setValue<int>(oCurrentModel.m_oReferenceConfiguration.get(eJumpSpeed));
        oCurrentModelConfiguration[Parameter::Width]->setValue<int>(oCurrentModel.m_oReferenceConfiguration.get(eScanWidth));
        oCurrentModelConfiguration[Parameter::SHZ]->setValue<int>(oCurrentModel.m_oReferenceConfiguration.get(eSamplingFrequency));
        oCurrentModelConfiguration[Parameter::Scale]->setValue<int>(oCurrentModel.m_oReferenceConfiguration.get(eRescaleIDMValue));
        oCurrentModelConfiguration[Parameter::X]->setValue<int>(oCurrentModel.m_oReferencePosition.x);
        oCurrentModelConfiguration[Parameter::Y]->setValue<int>(oCurrentModel.m_oReferencePosition.y);
        oCurrentModelConfiguration[Parameter::NumMeasurement]->setValue<int>(oCurrentModel.m_oReferenceNumberMeasurements);
        oCurrentModelConfiguration[Parameter::LateralResolution]->setValue<double>(oCurrentModel.m_oReferenceLateralResolution);
        oCurrentModelConfiguration[Parameter::DepthResolution]->setValue<double>(oCurrentModel.m_oReferenceDepthResolution);

        return oCurrentModelConfiguration;
    }




void CalibrationOCTData::computeEquivalentCoaxCalibrationData ( math::CalibrationData& rCalibrationData )
{
    rCalibrationData.resetConfig();
    const bool createDefaultFile = false;
    rCalibrationData.initConfig ( math::SensorModel::eLinearMagnification, createDefaultFile );

    double dPix = 1e-3;
    
    double beta0, betaZ;
    std::tie(beta0, betaZ) =  m_oOCTMeasurementModel.getEquivalentCoaxCalibration ( m_oCurrentConfiguration, dPix, dPix );
    rCalibrationData.setKeyValue ( "beta0", beta0 );
    rCalibrationData.setKeyValue ( "betaZ", betaZ );
    rCalibrationData.setKeyValue ( "HighPlaneOnImageTop", false );
    rCalibrationData.setKeyValue ( "DpixX", dPix );
    rCalibrationData.setKeyValue ( "DpixY", dPix );
    
    auto  scanCenter = m_oCurrentConfiguration.getOCTScanCenter(); //the sensor proxy sets the HWROI so that the scan center it's at the center of the "full sensor" image  
    
    double xTCP_mm  = m_oOCTMeasurementModel.getNewsonLineWidthCalibrated(m_oTCPNewson[0] - scanCenter.x)/1000.0; //distance from the center of the scan line
    double xTCP_pixel =  xTCP_mm *  beta0 / dPix + 512; // from image top left corner
    
    double yTCP_mm  = m_oOCTMeasurementModel.getNewsonLineWidthCalibrated(m_oTCPNewson[1] - scanCenter.y)/1000.0; //distance from the center of the scan line
    double yTCP_pixel =  yTCP_mm *  beta0/ dPix + 512 ;
        
    rCalibrationData.setKeyValue( "xtcp", std::round(xTCP_pixel));
    rCalibrationData.setKeyValue( "ytcp", std::round(yTCP_pixel));
    
    rCalibrationData.setKeyValue  ("SensorParametersChanged",false);
    //all the parameters have been set, now it's possible to update the coordinates field
    rCalibrationData.load3DFieldFromParameters();
    
}


const CalibrationOCTMeasurementModel& CalibrationOCTData::getMeasurementModel() const
{
    return m_oOCTMeasurementModel;
}

interface::Configuration CalibrationOCTData::makeConfiguration(bool withReferenceValues) const
{
    
    interface::Configuration oConfig {
        getKeyValue( s_x_tcp_newson), getKeyValue( s_y_tcp_newson), getKeyValue(s_distanceTCP)
    };
    
    if (withReferenceValues)
    {
        auto currentParameters = getParametersFromMeasurementModel();
        for (auto it =  currentParameters.begin(); it < currentParameters.end(); it++)
        {
            oConfig.push_back(*it);
            //oConfig.back()->setReadOnly(true);  //the measurement model parameters should not be changed in the calibration device page
        }
    }
    auto m_oCalibrationOCTLineParametersConfig = m_oCalibrationOCTLineParameters.makeConfiguration();
    oConfig.insert( oConfig.end(), m_oCalibrationOCTLineParametersConfig.begin(), m_oCalibrationOCTLineParametersConfig.end() );
    return oConfig;
}

interface::SmpKeyValue CalibrationOCTData::getKeyValue(std::string p_key) const
{
    if (p_key == s_x_tcp_newson)
    {
        return interface::SmpKeyValue{ new interface::TKeyValue<double>( s_x_tcp_newson, m_oTCPNewson[0], -100.0, 100.0, 0.0 ) };
    }
    if (p_key == s_y_tcp_newson)
    {
        return interface::SmpKeyValue{ new interface::TKeyValue<double>( s_y_tcp_newson, m_oTCPNewson[1], -100.0, 100.0, 0.0 ) };
    }
    if (p_key == s_distanceTCP)
    {
        return interface::SmpKeyValue{ new interface::TKeyValue<double>(s_distanceTCP, m_oDesiredDistanceFromTCP, -100.0, 100.0, 0.0 ) };
    }
    //update reference parameters
    auto currentParameters = getParametersFromMeasurementModel();
    for (auto spKv : currentParameters)
    {
        if (p_key == spKv->key())
        {
            return spKv->clone();
        }
    }
    
    auto spKV = m_oCalibrationOCTLineParameters.getKeyValue(p_key);
    if (!spKV.isNull())
    {
        return spKV;
    }
    
    return {};
}

interface::KeyHandle CalibrationOCTData::setKeyValue(interface::SmpKeyValue p_oSmpKeyValue)
{
    
    bool isTCPKeyValue = false;
    bool isMeasurementModelKeyValue = false;
    bool isLineKeyValue = false;
    
    auto p_key = p_oSmpKeyValue->key();
    
    
    if (p_oSmpKeyValue->type() == TDouble)
    {
        if (p_key == s_x_tcp_newson)
        {
            m_oTCPNewson[0] = p_oSmpKeyValue->value<double>();
            isTCPKeyValue = true;
        }
        if (p_key == s_y_tcp_newson)
        {
            m_oTCPNewson[1] = p_oSmpKeyValue->value<double>();
            isTCPKeyValue = true;
        }
        if (p_key == s_distanceTCP)
        {
            m_oDesiredDistanceFromTCP = p_oSmpKeyValue->value<double>();
            isTCPKeyValue = true;
        }
    }
    
    if (!isTCPKeyValue)
    {
        //update measurement model        
        auto currentParameters = getParametersFromMeasurementModel();
        for (auto & spKv: currentParameters)
        {
            if (p_key == spKv->key() && p_oSmpKeyValue->type() == spKv->type() )
            {
                switch(spKv->type())
                {
                    case TInt:
                        spKv->setValue(p_oSmpKeyValue->value<int>());
                        break;
                    case TDouble:
                        spKv->setValue(p_oSmpKeyValue->value<double>());
                        break;
                    default:
                        assert(false);
                }
                isMeasurementModelKeyValue = true;
                break;
            }
        }
        if (isMeasurementModelKeyValue)
        {
            //update the measurement model
            m_oOCTMeasurementModel = getModelFromParameters(currentParameters);
        }
    }
    
    if (!isTCPKeyValue && ! isMeasurementModelKeyValue)
    {
        auto h = m_oCalibrationOCTLineParameters.setKeyValue(p_oSmpKeyValue);
        isLineKeyValue = (h.handle() != -1);
    }
    
    if (isTCPKeyValue || isMeasurementModelKeyValue || isLineKeyValue)
    {
        if (isMeasurementModelKeyValue || isTCPKeyValue)
        {
            m_oEquivalentCoaxCalibModified = true;
        }
        //key values in m_oCalibrationOCTLineParameters are used only for the line calibration procedure, reload coax-equivalent calibration is not needed
            
        return {1};        
    }
    
    return {-1}; // handle invalid 
}



CalibrationOCTData::parameters_t CalibrationOCTData::getParametersFromXMLConfiguration (const Poco::Util::XMLConfiguration* pConfIn )
{
    
    assert ( pConfIn );

    //define default values
    parameters_t oModelConfiguration;
    for(unsigned int i= 0; i <oModelConfiguration.size(); ++i) 
    {
        oModelConfiguration[i] = (s_DefaultParameters[i]->clone());
    }
    for ( int i= 0; i <int ( Parameter::NumParameters ); ++i ) {
        const auto smpKeyValue = s_DefaultParameters[i];
        const Types oType ( smpKeyValue->type() );
        const auto oKey ( smpKeyValue->key() );
        try {
            switch ( oType ) {
            case TBool:
                oModelConfiguration[i]->setValue ( pConfIn->getBool ( oKey, smpKeyValue->defValue<bool>() ) ); // assign value
                break;
            case TInt:
                oModelConfiguration[i]->setValue ( pConfIn->getInt ( oKey, smpKeyValue->defValue<int>() ) ); // assign value
                break;
            case TUInt: {
                // NOTE: theres is no 'setUInt()' or 'getUInt()', thus needs cast, because written with 'setInt()'
                auto defValue = smpKeyValue->defValue<uint32_t>();
                oModelConfiguration[i]->setValue ( static_cast<uint32_t> ( pConfIn->getInt ( oKey, defValue ) ) ); // assign value
            }
            break;
            case TString:
                oModelConfiguration[i]->setValue ( pConfIn->getString ( oKey, smpKeyValue->defValue<std::string>() ) ); // assign value
                break;
            case TDouble:
                oModelConfiguration[i]->setValue ( pConfIn->getDouble ( oKey, smpKeyValue->defValue<double>() ) ); // assign value
                break;
            default:
                std::ostringstream oMsg;
                oMsg << "Invalid value type: '" << oType << "'\n";
                wmLogTr ( eWarning, "QnxMsg.Vdr.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str() );
                break;
            } // switch
        } // try
        catch ( const Poco::Exception &p_rException ) {
            oModelConfiguration[i]->resetToDefault();

            wmLog ( eDebug, "%s - '%s': %s\n", __FUNCTION__, p_rException.name(), p_rException.message().c_str() );
            std::ostringstream oMsg;
            oMsg << "Parameter '" << oKey.c_str() << "' of type '" << oType << "' could not be converted. Reset to default value.\n";
            wmLog ( eDebug, oMsg.str() );
            wmLogTr ( eWarning, "QnxMsg.Vdr.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str() );

        } // catch
    } // for
    return oModelConfiguration;

}

CalibrationOCTData::parameters_t CalibrationOCTData::getParametersFromMeasurementModel() const
{
    return getParametersFromModel ( m_oOCTMeasurementModel );
}

} //calibration    
} //precitec
