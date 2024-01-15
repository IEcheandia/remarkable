/**
 *
 *  @file       LevelTwoError.cpp
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Daniel Zumkeller
 *  @date       20.03.2020
 *  @brief      Base class for all LevelTwoError Sum Error types, checking each seam for NIO decision and calculating the global NIO dependant on the status.
 * 
 */

#include "analyzer/LevelTwoError.h"

namespace precitec
{
namespace analyzer
{

using precitec::interface::ResultArgs;
using precitec::interface::SmpResultPtr;
using precitec::interface::MeasureTask;
using precitec::interface::ResultType;
using precitec::interface::TResults;



//////////////////////////////////////////////////////
// LEVEL 2 (SEAM LEVEL) ERRORS - BASE CLASS         //
//////////////////////////////////////////////////////
LevelTwoError::~LevelTwoError()
{
}

LevelTwoError& LevelTwoError::operator=(LevelTwoError const &rhs)
{
    if (this != &rhs)
    {
        this->SumError::operator=(rhs);
        copyValues(this, rhs);
    }
    return *this;
}

void LevelTwoError::init()
{
    reset();
    m_oScope = eScopeSeam;
    m_oFlawedViolation = 0.0;
    m_oErrorType = ResultType::SumErrorNone;
    setTriggerBase(TriggerBase::ePositionTrigger);
}

void LevelTwoError::reset()
{
    m_oFlawedViolation = 0.0;
    this->SumError::reset();
}

void LevelTwoError::setErrorType(const std::int32_t p_oType)
{
    m_oErrorType = (ResultType)p_oType;
#ifdef __debugLevelTwoError
    wmLog( eDebug, "LevelTwoError::setErrorType m_oErrorType <%d>\n", p_oType);
#endif
}

void LevelTwoError::copyValues(LevelTwoError* dest, LevelTwoError const &src)
{
    dest->SumError::copyValues(dest, src);
    dest->m_oMaxViolation = src.m_oMaxViolation;
    dest->m_oFlawedViolation = src.m_oFlawedViolation;
}

ResultEvaluationData LevelTwoError::testResult(const ResultArgs& p_rRes, const interface::ResultArgs& lwmTriggerSignal)
{
    ResultEvaluationData result{eNotMasked, {}, {}, {}, {}};
    // nothing to do, we want to check not the result but the sumErrors!
    return result;
}

bool LevelTwoError::valueIsAlreadyHandled(std::int32_t sumErrorValue, const interface::tSeamIndex index)
{
    // test for the seams, we count only one error for each seam
    bool oReturnValue = true;
    // store the actual result
    std::int32_t oSeamSerie = std::get<0>(index);
    std::int32_t oSeam = std::get<1>(index);
    if ((std::get<0>(m_oLastSeamIndex) != oSeamSerie) || (std::get<1>(m_oLastSeamIndex) != oSeam))
    {
        #ifdef __debugLevelTwoError
            std::int32_t oSeamInterval = std::get<2>(index);
            std::ostringstream  oDebugString;
            oDebugString << "LevelTwoError::valueIsAlreadyHandled() different!" << " old <" << std::get<0>(m_oLastSeamIndex) << "><" << std::get<1>(m_oLastSeamIndex) << "><" << std::get<2>(m_oLastSeamIndex) << ">";
            oDebugString << " new <" << oSeamSerie << "><" << oSeam << "><" << oSeamInterval << "> return false" << std::endl;
            wmLog(eDebug, oDebugString.str());
        #endif
        m_oLastSeamIndex = index;
        oReturnValue = false;
    }
    else
    {
        #ifdef __debugLevelTwoError
            std::ostringstream  oDebugString;
            oDebugString << "LevelTwoError::valueIsAlreadyHandled() equal!" << " <" << std::get<0>(m_oLastSeamIndex) << "><" << std::get<1>(m_oLastSeamIndex) << "><" << std::get<2>(m_oLastSeamIndex) << ">  return true"  << std::endl;
            wmLog(eDebug, oDebugString.str());
        #endif
    }

    return oReturnValue;
}


} // namespace interface
} // namespace precitec
