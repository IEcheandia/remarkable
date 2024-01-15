
/**
 *	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2011
 *  @brief			Causal finite impulse result(FIR) filter. Parametrized with a functor object that contains the actual filter algorithm.
 */


#ifndef MOVINGWINDOW_H_20111021
#define MOVINGWINDOW_H_20111021


#include <vector>					///< vector
#include <tuple>					///< tuple
#include <functional>				///< function
#include <cassert>					///< assert

#include "system/types.h"			///< byte type
#include "module/moduleLogger.h"	///< wmLog

#include "filter/parameterEnums.h"	///< enum ValueRankType

#include "geo/array.h"				///< TArray
#include "algoStl.h"        //calcMedian1d


namespace precitec {
namespace filter {


/**
 * @brief	Causal finite impulse result(FIR) filter. Parametrized with a functor object that contains the actual filter algorithm.
 * @details Moving window stateful functor, parametrized with filter function like calcMean or calcMedian. Uses a fixed length ringbuffer. Processes boundary values.
 *			Moving average filters are normally used as low pass filters. 
 *			Usage: MovingWindow<int> oMovingWindow(...); oMovingWindow.processCentric(oLineIn, oLineOut);
*/
template <typename T>
class MovingWindow {
public:
	/**
	 * @brief	Constructor for full initialization
	 * @param	p_oSize					Filter lenght. Must be greater zero.
	 * @param	p_oFtor					Filter algorithm to be applied.
	 * @param	p_oPassThroughBadRank	If bad ranked values are always passed through and not eliminated.
	*/
	MovingWindow(unsigned int	p_oSize, std::function<std::tuple<T, int>(const geo2d::TArray<T>&)>	p_oFtor, bool p_oPassThroughBadRank = false) : 
		m_oSizeFilter				(p_oSize), 
		m_oHalfSizeFilter			(m_oSizeFilter / 2),
		m_oAlgorithm				(p_oFtor),
		m_oPassThroughBadRank		(p_oPassThroughBadRank),
		m_oRingbuffer				(m_oSizeFilter)
	{
		// validate preconditions

		poco_assert_dbg(m_oSizeFilter != 0 && "Filter length is zero."); // Parameter assertion. Should be pre-checked by UI / MMI / GUI.
		poco_assert_dbg(m_oAlgorithm != nullptr);
	}

	/**
	 * @brief	Prohibit assignment operator due to const members.
	*/
    MovingWindow& operator=(const MovingWindow&) = delete;

	/**
	 * @brief	Causal operation to be applied. E.g. N = 3, f[x] = (f[x] + f[x-1] + f[x-2]) / 3
	 * @param	p_rInput	Input line.
	 * @param	p_rOutput	Output line.
	 * @return	void
	*/
	void process(
	const geo2d::TArray<T>						&p_rInput,		
	geo2d::TArray<T>							&p_rOutput) 
	{
		const unsigned int oNbElements		= p_rInput.size(); // get number of elements
		poco_assert_dbg(oNbElements >= m_oSizeFilter);	// Parameter assertion. Should be pre-checked by UI / MMI / GUI.
		poco_assert_dbg(p_rOutput.size() >= oNbElements && "Output array length >= input array length."); // ok: asserted by initialization of p_rLineOut
		if (m_oSizeFilter < oNbElements == false) {
			wmLog(eWarning, "Window lenght (%u) must be smaller than line lenght (%u). Abort inspection.", m_oSizeFilter, oNbElements);
			return;
		} // if
		if (m_oSizeFilter == 1) {
			p_rOutput = p_rInput; // handle degenerated filter case for filter size = 1
			return;
		}
		for (unsigned int oElementN = 0; oElementN != oNbElements; ++oElementN) {
			const unsigned int	oRingPosition	= oElementN % m_oSizeFilter; // get index for ring buffer
			m_oRingbuffer[oRingPosition]		= p_rInput[oElementN]; // copy element in buffer		
			p_rOutput[oElementN]				= m_oAlgorithm(m_oRingbuffer); // apply algorithm and store result					
		} // for
		m_oRingbuffer.assign(m_oSizeFilter, 0, eRankMin); // reset buffer
	} // process



	/**
	 * @brief	Non-causal operation to be applied. E.g. N = 3, f[x] = (f[x+1] + f[x] + f[x-1]) / 3
	 * @details	Boundary treatment (distance to begin/end < filter length):	Input values and rank are copied.
	 *			Non-boundary treatment value: Median value is calculated.
	 *			Non-boundary treatment rank: If the majority of ranks is bad, bad rank is set, else mean good rank is set. However, bad input rank is always passed through.
	 * @param	p_rInput	Input line.
	 * @param	p_rOutput	Output line.
	 * @return	void
	*/
	void processCentric(const geo2d::TArray<T> &p_rInput, geo2d::TArray<T> &p_rOutput) {

		poco_assert_dbg(p_rOutput.size() >= p_rInput.size() && "Output array length >= input array length."); // ok: asserted by initialization of p_rLineOut
		
		if (m_oSizeFilter <= 1 || p_rInput.size() < m_oSizeFilter) 
        {
			p_rOutput = p_rInput; // handle degenerated filter case for filter size = 1
			return;
		}
        p_rOutput.resize(p_rInput.size());

		// process start boundary values (including bad values in buffer)
		const auto					oZeroElement		( std::make_tuple(T(), eRankMin) ); // default value with bad rank used to fill buffer
		unsigned int				oCurrentPos			( 0 );
		for (oCurrentPos = 0; oCurrentPos != m_oHalfSizeFilter; ++oCurrentPos) 
        {
			const unsigned int	oRingPosition	= oCurrentPos % m_oSizeFilter; // get index for ring buffer
			assert(oRingPosition == oCurrentPos && "ringbuffer too short");
			m_oRingbuffer[oRingPosition]		= p_rInput[oCurrentPos]; // copy element in buffer
		} // for
		// process values except boundaries 

		auto itOutData = p_rOutput.getData().begin() + oCurrentPos - m_oHalfSizeFilter;
		auto itOutRank = p_rOutput.getRank().begin() + oCurrentPos - m_oHalfSizeFilter;
        
		const unsigned int oNbElements		= p_rInput.size(); // get number of elements
		auto						oItInRank			( std::begin(p_rInput.getRank()) );
		for (; oCurrentPos != oNbElements ; 
             ++oCurrentPos, ++oItInRank, ++itOutData, ++itOutRank) 
        {
			const unsigned int	oRingPosition			= oCurrentPos % m_oSizeFilter; // get index for ring buffer
			m_oRingbuffer[oRingPosition]				= p_rInput[oCurrentPos]; // copy element in buffer	
			std::tie(*itOutData, *itOutRank)  = m_oAlgorithm(m_oRingbuffer); // apply algorithm and store result
			
			if (m_oPassThroughBadRank && *oItInRank == eRankMin) 
            { 
				std::tie(*itOutData, *itOutRank) = oZeroElement;
			} //

			assert(p_rOutput[oCurrentPos - m_oHalfSizeFilter] == std::tie(*itOutData, *itOutRank) );

		} // for
		// process end boundary values (including bad values in buffer)

		for (; oCurrentPos != oNbElements + m_oHalfSizeFilter; 
             ++oCurrentPos, ++oItInRank, ++itOutData, ++itOutRank) 
        {
			const unsigned int	oRingPosition			= oCurrentPos % m_oSizeFilter; // get index for ring buffer
			m_oRingbuffer[oRingPosition]				= oZeroElement; // copy element in buffer		
			
			std::tie(*itOutData, *itOutRank)  = m_oAlgorithm(m_oRingbuffer); // apply algorithm and store result
				
			if (m_oPassThroughBadRank  && *oItInRank == eRankMin) 
            { 
				std::tie(*itOutData, *itOutRank) = oZeroElement;
			} // if
            assert(p_rOutput[oCurrentPos - m_oHalfSizeFilter] == std::tie(*itOutData, *itOutRank) );

		} // for

		m_oRingbuffer.reinitialize(); // reset buffer
	} // processCentric


    
	template<bool t_oPassThroughBadRank>
	static void movingMedian(const geo2d::TArray<T> &p_rInput, geo2d::TArray<T> &p_rOutput, std::vector<T> & rValuesInWindow)
    {
        int p_oSizeFilter = rValuesInWindow.size();

        // 1) check input
        p_rOutput.resize(p_rInput.size());

        poco_assert_dbg(p_rOutput.size() >= p_rInput.size() && "Output array length >= input array length."); // ok: asserted by initialization of p_rLineOut
		
		if (p_oSizeFilter <= 1 || (int)p_rInput.size() < p_oSizeFilter) 
        {
			p_rOutput = p_rInput; // handle degenerated filter case for filter size = 1
			return;
		}
		

        const auto & rInData = p_rInput.getData();
        const auto & rInRank = p_rInput.getRank();
    
		auto itOutData = p_rOutput.getData().begin();
		auto itOutRank = p_rOutput.getRank().begin();


        // 2) define the container for the window, and the helper function to update it
        //the window is preallocated, but we update the oValuesInWindowSize
        int oValuesInWindowSize = 0;
        
        int oRankSumInWindow = 0;

        auto fAppendValuesInWindow = [&rValuesInWindow, &oRankSumInWindow, &p_oSizeFilter, & oValuesInWindowSize] (T const & rValueToAdd, int const & rRankToAdd )
        {            
            oRankSumInWindow += (rRankToAdd);
            if (rRankToAdd != eRankMin)
            {
                assert(oValuesInWindowSize < p_oSizeFilter);
                rValuesInWindow[oValuesInWindowSize] = rValueToAdd;
                oValuesInWindowSize ++;
            }
        };        
        
                
        auto fSearchValuesInWindow = [&rValuesInWindow, &oValuesInWindowSize] (T const & rValueToRemove, int const & rRankToRemove)
        {
            auto indexToRemove = -1;
            if (rRankToRemove != eRankMin)
            {
                auto itEnd = rValuesInWindow.begin() + oValuesInWindowSize;
                //we cannot assume that the value to remove is the first, because calcMedian partially reorders to oValuesInWindow
                auto it =  std::find(rValuesInWindow.begin(), itEnd, rValueToRemove);
#ifndef NDEBUG
                if(it == itEnd)
                {
                    std::cout << " Cant find " << rValueToRemove << "  diffs: " ;
                    for (auto && v: rValuesInWindow)
                    {
                        std::cout << v - rValueToRemove << " " ;
                    }
                    std::cout << std::endl;
                    assert(false);
                }
            #endif
                indexToRemove = it  - rValuesInWindow.begin();
                assert(indexToRemove < oValuesInWindowSize);
                assert(rValuesInWindow[indexToRemove] == rValueToRemove);
               
            }
            return indexToRemove;
    
        };
                
        auto fRemoveFromWindowValues = [&rValuesInWindow, &oValuesInWindowSize] (int indexToRemove)
        {
            assert(indexToRemove >= 0 && indexToRemove < (int)(oValuesInWindowSize));
            int lastIndex = oValuesInWindowSize - 1;
            rValuesInWindow[indexToRemove] = rValuesInWindow[lastIndex];

            oValuesInWindowSize = lastIndex;

        };       
        
        auto fRemoveFromValuesInWindow = [&oRankSumInWindow, & fRemoveFromWindowValues, &fSearchValuesInWindow] (T const & rValueToRemove, int const & rRankToRemove)
        {
            if (rRankToRemove != eRankMin)
            {
                int indexToRemove = fSearchValuesInWindow(rValueToRemove, rRankToRemove);
                assert(indexToRemove >= 0);
                oRankSumInWindow -= rRankToRemove;
                fRemoveFromWindowValues(indexToRemove);
            }
        };
        
        auto fUpdateValuesInWindow = [&rValuesInWindow, &oRankSumInWindow, &fAppendValuesInWindow, & fRemoveFromWindowValues, &fSearchValuesInWindow] (T const & rValueToRemove, int const & rRankToRemove, T const & rValueToAdd, int const & rRankToAdd )
        {
            auto indexToRemove = fSearchValuesInWindow(rValueToRemove, rRankToRemove);
    
            if (indexToRemove >= 0)
            {                   
                oRankSumInWindow -= rRankToRemove;

                if (rRankToAdd != eRankMin)
                {
                    oRankSumInWindow += (rRankToAdd);
                    rValuesInWindow[indexToRemove] = rValueToAdd;
                }
                else
                {
                    fRemoveFromWindowValues(indexToRemove);      
                }
            }
            else
            {
                fAppendValuesInWindow(rValueToAdd, rRankToAdd);
            }
        };
        

        auto fWriteOutput = [&rValuesInWindow, &oRankSumInWindow, &oValuesInWindowSize](const int & rInRankAtOutputPosition, T  & rOutValue, int & rOutRank )
        {
            if (t_oPassThroughBadRank && rInRankAtOutputPosition == eRankMin )
            {
                rOutValue = T();
                rOutRank = eRankMin;
            }
            else
            {
                auto numValidValues = oValuesInWindowSize;
                if (numValidValues > 0)
                {
                    rOutValue = * (filter::calcMedian(rValuesInWindow.begin(), rValuesInWindow.begin() + oValuesInWindowSize));
                    rOutRank = oRankSumInWindow / numValidValues;
                }
                else
                {
                    rOutRank = eRankMin;
                }                
            }
            
        };
        
        // 3)processing: read the input line and compute the median on the window
        
        int indexToRead = 0;
        int indexOutput = - p_oSizeFilter/2;
        
        //left boundary: initialize window container
        //contrarily to the comment, processCentric(calcMedian1d) computes the output from the incomplete buffer
        for(indexToRead = 0;  indexToRead < p_oSizeFilter;   ++indexToRead, ++indexOutput )
        {
            fAppendValuesInWindow(rInData[indexToRead], rInRank[indexToRead]);
    
            if (indexOutput>=0)
            {
                fWriteOutput(p_rInput.getRank()[indexOutput], p_rOutput.getData()[indexOutput], p_rOutput.getRank()[indexOutput]);
            }
        }
        
        //update the window container 
        
        itOutData = p_rOutput.getData().begin() + indexOutput;
        itOutRank = p_rOutput.getRank().begin() + indexOutput;

        int indexToRemove = indexToRead - p_oSizeFilter;
        assert(indexToRemove == 0);
        
        for (int end = p_rInput.getData().size(); indexToRead < end;
           ++indexToRead, ++indexToRemove, ++itOutData, ++itOutRank, ++indexOutput)
        {
            fUpdateValuesInWindow(rInData[indexToRemove], rInRank[indexToRemove], rInData[indexToRead], rInRank[indexToRead]);
            fWriteOutput(rInRank[indexOutput], *itOutData, *itOutRank );              
        }
        
        //right boundary: remove values from  window 
        for (int end = p_rOutput.getData().size(); indexOutput < end;
           ++indexToRemove, ++itOutData, ++itOutRank, ++indexOutput)
        {
            assert(indexToRemove < (int)p_rInput.size());
            fRemoveFromValuesInWindow(rInData[indexToRemove], rInRank[indexToRemove]);
            fWriteOutput(rInRank[indexOutput], *itOutData, *itOutRank );              
        }
          
        
    } 
    
	static void movingMedian(const geo2d::TArray<T> &p_rInput, geo2d::TArray<T> &p_rOutput, std::vector<T> & oValuesInWindow, bool p_oPassThroughBadRank)
    {
        if (p_oPassThroughBadRank)
        {
            movingMedian<true>(p_rInput, p_rOutput, oValuesInWindow);
        }
        else
        {
            movingMedian<false>(p_rInput, p_rOutput, oValuesInWindow);
        }
    }


	template<bool t_oPassThroughBadRank>
	static void movingMean(const geo2d::TArray<T> &p_rInput, geo2d::TArray<T> &p_rOutput, int p_oSizeFilter) 
    {
        // 1) check input
        poco_assert_dbg(p_rOutput.size() >= p_rInput.size() && "Output array length >= input array length."); // ok: asserted by initialization of p_rLineOut
		
		if (p_oSizeFilter <= 1 || (int)p_rInput.size() < p_oSizeFilter) 
        {
			p_rOutput = p_rInput; // handle degenerated filter case for filter size = 1
			return;
		}
		
        p_rOutput.resize(p_rInput.size());

        const auto & rInData = p_rInput.getData();
        const auto & rInRank = p_rInput.getRank();
    
		auto itOutData = p_rOutput.getData().begin();
		auto itOutRank = p_rOutput.getRank().begin();

        
        // 2) define the container for the window, and the helper function to update it
        
        T oSumValuesInWindow = 0;
        int oNumValuesInWindow  = 0;
        int oRankSumInWindow = 0;

        auto fAddToValuesInWindow = [&oSumValuesInWindow,  &oNumValuesInWindow,  &oRankSumInWindow] (T const & rValue, int const & rRank )
        {
                oRankSumInWindow += (rRank);
                bool isNotBadRank = rRank != eRankMin;
                if (isNotBadRank)
                {
                    oSumValuesInWindow += rValue;
                    ++oNumValuesInWindow;
                }
        };
               
        auto fRemoveFromValuesInWindow = [&oSumValuesInWindow, & oNumValuesInWindow,  &oRankSumInWindow] (T const & rValue, int const & rRank )
        {
                oRankSumInWindow -= (rRank);
                bool isNotBadRank = rRank != eRankMin;
                if (isNotBadRank)
                {
                    oSumValuesInWindow -= rValue;
                    --oNumValuesInWindow;
                }
        };
        
        auto fWriteOutput = [&oSumValuesInWindow, & oNumValuesInWindow, &oRankSumInWindow](const int & rInRankAtOutputPosition, T  & rOutValue, int & rOutRank )
        {

            if (t_oPassThroughBadRank && rInRankAtOutputPosition == eRankMin )
            {
                rOutValue = T();
                rOutRank = eRankMin;
            }
            else
            {
                if (oNumValuesInWindow > 0)
                {
                    rOutValue= oSumValuesInWindow / oNumValuesInWindow;
                    rOutRank = oRankSumInWindow / oNumValuesInWindow;
                }
                else
                {
                    rOutRank = eRankMin;
                }                
            }
            
        };
        
        // 3)processing: read the input line and compute the median on the window

        int indexToRead = 0;
        int indexOutput = - p_oSizeFilter/2;
        for(indexToRead = 0;  indexToRead < p_oSizeFilter;   ++indexToRead, ++indexOutput )
        {
            fAddToValuesInWindow(rInData[indexToRead], rInRank[indexToRead]);
    
            if (indexOutput>=0)
            {
                //p_rOutput[indexOutput] = p_rInput[indexOutput];
                fWriteOutput(p_rInput.getRank()[indexOutput], p_rOutput.getData()[indexOutput], p_rOutput.getRank()[indexOutput]);
            }
        }
        
        itOutData = p_rOutput.getData().begin() + indexOutput;
        itOutRank = p_rOutput.getRank().begin() + indexOutput;

        int indexToRemove = indexToRead - p_oSizeFilter;
        assert(indexToRemove == 0);
        
        for (int end = p_rInput.getData().size(); indexToRead < end;
           ++indexToRead, ++indexToRemove, ++itOutData, ++itOutRank, ++indexOutput)
        {
            fRemoveFromValuesInWindow(rInData[indexToRemove], rInRank[indexToRemove]);
            fAddToValuesInWindow(rInData[indexToRead], rInRank[indexToRead]);
            fWriteOutput(rInRank[indexOutput], *itOutData, *itOutRank );              
        }
        
        for (int end = p_rOutput.getData().size(); indexOutput < end;
           ++indexToRemove, ++itOutData, ++itOutRank, ++indexOutput)
        {
            assert(indexToRemove < (int)p_rInput.size());
            fRemoveFromValuesInWindow(rInData[indexToRemove], rInRank[indexToRemove]);
            fWriteOutput(rInRank[indexOutput], *itOutData, *itOutRank );              
        }                  
    } //movingAverage
    

	static void movingMean(const geo2d::TArray<T> &p_rInput, geo2d::TArray<T> &p_rOutput, int p_oSizeFilter, bool p_oPassThroughBadRank)
    {
        if (p_oPassThroughBadRank)
        {
            movingMean<true>(p_rInput, p_rOutput, p_oSizeFilter);
        }
        else
        {
            movingMean<false>(p_rInput, p_rOutput, p_oSizeFilter);
        }
    }
    const std::function<std::tuple<T, int>(const geo2d::TArray<T>&)> & getAlgorithm()
    {
        return m_oAlgorithm;
    }
    
private:
	const unsigned int													m_oSizeFilter;			///< filter-size
	const unsigned int													m_oHalfSizeFilter;		///< half filter-size
	const std::function<std::tuple<T, int>(const geo2d::TArray<T>&)>	m_oAlgorithm;			///< Causal algorithm to be applied on moving window
	const bool															m_oPassThroughBadRank;	///< if bad ranked values are always passed through and not eliminated
	
	geo2d::TArray<T>													m_oRingbuffer;			///< Ringbuffer 
}; // MovingWindow



} // namespace filter
} // namespace precitec


#endif // MOVINGWINDOW_H_20111021
