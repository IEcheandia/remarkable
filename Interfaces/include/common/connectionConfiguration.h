#ifndef CONNECTIONCONFIGURATION_H_
#define CONNECTIONCONFIGURATION_H_

#include <sstream>
#include <cstdlib>
#include <cstring> // memcpy
#include "Poco/Mutex.h"
#include "Poco/SharedMemory.h"
#include "Poco/AutoPtr.h"
#include "Poco/Util/PropertyFileConfiguration.h"
#include "Poco/NamedMutex.h"
#include <iostream>


#if !defined(CONFIG_LEAN)
#include "module/moduleLogger.h"
#endif

namespace precitec
{
	namespace interface
	{
		using Poco::SharedMemory;

		class ConnectionConfiguration
		{

        public:
            explicit ConnectionConfiguration(const std::string &stationName);

            static ConnectionConfiguration &instance();

		private:
            ConnectionConfiguration();
            static const unsigned int ShMemMaxSize = 8196;
            std::string m_stationName;
            Poco::NamedMutex m_mutex;

            const std::string &stationName() const;

			SharedMemory GetShMem(bool server = false)
			{
				SharedMemory ret(stationName() + "SgmConnConf",ShMemMaxSize,SharedMemory::AM_WRITE,0,server);
				if(ret.begin() == NULL)
				{
					throw new Poco::InvalidAccessException();
				}
				else
				{
					return ret;
				}
			}

            Poco::NamedMutex &mutex()
            {
                return m_mutex;
            }

			/**
			 * myConf in shMem schreiben
			 */
			void WriteConfigToShMem(Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> myConf)
			{
				SharedMemory myShMem = GetShMem();

				std::ostringstream ostrstr;
				myConf->save(ostrstr);
				std::string oStr = ostrstr.str();
				std::memset( myShMem.begin(), 0, ShMemMaxSize );
				memcpy( myShMem.begin(), oStr.c_str(), oStr.length()+0 );
			}

			Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> LoadConfigFromShMem()
			{
				SharedMemory myShMem = GetShMem();

				std::istringstream istrstr(myShMem.begin());
				Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> myConf = new Poco::Util::PropertyFileConfiguration();
				myConf->load(istrstr);
				return myConf;
			}

		public:

			// get
			std::string getString(const std::string& key, const std::string& defaultValue)
			{
				Poco::NamedMutex::ScopedLock lock(mutex());

				Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> pConf = LoadConfigFromShMem();
				return pConf->getString(key, defaultValue);
			}

			bool getBool(const std::string& key, bool defaultValue)
			{
				Poco::NamedMutex::ScopedLock lock(mutex());

				Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> pConf = LoadConfigFromShMem();
				return pConf->getBool(key, defaultValue);
			}

			int getInt(const std::string& key, int defaultValue)
			{
				Poco::NamedMutex::ScopedLock lock(mutex());

				Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> pConf = LoadConfigFromShMem();
				return pConf->getInt(key, defaultValue);
			}

			void getOstringstream(std::ostringstream &ostrstr)
			{
				Poco::NamedMutex::ScopedLock lock(mutex());

				Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> pConf = LoadConfigFromShMem();
				pConf->save(ostrstr);
			}

			void saveConfig(Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> conf)
			{
				WriteConfigToShMem(conf);
			}

			//set
			void setInt(const std::string& key, int value)
			{
				Poco::NamedMutex::ScopedLock lock(mutex());

				Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> pConf = LoadConfigFromShMem();
				pConf->setInt(key, value);
				WriteConfigToShMem(pConf);
			}

			void setBool(const std::string& key, bool value)
			{
				Poco::NamedMutex::ScopedLock lock(mutex());

				Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> pConf = LoadConfigFromShMem();
				pConf->setBool(key, value);
				WriteConfigToShMem(pConf);
			}

			void setString(const std::string& key, std::string value)
			{
				Poco::NamedMutex::ScopedLock lock(mutex());

				Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> pConf = LoadConfigFromShMem();
				pConf->setString(key, value);
				WriteConfigToShMem(pConf);
			}

			SharedMemory Init()
			{
                return GetShMem(true);
			}

			// dump
			void mydump()
			{
				SharedMemory myShMem = GetShMem();

				std::istringstream istrstr(myShMem.begin());
				std::stringstream oSt;
				std::string oString = istrstr.str();
				auto oIter = std::string::iterator();
				for( oIter = oString.begin() ; oIter != oString.end() ; ++oIter)
				{
					if ( (*oIter) == '\n' )
					{
						std::cout << oSt.str().c_str() << std::endl;
						oSt.str( std::string("") );
					} else
						oSt << (*oIter);
				}
				if (oSt.str().length() > 0)
					std::cout << oSt.str().c_str() << std::endl;

			}

#if !defined(CONFIG_LEAN)
			void dump()
			{
				Poco::SharedMemory shMem = GetShMem();
				if(shMem.begin() == NULL)
					wmLog( eDebug, "No connection configuration available.\n" );
				else
				{
					Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> pConf = new Poco::Util::PropertyFileConfiguration;
					std::istringstream istrstr(shMem.begin());
					std::stringstream oSt;
					std::string oString = istrstr.str();
					auto oIter = std::string::iterator();
					for( oIter = oString.begin() ; oIter != oString.end() ; ++oIter)
					{
					    if ( (*oIter) == '\n' )
					    {
					        //wmLog( eDebug, "Connect.config: %s\n", oSt.str().c_str() );
					        oSt.str( std::string("") );
					    } else
					        oSt << (*oIter);
					}
					//if (oSt.str().length() > 0)
					    //wmLog( eDebug, "Connect.config: %s\n", oSt.str().c_str() );
				}

			}
#endif
		};

		static const std::string pidKeys[] =
		{
				"ModuleManager.Pid",
				"Grabber.Pid",
				"WeldHeadControl.Pid",
				"Service.Pid",
				"Calibration.Pid",
				"Workflow.Pid",
				"InspectionControl.Pid",
				"VideoRecorder.Pid",
				"LoggerServer.Pid",
				"ECatMaster.Pid",
				"Simulation.Pid",
				"Storage.Pid",
				"Gui.Pid",
				"CHRCommunication.Pid",
                "Fieldbus.Pid",
                "Trigger.Pid",
                "TCPCommunication.Pid",
                "Scheduler.Pid"

		};

		#define MODULEMANAGER_KEY_INDEX			0
		#define GRABBER_KEY_INDEX				1
		#define WELDHEADCONTROL_KEY_INDEX		2
		#define SERVICE_KEY_INDEX				3
		#define CALIBRATION_KEY_INDEX			4
		#define WORKFLOW_KEY_INDEX				5
		#define INSPECTIONCONTROL_KEY_INDEX		6
		#define VIDEORECORDER_KEY_INDEX			7
		#define LOGGERSERVER_KEY_INDEX			8
		#define ECATMASTER_KEY_INDEX			9
		#define SIMULATION_KEY_INDEX			10
		#define STORAGE_KEY_INDEX				11
		#define GUI_KEY_INDEX					12
		#define CHRCOMMUNICATION_KEY_INDEX		13
        #define FIELDBUS_KEY_INDEX              14
        #define TRIGGER_KEY_INDEX               15
        #define TCPCOMMUNICATION_KEY_INDEX      16
        #define SCHEDULER_KEY_INDEX             17
        #define LAST_KEY_INDEX                  17



	}
}
#endif /*CONNECTIONCONFIGURATION_H_*/
