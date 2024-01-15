/**
*  @file
*  @copyright		Precitec Vision GmbH & Co. KG
*  @author			LB
*  @date			2017
*  @brief			Accumulator for descriptive statistics and histogram computation
*/

#ifndef DESCRIPTIVESTATS_H
#define DESCRIPTIVESTATS_H

#include <vector>
#include <array>
#include <string>

#include "Analyzer_Interface.h"
namespace precitec
{
namespace math
{

	class ANALYZER_INTERFACE_API DescriptiveStats
	{
		
	public:
		/*
		* @Initialize the statistical accumulator
		* @param p_IntensityMin: minimum value for the histogram (m_oRangeMin, doesn't affect the computation of the minimum value)
		* @param p_IntensityMax: maximum value for the histogram (m_oRangeMax, doesn't affect the computation of the maximum value)
		* @param p_NumberOfBins: number of bins used to subdvided the input range. The resulting histogram will have  
								p_NumberOfBins for values val such that m_oRangeMin <= val < m_oRangeMax (semi closed interval), + 2 bins for the values above 
								and below the range
		*/
		DescriptiveStats(const double p_IntensityMin, const double p_IntensityMax, const unsigned int p_NumberOfBins);
		~DescriptiveStats();
		void addIntensityValue(const double p_Value);
		double clampValueToRange(const double p_Value);
		double getMin();
		double getMax();
		double getMean();
		int getCount();
		double getRangeMin();
		double getRangeMax();
		double getNumberOfBins();
		unsigned int getBin(const double p_Value);
		std::string printBinRange(const unsigned int p_bin);
		void printResultHistogram(std::vector<std::string> & result, const int p_totalCount);

	private:
		unsigned int m_oNumberOfBins;
		unsigned int m_oBinOverRange;
		unsigned int  m_oBinUnderRange;
		double m_oBinWidth;
		double m_oRangeMax;
		double m_oRangeMin;
		double m_oMinValue;
		double m_oMaxValue;
		double m_oSum;
		int m_oCount;
		std::vector<std::pair<double, int > >  m_ResultHistogram;		
		bool checkConsistency();

	};

}
}

#endif //DESCRIPTIVESTATS_H
