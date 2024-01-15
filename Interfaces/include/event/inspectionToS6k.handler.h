#pragma once
#include "event/inspectionToS6k.interface.h"
#include "server/eventHandler.h"

namespace precitec
{

namespace interface
{

	using namespace  message;

	template <>
	class TInspectionToS6k<EventHandler> : public Server<EventHandler>, public TInspectionToS6kMessageDefinition
	{
	public:
		EVENT_HANDLER( TInspectionToS6k );
	public:
		void registerCallbacks()
		{
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
			REGISTER_EVENT(SetS6K_QualityResults, setS6K_QualityResults);
			REGISTER_EVENT(SetS6K_CS_DataBlock, setS6K_CS_DataBlock);
		}

		void setS6K_QualityResults(Receiver &receiver)
		{
			int32_t oSeamNo; receiver.deMarshal(oSeamNo);
			struct S6K_QualityData_S1S2 oQualityData; receiver.deMarshal(oQualityData);
			getServer()->setS6K_QualityResults(oSeamNo, oQualityData);
		}

		void setS6K_CS_DataBlock(Receiver &receiver)
		{
			CS_TCP_MODE oSendCmd; receiver.deMarshal(oSendCmd);
			uint16_t oSeamNo; receiver.deMarshal(oSeamNo);
			uint16_t oBlockNo; receiver.deMarshal(oBlockNo);
			uint16_t oFirstMeasureInBlock; receiver.deMarshal(oFirstMeasureInBlock);
			uint16_t oMeasureCntInBlock; receiver.deMarshal(oMeasureCntInBlock);
			uint16_t oMeasuresPerResult; receiver.deMarshal(oMeasuresPerResult);
			uint16_t oValuesPerMeasure; receiver.deMarshal(oValuesPerMeasure);
			CS_BlockType oCS_DataBlock; receiver.deMarshal(oCS_DataBlock);
			getServer()->setS6K_CS_DataBlock(oSendCmd, oSeamNo, oBlockNo, oFirstMeasureInBlock, oMeasureCntInBlock, oMeasuresPerResult, oValuesPerMeasure, oCS_DataBlock);
		}

	private:
		TInspectionToS6k<AbstractInterface> * getServer()
		{
			return server_;
		}

	};

} // namespace interface
} // namespace precitec

