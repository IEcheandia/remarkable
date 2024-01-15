/**
 *
 *  @file       LevelTwoErrorAdjacent.cpp
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Daniel Zumkeller
 *  @date       20.03.2020
 *  @brief      Level Two Type Sum Error Implementation, which defines the global product status dependant on the count of adjacent (following) NIO seam count. Derived from the generic LevelTwoError.
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


//////////////////////////////////////////////////////
// LEVEL 2 ERROR: consecutive seam error check      //
//////////////////////////////////////////////////////
LevelTwoErrorAdjacent::~LevelTwoErrorAdjacent()
{
    // nothing to handle yet.
}

void LevelTwoErrorAdjacent::init()
{
    this->LevelTwoError::init();
    m_oErrorType = ResultType::LevelTwoErrorAdjacent;
    m_oLastTriggeredSeamSeries = -1;
    m_oLastTriggeredSeam = -1;
    m_oLastTriggeredSeamInterval = -1;
    m_oLastSeamIndex = interface::tSeamIndex(-1, -1, -1);
    m_oScopeList.clear();
    #ifdef __debugLevelTwoError
        wmLog(eDebug, "LevelTwoErrorAdjacent::init()\n");
    #endif
}

void LevelTwoErrorAdjacent::reset()
{
    #ifdef __debugLevelTwoError
    std::ostringstream oDebugString;
    oDebugString << "LevelTwoErrorAdjacent::reset() scope <" << scope() << ">" << std::endl;
    wmLog(eDebug, oDebugString.str());
    #endif
  this->LevelTwoError::reset();
  m_oLastTriggeredSeamSeries = -1;
  m_oLastTriggeredSeam = -1;
  m_oLastTriggeredSeamInterval = -1;
  m_oLastSeamIndex = interface::tSeamIndex(-1, -1, -1);
  m_oScopeList.clear();
}

ResultEvaluation LevelTwoErrorAdjacent::testSumErrorLevel2(const Poco::SharedPtr<SumError> &p_oSumErr, const interface::tSeamIndex index)
{
    ResultEvaluation oTypeOrScope(eNotMasked);
    if (m_oResultTypes.size() <= 0)   // nothing to surveil...
    {
        wmLog(eDebug, "LevelTwoErrorAdjacent::testSumErrorLevel2() m_oResultTypes.size() <= 0; nothing to monitor..  return!\n");
        return oTypeOrScope;
    }
    if (m_oHasBeenSend) //no need to recalculate if this LevelTwoError has already triggered before...
    {
        return eSumErrorNotTriggered;
    }
    
    // the data of the incoming sumError 
    std::int32_t oSumErrorThrowValue = p_oSumErr->errorType();
    std::int32_t oSeamSerie = std::get<0>(index);
    std::int32_t oSeam = std::get<1>(index);
    std::int32_t oSeamInterval = std::get<2>(index);
    
    #ifdef __debugLevelTwoError
        std::int32_t oSumErrorTypeValue = p_oSumErr->sumErrorType();
        wmLog(eDebug, "LevelTwoErrorAdjacent::testSumErrorLevel2() Incoming sumError [%d] with type [%d] at scope [%d|%d|%d]\n", p_oSumErr->sumErrorType(), oSumErrorThrowValue, oSeamSerie, oSeam, oSeamInterval);
        wmLog(eDebug, "LevelTwoErrorAdjacent::testSumErrorLevel2() Violation currently at [%f / %f]\n", m_oFlawedViolation, m_oMaxViolation);
    #endif      
    
    
    bool nioInSeamDetected = false;
    
    if (m_checkSpecificError)
    {
        // the local selected error to surveil
        std::int32_t oSumErrorToWatch = m_oResultTypes[0];
        
        #ifdef __debugLevelTwoError
            wmLog(eDebug, "LevelTwoErrorAdjacent::testSumErrorLevel2() ErrorLevelTwo type:[%d] watching out for specific error [%d]\n", sumErrorType(), oSumErrorToWatch);
        #endif      
            
        if (oSumErrorToWatch != oSumErrorThrowValue)
        {
            #ifdef __debugLevelTwoError
                wmLog(eDebug, "LevelTwoErrorAdjacent::testSumErrorLevel2() Monitoring Error [%d], but incoming was [%d] - nothing to do, return\n", oSumErrorToWatch, oSumErrorThrowValue);
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
                    wmLog(eDebug, "LevelTwoErrorAdjacent::testSumErrorLevel2() SumError [%d] with type [%d] detected NIO at scope [%d|%d|%d]\n", oSumErrorTypeValue, oSumErrorThrowValue, oSeamSerie, oSeam, oSeamInterval);
                #endif
                break;
            }
        }
        
    }
    
    if (!nioInSeamDetected) 
    {
        return eSumErrorNotTriggered;
    }
    else if (isConsecutive(oSeamSerie, oSeam, oSeamInterval)) // check if the errors are consecutive
    {
        m_oFlawedViolation += 1;
        m_oLastTriggeredSeamSeries = oSeamSerie;
        m_oLastTriggeredSeam = oSeam;
        m_oLastTriggeredSeamInterval = oSeamInterval;
        #ifdef __debugLevelTwoError
            wmLog(eDebug, "LevelTwoErrorAdjacent::testSumErrorLevel2() CONSECUTIVE NIO! Current seamSeries/Seam [%d|%d]. Now at %f%% to trigger Level2Error!\n", oSeamSerie, oSeam, (100.0 * (m_oFlawedViolation / m_oMaxViolation)) );
        #endif
    }
    else //errors were not consecutive, reset and set count to 1 as this seam had a NIO
    {
        #ifdef __debugLevelTwoError
            wmLog(eDebug, "LevelTwoErrorAdjacent::testSumErrorLevel2() NOT CONSECUTIVE! Last triggered seamSeries/Seam was [%d|%d] - current [%d|%d] - Resetting!\n", m_oLastTriggeredSeamSeries, m_oLastTriggeredSeam, oSeamSerie, oSeam);
        #endif
        // last stored seam was not consecutive, reset the values and get out
        m_oFlawedViolation = 1;
        m_oLastTriggeredSeamSeries = oSeamSerie;
        m_oLastTriggeredSeam = oSeam;
        m_oLastTriggeredSeamInterval = oSeamInterval;
    }

    
    if (m_oFlawedViolation >= m_oMaxViolation)
    {
        #ifdef __debugLevelTwoError
            wmLog(eInfo, "LevelTwoErrorAdjacent::testSumErrorLevel2() Error condition reached <%f> ! -> Inform Host\n", m_oMaxViolation);
        #endif
        m_oHasBeenSend = true;
        return eSumErrorTriggered;
    }
    else
    {
        return eSumErrorNotTriggered;
    }
}

bool LevelTwoErrorAdjacent::isConsecutive(const std::int32_t seamSeries, const std::int32_t seam, const std::int32_t oSeamInterval)
{
    bool oReturnValue = false;
    #ifdef __debugLevelTwoError
        for (auto i = m_oScopeList.begin(); i != m_oScopeList.end(); ++i)
        {
            wmLog(eDebug, "LevelTwoErrorAdjacent::isConsecutive() list <%d> <%d> <%d>\n", std::get<0>(*i), std::get<1>(*i), std::get<2>(*i));
        }
    #endif
    if (m_oScopeList.empty())
    {
        return false;
    }
    // the first entry is the current scope, always valid
    interface::tSeamIndex oLastScope = *(m_oScopeList.begin());
    for (auto i = m_oScopeList.begin(); i != m_oScopeList.end(); ++i)
    {
        // search for the entry befor the current scopein the scope list 
        oLastScope = *(i);
        if (std::get<0>(oLastScope) == seamSeries)
        {
            if ((std::get<1>(oLastScope) < seam) && (std::get<2>(oLastScope) == 0))
            {
                #ifdef __debugLevelTwoError
                    wmLog(eDebug, "LevelTwoErrorAdjacent::isConsecutive() previous seam found <%d> <%d> <%d>\n", std::get<0>(oLastScope), std::get<1>(oLastScope), std::get<2>(oLastScope));
                #endif
                break;
            }
        }
        else
        {
            // other seamSerie
            #ifdef __debugLevelTwoError
                wmLog(eDebug, "LevelTwoErrorAdjacent::isConsecutive() previous seamSerie found <%d> <%d> <%d>\n", std::get<0>(oLastScope), std::get<1>(oLastScope), std::get<2>(oLastScope));
            #endif
            break;
        }
    }

    #ifdef __debugLevelTwoError
        std::ostringstream  oDebugString;
        oDebugString << "LevelTwoErrorAdjacent::isConsecutive() scope of last error <" << m_oLastTriggeredSeamSeries << "><" << m_oLastTriggeredSeam << "><" << m_oLastTriggeredSeamInterval << "> ";
        oDebugString << "last on product <" << std::get<0>(oLastScope) << "><" << std::get<1>(oLastScope) << "><" << std::get<2>(oLastScope) << "> ";
        oDebugString << "scope of new error <" << seamSeries << "><" << seam << "><" << oSeamInterval << ">" << std::endl;
        wmLog(eDebug, oDebugString.str());
    #endif

    // m_oLastTriggeredSeamSeries and m_oLastTriggeredSeam is the last found sumError scope
    if (m_oLastTriggeredSeamSeries == std::get<0>(oLastScope) && m_oLastTriggeredSeam == std::get<1>(oLastScope))
    {
        oReturnValue = true;
    }
    return oReturnValue;
}

void LevelTwoErrorAdjacent::trackScopeOnProduct(const ResultArgs& p_rRes)
{
    tSeamIndex oCurrentInspectKey;
    MeasureTask const &oMeasureTask = *(p_rRes.taskContext().measureTask());
    oCurrentInspectKey = tSeamIndex(oMeasureTask.seamseries(), oMeasureTask.seam(), oMeasureTask.seaminterval());
    if (std::find(m_oScopeList.begin(), m_oScopeList.end(), oCurrentInspectKey) == m_oScopeList.end())
    {
        #ifdef __debugLevelTwoError
            wmLog(eDebug, "LevelTwoErrorAdjacent::trackScopeOnProduct() add to scopeList<%d> <%d> <%d>\n", std::get<0>(oCurrentInspectKey), std::get<1>(oCurrentInspectKey), std::get<2>(oCurrentInspectKey));
        #endif
        // this scope is not yet registered in the list, put it in front of the list
        m_oScopeList.insert(m_oScopeList.begin(), std::make_tuple(oMeasureTask.seamseries(), oMeasureTask.seam(), oMeasureTask.seaminterval()));
    }
}

} // namespace interface
} // namespace precitec
