#ifndef SENSORFILTERINTERFACE_H_
#define SENSORFILTERINTERFACE_H_

#include "InterfacesManifest.h"

#include <string>

namespace precitec
{
namespace interface
{
	// In dieser Klasse sind die ID's der SensorPipes global definiert

	/*abstract*/
	class INTERFACES_API SensorFilterInterface
	{
		public:
			static const std::string SENSOR_IMAGE_FRAME_PIPE;
			static const std::string SENSOR_SAMPLE_FRAME_PIPE;
	};

}
}

#endif /*SENSORFILTERINTERFACE_H_*/
