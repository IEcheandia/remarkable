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


////////////////////////////////////////////////////////////////
// Signal Type Sum Error for Single Length/Area Outlier Class //
////////////////////////////////////////////////////////////////
namespace precitec
{
namespace analyzer
{

using precitec::interface::ResultType;


SignalSumErrorSingleOutlier::~SignalSumErrorSingleOutlier()
{
}

SignalSumErrorSingleOutlier* SignalSumErrorSingleOutlier::clone()
{
    SignalSumErrorSingleOutlier* cpy = new SignalSumErrorSingleOutlier(this->m_oMin, this->m_oMax, this->m_oThreshold, this->m_lwmSignalThreshold, this->m_isAreaError);
    this->SignalSumError::copyValues(cpy, *this);
    return cpy;
}

void SignalSumErrorSingleOutlier::init()
{
    this->SignalSumError::init();
    m_oErrorType = (m_useReferenceBoundaries) ? ResultType::SignalSumErrorSingleOutlierReferenceBoundary : ResultType::SignalSumErrorSingleOutlierStaticBoundary;
    m_oHasBeenSend = false;
}

double SignalSumErrorSingleOutlier::testSignalToNioCondition(double deltaToBoundary, int signalDistance, float signalQuality)
{
    //check for errors
    if (signalQuality > 100.0)
    {
        if (m_oFlawedViolation == 0) //reset error
        {
            SumError::clearErrors();
        }

        if (m_isAreaError) //In case of an area sum error, set multiplier for oSignalDistance, see a few lines below
        {
            m_oFlawedViolation += ( (deltaToBoundary * signalDistance) / 1000.0 );
        }
        else
        {
            m_oFlawedViolation += ( signalDistance / 1000.0 );
        }

        const double nioPercentage = std::fmin(100.0, 100.0 * (m_oFlawedViolation / m_oThreshold));
        #ifdef __debugSignalSumError
            wmLog( eDebug, "SignalSumErrorSingleOutlier::testResult() Position[%d] - Error condition: Value out of bounds (%f%% to error).\n", m_currentPosition, nioPercentage);
            wmLog( eDebug, "SignalSumErrorSingleOutlier::testResult() Position[%d] - m_oFlawedViolation[%f] - deltaToBoundary[%f] - signalDistance[%f]\n", m_currentPosition, m_oFlawedViolation, deltaToBoundary, signalDistance);
        #endif
        return nioPercentage;
    }
    else if (m_oFlawedViolation != 0) // reset errorDetection for next singleError
    {
        m_oFlawedViolation = 0;
        SumError::clearErrors();
        #ifdef __debugSignalSumError
            wmLog( eDebug, "SignalSumErrorSingleOutlier::testResult() Position[%d] - No Error condition -> Reset Counter!\n", m_currentPosition);
        #endif
    }
    return 0.0;
}

bool SignalSumErrorSingleOutlier::globalNioConditionReached()
{
    bool nioReached = m_oFlawedViolation >= m_oThreshold;
    if (nioReached && (!m_oHasBeenSend || m_allowMultipleEvaluation))
    {
#ifdef __debugSignalSumError
        wmLog( eInfo, "SignalSumErrorSingleOutlier::testResult() Position[%d] - Error condition reached!\n", m_currentPosition);
#endif
        m_oHasBeenSend = true;
        return true;
    }
    return false;
}

SignalSumErrorSingleOutlier& SignalSumErrorSingleOutlier::operator=(SignalSumErrorSingleOutlier const &rhs)
{
    if (this != &rhs)
    {
        this->SignalSumError::operator=(rhs);
    }
    return *this;
}


} // namespace interface
} // namespace precitec
