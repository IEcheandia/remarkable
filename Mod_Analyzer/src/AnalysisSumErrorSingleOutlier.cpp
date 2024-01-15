/**
 *
 *  @file       AnalysisSumErrorSingleOutlier.cpp
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Daniel Zumkeller
 *  @date       20.03.2020
 *  @brief      Analysis Type Sum Error Implementation for detection of single Length/Area-Inliers. Derived from the generic Analysis Sum Error.
 * 
 *  This class is derived from the base class lying in AnalysisSumError.cpp file
 */

#include "analyzer/AnalysisSumError.h"


///////////////////////////////////////////////////////////
// Analysis Error Single Outlier Check Class             //
///////////////////////////////////////////////////////////
namespace precitec
{
namespace analyzer
{

using precitec::interface::ResultArgs;
using precitec::interface::SmpResultPtr;
using precitec::interface::MeasureTask;
using precitec::interface::ResultType;


AnalysisSumErrorSingleOutlier::~AnalysisSumErrorSingleOutlier()
{
}

AnalysisSumErrorSingleOutlier* AnalysisSumErrorSingleOutlier::clone()
{
    AnalysisSumErrorSingleOutlier* cpy = new AnalysisSumErrorSingleOutlier(this->m_oMaxViolation);
    this->AnalysisSumError::copyValues(cpy, *this);
    return cpy;
}

void AnalysisSumErrorSingleOutlier::init()
{
    this->AnalysisSumError::init();
    m_oErrorType = ResultType::AnalysisSumErrorAdjacentOutlier;
    m_oHasBeenSend = false;
}

std::uint64_t AnalysisSumErrorSingleOutlier::getLastErrorDistance(const ResultArgs& p_rRes)
{

    if (!m_oLastResultsList.empty())
    {
        ResultArgs oLastResult = m_oLastResultsList.front();
        SumError::tIndex oImageNumberCurrent = getImageTrigger(p_rRes);
        SumError::tIndex oImageNumberLast = getImageTrigger(oLastResult);

#ifdef __debugAnalysisError
        wmLog( eDebug, "AnalysisSumErrorSingleOutlier::getLastErrorDistance() lastResult    imageNumber <%d> pos <%d>\n", (int)getImageTrigger(oLastResult), (int)getPositionTrigger(oLastResult) );
        wmLog( eDebug, "AnalysisSumErrorSingleOutlier::getLastErrorDistance() currentResult imageNumber <%d> pos <%d>\n", (int)getImageTrigger(p_rRes), (int)getPositionTrigger(p_rRes));
#endif
        ResultEvaluation oResEval(eNotMasked);
        oResEval = testValidateResult(oLastResult);
        if (oResEval != eMasked)
        {
            // last result had no error
            return 0;
        }
        // test the entries of adjacenting - use the trigger distance, usualy the distance between two images in um ..todo:: can be 0! dont use this now
        if ((oImageNumberCurrent - oImageNumberLast) != 1)
        {
            // last stored result is not the image before the current
#ifdef __debugAnalysisError
            wmLog( eDebug, "AnalysisSumErrorSingleOutlier::getLastErrorDistance() currentResult <%d> oLastResult <%d>\n", (int)oImageNumberCurrent, (int)oImageNumberLast);
#endif
            return 0;
        }

        return 1;
    }
    // very first entry. Though it really has distance 0 to itself, we send 1 to mark it "adjacent to itself"
    return 1;
}


ResultEvaluation AnalysisSumErrorSingleOutlier::testValidateResult(const ResultArgs& p_rRes)
{
    ResultEvaluation oResEval(eNotMasked);

    if (p_rRes.isNio() == false)
    {
        // the result is not NIO, no Analysis Error
#ifdef __debugAnalysisError
        wmLog( eDebug, "AnalysisSumErrorSingleOutlier::testValidateResult() imageNumber <%d> pos <%d> is not NIO, return <%d> \n", (int)getImageTrigger(p_rRes), (int)getPositionTrigger(p_rRes), oResEval);
#endif
        return oResEval;
    }
    // general tests:
    oResEval = AnalysisSumError::testAnalysisResult(p_rRes);
#ifdef __debugAnalysisError
    wmLog( eDebug, "AnalysisSumErrorSingleOutlier::testValidateResult() imageNumber <%d> pos <%d>  returns <%d> \n", (int)getImageTrigger(p_rRes), (int)getPositionTrigger(p_rRes),  oResEval);
#endif
    return oResEval;
}

ResultEvaluationData AnalysisSumErrorSingleOutlier::testResult(const ResultArgs& p_rRes, const interface::ResultArgs& lwmTriggerSignal) //TODO recode to calculate signalQuality and nioPercentage if needed
{
    //TODO verify if this method is ever called and if this class can be deleted
    wmLog(eWarning, "Unexpected call to AnalysisSumErrorSingleOutlier::testResult\n");
    return SumError::testResult(p_rRes, lwmTriggerSignal);

    #ifdef __debugAnalysisError
        MeasureTask const &oMt = *(p_rRes.taskContext().measureTask());
        interface::ImageContext const &oIct = p_rRes.context();
        std::tuple<std::int32_t, std::int32_t, std::int32_t> oCurrentMeasureTask = std::make_tuple(oMt.seamseries(), oMt.seam(), oMt.seaminterval());
        wmLog( eDebug, "AnalysisSumErrorSingleOutlier::testResult() at <%d> <%d> <%d>\n",std::get<0>(oCurrentMeasureTask), std::get<1>(oCurrentMeasureTask), std::get<2>(oCurrentMeasureTask));
        wmLog( eDebug, "     ResultTyp <%d> imgNumber <%d> pos (um) <%d> measFlag <%d>\n", p_rRes.resultType(), (int)oIct.imageNumber(), (int)oIct.position(), (int)oIct.measureTaskPosFlag());
    #endif
    
    ResultEvaluationData result{eNotMasked, {}, {}};

    if (p_rRes.isNio() == false)
    {
        // the result is not NIO, no Analysis Error
        #ifdef __debugAnalysisError
                wmLog( eDebug, "AnalysisSumErrorSingleOutlier::testResult() is not NIO, return\n");
        #endif
        return result;
    }
    // singularity test - only proceed with one result
    if (AnalysisSumError::testSingularity(p_rRes) == eSumErrorNotTriggered)
    {
        result.evaluationStatus = eSumErrorNotTriggered;
    #ifdef __debugAnalysisError
            wmLog( eDebug, "AnalysisSumErrorSingleOutlier::testResult() imageNumber <%d> pos <%d> already done, return <%d>\n", 
            (int)getImageTrigger(p_rRes), (int)getPositionTrigger(p_rRes), eSumErrorNotTriggered);
    #endif
        return result;
    }

    result.evaluationStatus = AnalysisSumError::testAnalysisResult(p_rRes);
    // common test (incorrect error Type, no scope...)
    if ((result.evaluationStatus == eBadScope) || (result.evaluationStatus == eNotMasked))
    {
        return result;
    }
    updateTriggerDistance(p_rRes);

    result.evaluationStatus = eSumErrorNotTriggered;
    // test if the last result has an error
    //TODO check if even needed...
    int32_t oErrorDist = (int)getLastErrorDistance(p_rRes);
    //wmLog( eDebug, "AnalysisSumErrorSingleOutlier::testResult() m_oFlawedViolation <%f> m_oTriggerDistance <%d>\n", m_oFlawedViolation, m_oTriggerDistance);
    if ((oErrorDist == 1) || (m_oFlawedViolation == 0)) // adjacent error?
    {
#ifdef __debugAnalysisError
        wmLog( eDebug, "AnalysisSumErrorSingleOutlier::testResult() errorDist <%d> m_oFlawedViolation <%f> m_oTriggerDistance <%d>\n", oErrorDist,  m_oFlawedViolation, m_oTriggerDistance);
#endif
        if (m_oFlawedViolation == 0)
        {
            SumError::clearErrors();
        }
    }
    else
    {
#ifdef __debugAnalysisError
        wmLog( eDebug, "AnalysisSumErrorSingleOutlier::testResult() reset values ! errorDist <%d>\n", oErrorDist);
#endif
        m_oFlawedViolation = 0; // reset counter
        SumError::clearErrors();
    }
    m_oFlawedViolation += m_oTriggerDistance; // Violation and distance depict number of frames 

#ifdef __debugAnalysisError
    wmLog( eDebug, "AnalysisSumErrorSingleOutlier::testResult() new FlawedViolation: [%f] (triggerDist: [%f])\n", m_oFlawedViolation, m_oTriggerDistance);
#endif
    if ((m_oMaxViolation > 0) && (m_oFlawedViolation > m_oMaxViolation))
    {
        if (!m_oHasBeenSend) // only send once
        {
#ifdef __debugAnalysisError
            wmLog( eDebug, "AnalysisSumErrorSingleOutlier::testResult() Error condition reached <%f> ! -> Inform Host\n", m_oMaxViolation);
#endif
            m_oHasBeenSend = true;
            result.evaluationStatus = eSumErrorTriggered;
            return result;
        }
        else
        {
            return result;
        }
    }
    return result;
}

AnalysisSumErrorSingleOutlier& AnalysisSumErrorSingleOutlier::operator=(AnalysisSumErrorSingleOutlier const &rhs)
{
    if (this != &rhs)
    {
        this->AnalysisSumError::operator=(rhs);
    }
    return *this;
}

} // namespace interface
} // namespace precitec
