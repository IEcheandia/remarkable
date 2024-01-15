/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author       Andreas Beschorner (AB)
 *  @date           2012
 * 
 *  Adapted for oversampling and refactored: Daniel Zumkeller 20.03.2020
*/

#include "analyzer/sumError.h"



//////////////////////////////////////////////
/* Implementation of base class SumError */
//////////////////////////////////////////////
namespace precitec
{
namespace analyzer
{

using precitec::interface::ResultArgs;
using precitec::interface::SmpResultPtr;
using precitec::interface::MeasureTask;
using precitec::interface::ResultType;
using precitec::interface::TResults;

SumError::~SumError()
{
}

SumError& SumError::operator=(SumError const &rhs)
{
    if (this != &rhs)
    {
        copyValues(this, rhs);
    }
    return *this;
}

void SumError::copyValues(SumError *dest, const SumError& src)
{
    dest->m_oScope = src.m_oScope;
    dest->m_oTriggerDistance = src.m_oTriggerDistance;
    dest->m_oTriggerBase = src.m_oTriggerBase;
    dest->m_oPercentageLimit = src.m_oPercentageLimit;
    dest->m_oErrorType = src.m_oErrorType;
    dest->m_oGuid = src.m_oGuid;
    dest->m_oGetTriggerAlgorithm = src.m_oGetTriggerAlgorithm;

    //-- delete own lists and copy new ones --//

    // Resulttypes just need to be copied.
    dest->m_oResultTypes.clear();
    for (unsigned int i=0; i < src.m_oResultTypes.size(); ++i)
    {
        dest->m_oResultTypes.push_back(src.m_oResultTypes[i]);
    }
}

void SumError::reset()
{
    m_oTriggerDistance = -1;
    m_oHasBeenSend = false;
    clearErrors();
}

void SumError::clearErrors()
{
    m_oLastResultsList.clear();
}

void SumError::clear()
{
    m_oResultTypes.clear();
    this->clearErrors();
}

/*virtual*/ interface::ResultArgs* SumError::arm(const fliplib::ArmStateBase& state )
{
	return nullptr;
}

bool SumError::operator | (SumError& p_rOther) const
{
    return this->isError() | p_rOther.isError();
}

bool SumError::operator & (SumError& p_rOther) const
{
    return this->isError() & p_rOther.isError();
}

bool SumError::setScope(const ErrorScopeType p_oScope, const std::int32_t p_oSeamseries, const std::int32_t p_oSeam, const std::int32_t p_oSeaminterval)
{
    if ( (p_oScope >= eScopeNone) && (p_oScope < eNumScopes) )
    {
        if ( (p_oScope >= eScopeProduct) && (p_oScope <= eScopeSeaminterval) )
        {
            m_oScope = p_oScope;
        } else
        {
            wmLogTr(eError, "QnxMsg.Misc.SEScopeInv", "Invalid scope for sumerror. Must be product, seamseries, seam or seaminterval.");
            return false;
        }
    } else
    {
        m_oScope = eScopeNone;
        wmLogTr(eError, "QnxMsg.Misc.SEScopeInv", "Invalid scope for sumerror. Must be product, seamseries, seam or seaminterval.");
        return false;
    }
    m_oSeamseries = p_oSeamseries;
    m_oSeam = p_oSeam;
    m_oSeaminterval = p_oSeaminterval;
    return true;
}

void SumError::setPercentageLimit(int& p_rLimit)
{

}

/** region trigger counter algorithm functors (imagenr, (encoder)position, timecode) */
SumError::tIndex SumError::getImageTrigger(const ResultArgs& p_rRes)
{
    return p_rRes.context().imageNumber();
}

// not yet implemented, returns false
SumError::tIndex SumError::getTimecodeTrigger(const ResultArgs& p_rRes)
{
    // not yet implemented (hard to tell the exact distance of relative time, thus adjacent errors are nearly impossible to be determined)
    wmLogTr(eError, "QnxMsg.Misc.SETimecodeImpl", "Timecode trigger based error handling not yet implemented.");
    return -1;
}

SumError::tIndex SumError::getPositionTrigger(const ResultArgs& p_rRes)
{
    return p_rRes.context().position();
}

void SumError::setTriggerBase(const TriggerBase& p_rTrigger)
{
    m_oTriggerDistance = -1; // if -1 it must be set at some time! In our case, the function updateTriggerDistance does the job.
    switch (p_rTrigger)
    {
        case TriggerBase::eImageTrigger:
        {
            //wmLog( eDebug, "SumError::setTriggerBase() TriggerBase::eImageTrigger :  m_oGetTriggerAlgorithm = &SumError::getImageTrigger\n");
            m_oGetTriggerAlgorithm = std::mem_fn(&SumError::getImageTrigger);
            m_oTriggerDistance = 1;
            break;
         }
        case TriggerBase::eTimecodeTrigger:
        {
            //wmLog( eDebug, "SumError::setTriggerBase() TriggerBase::eTimecodeTrigger :  m_oGetTriggerAlgorithm = &SumError::getTimecodeTrigger\n");
            m_oGetTriggerAlgorithm = std::mem_fn(&SumError::getTimecodeTrigger);
            break;
        }
        case TriggerBase::ePositionTrigger:
        {
            //wmLog( eDebug, "SumError::setTriggerBase() TriggerBase::ePositionTrigger :  m_oGetTriggerAlgorithm = &SumError::getPositionTrigger\n");
            m_oGetTriggerAlgorithm = std::mem_fn(&SumError::getPositionTrigger);
            break;
        }
        default: 
            wmLog( eDebug, "SumError::setTriggerBase() default m_oGetTriggerAlgorithm = nullptr\n");
            m_oGetTriggerAlgorithm = nullptr; 
            break;
    }
}
/** endregion trigger counter */

// the base class only tests whether the parameter result type is found in the result type list for the SumError
ResultEvaluationData SumError::testResult(const ResultArgs& p_rRes, const interface::ResultArgs& lwmTriggerSignal)
{
    ResultEvaluationData result{findResult(p_rRes), {}, {}};
    return result;
}

ResultEvaluation SumError::testSumErrorLevel2(const Poco::SharedPtr<SumError> &p_oSumErr, const interface::tSeamIndex index)
{
    ResultEvaluation oResEval(eNotMasked);
    return oResEval;
}

ResultEvaluation SumError::findResult(const ResultArgs& p_rRes) const
{
    ResultEvaluation oTypeOrScope(eNotMasked);

    // test if result valid
    if ( !p_rRes.isValid() )
    {
    	return oTypeOrScope;
    }

    // test result type
    if (m_oResultTypes.size() <= 0 )                    // nothing to surveil...
    {
#ifdef __debugSumError
        wmLog( eDebug, "SumError::testResult() m_oResultTypes.size() <= 0; return! ResultTyp <%d>\n", p_rRes.resultType());
#endif
        return oTypeOrScope;
    }
    for (unsigned int i=0; i < m_oResultTypes.size() && !oTypeOrScope; ++i)
    {
        // each result has one and only one result type. Ergo, as soon as we have a match, we can return.
        if ( p_rRes.resultType() == this->m_oResultTypes[i] )   // is (N)IO type one with impact?
        {
            oTypeOrScope = eBadScope;
        }
    }

    if (oTypeOrScope == eNotMasked)
    {
        return oTypeOrScope;
    }

    // test scope
    oTypeOrScope = SumError::testScope(p_rRes);

    return oTypeOrScope; // type is already ok here
}

void SumError::keepResultAsLastResult(const ResultArgs& p_rRes)
{
    // save only the last result, reset the list first
    m_oLastResultsList.clear();
    // add the new result, must be the first
    m_oLastResultsList.push_back(p_rRes);
}

void SumError::trackScopeOnProduct(const ResultArgs& p_rRes)
{
    return;
}

ResultEvaluation SumError::testScope(const ResultArgs& p_rRes) const
{
    ResultEvaluation oTypeOrScope(eNotMasked);
    MeasureTask const &mt = *(p_rRes.taskContext().measureTask());
    switch (m_oScope)
    {
    case eScopeProduct:
    {
        oTypeOrScope = eMasked;
        break;
    }
    case eScopeSeamseries:
    {
        if (m_oSeamseries == mt.seamseries())
        {
            oTypeOrScope = eMasked;
        }
        break;
    }
    case eScopeSeam:
    {
        if ((m_oSeamseries == mt.seamseries()) && (m_oSeam == mt.seam()))
        {
            oTypeOrScope = eMasked;
        }
        break;
    }
    case eScopeSeaminterval:
    {
        if ((m_oSeamseries == mt.seamseries()) && (m_oSeam == mt.seam()) && (m_oSeaminterval == mt.seaminterval()))
        {
            oTypeOrScope = eMasked;
        }
        break;
    }
    default: 
        break;
    }
    return oTypeOrScope;
}

bool SumError::updateTriggerDistance(const ResultArgs& p_rRes)
{
    switch (m_oTriggerBase)
    {
        case TriggerBase::eImageTrigger:
        {
            m_oTriggerDistance = 1;
            wmLog( eDebug, "SumError::updateTriggerDistance() TriggerBase::eImageTrigger : m_oTriggerDistance <%d>\n", m_oTriggerDistance);
            return true;
        }
        case TriggerBase::ePositionTrigger:
        {
            // wmLog( eDebug, "SumError::updateTriggerDistance() TriggerBase::ePositionTrigger :  m_oTriggerDistance <%d>\n", m_oTriggerDistance);
            if (!m_oLastResultsList.empty())
            {
                // the ImageContext of the actual image
                interface::ImageContext const &oIctNew = p_rRes.context();
                // the ImageContext of the previous image
                ResultArgs oLastResult = m_oLastResultsList.front();
                interface::ImageContext const &oIct = oLastResult.context();
                // calculate the trigger distance
                m_oTriggerDistance = oIctNew.position() - oIct.position();
#ifdef __debugSumError
                std::int32_t mTriggerDistance = oIctNew.taskContext().measureTask().get()->triggerDelta();
                wmLog( eDebug, "SumError::updateTriggerDistance() NEW imageNumber <%d> pos <%d> mTriggerDistance <%d>\n", oIctNew.imageNumber(), oIctNew.position(), mTriggerDistance);
                wmLog( eDebug, "SumError::updateTriggerDistance() OLD imageNumber <%d> pos <%d> m_oTriggerDistance <%d>\n", oIct.imageNumber(), oIct.position(), m_oTriggerDistance);
#endif
            }
            else
            {
                // first image, get the distance from the seam
                m_oTriggerDistance = p_rRes.context().taskContext().measureTask().get()->triggerDelta();
#ifdef __debugSumError
                // the ImageContext of the actual image
                interface::ImageContext const &oIctNew = p_rRes.context();
                wmLog( eDebug, "SumError::updateTriggerDistance() first image imageNumber <%d> pos <%d> m_oTriggerDistance <%d>\n", oIctNew.imageNumber(), oIctNew.position(), m_oTriggerDistance);
#endif
            }
            return true;
        }
        default:
        {
            m_oTriggerDistance = -1;
            wmLog( eDebug, "SumError::updateTriggerDistance() not defined; set to default!! : m_oTriggerDistance <%d>\n", m_oTriggerDistance);
            return false;
        }
    }
}

ErrorScopeType SumError::scopeStringToEnum(const std::string p_oScopeStr)
{
    std::string oScopeStr = p_oScopeStr;
    std::transform(oScopeStr.begin(), oScopeStr.end(), oScopeStr.begin(), ::toupper);
    if (oScopeStr == "PRODUCT")
    {
        return ErrorScopeType::eScopeProduct;
    } else
    if ( (oScopeStr == "SEAMSERIE") || (oScopeStr == "SEAMSERIES") )
    {
        return ErrorScopeType::eScopeSeamseries;
    } else
    if (oScopeStr == "SEAM")
    {
        return ErrorScopeType::eScopeSeam;
    } else
    if (oScopeStr == "SEAMINTERVAL")
    {
        return ErrorScopeType::eScopeSeaminterval;
    }
    return ErrorScopeType::eScopeNone;
}

char* SumError::getLong64AsString(tIndex lValue)
{
    static char chBuffer[256];
    memset(chBuffer, 0x0, 256);
    sprintf (chBuffer, "%" PRId64 , lValue);
    return chBuffer;
}

void SumError::setErrorType(const std::int32_t p_oType)
{
    return; // we cannot change the error Type of the base clase
}

void SumError::show()
{
    std::cout << "SumError\n--------\n  ErrorType: " << m_oErrorType << ";  Scope: " << m_oScope << "\n";
    std::cout << "  Seamseries: " << m_oSeamseries << ";  Seam: " << m_oSeam << ";  Seaminterval: " << m_oSeaminterval << "\n";
    std::cout << "  Triggerdistance: " << m_oTriggerDistance << ";  Triggerbase: " << m_oTriggerBase << "\n";
    std::cout << "  #SurveilledValues: <" << m_oResultTypes.size() << "> ResultTypes ";
    for (unsigned int i=0; i < m_oResultTypes.size(); ++i)
    {
        std::cout << " <" << m_oResultTypes[i] << ">";
    }
}

} // namespace interface
} // namespace precitec
