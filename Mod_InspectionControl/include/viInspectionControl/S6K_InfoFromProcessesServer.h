#ifndef S6KINFOFROMPROCESSESSERVER_H_
#define S6KINFOFROMPROCESSESSERVER_H_

#include "event/S6K_InfoFromProcesses.handler.h"
#include "VI_InspectionControl.h"

namespace precitec
{
    using namespace interface;

namespace ethercat
{

/**
 * S6K_InfoFromProcessesServer
 **/
class S6K_InfoFromProcessesServer : public TS6K_InfoFromProcesses<AbstractInterface>
{
    public:

    /**
     * Ctor.
     * @param _service Service
     * @return void
     **/
    S6K_InfoFromProcessesServer(VI_InspectionControl& p_rVI_InspectionControl);
    virtual ~S6K_InfoFromProcessesServer();

    virtual void maxSouvisSpeed(uint32_t p_oSpeed)
    {
        wmLog(eDebug, "S6K_InfoFromProcessesServer::maxSouvisSpeed %d\n", p_oSpeed);
        m_rVI_InspectionControl.maxSouvisSpeed(p_oSpeed);
    }

    virtual void souvisControlBits(bool p_oSouvisPresent, bool p_oSouvisSelected)
    {
        wmLog(eDebug, "S6K_InfoFromProcessesServer::souvisControlBits %d,%d\n", p_oSouvisPresent, p_oSouvisSelected);
        m_rVI_InspectionControl.souvisControlBits(p_oSouvisPresent, p_oSouvisSelected);
    }

    virtual void passS6K_CS_DataBlock_To_Inspect (uint32_t p_oProductNo, uint32_t p_oBatchID, uint16_t p_oSeamNo, uint16_t p_oBlockNo, uint16_t p_oFirstMeasureInBlock,
                                                  uint16_t p_oMeasureCntInBlock, uint16_t p_oMeasuresPerResult, uint16_t p_oValuesPerMeasure, CS_BlockType p_oCS_DataBlock)
    {
        m_rVI_InspectionControl.passS6K_CS_DataBlock_To_Inspect(p_oProductNo, p_oBatchID, p_oSeamNo, p_oBlockNo, p_oFirstMeasureInBlock, p_oMeasureCntInBlock, p_oMeasuresPerResult, p_oValuesPerMeasure, p_oCS_DataBlock);
    }

    virtual void numberOfPresentSeams(uint32_t p_oSeams)
    {
        wmLog(eDebug, "S6K_InfoFromProcessesServer::numberOfPresentSeams %d\n", p_oSeams);
        m_rVI_InspectionControl.numberOfPresentSeams(p_oSeams);
    }

    void productNoFromTCP(uint32_t p_oProductNo) override
    {
        wmLog(eDebug, "S6K_InfoFromProcessesServer::productNoFromTCP %d\n", p_oProductNo);
        m_rVI_InspectionControl.productNoFromTCP(p_oProductNo);
    }

    private:
        VI_InspectionControl &m_rVI_InspectionControl;
};

} // namespace ethercat
} // namespace precitec

#endif // S6KINFOFROMPROCESSESSERVER_H_

