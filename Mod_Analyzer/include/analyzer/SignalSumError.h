/**
 * 	@file SignalSumError.h
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		  Daniel Zumkeller (Zm)
 * 	@date		    2020
 * 
 *  Class definitions of all Signal type SumErrors
*/

// FOR DEBUGGING
//#define __debugSignalSumError //for all signalSumError types

#pragma once

#include "analyzer/sumError.h"



namespace precitec
{
namespace analyzer
{

    
////////////////////////////////////
// SIGNAL SUM ERROR - BASE CLASS  //
////////////////////////////////////
class SignalSumError : public SumError
{
protected:
    explicit SignalSumError(double p_oMax, double p_oMin, double threshold, double secondThreshold, double lwmSignalThreshold,
                            geo2d::VecDPoint topReferenceCurve, geo2d::VecDPoint middleReferenceCurve, geo2d::VecDPoint bottomReferenceCurve, bool isAreaError, bool isPeakError, bool useReferenceBoundaries, bool useMiddleReference)
    : SumError()
    , m_oMax(p_oMax)
    , m_oMin(p_oMin)
    , m_oThreshold(threshold)
    , m_secondThreshold(secondThreshold)
    , m_lwmSignalThreshold(lwmSignalThreshold)
    , m_isAreaError(isAreaError)
    , m_useReferenceBoundaries(useReferenceBoundaries)
    , m_isPeakError(isPeakError)
    , m_oTopReferenceCurve(topReferenceCurve)
    , m_oBottomReferenceCurve(bottomReferenceCurve)
    , m_oMiddleReferenceCurve(middleReferenceCurve)
    , m_useMiddleReference(useMiddleReference)
    {
        isInlierError = false;
        init();
    }


public:
    void reset() override;

    void changeBounds(double p_oNewLower, double p_oNewUpper);
    void changeReference(geo2d::VecDPoint topReferenceCurve, geo2d::VecDPoint middleReferenceCurve, geo2d::VecDPoint bottomReferenceCurve);
    void setMaxViolation(double threshold) { m_oThreshold = threshold; }
    
    SignalSumError& operator=(SignalSumError const &rhs);
    void copyValues(SignalSumError* dest, SignalSumError const &src);

    static std::string getResultEvaluationAsString(ResultEvaluation value);

    unsigned int getEnvelopeBoundsIndex(const double position, const double signalDistance);
    void getEnvelopeBoundsAtIndex(double &p_rLower, double &p_rUpper, double &p_rMiddle, const unsigned int index);
    
    ResultEvaluationData testResult(const interface::ResultArgs &p_rRes, const interface::ResultArgs& lwmTriggerSignal) override; //generic 'testResult' routine for SignalSumError types
    virtual double testSignalToNioCondition(double deltaToBoundary, int signalDistance, float signalQuality) { return -1.0; }; //check on each signal for nioCondition (Implemented in child class)
    virtual bool globalNioConditionReached() { return false;} ; //check on each signal for nioCondition (Implemented in child class)

    void show() override;
    void getSELimits(double &lower, double &upper) override
    {
        lower = m_oMin;
        upper = m_oMax;
    }
    
    void setErrorType(const std::int32_t) override;
    interface::ResultArgs* arm(const fliplib::ArmStateBase& state) override;
    void allowMultipleEvaluation (bool value)
    {
        m_allowMultipleEvaluation = value;
    }

protected:

    void init();
    void checkBounds();
    // parameters
    double m_oMax;                    ///< lower static bound (static boundary errors only)
    double m_oMin;                    ///< upper static bound (static boundary errors only)
    double m_oThreshold;                  ///< pooled length or area of flawed parts necessary to throw a SignalSumErrorAll
    double m_secondThreshold;           ///< second threshold used for resettingOutlier sumError
    double m_lwmSignalThreshold;        /// lwm signal threshold value, used to determine when the signal inspection should begin. Only used for errors, monitoring lwm signals
    bool m_isAreaError;                       ///< TRUE = calculate integral value of outside path, FALSE = only calculate length outside, not value
    bool m_useReferenceBoundaries;                   ///< TRUE = use reference curves as boundaries, FALSE = use static boundaries 
    bool m_isPeakError; //defines whether the error is a peak error (fire at first NIO) or not.
    bool isInlierError;
    geo2d::VecDPoint m_oTopReferenceCurve;    ///< upper reference curve (reference boundary errors only)
    geo2d::VecDPoint m_oBottomReferenceCurve; ///< lower reference curve (reference boundary errors only)
    geo2d::VecDPoint m_oMiddleReferenceCurve; ///< original reference curve (reference boundary errors only)
    bool m_useMiddleReference;
    bool m_allowMultipleEvaluation = true;

    // internal variables
    double m_oFlawedViolation;         ///< internal "flawed" bucket, tells us how much is flawed
    double m_oFaultlessValue;          ///< internal bucket for a faultless distance, need for SumErrorInvert
    double m_currentPosition; ///< position from seam start [um]
    double m_lwmOffset = 0.0;           // position of the first sample above the threshold
    bool m_lwmThresholdReached = false;
};
//--------------------------------------------------------------------------------------//


//--------------------------------------------------------------------------------------//
/////////////////////////////////////////////////////////////////////
// SIGNAL SUM ERROR - Accumulated Length/Area Outlier Class        //
class SignalSumErrorAccumulatedOutlier : public SignalSumError
{
public:
    // static boundary error explicit ctor
    explicit SignalSumErrorAccumulatedOutlier(double upperBound, double lowerBound, double threshold, double lwmSignalThreshold, bool isAreaError, bool isPeakError)
        : SignalSumError(upperBound, lowerBound, threshold, 0, lwmSignalThreshold, {}, {}, {}, isAreaError, isPeakError, false, false)
    {
        init();
    }

    // reference boundary error explicit ctor
    explicit SignalSumErrorAccumulatedOutlier(geo2d::VecDPoint topReferenceCurve, geo2d::VecDPoint middleReferenceCurve, geo2d::VecDPoint bottomReferenceCurve, double upperDeviation, double lowerDeviation, double threshold, double lwmSignalThreshold, bool isAreaError, bool isPeakError, bool useMiddleReference)
        : SignalSumError(upperDeviation, lowerDeviation, threshold, 0, lwmSignalThreshold, topReferenceCurve, middleReferenceCurve, bottomReferenceCurve, isAreaError, isPeakError, true, useMiddleReference)
    {
        init();
    }

    ~SignalSumErrorAccumulatedOutlier();
    void init();

    double testSignalToNioCondition(double deltaToBoundary, int signalDistance, float signalQuality) override;
    bool globalNioConditionReached() override;

    SignalSumErrorAccumulatedOutlier& operator=(SignalSumErrorAccumulatedOutlier const&);
    SignalSumErrorAccumulatedOutlier* clone();
    void copyValues(SignalSumErrorAccumulatedOutlier* dest, SignalSumErrorAccumulatedOutlier const &src);

protected:
    void checkBounds();                ///< ensures that lower bound is less or equal than upper bound
};
//--------------------------------------------------------------------------------------//


//--------------------------------------------------------------------------------------//
////////////////////////////////////////////////////////////////
// SIGNAL SUM ERROR - Single Length/Area Outlier Class        //
////////////////////////////////////////////////////////////////
class SignalSumErrorSingleOutlier : public SignalSumError
{
public:
    // static boundary error explicit ctor
    explicit SignalSumErrorSingleOutlier(double upperBound, double lowerBound, double threshold, double lwmSignalThreshold, bool isAreaError)
        : SignalSumError(upperBound, lowerBound, threshold, 0, lwmSignalThreshold, {}, {}, {}, isAreaError, false, false, false)
    {
        init();
    }

    // reference boundary error explicit ctor
    explicit SignalSumErrorSingleOutlier(geo2d::VecDPoint topReferenceCurve, geo2d::VecDPoint middleReferenceCurve, geo2d::VecDPoint bottomReferenceCurve, double upperDeviation, double lowerDeviation, double threshold, double lwmSignalThreshold, bool isAreaError, bool useMiddleReference)
        : SignalSumError(upperDeviation, lowerDeviation, threshold, 0, lwmSignalThreshold, topReferenceCurve, middleReferenceCurve, bottomReferenceCurve, isAreaError, false, true, useMiddleReference)
    {
        init();
    }

    ~SignalSumErrorSingleOutlier();
    void init();
    SignalSumErrorSingleOutlier& operator=(SignalSumErrorSingleOutlier const&);
    SignalSumErrorSingleOutlier* clone();
    void copyValues(SignalSumErrorSingleOutlier* dest, SignalSumErrorSingleOutlier const &src);
    
    double testSignalToNioCondition(double deltaToBoundary, int signalDistance, float signalQuality) override;
    bool globalNioConditionReached() override;
};
//--------------------------------------------------------------------------------------//


//--------------------------------------------------------------------------------------//
///////////////////////////////////////////////////////////
// SIGNAL SUM ERROR - Single Length Inlier Class         //
///////////////////////////////////////////////////////////
class SignalSumErrorSingleInlier : public SignalSumError
{
public:
    // static boundary error explicit ctor
    explicit SignalSumErrorSingleInlier(double upperBound, double lowerBound, double threshold, double lwmSignalThreshold)
        : SignalSumError(upperBound, lowerBound, threshold, 0, lwmSignalThreshold, {}, {}, {}, false, false, false, false)
    {
        init();
    }

    // reference boundary error explicit ctor
    explicit SignalSumErrorSingleInlier(geo2d::VecDPoint topReferenceCurve, geo2d::VecDPoint middleReferenceCurve, geo2d::VecDPoint bottomReferenceCurve, double upperDeviation, double lowerDeviation, double threshold, double lwmSignalThreshold, bool useMiddleReference)
        : SignalSumError(upperDeviation, lowerDeviation, threshold, 0, lwmSignalThreshold, topReferenceCurve, middleReferenceCurve, bottomReferenceCurve, false, false, true, useMiddleReference)
    {
        init();
    }

    ~SignalSumErrorSingleInlier();
    void init();
    interface::ResultArgs* arm(const fliplib::ArmStateBase& state) override;

    double testSignalToNioCondition(double deltaToBoundary, int signalDistance, float signalQuality) override;
    bool globalNioConditionReached() override;

protected:
    void reset() override;

    bool m_oFullfillConditionOk; ///< Condition fullfilled, no error for this SumError

private:
    mutable tResultProxy*  m_pResultProxy;   ///< pointer to Resultproxy. send result at end of scope (inverted sumError)
};
//--------------------------------------------------------------------------------------//

//--------------------------------------------------------------------------------------//
///////////////////////////////////////////////////////////
// SIGNAL SUM ERROR - Dual Outlier Within Specified Range Class         //
///////////////////////////////////////////////////////////
class SignalSumErrorDualOutlierInRange : public SignalSumError
{
public:
    // static boundary error explicit ctor
    explicit SignalSumErrorDualOutlierInRange(double upperBound, double lowerBound, double threshold, double resettingThreshold, double lwmSignalThreshold)
        : SignalSumError(upperBound, lowerBound, threshold, resettingThreshold, lwmSignalThreshold, {}, {}, {}, false, false, false, false)
    {
        init();
    }

    // reference boundary error explicit ctor
    explicit SignalSumErrorDualOutlierInRange(geo2d::VecDPoint topReferenceCurve, geo2d::VecDPoint middleReferenceCurve, geo2d::VecDPoint bottomReferenceCurve, double upperDeviation, double lowerDeviation, double threshold, double resettingThreshold, double lwmSignalThreshold, bool useMiddleReference)
        : SignalSumError(upperDeviation, lowerDeviation, threshold, resettingThreshold, lwmSignalThreshold, topReferenceCurve, middleReferenceCurve, bottomReferenceCurve, false, false, true, useMiddleReference)
    {
        init();
    }

    ~SignalSumErrorDualOutlierInRange();
    void init();
    SignalSumErrorDualOutlierInRange& operator=(SignalSumErrorDualOutlierInRange const&);
    SignalSumErrorDualOutlierInRange* clone();
    void copyValues(SignalSumErrorDualOutlierInRange* dest, SignalSumErrorDualOutlierInRange const &src);

    double testSignalToNioCondition(double deltaToBoundary, int signalDistance, float signalQuality) override;
    bool globalNioConditionReached() override;
    
protected:
    
    bool m_firstOutlierInRangeDetected;
    double m_rangeCounter;

private:
    mutable tResultProxy*  m_pResultProxy;   ///< pointer to Resultproxy. send result at end of scope (inverted sumError)
};
//--------------------------------------------------------------------------------------//


} // namespace interface
} // namespace precitec
