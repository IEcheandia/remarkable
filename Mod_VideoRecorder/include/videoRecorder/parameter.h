/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief		Contains recording parameter.
 */

#ifndef PARAMETER_H_20120215_INCLUDED
#define PARAMETER_H_20120215_INCLUDED

#include "message/device.h"
#include <map>
#include <cstdint>
#include <mutex>


namespace precitec {
namespace vdr {

/**
 * @ingroup VideoRecorder
 * @brief	Contains recording parameter.
 * @details	Currently all recording parameters as read from the XML-file 'video_recorder.xml' located in '($WM_BASE_DIR)/config/'.
 * 			If you add parameters, consider also adding checks in 'VideoRecorder::set'.
 */
typedef std::map<std::string, interface::KeyValue*> key_value_map_t;

class Parameter {
public:
	Parameter();
	/// attention: if a new key is added check get(), set(), and getConfiguration() in videoReorder.h
	/// in regard to gui +1 hack for seam, seam series special treatment
	/// also in createMap() the new parameter must be added
	/// in /wm_inst the video_recorder.xml config template has to be be updated

	bool isEnabled() const;	///<	handle concurrent access

	void isEnabled(bool);	///<	handle concurrent access

private:
	interface::TKeyValue<bool>		m_oIsShutOff;			///< Flag if recording is globally disabled.
	interface::TKeyValue<bool>		m_oIsEnabled;			///< Flag if recording is enabled.

public:
	interface::TKeyValue<int>		m_oNioMode;				///< Recording mode concerning NIOs.
	interface::TKeyValue<uint32_t>	m_oNbProductsToKeep;	///< Number of product folders to be hold on disk.
	interface::TKeyValue<uint32_t>	m_oNbLiveModeToKeep;	///< Number of live mode folders to be hold on disk.
	interface::TKeyValue<uint32_t>	m_oNbMaxImgLive;		///< Max number of images to be recorded in live mode cycle.
	interface::TKeyValue<double>	m_oMaxDiskUsage;		///< Maximum usage of disk space.
	interface::TKeyValue<bool> m_skipIfFull; ///< Flag skipping frames when queue is full.

	const key_value_map_t			m_oParamMap;			///< map that references all parameters for quick access by key

private:
	/**
	 * @brief	Puts pointers to all key values in a map. The key of the map is the key value key.
	 * @return	key_value_map_t Key value map.
	 */
	key_value_map_t createMap(); // map factory

	std::mutex 						m_oWriteMutex;			///< handle concurrent access
}; // Parameter

/**
 * @brief	Resets all invalid parameter to their default value.
 * @param	p_rParameter	A Parameter object.
 * @return	void
 */
void invalidToDefault(Parameter &p_rParameter);

/**
 * @brief	Prints all parameter values to cout.
 * @param	p_rParameter	A Parameter object.
 * @return	void
 */
void print(const Parameter &p_rParameter);

/**
 * @brief	Creates a Configuration from a Parameter instance.
 * @param	p_rParameter	A Parameter object.
 * @param	p_oSeamPlus1	Add one to all seam and seam series related values (for gui display needed).
 * @return	Configuration	All parameters of the passed parameter object in a Configuration container.
 */
interface::Configuration makeConfiguration(const Parameter &p_rParameter, bool p_oSeamPlus1 = false);


} // vdr
} // precitec

#endif /* PARAMETER_H_20120215_INCLUDED */
