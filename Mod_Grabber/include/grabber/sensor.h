/**
 * @file
 * @brief  Sensoreigenschaften fuer Kamera und Grabber
 *
 * @author JS
 * @date   18.05.11
 * @version 0.1
 *
 */



#ifndef SENSOR_HEADER_FILE
#define SENSOR_HEADER_FILE

//#include <string>
#include <iostream>
#include <sstream>
#include <set>
#include <vector>
//#include "pf/photonFocus.h"

#include "system/types.h"
//#include "message/device.h"


/**
*  Klasse SensorKey und TSensorProperty
*  Zweck: Beschreibung eines Kamera oder Grabber Properties mit
*  min, max, default und dem Daten Typ
*
*/
/// Sensor ( Kamera/Grabber ) Eigenschaft
class SensorKey
{
	public:
		virtual ~SensorKey() {}			///<virtueller DTor
		SensorKey(std::string prop, precitec::Types typ){property = prop; propertyType = typ; } ///<CTor mit Property und datatype
		std::string property;	///<property name
		precitec::Types propertyType;		///<data type
};


/**
*  Klasse TSensorProperty
*  Haelt min,max und default
*
 */
/// Haelt min,max und default
template<typename T>
class TSensorProperty : public SensorKey
{
public:
	TSensorProperty(std::string key, T minVal, T maxVal, T defVal, precitec::Types typ) : SensorKey(key,typ ){min = minVal;max = maxVal;defValue = defVal; }

	T min;	///<minima
	T max;	///<maxima
	T defValue;	///<default Value
};

typedef SensorKey *pSensorKey;
typedef std::vector<pSensorKey> SensorConfigVector;

/// Spiegel der Kamera- und Framegrabber Parameter
//typedef std::vector<SmpKeyValue> Configuration;
//precitec::interface::Configuration sensorConfig;



#endif // SENSOR_HEADER_FILE
