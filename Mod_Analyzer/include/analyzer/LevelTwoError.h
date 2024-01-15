/**
 *  @file LevelTwoError.h
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Daniel Zumkeller (Zm)
 *  @date       2020
 * 
 *  Class definitions of all LevelTwo type SumErrors
*/
#pragma once

#include "analyzer/sumError.h"

namespace precitec
{
namespace analyzer
{

//==============================LevelTwoError=(BASE=CLASS)===================================================//
class LevelTwoError : public SumError
{
protected:
    explicit LevelTwoError(double p_oMaxViolation, bool p_oCheckSpecificError)
    : SumError()
    , m_oMaxViolation(p_oMaxViolation)
    , m_checkSpecificError(p_oCheckSpecificError)
    {
        init();
    }


public:
    virtual ~LevelTwoError();

    virtual interface::ResultType type() const override { return interface::ResultType::SumErrorNone; } // not a user type!
    virtual void reset() override;
    LevelTwoError& operator=(LevelTwoError const &rhs);
    void copyValues(LevelTwoError* dest, LevelTwoError const &src);
    //setters
    virtual void setErrorType(const std::int32_t) override;
    void setMaxViolation(double p_oNewMaxViolation) { m_oMaxViolation = p_oNewMaxViolation; }
    virtual ResultEvaluationData testResult(const interface::ResultArgs &p_rRes, const interface::ResultArgs& lwmTriggerSignal) override;
    /**
     * @brief Check if this seam was already included in the sumError (maybe two ore more sumErrors defined on the seam)
    */
    bool valueIsAlreadyHandled(std::int32_t sumErrorValue, const interface::tSeamIndex index);

protected:

    void init();
    // parameters
    double m_oMaxViolation; // pooled length or area of flawed parts necessary to throw an NIO
    bool m_checkSpecificError;
    // internal variables
    double m_oFlawedViolation;         ///< internal "flawed" bucket, tells us how much is flawed
        interface::tSeamIndex m_oLastSeamIndex;
    std::vector<interface::ResultArgs> m_oSingularityList;
};
//==============================LevelTwoError=(BASE=CLASS)===================================================//


//------------------------------LevelTwoErrorAccumulated-----------------------------------------------------//
class LevelTwoErrorAccumulated : public LevelTwoError
{
  /* This error checks each incoming sumerror of matching kind.
   * SumErrors must NOT be consecutive
   *
   */
public:
    explicit LevelTwoErrorAccumulated(double p_oMaxViolation, bool p_oCheckSpecificError) : // explicit ctor
        LevelTwoError(p_oMaxViolation, p_oCheckSpecificError)
    {
        init();
    }

    virtual ~LevelTwoErrorAccumulated();
    void init();

    virtual ResultEvaluation testSumErrorLevel2(const Poco::SharedPtr<SumError> &p_oSumErr, const interface::tSeamIndex index) override;

protected:
    virtual void reset() override;

};
//------------------------------LevelTwoErrorAccumulated-----------------------------------------------------//



//------------------------------LevelTwoErrorAdjacent--------------------------------------------------------//
class LevelTwoErrorAdjacent : public LevelTwoError
{
  /* This error checks each incoming sumerror of matching kind.
   * SumErrors MUST be consecutive
   *
   */
public:
    explicit LevelTwoErrorAdjacent(double p_oMaxViolation, bool p_oCheckSpecificError) : // explicit ctor
        LevelTwoError(p_oMaxViolation, p_oCheckSpecificError)
    {
        init();
    }

    virtual ~LevelTwoErrorAdjacent();
    void init();

    virtual ResultEvaluation testSumErrorLevel2(const Poco::SharedPtr<SumError> &p_oSumErr, const interface::tSeamIndex index) override;
    /**
    * @brief Track the changes in Product (eg new seam or new seamInerval)
    */
    virtual void trackScopeOnProduct(const interface::ResultArgs& p_rRes) override;
    /**
    * @brief Test if the seam handled before the actual seam had an error
    */
    bool isConsecutive(const std::int32_t seamSeries, const std::int32_t seam, const std::int32_t oSeamInterval);

protected:
    virtual void reset() override;
    std::int32_t m_oLastTriggeredSeamSeries;       ///< seamSeries position of last seamError
    std::int32_t m_oLastTriggeredSeam;             ///< seam position of last seamError
    std::int32_t m_oLastTriggeredSeamInterval;     ///< m_oSeamIntervalStart position of last seamError
    std::vector<interface::tSeamIndex> m_oScopeList; ///< store all the scope's of the product
};
//------------------------------LevelTwoErrorAdjacent--------------------------------------------------------//


} // namespace interface
} // namespace precitec
