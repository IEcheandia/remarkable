
/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2010-2011
 *  @brief			algorithmic interface for class TArray
 */


#ifndef ALGOARRAY_H_20110928
#define ALGOARRAY_H_20110928


#include "Analyzer_Interface.h"

#include "module/moduleLogger.h"	///< logger
#include "common/defines.h"			///< constants

#include "geo/array.h"				///< array data structure
#include "geo/geo.h"				///< GeoVecIntarray
#include "geo/range.h"				///< range

#include "filter/parameterEnums.h"	///< enum ValueRankType
#include "filter/algoStl.h"			///< stl algos
#include "system/templates.h"		///< traits

#include <iostream>					///< cout, ostream
#include <utility>					///< pair, tuple
#include <algorithm>				///< for_each
#include <functional>				///< bind, function
#include <array>					///< array
#include <limits>					///< max, min
#include <vector>					///< dynamic array
#include <tuple>

#undef max
#undef min


namespace precitec {
namespace filter {


/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Conversion from int rank [0 255] to double rank [0.0 1.0]
 *	@details	ATTENTION: filter::eRankMax must not be zero.
 *				Usage: double oGeoRank = intToDoubleRank(RANK_MAX);
 *	@param		p_oValRank	Int rank value, usually used in class TArray.
 *	@return		double		Double rank value, usually used in Geo-structures.
 *	@sa			parameterEnums.h, array.h, filter::ValueRankType, TGeo, TArray
*/
inline double intToDoubleRank(int p_oValRank) { assert(filter::eRankMax != 0); return p_oValRank / static_cast<double>(filter::eRankMax); }


/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Conversion from double rank [0.0 1.0] to int rank [0 255]
 *	@details	Usage: int oRank = doubleToIntRank(0.5);
 *	@param		p_oRank	Double rank value, usually used in Geo-structures.
 *	@return		int		Int rank value, usually used in class TArray.
 *	@sa			parameterEnums.h, array.h, filter::ValueRankType, TGeo, TArray
*/
inline int doubleToIntRank(double p_oRank) { return static_cast<int>(p_oRank * static_cast<int>(filter::eRankMax)); }



/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Inserts array data into output stream.
 *	@param		p_rOStream	Out stream to be modified.
 *	@param		p_rArray	Array to be inserted into stream.
 *	@return		ostream		Modified stream.
 *	@sa			array.h, algoStl.h
*/
template <typename T>
std::ostream& operator<<( std::ostream& p_rOStream, const geo2d::TArray<T> &p_rArray ) {
	return p_rOStream << "Data:\n" << p_rArray.getData() << "Rank:\n" << p_rArray.getRank();
} // operator<<



/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Calculates weighted mean value and mean rank of a TArray<T>. DEPRECATED. 20130218: CONSENSUS BY SB, AB, HS IS NOT TO USE WEIGHTED MEAN.
 *	@details	Sums up all values weighted with their rank and divide this sum by rank sum. Also calculates mean rank.
 *	@param		p_rArrayIn			Input Array containing data and rank vector
 *	@return		std::tuple<T, int>	Output weighted mean of data (first) and mean rank (second)
 *	@sa			TArray, std::inner_product
*/
template <typename T>
std::tuple<T, int> calcWeightedMean (const geo2d::TArray<T> &p_rArrayIn) {
	const std::vector<T> &rInData	= p_rArrayIn.getData();
	const std::vector<int> &rInRank	= p_rArrayIn.getRank();

	poco_assert_dbg( ! rInData.empty() );
	poco_assert_dbg( ! rInRank.empty() );

	const int	oRankSum	= std::accumulate( rInRank.begin(), rInRank.end(), 0 ); // sum up values

	std::pair<T, int> oWeightedMean;	// return value

	if (oRankSum != 0) {
		std::get<eData>(oWeightedMean)	= std::inner_product( rInData.begin(), rInData.end(), rInRank.begin(), T() ) / oRankSum; // assign mean value
	}
	std::get<eRank>(oWeightedMean)	= oRankSum / rInRank.size(); // assign mean rank. non zero division asserted above 

	return oWeightedMean; // return rank weighted mean value and average rank

} // calcWeightedMean



/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Calculates mean value and mean rank of a TArray<T>.
 *	@details	Calculates the mean value ignoring bad ranked-values (zero rank).
 *	@param		p_rArrayIn			Input Array containing data and rank vector
 *	@return		std::tuple<T, int>	Output weighted mean of data (first) and mean rank (second)
 *	@sa			TArray, calcWeightedMean
*/
template <typename T>
std::tuple<T, int> calcMean (const geo2d::TArray<T> &p_rArrayIn) {
	const std::vector<T>		&rInData		( p_rArrayIn.getData() );
	const std::vector<int>		&rInRank		( p_rArrayIn.getRank() );

	poco_assert_dbg( ! rInData.empty() );
	poco_assert_dbg( ! rInRank.empty() );

	auto						oItRank			( std::begin(rInRank) );
	auto						oItData			( std::begin(rInData) );
	int							oNbNotBadValues	( 0 );
	T							oSumData		( (T()) );
	int							oSumRank		( 0 );
	while(oItData != std::end(rInData))
	{
		// rank doesn't matter
		if (*oItRank == eRankMin) { // ignore min rank values
			++oItData;
			++oItRank;

			continue;
		} // if


		oSumData = oSumData + *oItData; // missing += op for TPoint
		oSumRank += *oItRank;

		++oItData;
		++oItRank;
		++oNbNotBadValues;
	} // for

	std::pair<T, int>			oMean			( T(), eRankMin );	// return value
	if (oNbNotBadValues == 0) {
		return oMean;
	}

	std::get<eData>(oMean)	= oSumData / oNbNotBadValues; // assign mean value
	std::get<eRank>(oMean)	= oSumRank / oNbNotBadValues; // assign mean rank. non zero division asserted above 

	return oMean; // return mean value and mean rank

} // calcMean



/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Calculates median of a TArray<T>. Works with any data type that can be compared. Ignores bad-ranked values.
 *	@param		p_oArrayIn			Input Array containing data and rank vector
 *	@return		std::pair<T, int>	Output weighted mean of data (first) and mean rank (second)
*/
template <typename T>
std::tuple<T, int> calcMedian1d (geo2d::TArray<T> p_oArrayIn) {
	auto&			rInData			( p_oArrayIn.getData() );
	auto&			rInRank			( p_oArrayIn.getRank() );
	
	std::tuple<T, int>			oMedian;			// return value
	const auto					oIsNotBadRank			( [] (int p_rRank) { return p_rRank != eRankMin; } );
	const auto					oNbOfNotBadRank			( std::count_if(rInRank.begin(), rInRank.end(), oIsNotBadRank ) );
	
	if (oNbOfNotBadRank != 0) {
		auto oItDataDest	= std::begin(rInData); // same as source
		auto oItRank		= std::begin(rInRank);
		for(auto oItData = std::begin(rInData); oItData != std::end(rInData); ++oItData, ++oItRank) {
			if (oIsNotBadRank(*oItRank) == true) {
				*oItDataDest	= *oItData;
				++oItDataDest;
			} // if
		} // for
		rInData.resize(oNbOfNotBadRank);
		const auto			oRankSum		( std::accumulate(std::begin(rInRank), std::end(rInRank), 0) ); // sum up values
		std::get<eData>(oMedian)	= *calcMedian(std::begin(rInData), std::end(rInData));
		std::get<eRank>(oMedian)	= int(oRankSum / oNbOfNotBadRank); // mean rank
	}
	else {
		std::get<eRank>(oMedian)			= eRankMin;
	}

	return oMedian; // return rank weighted mean value and average rank

} // calcMedian



/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Calc Median from 1d histogram. Only works with positive integer values.
 *	@param		p_rArrayIn			Input Array containing data and rank vector
 *	@param		p_oValueRange		Possible value range of data. May be negative.
 *	@param		p_rHistoMem			(Empty) vector for computation. Size wille be adjusted within function.
 *	@return		std::pair<T, int>	Output weighted mean of data (first) and mean rank (second)
*/
template <typename T>
std::pair<T, int> calcMedianHist (const geo2d::TArray<T> &p_rArrayIn, geo2d::Range p_oValueRange, geo2d::TArray<int> p_rHistoMem) {
	const std::vector<T>	&rInData	= p_rArrayIn.getData();
	const std::vector<int>	&rInRank	= p_rArrayIn.getRank();

	const auto				oOffset		= p_oValueRange.start() < 0 ? - p_oValueRange.start() : 0; // if negative range begin set offset

	poco_assert_dbg( ! rInData.empty() );
	poco_assert_dbg( ! rInRank.empty() );

	p_rHistoMem.assign(p_oValueRange.length(), 0); // init histogram
	std::vector<int> &rHistogramData		= p_rHistoMem.getData();
	std::vector<int> &rHistogramRank		= p_rHistoMem.getRank();

	auto oItRankIn	= rInRank.begin();
	for (auto oItData = rInData.begin(); oItData != rInData.end(); ++oItData) { // loop over all elements	
		if (p_oValueRange.contains(int(*oItData)) == false) {
			std::ostringstream	oMsg;
			oMsg << __FUNCTION__ << " WARNING: Value (" << *oItData << ") does not lie within given value range " << p_oValueRange << " - skipped.\n";
			wmLog( eDebug, oMsg.str() );
			++oItRankIn;
			continue;
		}
		++rHistogramData[int(*oItData) + oOffset];
		rHistogramRank[int(*oItData) + oOffset] += *oItRankIn; // accumulate rank for corresponding data bin.
		++oItRankIn;
	} // for


	std::pair<T, int> oMedian;	// return value

	const unsigned int oHalfIntegral	= rInData.size() / 2;

	const unsigned int oMedianBinOff	= calcMedianHist(rHistogramData, oHalfIntegral); // substract offset
	poco_assert_dbg(rHistogramData[oMedianBinOff] != 0); // integrity error
	std::get<eData>(oMedian)			= oMedianBinOff - oOffset;

	const unsigned int oNbOfBadRank		( std::count_if(rInRank.begin(), rInRank.end(), [] (int p_rRank) { return p_rRank == eRankMin; } ) );
	if (oNbOfBadRank <= oHalfIntegral) { // less equal than truncated half
		std::get<eRank>(oMedian)			= rHistogramRank[oMedianBinOff] / rHistogramData[oMedianBinOff]; // get mean rank: divide rank sum by number of values
	}
	else {
		std::get<eRank>(oMedian)			= eRankMin;
	}

	return oMedian; // return rank weighted mean value and average rank

} // calcMedian



/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Calculates extremum of data array. Bad rank values are excluded from search.
 *	@param		p_rArrayIn			Input Array containing data and rank vector
 *	@param		p_oDirection		Search direction (from left or from right)
 *	@return		int					Index of extremum
 *	@sa			TArray
*/
template <typename T, ExtremumType p_oExtremumType>
std::pair<int, int> calcExtremum (const geo2d::TArray<T> &p_rArrayIn, SearchDirType p_oDirection = eFromLeft) {
	poco_assert_dbg(p_rArrayIn.size() != 0);
    
    static_assert(p_oExtremumType != eZeroCrossing, "zero crossing at the moment is implemented only in  LineExtremumNumber::searchZeroCrossing");
    
	const std::vector<T> &rInData	= p_rArrayIn.getData();
	const std::vector<int> &rInRank	= p_rArrayIn.getRank();

	std::function<bool (T, T)>	oCompFtor		= std::greater<T>();	// comparison type functor object
	T								oExtremumValue	= 0;
	std::size_t						oExtremumIndex	= 0;

	if (p_oExtremumType == eMinimum) {
		oExtremumValue	= std::numeric_limits<T>::max();	// initialize current extremum
	}
	else if (p_oExtremumType == eMaximum) {
		oExtremumValue	= std::numeric_limits<T>::lowest();	// initialize current extremum
	} 
	else {
		wmLog( eDebug, "Invalid ExtremumType '%i'\n", p_oExtremumType);
	}
	
	auto fCompare = [&oExtremumValue](T in){
        //switch condtion known at compile time
        switch(p_oExtremumType)
        {
            case eMinimum: return  in < oExtremumValue;
            case eMaximum: return in > oExtremumValue;
            case eZeroCrossing: assert(false && "not implemented");  return false;
        }
    };

	if (p_oDirection == eFromLeft)
	{
		// search extremum from left to right
		for (std::size_t x = 0; x < rInData.size(); ++x) {
			// rank valid and new extremum found
			if ( rInRank[x] != eRankMin && fCompare(rInData[x]) )
			{
				oExtremumValue = rInData[x];
				oExtremumIndex = x;
			}
		}
	} else if (p_oDirection == eFromRight)
	{
		// search extremum from right to left
		for (std::size_t x = rInData.size(); x > 0; --x) {
			// rank valid and new extremum found
			if ( rInRank[x-1] != filter::eRankMin && fCompare(rInData[x-1]) )
			{
				oExtremumValue = rInData[x-1];
				oExtremumIndex = x-1;
			}
		}
	}
	else {
		wmLog( eDebug, "Invalid SearchDirType '%i'\n", p_oDirection);
	}

	return	std::make_pair(oExtremumIndex, rInRank[oExtremumIndex]);
} // calcExtremum


/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Calculates extremum of vector. Bad rank values are excluded from search.
 *	@param		p_rArrayIn			Input Array containing data and rank vector
 *	@param		p_oExtremumType		Type of extremum (minimum or maximum)
 *	@return		triplet<T Index, int rank, T index>
 *	@sa			TArray
*/
template <typename T>
std::tuple<int, int, T> calcValueExtremum (const geo2d::TArray<T> &p_rArrayIn, filter::ExtremumType p_oExtremumType, bool p_oDirection=true) {
	// direction: true = left to right, false = right to left
	poco_assert_dbg(p_rArrayIn.size() != 0);
	const std::vector<T> &rInData	= p_rArrayIn.getData();
	const std::vector<int> &rInRank	= p_rArrayIn.getRank();

	std::function<bool (T, T)>	oCompFtor		= std::greater<T>();	// comparison type functor object
	T								oExtremumValue	= 0;
	std::size_t						oExtremumIndex	= 0;

	if (p_oExtremumType == filter::eMinimum) {
		oCompFtor	= std::less<T>();						// comparison type functor object
		oExtremumValue	= std::numeric_limits<T>::max();	// initialize current extremum
	}
	else if (p_oExtremumType == filter::eMaximum) {
		oCompFtor	= std::greater<T>();					// comparison type functor object
		oExtremumValue	= std::numeric_limits<T>::lowest();	// initialize current extremum
	} 
	else {
		wmLog( eDebug, "Invalid ExtremumType" );
		assert(! "Invalid ExtremumType");
	}

	if (p_oDirection)
	{
		// search extremum from left to right
		for (std::size_t x = 0; x < rInData.size(); ++x) {
			// rank valid and new extremum found
			if ( rInRank[x] != filter::eRankMin && oCompFtor(rInData[x], oExtremumValue) )
			{
				oExtremumValue = rInData[x];
				oExtremumIndex = x;
			}
		}
	} else
	{
		// search extremum from right to left
		for (std::size_t x = rInData.size(); x > 0; --x) {
			// rank valid and new extremum found
			if ( rInRank[x-1] != filter::eRankMin && oCompFtor(rInData[x-1], oExtremumValue) )
			{
				oExtremumValue = rInData[x-1];
				oExtremumIndex = x-1;
			}
		}
	}

	return	std::make_tuple(oExtremumIndex, rInRank[oExtremumIndex], oExtremumValue);
} // calcExtremum


/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Calculates data minimum.
 *	@param		p_rArrayIn			Input Array containing data and rank vector
  *	@return		std::tuple<T, int>	Output minimum data (first) and its rank(second)
 *	@sa			TArray
*/
template <typename T>
std::tuple<T, int> calcDataMinimum (const geo2d::TArray<T> & p_rArrayIn) {

	std::tuple<int, int, T> temp = calcValueExtremum(p_rArrayIn,filter::eMinimum);

	int rankOfValue = std::get<1>(temp);
	T minValue = std::get<2>(temp);

	std::tuple<T, int> minimum = std::make_tuple(minValue,rankOfValue);

	return	minimum;
} // calcDataMinimum


/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Calculates data maximum.
 *	@param		p_rArrayIn			Input Array containing data and rank vector
  *	@return		std::tuple<T, int>	Output maximum data (first) and its rank(second)
 *	@sa			TArray
*/
template <typename T>
std::tuple<T, int> calcDataMaximum (const geo2d::TArray<T> & p_rArrayIn) {

	std::tuple<int, int, T> temp = calcValueExtremum(p_rArrayIn,filter::eMaximum);

	int rankOfValue = std::get<1>(temp);
	T maxValue = std::get<2>(temp);
	
	std::tuple<T, int> maximum = std::make_tuple(maxValue, rankOfValue);

	return	maximum;
} // calcDataMinimum

template <typename T>
std::tuple<T, int> calcStdDeviation (const geo2d::TArray<T> & p_rArrayIn)
{
    int counter = 0;
    double sum = 0;
    double sumSquare = 0;
    forEachValidDataInArray(p_rArrayIn, [&sum, &sumSquare, &counter](T value)
        {
            sum += value;
            sumSquare += (value*value);
            counter++;
        }
    );
    if (counter <= 1)
    {
        return {0.0, eRankMin};
    }
    auto mean = sum / counter;
    auto variance = (sumSquare - mean*sum) / (counter - 1);
    return {std::sqrt(variance), eRankMax};
}

/**
 *  @ingroup	AnalyzerInterface
 *  @brief		Causal finite impulse result(FIR) filter. Parameterized with a functor object that contains the actual low pass filter algorithm.
 *  @details 	Stateful function object, parameterized with filter function like calcWeightedMean or calcMedian.
 *				Uses a fixed length ringbuffer.
 *				Usage: LowPass oLowPass(...); std::tuple<int, int> oResult = oLowPass.process(oTupleIn); // two ints due to rank
 * 				Boundary treatment (distance to begin/end < filter length):	Identical to non-boundary treatment, using the internal buffer initialized with bad ranked values.
 *				Non-boundary treatment value: Median value is calculated.
 *				Non-boundary treatment rank: If the majority of ranks is bad, bad rank is set, else mean good rank is set. However, bad input rank is always passed through.
 */
template <typename T>
class LowPass {
public:
	/**
	 * @brief	Constructor for full initialization
	 * @param	p_oSize					Filter lenght for values. Must be greater zero.
	 * @param	p_oFtor					Filter algorithm to be applied.
	 * @param	p_oPassThroughBadRank	If bad ranked values are always passed through and not eliminated.
	*/
	LowPass(unsigned int p_oSize, std::function<std::tuple<T, int>(const geo2d::TArray<T>&)> p_oFtor, bool p_oPassThroughBadRank = false) : 
		m_oSizeFilter				(p_oSize), 
		m_oHalfSizeFilter			(m_oSizeFilter / 2),
		m_oAlgorithm				(p_oFtor),
		m_oPassThroughBadRank		(p_oPassThroughBadRank),
		m_oCounter					(0),
		m_oRingbuffer				(m_oSizeFilter)
	{
		// validate preconditions

		poco_assert_dbg(m_oSizeFilter != 0); // Parameter assertion. Should be pre-checked by UI / MMI / GUI.
		poco_assert_dbg(m_oAlgorithm != nullptr);
	} // LowPass



	/**
	 * @brief	Causal operation to be applied. E.g. N = 3, f[x] = (f[x] + f[x-1] + f[x-2]) / 3
	 * @param	p_oNewValue						Input value with rank.
	 * @return	std::tuple<T, int>				Output point with rank.
	*/
	std::tuple<T, int> process(std::tuple<T, int> p_oNewValue) {
		if (m_oSizeFilter == 1) {
			return p_oNewValue;  // handle degenerated filter case for filter size = 1
		}
		std::tuple<T, int>			oResult				{};			
		const unsigned int			oRingPosition		{ m_oCounter % m_oSizeFilter }; // get index for ring buffer
		
		int oRank = std::get<eRank>(p_oNewValue);

		m_oRingbuffer[oRingPosition]	= p_oNewValue; // copy element in buffer	
		

		//pass value if rank is bad ----- 
		if (m_oPassThroughBadRank == true && oRank==eRankMin)
		{ 

			int &dummyRank = std::get<eRank>(m_oRingbuffer[oRingPosition]);
			dummyRank = eRankMax;

			//oResult	= std::make_tuple(T{}, eRankMin); // default value with bad rank --- which default value ?? 		 
			oResult = m_oAlgorithm(m_oRingbuffer); // apply algorithm and store result	
			
  	    } 
		else //no difference between the cases
		{
			oResult	= m_oAlgorithm(m_oRingbuffer); // apply algorithm and store result	
		} 
		
		++m_oCounter; // overflow ok
		return oResult;
	} // process



	/**
	 * @brief	Reset ringbuffers.
	 * @return	void
	*/
	void resetBuffer() {
		m_oRingbuffer.reinitialize();
	} // resetBuffer


private:
	const unsigned int													m_oSizeFilter;			///< filter-size for values
	const unsigned int													m_oHalfSizeFilter;		///< half filter-size for  values
	const std::function<std::tuple<T, int>(const geo2d::TArray<T>&)>	m_oAlgorithm;			///< Causal algorithm to be applied on moving window
	const bool															m_oPassThroughBadRank;	///< if bad ranked values are always passed through and not eliminated
	
	unsigned int														m_oCounter;				///< counter
	geo2d::TArray<T>													m_oRingbuffer;			///< Ringbuffer for  values
}; // LowPass



/// UTILITY

/**
 *  @ingroup	AnalyzerInterface
 *	@brief		DEPRECATED, use assign directly. Reset out structure based on input dimension with zeros and bad rank.
 *	@param		p_rRefIn			Input Array as size reference for ouput (re)set
 *	@param		p_rOut				Output Array to be (re)set
 *	@return		void
 *	@sa			TArray
*/
template <typename T>
void resetFromInput(
		const geo2d::TArray<T>	&p_rRefIn,
		geo2d::TArray<T>		&p_rOut
	) {
	const unsigned int oSize		( p_rRefIn.size() ); 
	p_rOut.assign(oSize, T(), eRankMin);
} // resetFromInput

/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Reset out structure based on input dimension with zeros and bad rank.
 *	@param		p_rRefIn			Input vector of Array as size reference for ouput (re)set
 *	@param		p_rOut				Output vector of Array to be (re)set
 *	@return		void
 *	@sa			TArray
*/
template <typename T>
void resetFromInput(
		const std::vector<geo2d::TArray<T>>	&p_rRefIn,
		std::vector<geo2d::TArray<T>>		&p_rOut
	) {
	poco_assert_dbg( ! p_rRefIn.empty());
	const auto oNbArrays	= p_rRefIn.size(); 
	const auto oDataLenght	= p_rRefIn.front().size(); // empty vector not permitted - see above

	p_rOut.resize(oNbArrays);
	std::for_each( p_rOut.begin(), p_rOut.end(),  std::bind(&geo2d::TArray<T>::assign, std::placeholders::_1/*this*/, oDataLenght, T(), eRankMin) );
} // resetFromInput



/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Determines if the content of a TArray<T> is invalid (bad rank, empty data, inconsistent data).
 *	@param		p_rDataIn				Input data
 *	@return		bool					If input is invalid.
 *	@sa			TArray, overloads of inputIsInvalid
*/
template <typename T>
bool inputIsInvalid(const geo2d::TArray<T> &p_rDataIn) {
	const std::vector<T>	&rDataIn = p_rDataIn.getData();
	const std::vector<int>	&rRankIn = p_rDataIn.getRank();

	const bool	oAllBadRank		= std::all_of(rRankIn.begin(), rRankIn.end(), [] (int p_rRank) { return p_rRank == eRankMin; } );
	const bool	oDataInEmpty	= rDataIn.empty();
	if (oDataInEmpty) {
		wmLog( eDebug, "Input vector is empty.\n" );
	}

	return oAllBadRank || oDataInEmpty;
} // inputIsInvalid

template <typename T>
bool inputIsInvalid(const geo2d::TAnnotatedArray<T> &p_rDataIn) {
	const std::vector<T>	&rDataIn = p_rDataIn.getData();
	const std::vector<int>	&rRankIn = p_rDataIn.getRank();

	const bool	oAllBadRank		= std::all_of(rRankIn.begin(), rRankIn.end(), [] (int p_rRank) { return p_rRank == eRankMin; } );
	const bool	oDataInEmpty	= rDataIn.empty();
	if (oDataInEmpty) {
		wmLog( eDebug, "Input vector is empty.\n" );
	}

	return oAllBadRank || oDataInEmpty;
} // inputIsInvalid


/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Determines if the content of a TGeo<TArray<T>> is invalid (bad geo rank, empty data, inconsistent data). Wrapper to TGeo<std::vector<TArray<T>>>.
 *	@param		p_rGeoDataIn			Input data
 *	@return		bool					If input is invalid.
 *	@sa			TArray, TGeo, overloads of inputIsInvalid
*/
template <typename T>
bool inputIsInvalid(const interface::TGeo<geo2d::TArray<T>> &p_rGeoDataIn) {
	const bool			oGeoRankNull		= p_rGeoDataIn.rank() == 0.0;
	if (oGeoRankNull) {
		wmLog( eDebug, "Input geo rank is bad.\n" );
	}
	return oGeoRankNull || inputIsInvalid(p_rGeoDataIn.ref());
} // inputIsInvalid

template <typename T>
bool inputIsInvalid(const interface::TGeo<geo2d::TAnnotatedArray<T>> &p_rGeoDataIn) {
	const bool			oGeoRankNull		= p_rGeoDataIn.rank() == 0.0;
	if (oGeoRankNull) {
		wmLog( eDebug, "Input geo rank is bad.\n" );
	}
	return oGeoRankNull || inputIsInvalid(p_rGeoDataIn.ref());
} // inputIsInvalid



/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Determines if the content of a std::vector<TArray<T>> is invalid (bad rank, empty data, inconsistent data).
 *	@param		p_rDataIn			Input data
 *	@return		bool				If input is invalid.
 *	@sa			overloads of inputIsInvalid
*/
template <typename T>
bool inputIsInvalid(const std::vector<geo2d::TArray<T>> &p_rVecArrayIn) {
	
	if (p_rVecArrayIn.size() <= 0)
	{
		return true;
	}
	//invalid only if all the profiles are invalid  (asserting that all profiles are valid would completely discard the stack of lines)
	return std::all_of(p_rVecArrayIn.begin(), p_rVecArrayIn.end(), [](const geo2d::TArray<T> &rArray) { return inputIsInvalid(rArray);} );
	
} // inputIsInvalid

template <typename T>
bool inputIsInvalid(const std::vector<geo2d::TAnnotatedArray<T>> &p_rVecArrayIn) {

	if (p_rVecArrayIn.size() <= 0)
	{
		return true;
	}
	//invalid only if all the profiles are invalid  (asserting that all profiles are valid would completely discard the stack of lines)
	return std::all_of(p_rVecArrayIn.begin(), p_rVecArrayIn.end(), [](const geo2d::TAnnotatedArray<T> &rArray) { return inputIsInvalid(rArray);} );

} // inputIsInvalid

/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Determines if the content of a TGeo<std::vector<TArray<T>>> is invalid (bad geo rank, empty data, inconsistent data).
 *	@param		p_rGeoDataIn			Input data
 *	@return		bool					If input is invalid.
 *	@sa			TArray, TGeo, overloads of inputIsInvalid
*/
template <typename T>
bool inputIsInvalid(const interface::TGeo<std::vector<geo2d::TArray<T>>> &p_rGeoDataIn) {
	const bool			oGeoRankNull		= p_rGeoDataIn.rank() == interface::NotPresent;
	if (oGeoRankNull) {
		wmLog( eDebug, "Input geo rank is bad.\n" );
	}
	return oGeoRankNull || inputIsInvalid(p_rGeoDataIn.ref());
} // inputIsInvalid

template <typename T>
bool inputIsInvalid(const interface::TGeo<std::vector<geo2d::TAnnotatedArray<T>>> &p_rGeoDataIn) {
	const bool			oGeoRankNull		= p_rGeoDataIn.rank() == interface::NotPresent;
	if (oGeoRankNull) {
		wmLog( eDebug, "Input geo rank is bad.\n" );
	}
	return oGeoRankNull || inputIsInvalid(p_rGeoDataIn.ref());
} // inputIsInvalid

/** 
 *  @ingroup	AnalyzerInterface
 *	@brief		Get a coordinate out of a point array converted to a double arrray.
 *	@param		p_rPoints				Array of 2d points.
 *	@param		p_oCoordinateType		Coordinate to be extracted (x or y).
 *	@return		Doublearray	1d double arrray.
*/
inline geo2d::Doublearray getCoordinate(const geo2d::Pointarray &p_rPoints, filter::CoordinateType p_oCoordinateType) {
	const std::size_t	oSize				( p_rPoints.size() );
	geo2d::Doublearray	oDoublearray		( oSize );
	auto				oItDataPoints		( p_rPoints.getData().cbegin() );	
	auto				oItRankPoints		( p_rPoints.getRank().cbegin() );
	auto				oItDataDoubles		( oDoublearray.getData().begin() );	
	auto				oItRankDoubles		( oDoublearray.getRank().begin() );
	for (std::size_t i (0); i < oSize; ++i) { // loop over all elements	
		if (p_oCoordinateType == filter::eX) {
			*oItDataDoubles	= oItDataPoints->x;
		} // if
		else if (p_oCoordinateType == filter::eY) {
			*oItDataDoubles	= oItDataPoints->y;
		} // else if
		else {
			std::ostringstream	oMsg;
			oMsg << "Invalid CoordinateType: " << p_oCoordinateType << '\n';
			wmLog( eDebug, oMsg.str() );
		} // else
		*oItRankDoubles	= *oItRankPoints;

		++oItDataPoints;
		++oItRankPoints;
		++oItDataDoubles;
		++oItRankDoubles;
	} // for
	return oDoublearray;
} // getCoordinate


inline geo2d::Doublearray getCoordinate(const geo2d::DPointarray &p_rPoints, filter::CoordinateType p_oCoordinateType) {
	const std::size_t	oSize(p_rPoints.size());
	geo2d::Doublearray	oDoublearray(oSize);
	auto				oItDataPoints(p_rPoints.getData().cbegin());
	auto				oItRankPoints(p_rPoints.getRank().cbegin());
	auto				oItDataDoubles(oDoublearray.getData().begin());
	auto				oItRankDoubles(oDoublearray.getRank().begin());
	for (std::size_t i(0); i < oSize; ++i) { // loop over all elements
		if (p_oCoordinateType == filter::eX) {
			*oItDataDoubles = oItDataPoints->x;
		} // if
		else if (p_oCoordinateType == filter::eY) {
			*oItDataDoubles = oItDataPoints->y;
		} // else if
		else {
			std::ostringstream	oMsg;
			oMsg << "Invalid CoordinateType: " << p_oCoordinateType << '\n';
			wmLog(eDebug, oMsg.str());
		} // else
		*oItRankDoubles = *oItRankPoints;

		++oItDataPoints;
		++oItRankPoints;
		++oItDataDoubles;
		++oItRankDoubles;
	} // for
	return oDoublearray;
} // getCoordinate


inline geo2d::Doublearray getCoordinate(const geo2d::AnnotatedDPointarray &p_rPoints, filter::CoordinateType p_oCoordinateType) {
	const std::size_t	oSize(p_rPoints.size());
	geo2d::Doublearray	oDoublearray(oSize);
	auto				oItDataPoints(p_rPoints.getData().cbegin());
	auto				oItRankPoints(p_rPoints.getRank().cbegin());
	auto				oItDataDoubles(oDoublearray.getData().begin());
	auto				oItRankDoubles(oDoublearray.getRank().begin());
	for (std::size_t i(0); i < oSize; ++i) { // loop over all elements	
		if (p_oCoordinateType == filter::eX) {
			*oItDataDoubles = oItDataPoints->x;
		} // if
		else if (p_oCoordinateType == filter::eY) {
			*oItDataDoubles = oItDataPoints->y;
		} // else if
		else {
			std::ostringstream	oMsg;
			oMsg << "Invalid CoordinateType: " << p_oCoordinateType << '\n';
			wmLog(eDebug, oMsg.str());
		} // else
		*oItRankDoubles = *oItRankPoints;

		++oItDataPoints;
		++oItRankPoints;
		++oItDataDoubles;
		++oItRankDoubles;
	} // for
	return oDoublearray;
} // getCoordinate



/** 
 *  @ingroup	AnalyzerInterface
 *	@brief		Converts an array of data type int to  an array of data type double.
 *	@param		p_rInput				Input array.
 *	@return		geo2d::Doublearray		Output array.
*/
inline geo2d::Doublearray intToDouble(const geo2d::Intarray &p_rInput) {
	geo2d::Doublearray	oOutput;
	oOutput.getData()	= intToDouble( p_rInput.getData() );
	oOutput.getRank()	= p_rInput.getRank();
	return oOutput;
} // intToDouble



/** 
 *  @ingroup	AnalyzerInterface
 *	@brief		Converts an array of data type double to  an array of data type int.
 *	@param		p_rInput				Input array.
 *	@return		geo2d::Intarray			Output array.
*/
inline geo2d::Intarray doubleToInt(const geo2d::Doublearray &p_rInput) {
	geo2d::Intarray	oOutput;
	oOutput.getData()	= doubleToInt( p_rInput.getData() );
	oOutput.getRank()	= p_rInput.getRank();
	return oOutput;
} // doubleToInt



/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Calculates position of first value with value not equal to min rank.
 *	@param		p_rArrayIn			Input Array containing data and rank vector
 *	@return		std::size_t			Position of first value with value not equal to min rank
*/
template <typename T>
std::size_t getFirstValidIndex (const geo2d::TArray<T> &p_rArrayIn) {
	poco_assert(p_rArrayIn.size() != 0);
	const std::vector<int> &rInRank	= p_rArrayIn.getRank();
	assert( ! rInRank.empty() );

	// get position of first value with value not equal to min rank
	const auto oItFirstGood = std::find_if(
		std::begin(rInRank),
		std::end(rInRank),
		std::bind(std::not_equal_to<int>(), eRankMin, std::placeholders::_1)
	);
	return std::distance(std::begin(rInRank), oItFirstGood);
} // getFirstValidIndex



/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Calculates position of last value with value not equal to min rank.
 *	@param		p_rArrayIn			Input Array containing data and rank vector
 *	@return		std::size_t			Position of last value with value not equal to min rank
*/
template <typename T>
std::size_t getLastValidIndex (const geo2d::TArray<T> &p_rArrayIn) {
	poco_assert(p_rArrayIn.size() != 0);
	const std::vector<int> &rInRank	= p_rArrayIn.getRank();
	assert( ! rInRank.empty() );

	// get position of last value with value not equal to min rank
	const auto oItLastGood = std::find_if(
		rInRank.rbegin(),
		rInRank.rend(),
		std::bind(std::not_equal_to<int>(), eRankMin, std::placeholders::_1)
	);
	return std::distance(oItLastGood, (rInRank.rend() - 1));
} // getLastValidIndex


/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Calculates position of first value with value not equal to min rank.
 *	@param		p_rArrayIn			Input Array containing data and rank vector
 *	@return		std::size_t			Position of first value with value not equal to min rank
*/
template <typename T>
std::size_t getFirstInvalidIndex (const geo2d::TArray<T> &p_rArrayIn) {
	poco_assert(p_rArrayIn.size() != 0);
	const std::vector<int> &rInRank	= p_rArrayIn.getRank();
	assert( ! rInRank.empty() );

	// get position of first value with value equal to min rank
	const auto oItFirstBad = std::find_if(
		std::begin(rInRank),
		std::end(rInRank),
		std::bind(std::equal_to<int>(), eRankMin, std::placeholders::_1)
	);
	return std::distance(std::begin(rInRank), oItFirstBad );
} // getFirstValidIndex



/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Calculates position of last value with value not equal to min rank.
 *	@param		p_rArrayIn			Input Array containing data and rank vector
 *	@return		std::size_t			Position of last value with value not equal to min rank
*/
template <typename T>
std::size_t getLastInvalidIndex (const geo2d::TArray<T> &p_rArrayIn) {
	poco_assert(p_rArrayIn.size() != 0);
	const std::vector<int> &rInRank	= p_rArrayIn.getRank();
	assert( ! rInRank.empty() );

	// get position of last value with value equal to min rank
	const auto oItLastBad = std::find_if(
		rInRank.rbegin(),
		rInRank.rend(),
		std::bind(std::equal_to<int>(), eRankMin, std::placeholders::_1)
	);
	return std::distance( oItLastBad, (rInRank.rend() - 1));
} // getLastValidIndex


/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Checks if both indices are valid for the given array.
 *	@param		p_rArrayIn			Input Array containing data and rank vector
 *	@param		p_oStart			Start index to be checked.
 *	@param		p_oEnd				End index to be checked.
 *	@return		bool				If both indices are valid
*/
template <typename T>
bool checkIndices (const geo2d::TArray<T> &p_rArrayIn, int p_oStart, int p_oEnd) {
	return checkIndices(p_rArrayIn.getData(), p_oStart, p_oEnd);
} // checkIndices


/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Finds the [start,end) indexes of the first range of valid data between starting from startSearch index
 *	@param		rLine			Input Array containing data and rank vector
 *	@param		startSearch			Start Index for search
 *	@return		pair(validStart, validEnd). validEnd is the index after the last valid element of the first valid range (only considering the search range). if no valid element was found, validEnd is equal to rLine.size()
*/

template <class isValidFunction>
inline std::pair<std::size_t, std::size_t> searchFirstValidArrayRange (const std::vector<int> & rRank, std::size_t startSearch, isValidFunction isValid)
{
    assert(startSearch <= rRank.size());

    auto itValidRankBegin = std::find_if(rRank.begin() + startSearch, rRank.end(), isValid);
    auto itValidRankEnd = std::find_if_not(itValidRankBegin, rRank.end(), isValid);

    std::pair<std::size_t, std::size_t> result
            {std::distance(rRank.begin(), itValidRankBegin),
            std::distance(rRank.begin(), itValidRankEnd)};

    return result;
}

inline std::pair<std::size_t, std::size_t> searchFirstValidArrayRange (const precitec::geo2d::Doublearray & rLine, std::size_t startSearch)
{
    static const auto isValid = [](int rankElement ){ return rankElement != eRankMin;};

    return searchFirstValidArrayRange (rLine.getRank(), startSearch, isValid);
}


/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Finds the minimum and maximum value (having rank!=0) in a double array
 *	@param		rLine			Input Array containing data and rank vector
 *	@return		std::tuple<bool, double, double> tuple(initialized, yMin, yMax)
*/
inline std::tuple<bool, double, double> min_maxValidArrayElement(const precitec::geo2d::Doublearray & rLine)
{
    double yMin = 0.0;
    double yMax = yMin;
    bool initialized = false;

    auto lineLength = rLine.size();
    std::size_t xValidEnd = 0;
    std::size_t xValidStart = xValidEnd;
    while( xValidStart < lineLength)
    {
        std::tie( xValidStart, xValidEnd ) = searchFirstValidArrayRange (rLine, xValidStart );
        if ( xValidStart >= lineLength)
        {
            break;
        }

        auto bounds = std::minmax_element(rLine.getData().begin() + xValidStart, rLine.getData().begin() + xValidEnd );
        if (!initialized)
        {
            yMin = *bounds.first;
            yMax = *bounds.second;
            initialized = true;
        }

        yMin = yMin < *bounds.first ? yMin : *bounds.first;
        yMax = yMax > *bounds.second ? yMax : *bounds.second;

        xValidStart = xValidEnd;
    }
    return std::tuple<bool, double, double>{initialized, yMin, yMax};
}



/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Finds the minimum and maximum value (having rank!=0) in a VecDoubleArray
 *	@param		rLine Vector of	Input Array containing data and rank vector
 *	@return		std::tuple<bool, double, double> tuple(initialized, yMin, yMax)
*/
inline std::tuple<bool, double, double> min_maxValidVectorArrayElement(const std::vector<precitec::geo2d::Doublearray> & rLines)
{
    double yMin = 0.0;
    double yMax = yMin;
    const unsigned int oNbLines = rLines.size();
    bool initialized = false;

    for (unsigned int lineN = 0; lineN < oNbLines; lineN ++ )
    {
        bool hasData;
        double yLineMin, yLineMax;

        std::tie( hasData, yLineMin, yLineMax) = min_maxValidArrayElement(rLines[lineN]);
        if ( hasData )
        {
            if (!initialized)
            {
                yMin = yLineMin;
                yMax = yLineMax;
                initialized = true;
            }
            yMin = yMin < yLineMin ? yMin : yLineMin;
            yMax = yMax > yLineMax? yMax : yLineMax;
        }
    }
    return std::tuple<bool, double, double>{initialized, yMin, yMax};
}
/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Apply a different function on valid and invalid elements of a DoubleArray and store the result in another range, beginning at itDest.
 *	@param		rLine Vector of	Input Array containing data and rank vector
 *	@param		itDest Destination iterator
 *	@param		fOnValidElements Unary Function on data elements whose corresponding rank is valid
 *	@param		fOnInvalidElements Unary Function on data elements whose corresponding rank is invalid
*/
template <typename UnaryFunctionOnValid, typename UnaryFunctionOnInValid, typename DestIterator>
void transformArray(const precitec::geo2d::Doublearray & rLine,  DestIterator itDest,
    UnaryFunctionOnValid fOnValidElements, UnaryFunctionOnInValid fOnInvalidElements )
{
    auto isValidRank = [](int rank){ return rank!=0.0;};
    auto itValidRankBegin = rLine.getRank().begin();
    auto itValidRankEnd = itValidRankBegin;

    int xStartInvalid = 0;

    //process the array block-wise (in each iteration: a block of invalid elements followed by a block of valid elements)
    while( itValidRankBegin != rLine.getRank().end())
    {
        // find limit of current block (invalid elements followed by valid elements)
        itValidRankBegin = std::find_if(itValidRankBegin, rLine.getRank().end(),isValidRank );
        itValidRankEnd = std::find_if(itValidRankBegin, rLine.getRank().end(), [&isValidRank](int rank){ return !isValidRank(rank);});

        int xStartValid = itValidRankBegin - rLine.getRank().begin();
        int xEndValid = itValidRankEnd - rLine.getRank().begin();

        //sanity checks in debug build: the block is defined in this order xStartInValid -> xStartValid -> xEndValid
        assert(xStartInvalid <= xStartValid);
        assert(xStartValid <= xEndValid);

        assert(xStartInvalid == xStartValid || rLine.getRank()[xStartInvalid] == 0);
        assert(xStartValid == 0 || rLine.getRank()[xStartValid -1] == 0);
        assert(xStartValid == (int)rLine.getRank().size() || rLine.getRank()[xStartValid] != 0);

        // apply functions
        std::transform(rLine.getData().begin() + xStartInvalid, rLine.getData().begin() + xStartValid, itDest + xStartInvalid, fOnInvalidElements);
        std::transform(rLine.getData().begin() + xStartValid, rLine.getData().begin() + xEndValid, itDest + xStartValid , fOnValidElements);

        //update indexes for next iteration
        xStartInvalid = xEndValid;
        itValidRankBegin = itValidRankEnd;
    }
}



/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Apply a different function on valid and invalid elements (data and rank) of a range in a DoubleArray.
 *              If the same operation needs to be applied to valid and invalid elements, forEachInArrayRange_RankIndepent is faster
 *	@param		rLine Vector of	Input Array containing data and rank vector
  *	@param		indexStart start index of the range to process
  *	@param		indexEnd end index of the range to process (excluded)
 *	@param		fOnValidData Unary Function on data elements whose corresponding rank is valid
 *	@param		fOnValidRank Unary Function on valid rank elements
 *	@param		fOnInvalidData Unary Function on data elements whose corresponding rank is invalid
 *	@param		fOnInvalidRank Unary Function on invalid rank elements
*/
template <typename UnaryFunctionOnValidData, typename UnaryFunctionOnInValidData, typename UnaryFunctionOnValidRank, typename UnaryFunctionOnInValidRank>
void forEachInArrayRange(const precitec::geo2d::Doublearray & rLine, unsigned int indexStart, unsigned int indexEnd,
                     UnaryFunctionOnValidData fOnValidData, UnaryFunctionOnValidRank fOnValidRank,
                     UnaryFunctionOnInValidData fOnInvalidData, UnaryFunctionOnInValidRank fOnInvalidRank )
{
    auto isValidRank = [](int rank){ return rank != eRankMin;};

    assert(indexEnd <= rLine.getRank().size());
    auto itEndRank = rLine.getRank().begin() + indexEnd;

    auto itValidRankBegin = rLine.getRank().begin() + indexStart;
    auto itValidRankEnd = itValidRankBegin;

    unsigned int xStartInvalid = indexStart;

    //process the array block-wise (in each iteration: a block of invalid elements followed by a block of valid elements)
    while( itValidRankBegin != itEndRank)
    {
        // find limit of current block (invalid elements followed by valid elements)
        itValidRankBegin = std::find_if(itValidRankBegin, itEndRank, isValidRank );
        itValidRankEnd = std::find_if(itValidRankBegin, itEndRank, [&isValidRank](int rank) {return !isValidRank(rank);});

        auto xStartValid = itValidRankBegin - rLine.getRank().begin();
        auto xEndValid = itValidRankEnd - rLine.getRank().begin();

        //sanity checks in debug build: the block is defined in this order xStartInValid -> xStartValid -> xEndValid
        assert(xStartInvalid >= indexStart);
        assert(xStartValid >= indexStart);

        assert(xStartInvalid <= xStartValid);
        assert(xStartValid <= xEndValid);

        assert(xStartInvalid == xStartValid || rLine.getRank()[xStartInvalid] == 0);
        assert(xStartValid == (int)indexStart || *(itValidRankBegin-1) == 0);
        assert(xStartValid == (int)indexEnd || rLine.getRank()[xStartValid] != 0);


        // apply functions
        std::for_each(rLine.getData().begin() + xStartInvalid, rLine.getData().begin() + xStartValid, fOnInvalidData);
        std::for_each(rLine.getRank().begin() + xStartInvalid, rLine.getRank().begin() + xStartValid, fOnInvalidRank);

        std::for_each(rLine.getData().begin() + xStartValid, rLine.getData().begin() + xEndValid, fOnValidData);
        std::for_each(itValidRankBegin, itValidRankEnd, fOnValidRank);

        //update indexes for next iteration
        xStartInvalid = xEndValid;
        itValidRankBegin = itValidRankEnd;
    }
}

/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Apply a different function on valid and invalid elements (data and rank) of a DoubleArray.
 *              If the same operation needs to be applied to valid and invalid elements, forEachInArray_RankIndepent is faster
 *	@param		rLine Vector of	Input Array containing data and rank vector
 *	@param		fOnValidData Unary Function on data elements whose corresponding rank is valid
 *	@param		fOnValidRank Unary Function on valid rank elements
 *	@param		fOnInvalidData Unary Function on data elements whose corresponding rank is invalid
 *	@param		fOnInvalidRank Unary Function on invalid rank elements
*/
template <typename UnaryFunctionOnValidData, typename UnaryFunctionOnInValidData, typename UnaryFunctionOnValidRank, typename UnaryFunctionOnInValidRank>
void forEachInArray(const precitec::geo2d::Doublearray & rLine,
                     UnaryFunctionOnValidData fOnValidData, UnaryFunctionOnValidRank fOnValidRank,
                     UnaryFunctionOnInValidData fOnInvalidData, UnaryFunctionOnInValidRank fOnInvalidRank )
{
    forEachInArrayRange(rLine, 0, rLine.size(), fOnValidData, fOnValidRank, fOnInvalidData, fOnInvalidRank);
}


template <typename UnaryFunctionOnValidData>
void forEachValidDataInArray(const precitec::geo2d::Doublearray & rLine, UnaryFunctionOnValidData fOnValidData)
{
    auto ignoreData = [](double){};
    auto ignoreRank = [](int){};
    forEachInArrayRange(rLine, 0, rLine.size(), fOnValidData, ignoreRank, ignoreData, ignoreRank);
}


/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Apply a function on elements (data and rank) of a range in DoubleArray. If the rank needs to be checked, use forEachInArrayRange
 *	@param		rLine Vector of	Input Array containing data and rank vector
 *	@param		fOnData Unary Function on data elements whose corresponding rank is valid
 *	@param		fOnRank Unary Function on valid rank elements
*/
template <typename UnaryFunctionOnData, typename UnaryFunctionOnRank>
void forEachInArrayRange_RankIndepent(const precitec::geo2d::Doublearray & rLine, unsigned int indexStart, unsigned int indexEnd,
                     UnaryFunctionOnData fOnData, UnaryFunctionOnRank fOnRank )
{
    assert(indexEnd <= rLine.size());
    std::for_each(rLine.getData().begin() + indexStart, rLine.getData().begin() + indexEnd, fOnData );
    std::for_each(rLine.getRank().begin() + indexStart, rLine.getRank().begin() + indexEnd, fOnRank );
}


/**
 *  @ingroup	AnalyzerInterface
 *	@brief		Apply a function on elements (data and rank) of a DoubleArray. If the rank needs to be checked, use forEachInArrayRange
 *	@param		rLine Vector of	Input Array containing data and rank vector
 *	@param		fOnData Unary Function on data elements whose corresponding rank is valid
 *	@param		fOnRank Unary Function on valid rank elements
*/
template <typename UnaryFunctionOnData, typename UnaryFunctionOnRank>
void forEachInArray_RankIndepent(const precitec::geo2d::Doublearray & rLine,
                     UnaryFunctionOnData fOnData, UnaryFunctionOnRank fOnRank)
{
    forEachInArrayRange_RankIndepent(rLine, 0, rLine.size(), fOnData, fOnRank);
}


inline bool processEntireProfileLines(size_t numberLines, size_t arraySize, const std::string& filterName, const std::string& label)
{
        if (numberLines == arraySize)
        {
            return true;
        }
        if (arraySize != 1 && !(label.empty() || filterName.empty()))
        {
            wmLog(eDebug, "Filter '%s': Received %u %s values, expected %d. Only process first element, rest will be discarded.\n",
                  filterName.c_str(), arraySize, label.c_str(), numberLines);
        }
        return false;
}


} // namespace filter
} // namespace precitec


#endif // ALGOARRAY_H_20110928
