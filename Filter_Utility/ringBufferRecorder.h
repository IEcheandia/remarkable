/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2013
 * 	@brief		This filter stores data elements in a ring buffer. They can be accessed in a different filter graph using the buffer player filter.
 */

#ifndef RINGBUFFERRECORDER_H_
#define RINGBUFFERRECORDER_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>

namespace precitec {
namespace filter {

class StatisticCalculator
{
public:
	StatisticCalculator();
	void reset();
	void addValue(double value);
	double getMedian();
	double getMean();

private:
	std::vector<double> _data;
	void sortIt();
	void exchange(double & d1, double & d2);
};

class OneDataSet
{
public:
	OneDataSet();
	OneDataSet(const OneDataSet & anotherSet);
	OneDataSet(double data_, int data_rank_, double pos_, int pos_rank_);
	double data;
	int data_rank;
	double pos;
	int pos_rank;
};

class OwnBuffer
{
public:
	OwnBuffer();
	void reset();
	void addOneDataSet(OneDataSet set);
	OneDataSet getOneDataSet(std::size_t i);
	std::size_t getCurrentSize();
	bool isSorted();
	bool is360();
	void sort();
	void setTicksPer360(int number);
	void setWidths(int widthMean, int widthMedian);
	void calcStartEndValues(std::size_t & start, std::size_t & end);
	double getMinPos();
	double getMaxPos();
	void reduceToStartEnd(std::size_t start, std::size_t end);
	bool isInRange(double posTicks1, double posTicks2, double angleMax);
	void makePosModulo();

	double calcMeanWithMinRank(int minRank);
	void setDataForBadRank(int badRank, double dataToSet);
	void doMeanOnRange(int firstSet, int lastSet, int meanSize);
	void doMedianOnRange(int firstSet, int lastSet, int medianSize);
	void interpolateLinearOnRange(int startLastValid, int endFirstValid);
	bool cutUselessAreasAtBeginAndEnd();
	bool lowPassAndInterpolate(unsigned int mode);

	void doLowPass();
	void doLowPass(unsigned int mode);

private:
	std::vector<OneDataSet> _dataSet;
	void exchange(std::size_t i, std::size_t j);
	int _ticksPer360, _widthMean, _widthMedian;
	void doLowPassOnRange(int firstSet, int lastSet, int lowPassSize, bool useMean);
	int round(double d); 

};




/**
 * @brief This filter stores data elements in a buffer. They can be accessed in a different filter graph using the buffer player filter.
 */
class FILTER_API RingBufferRecorder : public fliplib::TransformFilter
{
public:

	/**
	 * @brief CTor.
	 */
	RingBufferRecorder();
	/**
	 * @brief DTor.
	 */
	virtual ~RingBufferRecorder();

	// Declare constants
	static const std::string m_oFilterName;			///< Filter name.
	static const std::string m_oParamSlot;			///< Parameter: Slot, into which the data is written.
	static const std::string m_oParamMode;			///< Parameter
	static const std::string m_oParamTicks;			///< Parameter
	static const std::string m_oParamWidth;			///< Parameter
	static const std::string m_oParamWidthMedian;	///< Parameter

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
	 * @param p_rEvent
	 */
	void proceedGroup( const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEvent );

protected:

	const fliplib::SynchronePipe< interface::GeoDoublearray >*	m_pPipeInData;			///< Data in-pipe.
	const fliplib::SynchronePipe< interface::GeoDoublearray >*	m_pPipeInPos;			///< Position in-pipe.

	unsigned int												m_oSlot;				///< Parameter: Slot, into which the data is written.
	unsigned int												m_oMode;				///< Parameter: Mode, switch for different interpolation modes.
	unsigned int												m_oTicks;				///< Parameter: Number of ticks equal to 360 degrees.
	unsigned int												m_oWidth;				///< Parameter: Width for low pass filtering (Mean).
	unsigned int												m_oWidthMedian;			///< Parameter: Width for low pass filtering (Median).

	int															m_oCount;

	std::shared_ptr< interface::GeoDoublearray > 				m_pData;				///< Pointer to the actual data array. Is valid after arm.
	std::shared_ptr< interface::GeoDoublearray > 				m_pPos;					///< Pointer to the actual pos array. Is valid after arm.

	int 														m_oTriggerDelta;		///< Trigger distance [um]

	interface::GeoDoublearray m_bufferData;
	interface::GeoDoublearray m_bufferPos;

	OwnBuffer _ownBuffer;

}; // class RingBufferRecorder


} // namespace filter
} // namespace precitec

#endif /* RINGBUFFERRECORDER_H_ */
