/**
 *  @file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 *  @author		Ralph Kirchner (KiR), Andreas Beschorner (AB)
 *  @date		2011
 *  @brief		The resultmanager is connected to the (invisible) out-pipes of the result filters and is responsible to send the data to windows.
 */

#include "analyzer/resultHandler.h"
#include "event/results.proxy.h"
#include "event/results.interface.h"
#include "geo/geo.h"
#include "module/moduleLogger.h"


static const std::vector<int> s_lwmResultTypes = {737, 738, 739, 740};

using namespace fliplib;

namespace precitec
{
    using namespace interface;
    using namespace system;
    using namespace workflow;
namespace analyzer
{


ResultHandler::ResultHandler(TResults<AbstractInterface>  *resultProxy) :
    SinkFilter("analyzer::resulthandler"),
    resultProxy_( resultProxy ), m_pProduct(nullptr), m_oNioReceived( {{ false }} ), m_lastImageProcessed(-1)
{
    m_oState = State::eInit;
}

/// set new product reference
void ResultHandler::setProduct(const analyzer::Product* p_pProduct)
{
    m_pProduct = p_pProduct;
}

void ResultHandler::setState(State p_oState)
{
    m_oState = p_oState;
}

bool ResultHandler::nioReceived(std::size_t p_oSlot)
{
    const auto oNioReceived	=	m_oNioReceived[p_oSlot];
    m_oNioReceived[p_oSlot]	=	false;

    return oNioReceived;
}

void ResultHandler::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e)
{
#ifdef __debugSumErrors
        wmLog( eDebug, "\n");
        wmLog( eDebug, "ResultHandler::proceedGroup()\n");
#endif

    std::vector<ResultDoubleArray> resultsFromPipe;
    resultsFromPipe.reserve(m_oInPipes.size());

    const auto& lwmTriggerSignal = m_pProduct->m_rProductData.lwmTriggerSignal();
    ResultArgs lwmTriggerSignalResults = {};

    for (auto inPipe : m_oInPipes)
    {
        if (!inPipe->dataAvailable(m_oCounter))
        {
            continue;
        }

        auto result = inPipe->read(m_oCounter);

        if (result.resultType() == lwmTriggerSignal)
        {
            lwmTriggerSignalResults = result;
        }

        resultsFromPipe.push_back(std::move(result));
    }

    std::vector<ResultDoubleArray> resultsAfterInspection;
    resultsAfterInspection.reserve(m_oInPipes.size());

    for (auto& result : resultsFromPipe)
    {
        if (std::any_of(s_lwmResultTypes.begin(), s_lwmResultTypes.end(), [&result] (auto type) { return type == result.resultType(); }))
        {
            resultsAfterInspection.push_back(sendResult(result, lwmTriggerSignalResults));
        } else
        {
            resultsAfterInspection.push_back(sendResult(result));
        }
    }

    if (resultProxy_ && !resultsAfterInspection.empty())
    {
        resultProxy_->result(resultsAfterInspection);
    }
    m_lastImageProcessed = m_oCounter;

    preSignalAction(); // unlock
    // no signal() in result handlers
    *m_processing = false;
} // proceed


void ResultHandler::sendResultAsIOAndValid( ResultDoubleArray &resultArgs )
{
    resultArgs.setNio(false);          // reset the flag
    
    // if in live-mode, one can disable the hardware-results 
    if ( g_oDisableHWResults && m_oState == workflow::State::eLiveMode )
    {
        resultArgs.setValid( false );
    }
    else
    {
        resultArgs.setValid(true);         // reset the flag
    }
}


void ResultHandler::setResultDeviationfromSumErrorLimits( ResultDoubleArray &resultArgs, SmpSumError &sumError )
{
    double lower;
    double upper;
    sumError->getSELimits(lower, upper);
    resultArgs.setdeviation(lower, upper);
}

// if there are more then one SE for one resulttype, 
// we write the SE limits from the lowest scope in the resultdeviation
// lowest scope is seaminterval, highest scope is product
void ResultHandler::setResultDeviationGivenFromLowestScope( tSumErrorList &summErrorList, ResultDoubleArray &resultArgs )
{
    if (summErrorList.size() > 0)
    {
        for (auto &sumError : summErrorList)
        {
            if(sumError->scope() == ErrorScopeType::eScopeSeaminterval)
            {
                setResultDeviationfromSumErrorLimits(resultArgs, sumError);
                continue;
            }
            if(sumError->scope() == ErrorScopeType::eScopeSeam)
            {
                setResultDeviationfromSumErrorLimits(resultArgs, sumError);
                continue;
            }
            if(sumError->scope() == ErrorScopeType::eScopeSeamseries)
            {
                setResultDeviationfromSumErrorLimits(resultArgs, sumError);
                continue;
            }
            if(sumError->scope() == ErrorScopeType::eScopeProduct)
            {
                setResultDeviationfromSumErrorLimits(resultArgs, sumError);
                continue;
            }
        }
    }
}

// if we use SE for a resulttype overwrite the resultdeviation for saving in database optionaly.
ResultDoubleArray ResultHandler::sendResult( ResultDoubleArray &p_rRes, interface::ResultDoubleArray lwmTriggerResult)
{
    if (resultProxy_ != nullptr)
    {

        ResultDoubleArray p_rIORes = p_rRes; //make a copy of the result, to be set to IO and sent to the plotter 
        
        //special processing for results coming from debug timings
        if (p_rRes.resultType() == ResultType::ProcessTime || p_rRes.resultType() == ResultType::InspectManagerTime) 
        {
            sendResultAsIOAndValid(p_rIORes);
            resultProxy_->result(p_rIORes);
            return p_rIORes;
        }
        
        tSumErrorList oSumErrors;
        if (m_pProduct != nullptr)
        {
            ResultEvaluationList resultList = m_pProduct->seHandleResult(oSumErrors, p_rRes, lwmTriggerResult);
            
#ifdef __debugSumErrors  
            wmLog( eDebug, "ResultHandler::sendResult() start ImageNumber:[%d] Pos:[%d] resultType:[%d] nioType:[%d]\n", p_rRes.context().imageNumber(), p_rRes.context().position(), p_rRes.resultType(),p_rRes.nioType());
#endif
            
            tSumErrorList sumErrors;
            m_pProduct->getSumErrorsGivenScopeAndResultTyp(sumErrors, p_rIORes);
            setResultDeviationGivenFromLowestScope(sumErrors, p_rIORes);
            // SumError is defined for this result; check the content of oSumErrors
            if (oSumErrors.size() > 0)
            {
                m_oNioReceived[m_oCounter % g_oNbPar] = true;
                if (oSumErrors.size() != resultList.size()) { wmLog(eError, "ResultHandler::sendResult() resultList size does not match number of errors on this scope!"); return p_rIORes; }
                
                for (auto i = 0u; i < oSumErrors.size(); i++)
                {
                    auto &oError = oSumErrors[i];
                    auto evaluationResult = resultList[i];

                    if (!evaluationResult.signalQuality.empty())
                    {
                        p_rIORes.setQuality(evaluationResult.signalQuality);
                    }
                    if (!evaluationResult.upperReference.empty())
                    {
                        p_rIORes.setUpperReference(evaluationResult.upperReference);
                        p_rIORes.setLowerReference(evaluationResult.lowerReference);
                    }
                    
                    if (!evaluationResult.nioPercentage.empty())
                    {
                        ResultDoubleArray errorEvaluationResult = ResultDoubleArray(p_rRes);
                        
                        errorEvaluationResult.setNioResult(evaluationResult.nioPercentage);
                        
                        errorEvaluationResult.setResultType(oError->errorType()); //ErrorType chosen by user
                        if (((errorEvaluationResult.resultType() >= QualityFaultTypeA) && (errorEvaluationResult.resultType() <= QualityFaultTypeX)) ||
                            ((errorEvaluationResult.resultType() >= QualityFaultTypeA_Cat2) && (errorEvaluationResult.resultType() <= QualityFaultTypeX_Cat2)) ||
                            (errorEvaluationResult.resultType() == FastStop_DoubleBlank))
                        {
                            errorEvaluationResult.setNioType(errorEvaluationResult.resultType());
#ifdef __debugSumErrors  
                            wmLog( eDebug, "ResultHandler::sendResult() QualityFault!! values resultType:[%d] nioType:[%d] \n", 
                                    errorEvaluationResult.resultType(), errorEvaluationResult.nioType());
#endif
                        }
                    
                        #ifdef __debugSumErrors
                            wmLog(eDebug, "ResultHandler::sendResult() exchange resultType:[%d] with sumErrType:[%d]. sigQualSize:[%d], nioPercSize:[%d]!!\n", 
                                    errorEvaluationResult.resultType(), oError->errorType(), evaluationResult.signalQuality.size(), evaluationResult.nioPercentage.size());
                            wmLog(eDebug, "ResultHandler::sendResult() send SumError:[%d] !! ImageNumber:[%d] Position:[%d]\n", 
                                    oError->sumErrorType(), errorEvaluationResult.context().imageNumber(), errorEvaluationResult.context().position());
                            wmLog(eDebug, "errorEvaluationResult data: isNio:[%d] isValid:[%d] resultType:[%d] nioType:[%d] \n", 
                                    errorEvaluationResult.isNio(), errorEvaluationResult.isValid(), errorEvaluationResult.resultType(), errorEvaluationResult.nioType());
                        #endif
                        
                        setResultDeviationfromSumErrorLimits(errorEvaluationResult, oError);
                        if (evaluationResult.evaluationStatus == eSumErrorTriggered) 
                        {
                            
                            errorEvaluationResult.setNio(true); 
                            resultProxy_->nio(errorEvaluationResult);        
                        }
                        else
                        {
                            resultProxy_->result(errorEvaluationResult);
                        }
                    }
                    
                   
                }
            }
            sendResultAsIOAndValid(p_rIORes);
            return p_rIORes;
        }
        else if (m_oState != workflow::State::eLiveMode)
        {
            wmLog(eError, "Internal error Result-Handler: Product is nullptr");
        }
    }
    else
    {
        wmLog(eError, "%s: Result proxy is nullptr", __FUNCTION__);
    }
#ifdef __debugSumErrors
    wmLog( eDebug, "ResultHandler::sendResult() ends\n");
#endif
    return {};
}

void ResultHandler::startProcessing()
{
    *m_processing = true;
}

bool ResultHandler::isProcessing() const
{
    return *(const_cast<ResultHandler*>(this)->m_processing);
}

} // namespace analyzer
} // namespace precitec
