#pragma once

#include "event/inspectionToS6k.interface.h"
#include "server/eventProxy.h"

namespace precitec
{

namespace interface
{

	template <>
	class TInspectionToS6k<EventProxy> : public Server<EventProxy>, public TInspectionToS6k<AbstractInterface>, public TInspectionToS6kMessageDefinition
	{
	public:
		/// Der CTor braucht die Klasse, die die Arbeit erledigt, also den eigentlichen Server
		TInspectionToS6k() : EVENT_PROXY_CTOR(TInspectionToS6k), TInspectionToS6k<AbstractInterface>()
		{
			//std::cout << "remote CTor::TRegistrar<Proxy> ohne Protokoll" << std::endl;
		}

		virtual ~TInspectionToS6k() {}

	public:

		void setS6K_QualityResults(int32_t p_oSeamNo, struct S6K_QualityData_S1S2 p_oQualityData) override
		{
			INIT_EVENT(SetS6K_QualityResults);
			signaler().marshal(p_oSeamNo);
			signaler().marshal(p_oQualityData);
			signaler().send();
		}

		void setS6K_CS_DataBlock (CS_TCP_MODE p_oSendCmd, uint16_t p_oSeamNo, uint16_t p_oBlockNo, uint16_t p_oFirstMeasureInBlock,
                                  uint16_t p_oMeasureCntInBlock, uint16_t p_oMeasuresPerResult, uint16_t p_oValuesPerMeasure, CS_BlockType p_oCS_DataBlock) override
		{
			INIT_EVENT(SetS6K_CS_DataBlock);
			signaler().marshal(p_oSendCmd);
			signaler().marshal(p_oSeamNo);
			signaler().marshal(p_oBlockNo);
			signaler().marshal(p_oFirstMeasureInBlock);
			signaler().marshal(p_oMeasureCntInBlock);
			signaler().marshal(p_oMeasuresPerResult);
			signaler().marshal(p_oValuesPerMeasure);
			signaler().marshal(p_oCS_DataBlock);
			signaler().send();
		}

	};

} // namespace interface
} // namespace precitec
