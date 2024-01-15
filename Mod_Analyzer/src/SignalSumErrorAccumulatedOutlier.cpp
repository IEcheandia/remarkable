/**
 *
 *  @file       sumErrorOutliers.cpp
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Daniel Zumkeller
 *  @date       20.03.2020
 *  @brief      Signal Type Sum Error Implementation for the detection of Accumulated Length/Area-Outliers. Derived from the generic Signal Sum Error.
 * 
 *  Code refactored and adapted to work with oversampling signals, based upon the SumErrorOutliers.cpp file
 *
*/

#include "analyzer/SignalSumError.h"


/////////////////////////////////////////////////////////////////////
// Signal Type Sum Error for Accumulated Length/Area Outlier Class //
/////////////////////////////////////////////////////////////////////
namespace precitec
{
namespace analyzer
{

using precitec::interface::ResultType;


SignalSumErrorAccumulatedOutlier::~SignalSumErrorAccumulatedOutlier()
{
}

SignalSumErrorAccumulatedOutlier* SignalSumErrorAccumulatedOutlier::clone()
{
    SignalSumErrorAccumulatedOutlier* cpy = new SignalSumErrorAccumulatedOutlier(this->m_oMin, this->m_oMax, this->m_oThreshold, this->m_lwmSignalThreshold, this->m_isAreaError, this->m_isPeakError);
    this->SignalSumError::copyValues(cpy, *this);
    return cpy;
}

void SignalSumErrorAccumulatedOutlier::init()
{
    this->SignalSumError::init();
    if (m_isPeakError)
    {
        m_oErrorType = (m_useReferenceBoundaries) ? interface::ResultType::SignalSumErrorPeakReferenceBoundary : interface::ResultType::SignalSumErrorPeakStaticBoundary;
        // set deviations to zero
    }
    else
    {
        m_oErrorType = (m_useReferenceBoundaries) ? ResultType::SignalSumErrorAccumulatedOutlierReferenceBoundary : ResultType::SignalSumErrorAccumulatedOutlierStaticBoundary;
    }
}

double SignalSumErrorAccumulatedOutlier::testSignalToNioCondition(double deltaToBoundary, int signalDistance, float signalQuality)
{    
    if (signalQuality == -1.0)
    {
        wmLog( eDebug, "SignalSumErrorAccumulatedOutlier::testResult() Position[%f] - No calculation possible as no boundary was set...\n", m_currentPosition);
        return 0.0;
    }
    else if (signalQuality > 100.0)
    {
        if (m_isPeakError)
        {
            wmLog( eInfo, "Position[%f] Peak outlier detected! (Distance from reference:[%f / %f%%])\n", m_currentPosition, deltaToBoundary, signalQuality);
            m_oFlawedViolation = m_oThreshold; //triggers global nio reached
            return 100.0;
        }
        else if (m_isAreaError) //In case of an area sum error, set multiplier for oSignalDistance, see a few lines below
        {
            m_oFlawedViolation += ( (deltaToBoundary * signalDistance) / 1000.0);
        }
        else
        {
            m_oFlawedViolation += (signalDistance / 1000.0);
        }
        
        const double nioPercentage = std::fmin(100.0, 100.0 * (m_oFlawedViolation / m_oThreshold));
        #ifdef __debugSignalSumError
            wmLog( eInfo, "SignalSumErrorAccumulatedOutlier::testResult() Position[%f] - Outlier detected! New flawedViolation:[%f] (%f%% to error)\n", 
                    m_currentPosition, m_oFlawedViolation, nioPercentage);
        #endif
        return nioPercentage;
    }
    return 0.0;
}

bool SignalSumErrorAccumulatedOutlier::globalNioConditionReached()
{
    bool nioReached = m_oFlawedViolation >= m_oThreshold;
    if (nioReached && (!m_oHasBeenSend || m_allowMultipleEvaluation))
    {
        #ifdef __debugSignalSumError
            wmLog( eWarning, "SignalSumErrorAccumulatedOutlier::testResult() - Error condition reached at position [%f]!\n", m_currentPosition);
        #endif
        m_oHasBeenSend = true; 
        return true;
    }
    return false;
}

SignalSumErrorAccumulatedOutlier& SignalSumErrorAccumulatedOutlier::operator=(SignalSumErrorAccumulatedOutlier const &rhs)
{
    if (this != &rhs)
    {
        this->SignalSumError::operator=(rhs);
    }
    return *this;
}


} // namespace interface
} // namespace precitec
