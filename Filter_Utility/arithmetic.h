/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2013
 * 	@brief		This filter computes basic arithmetic operations on two input arrays (plus, minus, ...).
 */

#ifndef ARITHMETIC_H_
#define ARITHMETIC_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>

namespace precitec {
namespace filter {

/**
 * @brief This filter computes basic arithmetic operations on two input arrays (plus, minus, ...).
 */

class FILTER_API Arithmetic : public fliplib::TransformFilter
{
	
	enum class Operation : int {
		eAddition = 0,         //  0
		eSubtraction,          //  1
		eMultiplication,       //  2
		eDivision,             //  3
		eModulo,               //  4
		eMaximum,              //  5
		eMinimum,              //  6
		eSetRank,              //  7
		eLogicalAND,           //  8
		eLogicalOR,            //  9
		eABiggerEqualB,        // 10
		eBBiggerEqualA,        // 11
		eEuclideanNorm,        // 12
        eAppend
	};
	
public:

	/**
	 * CTor.
	 */
	Arithmetic();
	/**
	 * @brief DTor.
	 */
	virtual ~Arithmetic();

	// Declare constants
	static const std::string m_oFilterName;			///< Filter name.
	static const std::string m_oPipeOutName;		///< Pipe: Data out-pipe.
	static const std::string m_oParamOperation;		///< Parameter: Operation, e.g. Addition, Subtraction, Multiplication, ...

	/**
	 * @brief Set filter parameters.
	 */
	void setParameter();

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
	 * @param p_rEvent  EventArgs
	 */
	void proceedGroup( const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEvent );

protected:
    geo2d::Doublearray mergeArrays(const geo2d::Doublearray & array1, const geo2d::Doublearray & array2);

	const fliplib::SynchronePipe< interface::GeoDoublearray >*	m_pPipeInDataA;			///< Data in-pipe.
	const fliplib::SynchronePipe< interface::GeoDoublearray >*	m_pPipeInDataB;			///< Data in-pipe.
	fliplib::SynchronePipe< interface::GeoDoublearray >			m_oPipeOutData;			///< Data out-pipe.

	Operation													m_oOperation;			///< Parameter: Operation, e.g. Addition, Subtraction, Multiplication, ...

}; // class Arithmetic

} // namespace filter
} // namespace precitec

#endif // ARITHMETIC_H_
