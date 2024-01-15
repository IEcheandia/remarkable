#include "math/descriptiveStats.h"
#include <cassert>
#include <cmath>
#include <limits>
#include <string>
#include <sstream>
#include <iostream>


using std::numeric_limits;
namespace precitec
{
namespace math
{

	DescriptiveStats::DescriptiveStats(const double p_IntensityMin, const double p_IntensityMax, const unsigned int p_NumberOfBins) :
		m_oMinValue(numeric_limits<double>::max()),
		m_oMaxValue(numeric_limits<double>::min()),
		m_oSum(0),
		m_oCount(0)
	{
		assert(p_IntensityMax >= p_IntensityMin);
		m_oNumberOfBins = p_NumberOfBins;
		m_oRangeMax = p_IntensityMax;
		m_oRangeMin = p_IntensityMin;
		double expectedRange = m_oRangeMax - m_oRangeMin;
        if (expectedRange > 0)
        {
            m_oBinWidth = expectedRange / m_oNumberOfBins;
        }
        else
        {
            m_oNumberOfBins =1;
            m_oBinWidth = 1.0;
        }
		m_oBinOverRange = m_oNumberOfBins;
		m_oBinUnderRange = m_oBinOverRange + 1;

		m_ResultHistogram = std::vector<std::pair<double, int > >(m_oNumberOfBins + 2);
		for ( unsigned int i = 0; i < m_oNumberOfBins; i++ )
		{
			m_ResultHistogram[i] = std::make_pair(p_IntensityMin + m_oBinWidth*i, 0);
		}
		m_ResultHistogram[m_oBinOverRange] = std::make_pair(p_IntensityMax, 0);
		m_ResultHistogram[m_oBinUnderRange] = std::make_pair(p_IntensityMin, 0);

	}

	DescriptiveStats::~DescriptiveStats()
	{}

	double DescriptiveStats::getMin()
	{
		return m_oMinValue;
	}

	double DescriptiveStats::getMax()
	{
		return m_oMaxValue;
	}

	double DescriptiveStats::getMean()
	{
		return m_oSum / double(m_oCount);
	}

	int DescriptiveStats::getCount()
	{
		return m_oCount;
	}

	double DescriptiveStats::getRangeMin()
	{
		return m_oRangeMin;
	}

	double DescriptiveStats::getRangeMax()
	{
		return m_oRangeMax;
	}

	double DescriptiveStats::getNumberOfBins()
	{
		return m_oNumberOfBins;
	}

	unsigned int DescriptiveStats::getBin( const double p_Value)
	{
		unsigned int bin;
		if ( p_Value < m_oRangeMax )
		{
			if ( p_Value >= m_oRangeMin )
			{    //m_oRangeMin <= val < m_oRangeMax
				bin = int(std::floor((p_Value - m_oRangeMin) / m_oBinWidth));
			}
			else
			{	 //val < m_oRangeMin
				bin = m_oBinUnderRange;
			}
		}
		else
		{	//val >= m_oRangeMax
			bin = m_oBinOverRange;
		}
		return bin;
	}


	void DescriptiveStats::addIntensityValue(const double p_Value)
	{
		m_oMinValue = (p_Value < m_oMinValue) ? p_Value : m_oMinValue;
		m_oMaxValue = (p_Value > m_oMaxValue) ? p_Value : m_oMaxValue;
		m_oSum += p_Value;
		++m_oCount;
		unsigned int bin = getBin(p_Value);
		m_ResultHistogram[bin].second++;

#ifndef NDEBUG
		assert(m_ResultHistogram[bin].second > 0);
		bool outOfRange = (p_Value  < m_oRangeMin) || (p_Value >= m_oRangeMax);
		if ( !outOfRange )
		{
			assert(bin != m_oBinOverRange);
			assert(bin != m_oBinUnderRange);
			assert((p_Value + 0.000001) > m_ResultHistogram[bin].first);
			assert((p_Value - 0.000001) < m_ResultHistogram[bin < m_oBinOverRange ? bin + 1 : m_oBinOverRange].first);
		}
		else
		{
			assert(bin == m_oBinUnderRange || bin == m_oBinOverRange);
			if ( (p_Value + 0.000001) >= m_oRangeMax )
			{
				assert(bin == m_oBinOverRange);
			}
			else
			{
				assert(bin == m_oBinUnderRange);
			}
		}
#endif
	}


	std::string DescriptiveStats::printBinRange(const unsigned int p_bin)
	{
		if ( p_bin >= m_ResultHistogram.size() )
		{
			return "Bin doesn't exist";
		}
		double start = m_ResultHistogram[p_bin].first;

		std::ostringstream msg;
		if ( p_bin == m_oBinUnderRange )
		{
			assert(start == m_oRangeMin);
			msg << " x < " << start;
		}
		else if ( p_bin == m_oBinOverRange )
		{
			assert(start == m_oRangeMax);
			msg << " x >= " << start;
		}
		else
		{
			//it works also for the last bin, because the next bin is for the values above the range
			double end = m_ResultHistogram[p_bin+1].first; 
			msg << start << "<= x < " << end;
		}
		return msg.str();
	}


	void DescriptiveStats::printResultHistogram(std::vector<std::string> & result, const int p_totalCount)
	{
		assert(checkConsistency());

		{
			std::ostringstream msg;
			msg << "Min: " << m_oMinValue << " Max: " << m_oMaxValue << " Mean: " << m_oSum / m_oCount;
			result.push_back(msg.str());
		}

		double binMax = m_ResultHistogram[m_oBinUnderRange].first;
		int count = m_ResultHistogram[m_oBinUnderRange].second;
		int sum = count;
		{
			std::ostringstream msg;
			msg << " x < " << binMax << " : " << count << "(" << count * 100 / p_totalCount << "%)";
			result.push_back(msg.str());
		}

		double binMin = binMax;
		for ( unsigned int i = 0; i < m_oNumberOfBins; i++ )
		{
			binMin = m_ResultHistogram[i].first;
			count = m_ResultHistogram[i].second;
			assert(binMax == binMin);
			binMax = m_ResultHistogram[i + 1].first;
			std::ostringstream msg;
			msg << binMin << "<= x <" << binMax << " : " << count << "(" << count * 100 / p_totalCount << "%)";
			result.push_back(msg.str());
			sum += count;
		}

		count = m_ResultHistogram[m_oBinOverRange].second;
		{
			std::ostringstream msg;
			msg << "x >=" << m_ResultHistogram[m_oBinOverRange].first << " : " << count  << "(" << count * 100 / p_totalCount << "%)";
			result.push_back(msg.str());
		}

		sum += count;
		assert(sum == p_totalCount);
	}

	bool DescriptiveStats::checkConsistency()
	{
		bool valid = true;

		if ( m_oNumberOfBins != m_ResultHistogram.size() - 2 || m_oBinOverRange != m_oNumberOfBins || m_oBinUnderRange != (m_oBinOverRange + 1) )
		{
			std::cout << "Error in num bins" << std::endl;
			valid = false;
		}
		assert(valid);
		if ( m_oMaxValue >= m_oRangeMax )
		{
			if ( !(m_ResultHistogram[m_oBinOverRange].second > 0) )
			{
				valid = false;
			}
		}
		else
		{
			if ( !(m_ResultHistogram[m_oBinOverRange].second == 0) )
			{
				valid = false;
			}
		}

		if ( m_oMinValue < m_oRangeMin )
		{
			if ( !(m_ResultHistogram[m_oBinUnderRange].second > 0) )
			{
				valid = false;
			}
		}
		else
		{
			if ( !(m_ResultHistogram[m_oBinUnderRange].second == 0) )
			{
				valid = false;
			}
		}

		int count = 0;
		for ( auto && histogramValue : m_ResultHistogram )
		{
			count += histogramValue.second;
		}
		if ( !(count == m_oCount) )
		{
			valid = false;
		}

		return valid;
	}

}
}
