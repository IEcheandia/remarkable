/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2016
 * 	@brief		Contains key value parameter.
 */

#ifndef DEVICEPARAMETER_H_20161118_INCLUDED
#define DEVICEPARAMETER_H_20161118_INCLUDED

#include "message/device.h"
#include "Mod_Analyzer.h"

#include "Poco/Path.h"

#include <map>
#include <cstdint>
#include <memory>
#if defined __QNX__ || defined __linux__ // otherwise wm host clr error
#include <mutex>
#endif


namespace precitec {
namespace analyzer {

/**
 * @ingroup Workflow
 * @brief	Contains key value parameter.
 * @details	Currently all parameters as read from the XML-file 'workflow.xml' located in '($WM_BASE_DIR)/config/'.
 */
typedef std::shared_ptr<interface::KeyValue>						sp_key_value_t;	// shared ptr: to work with base class ptr, unique ptr not ok with map
typedef std::map<std::string, sp_key_value_t> key_value_map_t;

class MOD_ANALYZER_API DeviceParameter {
public:
	DeviceParameter(bool simulationStation);
	/// attention: if a new key is added
	/// in /wm_inst the workflow.xml config template has to be be updated

	/**
	 * @brief	set a value
	 * @param	p_oSmpKeyValue	key and value
	 * @return	KeyHandle		handle (token)
	 */
	interface::KeyHandle set(interface::SmpKeyValue p_oSmpKeyValue);

	/**
	 * @brief	get a value by key
	 * @param	p_oKey			key
	 * @return	SmpKeyValue		key and value
	 */
	interface::SmpKeyValue get(interface::Key p_oKey) const;
    bool getConservativeCheckOvertriggering() const;
    bool getLogAllFilterProcessingTime() const;
    double getToleranceOvertriggering_ms() const;

    bool getRealTimeGraphProcessing() const;

	key_value_map_t			m_oParameters;			///< map that references all parameters for quick access by key

private:
	/**
	 * @brief	Puts pointers to all key values in a map. The key of the map is the key value key.
	 * @return	key_value_map_t Key value map.
	 */
	key_value_map_t createMap(); // map factory

    template <typename T>
    bool setValue(const interface::SmpKeyValue & srcKey, std::shared_ptr<interface::KeyValue> destKey )
    {
        poco_assert_msg( destKey->key() == srcKey->key(), "Trying to validate a key-value using a different one as reference ");
        const T & value =  srcKey->value<T>() ;
        if ( destKey->minima<T>() <= value && value <= destKey->maxima<T>())
        {
            destKey->setValue<T>(value);
            return true;
        }
        else
        {
            wmLog(eDebug, "Attention, value not valid for %s!\n", destKey->key().c_str());
            return false;
        }
    }

	static const std::string		m_oKeyIsParallelInspection;	       ///< key for parameter map
	static const std::string		m_oKeyDisableHWResults;	           ///< key for parameter map
    static const std::string        m_oKeyLineLaser1DefaultIntensity;  ///< key for parameter map
    static const std::string        m_oKeyLineLaser2DefaultIntensity;  ///< key for parameter map
    static const std::string        m_oKeyFieldLight1DefaultIntensity; ///< key for parameter map
    static const std::string        m_oKeyRealTimeGraphProcessing;

	
	Poco::Path						m_oConfigDir;				///< Directory of the xml configuration file.
#if defined __QNX__ || defined __linux__ // otherwise wm host clr error
	std::mutex 						m_oWriteMutex;				///< handle concurrent access
#endif // #ifdef __QNX__
    bool m_simulationStation;
    bool m_oConservativeCheckOvertriggeringValue;
    bool m_oLogAllFilterProcessingTime;
    bool m_realTimeGraphProcessing = false;
    double m_oToleranceOvertriggering_ms;
}; // Parameter

/**
 * @brief	Resets all invalid parameter to their default value.
 * @param	p_rParameter	A Parameter object.
 * @return	void
 */
void invalidToDefault(DeviceParameter &p_rParameter);

/**
 * @brief	Prints all parameter values to cout.
 * @param	p_rParameter	A Parameter object.
 * @return	void
 */
void print(const DeviceParameter &p_rParameter);

/**
 * @brief	Creates a Configuration from a Parameter instance.
 * @param	p_rParameter	A Parameter object.
 * @return	Configuration	All parameters of the passed parameter object in a Configuration container.
 */
interface::Configuration makeConfiguration(const DeviceParameter &p_rParameter);


} // analyzer
} // precitec

#endif /* DEVICEPARAMETER_H_20161118_INCLUDED */
