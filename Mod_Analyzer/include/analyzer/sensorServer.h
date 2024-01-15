/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			KIR
 *  @date			2009
 *  @brief			Receive sensor data from grabber
 */



#ifndef SENSORSERVER_H_
#define SENSORSERVER_H_

#include "Mod_Analyzer.h"
#include "event/sensor.h"
#include "event/sensor.interface.h"
#include "event/imageShMem.h"
#include "inspectManager.h"
#include "system/tools.h"

namespace precitec
{
namespace analyzer
{
	// Nimmt die Datenpakete vom VMI-Sensor entgegen und verarbeitet diese
	class MOD_ANALYZER_API SensorServer : public TSensor<AbstractInterface>
	{
	public:
		SensorServer(InspectManager *inspectManager) : inspectManager_(inspectManager),
		imageShMem_(greyImage::sharedMemoryHandle(), greyImage::sharedMemoryName(), precitec::system::SharedMem::StdServer, greyImage::sharedMemorySize(inspectManager->isSimulationStation()))
		{
		}
		virtual ~SensorServer() {}

	public:
		// liefert ein Bild
		void data(int sensorId, interface::TriggerContext const& context, image::BImage const& data) {
			try {
				inspectManager_->data(sensorId, context, data);
			} // try
			catch(...) {
				logExcpetion(__FUNCTION__, std::current_exception());
			} // catch		
		}

		// liefert eine AnalogMessung/...
		void data(int sensorId, interface::TriggerContext const& context, image::Sample const& data) {
			try {
				inspectManager_->data(sensorId, context, data);
			} // try
			catch(...) {
				logExcpetion(__FUNCTION__, std::current_exception());
			} // catch		
		}

        void simulationDataMissing(interface::TriggerContext const& context) override
        {
            inspectManager_->simulationDataMissing(context);
        }

	private:
		InspectManager *inspectManager_;
		system::SharedMem imageShMem_;
	};

}	// analyzer
}	// precitec

#endif /*SENSORSERVER_H_*/
