/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2013
 * 	@brief		This filter stores data elements in a buffer. They can be accessed in a different filter graph using the buffer player filter.
 */

#ifndef ONESAMPLERECORDER_H_
#define ONESAMPLERECORDER_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>

#include "event/resultType.h"
#include "common/geoContext.h"

namespace precitec {
namespace filter {

class ImageContext;

#define MAX_SLOTS_FOR_SAMPLE_RECORDER 10

/**
 * @brief This filter stores data elements in a buffer. They can be accessed in a different filter graph using the buffer player filter.
 */
class FILTER_API OneSampleRecorder : public fliplib::TransformFilter
{
public:

	/**
	* @brief CTor.
	*/
	OneSampleRecorder();
	/**
	 * @brief DTor.
	 */
	virtual ~OneSampleRecorder();

	// Declare constants
	static const std::string m_oFilterName;			///< Filter name.

	/**
	 * @brief Set filter parameters.
	 */
	void setParameter();

	/**
	 * @brief Arm the filter. This means here, that the length of the seam is determined and memory is allocated for all the data elements.
	 */
	virtual void arm(const fliplib::ArmStateBase& state);

	static void GetSampleValue(int index, int currentNumber, double& p_rSample, ValueRankType& p_rRank, interface::SmpTrafo& p_rTrafo, interface::ResultType& p_rResult);

protected:

	static double m_oSampleValue1[MAX_SLOTS_FOR_SAMPLE_RECORDER];
	static double m_oSampleValue2[MAX_SLOTS_FOR_SAMPLE_RECORDER];
	static int m_oImageNumber1[MAX_SLOTS_FOR_SAMPLE_RECORDER];
	static int m_oImageNumber2[MAX_SLOTS_FOR_SAMPLE_RECORDER];
	//static int m_oImageCounter[MAX_SLOTS_FOR_SAMPLE_RECORDER];
	static Poco::Mutex m_oMutex;
	static ValueRankType m_oRank1[MAX_SLOTS_FOR_SAMPLE_RECORDER];
	static ValueRankType m_oRank2[MAX_SLOTS_FOR_SAMPLE_RECORDER];
	static interface::SmpTrafo m_oTrafo1[MAX_SLOTS_FOR_SAMPLE_RECORDER];
	static interface::SmpTrafo m_oTrafo2[MAX_SLOTS_FOR_SAMPLE_RECORDER];
	static interface::ResultType m_oAnalysisResult1[MAX_SLOTS_FOR_SAMPLE_RECORDER];
	static interface::ResultType m_oAnalysisResult2[MAX_SLOTS_FOR_SAMPLE_RECORDER];


	int m_oSlotNumber;
	int m_oCurrentNumber;
	double m_oInitialSampleValue;

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
	void proceed( const void* p_pSender, fliplib::PipeEventArgs& p_rEvent );

protected:

	const fliplib::SynchronePipe< interface::GeoDoublearray >*	m_pPipeInData;			///< Data in-pipe.


}; // class BufferRecorder


} // namespace filter
} // namespace precitec

#endif /* ONESAMPLERECORDER_H_ */
