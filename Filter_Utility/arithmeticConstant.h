/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2015
 * 	@brief		This filter computes basic arithmetic operations on a single input array and a constant (plus, minus, ...).
 */

#ifndef ARITHMETICCONSTANT_H_
#define ARITHMETICCONSTANT_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>
// stl includes
#include <deque>

// This classes (for Unittest) may access the private variables
class ArithmeticConstantTest;
class ArithmeticConstantTest_WithQueue;
class ArithmeticConstantTest_WithArray;

namespace precitec {
namespace filter {
    
/**
 * @brief This filter computes basic arithmetic operations on a single input array and a constant (plus, minus, ...).
 */
class FILTER_API ArithmeticConstant : public fliplib::TransformFilter
{

    enum class Operation : int {
        eAddition = 0,           //  0
        eSubtraction,            //  1
        eMultiplication,         //  2
        eDivision,               //  3
        eModulo,                 //  4
        eMaximum,                //  5
        eMinimum,                //  6
        eReaches,                //  7
        eReachesNot,             //  8
        eMaxInWindow,            //  9
        eMinInWindow,            // 10
        eAverageInWindow,        // 11
        eExponentialFunction,    // 12
        eMaxElement,             // 13
        eMinElement,             // 14
        eTruncate,               // 15
        eRound                   // 16
    };
    
public:

	/**
	 * CTor.
	 */
	ArithmeticConstant();
	/**
	 * @brief DTor.
	 */
	virtual ~ArithmeticConstant();

	// Declare constants
	static const std::string m_oFilterName;			///< Filter name.
	static const std::string m_oPipeOutName;		///< Pipe: Data out-pipe.
	static const std::string m_oParamOperation;		///< Parameter: Type of operation.
	static const std::string m_oParamValue;			///< Parameter: Constant value.

	/**
	 * @brief Set filter parameters.
	 */
	void setParameter();
    
	/**
	 * @brief Arm the filter. This means here, that the length of the seam is determined and memory is allocated for all the data elements.
	 */
	virtual void arm(const fliplib::ArmStateBase& state);

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
	void proceed( const void* p_pSender, fliplib::PipeEventArgs& p_rE );

protected:

	const fliplib::SynchronePipe< interface::GeoDoublearray >*	m_pPipeInData;			///< Data in-pipe.
	fliplib::SynchronePipe< interface::GeoDoublearray >			m_oPipeOutData;			///< Data out-pipe.

	Operation													m_oOperation;			///< Parameter: Operation, e.g. Addition, Subtraction, Multiplication, ...
	double														m_oValue;				///< Parameter: Constant value.

	std::deque< double >                                        m_oWindow;              ///< Window of the last n values
	
	// To allow the Unittest to access the private variables
	friend ArithmeticConstantTest;
	friend ArithmeticConstantTest_WithQueue;
	friend ArithmeticConstantTest_WithArray;
	
}; // class ArithmeticConstant

} // namespace filter
} // namespace precitec

#endif /* ARITHMETICCONSTANT_H_ */
