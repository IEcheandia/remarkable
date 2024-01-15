/**
 *
 *  @file       SignalSumErrorDualOutlierInRange.cpp
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


SignalSumErrorDualOutlierInRange::~SignalSumErrorDualOutlierInRange()
{
}

SignalSumErrorDualOutlierInRange* SignalSumErrorDualOutlierInRange::clone()
{
    SignalSumErrorDualOutlierInRange* cpy = new SignalSumErrorDualOutlierInRange(this->m_oMin, this->m_oMax, this->m_oThreshold, this->m_secondThreshold, this->m_lwmSignalThreshold);
    this->SignalSumError::copyValues(cpy, *this);
    return cpy;
}

void SignalSumErrorDualOutlierInRange::init()
{
    this->SignalSumError::init();

    m_oErrorType = (m_useReferenceBoundaries) ? ResultType::SignalSumErrorDualOutlierInRangeReferenceBoundary : ResultType::SignalSumErrorDualOutlierInRangeStaticBoundary;
    m_rangeCounter = 0.0;
    m_oFlawedViolation = 0.0;
    m_firstOutlierInRangeDetected = false;
}

double SignalSumErrorDualOutlierInRange::testSignalToNioCondition(double deltaToBoundary, int signalDistance, float signalQuality)
{    
    if (signalQuality == -1.0)
    {
        wmLog( eDebug, "SignalSumErrorDualOutlierInRange::testResult() Position[%f] - No calculation possible as no boundary was set...\n", m_currentPosition);
        return 0.0;
    }
    else if (signalQuality > 100.0)
    {
        if (m_oHasBeenSend && !m_allowMultipleEvaluation) //error was already triggered, so each outlier counts as error...
        {
            return 100.1;
        }
        
        m_oFlawedViolation += (signalDistance / 1000.0);
                
        double nioPercentage = std::fmin(50.0, 50.0 * (m_oFlawedViolation / m_oThreshold));
        
        if (m_firstOutlierInRangeDetected) 
        {
            nioPercentage += 50.0;
        } 
        else if ( (m_oFlawedViolation / m_oThreshold) >= 1.0) 
        {
            m_firstOutlierInRangeDetected = true;
            m_oFlawedViolation = 0.0; //reset to count second outlier
        }
        
       // #ifdef __debugSignalSumError
            if (m_firstOutlierInRangeDetected)
            {
                wmLog( eInfo, "SignalSumErrorDualOutlierInRange::testResult() Position[%f] - Possible 2nd outlier in range! FlawedViolation:[%f | %f] (%f%% to error)\n", 
                    m_currentPosition, m_oFlawedViolation, m_oThreshold, nioPercentage);
            }
            else
            {
               wmLog( eInfo, "SignalSumErrorDualOutlierInRange::testResult() Position[%f] - Possible 1st outlier detected! New flawedViolation:[%f | %f] (%f%% to error)\n", 
                    m_currentPosition, m_oFlawedViolation, m_oThreshold, nioPercentage); 
            }
       // #endif
        return nioPercentage;
    }
    else if (m_firstOutlierInRangeDetected)
    {
        m_oFlawedViolation = 0.0;
        m_rangeCounter += (signalDistance / 1000.0);
        
        //reset error
        if (m_rangeCounter >= m_secondThreshold)
        {
            m_rangeCounter = 0.0;
            m_firstOutlierInRangeDetected = false;
            return 0.0;
        }
        return 50.0;
    }
    else
    {
        m_oFlawedViolation = 0.0;
        return 0.0;
    }   
    
}

bool SignalSumErrorDualOutlierInRange::globalNioConditionReached()
{  
    bool nioReached =  m_firstOutlierInRangeDetected && (m_oFlawedViolation >= m_oThreshold);
    if (nioReached && (!m_oHasBeenSend || m_allowMultipleEvaluation))
    {
        #ifdef __debugSignalSumError
            wmLog( eWarning, "SignalSumErrorDualOutlierInRange::testResult() - Error condition reached at position [%f]!\n", m_currentPosition);
        #endif
        m_oHasBeenSend = true; 
        return true;
    }
    return false;
}

SignalSumErrorDualOutlierInRange& SignalSumErrorDualOutlierInRange::operator=(SignalSumErrorDualOutlierInRange const &rhs)
{
    if (this != &rhs)
    {
        this->SignalSumError::operator=(rhs);
    }
    return *this;
}


} // namespace interface
} // namespace precitec
