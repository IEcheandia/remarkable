#include "common/systemConfiguration.h"

#include "../include/viService/serviceToGuiServer.h"

namespace precitec {
namespace viService {

#define DEBUG_SERVICETOGUISERVER     0

    ServiceToGuiServer::ServiceToGuiServer(TviServiceToGUI<EventProxy>& viServiceToGuiProxy):
        viServiceToGuiProxy_(viServiceToGuiProxy),
        m_oSlaveInfoFromECATValid(false),
        m_oSlaveInfoFromFieldbusValid(false)
    {
        // SystemConfig Switch for FieldbusViaSeparateFieldbusBoard
        m_oFieldbusViaSeparateFieldbusBoard = SystemConfiguration::instance().getBool("FieldbusViaSeparateFieldbusBoard", false);
        wmLog(eDebug, "m_oFieldbusViaSeparateFieldbusBoard (bool): %d\n", m_oFieldbusViaSeparateFieldbusBoard);
        // SystemConfig Switch for EtherCATMasterIsActive
        m_etherCATMasterIsActive = SystemConfiguration::instance().getBool("EtherCATMasterIsActive", true);
        wmLog(eDebug, "m_etherCATMasterIsActive (bool): %d\n", m_etherCATMasterIsActive);

        m_pdInVector.reserve(100);
        m_pdOutVector.reserve(100);

        m_oFieldbusInputSize = 20;
        for(int i = 0;i < MAX_FIELDBUS_INPUT_BYTES;i++)
        {
            m_oFieldbusInputBuffer[i] = 0;;
        }
        m_oFieldbusOutputSize = 20;
        for(int i = 0;i < MAX_FIELDBUS_OUTPUT_BYTES;i++)
        {
            m_oFieldbusOutputBuffer[i] = 0;
        }
    }

    ServiceToGuiServer::~ServiceToGuiServer()
    {
    }

    void ServiceToGuiServer::SetTransferModus(bool onOff)
    {
        m_sendDataToHost = onOff;
    }

    void ServiceToGuiServer::IncomingDataECAT(short inSize, const char* inData, short outSize, const char* outData)
    {
        if (!m_etherCATMasterIsActive) // EtherCATMaster is running but SystemConfig switch is off
        {
            return;
        }

        if (m_sendDataToHost)
        {
            char oInputBuffer[MAX_SIZE];
            memcpy(oInputBuffer, inData, inSize);
            memcpy((oInputBuffer + inSize), m_oFieldbusInputBuffer, m_oFieldbusInputSize);
            m_pdInVector.emplace_back((inSize + m_oFieldbusInputSize), oInputBuffer);

            char oOutputBuffer[MAX_SIZE];
            memcpy(oOutputBuffer, outData, outSize);
            memcpy((oOutputBuffer + outSize), m_oFieldbusOutputBuffer, m_oFieldbusOutputSize);
            m_pdOutVector.emplace_back((outSize + m_oFieldbusOutputSize), oOutputBuffer);

            //beide Vektoren sind gleich gross...
            if (m_pdInVector.size() >= 100 )
            {
                ProcessImage(m_pdInVector,m_pdOutVector);
                m_pdInVector.clear();
                m_pdOutVector.clear();
            }
        }
    }

    void ServiceToGuiServer::IncomingDataFieldbus(short inSize, const char* inData, short outSize, const char* outData)
    {
        if (m_etherCATMasterIsActive)
        {
            m_oFieldbusInputSize = inSize;
            memcpy(m_oFieldbusInputBuffer, inData, inSize);
            m_oFieldbusOutputSize = outSize;
            memcpy(m_oFieldbusOutputBuffer, outData, outSize);
        }
        else
        {
            if (m_sendDataToHost)
            {
                char oInputBuffer[MAX_SIZE];
                memcpy(oInputBuffer, inData, inSize);
                m_pdInVector.emplace_back(inSize, oInputBuffer);

                char oOutputBuffer[MAX_SIZE];
                memcpy(oOutputBuffer, outData, outSize);
                m_pdOutVector.emplace_back(outSize, oOutputBuffer);

                if (m_pdInVector.size() >= 100 )
                {
                    ProcessImage(m_pdInVector,m_pdOutVector);
                    m_pdInVector.clear();
                    m_pdOutVector.clear();
                }
            }
        }
    }

    void ServiceToGuiServer::ProcessImage(ProcessDataVector& input, ProcessDataVector& output)
    {
        viServiceToGuiProxy_.ProcessImage(input, output);
    }

    void ServiceToGuiServer::BuildAndSendCollectedSlaveInfo(void)
    {
        // find the last used bit offset in EtherCAT bit stream
        EC_T_DWORD oMaxBitOffset = -1; // EC_T_DWORD is uint32_t !
        EC_T_DWORD oLastBitSize = 0;
        for(auto oLocalInfo: m_slaveInfoECAT)
        {
            if ((int)oLocalInfo.dwPdOffsIn > (int)oMaxBitOffset)
            {
                oMaxBitOffset = oLocalInfo.dwPdOffsIn;
                oLastBitSize = oLocalInfo.dwPdSizeIn;
            }
            if ((int)oLocalInfo.dwPdOffsOut > (int)oMaxBitOffset)
            {
                oMaxBitOffset = oLocalInfo.dwPdOffsOut;
                oLastBitSize = oLocalInfo.dwPdSizeOut;
            }
        }
        // complete both SlaveInfo vectors and insert correct bit offsets for fieldbus data
        std::vector <EC_T_GET_SLAVE_INFO> oSlaveInfoCollected(m_slaveInfoECAT);
        for(auto oLocalInfo: m_slaveInfoFieldbus)
        {
            oLocalInfo.dwPdOffsIn = oMaxBitOffset + oLastBitSize;
            oMaxBitOffset = oLocalInfo.dwPdOffsIn;
            oLastBitSize = oLocalInfo.dwPdSizeIn;
            oLocalInfo.dwPdOffsOut = oMaxBitOffset + oLastBitSize;
            oMaxBitOffset = oLocalInfo.dwPdOffsOut;
            oLastBitSize = oLocalInfo.dwPdSizeOut;
            oSlaveInfoCollected.push_back(oLocalInfo);
        }
        SlaveInfo oSlaveInfoComplete(m_countSlavesECAT + m_countSlavesFieldbus);
        oSlaveInfoComplete.FillBuffer(oSlaveInfoCollected.data());
#if DEBUG_SERVICETOGUISERVER
        printf("-----> ServiceToGuiServer::oSlaveInfoComplete: oSlaveInfoComplete.GetSize(%d)\n", oSlaveInfoComplete.GetSize());
        for(int i = 0;i < oSlaveInfoComplete.GetSize();i++)
        {
            EC_T_GET_SLAVE_INFO oSlaveInfo = oSlaveInfoComplete.GetInfoAt(i);
            printf("-----> ServiceToGuiServer::oSlaveInfoComplete: abyDeviceName: (%s)\n", oSlaveInfo.abyDeviceName);
            printf("-----> ServiceToGuiServer::oSlaveInfoComplete: wCfgPhyAddress:(%d)\n", oSlaveInfo.wCfgPhyAddress);
            printf("-----> ServiceToGuiServer::oSlaveInfoComplete: dwVendorId:    (0x%04X)\n", oSlaveInfo.dwVendorId);
            printf("-----> ServiceToGuiServer::oSlaveInfoComplete: dwProductCode: (0x%04X)\n", oSlaveInfo.dwProductCode);
            printf("-----> ServiceToGuiServer::oSlaveInfoComplete: dwPdOffsIn:    (%d)\n", oSlaveInfo.dwPdOffsIn);
            printf("-----> ServiceToGuiServer::oSlaveInfoComplete: dwPdSizeIn:    (%d)\n", oSlaveInfo.dwPdSizeIn);
            printf("-----> ServiceToGuiServer::oSlaveInfoComplete: dwPdOffsOut:   (%d)\n", oSlaveInfo.dwPdOffsOut);
            printf("-----> ServiceToGuiServer::oSlaveInfoComplete: dwPdSizeOut:   (%d)\n", oSlaveInfo.dwPdSizeOut);
        }
#endif
        // send completed SlaveInfo to GUI
        viServiceToGuiProxy_.SlaveInfoECAT(oSlaveInfoComplete.GetSize(),oSlaveInfoComplete); // interface viServiceToGui

        if (m_pServiceCallbackFunction)
        {
            m_pServiceCallbackFunction(oSlaveInfoComplete);
        }
    }

    void ServiceToGuiServer::BuildAndSendFieldbusOnlySlaveInfo(void)
    {
        EC_T_DWORD oMaxBitOffset{0};
        EC_T_DWORD oLastBitSize{0};
        std::vector<EC_T_GET_SLAVE_INFO> oSlaveInfoCollected{};
        for (auto oLocalInfo : m_slaveInfoFieldbus)
        {
            oLocalInfo.dwPdOffsIn = oMaxBitOffset + oLastBitSize;
            oMaxBitOffset = oLocalInfo.dwPdOffsIn;
            oLastBitSize = oLocalInfo.dwPdSizeIn;
            oLocalInfo.dwPdOffsOut = oMaxBitOffset + oLastBitSize;
            oMaxBitOffset = oLocalInfo.dwPdOffsOut;
            oLastBitSize = oLocalInfo.dwPdSizeOut;
            oSlaveInfoCollected.push_back(oLocalInfo);
        }
        SlaveInfo oSlaveInfoComplete(m_countSlavesFieldbus);
        oSlaveInfoComplete.FillBuffer(oSlaveInfoCollected.data());
#if DEBUG_SERVICETOGUISERVER
        printf("-----> ServiceToGuiServer::oSlaveInfoComplete: oSlaveInfoComplete.GetSize(%d)\n", oSlaveInfoComplete.GetSize());
        for(int i = 0;i < oSlaveInfoComplete.GetSize();i++)
        {
            EC_T_GET_SLAVE_INFO oSlaveInfo = oSlaveInfoComplete.GetInfoAt(i);
            printf("-----> ServiceToGuiServer::oSlaveInfoComplete: abyDeviceName: (%s)\n", oSlaveInfo.abyDeviceName);
            printf("-----> ServiceToGuiServer::oSlaveInfoComplete: wCfgPhyAddress:(%d)\n", oSlaveInfo.wCfgPhyAddress);
            printf("-----> ServiceToGuiServer::oSlaveInfoComplete: dwVendorId:    (0x%04X)\n", oSlaveInfo.dwVendorId);
            printf("-----> ServiceToGuiServer::oSlaveInfoComplete: dwProductCode: (0x%04X)\n", oSlaveInfo.dwProductCode);
            printf("-----> ServiceToGuiServer::oSlaveInfoComplete: dwPdOffsIn:    (%d)\n", oSlaveInfo.dwPdOffsIn);
            printf("-----> ServiceToGuiServer::oSlaveInfoComplete: dwPdSizeIn:    (%d)\n", oSlaveInfo.dwPdSizeIn);
            printf("-----> ServiceToGuiServer::oSlaveInfoComplete: dwPdOffsOut:   (%d)\n", oSlaveInfo.dwPdOffsOut);
            printf("-----> ServiceToGuiServer::oSlaveInfoComplete: dwPdSizeOut:   (%d)\n", oSlaveInfo.dwPdSizeOut);
        }
#endif
        // send modified SlaveInfo to GUI
        viServiceToGuiProxy_.SlaveInfoECAT(oSlaveInfoComplete.GetSize(),oSlaveInfoComplete); // interface viServiceToGui

        if (m_pServiceCallbackFunction)
        {
            m_pServiceCallbackFunction(oSlaveInfoComplete);
        }
    }

    void ServiceToGuiServer::SlaveInfoECAT(short count, SlaveInfo info)
    {
        if (!m_etherCATMasterIsActive) // EtherCATMaster is running but SystemConfig switch is off
        {
            return;
        }

#if DEBUG_SERVICETOGUISERVER
        printf("-----> ServiceToGuiServer::SlaveInfoECAT: info.GetSize(%d)\n", info.GetSize());
        for(int i = 0;i < info.GetSize();i++)
        {
            EC_T_GET_SLAVE_INFO oSlaveInfo = info.GetInfoAt(i);
            printf("-----> ServiceToGuiServer::SlaveInfoECAT: abyDeviceName: (%s)\n", oSlaveInfo.abyDeviceName);
            printf("-----> ServiceToGuiServer::SlaveInfoECAT: wCfgPhyAddress:(%d)\n", oSlaveInfo.wCfgPhyAddress);
            printf("-----> ServiceToGuiServer::SlaveInfoECAT: dwVendorId:    (0x%04X)\n", oSlaveInfo.dwVendorId);
            printf("-----> ServiceToGuiServer::SlaveInfoECAT: dwProductCode: (0x%04X)\n", oSlaveInfo.dwProductCode);
            printf("-----> ServiceToGuiServer::SlaveInfoECAT: dwPdOffsIn:    (%d)\n", oSlaveInfo.dwPdOffsIn);
            printf("-----> ServiceToGuiServer::SlaveInfoECAT: dwPdSizeIn:    (%d)\n", oSlaveInfo.dwPdSizeIn);
            printf("-----> ServiceToGuiServer::SlaveInfoECAT: dwPdOffsOut:   (%d)\n", oSlaveInfo.dwPdOffsOut);
            printf("-----> ServiceToGuiServer::SlaveInfoECAT: dwPdSizeOut:   (%d)\n", oSlaveInfo.dwPdSizeOut);
        }
#endif
        m_countSlavesECAT = count;
        m_slaveInfoECAT = info.GetSlaveInfoVector();
        m_oSlaveInfoFromECATValid = true;
        if (m_oFieldbusViaSeparateFieldbusBoard)
        {
            if (m_etherCATMasterIsActive)
            {
                if ((m_oSlaveInfoFromECATValid) && (m_oSlaveInfoFromFieldbusValid))
                {
                    BuildAndSendCollectedSlaveInfo();
                }
            }
        }
        else
        {
            viServiceToGuiProxy_.SlaveInfoECAT(count,info); // interface viServiceToGui
        }
    }

    void ServiceToGuiServer::SlaveInfoFieldbus(short count, SlaveInfo info)
    {
#if DEBUG_SERVICETOGUISERVER
        printf("-----> ServiceToGuiServer::SlaveInfoFieldbus: info.GetSize(%d)\n", info.GetSize());
        for(int i = 0;i < info.GetSize();i++)
        {
            EC_T_GET_SLAVE_INFO oSlaveInfo = info.GetInfoAt(i);
            printf("-----> ServiceToGuiServer::SlaveInfoFieldbus: abyDeviceName: (%s)\n", oSlaveInfo.abyDeviceName);
            printf("-----> ServiceToGuiServer::SlaveInfoFieldbus: wCfgPhyAddress:(%d)\n", oSlaveInfo.wCfgPhyAddress);
            printf("-----> ServiceToGuiServer::SlaveInfoFieldbus: dwVendorId:    (0x%04X)\n", oSlaveInfo.dwVendorId);
            printf("-----> ServiceToGuiServer::SlaveInfoFieldbus: dwProductCode: (0x%04X)\n", oSlaveInfo.dwProductCode);
            printf("-----> ServiceToGuiServer::SlaveInfoFieldbus: dwPdOffsIn:    (%d)\n", oSlaveInfo.dwPdOffsIn);
            printf("-----> ServiceToGuiServer::SlaveInfoFieldbus: dwPdSizeIn:    (%d)\n", oSlaveInfo.dwPdSizeIn);
            printf("-----> ServiceToGuiServer::SlaveInfoFieldbus: dwPdOffsOut:   (%d)\n", oSlaveInfo.dwPdOffsOut);
            printf("-----> ServiceToGuiServer::SlaveInfoFieldbus: dwPdSizeOut:   (%d)\n", oSlaveInfo.dwPdSizeOut);
        }
#endif
        m_countSlavesFieldbus = count;
        m_slaveInfoFieldbus = info.GetSlaveInfoVector();
        m_oSlaveInfoFromFieldbusValid = true;
        if (m_oFieldbusViaSeparateFieldbusBoard)
        {
            if (m_etherCATMasterIsActive)
            {
                if ((m_oSlaveInfoFromECATValid) && (m_oSlaveInfoFromFieldbusValid))
                {
                    BuildAndSendCollectedSlaveInfo();
                }
            }
            else
            {
                BuildAndSendFieldbusOnlySlaveInfo();
            }
        }
        else
        {
            // sole fieldbus connection is not provided
        }
    }

    void ServiceToGuiServer::sendSlaveInfo()
    {
        if (m_oFieldbusViaSeparateFieldbusBoard)
        {
            if (m_etherCATMasterIsActive)
            {
                if ((m_oSlaveInfoFromECATValid) && (m_oSlaveInfoFromFieldbusValid))
                {
                    BuildAndSendCollectedSlaveInfo();
                }
            }
            else
            {
                BuildAndSendFieldbusOnlySlaveInfo();
            }
        }
        else
        {
            SlaveInfo info(m_slaveInfoECAT.size());
            info.FillBuffer(m_slaveInfoECAT.data());
            viServiceToGuiProxy_.SlaveInfoECAT(m_slaveInfoECAT.size(), info);
        }
    }

    void ServiceToGuiServer::ConfigInfo(std::string config)
    {
        std::string configPath(getenv("WM_BASE_DIR"));
        configPath += "/config/VI_Config.xml";

        std::ifstream in(configPath);
        std::string configString((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

        viServiceToGuiProxy_.ConfigInfo(configString);
    }

    void ServiceToGuiServer::ecatAllDataIn(uint16_t size, stdVecUINT8 data)
    {
        IncomingDataECAT(size, reinterpret_cast<const char*>(data.data()), size, reinterpret_cast<const char*>(data.data()));
    }

    void ServiceToGuiServer::fieldbusAllDataIn(uint16_t size, stdVecUINT8 data)
    {
        IncomingDataFieldbus(size, reinterpret_cast<const char*>(data.data()), size, reinterpret_cast<const char*>(data.data()));
    }

    void ServiceToGuiServer::connectCallback(ServiceCallbackFunction p_pServiceCallbackFunction)
    {
        m_pServiceCallbackFunction = p_pServiceCallbackFunction;
    }

} // namespace viService
} // namespace precitec

