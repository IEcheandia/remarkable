/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2013
 * 	@brief		This filter generates a new output signal, following a user-selectable function.
 */

#ifndef FUNCTION_GENERATOR_H_
#define FUNCTION_GENERATOR_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>
#include <common/frame.h>
// stl includes
#include <queue>
#include <random>

namespace precitec {
namespace filter {

/**
 * @brief This filter generates a new output signal, following a user-selectable function.
 */
class FILTER_API FunctionGenerator : public fliplib::TransformFilter
{
public:

	/**
	 * @brief CTor.
	 */
	FunctionGenerator();
	/**
	 * @brief DTor.
	 */
	virtual ~FunctionGenerator();

	// Declare constants
	static const std::string m_oFilterName;				///< Filter name.
	static const std::string m_oPipeOutName;			///< Pipe: Data out-pipe.

	static const std::string m_oParamFunction;			///< Parameter: Type of function.
	static const std::string m_oParamPhaseShift;		///< Parameter: Phase shift.
	static const std::string m_oParamAmplitudeShift;	///< Parameter: Amplitude shift.
	static const std::string m_oParamAmplitude;			///< Parameter: Amplitude.
	static const std::string m_oParamFrequency;     	///< Parameter: Frequency.

	/**
	 * @brief Set filter parameters.
	 */
	void setParameter();

	/**
	 * @brief Arm the filter. The filter will empty
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
	 * @param p_pSender
	 * @param p_rEvent
	 */
	void proceed( const void* p_pSender, fliplib::PipeEventArgs& p_rEvent );

protected:

	const fliplib::SynchronePipe< interface::ImageFrame >*		m_pPipeInImage;			///< Data in-pipe.
	fliplib::SynchronePipe< interface::GeoDoublearray >			m_oPipeOutData;			///< Data out-pipe.

	// todo: function type enum

	unsigned int												m_oFunction;			///< Type of function.
	double														m_oPhaseShift;			///< Phase shift.
	double														m_oAmplitudeShift;		///< Amplitude shift.
	double														m_oAmplitude;			///< Amplitude of the output function.
	double														m_oFrequency;			///< Frequency of the function.

	unsigned int												m_oCounter;
	unsigned int												m_oTriggerFreq;
    
    bool m_applyRandomness;
    unsigned int m_oOversamplingRate; ///< how many signals per loop are sampled (signal sampling rate / 1Khz)
    double m_oRandomnessApplied; ///< a random number generated by std::rand, with the interval between [-m_oAmplitudeShift, m_oAmplitudeShift]
    std::mt19937 m_oMersenneTwister;
    std::normal_distribution<double> m_oNormalDist;

}; // class FunctionGenerator

} // namespace filter
} // namespace precitec

#endif // FUNCTION_GENERATOR_H
