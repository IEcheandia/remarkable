/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       03.10.2011
 *  @brief      Part of the inspectionOut Interface
 *  @details
 */

#ifndef INSPECTIONOUT_H_
#define INSPECTIONOUT_H_

#include <stdint.h>
#include <array>

#include "event/viWeldHeadPublish.h"

namespace precitec
{

namespace interface
{

#define CS_MEASURE_COUNT_PER_TCP_BLOCK   170
#define CS_VALUES_PER_MEASURE            4
#define CS_IDX_RESULT1                   0
#define CS_IDX_RESULT2                   1
#define CS_IDX_RESULT3                   2
#define CS_IDX_RESULT4                   3

    struct S6K_QualityData_S1S2
    {
        uint32_t m_oQualityError;
        uint32_t m_oQualityErrorCat1;
        uint32_t m_oQualityErrorCat2;
        uint32_t m_oExceptions;
        int16_t  m_oFirstErrorPos;
        int16_t  m_oAvgElementWidth;
        int16_t  m_oAvgHeightDiff;
        int16_t  m_oAvgConcavity;
        int16_t  m_oAvgConvexity;
        int16_t  m_oFirstPoreWidth;
    };

    typedef std::array<int16_t, CS_VALUES_PER_MEASURE> CS_BlockLineType;
    typedef std::array<CS_BlockLineType, CS_MEASURE_COUNT_PER_TCP_BLOCK> CS_BlockType;

    enum CS_TCP_MODE {eCS_TCP_OPEN_CONNECTION, eCS_TCP_CLOSE_CONNECTION, eCS_TCP_SEND_DATABLOCK};

} // namespace interface
} // namespace precitec

#endif /* INSPECTIONOUT_H_ */

