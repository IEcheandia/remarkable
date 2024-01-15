/**
 *
 *  @file       AnalysisSumErrorAccumulatedOutlier.cpp
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Daniel Zumkeller
 *  @date       20.03.2020
 *  @brief      Analysis Type Sum Error Implementation for detection of accumulated Length/Area-Inliers. Derived from the generic Analysis Sum Error.
 * 
 *  This class is derived from the base class lying in AnalysisSumError.cpp file
 */

#include "analyzer/AnalysisSumError.h"


///////////////////////////////////////////////////////////
// Analysis Error Accumulated (All) Outlier Check Class  //
///////////////////////////////////////////////////////////
namespace precitec
{
namespace analyzer
{

using precitec::interface::ResultArgs;
using precitec::interface::SmpResultPtr;
using precitec::interface::MeasureTask;
using precitec::interface::ResultType;


AnalysisSumErrorAccumulatedOutlier::~AnalysisSumErrorAccumulatedOutlier()
{
}

AnalysisSumErrorAccumulatedOutlier* AnalysisSumErrorAccumulatedOutlier::clone()
{
    AnalysisSumErrorAccumulatedOutlier* cpy = new AnalysisSumErrorAccumulatedOutlier(this->m_oMaxViolation);
    this->AnalysisSumError::copyValues(cpy, *this);
    return cpy;
}

void AnalysisSumErrorAccumulatedOutlier::init()
{
    this->AnalysisSumError::init();
    m_oErrorType = ResultType::AnalysisSumErrorAccumulatedOutlier;
}


ResultEvaluationData AnalysisSumErrorAccumulatedOutlier::testResult(const ResultArgs& p_rRes, const interface::ResultArgs& lwmTriggerSignal) //TODO recode to calculate nioPercentage and signalQuality...
{
    //TODO verify if this method is ever called and if this class can be deleted
    wmLog(eWarning, "Unexpected call to AnalysisSumErrorAccumulatedOutlier::testResult\n");
    return SumError::testResult(p_rRes, lwmTriggerSignal);

    ResultEvaluationData result{eNotMasked, {}, {}};
    
#ifdef __debugAnalysisError
    MeasureTask const &oMt = *(p_rRes.taskContext().measureTask());
    interface::ImageContext const &oIct = p_rRes.context();
    std::tuple<std::int32_t, std::int32_t, std::int32_t> oCurrentMeasureTask = std::make_tuple(oMt.seamseries(), oMt.seam(), oMt.seaminterval());
    wmLog( eDebug, "AnalysisSumErrorAccumulatedOutlier::testResult() at <%d> <%d> <%d>\n",std::get<0>(oCurrentMeasureTask), std::get<1>(oCurrentMeasureTask), std::get<2>(oCurrentMeasureTask));
    wmLog( eDebug, "     ResultTyp <%d> imgNumber <%d> pos (um) <%d> measFlag <%d>\n", p_rRes.resultType(), (int)oIct.imageNumber(), (int)oIct.position(), (int)oIct.measureTaskPosFlag());
#endif
    
    if (p_rRes.isNio() == false)
    {
        // the result is not NIO, no Analysis Error
#ifdef __debugAnalysisError
        wmLog( eDebug, "AnalysisSumErrorAccumulatedOutlier::testResult() is not NIO, nothing to do; return\n");
#endif
        return result;
    }

    // singularity test - only proceed with one result
    result.evaluationStatus = AnalysisSumError::testSingularity(p_rRes);
    if (result.evaluationStatus == eSumErrorNotTriggered)
    {
#ifdef __debugAnalysisError
        interface::ImageContext const &oIct = p_rRes.context();
        wmLog( eDebug, "AnalysisSumErrorAccumulatedOutlier::testResult() imageNumber <%d> already done, return\n", (int)oIct.imageNumber());
#endif
        return result;
    }

    // common test (incorrect error Type, no scope...)
    result.evaluationStatus = AnalysisSumError::testAnalysisResult(p_rRes);
    if ((result.evaluationStatus == eBadScope) || (result.evaluationStatus == eNotMasked))
    {
        return result;
    }
    updateTriggerDistance(p_rRes);
    result.evaluationStatus = eSumErrorNotTriggered;

#ifdef __debugAnalysisError
    wmLog( eDebug, "AnalysisSumErrorAccumulatedOutlier::testResult() isNio <%d> resultType <%d> nioType <%d>\n", p_rRes.isNio(), p_rRes.resultType(), p_rRes.nioType());
    wmLog( eDebug, "AnalysisSumErrorAccumulatedOutlier::testResult() type <%d> FlawedViolation <%f> MaxViolation <%f>\n", p_rRes.type(), m_oFlawedViolation, m_oMaxViolation);
#endif
    m_oFlawedViolation += m_oTriggerDistance;

#ifdef __debugAnalysisError
   wmLog( eDebug, "AnalysisSumErrorAccumulatedOutlier::testResult() new FlawedViolation: [%f] (triggerDist: [%f])\n", m_oFlawedViolation, m_oTriggerDistance);
#endif
    if ((m_oMaxViolation > 0) && (m_oFlawedViolation > m_oMaxViolation))
    {
        if (!m_oHasBeenSend) // only send once
        {
#ifdef __debugAnalysisError
             wmLog( eDebug, "AnalysisSumErrorAccumulatedOutlier::testResult() Error condition reached <%f> ! -> Inform Host\n", m_oMaxViolation);
#endif
            m_oHasBeenSend = true;
            result.evaluationStatus = eSumErrorTriggered;
        }
        else
        {
            return result;
        }
    }

    return result;
}

AnalysisSumErrorAccumulatedOutlier& AnalysisSumErrorAccumulatedOutlier::operator=(AnalysisSumErrorAccumulatedOutlier const &rhs)
{
    if (this != &rhs)
    {
        this->AnalysisSumError::operator=(rhs);
    }
    return *this;
}

} // namespace interface
} // namespace precitec
