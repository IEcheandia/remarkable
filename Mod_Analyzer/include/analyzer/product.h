/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			HS
 *  @date			2013
 *  @brief			'Product' is the highest entity in a station, defined by an ID and a number. A product holds one ore more seam series.
 */

#ifndef PRODUCT_H_INCLUDED_20130724
#define PRODUCT_H_INCLUDED_20130724

#include <map>
#include <memory>

#include "common/product.h"

#include "common/measureTask.h"
#include "fliplib/FilterGraph.h" 
#include "message/db.interface.h"

#include "analyzer/seamSeries.h"
#include "event/resultType.h"
#include "event/results.proxy.h"
#include "event/results.h"
#include "sumError.h"
#include "SignalSumError.h"
#include "AnalysisSumError.h"
#include "LevelTwoError.h"
#include "common/graph.h"
#include "fliplib/ArmState.h"
#include "common/referenceCurveSet.h"


namespace precitec {
namespace analyzer {

    /// To manage lifetime and secure release we use shared pointers of global errors.
    typedef Poco::SharedPtr<SumError> SmpSumError;
    /// several global errors can be defined for a given triplet (seamseries, seam, seaminterval), so we need a map to a vector or something similar.
    typedef std::list<SmpSumError> tListSumErrors;
    typedef std::list<SmpSumError>::iterator tItListSumErrors;

    // index types.
    /// Position of global error within vector for previous index. Using a list guarantess that iterators/ references remain unaffected!

    /// This is THE map for storing global errors, which are references by the triplet (seamseries, seam, seamninterval)
    typedef std::map<interface::tSeamIndex, tListSumErrors> tSumErrors;
    typedef std::map<interface::tSeamIndex, tListSumErrors>::iterator tItSumErrors;

    typedef interface::TResults<interface::AbstractInterface>   tResultProxy;

/**
  * @brief 'Product' is the highest entity in a station, defined by an ID and a number. A product holds one ore more seam series.
  * @ingroup Analyzer
  */
class Product { // name conflict with interface::Product, but name maintained to represent gui structure - to be resolved by refactoring interface::Product
public:
    /**
      * @brief CTOR initialized with given product data.
      * @param p_rProductData Product meta data.
      */
    explicit Product(const interface::Product& p_rProductData);
    virtual ~Product();
    /**
      * @brief Adds a new seam series initialized with given measure task.
      * @param p_rMeasureTask Seam series specific measure task data (level 0 task).
      */
    SeamSeries* addSeamSeries(const interface::MeasureTask& p_rMeasureTask);

    /**
      * @brief Comparison operator for map insertion. Compares the inner product ids.
      * @param p_rFirst LHS instance.
      * @param p_rSecond RHS instance.
      */
    friend bool operator<(const Product& p_rFirst, const Product& p_rSecond);

    struct SumErrorParams
    {
        interface::ResultType m_oErrorType;
        ErrorScopeType m_oScope;
        std::int32_t m_oSeamseries;
        std::int32_t m_oSeam;
        std::int32_t m_oSeaminterval;
        double m_oThreshold;
        double m_oSecondThreshold;
        interface::ResultType m_oResultType;
        double m_oMin;
        double m_oMax;
        Poco::UUID m_oReferenceSet;
        bool m_oUseMiddleReference;
        double m_oLwmSignalThreshold;
    };

    bool setResultProxy(tResultProxy *p_pResultProxy) const;
    std::string upperString(std::string p_oStr) const; //helper

    void setSumErrorParam(const interface::SpFilterParameter& p_rParam, SumErrorParams &r_oParams) const;
    void createSumErrors(interface::ParameterList &p_rParamList, const std::vector<interface::ReferenceCurveSet>& productReferenceCurves) const;

    SmpSumError seGetError(Poco::UUID) const;
    bool seDelete(Poco::UUID) const;
    void seDeleteResultTypeList(Poco::UUID) const;
    bool seAddResultType(Poco::UUID, interface::ResultType p_oType) const;
    void seSetErrorType(Poco::UUID, std::int32_t p_oType) const;
    ResultEvaluationList seHandleResult(tSumErrorList &p_rErrors, const interface::ResultArgs &p_rRes, const interface::ResultArgs &lwmTriggerSignal) const;
    void seResetAll() const;
    void seArm(const fliplib::ArmStateBase& state, const interface::tSeamIndex index) const;

    bool seOutlierChangeMaxViolation(Poco::UUID p_oGuid, double p_oNewMaxViolation) const;
    bool seOutlierChangeMinMax(Poco::UUID p_oGuid, double p_oNewMin, double p_oNewMax) const;
    bool seOutlierChangeReference(Poco::UUID p_oGuid, const interface::ReferenceCurveSet &referenceSet) const;


    void seClearAll() const;

    virtual void show();

    struct lStopParams
    {
        std::int32_t m_oVar1MaxFaults;      ///< max err per single part
        std::int32_t m_oVar2PartsCount;     ///< parts defined for err variant 2
        std::int32_t m_oVar2MaxFaults;      ///< max err per parts defined 'm_oVar2PartsCount'
        std::int32_t m_oVar3PartsCount;     ///< parts defined for err variant 3, not realized
        std::int32_t m_oVar3MaxFaults;      ///< max err per parts defined 'm_oVar3PartsCount', not realized
    };
    lStopParams m_olStopParams;
    void lStopSetValues(interface::ParameterList &p_rParamList) const;

    void getSumErrorsGivenScopeAndResultTyp(tSumErrorList &summErrorList, const interface::ResultArgs &resultArgs) const;

    void setExtendedProductInfo(const std::string &extendedProductInfo)
    {
        m_extendedProductInfo = extendedProductInfo;
    }
    const std::string &extendedProductInfo() const
    {
        return m_extendedProductInfo;
    }

    const interface::Product& m_rProductData;
    std::map<int, SeamSeries> m_oSeamSeries;
    mutable int m_oProductNb;


protected:
    void seSendError(const SumError& p_rError, const interface::ResultArgs&) const;
    bool seExistsError(Poco::UUID) const;

    //NOTE: all adders return false if guid already maps to another SumError.
    
/* SeamErrors */
    //generic static boundary accumulated outlier (length, area, peak)
    bool addSignalSumErrorAcccumulatedOutlierStaticBoundary(const int sumErrType, const Poco::UUID p_oGuid, const SumErrorParams &params, const bool p_oAreaError, const bool p_oPeakError) const;
    //generic reference boundary accumulated outlier (length, area, peak)
    bool addSignalSumErrorAccumulatedOutlierReferenceBoundary(const int sumErrType, const Poco::UUID p_oGuid,const SumErrorParams &params, const interface::ReferenceCurveSet &referenceSet, const bool p_oAreaError, const bool p_oPeakError) const;        
    //generic reference boundary single outlier (length, area)
    bool addSignalSumErrorSingleOutlierReferenceBoundary(const int sumErrType, const Poco::UUID p_oGuid, const SumErrorParams &params, const interface::ReferenceCurveSet &referenceSet, const bool p_oAreaError) const;
    //reference boundary single inlier
    bool addSignalSumErrorInlierReferenceBoundary(const int sumErrType, const Poco::UUID p_oGuid, const SumErrorParams &params, const interface::ReferenceCurveSet &referenceSet) const;      
            
    //generic static boundary single inlier (length, area)
    bool addSignalSumErrorSingleOutlierStaticBoundary(const int sumErrType, const Poco::UUID p_oGuid, const SumErrorParams &params, const bool p_oAreaError) const;            
    //static boundary single inlier
    bool addSignalSumErrorSingleInlierStaticBoundary(const int sumErrType, const Poco::UUID p_oGuid, const SumErrorParams &params) const;
    
    //static boundary dual outlier
    bool addSignalSumErrorDualOutlierInRangeStaticBoundary(const int sumErrType, const Poco::UUID p_oGuid, const SumErrorParams &params) const;
    //reference boundary dual outlier
    bool addSignalSumErrorDualOutlierInRangeReferenceBoundary(const int sumErrType, const Poco::UUID p_oGuid, const SumErrorParams &params, const interface::ReferenceCurveSet &referenceSet) const;
            

/* Analysis Type Seam Errors */
    bool addAnalysisSumErrorAccumulatedOutlier(const int sumErrType, const Poco::UUID p_oGuid, const SumErrorParams &params) const;
    bool addAnalysisSumErrorAdjacentOutlier(const int sumErrType, const Poco::UUID p_oGuid, const SumErrorParams &params) const;
    
    
/* Level Two Errors */
    //generic accumulated level two error (for LevelTwoErrorAccumulated and LevelTwoErrorErrorOnlyAccumulated)
    bool addLevelTwoErrorAccumulated(const int sumErrType, const Poco::UUID p_oGuid, const SumErrorParams &params, const bool p_oCheckSpecificError) const;
    //generic adjacent level two error (for LevelTwoErrorAdjacent and LevelTwoErrorErrorOnlyAdjacent)
    bool addLevelTwoErrorAdjacent(const int sumErrType, const Poco::UUID p_oGuid, const SumErrorParams &params, const bool p_oCheckSpecificError) const;


    // maps Error GUID to ResultType
    static std::map<std::string, interface::ResultType> createSumErrorMap() // maps GUID FehlerTypID
    {
    std::map<std::string, interface::ResultType> oMap;
    //SignalSeamError                                                                                                           // [What?]                   [Where?]           
    oMap["CEDEACB4-D4BB-4FDC-945A-BDC54E5848A6"] = interface::ResultType::SignalSumErrorAccumulatedOutlierStaticBoundary;       //  Total Length              outside Static Boundary over threshold
    oMap["3B5FE50F-6FD5-4FBC-BD78-06B892E1F97D"] = interface::ResultType::SignalSumErrorSingleOutlierStaticBoundary;            //  First consecutive length  outside Static Boundary over threshold
    oMap["740FD8B3-852C-485A-BC24-6C67A36DABD2"] = interface::ResultType::SignalSumErrorAccumulatedAreaStaticBoundary;          //  Total Area                outside Static Boundary over threshold
    oMap["73708EA1-580A-4660-8D80-63622670BC7C"] = interface::ResultType::SignalSumErrorSingleAreaStaticBoundary;               //  First consecutive Area    outside Static Boundary over threshold
    oMap["3AF9EF6A-A4E9-4234-8BA5-7B42D3E58B2C"] = interface::ResultType::SignalSumErrorInlierStaticBoundary;                   //  First conescutive length  inside  Static Boundary over threshold
    oMap["396CA433-AD11-4073-A2B2-5314CC41D152"] = interface::ResultType::SignalSumErrorPeakStaticBoundary;                     //  First value               outside Static Boundary over threshold
    oMap["55DCC3D9-FE50-4792-8E27-460AADDDD09F"] = interface::ResultType::SignalSumErrorDualOutlierInRangeStaticBoundary;       //  First Two Lengths         outside Static Boundary within threshold Range
    
    oMap["F8F4E0A8-D259-40F9-B134-68AA24E0A06C"] = interface::ResultType::SignalSumErrorAccumulatedOutlierReferenceBoundary;    //  Total Length              outside Reference Boundary over threshold
    oMap["5EB04560-2641-4E64-A016-14207E59A370"] = interface::ResultType::SignalSumErrorSingleOutlierReferenceBoundary;         //  First consecutive length  outside Reference Boundary over threshold
    oMap["527B7421-5DDD-436C-BE33-C1A359A736F6"] = interface::ResultType::SignalSumErrorAccumulatedAreaReferenceBoundary;       //  Total Area                outside Reference Boundary over threshold
    oMap["D36ECEBA-286B-4D06-B596-0491B6544F40"] = interface::ResultType::SignalSumErrorSingleAreaReferenceBoundary;            //  First consecutive Area    outside Reference Boundary over threshold
    oMap["4A6AE9B0-3A1A-427F-8D58-2D0205452377"] = interface::ResultType::SignalSumErrorInlierReferenceBoundary;                //  First conescutive length  inside  Reference Boundary over threshold
    oMap["7CF9F16D-36DE-4840-A2EA-C41979F91A9B"] = interface::ResultType::SignalSumErrorPeakReferenceBoundary;                  //  First value               outside Reference Boundary over threshold
    oMap["C0C80DA1-4E9D-4EC0-859A-8D43A0674571"] = interface::ResultType::SignalSumErrorDualOutlierInRangeReferenceBoundary;    //  First Two Lengths         outside Reference Boundary within threshold Range
    
    //AnalysisSumError                                                                                          // [What?]                 [Where?]  
    oMap["8C30056C-EB34-435D-A4F8-EEBF9034BDE9"] = interface::ResultType::AnalysisSumErrorAccumulatedOutlier;   // Total length             outside Static Boundary
    oMap["B73E885B-A790-4E73-80C0-A86DF750EB21"] = interface::ResultType::AnalysisSumErrorAdjacentOutlier;      // First consecutive length outside Static Boundary
    
    //LevelTwoError                                                                                         // [What?]                         [Where?]  
    oMap["C19E43C7-EBC0-4771-A701-BA102511AD9F"] = interface::ResultType::LevelTwoErrorAccumulated;         // Total NIO over Value             across allseams
    oMap["37E21057-EFD4-4C18-A298-BE9F804C6C04"] = interface::ResultType::LevelTwoErrorAdjacent;            // First consecutive NIO over value across seams
    oMap["B47EBF8B-0D37-48BD-8A6A-0D6E09885C1F"] = interface::ResultType::LevelTwoErrorErrorOnlyAccumulated;// Total NIO count                  across all seams
    oMap["753EA75B-4B82-418E-B1F2-0EEA0D4C0D50"] = interface::ResultType::LevelTwoErrorErrorOnlyAdjacent;   // First consecutive NIO count      across all seams
    return oMap;
    }
    static const std::map<std::string, interface::ResultType> sumErrorMap_;

    static std::map<std::string, unsigned int> createParamListMap()
    {
        std::map<std::string, unsigned int> oMap;
        oMap["ERROR"]           = 0;
        oMap["SCOPE"]           = 1;
        oMap["SEAMSERIES"]      = 2;
        oMap["SEAM"]            = 3;
        oMap["SEAMINTERVAL"]    = 4;
        oMap["THRESHOLD"]       = 5; // max area or length of allowed violations 
        oMap["RESULT"]          = 6;
        oMap["MIN"]             = 7; // min limit for static | lower deviation for reference boundaries
        oMap["MAX"]             = 8; // max limit for static | upper deviation for reference boundaries
        oMap["REFERENCE"]       = 9;
        oMap["MIDDLEREFERENCE"] = 10; //use middleReference or Upper/Lower references
        oMap["SECONDTHRESHOLD"] = 11; //second threshold value, currently used for dualOutlier errors
        oMap["LWMSIGNALTHRESHOLD"] = 12; // lwm signal threshold value, used to determine when the inspection of lwm signals should begin
        return oMap;
    }
    static const std::map<std::string, unsigned int> paramListMap_;

    static const unsigned int m_oEntrySize; // number of entries of paramListMap_ --> keep this consistent with the mentioned map!!!

    /* end section sum errors */
    ////////////////////////////

private:
    /// This is the connection from global errors to outer spheres via GUID. Entry ist triplet seamseries, seam, seaminterval plus vector index
    struct tSeamErrorIndex
    {
        explicit tSeamErrorIndex(interface::tSeamIndex p_oIdx, tItListSumErrors p_oIt)
        {
            idx = p_oIdx;
            theIt = p_oIt;
        }

        tSeamErrorIndex()
        {
            idx = interface::tSeamIndex(-1, -1, -1);
        }

        interface::tSeamIndex idx;
        tItListSumErrors theIt;
    };
    typedef std::map<Poco::UUID, tSeamErrorIndex > tMapIdGlobalError;

    mutable tSumErrors m_oErrorList;
    mutable tMapIdGlobalError m_oGuidToError;
    mutable tResultProxy* ResultProxy_;
    bool seFinishAdd(const std::int32_t p_oSumErrType, Poco::UUID m_oGuid, ErrorScopeType p_oScope,
            std::int32_t p_oSeamseries, std::int32_t p_oSeam, std::int32_t p_oSeaminterval,
            SmpSumError &p_rError) const;
            
    bool addReferenceSignalError(const int sumErrType, const Poco::UUID p_oGuid, const SumErrorParams &params, const interface::ReferenceCurveSet &referenceSet) const;
    std::shared_ptr<Poco::FastMutex> m_oSumErrorMutex;

    std::string m_extendedProductInfo;

};



bool operator<(const Product& p_rFirst, const Product& p_rSecond); // for ordered containers

} // namespace analyzer
} // namespace precitec

#endif /* PRODUCT_H_INCLUDED_20130724 */
