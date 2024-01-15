/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Al / EA
 *  @date       04.10.2011
 *  @brief      Main for VI_InspectionControl
 *  @details
 */

#ifndef APPMAIN_H_
#define APPMAIN_H_

#include <stdlib.h>
#include <stdio.h>
#include <sys/resource.h>

#include "Poco/ListMap.h"
#include "Poco/Thread.h"

#include "message/module.h"
// die Basisklasse
#include "module/baseModule.h"

#include "event/inspectionCmd.proxy.h"
#include "event/inspection.proxy.h"
#include "event/sensor.proxy.h"
#include "message/db.proxy.h"
#include "event/S6K_InfoToProcesses.proxy.h"
#include "event/ethercatOutputs.proxy.h"

#include "event/triggerCmd.handler.h"
#include "viInspectionControl/TriggerCmdServer.h"

#include "event/inspectionOut.handler.h"


#include "event/S6K_InfoFromProcesses.handler.h"
#include "viInspectionControl/S6K_InfoFromProcessesServer.h"

#include "event/ethercatInputs.handler.h"
#include "viInspectionControl/EthercatInputsServer.h"

#include "event/controlSimulation.handler.h"
#include "viInspectionControl/ControlSimulationServer.h"

#include "event/results.handler.h"
#include "viInspectionControl/ResultsServer.h"

#include "event/results.proxy.h"

#include "message/device.handler.h"
#include "viInspectionControl/deviceServer.h"

#include "viInspectionControl/VI_InspectionControl.h"
#include "common/connectionConfiguration.h"

#include "event/dbNotification.handler.h"
#include "viInspectionControl/dbNotificationServer.h"

namespace precitec
{

	using namespace Poco;
	using namespace ethercat;

    enum VdrFileType
	{
		UnknownVdrFileType,
		ImageVdrFileType,
		SampleVdrFileType
	};

namespace viInspectionControl{

class AppMain : public framework::module::BaseModule {
public:
	AppMain();
	virtual ~AppMain();
	virtual void runClientCode();
	int init(int argc, char * argv[]);

public:
	TInspection<EventProxy> 		inspectProxy_;
	TInspectionCmd<EventProxy> 		inspectCmdProxy_;
	TSensor<EventProxy>    			m_oSensorProxy;
	TDb<MsgProxy> 					m_oDbProxy;
    std::shared_ptr<TInspectionToS6k<EventProxy>> m_inspectionToS6k;
    
    DbNotificationServer				m_dbNotificationServer;
	TDbNotification<EventHandler>		m_dbNotificationHandler;

	VI_InspectionControl			m_control;
	TInspectionOut<EventHandler>	m_oInspectionOutHandler;

	TS6K_InfoToProcesses<EventProxy> m_oS6K_InfoToProcessesProxy;

    S6K_InfoFromProcessesServer      m_oS6K_InfoFromProcessesServer;
    TS6K_InfoFromProcesses<EventHandler> m_oS6K_InfoFromProcessesHandler;

	TriggerCmdServer                m_oTriggerCmdServer;
	TTriggerCmd<EventHandler>       m_oTriggerCmdHandler;

	TEthercatOutputs<EventProxy>    m_oEthercatOutputsProxy;

 	EthercatInputsServer            m_oEthercatInputsServer;
	TEthercatInputs<EventHandler>   m_oEthercatInputsHandler;

    ControlSimulationServer         m_oControlSimulationServer;
    TControlSimulation<EventHandler> m_oControlSimulationHandler;

	ResultsServer					m_oResultsServer;
	TResults<EventHandler>			m_oResultsHandler;
    std::shared_ptr<TResults<EventProxy>> m_resultsProxy;

	DeviceServer					m_oDeviceServer;				///< Device server
	TDevice<MsgHandler>				m_oDeviceHandler;				///< Device handler
	
	

	//using VdrFolderMap_t = HashMap<std::string, VdrFileInfo>;
	using ProductFolderMap_t = Poco::ListMap<std::string, std::pair<Poco::UUID, uint32_t>>;

private:
    
	void ShowHelpInfo();
    void FindEnvironmentPath();
    bool CheckSequencesPath(std::string strSequencesPath);
    void ScanSequencesFolder();
    bool FindProductFolders(std::string & _folderName, ProductFolderMap_t  & _productFolderMap);
    bool ParseFolderName(std::string const & _folderName, std::string const & _regularExpression, uint32_t & _serialNumber);
    VdrFileType parseVdrFileName(std::string const & _fileName, uint32_t & _vdrNumber);

	bool m_simulateHW;
	bool m_helpRequested;

    std::string m_strBasepath;
    std::vector<SequenceType> m_Sequences;
    int m_NbImages;
    int m_NbSamples;
  

};
} // namespace viInspectionControl
} // namespace precitec

#endif /* APPMAIN_H_ */
