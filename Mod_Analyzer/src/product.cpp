/**
 *  @file
 *  @copyright    Precitec Vision GmbH & Co. KG
 *  @author       HS
 *  @date         2013
 *  @brief        'Product' is the highest entity in a station, defined by an ID and a number. A product holds one ore more seam series.
 */

#include "analyzer/product.h"
#include "analyzer/centralDeviceManager.h"
#include "analyzer/graphAssistent.h"
#include "module/moduleLogger.h"

using namespace fliplib;
namespace precitec {
  using namespace interface;
namespace analyzer {

Product::Product(const interface::Product& p_rProductData) 
    : 
    m_rProductData  ( p_rProductData ),
    m_oProductNb  ( 0 ),
    m_oSumErrorMutex(std::make_shared<Poco::FastMutex>())
{}

const std::map<std::string, ResultType> Product::sumErrorMap_ = createSumErrorMap();
const std::map<std::string, unsigned int> Product::paramListMap_ = createParamListMap();
const unsigned int Product::m_oEntrySize = 13;
Product::~Product()
{
    seClearAll();
}


SeamSeries*  Product::addSeamSeries(const MeasureTask& p_rMeasureTask) {
    const auto oSeamSeriesNumber  = p_rMeasureTask.seamseries();
    const auto oSeamSeries      = SeamSeries    ( oSeamSeriesNumber, p_rMeasureTask.hwParametersatzID() );
    auto& rSeamSeries       = m_oSeamSeries.insert(std::map<int, SeamSeries>::value_type(oSeamSeriesNumber, oSeamSeries)).first->second;
    
    return &rSeamSeries;
}

bool operator<(const Product& p_rFirst, const Product& p_rSecond) {
    return p_rFirst.m_rProductData < p_rSecond.m_rProductData;
}

// --------------- sum error common ------------------

std::string Product::upperString(std::string p_oStr) const
{
    std::string tmpStr = p_oStr;
    std::transform(tmpStr.begin(), tmpStr.end(), tmpStr.begin(), ::toupper);
    return tmpStr;
}

bool Product::setResultProxy(TResults<AbstractInterface> *p_pResultProxy) const
{
    if (p_pResultProxy == nullptr)
    {
        return false;
    }
    ResultProxy_ = p_pResultProxy;
    return true;
}



void Product::setSumErrorParam(const SpFilterParameter& p_rParam, SumErrorParams &r_oParams)  const
{
    // set sumErrorParams according to parameterlist p_rParam
    std::map<std::string, unsigned int>::const_iterator tmpIt = paramListMap_.find( upperString(p_rParam->name()) );
    if (tmpIt == paramListMap_.end())
    {
        return;
    } 
    switch (tmpIt->second)
    { // see InspectManager.h for mapping details and keep this function/switch consistent!

        case 0:
        {
            r_oParams.m_oErrorType = (ResultType)p_rParam->value<int>();
            break;
        }
        case 1:
        {
            std::string oStrScope = p_rParam->value<std::string>();
            r_oParams.m_oScope = SumError::scopeStringToEnum(oStrScope);
            break;
        }
        case 2:
        {
            r_oParams.m_oSeamseries = p_rParam->value<int>();
            break;
        }
        case 3:
        {
            r_oParams.m_oSeam = p_rParam->value<int>();
            break;
        }
        case 4:
        {
            r_oParams.m_oSeaminterval = p_rParam->value<int>();
            break;
        }
        case 5:
        {
            r_oParams.m_oThreshold = p_rParam->value<double>(); // (mm)
            break;
        }
        case 6:
        {
            r_oParams.m_oResultType = (ResultType)p_rParam->value<int>();
            break;
        }
        case 7:
        {
            r_oParams.m_oMin = p_rParam->value<double>();
            break;
        }
        case 8:
        {
            r_oParams.m_oMax = p_rParam->value<double>();
            break;
        }
        case 9:
        {
            std::string oStrUuid = p_rParam->value<std::string>();
            r_oParams.m_oReferenceSet = Poco::UUID(oStrUuid);
            break;
        }
        case 10:
        {
            r_oParams.m_oUseMiddleReference = p_rParam->value<bool>();
            break;
        }
        case 11:
        {
            r_oParams.m_oSecondThreshold = p_rParam->value<double>(); // (mm)
            break;
        }
        case 12:
        {
            r_oParams.m_oLwmSignalThreshold = p_rParam->value<double>();
            break;
        }
        default: break;
    }
}

void Product::createSumErrors(ParameterList &p_rParamList, const std::vector<ReferenceCurveSet>& productReferenceCurves)  const
{
    Poco::ScopedLock<Poco::FastMutex> lock(*(m_oSumErrorMutex.get()));
    if (p_rParamList.size() <= 0)
    {
        wmLog( eDebug, "Product::createSumErrors() no summerrors defined\n");
        return; // no sum errors at all
    }
    wmLog( eDebug, "Product::createSumErrors() start with p_rParamList.size() <%d>\n", p_rParamList.size());

    // check the sumErrorsParameterList and reduce the list if needed
    // list must contain 11 parameters (before 11/20) or 12 parameters (secondThreshold added)
    // search for the additional (not needed) parameters and remove them
    for(std::vector<std::shared_ptr<FilterParameter>>::iterator itTest = p_rParamList.begin() ; itTest != p_rParamList.end();)
    {
        std::string oName = (*itTest)->name();
        std::string oDeletePatternMinLimit ("MinLimit");
        std::string oDeletePatternMaxLimit ("MaxLimit");
        std::string oDeletePatternShift ("Shift");
        if ((oName.compare(oDeletePatternMinLimit) == 0) ||
            (oName.compare(oDeletePatternMaxLimit) == 0) ||
            (oName.compare(oDeletePatternShift) == 0))
        {

            itTest = p_rParamList.erase(itTest );
        }
        else
        {
            ++itTest;
        }
    }

    wmLog( eDebug, "Product::createSumErrors() adjusted p_rParamList.size() <%d> summerrors <%d>\n", p_rParamList.size(), (p_rParamList.size() / m_oEntrySize) );
    // empty or incomplete parameter set?
    if ( ((p_rParamList.size() / m_oEntrySize) <= 0) || ((p_rParamList.size() % m_oEntrySize) != 0) )
    {
        wmLog(eError, "Product::createSumErrors() -> Incomplete Sum Error list; check pushed entries! \n");
        return;
    }

    unsigned int oPos = 0;
    auto itTraverse = p_rParamList.begin();
    /// traverse all error, each covering m_oEntrySizes parameters
    while ((oPos+m_oEntrySize) <= p_rParamList.size())
    {
        itTraverse += ( (int)(oPos > 0) ) * m_oEntrySize; // address current/next error in list
        Poco::UUID oErrorGuid = (*itTraverse)->typID();
        Poco::UUID oErrorInstance = (*itTraverse)->instanceID();
#ifdef __debugSumErrors
        wmLog( eDebug, "Product::createSumErrors() oPos <%d>\n", oPos);
        wmLog( eDebug, "     oErrorGuid <%s> oErrorInstance <%s>\n",  oErrorGuid.toString().c_str(), oErrorInstance.toString().c_str() );
#endif
        // get kind of SumError from Instance GUID oErrorInstance
        std::string oErrorIdUpperCase = upperString(oErrorGuid.toString());
        std::map<std::string, ResultType>::const_iterator seIt = sumErrorMap_.find( oErrorIdUpperCase );
        if (seIt == sumErrorMap_.end())
        {
          // invalid guid, does not reference a SumError
          wmLog(eError, "Product::createSumErrors() ->  Invalid Sum Error GUID %s! \n", oErrorGuid.toString().c_str());
        }
        else
        {
            // set params by param guids!
            SumErrorParams oParams;
            for (unsigned int oNrParams = 0; oNrParams < m_oEntrySize; ++oNrParams)
            {
                setSumErrorParam(p_rParamList[oPos + oNrParams], oParams);
            }

            // error already exists? Yes, then change parameters and adjust result and error list to potential changes
            if (seExistsError(oErrorInstance))
            {
                // error exists, so change paremeters
                seDeleteResultTypeList(oErrorInstance);
                seAddResultType(oErrorInstance, oParams.m_oResultType);
                seSetErrorType(oErrorInstance, oParams.m_oErrorType);
                seOutlierChangeMinMax(oErrorInstance, oParams.m_oMin, oParams.m_oMax);
                seOutlierChangeMaxViolation(oErrorInstance, oParams.m_oThreshold);
                switch (seIt->second)
                {
                    case ResultType::SignalSumErrorAccumulatedOutlierReferenceBoundary:
                    case ResultType::SignalSumErrorSingleOutlierReferenceBoundary:
                    case ResultType::SignalSumErrorAccumulatedAreaReferenceBoundary:
                    case ResultType::SignalSumErrorSingleAreaReferenceBoundary:
                    case ResultType::SignalSumErrorInlierReferenceBoundary:
                    case ResultType::SignalSumErrorPeakReferenceBoundary:
                    case ResultType::SignalSumErrorDualOutlierInRangeReferenceBoundary:
                    {
                        const auto referenceSet = std::find_if(productReferenceCurves.begin(), productReferenceCurves.end(), [&oParams] (auto &curveSet) { return curveSet.m_uuid == oParams.m_oReferenceSet;});
                        if ( referenceSet == productReferenceCurves.end() )
                        {
                            wmLog( eError, "Product::createSumErrors() unable to obtain referenceCurveSet for the Error, thus skipping to next in pipeline\n");
                        }
                        else if (referenceSet->m_middle.m_curve.empty())
                        {
                            wmLog( eError, "Product::createSumErrors() unable to create, because referenceCurve does not contain values\n");
                        }
                        else
                        {
                            seOutlierChangeReference(oErrorInstance, (*referenceSet));
                        }
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
            else
            {
    #ifdef __debugSumErrors
                std::ostringstream  oDebugString;
                oDebugString << "Product::createSumErrors() Error <" << seIt->first << "> SumErrorType eg (SumErrorAccumulatedOutlierStaticBoundary) <" << (ResultType)seIt->second << ">" << std::endl;
                    //std::cout << oDebugString.str() << std::endl;
                wmLog( eDebug, oDebugString.str());
                oDebugString.str("");
                oDebugString << "  Parameter: Value to observe (eg Mismatch) <" << oParams.m_oResultType
                            << "> throw Error (eg Rank violation) <" << oParams.m_oErrorType
                            << "> Scope <" << oParams.m_oScope
                            << ">" << std::endl;
                            //std::cout << oDebugString.str() << std::endl;
                            wmLog( eDebug, oDebugString.str());
                            oDebugString.str("");
                oDebugString << "  Seamseries <" << oParams.m_oSeamseries
                            << "> Seam <" << oParams.m_oSeam
                            << "> Seaminterval <" << oParams.m_oSeaminterval
                            << "> MaxViolation <" << oParams.m_oThreshold
                            << "> Min <" << oParams.m_oMin
                            << "> Max <" << oParams.m_oMax
                            << ">" << std::endl;
                    //std::cout << oDebugString.str() << std::endl;
                    wmLog( eDebug, oDebugString.str());
    #endif
                // error does not exist, so create new one
                bool ok = false;
                switch(seIt->second)
                {
//---SIGNAL-SUM-ERRORS----
                    case ResultType::SignalSumErrorAccumulatedOutlierStaticBoundary:
                    {
                        ok = addSignalSumErrorAcccumulatedOutlierStaticBoundary(seIt->second, oErrorInstance, oParams, false, false);
                        break;
                    }
                    case ResultType::SignalSumErrorSingleOutlierStaticBoundary:
                    {
                        ok = addSignalSumErrorSingleOutlierStaticBoundary(seIt->second, oErrorInstance, oParams, false);
                        break;
                    }
                    case ResultType::SignalSumErrorAccumulatedAreaStaticBoundary:
                    {
                        ok = addSignalSumErrorAcccumulatedOutlierStaticBoundary(seIt->second, oErrorInstance, oParams, true, false);
                        break;
                    }
                    case ResultType::SignalSumErrorSingleAreaStaticBoundary:
                    {
                        ok = addSignalSumErrorSingleOutlierStaticBoundary(seIt->second, oErrorInstance, oParams, true);
                        break;
                    }
                    case ResultType::SignalSumErrorInlierStaticBoundary:
                    {
                        ok = addSignalSumErrorSingleInlierStaticBoundary(seIt->second, oErrorInstance, oParams);
                        break;
                    }                   
                    case ResultType::SignalSumErrorPeakStaticBoundary:
                    {
                        ok = addSignalSumErrorAcccumulatedOutlierStaticBoundary(seIt->second, oErrorInstance, oParams, false, true);
                        break;
                    }
                    case ResultType::SignalSumErrorDualOutlierInRangeStaticBoundary:
                    {
                        ok = addSignalSumErrorDualOutlierInRangeStaticBoundary(seIt->second, oErrorInstance, oParams);
                        break;
                    }
                    
                    case ResultType::SignalSumErrorAccumulatedOutlierReferenceBoundary:
                    case ResultType::SignalSumErrorSingleOutlierReferenceBoundary:
                    case ResultType::SignalSumErrorAccumulatedAreaReferenceBoundary:
                    case ResultType::SignalSumErrorSingleAreaReferenceBoundary:
                    case ResultType::SignalSumErrorInlierReferenceBoundary:
                    case ResultType::SignalSumErrorPeakReferenceBoundary:
                    case ResultType::SignalSumErrorDualOutlierInRangeReferenceBoundary:
                    {
                        const auto referenceSet = std::find_if(productReferenceCurves.begin(), productReferenceCurves.end(), [&oParams] (auto &curveSet) { return curveSet.m_uuid == oParams.m_oReferenceSet;});
                        if ( referenceSet == productReferenceCurves.end() )
                        {
                            wmLog( eError, "Product::createSumErrors() unable to obtain referenceCurveSet for the Error, thus skipping to next in pipeline\n");
                            ok = false;
                        } 
                        else if (referenceSet->m_middle.m_curve.empty())
                        {
                            wmLog( eError, "Product::createSumErrors() unable to create, because referenceCurve does not contain values\n");
                            ok = false;
                        }
                        else
                        {  
                            ok = addReferenceSignalError(seIt->second, oErrorInstance, oParams, (*referenceSet) );
                        }
                        break;
                    }                 

//---ANALYSIS-SUM-ERRORS-----
                    case ResultType::AnalysisSumErrorAccumulatedOutlier:
                    {
                        ok = addAnalysisSumErrorAccumulatedOutlier(seIt->second, oErrorInstance, oParams);
                        break;
                    }
                    case ResultType::AnalysisSumErrorAdjacentOutlier:
                    {
                        ok = addAnalysisSumErrorAdjacentOutlier(seIt->second, oErrorInstance, oParams);
                        break;
                    }

                
//---LEVEL-TWO-ERRORS-----
                    case ResultType::LevelTwoErrorAccumulated:
                    {
                        // set last parameter to true if we count NIO seams respect of the sumError value
                        ok = addLevelTwoErrorAccumulated(seIt->second, oErrorInstance, oParams, true);
                        break;
                    }
                    case ResultType::LevelTwoErrorAdjacent:
                    {
                        // set last parameter to true if we count NIO seams regardless of the sumError value
                        ok = addLevelTwoErrorAdjacent(seIt->second, oErrorInstance, oParams, true);
                        break;
                    }
                    case ResultType::LevelTwoErrorErrorOnlyAccumulated:
                    {
                        // set last parameter to false if we count NIO seams regardless of the sumError value
                        ok = addLevelTwoErrorAccumulated(seIt->second, oErrorInstance, oParams, false);
                        break;
                    }
                    case ResultType::LevelTwoErrorErrorOnlyAdjacent:
                    {
                        // set last parameter to false if we count NIO seams regardless of the sumError value
                        ok = addLevelTwoErrorAdjacent(seIt->second, oErrorInstance, oParams, false);
                        break;
                    }

                    default:
                    {
                        wmLog(eError, "Product::createSumErrors() switch Invalid Sum Error (= ResultType) %d! \n", seIt->second);
                        break;
                    }
                } // switch
                if (ok)
                {
                    seAddResultType(oErrorInstance, oParams.m_oResultType);
                    seSetErrorType(oErrorInstance, oParams.m_oErrorType);
                    #ifdef __debugSumErrors
                        std::ostringstream  oDebugString;
                        oDebugString << "Product::createSumErrors() created SumErrorType:[" << (ResultType)seIt->second << "], ResultType [" << oParams.m_oResultType << "], ErrorType [" << oParams.m_oErrorType << "]";
                        oDebugString << " at scope [" << oParams.m_oSeamseries << "|" <<  oParams.m_oSeam << "|" << oParams.m_oSeaminterval << "]" << std::endl;
                        wmLog(eDebug, oDebugString.str());
                    #endif
                }
            } // else (error exists)
        } // if valid GUID
        oPos += m_oEntrySize;
    } // while loop (process all errors in list)
}



bool Product::addReferenceSignalError(const int sumErrType, const Poco::UUID errorInstance, const SumErrorParams &params, const interface::ReferenceCurveSet &referenceSet) const
{
    switch (sumErrType)
    {
        case ResultType::SignalSumErrorAccumulatedOutlierReferenceBoundary:
        {
            return addSignalSumErrorAccumulatedOutlierReferenceBoundary(sumErrType, errorInstance, params, referenceSet, false, false);
        }
        case ResultType::SignalSumErrorSingleOutlierReferenceBoundary:
        {
            return addSignalSumErrorSingleOutlierReferenceBoundary(sumErrType, errorInstance, params, referenceSet, false);
        }
        case ResultType::SignalSumErrorAccumulatedAreaReferenceBoundary:
        {
            return addSignalSumErrorAccumulatedOutlierReferenceBoundary(sumErrType, errorInstance, params, referenceSet, true, false);
        }
        case ResultType::SignalSumErrorSingleAreaReferenceBoundary:
        {
            return addSignalSumErrorSingleOutlierReferenceBoundary(sumErrType, errorInstance, params, referenceSet, true);
        }
        case ResultType::SignalSumErrorInlierReferenceBoundary:
        {
            return addSignalSumErrorInlierReferenceBoundary(sumErrType, errorInstance, params, referenceSet);
        }
        case ResultType::SignalSumErrorPeakReferenceBoundary:
        {
            return addSignalSumErrorAccumulatedOutlierReferenceBoundary(sumErrType, errorInstance, params, referenceSet, false, true);
        }
        case ResultType::SignalSumErrorDualOutlierInRangeReferenceBoundary:
        {
            return addSignalSumErrorDualOutlierInRangeReferenceBoundary(sumErrType, errorInstance, params, referenceSet);
        }
        
        default:
        {
            return false;
        }
    }
}




// Handle result, (seamseries, seam, seaminterval) given indirectly by taskContext.measureTask of parameter p_rRes
// returs the list with the found sumerrors
ResultEvaluationList Product::seHandleResult(tSumErrorList &p_rErrors, const ResultArgs &p_rRes, const ResultArgs &lwmTriggerSignal) const
{
    Poco::ScopedLock<Poco::FastMutex> lock(*(m_oSumErrorMutex.get()));
    // get errors matching given scope
    p_rErrors.clear();
    ResultEvaluationList resultList(0);
    MeasureTask const &oMt = *(p_rRes.taskContext().measureTask());
#ifdef __debugSumErrors
    wmLog(eDebug, "Product::seHandleResult() start - resultType:[%d] nioType:[%d] isNio:[%d] isValid:[%d]\n", p_rRes.resultType(), p_rRes.nioType(), p_rRes.isNio(), p_rRes.isValid());
    wmLog(eDebug, "Product::seHandleResult() start - Image:[%d], Position:[%d] - Scope: [%d|%d|%d]\n", p_rRes.context().imageNumber(), p_rRes.context().position(), oMt.seamseries(), oMt.seam(), oMt.seaminterval() );
#endif

    tSeamIndex oCurrentInspectKey;
    // go through all levels to check
    for (unsigned int oLevel = eScopeProduct; oLevel <= eScopeSeaminterval; ++oLevel)
    {
        // fill up the <tuple>..
        switch(oLevel)
        {
            case eScopeProduct:
                oCurrentInspectKey = tSeamIndex(-1, -1, -1);
                break;
            case eScopeSeamseries:
                oCurrentInspectKey = tSeamIndex(oMt.seamseries(), -1, -1);
                break;
            case eScopeSeam:
                oCurrentInspectKey = tSeamIndex(oMt.seamseries(), oMt.seam(), -1);
                break;
            case eScopeSeaminterval:
                oCurrentInspectKey = tSeamIndex(oMt.seamseries(), oMt.seam(), oMt.seaminterval());
                break;
            default:
                continue;
        }

        tItSumErrors itErr = m_oErrorList.find(oCurrentInspectKey);
        // are there any global errors defined on the given triplet?
        if ( itErr != m_oErrorList.end() )
        {
            #ifdef __debugSumErrors
                wmLog(eDebug, "Product::seHandleResult() Error(s) defined for scope [%d] - [%d|%d|%d] \n", oLevel, std::get<0>(oCurrentInspectKey), std::get<1>(oCurrentInspectKey), std::get<2>(oCurrentInspectKey));
            #endif
            // oErrors is the list of the defined errors for a triple (eg Seamserie 1, seam 2, seamInterval 2)
            tListSumErrors &oErrors = itErr->second;
            for (auto &oMyError : oErrors)
            {
                // polymorphismus..
                ResultEvaluationData sumErrorResult = oMyError->testResult(p_rRes, lwmTriggerSignal);
                
                if ( sumErrorResult.evaluationStatus >= eSumErrorNotTriggered )
                {
                    resultList.push_back(sumErrorResult);
                    oMyError->keepResultAsLastResult(p_rRes);
                    p_rErrors.push_back( oMyError ); // add error to list to push its calculated nioPercentage into the results
                    if ( sumErrorResult.evaluationStatus == eSumErrorTriggered )
                    {
                        wmLog( eDebug, "Product::seHandleResult() sumError:[%d] triggered to throw error:[%d] of resultType:[%d] (original nioType:[%d])\n", 
                                oMyError->sumErrorType(), oMyError->errorType(), p_rRes.resultType(), p_rRes.nioType());
                    }
                    oMyError->trackScopeOnProduct(p_rRes);
                }
            }
        }
    }

    // check the Level 2 sumErrors
    //-------------------------------------------------------------------------
    if (p_rErrors.size() > 0)
    {
        tSumErrorList oListToAdd{};
        // sum errors triggered; check if a level2 SumError is defined
         for (const SmpSumError &oTriggeredError : p_rErrors)
        {
            #ifdef __debugSumErrors
                wmLog(eDebug, "Product::seHandleResult() Level2Error check: sumError <%d> throwing error <%d> \n ", oTriggeredError->sumErrorType(), oTriggeredError->errorType());
            #endif
            // LevelTwoErrors are on product or seamseries, check both levels
            for (unsigned int oLevel = eScopeProduct; oLevel <= eScopeSeamseries; ++oLevel)
            {
                // fill up the <tuple>..
                switch (oLevel)
                {
                    case eScopeProduct:
                        oCurrentInspectKey = tSeamIndex(-1, -1, -1);
                        break;
                    case eScopeSeamseries:
                        oCurrentInspectKey = tSeamIndex(oMt.seamseries(), -1, -1);
                        break;
                    default:
                        continue;
                }
                tItSumErrors itErr = m_oErrorList.find(oCurrentInspectKey);
                // are there any sum errors defined on the given triplet?
                if (itErr != m_oErrorList.end())
                {
                    // use for adjacent error and others
                    tSeamIndex oCurrentActiveIndex = tSeamIndex(oMt.seamseries(), oMt.seam(), oMt.seaminterval());
                    tListSumErrors &oErrors = itErr->second;
                    
                    for (SmpSumError &oTestError : oErrors) //go through list of all Errors
                    {
                        ResultEvaluation oResult = oTestError->testSumErrorLevel2(oTriggeredError, oCurrentActiveIndex);
                        if (oResult == eSumErrorTriggered)
                        {
                            ResultEvaluationData levelTwoResult{oResult, {}, std::vector<geo2d::DPoint>{geo2d::DPoint(0.0, 100.0)}, {}, {}};
                            resultList.push_back(levelTwoResult);
                            oListToAdd.push_back(oTestError); // add error to list
                            wmLog(eDebug, "Product::seHandleResult() push_back'd sumErrorLevel2 <%d> throw error <%d> of resultType <%d> (original nioType <%d>)\n ", oTestError->sumErrorType(), oTestError->errorType(), p_rRes.resultType(), p_rRes.nioType());
                        }
                    }
                }
            } // for  go through all levels to check
         }
        p_rErrors.insert(p_rErrors.end(), oListToAdd.begin(), oListToAdd.end());
    }    
    return resultList;
}

bool Product::seExistsError(Poco::UUID p_oGuid) const
{
  auto oIt = m_oGuidToError.find(p_oGuid);
  if ( oIt != m_oGuidToError.end() )
  {
    wmLog( eError, "Product::seExistsError()  <%s>\n", p_oGuid.toString().c_str());
    return true; // guid already points to an error!
  }
  return false; // guid free to use!
}

SmpSumError Product::seGetError(Poco::UUID p_oGuid) const
{
  auto oIt = m_oGuidToError.find(p_oGuid);
  if ( oIt != m_oGuidToError.end() )
  {
    tSeamErrorIndex oIdx = oIt->second;
    return (*oIdx.theIt);
  }
  return nullptr;
}

bool Product::seDelete(Poco::UUID p_oGuid) const
{
  Poco::ScopedLock<Poco::FastMutex> lock(*(m_oSumErrorMutex.get()));
  auto oIt = m_oGuidToError.find(p_oGuid);
  bool allOK = true;
  if ( oIt != m_oGuidToError.end() )
  {
    tSeamErrorIndex oIdx = oIt->second;                           // get struct if index triplet and iterator
    tItSumErrors oItSE = m_oErrorList.find(oIdx.idx);             // find associate list to index triplet
    if (oItSE != m_oErrorList.end() )
    {
      oItSE->second.erase(oIdx.theIt); // erase element from list
    } else
    {
      wmLog(eError, "ERROR [Product::seDelete]  ->  Cannot find SumError referenced by GUID %s.", p_oGuid.toString().c_str());
      allOK = false;
    }
    m_oGuidToError.erase(oIt);                                    // erase complete entry associated with GUID
  }
  else
  {
    wmLog(eError, "ERROR [Product::seDelete]  ->  Invalid GUID %s for SumError.", p_oGuid.toString().c_str());
    allOK = false;
  }
  return allOK;
}

void Product::seClearAll() const
{
  try
  {
    Poco::ScopedLock<Poco::FastMutex> lock(*(m_oSumErrorMutex.get()));
    // delete SumErrors themselves
    for (auto oIt = m_oErrorList.begin(); oIt != m_oErrorList.end(); ++oIt)
    {
      oIt->second.clear();
    }

    // detele guid -> SumError references
    tMapIdGlobalError::iterator oItGuids = m_oGuidToError.begin();
    while(oItGuids != m_oGuidToError.end())
    {
      oItGuids = m_oGuidToError.erase(oItGuids);
    }

  }
  catch(...)
  {
      wmLog(eError, "Product::seClearAll() m_oProductNb <%d>\n",m_oProductNb);
  }
}

void Product::seDeleteResultTypeList(Poco::UUID p_oGuid) const
{
  SmpSumError pError = seGetError(p_oGuid);
  if (pError.isNull())
  {
    return;
  }
  pError->clearResultTypeList();
}

void Product::seSetErrorType(Poco::UUID p_oGuid, std::int32_t p_oType) const
{
  SmpSumError pError = seGetError(p_oGuid);
  if (pError.isNull())
  {
    return;
  }
  pError->setErrorType(p_oType);
}

bool Product::seAddResultType(Poco::UUID p_oGuid, ResultType p_oType) const
{
  SmpSumError pError = seGetError(p_oGuid);
  if (pError.isNull())
  {
        return false;
  }
  pError->addResultToSurveil(p_oType);
#ifdef __debugSumErrors
    wmLog( eDebug, "Product::seAddResultType() <%s> Value to observe (p_oType) <%d>\n", p_oGuid.toString().c_str(), p_oType);
#endif
    return true;
}

/// new cycle -> reset errors!
void Product::seResetAll() const
{
    Poco::ScopedLock<Poco::FastMutex> lock(*(m_oSumErrorMutex.get()));
    for (auto oIt = m_oErrorList.begin(); oIt != m_oErrorList.end(); ++oIt)
    {
        tListSumErrors smpErrorList = oIt->second;
        for (auto lIt = smpErrorList.begin(); lIt != smpErrorList.end(); ++lIt)
        {
            (*lIt)->reset();
        }
    }
}

void Product::seArm(const fliplib::ArmStateBase& state, const interface::tSeamIndex index) const
{
    interface::ResultArgs* pResForSendError = nullptr;
    tSumErrorList oErrorList;
    tSeamIndex oCurrentInspectKey;

    #ifdef __debugSumErrors
        wmLog(eDebug, "Product::seArm() start for index <%d> <%d> <%d> state <%d>\n", std::get<0>(index), std::get<1>(index), std::get<2>(index), state.getStateID());
    #endif

    Poco::ScopedLock<Poco::FastMutex> lock(*(m_oSumErrorMutex.get()));
    tItSumErrors itErr = m_oErrorList.find( index );

    if ( itErr != m_oErrorList.end() )
    {
        tListSumErrors oErrors = itErr->second;
        for (auto &oIt : oErrors)
        {
            interface::ResultArgs* pRes = oIt->arm( state );
            if ( pRes != nullptr )
            {
                wmLog( eDebug, "Product::seArm() isNio! sended data: ImgNum <%d> Pos <%d> resultType <%d> nioType <%d> \n",
                        pRes->context().imageNumber(), pRes->context().position(), pRes->resultType(), pRes->nioType());
                // here we only need to send it as a nio, as the result() call was already made earlier ...
                ResultProxy_->nio( (interface::ResultDoubleArray)*pRes );
                oErrorList.push_back(oIt);
                pResForSendError = pRes;
            }
        }
    }
    // check the Level 2 Errors
    if (oErrorList.size() > 0)
    {
        #ifdef __debugSumErrors
            wmLog(eDebug, "Product::seArm() oErrorList.size() > 0, start to check the level 2 errors\n");
        #endif
        // sum errors triggered; check if a level2 SumError is defined
        for (SmpSumError &oTriggeredError : oErrorList)
        {
            #ifdef __debugSumErrors
                wmLog(eDebug, "Product::seArm() triggered sumError <%d> throwing error <%d> \n ", oTriggeredError->sumErrorType(), oTriggeredError->errorType());
            #endif
            for (unsigned int oLevel = eScopeProduct; oLevel <= eScopeSeamseries; ++oLevel)
            {
                // fill up the <tuple>..
                switch (oLevel)
                {
                case eScopeProduct:
                    oCurrentInspectKey = tSeamIndex(-1, -1, -1);
                    break;
                case eScopeSeamseries:
                    oCurrentInspectKey = tSeamIndex(std::get<0>(index), -1, -1);
                    break;
                default:
                    continue;
                }
                #ifdef __debugSumErrors
                    wmLog(eDebug, "Product::arm() sumErrLevel2 key <%d> <%d> <%d>\n", std::get<0>(oCurrentInspectKey), std::get<1>(oCurrentInspectKey), std::get<2>(oCurrentInspectKey));
                #endif
                tItSumErrors itErr = m_oErrorList.find(oCurrentInspectKey);
                if (itErr != m_oErrorList.end())
                {
                    // use for adjacent error and others
                    tSeamIndex oCurrentActiveIndex = tSeamIndex(std::get<0>(index), std::get<1>(index), std::get<2>(index));
                    #ifdef __debugSumErrors
                        wmLog(eDebug, "Product::arm()  sumErrLevel2 start evaluation oCurrentActiveIndex <%d> <%d> <%d>\n", std::get<0>(oCurrentActiveIndex), std::get<1>(oCurrentActiveIndex), std::get<2>(oCurrentActiveIndex));
                    #endif
                    // oErrors is the list with all sumerrors on the product or seamseries-level of the incoming Error
                    tListSumErrors oErrors = itErr->second;
                    for (SmpSumError &oTestError : oErrors)
                    {
                        ResultEvaluation oResult = oTestError->testSumErrorLevel2(oTriggeredError, oCurrentActiveIndex);
                        if (oResult == eSumErrorTriggered)
                        {
                            #ifdef __debugSumErrors
                                wmLog(eDebug, "Product::arm() push_back'd sumErrorLevel2 <%d> throw error <%d> \n ", oTestError->sumErrorType(), oTestError->errorType());
                            #endif
                            if (pResForSendError != nullptr)
                            {
                                pResForSendError->setResultType(oTestError->errorType());
                                pResForSendError->setNioType(oTestError->errorType());
                                wmLog(eDebug, "Product::arm() isNio! sended data: ImgNum <%d> Pos <%d> resultType <%d> nioType <%d> \n",
                                    pResForSendError->context().imageNumber(), pResForSendError->context().position(), pResForSendError->resultType(), pResForSendError->nioType());
                                // here we only need to send it as a nio, as the result() call was already made earlier ...
                                ResultProxy_->nio((interface::ResultDoubleArray)*pResForSendError); // funny side-note: in most parts of the result system everything is kept very general, but in reality we only support doublearray-results, as the pipes of the result filters have this type and the result handler expects the incoming results to be double arrays ...
                            }
                        }
                    }
                }
            } // for  go through all levels to check
        }
    }
#ifdef __debugSumErrors
    wmLog(eDebug, "Product::seArm() ends for index <%d> <%d> <%d> state <%d>\n", std::get<0>(index), std::get<1>(index), std::get<2>(index), state.getStateID());
#endif
}

bool Product::seFinishAdd(const std::int32_t p_oSumErrType, Poco::UUID p_oGuid, ErrorScopeType p_oScope,
    std::int32_t p_oSeamseries, std::int32_t p_oSeam, std::int32_t p_oSeaminterval,
    SmpSumError &p_rError) const
{
    /*
    * The product holds a list of sum errors for each triplet (seamseries, seam, seaminterval) that has at least
    * one sum error defined (map m_oErrorList).
    * The specific error within this list is referenced by a guid as a key and an iterator as value (which is
    * the reason for the errors themselves are kept within a list and not a vector: lists by specification
    * guarantee that iterator remain unchanged no matter whether other entries are deleted, added ec.)
    */
        if (seExistsError(p_oGuid))
    {
        #ifdef __debugSumErrors
            wmLog( eDebug, "Product::seFinishAdd() guid already in use! <%s> return false\n", p_oGuid.toString().c_str());
        #endif
        return false;
    }
    tSeamIndex oCurrentInspectKey;
    switch(p_oScope)
    {
        case ErrorScopeType::eScopeProduct:
            oCurrentInspectKey = tSeamIndex(-1, -1, -1);
            break;
        case ErrorScopeType::eScopeSeamseries:
            oCurrentInspectKey = tSeamIndex(p_oSeamseries, -1, -1);
            break;
        case ErrorScopeType::eScopeSeam:
            oCurrentInspectKey = tSeamIndex(p_oSeamseries, p_oSeam, -1);
            break;
        case ErrorScopeType::eScopeSeaminterval:
            oCurrentInspectKey = tSeamIndex(p_oSeamseries, p_oSeam, p_oSeaminterval);
            break;
        case ErrorScopeType::eScopeNone:
        default:
            return false;
    }
    tItSumErrors itErr = m_oErrorList.find(oCurrentInspectKey);
    // there is not yet a list for the triplet, so create one!
    if (itErr == m_oErrorList.end())
    {
        tListSumErrors pNewErrList;
        itErr = m_oErrorList.insert(m_oErrorList.end(), std::pair<tSeamIndex, tListSumErrors>(oCurrentInspectKey, pNewErrList));
        #ifdef __debugSumErrors
            wmLog( eDebug, "Product::seFinishAdd() create new map of tuples insert error size of List m_oErrorList <%d>\n", m_oErrorList.size());
        #endif
    }
    tListSumErrors *pErrorList = &itErr->second;

    tItListSumErrors oInsertionIndex = pErrorList->insert(pErrorList->end(), p_rError);
    tSeamErrorIndex oErrorRef(oCurrentInspectKey, oInsertionIndex);
    //TODO: map key already used or guid zero -> ERROR!
    p_rError->setGuid(p_oGuid);
    p_rError->setSumErrorType(p_oSumErrType);

    m_oGuidToError[p_oGuid] = oErrorRef; // add pair (triplet, errorIt) to map

    #ifdef __debugSumErrors
    wmLog(eDebug, "Product::seFinishAdd() for <%d> ends; defined <%d> <%d> <%d> scope <%d>\n",
        p_rError->sumErrorType(), std::get<0>(oCurrentInspectKey), std::get<1>(oCurrentInspectKey), std::get<2>(oCurrentInspectKey), p_oScope);
    #endif
    return true;
}

// ------------ SumError Outlier unspecific -------------

bool Product::seOutlierChangeMaxViolation(Poco::UUID p_oGuid, double p_oNewMaxViolation) const
{
    SmpSumError pError = seGetError(p_oGuid);
    if (pError.isNull())
    {
        return false;
    }
    SignalSumError *oError = (SignalSumError*)(&(*pError));
    oError->setMaxViolation(p_oNewMaxViolation);
    return true;
}

bool Product::seOutlierChangeMinMax(Poco::UUID p_oGuid, double p_oNewMin, double p_oNewMax) const
{
    SmpSumError pError = seGetError(p_oGuid);
    if (pError.isNull())
    {
        return false;
    }
    SignalSumError *oError = (SignalSumError*)(&(*pError));
    oError->changeBounds(p_oNewMin, p_oNewMax);
    return true;
}

bool Product::seOutlierChangeReference(Poco::UUID p_oGuid, const interface::ReferenceCurveSet &referenceSet) const
{
    SmpSumError pError = seGetError(p_oGuid);
    if ( pError.isNull() )
    {
        return false;
    }
    SignalSumError *oError = (SignalSumError*)(&(*pError));
    oError->changeReference( referenceSet.m_upper.m_curve, referenceSet.m_middle.m_curve, referenceSet.m_lower.m_curve );
    return true;
}

// ----------- SumError OutlierAll -----------------

bool Product::addSignalSumErrorAcccumulatedOutlierStaticBoundary(const int p_oSumErrType, const Poco::UUID p_oGuid, const SumErrorParams &params, const bool isAreaError, const bool isPeakError) const
{
    #ifdef __debugSumErrors
        wmLog(eDebug, "Product::addSignalSumErrorAcccumulatedOutlierStaticBoundary() <%d>\n", p_oSumErrType);
    #endif
    SmpSumError oError = SmpSumError(new SignalSumErrorAccumulatedOutlier(params.m_oMax, params.m_oMin, params.m_oThreshold, params.m_oLwmSignalThreshold, isAreaError, isPeakError) );
    if ( oError->setScope(params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval) )
    {
        return seFinishAdd(p_oSumErrType, p_oGuid, params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval, oError);
    }
    return false;
}

bool Product::addSignalSumErrorAccumulatedOutlierReferenceBoundary(const int p_oSumErrType, const Poco::UUID p_oGuid, const SumErrorParams &params, const interface::ReferenceCurveSet &referenceSet, const bool isAreaError, const bool isPeakError) const
{
    #ifdef __debugEnvelopeError
        wmLog( eDebug, "Product::addSignalSumErrorAccumulatedOutlierReferenceBoundary() <%d>\n", p_oSumErrType);
    #endif
    SmpSumError oError = SmpSumError(new SignalSumErrorAccumulatedOutlier( referenceSet.m_upper.m_curve, referenceSet.m_middle.m_curve, referenceSet.m_lower.m_curve, params.m_oMax, params.m_oMin, params.m_oThreshold, params.m_oLwmSignalThreshold, isAreaError, isPeakError, params.m_oUseMiddleReference) );
    if ( oError->setScope(params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval) )
    {
        return seFinishAdd(p_oSumErrType, p_oGuid, params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval, oError);
    }
    #ifdef __debugEnvelopeError
        wmLog( eDebug, "Product::addSignalSumErrorAccumulatedOutlierReferenceBoundary() returns false\n");
    #endif
    return false;
}

bool Product::addSignalSumErrorSingleOutlierStaticBoundary(const int p_oSumErrType, const Poco::UUID p_oGuid, const SumErrorParams &params, const bool isAreaError) const
{
    #ifdef __debugSumErrors
        wmLog(eDebug, "Product::addSignalSumErrorSingleOutlierStaticBoundary() <%d>\n", p_oSumErrType);
    #endif
    SmpSumError oError = SmpSumError(new SignalSumErrorSingleOutlier(params.m_oMax, params.m_oMin, params.m_oThreshold, params.m_oLwmSignalThreshold, isAreaError) );
    if ( oError->setScope(params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval) )
    {
        return seFinishAdd(p_oSumErrType, p_oGuid, params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval, oError);
    }
    return false;
}

bool Product::addSignalSumErrorSingleOutlierReferenceBoundary(const int p_oSumErrType, const Poco::UUID p_oGuid, const SumErrorParams &params, const interface::ReferenceCurveSet &referenceSet, const bool isAreaError) const
{
    #ifdef __debugEnvelopeError
        wmLog( eDebug, "Product::addSignalSumErrorSingleOutlierReferenceBoundary() <%d>\n", p_oSumErrType);
    #endif
    SmpSumError oError = SmpSumError(new SignalSumErrorSingleOutlier(referenceSet.m_upper.m_curve, referenceSet.m_middle.m_curve, referenceSet.m_lower.m_curve, params.m_oMax, params.m_oMin, params.m_oThreshold, params.m_oLwmSignalThreshold, isAreaError, params.m_oUseMiddleReference) );
    if ( oError->setScope(params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval) )
    {
        return seFinishAdd(p_oSumErrType, p_oGuid, params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval, oError);
    }
    #ifdef __debugEnvelopeError
        wmLog( eDebug, "Product::addSignalSumErrorSingleOutlierReferenceBoundary() returns false\n");
    #endif
    return false;
}

bool Product::addSignalSumErrorSingleInlierStaticBoundary(const int p_oSumErrType, const Poco::UUID p_oGuid, const SumErrorParams &params) const
{
    #ifdef __debugSumInvertError
        wmLog(eDebug, "Product::addSignalSumErrorSingleInlierStaticBoundary() <%d>\n", p_oSumErrType);
    #endif
    SmpSumError oError = SmpSumError(new SignalSumErrorSingleInlier(params.m_oMax, params.m_oMin, params.m_oThreshold, params.m_oLwmSignalThreshold));

    if (oError->setScope(params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval))
    {
        return seFinishAdd(p_oSumErrType, p_oGuid, params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval, oError);
    }
    #ifdef __debugSumInvertError
        wmLog(eDebug, "Product::addSignalSumErrorSingleInlierStaticBoundary() returns false\n");
    #endif
    return false;
}

bool Product::addSignalSumErrorInlierReferenceBoundary(const int p_oSumErrType, const Poco::UUID p_oGuid, const SumErrorParams &params, const interface::ReferenceCurveSet &referenceSet) const
{
    #ifdef __debugSumInvertError
        wmLog(eDebug, "Product::addSignalSumErrorInlierReferenceBoundary() <%d>\n", p_oSumErrType);
    #endif
    SmpSumError oError = SmpSumError(new SignalSumErrorSingleInlier(referenceSet.m_upper.m_curve, referenceSet.m_middle.m_curve, referenceSet.m_lower.m_curve, params.m_oMax, params.m_oMin, params.m_oThreshold, params.m_oUseMiddleReference, params.m_oLwmSignalThreshold) );

    if (oError->setScope(params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval))
    {
        return seFinishAdd(p_oSumErrType, p_oGuid, params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval, oError);
    }
    #ifdef __debugSumInvertError
        wmLog(eDebug, "Product::addSignalSumErrorInlierReferenceBoundary() returns false\n");
    #endif
    return false;
}


bool Product::addSignalSumErrorDualOutlierInRangeStaticBoundary(const int p_oSumErrType, const Poco::UUID p_oGuid, const SumErrorParams &params) const
{
    #ifdef __debugSumDualError
        wmLog(eDebug, "Product::addSignalSumErrorDualOutlierInRangeStaticBoundary() <%d>\n", p_oSumErrType);
    #endif
    SmpSumError oError = SmpSumError(new SignalSumErrorDualOutlierInRange(params.m_oMax, params.m_oMin, params.m_oThreshold, params.m_oSecondThreshold, params.m_oLwmSignalThreshold) );

    if (oError->setScope(params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval))
    {
        return seFinishAdd(p_oSumErrType, p_oGuid, params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval, oError);
    }
    #ifdef __debugSumDualError
        wmLog(eDebug, "Product::addSignalSumErrorDualOutlierInRangeStaticBoundary() returns false\n");
    #endif
    
    return false;
}


bool Product::addSignalSumErrorDualOutlierInRangeReferenceBoundary(const int p_oSumErrType, const Poco::UUID p_oGuid, const SumErrorParams &params, const interface::ReferenceCurveSet &referenceSet) const
{
    #ifdef __debugSumDualError
        wmLog(eDebug, "Product::addSignalSumErrorDualOutlierInRangeReferenceBoundary() <%d>\n", p_oSumErrType);
    #endif
    SmpSumError oError = SmpSumError(new SignalSumErrorDualOutlierInRange(referenceSet.m_upper.m_curve, referenceSet.m_middle.m_curve, referenceSet.m_lower.m_curve, params.m_oMax, params.m_oMin, params.m_oThreshold, params.m_oSecondThreshold, params.m_oLwmSignalThreshold, params.m_oUseMiddleReference) );

    if (oError->setScope(params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval))
    {
        return seFinishAdd(p_oSumErrType, p_oGuid, params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval, oError);
    }
    #ifdef __debugSumDualError
        wmLog(eDebug, "Product::addSignalSumErrorDualOutlierInRangeReferenceBoundary() returns false\n");
    #endif
    
    return false;
}





// ----------- Analysis Errors --------------
bool Product::addAnalysisSumErrorAccumulatedOutlier(const int p_oSumErrType, const Poco::UUID p_oGuid, const SumErrorParams &params) const
{
    #ifdef __debugSumErrors
        wmLog(eDebug, "Product::addAnalysisSumErrorAccumulatedOutlier() <%d>\n", p_oSumErrType);
    #endif
    SmpSumError oError = SmpSumError(new AnalysisSumErrorAccumulatedOutlier(params.m_oThreshold) );
    if ( oError->setScope(params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval) )
    {
        return seFinishAdd(p_oSumErrType, p_oGuid, params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval, oError);
    }
    return false;
}

bool Product::addAnalysisSumErrorAdjacentOutlier(const int p_oSumErrType, const Poco::UUID p_oGuid, const SumErrorParams &params) const
{
    #ifdef __debugSumErrors
        wmLog(eDebug, "Product::addAnalysisSumErrorAdjacentOutlier() <%d>\n", p_oSumErrType);
    #endif
    SmpSumError oError = SmpSumError(new AnalysisSumErrorSingleOutlier(params.m_oThreshold) );
    if ( oError->setScope(params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval) )
    {
        return seFinishAdd(p_oSumErrType, p_oGuid, params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval, oError);
    }
    return false;
}



// ----------- Level Two Error (Product/SeamSeries) --------------

bool Product::addLevelTwoErrorAccumulated(const int p_oSumErrType, const Poco::UUID p_oGuid, const SumErrorParams &params, const bool p_oCheckSpecificError) const
{
    #ifdef __debugSumErrorLevel2
        wmLog(eDebug, "Product::addLevelTwoErrorAccumulated() <%d>\n", p_oSumErrType);
    #endif
    SmpSumError oError = SmpSumError(new LevelTwoErrorAccumulated((int) params.m_oThreshold, p_oCheckSpecificError));

    if (oError->setScope(params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval))
    {
        return seFinishAdd(p_oSumErrType, p_oGuid, params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval, oError);
    }
    #ifdef __debugSumErrorLevel2
        wmLog(eDebug, "Product::addLevelTwoErrorAccumulated() returns false\n");
    #endif
    return false;
}

bool Product::addLevelTwoErrorAdjacent(const int p_oSumErrType, const Poco::UUID p_oGuid, const SumErrorParams &params, const bool p_oCheckSpecificError) const
{
    #ifdef __debugSumErrorLevel2
        wmLog(eDebug, "Product::addLevelTwoErrorAdjacent() <%d>\n", p_oSumErrType);
    #endif
    SmpSumError oError = SmpSumError(new LevelTwoErrorAdjacent(params.m_oThreshold, p_oCheckSpecificError));

    if (oError->setScope(params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval))
    {
        return seFinishAdd(p_oSumErrType, p_oGuid, params.m_oScope, params.m_oSeamseries, params.m_oSeam, params.m_oSeaminterval, oError);
    }
    #ifdef __debugSumErrorLevel2
        wmLog(eDebug, "Product::addLevelTwoErrorAdjacent() returns false\n");
    #endif
    return false;
}


void Product::show() // output
{
    Poco::ScopedLock<Poco::FastMutex> lock(*(m_oSumErrorMutex.get()));
    for (auto itErrs = m_oErrorList.begin(); itErrs != m_oErrorList.end(); ++itErrs)
    {
        tListSumErrors oErrList = itErrs->second;
        for (auto itErr = oErrList.begin(); itErr != oErrList.end(); ++itErr)
        {
            (*itErr)->show();
        }
    }
}

void Product::lStopSetValues(ParameterList &p_rParamList)  const
{
    if (p_rParamList.size() <= 0)
    {
        wmLog( eDebug, "Product::lStopSetValues() no parameters defined\n");
        return; // no sum errors at all
    }
    #ifdef __debugLineStop
        wmLog( eDebug, "Product::lStopSetValues() start with p_rParamList.size() <%d>\n", p_rParamList.size());
    #endif

    std::vector<std::shared_ptr<FilterParameter>> lineErrorList;
    for(std::vector<std::shared_ptr<FilterParameter>>::iterator itTest = p_rParamList.begin() ; itTest != p_rParamList.end();)
    {
        std::string oName = (*itTest)->name();
        Poco::UUID oParamVariantGuid = (*itTest)->typID();
        std::string tmpStr = oParamVariantGuid.toString();
        std::transform(tmpStr.begin(), tmpStr.end(), tmpStr.begin(), ::toupper);
        std::string oErrorIdUpperCase = tmpStr;
        // check the VariantID of the parameter
        //wmLog( eDebug, "Product::lStopSetValues() IN VariantID <%s> : Name <%s>\n", oErrorIdUpperCase.c_str(), oName.c_str() );
        if (oErrorIdUpperCase.compare("8E81426F-624F-4EB3-8CDB-024118F4E673") == 0)
        {
            #ifdef __debugLineStop
                Poco::UUID oParameterID = (*itTest)->parameterID();
                std::string oName = (*itTest)->name();
                //wmLog( eDebug, "Product::lStopSetValues() add VariantID <%s> : Name <%s>\n", oErrorIdUpperCase.c_str(), oName.c_str() );
                wmLog(eDebug, "Product::lStopSetValues() add parameter oParameterID <%s> Name <%s>\n",  oParameterID.toString().c_str(), oName.c_str() );
            #endif
            // found a parameter for lineStopp, copy it into a new list and remove from the origial list
            lineErrorList.push_back(*itTest);
            itTest = p_rParamList.erase(itTest );
        }
        else
        {
            ++itTest;
        }
    }
}
  
void Product::getSumErrorsGivenScopeAndResultTyp(tSumErrorList &sumErrorList, const ResultArgs &resultArgs) const
{
    Poco::ScopedLock<Poco::FastMutex> lock(*(m_oSumErrorMutex.get()));
    MeasureTask const &oMt = *(resultArgs.taskContext().measureTask());
    tSeamIndex oCurrentInspectKey;
    // go through all levels to check
    for (unsigned int oLevel = eScopeProduct; oLevel <= eScopeSeaminterval; ++oLevel)
    {
        // fill up the <tuple>..
        switch(oLevel)
        {
            case eScopeProduct:
                oCurrentInspectKey = tSeamIndex(-1, -1, -1);
                break;
            case eScopeSeamseries:
                oCurrentInspectKey = tSeamIndex(oMt.seamseries(), -1, -1);
                break;
            case eScopeSeam:
                oCurrentInspectKey = tSeamIndex(oMt.seamseries(), oMt.seam(), -1);
                break;
            case eScopeSeaminterval:
                oCurrentInspectKey = tSeamIndex(oMt.seamseries(), oMt.seam(), oMt.seaminterval());
                break;
            default:
                continue;
        }

        tItSumErrors itErr = m_oErrorList.find(oCurrentInspectKey);
        // are there any global errors defined on the given triplet?
        if ( itErr != m_oErrorList.end() )
        {
            tListSumErrors &sumErrors = itErr->second;
            for (auto &sumError : sumErrors)
            {
                ResultEvaluation eval = sumError->findResult(resultArgs); // matches resulttyp and scope
                if ( eval >= eMasked )
                {
                    sumErrorList.push_back( sumError ); // add error to list
                }
            }
        }
    }
}


} // namespace analyzer
} // namespace precitec
