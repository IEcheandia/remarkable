
/**
 *	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2011
 *  @brief			algorithmic interface for STL containers
 */



#ifndef ALGOSTL_H_20111020
#define ALGOSTL_H_20111020


#include "Analyzer_Interface.h"

#include "module/moduleLogger.h"		///< logger
#include "system/stdImplementation.h"	///< vector into stream
#include "geo/array.h"					///< array data structure
#include "geo/range.h"					///< range data structure
#include "filter/parameterEnums.h"		///< enum ValueRankType

#include "system/tools.h"				///< vector to stream
#include "common/defines.h"				///< debug assert integrity assumptions

#include <vector>						///< vector
#include <numeric>						///< accumulate
#include <iterator>						///< ostream_iterator
#include <queue>						///< fifo queue
#include <type_traits>					///< is_integral

namespace precitec {
namespace filter {


/**
 *	@brief		Rounds and converts a floating point value.
 *	@param		p_rVecIn		Input data
 *	@return		T				Rounded and converted input.
 */
template <typename T>
T roundToT(double p_oFloating) { 
	static_assert(std::is_integral<T>::value, "'roundToT' requires integral type.");
	return T(p_oFloating + 0.5);
} // roundToT



/**
 *	@brief		Calculate mean of data vector
 *	@param		p_rVecIn		Input data
 *	@return		T				Mean of input array
 */
template <typename T>
T calcMean1d(const std::vector<T> &p_rVecIn) { 
	poco_assert_dbg(! p_rVecIn.empty());
	T oMean = std::accumulate(p_rVecIn.begin(), p_rVecIn.end(), 0) / p_rVecIn.size(); // sum up values. divide sum by size.
	return oMean;
} // calcMean1d



/**
 *	@brief		Calculates the median for a sequence of values. Uses a selective algorithm. Supports all types that can be compared.
 *	@param		p_oItFirst		Random-access iterator to the initial position of the sequence to be used. 
 *	@param		p_oItLast		Random-access iterator to the final position of the sequence to be used. 
 *	@return		T				Random-access iterator pointing to the location within the range [p_oItFirst,p_oItLast] that will contain the median element.
 */
template <typename T>
T calcMedian(T p_oItFirst, T p_oItLast) {
	poco_assert_dbg(p_oItFirst <= p_oItLast);
	const T	oItMiddle		( p_oItFirst + std::distance(p_oItFirst, p_oItLast) / 2 );
	std::nth_element(p_oItFirst, oItMiddle, p_oItLast);
	return oItMiddle;
} // calcMedian



/**
 *	@brief		Calc Median from 1d histogram. Only works with positive integer values.
 *	@param		p_rHist					Input histogram
 *	@param		p_oHalfHistIntegral		Half integral of histogram (half of possible entries)
 *	@return		std::size_t				Position of median bin (the median)
 */
template<typename T>
std::size_t calcMedianHist (const std::vector<T> &p_rHist, unsigned int p_oHalfHistIntegral) {
	T					oIntegral	= 0;
	const std::size_t	oSize		= p_rHist.size();
	for(unsigned int i = 0; i < oSize; ++i) {
		oIntegral += p_rHist[i];
		if (static_cast<unsigned int>(oIntegral) > p_oHalfHistIntegral) { // if bin greater half oIntegral, it is the median bin, else not. greater becaus half integral is a truncated value
			return i;
		} // if
	} // for
	wmLog(eDebug, "'%s': Wrong control sequence. Check data integrity.", __FUNCTION__);
	return 0; // should never get here
} // calcMedianHist



/** 
 *	@brief	Converts a vector of data type int to  a vector of data type double.
 *	@param	p_rInput				Input vector.
 *	@return	std::vector<double>		Output vector.
*/
inline std::vector<double> intToDouble(const std::vector<int> &p_rInput) {
	std::vector<double>	oOutput	(std::begin(p_rInput), std::end(p_rInput));
	return oOutput;
} // intToDouble



/** 
 *	@brief	Converts a vector of data type double to  a vector of data type int.
 *	@param	p_rInput				Input vector.
 *	@return	std::vector<int>		Output vector.
*/
inline  std::vector<int> doubleToInt(const std::vector<double> &p_rInput) {
	std::vector<int>	oOutput	( p_rInput.size() );
	std::transform(std::begin(p_rInput), std::end(p_rInput), std::begin(oOutput), roundToT<int>);
	return oOutput;
} // doubleToInt



/**
 *	@brief	Checks if both indices are valid for the given vector.
 *	@param	p_rVectorIn			Input vector containing data and rank vector
 *	@param	p_oStart			Start index to be checked.
 *	@param	p_oEnd				End index to be checked.
 *	@return	bool				If both indices are valid
*/
template <typename T>
bool checkIndices (const std::vector<T> &p_rVectorIn, int p_oStart, int p_oEnd) {
	const geo2d::Range	oValidRange		( 0, p_rVectorIn.size() - 1 );
	const geo2d::Range	oRangeToCheck	( p_oStart, p_oEnd );
	// simple check

	if (p_oStart < 0 || p_oEnd < 0) {
		return false;
	} // if

	// range check

	if (oRangeToCheck.isValid() == false || oValidRange.contains(oRangeToCheck) == false) {
		return false;
	} // if

	return true;
} // checkIndices



/**
 *	@brief	Clears a std::queue.
 *	@param	p_rQueueIn			Input fifo queue to be cleared.
 *	@return	void
*/
template <typename T>
void clear (std::queue<T> &p_rQueueIn) {
	while(p_rQueueIn.empty() == false) {
		p_rQueueIn.pop();
	} // while
} // clear

} // namespace filter
} // namespace precitec


#endif // ALGOSTL_H_20111020
