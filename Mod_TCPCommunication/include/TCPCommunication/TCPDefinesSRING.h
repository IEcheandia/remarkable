/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       2019
 *  @brief      Handles the TCP/IP communication with SOURING machine
 */

#ifndef TCPDEFINESSRING_H_
#define TCPDEFINESSRING_H_

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

#include <stdint.h>
#include <string>

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

namespace precitec
{

namespace tcpcommunication
{

struct SeamDataStruct
{
    int32_t m_oProductNumber;
    int16_t m_oSeamPresent;
    int32_t m_oSeamLength;
    int16_t m_oTargetDifference;
    int16_t m_oBlankThicknessLeft;
    int16_t m_oBlankThicknessRight;
    int16_t m_oSetupNumber;
    int16_t m_oMaxWeldSpeed;
    int16_t m_oMaxGapWidth;
    int16_t m_oReserved16_1;
    int16_t m_oReserved16_2;
    int16_t m_oReserved16_3;
    int32_t m_oReserved32_1;
    int32_t m_oReserved32_2;
    int32_t m_oReserved32_3;
};

const uint16_t MAX_S6K_SEAM = 24;
const uint16_t MAX_S6K_SEAM_SRING = 24;
const uint16_t MAX_S6K_SEAM_SSPEED = 1;
const uint16_t MAX_S6K_SEGMENTS = 12;
const uint16_t MAX_S6K_QUALITY_GRADES = 4;
const uint16_t QUALITY_GRADE_1 = 0;
const uint16_t QUALITY_GRADE_2 = 1;
const uint16_t QUALITY_GRADE_3 = 2;
const uint16_t QUALITY_GRADE_4 = 3;

const uint16_t START_POS_OF_LAST_SEGMENT = 32767;

// SOURING TCP/IP Ports
const uint16_t SOURING_S6K_SERVER_PORT_S1        = 1024;
const uint16_t SOURING_S6K_SERVER_PORT_S2U       = 1025;
const uint16_t SOURING_S6K_SERVER_PORT_S2L       = 1026;

const uint16_t SOURING_MC_SERVER_PORT_S1         = 1024;
const uint16_t SOURING_MC_SERVER_PORT_S2U        = 1025;
const uint16_t SOURING_MC_SERVER_PORT_S2L        = 1026;

// SOURING TCP/IP Blocks
const uint16_t DBLOCK_SRING_GLOBDATA_TYPE        = 1;
const uint16_t DBLOCK_SRING_GLOBDATA_VERSION     = 0x0150;

const uint16_t DBLOCK_SRING_SEAMDATA_1_TYPE      = 11;
const uint16_t DBLOCK_SRING_SEAMDATA_2_TYPE      = 12;
const uint16_t DBLOCK_SRING_SEAMDATA_3_TYPE      = 13;
const uint16_t DBLOCK_SRING_SEAMDATA_4_TYPE      = 14;
const uint16_t DBLOCK_SRING_SEAMDATA_5_TYPE      = 15;
const uint16_t DBLOCK_SRING_SEAMDATA_6_TYPE      = 16;
const uint16_t DBLOCK_SRING_SEAMDATA_7_TYPE      = 17;
const uint16_t DBLOCK_SRING_SEAMDATA_8_TYPE      = 18;
const uint16_t DBLOCK_SRING_SEAMDATA_9_TYPE      = 19;
const uint16_t DBLOCK_SRING_SEAMDATA_10_TYPE     = 20;
const uint16_t DBLOCK_SRING_SEAMDATA_11_TYPE     = 21;
const uint16_t DBLOCK_SRING_SEAMDATA_12_TYPE     = 22;
const uint16_t DBLOCK_SRING_SEAMDATA_13_TYPE     = 23;
const uint16_t DBLOCK_SRING_SEAMDATA_14_TYPE     = 24;
const uint16_t DBLOCK_SRING_SEAMDATA_15_TYPE     = 25;
const uint16_t DBLOCK_SRING_SEAMDATA_16_TYPE     = 26;
const uint16_t DBLOCK_SRING_SEAMDATA_17_TYPE     = 27;
const uint16_t DBLOCK_SRING_SEAMDATA_18_TYPE     = 28;
const uint16_t DBLOCK_SRING_SEAMDATA_19_TYPE     = 29;
const uint16_t DBLOCK_SRING_SEAMDATA_20_TYPE     = 30;
const uint16_t DBLOCK_SRING_SEAMDATA_21_TYPE     = 31;
const uint16_t DBLOCK_SRING_SEAMDATA_22_TYPE     = 32;
const uint16_t DBLOCK_SRING_SEAMDATA_23_TYPE     = 33;
const uint16_t DBLOCK_SRING_SEAMDATA_24_TYPE     = 34;
const uint16_t DBLOCK_SRING_SEAMDATA_VERSION     = 0x0150;

const uint16_t DBLOCK_SRING_QUALRESULT_1_TYPE    = 51;
const uint16_t DBLOCK_SRING_QUALRESULT_2_TYPE    = 52;
const uint16_t DBLOCK_SRING_QUALRESULT_3_TYPE    = 53;
const uint16_t DBLOCK_SRING_QUALRESULT_4_TYPE    = 54;
const uint16_t DBLOCK_SRING_QUALRESULT_5_TYPE    = 55;
const uint16_t DBLOCK_SRING_QUALRESULT_6_TYPE    = 56;
const uint16_t DBLOCK_SRING_QUALRESULT_7_TYPE    = 57;
const uint16_t DBLOCK_SRING_QUALRESULT_8_TYPE    = 58;
const uint16_t DBLOCK_SRING_QUALRESULT_9_TYPE    = 59;
const uint16_t DBLOCK_SRING_QUALRESULT_10_TYPE   = 60;
const uint16_t DBLOCK_SRING_QUALRESULT_11_TYPE   = 61;
const uint16_t DBLOCK_SRING_QUALRESULT_12_TYPE   = 62;
const uint16_t DBLOCK_SRING_QUALRESULT_13_TYPE   = 63;
const uint16_t DBLOCK_SRING_QUALRESULT_14_TYPE   = 64;
const uint16_t DBLOCK_SRING_QUALRESULT_15_TYPE   = 65;
const uint16_t DBLOCK_SRING_QUALRESULT_16_TYPE   = 66;
const uint16_t DBLOCK_SRING_QUALRESULT_17_TYPE   = 67;
const uint16_t DBLOCK_SRING_QUALRESULT_18_TYPE   = 68;
const uint16_t DBLOCK_SRING_QUALRESULT_19_TYPE   = 69;
const uint16_t DBLOCK_SRING_QUALRESULT_20_TYPE   = 70;
const uint16_t DBLOCK_SRING_QUALRESULT_21_TYPE   = 71;
const uint16_t DBLOCK_SRING_QUALRESULT_22_TYPE   = 72;
const uint16_t DBLOCK_SRING_QUALRESULT_23_TYPE   = 73;
const uint16_t DBLOCK_SRING_QUALRESULT_24_TYPE   = 74;
const uint16_t DBLOCK_SRING_QUALRESULT_VERSION   = 0x0150;

const uint16_t DBLOCK_SRING_STATUSDATA_TYPE      = 81;
const uint16_t DBLOCK_SSPEED_STATUSDATA_TYPE     = 61;
const uint16_t DBLOCK_SRING_STATUSDATA_VERSION   = 0x0150;

const uint16_t DBLOCK_SRING_INSPECTDATA_1_TYPE   = 111;
const uint16_t DBLOCK_SRING_INSPECTDATA_2_TYPE   = 112;
const uint16_t DBLOCK_SRING_INSPECTDATA_3_TYPE   = 113;
const uint16_t DBLOCK_SRING_INSPECTDATA_4_TYPE   = 114;
const uint16_t DBLOCK_SRING_INSPECTDATA_5_TYPE   = 115;
const uint16_t DBLOCK_SRING_INSPECTDATA_6_TYPE   = 116;
const uint16_t DBLOCK_SRING_INSPECTDATA_7_TYPE   = 117;
const uint16_t DBLOCK_SRING_INSPECTDATA_8_TYPE   = 118;
const uint16_t DBLOCK_SRING_INSPECTDATA_9_TYPE   = 119;
const uint16_t DBLOCK_SRING_INSPECTDATA_10_TYPE  = 120;
const uint16_t DBLOCK_SRING_INSPECTDATA_11_TYPE  = 121;
const uint16_t DBLOCK_SRING_INSPECTDATA_12_TYPE  = 122;
const uint16_t DBLOCK_SRING_INSPECTDATA_13_TYPE  = 123;
const uint16_t DBLOCK_SRING_INSPECTDATA_14_TYPE  = 124;
const uint16_t DBLOCK_SRING_INSPECTDATA_15_TYPE  = 125;
const uint16_t DBLOCK_SRING_INSPECTDATA_16_TYPE  = 126;
const uint16_t DBLOCK_SRING_INSPECTDATA_17_TYPE  = 127;
const uint16_t DBLOCK_SRING_INSPECTDATA_18_TYPE  = 128;
const uint16_t DBLOCK_SRING_INSPECTDATA_19_TYPE  = 129;
const uint16_t DBLOCK_SRING_INSPECTDATA_20_TYPE  = 130;
const uint16_t DBLOCK_SRING_INSPECTDATA_21_TYPE  = 131;
const uint16_t DBLOCK_SRING_INSPECTDATA_22_TYPE  = 132;
const uint16_t DBLOCK_SRING_INSPECTDATA_23_TYPE  = 133;
const uint16_t DBLOCK_SRING_INSPECTDATA_24_TYPE  = 134;
const uint16_t DBLOCK_SRING_INSPECTDATA_VERSION  = 0x0150;

// CrossSection TCP/IP Port
const uint16_t CROSS_SECTION_TRAILING_SERVER_PORT = 1027;

// CrossSection TCP/IP Block
const uint16_t DBLOCK_CROSS_SECTION_TYPE         = 150;
const uint16_t DBLOCK_CROSS_SECTION_VERSION      = 0x0100;

// Acknowledge TCP/IP Block
const uint16_t DBLOCK_ACKN_TYPE                  = 99;
const uint16_t DBLOCK_ACKN_VERSION               = 0x0100;
const uint16_t DBLOCK_ACKN_OK                    = 111;
const uint16_t DBLOCK_ACKN_ERROR                 = 200;

} // namespace tcpcommunication

} // namespace precitec

#endif /* TCPDEFINESSRING_H_ */


