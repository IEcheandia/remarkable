/**
 * @file
 * @brief Headerfile zum DeviceServer
 *
 * @author JS
 * @date   20.05.10
 * @version 0.1
 * Erster Wurf
 */


#ifndef DEVICESERVER_H_
#define DEVICESERVER_H_

#include <string>
#include "message/device.h"
#include "message/device.interface.h"
#include "dataAcquire.h"
#include "../trigger/commandServer.h"

/**
 * \namespace precitec alles was nicht firmware ist, laeuft im namespace precitec ab
 *
 */

namespace precitec
{
	using namespace interface;
	using namespace trigger;


/**
* \namespace grabber kapselt die Device Schnittstelle zu Grabber / Kamera
*
*/
namespace grabber
{

	/// Schnittstelle zum Hardware Zugriff auf Kamera/Grabber
	class DeviceServer : public TDevice<AbstractInterface>
	{
		public:
			//DeviceServer(DataAcquire &grabber);
		    DeviceServer(DataAcquire &grabber,CommandServer &triggerCmdserver);
			virtual ~DeviceServer();

		public:
			/// 1. Ini- wird nur einmal durchgefuehrt
			int initialize(Configuration const& config, int subDevice);


			/// die Reset-Taste  - soll das close bedeuten ?
			void uninitialize();

			/// kann beliebig oft durchgefuehrt werden
			void reinitialize();



			/// spezifischen Parameter setzen
			//KeyHandle set(KeyValue const& keyValue, int subDevice=0);
			KeyHandle set(SmpKeyValue keyValue, int subDevice=0);

			/// mehrere Parameter d.h. Konfiguration setzen
			void set(Configuration config, int subDevice=0)
			{
				//do not lock here (set function has lock)
				#ifdef DEVICESERVER_LOG
				std::cout << __FUNCTION__ << " config " << config.size() <<  "\n";
				#endif

				for(unsigned int i=0;i<config.size();++i)
				{
					//set(*config[i],0);
					set(config[i],0);
				}

			}

			/// spezifischen Parameter auslesen
			//  --> typedef Poco::SharedPtr<KeyValue> SmpKeyValue;
			SmpKeyValue get(Key key, int subDevice=0);

			/// spezifischen Parameter via handle auslesen
			//  wird nicht unterstuetzt
			SmpKeyValue get(KeyHandle handle, int subDevice=0)
			{
				return SmpKeyValue(new KeyValue(TInt,"?",-1) );
			}

			/// alle Parameter auslesen
			Configuration get(int subDevice);

		private:
			DataAcquire   &m_rGrabber;    //grabber_
			CommandServer &m_rTriggerCmdServer; //Test
			bool deviceOK_;
			baseCameraConfig m_BaseCamConfig; //dataAcquire.h
			void setDeviceOK_(bool deviceOK){deviceOK_ = deviceOK;}
			void getCameraBaseConfig(void);

			bool checkAndSetGlobalParameter(SmpKeyValue keyValue);
			static Poco::FastMutex m_mutex;


	};

}

}


#endif /*DEVICESERVER_H_*/
