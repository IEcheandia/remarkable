/**
 * 
 *  @file       signalSumError.cpp
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Daniel Zumkeller
 *  @date       20.03.2020
 *  @brief      Base class for all Signal Sum Errors, holding all common functions. The functions "init()", "testSignalToNIOCondition()" and "globalNIOConditionReached()" 
 *              need to be overwritten by the respective implementations in order for the sumError to work properly.
 * 
 *  Code refactored and adapted to work with oversampling signals, based upon the SumErrorOutliers.cpp file
 *
*/

#include "analyzer/SignalSumError.h"
#include <filter/armStates.h>


///////////////////////////////////////////////////////////
// SignalSumError - BaseClass for all Signal type Errors //
///////////////////////////////////////////////////////////
namespace precitec
{
namespace analyzer
{

using precitec::interface::ResultArgs;
using precitec::interface::SmpResultPtr;
using precitec::interface::MeasureTask;
using precitec::interface::ResultType;
using precitec::interface::TResults;

SignalSumError& SignalSumError::operator=(SignalSumError const &rhs)
{
    if (this != &rhs)
    {
        this->SumError::operator=(rhs);
        copyValues(this, rhs);
    }
    return *this;
}

void SignalSumError::init()
{
    reset();
    m_oScope = eScopeSeam;
    m_oFlawedViolation = 0.0;
    m_oFaultlessValue = 0.0;
    m_oErrorType = ResultType::SumErrorNone;
    setTriggerBase(TriggerBase::ePositionTrigger);
    if (m_useReferenceBoundaries)
    {
        checkBounds();
    }
}

void SignalSumError::reset()
{
    m_oFlawedViolation = 0.0;
    m_oFaultlessValue = 0.0;
    m_lwmOffset = 0.0;
    m_lwmThresholdReached = false;

    SumError::reset();
}

void SignalSumError::setErrorType(const std::int32_t p_oType)
{
    m_oErrorType = (ResultType)p_oType;
#ifdef __debugSignalSumError
    wmLog( eDebug, "SignalSumError::setErrorType m_oErrorType <%d>\n", p_oType);
#endif
}

ResultArgs* SignalSumError::arm(const fliplib::ArmStateBase& state)
{
    // send only if it's the end of the seam and the lwm threshold has never been reached
    // only relevant for lwm signal errors, all others reach the threshold immediately
    if (m_lwmThresholdReached || state.getStateID() != filter::eSeamEnd)
    {
        return nullptr;
    }

    if (m_oLastResultsList.size() == 0)
    {
        return nullptr;
    }

    m_oErrorResult = m_oLastResultsList.front();
    m_oErrorResult.setResultType(m_oErrorType);
    m_oErrorResult.setNio(true);
    m_oErrorResult.setNioResult(std::vector<geo2d::DPoint>{geo2d::DPoint(0.001 * m_oErrorResult.context().position(), 100.0)});

    if (((m_oErrorResult.resultType() >= ResultType::QualityFaultTypeA) && (m_oErrorResult.resultType() <= ResultType::QualityFaultTypeX)) ||
        ((m_oErrorResult.resultType() >= ResultType::QualityFaultTypeA_Cat2) && (m_oErrorResult.resultType() <= ResultType::QualityFaultTypeX_Cat2)) ||
        (m_oErrorResult.resultType() == ResultType::FastStop_DoubleBlank))
    {
        m_oErrorResult.setNioType(m_oErrorResult.resultType());
    }

    return &m_oErrorResult;
}

void SignalSumError::copyValues(SignalSumError* dest, SignalSumError const &src)
{
    dest->SumError::copyValues(dest, src);
    dest->m_oThreshold = src.m_oThreshold;
    dest->m_oFlawedViolation = src.m_oFlawedViolation;
    dest->m_oMax = src.m_oMax;
    dest->m_oMin = src.m_oMin;
}

void SignalSumError::checkBounds()
{
    if (m_oMax < m_oMin)
    {
        std::swap(m_oMax, m_oMin);
    }
}

void SignalSumError::changeBounds(double p_oNewLower,double p_oNewUpper)
{
    m_oMin = p_oNewLower;
    m_oMax = p_oNewUpper;
    checkBounds();
}

void SignalSumError::changeReference(geo2d::VecDPoint p_oTopRefCurve, geo2d::VecDPoint p_oMiddleRefCurve, geo2d::VecDPoint p_oBottomRefCurve)
{
    m_oTopReferenceCurve = p_oTopRefCurve;
    m_oMiddleReferenceCurve = p_oMiddleRefCurve;
    m_oBottomReferenceCurve = p_oBottomRefCurve;
}

std::string SignalSumError::getResultEvaluationAsString(ResultEvaluation value)
{
    std::string returnStr;
    switch (value)
    {
        case eNotMasked : returnStr= "eNotMasked";break;
        case eBadScope : returnStr= "eBadScope";break;
        case eMasked : returnStr= "eMasked";break;
        case eSumErrorNotTriggered : returnStr= "eSumErrorNotTriggered";break;
        case eSumErrorTriggered : returnStr= "eSumErrorTriggered";break;
    }
    return returnStr;
}

ResultEvaluationData SignalSumError::testResult(const ResultArgs& p_resultArguments, const interface::ResultArgs& lwmTriggerSignal)
{
    ResultEvaluationData result{SumError::findResult(p_resultArguments), {}, {}, {}, {}};

    if ((result.evaluationStatus == eBadScope) || (result.evaluationStatus == eNotMasked))
    {
        return result;
    }
    else
    {
        //standard status, valid for all cases except when the sumError gets actually triggered
        result.evaluationStatus = eSumErrorNotTriggered;
    }

    const auto& frameStartPosition = p_resultArguments.context().position();

    if (frameStartPosition < 0)
    {
        wmLog(eError, "SignalSumError::testResult() Frame[%d] - Negative frameStartPosition (%d mm). Returning without calculation.", getImageTrigger(p_resultArguments), frameStartPosition);
        return result;
    }

    if (m_oHasBeenSend && m_allowMultipleEvaluation)
    {
        //reset the sumerror counters, but keep the lwm offset in order to use the correct position in the reference curve
        auto lwmOffset = m_lwmOffset;
        auto lwmThresholdReached = m_lwmThresholdReached;
        reset();
        m_lwmThresholdReached = lwmThresholdReached;
        m_lwmOffset = lwmOffset;
    }

    updateTriggerDistance(p_resultArguments);

    const auto& oversamplingRate = p_resultArguments.value<double>().size();
    const auto& signalDistance = oversamplingRate != 0 ? double(m_oTriggerDistance) / oversamplingRate : 0.0;

    auto lwmOffsetSamplesInThisFrame = 0;
    const auto& lwmSignalInspection = lwmTriggerSignal.isValid() && oversamplingRate > 1;

    if (!lwmSignalInspection)
    {
        // not an lwm signal, act as if the threshold has been reached immediately

        m_lwmThresholdReached = true;
        m_lwmOffset = 0.0;
    }
    else if (!m_lwmThresholdReached)
    {
        // inspect lwm trigger, as long as the threshold hasn't been reached

        m_lwmOffset = frameStartPosition;
        const auto& lwmTriggerSamples = lwmTriggerSignal.value<double>();

        auto it = std::find_if(lwmTriggerSamples.begin(), lwmTriggerSamples.end(), [this] (const auto& sample) { return sample > m_lwmSignalThreshold; });

        if (it != lwmTriggerSamples.end())
        {
            lwmOffsetSamplesInThisFrame = std::distance(lwmTriggerSamples.begin(), it);

            m_lwmThresholdReached = true;
            m_lwmOffset += lwmOffsetSamplesInThisFrame * signalDistance;
        }
    }

    if (oversamplingRate == 0)
    {
        // no values to be computed
        wmLog(eError, "SignalSumError::testResult() Frame[%d] - No value in Result, so no Error can be calculated.", getImageTrigger(p_resultArguments));
    }
    else if ((p_resultArguments.type() != interface::RegInt) && (p_resultArguments.type() != interface::RegDouble) && (p_resultArguments.type() != interface::RegDoubleArray))
    {
        wmLogTr(eError, "QnxMsg.Misc.SEInvalidType", "Invalid datatype for global error");
    }
    else
    {
        // ResultArgs are valid, signalQuality and nioPercentage can be calculated
        result.signalQuality.resize(oversamplingRate);

        // For Reference Curves min and max represent the deviation values.
        // These are defined as absolute values, therefore we take a negative value for min to ease computation.
        auto upperBoundary = m_oMax;
        auto lowerBoundary = m_useReferenceBoundaries ? -m_oMin : m_oMin;
        auto referenceValue = 0.5 * (upperBoundary + lowerBoundary);

        const auto& envelopeStartIndex = getEnvelopeBoundsIndex(0.001 * (frameStartPosition + lwmOffsetSamplesInThisFrame * signalDistance - m_lwmOffset), 0.001 * signalDistance);

        // Comparison with Reference Curves starts after the threshold value has been reached.
        // In case the observed signal is not a lwm signal, this happens immediately
        if (m_useReferenceBoundaries && m_lwmThresholdReached)
        {
            result.upperReference.reserve(oversamplingRate - lwmOffsetSamplesInThisFrame);
            result.lowerReference.reserve(oversamplingRate - lwmOffsetSamplesInThisFrame);
        }

        if (!m_lwmThresholdReached)
        {
            // Threshold not yet reached, meaning there is no active inspection and the signal quality is perfect.
            // Nio percentage is thus zero.
            std::fill(result.signalQuality.begin(), result.signalQuality.end(), 0.0);

            for (auto i = 0u; i < oversamplingRate; i++)
            {
                m_currentPosition = frameStartPosition + i * signalDistance;
                result.nioPercentage.emplace_back(0.001 * m_currentPosition, 0.0);
            }
        } else
        {
            for (auto i = 0; i < lwmOffsetSamplesInThisFrame; i++)
            {
                // section, before the threshold is reached
                result.signalQuality[i] = 0.0;

                m_currentPosition = frameStartPosition + i * signalDistance;
                result.nioPercentage.emplace_back(0.001 * m_currentPosition, 0.0);
            }

            for (std::size_t i = lwmOffsetSamplesInThisFrame; i < oversamplingRate; i++)
            {
                const auto& currentValue = p_resultArguments.value<double>().at(i);
                m_currentPosition = frameStartPosition + i * signalDistance;

                if (lwmSignalInspection && !m_oMiddleReferenceCurve.empty() && (0.001 * m_currentPosition > (0.001 * m_lwmOffset + m_oMiddleReferenceCurve.back().x)))
                {
                    // reached end of refrence, stop inspection for lwm signals
                    result.signalQuality[i] = 0.0;
                    result.nioPercentage.emplace_back(0.001 * m_currentPosition, testSignalToNioCondition(0.0, signalDistance, result.signalQuality.at(i)));
                    continue;
                }

                if (m_useReferenceBoundaries && !m_oMiddleReferenceCurve.empty() && (m_useMiddleReference ? true : !m_oTopReferenceCurve.empty() && !m_oBottomReferenceCurve.empty()))
                {
                    getEnvelopeBoundsAtIndex(upperBoundary, referenceValue, lowerBoundary, envelopeStartIndex + i - lwmOffsetSamplesInThisFrame);
                }

                result.upperReference.emplace_back(0.001 * m_currentPosition, upperBoundary);
                result.lowerReference.emplace_back(0.001 * m_currentPosition, lowerBoundary);

                // Calculate signal quality
                const auto& upwardsValue = currentValue > referenceValue;
                const auto& diffToIdeal = (m_isPeakError || m_isAreaError) ? (upwardsValue ? std::fmax(0.0, currentValue - upperBoundary) : std::fmax(0.0, lowerBoundary - currentValue)) : std::fabs(currentValue - referenceValue);
                auto maxAllowedDiff = m_isPeakError ? m_oThreshold : (upwardsValue ? std::fabs(upperBoundary - referenceValue) : std::fabs(lowerBoundary - referenceValue));

                if (m_isAreaError && !m_useReferenceBoundaries)
                {
                    maxAllowedDiff = upwardsValue ? std::fabs(upperBoundary) : std::fabs(lowerBoundary);
                    result.signalQuality[i] = (maxAllowedDiff > 0.0) ? 100.0 * std::fabs(currentValue / maxAllowedDiff) : ((diffToIdeal > 0.0) ? 100.0 : 0.0);
                }
                else
                {
                    result.signalQuality[i] = (maxAllowedDiff > 0.0) ? 100.0 * (diffToIdeal / maxAllowedDiff) : ((diffToIdeal > 0.0) ? 100.0 : 0.0);
                }

                const auto& thisNioPercent = testSignalToNioCondition(diffToIdeal, signalDistance, result.signalQuality.at(i));

                if (!m_oHasBeenSend)
                {
                    const bool showNio = (isInlierError && thisNioPercent < 100.0 && thisNioPercent > -1.0) || (!isInlierError && thisNioPercent > 0.0);
                    if (showNio && (result.nioPercentage.empty() || (result.nioPercentage.back().y != thisNioPercent)))
                    {
                        result.nioPercentage.emplace_back(0.001 * m_currentPosition, thisNioPercent);
                    }
                    if (globalNioConditionReached())
                    {
                        result.evaluationStatus = eSumErrorTriggered;
                    }
                }
                else if (result.signalQuality.at(i) >= 100.0)
                {
                    result.nioPercentage.emplace_back(0.001 * m_currentPosition, 100.0);
                }
            }
        }
    }

    return result;
}

void SignalSumError::show()
{
    std::ostringstream  oDebugString;
    oDebugString << "SignalSumError::show() SumError: <" << m_oSumErrorType << ">  throw error: <" << m_oErrorType << "> ResultTypes";
    for (unsigned int i=0; i < m_oResultTypes.size(); ++i)
    {
        oDebugString << " <" << (int)m_oResultTypes[i] << ">; ";
    }
    oDebugString << "\n";
    wmLog(eDebug, oDebugString.str()); oDebugString.str("");
    oDebugString << "  Min <" << m_oMin << "> Max <" << m_oMax << "> MaxViolation <" << m_oThreshold;
    oDebugString << "  at NF: <" << m_oSeamseries << "> N <" << m_oSeam << "> NB <" << m_oSeaminterval << "> Scope <" << m_oScope << ">\n";
    wmLog( eDebug, oDebugString.str()); oDebugString.str("");
    oDebugString << "  Triggerdistance <" << m_oTriggerDistance << ">  Triggerbase <" << m_oTriggerBase << ">\n";
    wmLog( eDebug, oDebugString.str()); oDebugString.str("");
}

// index of min/max envelope position 
unsigned int SignalSumError::getEnvelopeBoundsIndex(const double position, const double signalDistance)
{
    if (!m_useReferenceBoundaries)
    {
        return 0; //flag for static reference boundaries
    }
    
    unsigned int rIndex = 0;
    if (!m_useMiddleReference && (m_oTopReferenceCurve.size() != 0) && (m_oBottomReferenceCurve.size() != 0) )
    {
        auto topIt = std::find_if(m_oTopReferenceCurve.begin(), m_oTopReferenceCurve.end(), [position, signalDistance] (geo2d::DPoint &pv) { return (std::fabs(position - pv.x) <= 0.5 * signalDistance);});
        bool topAtEnd = (topIt == m_oTopReferenceCurve.end());
        auto bottomIt = std::find_if(m_oBottomReferenceCurve.begin(), m_oBottomReferenceCurve.end(), [position, signalDistance] (geo2d::DPoint &pv) { return (std::fabs(position - pv.x) <= 0.5 * signalDistance);});
        bool bottomAtEnd = (bottomIt == m_oBottomReferenceCurve.end());
        
        unsigned int topIdx = distance(m_oTopReferenceCurve.begin(), topIt);
        unsigned int bottomIdx = distance(m_oBottomReferenceCurve.begin(), bottomIt);
        rIndex = ( topAtEnd || bottomAtEnd ) ? std::min(topIdx, bottomIdx) : (( topIdx == bottomIdx) ? topIdx : std::floor(0.5 * (topIdx + bottomIdx)));
    }
    else if (m_useMiddleReference && (m_oMiddleReferenceCurve.size() != 0) )
    {
        auto topIt = std::find_if(m_oMiddleReferenceCurve.begin(), m_oMiddleReferenceCurve.end(), [position, signalDistance] (geo2d::DPoint &pv) { return (std::fabs(position - pv.x) <= 0.5 * signalDistance);});
        rIndex = distance(m_oMiddleReferenceCurve.begin(), topIt);
        wmLog( eDebug, "SignalSumError::getEnvelopeBoundsIndices() Position [%f] - Using middle reference curve: returnedIndex: [%d]\n", position, rIndex);
    }
    else
    {
        wmLog( eInfo, "SignalSumError::getEnvelopeBoundsIndex - NO REFERENCE CURVES TO GET INDEX FROM, RETURNING %d\n", rIndex);
    }

    #ifdef __debugSignalSumError
    wmLog( eDebug, "SignalSumError::getEnvelopeBoundsIndices() for position [%f] returned index: [%d]\n", position, rIndex);
    #endif
    return rIndex;
}

//value at envelopeBoundsIndex or at the first / last value if the index is out of bounds
void SignalSumError::getEnvelopeBoundsAtIndex(double &p_rUpper, double &p_rMiddle, double &p_rLower, const unsigned int index)
{
    if (index < 0) {
        const double middleRef = m_oMiddleReferenceCurve.front().y;
        p_rUpper = (m_useMiddleReference ? middleRef : m_oTopReferenceCurve.front().y) + m_oMax;
        p_rLower = (m_useMiddleReference ? middleRef : m_oBottomReferenceCurve.front().y) - m_oMin;
        p_rMiddle = middleRef;
    }
    else 
    {
        const double middleRef = (index < m_oMiddleReferenceCurve.size() ) ? m_oMiddleReferenceCurve[index].y : m_oMiddleReferenceCurve.back().y;
        if (m_useMiddleReference)
        {
            p_rUpper = middleRef + m_oMax;
            p_rLower = middleRef - m_oMin;
        }
        else
        {
            p_rUpper = (index < m_oTopReferenceCurve.size() ) ? (m_oTopReferenceCurve[index].y + m_oMax) : (m_oTopReferenceCurve.back().y + m_oMax);
            p_rLower = (index < m_oBottomReferenceCurve.size() ) ? (m_oBottomReferenceCurve[index].y - m_oMin) : (m_oBottomReferenceCurve.back().y - m_oMin);
        }
        p_rMiddle = middleRef;
    }
}

} // namespace interface
} // namespace precitec
