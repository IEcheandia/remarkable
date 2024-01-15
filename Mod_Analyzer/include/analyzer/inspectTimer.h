/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			HS
 *  @date			2014
 *  @brief			'InspectTimer' measures image- and sample-processing time, frame-to-frame time, overlay time.
 */

#ifndef INSPECTTIMER_H_INCLUDED_20140117
#define INSPECTTIMER_H_INCLUDED_20140117

#include "system/timer.h"
#include "Poco/Mutex.h"

#include <list>

namespace precitec {
namespace analyzer {

/**
  *	@brief		'InspectTimer' measures image- and sample-processing time, frame-to-frame time, overlay time.
  *	@ingroup Analyzer
  */
class InspectTimer {
public:
	/**
	  *	@brief		CTOR initialized with seam number and hw parameter set id.
	  */
	InspectTimer();

	/**
	  * @brief		Resets accumalated timings
	  */
	void resetAccumulatedTimings();

	/**
	  * @brief		Logs accumalated timings
	  */
	void logAccumulatedTimings();

    enum class OvertriggeringResult
    {
        /**
         * Image is not overtriggered
         **/
        EverythingOk,
        /**
         * Processing time is getting close to be overtriggered. Warning should be shown
         **/
        Dangerous,
        /**
         * Processing time is getting very close to be overtriggered. Image should be skipped.
         **/
        Critical,
        /**
         * Processing time is overtriggered, NIO should be thrown.
         **/
        Overtriggered
    };
	
	/**
	  * @brief		warn user about overtriggering if frame-to-frame time has surpassed trigger delta by more than 1ms 
	  * @param		p_oTimeDelta			time delta between triggers [us]
	  * @param		p_oImageNumber			seam-related number of image / sample
	  * @return     Evaluation of overtriggering
	  */
	OvertriggeringResult checkOvertriggering(int p_oTimeDelta, int p_oImageNumber, bool p_oConservative, int toleranceOvertriggering_us);
    std::pair<unsigned int,double> getLastInspectManagerTime_us()
    {
        return {m_inspectManagerAccumulated, std::chrono::duration_cast<std::chrono::microseconds>(m_lastInspectManagerTime).count()};
       
    }
    
    std::pair<unsigned int,double> getLastImageTime_us() 
    {
        return {m_oNbImagesAccumulated,std::chrono::duration_cast<std::chrono::microseconds>(m_lastImageTimes.back()).count()}; 
    }
    
    /**
     * Adds @p imageProcessing and @p sampleProcessing to the accumulated timings.
     **/
    void updateProcessingTime(const std::chrono::nanoseconds &imageProcessing, const std::chrono::nanoseconds &sampleProcessing, const std::chrono::nanoseconds &sendDataTime);

    void updateSkipImageProcessingTime(const std::chrono::nanoseconds & imageProcessing);

    /**
     * Adds @p time to the overall time spent in InspectManager for processing data
     */
    void updateInspectManagerTime(const std::chrono::nanoseconds &time, bool increaseFrameCount );

    /**
     * Get the current average processing time per image (ms)
     */
    double processingTime() const;
    
private:
	unsigned int					m_oNbOverTrigInARow;			///< nb of overtriggered frames in a row
	unsigned int					m_oNbImagesAccumulated;			///< nb of accumulated image timings
	unsigned int					m_oNbSamplesAccumulated;		///< nb of accumulated sample timings
	unsigned int					m_oNbSendDataAccumulated;		///< nb of accumulated overlay timings
    unsigned int					m_inspectManagerAccumulated;	///< nb of accumulated InspectManager timings

    Poco::FastMutex m_updateProcessingTimeMutex;
    std::chrono::nanoseconds m_imageProcessing;
    std::chrono::nanoseconds m_sampleProcessing;
    std::chrono::nanoseconds m_inspectManager;
    std::chrono::nanoseconds m_sendData;

    std::list<std::chrono::nanoseconds> m_lastImageTimes;
    std::chrono::nanoseconds m_lastInspectManagerTime; 
    std::chrono::nanoseconds m_currentPartialInspectManagerTime; 
    unsigned int m_oNbFramesSkipped;
};


} // namespace analyzer
} // namespace precitec

#endif /* INSPECTTIMER_H_INCLUDED_20140117 */
