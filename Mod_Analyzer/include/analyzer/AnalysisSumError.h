/**
 *  @file AnalysisSumError.h
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Daniel Zumkeller (Zm)
 *  @date       2020
 * 
 *  Class definitions of all Analysis type SumErrors
*/
#pragma once

// FOR DEBUGGING
//#define __debugAnalysisSumError //for all signalSumError types

#include "analyzer/sumError.h"


namespace precitec
{
namespace analyzer
{

    
///////////////////////////////////////////////////////////
// ANALYSIS ERRORS - BASE CLASS                          //
///////////////////////////////////////////////////////////
class AnalysisSumError : public SumError
{
protected:
    explicit AnalysisSumError(double p_oMaxViolation)
    : SumError()
    , m_oMaxViolation(p_oMaxViolation)
    {
        init();
    }

public:
    virtual ~AnalysisSumError();

    virtual interface::ResultType type() const override { return interface::ResultType::SumErrorNone; } // not a user type!
    virtual void reset() override;
    void setMaxViolation(double p_oNewMaxViolation) { m_oMaxViolation = p_oNewMaxViolation; }
//TODO IMPLEMENT THESE:   
//    virtual ResultEvaluationData testResult(const interface::ResultArgs &p_rRes) override;
//    //check on each signal for nioCondition, and global nioReached, implemented by each derived sumError
//    virtual float testSignalToNioCondition(double currentValue, double lowerBoundary, double upperBoundary, double signalDistance, float signalQuality);
//    virtual bool globalNioConditionReached(int position);
    
    AnalysisSumError& operator=(AnalysisSumError const &rhs);
    void copyValues(AnalysisSumError* dest, AnalysisSumError const &src);

    ResultEvaluation testAnalysisResult(const interface::ResultArgs& p_rRes);

    virtual void setErrorType(const std::int32_t) override;

protected:

    void init();
    ResultEvaluation testSingularity(const interface::ResultArgs &p_rRes);
    // parameters
    double m_oMaxViolation;                  ///< pooled length or area of flawed parts necessary to throw a SignalSumErrorAll

    // internal variables
    double m_oFlawedViolation;         ///< internal "flawed" bucket, tells us how much is flawed
    double m_oFaultlessValue;          ///< internal bucket for a faultless distance, need for SumErrorInvert
    std::vector<interface::ResultArgs> m_oSingularityList;
};
//--------------------------------------------------------------------------------------//


//--------------------------------------------------------------------------------------//
///////////////////////////////////////////////////////////
// Analysis Error Accumulated (All) Outlier Check Class  //
///////////////////////////////////////////////////////////
class AnalysisSumErrorAccumulatedOutlier : public AnalysisSumError
{
public:
    explicit AnalysisSumErrorAccumulatedOutlier(double p_oMaxViolation) :  // non-envelope error explicit ctor
        AnalysisSumError(p_oMaxViolation)
    {
        init();
    }
    virtual ~AnalysisSumErrorAccumulatedOutlier();
    void init();

    virtual ResultEvaluationData testResult(const interface::ResultArgs &p_rRes, const interface::ResultArgs& lwmTriggerSignal) override;

    AnalysisSumErrorAccumulatedOutlier& operator=(AnalysisSumErrorAccumulatedOutlier const&);
    virtual AnalysisSumErrorAccumulatedOutlier* clone();
    void copyValues(AnalysisSumErrorAccumulatedOutlier* dest, AnalysisSumErrorAccumulatedOutlier const &src);
};
//--------------------------------------------------------------------------------------//


//--------------------------------------------------------------------------------------//
///////////////////////////////////////////////////////////
// Analysis Error Single Outlier Check Class             //
///////////////////////////////////////////////////////////
class AnalysisSumErrorSingleOutlier : public AnalysisSumError
{
public:
    explicit AnalysisSumErrorSingleOutlier(double p_oMaxViolation) : // non-envelope error explicit ctor
        AnalysisSumError(p_oMaxViolation)
    {
        init();
    }
    virtual ~AnalysisSumErrorSingleOutlier();
    void init();

    std::uint64_t getLastErrorDistance(const interface::ResultArgs& p_rRes);
    ResultEvaluation testValidateResult(const interface::ResultArgs& p_rRes);
    virtual ResultEvaluationData testResult(const interface::ResultArgs &p_rRes, const interface::ResultArgs& lwmTriggerSignal) override;

    AnalysisSumErrorSingleOutlier& operator=(AnalysisSumErrorSingleOutlier const&);
    virtual AnalysisSumErrorSingleOutlier* clone();
    void copyValues(AnalysisSumErrorSingleOutlier* dest, AnalysisSumErrorSingleOutlier const &src);
};
//--------------------------------------------------------------------------------------//


} // namespace interface
} // namespace precitec
