/**
* 	@file
* 	@copyright	Precitec GmbH & Co. KG
* 	@author		AL
* 	@date		2015
* 	@brief		This filter computes basic subsampling
*/

#ifndef DATASUBSAMPLING_H_
#define DATASUBSAMPLING_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>

namespace precitec {
namespace filter {

enum class SubsamplingOperation
{
    NoneOperation = 0,
    LatestValue,
    ArithmeticMean,
    ReductionMeanFactor10,
    Minimum,
    Maximum,
    Median
};


/**
* @brief This filter computes basic arithmetic operations on two input arrays (plus, minus, ...).
*/
class FILTER_API DataSubsampling : public fliplib::TransformFilter
{
public:

    /**
    * CTor.
    */
    DataSubsampling();
    /**
    * @brief DTor.
    */
    virtual ~DataSubsampling();

    // Declare constants
    static const std::string m_oFilterName;			///< Filter name.
    static const std::string m_oPipeOutName;		///< Pipe: Data out-pipe.
    static const std::string m_oParamOperation;		///< Parameter: Operation, e.g. Addition, Subtraction, Multiplication, ...

    /**
    * @brief Set filter parameters.
    */
    void setParameter();

    static geo2d::Doublearray subsample(const geo2d::Doublearray& arrayIn, SubsamplingOperation operation, bool passThroughBadRank);
    static interface::GeoDoublearray subsample(const interface::GeoDoublearray& geoArrayIn, SubsamplingOperation operation, bool passThroughBadRank);

protected:

    /**
    * @brief In-pipe registration.
    * @param p_rPipe Reference to pipe that is getting connected to the filter.
    * @param p_oGroup Group number (0 - proceed is called whenever a pipe has a data element, 1 - proceed is only called when all pipes have a data element).
    */
    bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);

    /**
    * @brief Processing routine.
    * @param p_pSender pointer to
    * @param p_rEvent
    */
    void proceed(const void* p_pSender, fliplib::PipeEventArgs& e);

protected:

    const fliplib::SynchronePipe< interface::GeoDoublearray >* m_pPipeInDataA;	///< Data in-pipe.
    fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutData;			///< Data out-pipe.

    int m_oOperation;			///< Parameter: Operation, e.g. Addition, Subtraction, Multiplication, ...
    bool m_oPassThroughBadRank;	///< Parameter: If PassThroughBadRank is false, data with bad rank is ignored
}; // class DataSubsampling

} // namespace filter
} // namespace precitec

#endif // DATASUBSAMPLING_H_
