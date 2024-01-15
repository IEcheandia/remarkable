/**
 * @file
 * @brief Central component that connects the device interface of the qnx processes to the windows / GUI side.
 *
 * @author 	Stefan Birmanns (SB)
 * @date	06.09.12
 */

// WM includes
#include "module/moduleLogger.h"
#include "analyzer/centralDeviceManager.h"
#include "common/connectionConfiguration.h"
#include "analyzer/securedWeldHeadServer.h"
#include "analyzer/securedCalibrationServer.h"
#include "common/systemConfiguration.h"

// Poco includes
#include "Poco/UUID.h"

namespace precitec { using namespace interface;
namespace analyzer {

/* change in \Util\Precitec.Business.Components\HardwareParameterComponent.cs */
const Poco::UUID g_oVideoRecorderID	( "96599c45-4e20-4aaa-826d-25463670dd09" );
const Poco::UUID g_oWeldHeadID		( "3c57acde-707e-4c7d-a6b5-0e9352568095" );
const Poco::UUID g_oCameraID		( "1f50352e-a92a-4521-b184-e16809345026" );
const Poco::UUID g_oCalibID			( "c3a01597-53db-4262-a091-69b0345f083d" );
const Poco::UUID g_oServiceID       ( "a97a5a4c-dcd0-4a77-b933-9d1e20dbe73c" );
const Poco::UUID g_oInspectionID    ( "F42DDE6B-C8FF-4CE5-86DE-1A5CB51D633A" );
const Poco::UUID g_oWorkflowID    	( "A60D345E-16CA-4710-AB4D-CAD65CC42959" );
const Poco::UUID g_oStorageID       ( "1f58fcea-58bc-4c1a-945b-9b24a9e09963" );
const Poco::UUID g_oCHRCommunicationID ( "4fc80872-b8ad-11e7-abc4-cec278b6b50a" );

CentralDeviceManager::CentralDeviceManager(bool simulationStation )
{
#if defined __QNX__ || defined __linux__

    wmLog( eDebug, "Initializing the central device manager.\n");

    // Camera
    wmLog( eDebug, "CentralDeviceManager: Grabber ...\n");
    m_oHandlerList.emplace(std::make_pair(g_oCameraID,
        std::make_shared<SecuredDeviceHandler>(
            std::make_shared<SecuredDeviceServer>(std::string("Grabber"), system::module::GrabberModul, g_oCameraID),
            std::string("Grabber"))
    ));

    // Calibration
    wmLog( eDebug, "CentralDeviceManager: Calibration ...\n");
    m_oHandlerList.emplace(std::make_pair(g_oCalibID,
        std::make_shared<SecuredDeviceHandler>(
            std::make_shared<SecuredCalibrationServer>(std::string("Calibration"), system::module::CalibrationModul, g_oCalibID ),
            std::string("CalibrationControl"))
    ));

    if (!simulationStation)
    {
        // VideoRecorder
        wmLog( eDebug, "CentralDeviceManager: VideoRecorder ...\n");
        m_oHandlerList.emplace(std::make_pair(g_oVideoRecorderID,
            std::make_shared<SecuredDeviceHandler>(
                std::make_shared<SecuredDeviceServer>(std::string("Videorecorder"), system::module::VideoRecorderModul, g_oVideoRecorderID ),
                std::string("Videorecorder"))
        ));

        // WeldHeadControl
        wmLog( eDebug, "CentralDeviceManager: WeldHeadControl ...\n");
        m_oHandlerList.emplace(std::make_pair(g_oWeldHeadID,
            std::make_shared<SecuredDeviceHandler>(
                std::make_shared<SecuredWeldHeadServer>(m_oHandlerList.at(g_oCameraID), system::module::VIWeldHeadControl, g_oWeldHeadID ),
                std::string("WeldHeadControl"))
        ));

        // Service
        wmLog( eDebug, "CentralDeviceManager: Service ...\n");
        m_oHandlerList.emplace(std::make_pair(g_oServiceID,
            std::make_shared<SecuredDeviceHandler>(
                std::make_shared<SecuredDeviceServer>(std::string("Service"), precitec::system::module::VIServiceModul, g_oServiceID ),
                std::string("Service") )
        ));

        // Inspection
        wmLog( eDebug, "CentralDeviceManager: Inspection ...\n");
        m_oHandlerList.emplace(std::make_pair(g_oInspectionID,
            std::make_shared<SecuredDeviceHandler>(
                std::make_shared<SecuredDeviceServer>( std::string("Inspection"), precitec::system::module::VIInspectionControl, g_oInspectionID ),
                std::string("InspectionControl") )
        ));
    }


    // Storage
    wmLog( eDebug, "CentralDeviceManager: Storage ...\n");
    m_oHandlerList.emplace(std::make_pair(g_oStorageID,
        std::make_shared<SecuredDeviceHandler>(
            std::make_shared<SecuredDeviceServer>(std::string("Storage"), precitec::system::module::StorageModul, g_oStorageID ),
            std::string("Storage") )
    ));
    
    
    if ((interface::SystemConfiguration::instance().getBool("IDM_Device1Enable", false) || interface::SystemConfiguration::instance().getBool("CLS2_Device1Enable", false)) && !simulationStation)
    {
        // CHRCommunication
        wmLog( eDebug, "CentralDeviceManager: CHRCommunication ...\n");
        m_oHandlerList.emplace(std::make_pair(g_oCHRCommunicationID,
            std::make_shared<SecuredDeviceHandler>(
                std::make_shared<SecuredDeviceServer>(std::string("CHRCommunication"), system::module::CHRCommunicationModul, g_oCHRCommunicationID),
                std::string("CHRCommunication") )
        ));
    }
#endif

} // CTor.


void CentralDeviceManager::initWorkflowDevice(TDevice<interface::AbstractInterface> *workflowDevice)
{
    // Workflow
    wmLog( eDebug, "CentralDeviceManager: Workflow ...\n");
    m_oHandlerList.emplace(std::make_pair(g_oWorkflowID,
        std::make_shared<SecuredDeviceHandler>(
            std::make_shared<SecuredDeviceServer>(std::string("Workflow"), workflowDevice, g_oWorkflowID ),
            std::string("Workflow") )
    ));
}

void CentralDeviceManager::publishAllProxies(precitec::framework::module::ModuleManagerConnector *moduleManager)
{
    for (const auto& handler : m_oHandlerList)
    {
        handler.second->getServer()->publish(moduleManager);
    }
}

void CentralDeviceManager::subscribeAllHandlers(precitec::framework::module::ModuleManagerConnector *moduleManager)
{
    for (const auto& handler : m_oHandlerList)
    {
        handler.second->subscribe(moduleManager);
    }
}

void CentralDeviceManager::init()
{
    for (const auto& handler : m_oHandlerList)
    {
        handler.second->getServer()->init();
    }
}

void CentralDeviceManager::force( const Poco::UUID &p_rDeviceId, SmpKeyValue p_pKeyValue )
{
#if defined __QNX__ || defined __linux__
    const auto&  it = m_oHandlerList.find(p_rDeviceId);
    if (it == m_oHandlerList.end())
    {
		wmLog( eDebug, "CentralDeviceManager::force() - the device handler was not found!\n" );
		return;
    }

    it->second->getServer()->force(p_pKeyValue);

#endif
} // force


#if defined __QNX__ || defined __linux__
SmpKeyValue CentralDeviceManager::get( const Poco::UUID &p_rDeviceId, std::string key )
{
    const auto& it = m_oHandlerList.find(p_rDeviceId);
    if (it == m_oHandlerList.end())
    {
		wmLog( eDebug, "CentralDeviceManager::get() - the device handler was not found!\n" );
		return nullptr;
    }

    return it->second->getServer()->get(key);
}
#endif



void CentralDeviceManager::lock( bool p_oLocked )
{
#if defined __QNX__ || defined __linux__

    for(const auto& handler: m_oHandlerList)
    {
        handler.second->lock(p_oLocked);
    }

#endif

} // lock

void CentralDeviceManager::setDeviceNotification(const std::shared_ptr<TDeviceNotification<AbstractInterface>> &deviceNotification)
{
    m_deviceNotification = deviceNotification;

#if defined __QNX__ || defined __linux__
    for(const auto &handler: m_oHandlerList)
    {
        handler.second->getServer()->setDeviceNotification(deviceNotification);
    }
#endif
}

std::optional<double> CentralDeviceManager::getHardwareParameter(unsigned int appName, const std::string& parameterName) const
{
    Poco::UUID appUUID;
    switch(AppName(appName))
    {
        case(AppName::Calibration):
        {
            appUUID = g_oCalibID;
            break;
        }
        case(AppName::Camera):
        {
            appUUID = g_oCameraID;
            break;
        }
        case(AppName::Inspection):
        {
            appUUID = g_oInspectionID;
            break;
        }
        case(AppName::Service):
        {
            appUUID = g_oServiceID;
            break;
        }
        case(AppName::VideoRecorder):
        {
            appUUID = g_oVideoRecorderID;
            break;
        }
        case(AppName::WeldHeadControl):
        {
            appUUID = g_oWeldHeadID;
            break;
        }
        case(AppName::Workflow):
        {
            appUUID = g_oWorkflowID;
            break;
        }
        default:
            wmLog(eWarning, "Application name is not supported.\n");
            break;
    }

    std::optional<double> paramterValue;
    const auto it = m_oHandlerList.find(appUUID);
    if (it == m_oHandlerList.end())
    {
        return paramterValue;
    }

    const auto handler = it->second;
    const auto configs = handler->getServer()->get();

    for (const auto& config: configs)
    {
        const auto isValid = config.get()->isHandleValid() && config.get()->isValueValid();
        if (isValid)
        {
            if (config.get()->key() == parameterName)
            {
                const auto type = config.get()->type();
                switch (type)
                {
                    case TUInt:
                    {
                        paramterValue = static_cast<double>(config.get()->value<uInt>());
                        break;
                    }
                    case TInt:
                    {
                        paramterValue = static_cast<double>(config.get()->value<int>());
                        break;
                    }
                    case TBool:
                    {
                        paramterValue = static_cast<double>(config.get()->value<bool>());
                        break;
                    }
                    case TFloat:
                    {
                        paramterValue = static_cast<double>(config.get()->value<float>());
                        break;
                    }
                    case TDouble:
                    {
                        paramterValue = config.get()->value<double>();
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }
    return paramterValue;
}

} // namespace workflow
} // namespace precitec

