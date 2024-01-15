#pragma once

#include <string>
#include <cstdint>

namespace RTC6
{
class Board
{
    public:
        Board();
        Board(std::string p_oIPAdress, std::string p_oSubnetMask, unsigned int p_oMemorySizeListOne, unsigned int p_oMemorySizeListTwo);
        ~Board();
        void set_RTCConnectInfo(std::string p_oIPAdress, std::string p_oSubnetMask);
        void set_MemorySizeListOne(unsigned int p_oMemorySizeListOne);
        void set_MemorySizeListTwo(unsigned int p_oMemorySizeListTwo);
        void get_RTCConnectInfo(std::string& p_rIPAdress, std::string& p_rSubnetMask);
        unsigned int get_MemorySizeListOne();
        unsigned int get_MemorySizeListTwo();
        void GetVersionInfos(uint32_t& p_oDLLVersion, uint32_t& p_oBIOSVersion, uint32_t& p_oHexVersion, uint32_t& p_oRTCVersion, uint32_t& p_oSerialNumber);
        void terminate_DLL();
        int init();

    private:
        std::string m_oIPAdress;
        std::string m_oSubnetMask;
        unsigned int m_oMemorySizeListOne;
        unsigned int m_oMemorySizeListTwo;
};
}
