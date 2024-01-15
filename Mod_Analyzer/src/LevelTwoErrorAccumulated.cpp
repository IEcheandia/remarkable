/**
 *
 *  @file       LevelTwoErrorAccumulated.cpp
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Daniel Zumkeller
 *  @date       20.03.2020
 *  @brief      Level Two Type Sum Error Implementation, which defines the global product status dependant on the accumulated NIO seam count. Derived from the generic LevelTwoError.
 * 
 *  This class is derived from the base class lying in LevelTwoError.cpp file
 */

#include "analyzer/LevelTwoError.h"

namespace precitec
{
    using namespace interface;
namespace analyzer
{

using precitec::interface::ResultArgs;
using precitec::interface::SmpResultPtr;
using precitec::interface::MeasureTask;
using precitec::interface::ResultType;
using precitec::interface::TResults;


///////////////////////////////////////////////////////////
// LEVEL 2 (seam level) accumulated error check          //
///////////////////////////////////////////////////////////
LevelTwoErrorAccumulated::~LevelTwoErrorAccumulated()
{
    // nothing to handle yet.
}

void LevelTwoErrorAccumulated::init()
{
    this->LevelTwoError::init();
    m_oErrorType = ResultType::LevelTwoErrorAccumulated;
    m_oLastSeamIndex = interface::tSeamIndex(-1, -1, -1);
#ifdef __debugLevelTwoError
    wmLog(eDebug, "LevelTwoErrorAccumulated::init()\n");
#endif
}

void LevelTwoErrorAccumulated::reset()
{
#ifdef __debugLevelTwoError
  std::ostringstream oDebugString ;
  oDebugString << "LevelTwoErrorAccumulated::reset() scope <" << scope() << ">" << std::endl;
  wmLog(eDebug, oDebugString.str());
#endif
  this->LevelTwoError::reset();
  m_oLastSeamIndex = interface::tSeamIndex(-1, -1, -1);
}

ResultEvaluation LevelTwoErrorAccumulated::testSumErrorLevel2(const Poco::SharedPtr<SumError> &p_oSumErr, const interface::tSeamIndex index)
{
    ResultEvaluation oTypeOrScope(eNotMasked);
    if (m_oResultTypes.size() <= 0)   // nothing to surveil...
    {
        wmLog(eDebug, "LevelTwoErrorAccumulated::testSumErrorLevel2() m_oResultTypes.size() <= 0; nothing to monitor..  return!\n");
        return oTypeOrScope;
    }
    if (m_oHasBeenSend) //no need to recalculate if this LevelTwoError has already triggered before...
    {
        return eSumErrorNotTriggered;
    }
    
    std::int32_t oSumErrorThrowValue = p_oSumErr->errorType();
    
    #ifdef __debugLevelTwoError
        std::int32_t oSumErrorTypeValue = p_oSumErr->sumErrorType();
        
        std::int32_t oSeamSerie = std::get<0>(index);
        std::int32_t oSeam = std::get<1>(index);
        std::int32_t oSeamInterval = std::get<2>(index);
        wmLog(eDebug, "LevelTwoErrorAccumulated::testSumErrorLevel2() Incoming sumError [%d] with type [%d] at scope [%d|%d|%d]\n", oSumErrorTypeValue, oSumErrorThrowValue, oSeamSerie, oSeam, oSeamInterval);
        wmLog(eDebug, "LevelTwoErrorAccumulated::testSumErrorLevel2() Violation currently at [%f / %f]\n", m_oFlawedViolation, m_oMaxViolation);
    #endif      
    
    
    bool nioInSeamDetected = false;
    
    if (m_checkSpecificError)
    {
        // the local selected error to surveil
        std::int32_t oSumErrorToWatch = m_oResultTypes[0];
        
        #ifdef __debugLevelTwoError
            wmLog(eDebug, "LevelTwoErrorAccumulated::testSumErrorLevel2() ErrorLevelTwo type:[%d] watching out for specific error [%d]\n", sumErrorType(), oSumErrorToWatch);
        #endif      
            
        if (oSumErrorToWatch != oSumErrorThrowValue)
        {
            #ifdef __debugLevelTwoError
                wmLog(eDebug, "LevelTwoErrorAccumulated::testSumErrorLevel2() Monitoring Error [%d], but incoming was [%d] - nothing to do, return\n", oSumErrorToWatch, oSumErrorThrowValue);
            #endif
            return eSumErrorNotTriggered;
        } 
        else if (valueIsAlreadyHandled(oSumErrorToWatch, index))
        {
            // same error on the same seam already found, do not count again
            return eSumErrorNotTriggered;
        }
        nioInSeamDetected = true;
    }
    else //check all errors
    {
        for (std::size_t i = 0; i < m_oResultTypes.size(); i++)
        {
            if (!valueIsAlreadyHandled(m_oResultTypes[i], index) )
            {
                nioInSeamDetected = true;
                #ifdef __debugLevelTwoError
                    wmLog(eDebug, "LevelTwoErrorAccumulated::testSumErrorLevel2() SumError [%d] with type [%d] detected NIO at scope [%d|%d|%d]\n", oSumErrorTypeValue, oSumErrorThrowValue, oSeamSerie, oSeam, oSeamInterval);
                #endif
                break;
            }
        }
    }
    
    if (!nioInSeamDetected)
    {
        return eSumErrorNotTriggered;
    }
    else 
    {
        m_oFlawedViolation += 1;
        #ifdef __debugLevelTwoError
            wmLog(eDebug, "LevelTwoErrorAccumulated::testSumErrorLevel2() NIO detected in seamSeries/Seam [%d|%d]. Now at %f%% to trigger Level2Error!\n", oSeamSerie, oSeam, (100.0 * (m_oFlawedViolation / m_oMaxViolation)) );
        #endif
        if (m_oFlawedViolation >= m_oMaxViolation)
        {
            #ifdef __debugLevelTwoError
                wmLog(eInfo, "LevelTwoErrorAccumulated::testSumErrorLevel2() Error condition reached! -> Informing Host\n", m_oMaxViolation);
            #endif
            m_oHasBeenSend = true;
            return eSumErrorTriggered;
        }
        else
        {
            return eSumErrorNotTriggered;
        }
    }
    
}


} // namespace interface
} // namespace precitec
