/**
 * @file
 * @brief Central component that connects the device interface of the qnx processes to the windows / GUI side.
 *
 * @author 	Stefan Birmanns (SB)
 * @date	06.09.12
 */
#ifndef CENTRALDEVICEMANAGER_H_
#define CENTRALDEVICEMANAGER_H_

// clib includes
#include <vector>
// Poco includes
#include <Poco/UUID.h>
// WM includes
#include "Mod_Analyzer.h"
#if defined __QNX__ || defined __linux__
#include "analyzer/securedDeviceHandler.h"
#else
#include "Poco/SharedPtr.h"
#include "message/device.interface.h"
#endif

#include "event/deviceNotification.interface.h"
#include <filter/hardwareData.h>

class AnalyzerServer; // forward reference						analyzerServer_;

namespace precitec { using namespace interface;

namespace framework
{
namespace module
{
class ModuleManagerConnector;
}
}

namespace analyzer {

extern const Poco::UUID g_oVideoRecorderID;
extern const Poco::UUID g_oWeldHeadID;
extern const Poco::UUID g_oCameraID;
extern const Poco::UUID g_oCalibID;
extern const Poco::UUID g_oServiceID;
extern const Poco::UUID g_oWorkflowID;
extern const Poco::UUID g_oCHRCommunicationID;

enum class AppName
{
    Calibration,
    Camera,
    Inspection,
    Service,
    VideoRecorder,
    WeldHeadControl,
    Workflow
};

/**
 * @ingroup Workflow
 * @brief Central component that connects the device interface of the qnx processes to the windows / GUI side. It manages the handler/server/proxies that do the actual
 * work of routing the configuration commands to the processes like the grabber, video recorder, etc.
 */
class MOD_ANALYZER_API CentralDeviceManager
{
public:

	/**
	 * CTor.
	 * @param p_pStateContext Pointer to state context object. The central device server needs to know the current state, in order to find out if it is save to carry out device interface calls.
	 */
	CentralDeviceManager( bool simulationStation = false );

	/**
	 * Set device key, regardless of state.
	 */
	void force( const Poco::UUID &p_rDeviceId, SmpKeyValue p_pKeyValue );


	/**
	 * @brief Lock the device manager, i.e. access to the device interfaces is blocked (automatic mode).
	 * @param p_oLocked if true, the device manager blocks the calls.
	 */
	void lock( bool p_oLocked );

    void setDeviceNotification(const std::shared_ptr<TDeviceNotification<AbstractInterface>> &deviceNotification);

    void publishAllProxies(precitec::framework::module::ModuleManagerConnector *moduleManager);
    void subscribeAllHandlers(precitec::framework::module::ModuleManagerConnector *moduleManager);

    void init();
    void initWorkflowDevice(TDevice<interface::AbstractInterface> *workflowDevice);
    std::optional<double> getHardwareParameter(unsigned int appName, const std::string& parameterName) const;

#if defined __QNX__ || defined __linux__
	/**
	 * Get device key, regardless of state.
	 */
	SmpKeyValue get( const Poco::UUID &p_rDeviceId, std::string key );
protected:

	std::map<Poco::UUID, std::shared_ptr<SecuredDeviceHandler> >			m_oHandlerList;		   ///< List with handler objects that call the qnx processes.

#endif

private:
    std::shared_ptr<TDeviceNotification<AbstractInterface>> m_deviceNotification;

};

} // namespace workflow
} // namespace precitec


#endif /* CENTRALDEVICEMANAGER_H_ */
