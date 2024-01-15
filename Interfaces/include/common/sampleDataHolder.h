/**
 * @file   
 * @copyright	Precitec Vision GmbH & Co. KG
 * @author		GG
 * @date		2017
 * @brief		Is a container for Sample file data. Used in Grabber when used as file grabber.
*/

#pragma once

#include "InterfacesManifest.h"

#include <vector>

namespace fileio
{

/**
  * @brief	Data holder for Sample Data read from a smp file. Used in Grabber when used as file grabber.
  * @detail	Contains multiple Data from multiple sensors.
  */
class INTERFACES_API SampleDataHolder
{
public:

	class OneSensorData
	{
	public:
		int sensorID;
		std::vector<int> dataVector;
	};

	std::vector< OneSensorData > allData;

};
}
