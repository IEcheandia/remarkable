/**
 *
 *  @file       AnalysisSumError.cpp
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Daniel Zumkeller
 *  @date       20.03.2020
 *  @brief      Analysis Type Sum Error Base Class Implementation.
 * 
*/

#include "analyzer/AnalysisSumError.h"


///////////////////////////////////////////////////////////
// Analysis Error Base class                             //
///////////////////////////////////////////////////////////
namespace precitec
{
namespace analyzer
{

using precitec::interface::ResultArgs;
using precitec::interface::SmpResultPtr;
using precitec::interface::MeasureTask;
using precitec::interface::ResultType;


AnalysisSumError::~AnalysisSumError()
{
}

AnalysisSumError& AnalysisSumError::operator=(AnalysisSumError const &rhs)
{
    if (this != &rhs)
    {
        this->SumError::operator=(rhs);
        copyValues(this, rhs);
    }
    return *this;
}

void AnalysisSumError::init()
{
    reset();
    m_oScope = eScopeSeam;
    m_oFlawedViolation = 0.0;
    m_oFaultlessValue = 0.0;
    m_oErrorType = ResultType::SumErrorNone;
    setTriggerBase(TriggerBase::ePositionTrigger);
}

void AnalysisSumError::reset()
{
    m_oFlawedViolation = 0.0;
    m_oFaultlessValue = 0.0;
    this->SumError::reset();
}

void AnalysisSumError::setErrorType(const std::int32_t p_oType)
{
    m_oErrorType = (ResultType)p_oType;
#ifdef __debugSumError
    wmLog( eDebug, "AnalysisSumError::setErrorType m_oErrorType <%d>\n", p_oType);
#endif
}

void AnalysisSumError::copyValues(AnalysisSumError* dest, AnalysisSumError const &src)
{
    dest->SumError::copyValues(dest, src);
    dest->m_oMaxViolation = src.m_oMaxViolation;
    dest->m_oFlawedViolation = src.m_oFlawedViolation;
}

ResultEvaluation AnalysisSumError::testAnalysisResult(const ResultArgs& p_rRes)
{
    ResultEvaluation oTypeOrScope(eNotMasked);

    // returns false if result is not added (incorrect type, not in scope, ...)
    if (m_oTriggerDistance <= 0)
    {
        updateTriggerDistance(p_rRes);
    }

    // test result type
    if (m_oResultTypes.size() <= 0)   // nothing to surveil...
    {
#ifdef __debugAnalysisError
        wmLog( eDebug, "AnalysisSumError::testAnalysisResult() m_oResultTypes.size() <= 0; nothing to surveil..  return!\n" );
#endif
        return oTypeOrScope;
    }

    for (unsigned int i = 0; i < m_oResultTypes.size(); ++i)
    {
#ifdef __debugAnalysisError
        wmLog( eDebug, "AnalysisSumError::testAnalysisResult() imageNumber <%d> ResultTyp <%d>\n", (int)getImageTrigger(p_rRes), m_oResultTypes[i]);
#endif
        // Analysis : we are looking for the nioType coming with the result
        if (p_rRes.nioType() == m_oResultTypes[i])
        {
#ifdef __debugAnalysisError
            wmLog( eDebug, "AnalysisSumError::testAnalysisResult() found ResultTyp <%d> nioType <%d> m_oErrorType <%d>\n", m_oResultTypes[i],  p_rRes.nioType(), m_oErrorType);
#endif
            // marker for continue as found Result..
            oTypeOrScope = eBadScope;
            break;
        }
    }

    if (oTypeOrScope == eNotMasked)
    {
#ifdef __debugAnalysisError
        wmLog( eDebug, "AnalysisSumError::testAnalysisResult() no error defined for AnalysisResultTyp <%d>\n", p_rRes.nioType());
#endif
        return oTypeOrScope;
    }

    // test scope
    oTypeOrScope = SumError::testScope(p_rRes);

    return oTypeOrScope;
}

ResultEvaluation AnalysisSumError::testSingularity(const ResultArgs& p_rRes)
{
    // test for the singularity of the result ,analysisError come with each defined resultfilter,
    // but we only need one result for testing
    ResultEvaluation oTypeOrScope(eNotMasked);
    // store the actual result
    m_oSingularityList.push_back(p_rRes);
    if (m_oSingularityList.size() == 1)
    {
        // first entry, return for processing the result
        oTypeOrScope = eMasked;
#ifdef __debugAnalysisError
        wmLog( eDebug, "AnalysisSumError::testSingularity() first entry, return for processing the result, <%d>\n", oTypeOrScope);
#endif
    }
    else
    {
        // check if actual result is the same image again
        ResultArgs firstResult = m_oSingularityList.front();
        int firstImageNumber = firstResult.context().imageNumber();
        int actImageNumber = p_rRes.context().imageNumber();
#ifdef __debugAnalysisError
        wmLog( eDebug, "AnalysisSumError::testSingularity()\n");
        wmLog( eDebug, "     firstResultImgNr  <%d> resultType <%d> nioType <%d>\n", firstImageNumber, firstResult.resultType(), firstResult.nioType());
        wmLog( eDebug, "     actualResultImgNr <%d> resultType <%d> nioType <%d>\n", actImageNumber, p_rRes.resultType(), p_rRes.nioType());
#endif
        if (firstImageNumber == actImageNumber)
        {
            oTypeOrScope = eSumErrorNotTriggered;
#ifdef __debugAnalysisError
           wmLog( eDebug, "AnalysisSumError::testSingularity() already done (firstImageNumber <%d> == actImageNumber <%d>) <%d>\n", firstImageNumber, actImageNumber, oTypeOrScope);
#endif
        }
        else
        {
            // image number changed, reset the list
            m_oSingularityList.clear();
            // add the new result, must be the first
            m_oSingularityList.push_back(p_rRes);
            // new entry, return for processing the result
            oTypeOrScope = eMasked;
#ifdef __debugAnalysisError
            wmLog( eDebug, "AnalysisSumError::testSingularity() image number changed, return for processing the result. <%d>\n", oTypeOrScope);
#endif
        }
    }
    return oTypeOrScope;
}


} // namespace interface
} // namespace precitec
