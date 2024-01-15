#pragma once

#include "event/inspectionToS6k.interface.h"
#include "TCPCommunication.h"

namespace precitec
{

namespace tcpcommunication
{

using namespace interface;

/**
 * InspectionOutServer
 **/
class InspectionOutServer : public TInspectionToS6k<AbstractInterface>
{
    public:

    /**
     * Ctor.
     * @param _service Service
     * @return void
     **/
    InspectionOutServer(TCPCommunication& p_rTCPCommunication);
    virtual ~InspectionOutServer();
    /**
     * Uebergibt die Qualitaetsergebnisse fuer SOUVIS6000 an TCPCommunication
     * @param p_oSeamNo Nahtnummer fuer die Qualitaetsergebnisse
     * @param p_oQualityData Qualitaetsdaten wie fuer die TCP/IP-Kommunikation benoetigt
    **/
    void setS6K_QualityResults(int32_t p_oSeamNo, struct S6K_QualityData_S1S2 p_oQualityData) override
    {
        m_rTCPCommunication.setS6K_QualityResults(p_oSeamNo, p_oQualityData);
    }

	/**
	 * Uebergibt den Datenblock fuer Bindeflaechenmessung an TCPCommunication
	 * @param p_oSeamNo Nahtnummer fuer die Qualitaetsergebnisse
	 * @param p_oCS_DataBlock Datenblock zum Senden an anderes PostInspection system
	**/
	void setS6K_CS_DataBlock (CS_TCP_MODE p_oSendCmd, uint16_t p_oSeamNo, uint16_t p_oBlockNo, uint16_t p_oFirstMeasureInBlock,
                                          uint16_t p_oMeasureCntInBlock, uint16_t p_oMeasuresPerResult, uint16_t p_oValuesPerMeasure, CS_BlockType p_oCS_DataBlock) override
    {
        m_rTCPCommunication.setS6K_CS_DataBlock(p_oSendCmd, p_oSeamNo, p_oBlockNo, p_oFirstMeasureInBlock, p_oMeasureCntInBlock, p_oMeasuresPerResult, p_oValuesPerMeasure, p_oCS_DataBlock);
    }

    private:
        TCPCommunication &m_rTCPCommunication;
};

} // namespace tcpcommunication

} // namespace precitec


