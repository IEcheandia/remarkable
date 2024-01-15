/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			AL, SB
 *  @date			2010
 *  @brief			Transmit system status			
 */

#ifndef SYSTEM_STATUS_H
#define SYSTEM_STATUS_H

#include <iostream>
#include "Poco/UUID.h"
#include "system/types.h"
#include "message/serializer.h"
#include "message/messageBuffer.h"
#include "common/measureTask.h"
#include "Poco/Version.h"



namespace precitec
{
namespace interface
{

	enum Hardware 		{ SiSoGrabber, PFCamera, DigIO };
	enum ReadyState 	{ SystemUp, SystemDown, SystemBusy };
	/// (WoR): should be NioType or FaultType; NIO is not an error*; current Name is confusing
	enum ErrorType		{ Hole, Pore, Splatter, Geometry };
	enum ErrorState		{ NoError=0, Error1=1, Error2=2, Error3=4};
	/// Modes in Kompatibility-Version
	enum OperationState { NormalMode, LiveMode, AutomaticMode, CalibrationMode, NotReadyMode, ProductTeachInMode,EmergencyStop,StartUpMode=NormalMode, ShutDownMode=NormalMode, NumMode=8 };

	enum WorkingState   { WaitForTrigger, Triggered, SeamSeriesChanged, Stopped };
	enum UpsState       { Online, OnBattery, LowBattery, ReplaceBattery, NoCommunication, Shutdown };
	// new States
	//enum OperationState { StartUpMode, NormalMode, LiveMode, AutomaticMode, ShutDownMode, NumModes };

     
	class ProductInfo: public system::message::Serializable {

	public:

		explicit ProductInfo(
				const Poco::UUID&	p_oProductId		= Poco::UUID(),
				const Poco::UUID&	p_oGraphId			= Poco::UUID(),
				const Poco::UUID&	p_oStationId		= Poco::UUID(),
				const Poco::UUID&	p_oMeasureTaskId	= Poco::UUID(),
				int					p_oSeamseries		= -1,
				int					p_oSeam				= -1,
				const std::string&	p_rProductName		= "",
				const std::string&	p_rGraphName		= "",
				const std::string&	p_rStationName		= "",
				const std::string&	p_rMeasureTaskName	= "",
				int 				p_rProductHwType 	= -1,
				uInt 				p_rProductSeriesNr 	= 0,
                double              p_rProcessingTime   = 0.0 )
			:
			m_oProductId		(p_oProductId),
			m_oGraphId			(p_oGraphId),
			m_oStationId		(p_oStationId),
			m_oMeasureTaskId	(p_oMeasureTaskId),
			m_oSeamseries		(p_oSeamseries),
			m_oSeam				(p_oSeam),
			m_oProductName		(p_rProductName),
			m_oGraphName		(p_rGraphName),
			m_oStationName		(p_rStationName),
			m_oMeasureTaskName	(p_rMeasureTaskName),
			m_oProductHwType	(p_rProductHwType),
			m_oProductSeriesNr	(p_rProductSeriesNr),
			m_oProcessingTime   (p_rProcessingTime)

		{}

		ProductInfo(const Poco::UUID& p_oProductId, const std::string& p_rProductName, const interface::MeasureTask&	p_rMeasureTask, int p_rProductHwType, uInt p_rProductSeriesNr)
			:
			m_oProductId		(p_oProductId),
			m_oGraphId			(p_rMeasureTask.graphID()),
			m_oMeasureTaskId	(p_rMeasureTask.taskID()),
			m_oSeamseries		(p_rMeasureTask.seamseries()),
			m_oSeam				(p_rMeasureTask.seam()),
			m_oProductName		(p_rProductName),
			m_oGraphName		(p_rMeasureTask.graphName()),
			m_oMeasureTaskName	(p_rMeasureTask.name()),
			m_oProductHwType	(p_rProductHwType),
			m_oProductSeriesNr	(p_rProductSeriesNr),
			m_oProcessingTime   (0.0)
		{}

		virtual void serialize(system::message::MessageBuffer & buffer) const {
			marshal(buffer, m_oProductId);
			marshal(buffer, m_oProductName);
			marshal(buffer, m_oSeamseries);
			marshal(buffer, m_oSeam);
			marshal(buffer, m_oGraphId);
			marshal(buffer, m_oGraphName);
			marshal(buffer, m_oStationId);
			marshal(buffer, m_oStationName);
			marshal(buffer, m_oMeasureTaskId);
			marshal(buffer, m_oMeasureTaskName);
			marshal(buffer, m_oProductHwType);
			marshal(buffer, m_oProductSeriesNr);
			marshal(buffer, m_oProcessingTime);
			marshal(buffer, m_oSeamLabel);
		}

		virtual void deserialize(system::message::MessageBuffer const& buffer) {
			deMarshal(buffer, m_oProductId);
			deMarshal(buffer, m_oProductName);
			deMarshal(buffer, m_oSeamseries);
			deMarshal(buffer, m_oSeam);
			deMarshal(buffer, m_oGraphId);
			deMarshal(buffer, m_oGraphName);
			deMarshal(buffer, m_oStationId);
			deMarshal(buffer, m_oStationName);
			deMarshal(buffer, m_oMeasureTaskId);
			deMarshal(buffer, m_oMeasureTaskName);
			deMarshal(buffer, m_oProductHwType);
			deMarshal(buffer, m_oProductSeriesNr);
			deMarshal(buffer, m_oProcessingTime);
			deMarshal(buffer, m_oSeamLabel);
		}

		Poco::UUID	m_oProductId;
		Poco::UUID	m_oGraphId;
		Poco::UUID	m_oStationId;
		Poco::UUID	m_oMeasureTaskId;

		int			m_oSeamseries;
		int			m_oSeam;

		std::string m_oProductName;
		std::string m_oGraphName;
		std::string m_oStationName;
		std::string m_oMeasureTaskName;

		int 		m_oProductHwType;
		uInt		m_oProductSeriesNr;
        
        double      m_oProcessingTime;
        std::string m_oSeamLabel;
	};


} // namespace interface
} // namespace precitec


#endif // SYSTEM_STATUS_H
