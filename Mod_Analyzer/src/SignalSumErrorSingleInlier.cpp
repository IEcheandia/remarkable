/**
 *
 *  @file       signalSumErrorSingleInlier.cpp
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Daniel Zumkeller
 *  @date       20.03.2020
 *  @brief      Signal Type Sum Error Implementation for detection of single Length/Area-Inliers. Derived from the generic Signal Sum Error.
 * 
 *  Code refactored and adapted to work with oversampling signals, based upon the SumErrorInliers.cpp file
 *
*/

#include "analyzer/SignalSumError.h"
#include <filter/armStates.h>
#include "event/results.interface.h"


///////////////////////////////////////////////////////////
// Signal Type Sum Error for Single Length Inlier Class  //
///////////////////////////////////////////////////////////
namespace precitec
{
namespace analyzer
{

using precitec::interface::ResultArgs;
using precitec::interface::SmpResultPtr;
using precitec::interface::ResultType;


SignalSumErrorSingleInlier::~SignalSumErrorSingleInlier()
{
}

void SignalSumErrorSingleInlier::init()
{
    this->SignalSumError::init();
    m_oErrorType = (m_useReferenceBoundaries) ? ResultType::SignalSumErrorInlierReferenceBoundary : ResultType::SignalSumErrorInlierStaticBoundary;
    m_oHasBeenSend = false;
    m_oFullfillConditionOk = false;
    isInlierError = true;
}

interface::ResultArgs* SignalSumErrorSingleInlier::arm(const fliplib::ArmStateBase& state)
{
    interface::ResultArgs* pSendResult = nullptr;

    // initialize the result object in any case
    if ( m_oLastResultsList.size() > 0 )
    {
        m_oErrorResult = m_oLastResultsList.front();
        m_oErrorResult.setResultType( m_oErrorType );
        m_oErrorResult.setNio(true);
        m_oErrorResult.setNioResult(std::vector<geo2d::DPoint>{geo2d::DPoint(m_oErrorResult.context().position() / 1000.0, 100.0)});
        if (((m_oErrorResult.resultType() >= interface::ResultType::QualityFaultTypeA) && (m_oErrorResult.resultType() <= interface::ResultType::QualityFaultTypeX)) ||
            ((m_oErrorResult.resultType() >= interface::ResultType::QualityFaultTypeA_Cat2) && (m_oErrorResult.resultType() <= interface::ResultType::QualityFaultTypeX_Cat2)) ||
            (m_oErrorResult.resultType() == interface::ResultType::FastStop_DoubleBlank))
        {
            m_oErrorResult.setNioType( m_oErrorResult.resultType() );
            #ifdef __debugSignalSumError
                wmLog( eDebug, "SignalSumErrorSingleInlier::arm() QualityFault!! values resultType <%d> nioType <%d> \n", m_oErrorResult.resultType(), m_oErrorResult.nioType());
            #endif
        }
    }
    else
    {
        #ifdef __debugSignalSumError
            wmLog( eDebug, "SignalSumErrorSingleInlier::arm() - last result list was empty ...\n");
        #endif
        return nullptr;
    }

    // now decide if we really shoud send it out, depending on the scope. for this sum-error only the ending of a seam or seam-interval make sense ...
    switch( state.getStateID() )
    {
    case filter::eSeamEnd:
        #ifdef __debugSignalSumError
            wmLog( eDebug, "SignalSumErrorSingleInlier::arm() - seam end detected ...\n");
        #endif
        pSendResult = &m_oErrorResult;
        break;

    case filter::eSeamIntervalEnd:
        #ifdef __debugSignalSumError
            wmLog( eDebug, "SignalSumErrorSingleInlier::arm() - seam-interval end detected ...\n");
        #endif
        pSendResult = &m_oErrorResult;
        break;
    default:
        return nullptr;
    }

    if ( m_oFullfillConditionOk ) // in addition to the scope, also the actual condition has to be false, otherwise we still have to return a nullptr and thereby will not send an nio ...
    {
        return nullptr;
    }
    else
    {
        pSendResult->setNioResult(std::vector<geo2d::DPoint>{geo2d::DPoint(0.0, 100.0)});
        return pSendResult;
    }
}

void SignalSumErrorSingleInlier::reset()
{
    this->SignalSumError::reset();
    m_oFullfillConditionOk = false;
}

double SignalSumErrorSingleInlier::testSignalToNioCondition(double deltaToBoundary, int signalDistance, float signalQuality)
{
    
    if (m_oFullfillConditionOk == true) // no need to check for the inlier anymore, only calculate signalQuality now
    {
        #ifdef __debugSignalSumError
            wmLog( eDebug, "SignalSumErrorSingleInlier::testResult() Scope already found ok; only calculating signalQuality...\n");
        #endif
        return -1.0; //as the condition is true, there wo't be any NIO now
    }
    else if (signalQuality == -1.0)
    {
        wmLog( eDebug, "SignalSumErrorAccumulatedOutlier::testResult() Position[%f] - No calculation possible as no boundary was set...\n", m_currentPosition);
        m_oFullfillConditionOk = true;
        return 0.0;
    }
    else if (signalQuality < 100.0) //current signal is inside the boundaries
    {
        // update the distance measured of the previous found result
        m_oFaultlessValue += signalDistance / 1000.0;
        
        const auto nioPercentage = 100.0 * (1.0 - (m_oFaultlessValue / m_oThreshold));
        #ifdef __debugSignalSumError
            wmLog( eInfo, "SignalSumErrorSingleInlier::testResult() Position[%f] - Inside boundary! Remaining NioPercentage:[%f%%]", m_currentPosition, nioPercentage);
        #endif
        if (nioPercentage <= 0.0)
        {
            m_oFullfillConditionOk = true;
            #ifdef __debugSignalSumError
                wmLog( eInfo, "SignalSumErrorSingleInlier::testResult() Position[%f] - Required inlier length reached!\n", m_currentPosition);
            #endif
            return 0.0;
        }
        return nioPercentage;
    }
    else
    {
        #ifdef __debugSignalSumError
            wmLog( eDebug, "SignalSumErrorSingleInlier::testResult() Position[%f] - Outside boundaries... resetting possible progress\n", m_currentPosition);
        #endif
        // out of boundaries; start conditions again
        m_oFaultlessValue = 0; // reset distance
        return 100.0;
    }
}

bool SignalSumErrorSingleInlier::globalNioConditionReached()
{
    return false; // inverted error does not throw nio when the condition is reached, it throws nio if the condition is not reached by the end of the seam
}


} // namespace interface
} // namespace precitec
