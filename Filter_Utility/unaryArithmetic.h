/**
 * 	@file
 * 	@copyright	Precitec GmbH & Co. KG
 * 	@author		AL
 * 	@date		2015
 * 	@brief		This filter computes basic unary arithmetic operations + special Audi operations).
 */

#ifndef UNARYARITHMETIC_H_
#define UNARYARITHMETIC_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>

namespace precitec {
namespace filter {

/**
 * @brief This filter computes basic unary arithmetic operations + special Audi operations).
 */
class FILTER_API UnaryArithmetic : public fliplib::TransformFilter
{
public:

	/**
	 * CTor.
	 */
	UnaryArithmetic();
	/**
	 * @brief DTor.
	 */
	virtual ~UnaryArithmetic();

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
	 * @param p_rEvent
	 */
	void proceed(const void* sender, fliplib::PipeEventArgs& e);

	

protected:

	const fliplib::SynchronePipe< interface::GeoDoublearray >*	m_pPipeIn;			///< Data in-pipe.
	fliplib::SynchronePipe< interface::GeoDoublearray >			m_oPipeOut;			///< Data out-pipe.

	int 														m_oOperation;			///< Parameter: Operation, e.g. Addition, Subtraction, Multiplication, ...
	geo2d::Doublearray m_oLastValue;				///< last values
	void arm(const fliplib::ArmStateBase& p_rArmstate);

}; // class Arithmetic

} // namespace filter
} // namespace precitec

#endif // UNARYARITHMETIC_H_
