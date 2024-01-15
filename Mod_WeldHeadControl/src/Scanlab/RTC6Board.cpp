#include <unistd.h>
#include "viWeldHead/Scanlab/RTC6Board.h"

#include <iostream>
#include "rtc6.h"

namespace RTC6
{

Board::Board():
    m_oIPAdress("0.0.0.0"),
    m_oSubnetMask("0.0.0.0"),
    m_oMemorySizeListOne(4000),
    m_oMemorySizeListTwo(4000)
{
}

Board::Board(std::string p_oIPAdress, std::string p_oSubnetMask, unsigned int p_oMemorySizeListOne, unsigned int p_oMemorySizeListTwo):
    m_oIPAdress(p_oIPAdress),
    m_oSubnetMask(p_oSubnetMask),
    m_oMemorySizeListOne(p_oMemorySizeListOne),
    m_oMemorySizeListTwo(p_oMemorySizeListTwo)
{
}

Board::~Board()
{
    free_rtc6_dll();
}

void Board::set_RTCConnectInfo(std::string p_oIPAdress, std::string p_oSubnetMask)
{
    m_oIPAdress = p_oIPAdress;
    m_oSubnetMask = p_oSubnetMask;
}

void Board::set_MemorySizeListOne(unsigned int p_oMemorySizeListOne)
{
    m_oMemorySizeListOne = p_oMemorySizeListOne;
}

void Board::set_MemorySizeListTwo(unsigned int p_oMemorySizeListTwo)
{
    m_oMemorySizeListTwo = p_oMemorySizeListTwo;
}

void Board::get_RTCConnectInfo(std::string& p_rIPAdress, std::string& p_rSubnetMask)
{
    p_rIPAdress = m_oIPAdress;
    p_rSubnetMask = m_oSubnetMask;
}

unsigned int Board::get_MemorySizeListOne()
{
    return(m_oMemorySizeListOne);
}

unsigned int Board::get_MemorySizeListTwo()
{
    return(m_oMemorySizeListTwo);
}

void Board::GetVersionInfos(uint32_t& p_oDLLVersion, uint32_t& p_oBIOSVersion, uint32_t& p_oHexVersion, uint32_t& p_oRTCVersion, uint32_t& p_oSerialNumber)
{
    p_oDLLVersion = get_dll_version();
    p_oBIOSVersion = get_bios_version();
    p_oHexVersion = get_hex_version();
    p_oRTCVersion = get_rtc_version();
    p_oSerialNumber = get_serial_number();
}

void Board::terminate_DLL()
{
    free_rtc6_dll();
}

int Board::init()
{
    long int oErrorNumber = init_rtc6_dll();
    const auto oIpAddress = eth_convert_string_to_ip(m_oIPAdress.c_str());
    const auto oSubnetMask = eth_convert_string_to_ip(m_oSubnetMask.c_str());
    std::cout << "Start card detection - please wait...\n";
    eth_set_search_cards_timeout(10000);
    unsigned long int oNumCards = 0;
    for (int i = 0; i < 10; ++i)
    {
        oNumCards = eth_search_cards(oIpAddress, oSubnetMask);
        if (oNumCards > 0)
        {
            std::cout << "Found " << oNumCards << " RTC6 card(s).\n";
            break;
        }
        usleep(1000*1000);
    }

    if (oNumCards == 0 || oNumCards > 1)
    {
        std::cout << "Error - found " << oNumCards << " card(s).\n";
        free_rtc6_dll();
        return (-1);
    }

    const auto oCardNumber = eth_assign_card(1, 0);
    select_rtc(oCardNumber);

    reset_error(0xFFFFFFFF); // reset the accumulated RTC error

    oErrorNumber = load_program_file("/opt/Scanlab/Firmware/");
    if (oErrorNumber == 0)

    {
        std::cout << "Program file loaded successfully.\n";
    }
    else
    {
        std::cout << "Could not load program file. ErrorCode = " << oErrorNumber << "\n";
        free_rtc6_dll();
        return (-1);
    }

    config_list(m_oMemorySizeListOne, m_oMemorySizeListTwo);

    set_rtc6_mode();

    std::cout << "Initialization successful\n\n" << std::endl;
    return(0);
}
}
