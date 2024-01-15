/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		  Andreas Beschorner (AB)
 * 	@date		    2012
 * 
 *  Adapted for oversampling and refactored: Daniel Zumkeller 20.03.2020
 */

#ifndef __PV_GlobalSumFilterll
#define __PV_GlobalSumFilterll

// errors
//#define __debugSignalSumError //for all signalSumError types
//#define __debugAnalysisError  //for all AnalysisSumError types
//#define __debugLevelTwoError  //for all LevelTwoError types

/* DEBUG FLAGS FOR resultHandler AND product */
//#define __debugSumErrors
//#define __debugLineStop
//#define __debugEnvelopeError

#include <cstdint>
#include <cstdio>
#include <inttypes.h>
#include <iostream>
#include <map>
#include <vector>
#include <functional>

#include "common/defines.h"
#include "event/resultType.h"
#include "event/results.h"
#include "event/results.proxy.h"
#include "InterfacesManifest.h"
#include "common/measureTask.h"
#include "fliplib/ArmState.h"
#include "Poco/SharedPtr.h" // needed for SmpResult for global errors
#include "geo/point.h"

#include "module/moduleLogger.h" //for debugging outputs in the WM Logger


/*
 * The idea is, that global Errors are objects of a measureTask and thus intrinsically have their scope specified.
 * This class is NOT part of module_analyzer, as it is included by the product and we do not want include recursions...
 */

/*
    The naming ***Error*** is used to differentiate between NIOs and global Errors.

    In case of multiple sensors we have not yet defined anything, as they might produce identical NIO types.
    How do we want to handle this? Separate NIO-signal flows for each sensor or couple identical NIO types?...

    For now: One sensor, classification by NIO type
*/


//#define IncludeIntervalSegmentation

namespace precitec
{
namespace analyzer
{

enum TriggerBase { eImageTrigger = 0, eTimecodeTrigger, ePositionTrigger };
enum ErrorScopeType { eScopeNone= 0, eScopeProduct, eScopeSeamseries, eScopeSeam, eScopeSeaminterval, eNumScopes };
enum ResultEvaluation {eNotMasked = 0, eBadScope, eMasked, eSumErrorNotTriggered, eSumErrorTriggered};


struct ResultEvaluationData {
    ResultEvaluation evaluationStatus;
    std::vector<float> signalQuality; // quality of each signal value (0.0 means equal to reference, 100.0 means at bottom/top reference and just counted as NIO, >=100.0 means NIO)
    geo2d::VecDPoint nioPercentage;
    geo2d::VecDPoint upperReference; // upper reference curve part that was used to evaluate the error in this frame (NULL if using static boundary)
    geo2d::VecDPoint lowerReference; // lower reference curve part that was used to evaluate the error in this frame (NULL if using static boundary)
};




//======================================================================================//
//////////////////////////////////////////////////////////////////////
// SUM ERROR - BASE CLASS (CALLED IF NO SUM ERROR HAS BEEN DEFINED) //
//////////////////////////////////////////////////////////////////////
class SumError
{
public:
    typedef int64_t tIndex; // seampositionOffset and position in seam
    typedef std::multimap<tIndex, interface::SmpResultPtr> tResultMap;
    typedef std::multimap<tIndex, interface::SmpResultPtr>::iterator tResultMapIt;

    typedef interface::TResults<interface::AbstractInterface> tResultProxy;

protected:
    explicit SumError() :
            m_oTriggerDistance(-1), m_oTriggerBase(TriggerBase::ePositionTrigger), m_oPercentageLimit(0),
      m_oErrorType(interface::ResultType::SumErrorNone), m_oGetTriggerAlgorithm(std::mem_fn(&SumError::getImageTrigger))

    {
        // default: check for errors over complete product. Default trigger: Image number trigger.
        setTriggerBase(TriggerBase::ePositionTrigger);
        m_oHasBeenSend = false;
    };

public:
    static const interface::ResultArgs m_rLastResult1;
    virtual ~SumError();
    /// Resets internal data, counters and errors. Does NOT remove/reset the list of ResultTypes the error accepts, only errors and their data!
    virtual void reset();
    void copyValues(SumError *dest, const SumError& src);

    /// clears list of resulttypes
    void clearResultTypeList()
    {
        m_oResultTypes.clear();
    }

    /// This is the kind of result/NIO to surveil.
    virtual void addResultToSurveil(interface::ResultType p_oType)
    {
        m_oResultTypes.push_back(p_oType);
    }

    /* This method should be overriden by every derived class!
     * We cannot make it pure virtual or we would have to define the oeprators in the derived classes, too...
     */
    ///< Returns whether there is currently a global error, depending on individual criteria for each kind of error.
    virtual bool isError() const { return false; }

    /* Setup Methods, should be ok for derived classes, too */
    bool setScope(const ErrorScopeType, const std::int32_t, const std::int32_t, const std::int32_t);
    inline ErrorScopeType scope() const { return m_oScope; }

    inline interface::ResultType errorType() const { return this->m_oErrorType; }

    /* These methods must not be overriden by derived classes, as the error check is supposed to be consistent and remain untouched.
        The idea is, that we can make use of simple logic polymorphically. Internally, each derived class can have its individual
        isError() method, the result of which is used for the logic action taken in the respective operator, see globalFilter.cpp
    */
    virtual bool operator | (SumError&) const final;
    virtual bool operator & (SumError&) const final;

    SumError& operator=(SumError const &rhs);
    virtual void clearErrors();

    virtual void show();

    void setPercentageLimit(int&);                                  ///< set percentage limit for global NIOs w.r.t. seam
    inline int percentageLimit() { return m_oPercentageLimit; }     ///< returns percentage limit for global NIOs w.r.t. seam

    void setTriggerBase(const TriggerBase&);
    inline TriggerBase triggerBase() { return m_oTriggerBase; }

    virtual interface::ResultType type() const { return interface::ResultType::SumErrorNone; }

    virtual ResultEvaluationData testResult(const interface::ResultArgs& p_rRes, const interface::ResultArgs& lwmTriggerSignal); ///< check for error type and, if it meets global error criteria, call pushResult
    
    virtual ResultEvaluation testSumErrorLevel2(const Poco::SharedPtr<SumError> &p_oSumErr, const interface::tSeamIndex index);

    ResultEvaluation findResult(const interface::ResultArgs& p_rRes) const;
    ResultEvaluation testScope(const interface::ResultArgs& p_rRes) const;
    virtual void setErrorType(const std::int32_t);
    virtual void keepResultAsLastResult(const interface::ResultArgs& p_rRes);
    virtual void trackScopeOnProduct(const interface::ResultArgs& p_rRes);

    // please use with care, as the guid is used as a reference in the product!!
    void setGuid(Poco::UUID p_oGuid)
    {
        m_oGuid = p_oGuid;
    }
    Poco::UUID guid()
    {
        return m_oGuid;
    }

    void setSumErrorType(std::int32_t p_oSumErrType)
    {
        m_oSumErrorType = p_oSumErrType;
    }
    std::int32_t sumErrorType() const
    {
        return m_oSumErrorType;
    }

    
    static ErrorScopeType scopeStringToEnum(const std::string);

    char* getLong64AsString(tIndex lValue);

    virtual void clear();

    virtual void getSELimits(double &lower, double &upper) { };

    /**
     * @brief When the seam or seam series ends, some types of sum-error need to be informed, as they perform the actual checks at the on the scope.
     */
    virtual interface::ResultArgs* arm (const fliplib::ArmStateBase& state);

protected:
    bool updateTriggerDistance(const interface::ResultArgs&);

    tIndex getImageTrigger(const interface::ResultArgs&);    ///< algorithm functional, get result map key from image number based trigger
    tIndex getTimecodeTrigger(const interface::ResultArgs&); ///< algorithm functional, get result map key from timecode based trigger
    tIndex getPositionTrigger(const interface::ResultArgs&); ///< algorithm functional, get result map key from encoder position trigger

    std::vector<interface::ResultType> m_oResultTypes;    ///< Kinds of results/NIO to surveil
    std::vector<interface::ResultArgs> m_oLastResultsList; ///< keep the last result, need this for adjacent errors
    ErrorScopeType m_oScope;                   ///< type of validity. Interval validity not across seams or seamseriesses.
    std::int32_t m_oSeamseries;                ///< scope
    std::int32_t m_oSeam;                      ///< scope
    std::int32_t m_oSeaminterval;              ///< scope
    std::int32_t m_oTriggerDistance;           ///< 1 for images, triggerDistance of measureTask for position, ...
    bool m_oHasBeenSend;

    TriggerBase m_oTriggerBase;                ///< base trigger type;
    int m_oPercentageLimit;                    ///< limit for percentage w.r.t. seam before throwing a global NIO. ALWAYS w.r.t SEAM!!! Default 0 (ignore)
    interface::ResultType m_oErrorType;        /// eg QualityFault
    Poco::UUID m_oGuid;                        /// GUID
    std::int32_t m_oSumErrorType;              /// eg SumErrorOutlierAll
    interface::ResultArgs m_oErrorResult; 	   ///< We need to keep our own result object, on case we need to send a result outside of a seam ...

    std::function<tIndex (SumError&, const interface::ResultArgs&)> m_oGetTriggerAlgorithm;
};
//======================================================================================//

typedef std::vector<Poco::SharedPtr<SumError> > tSumErrorList;
typedef std::vector<ResultEvaluationData> ResultEvaluationList;


} // namespace interface
} // namespace precitec

#endif
