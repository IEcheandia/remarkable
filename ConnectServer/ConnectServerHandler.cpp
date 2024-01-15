#include "ConnectServerHandler.h"
#include "Poco/DirectoryIterator.h"
#include <Poco/UUIDGenerator.h>

#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <filesystem>
#include "common/connectionConfiguration.h"
#include "common/systemConfiguration.h"

using namespace precitec::interface;
namespace fs = std::filesystem;

namespace Precitec {
namespace Service {
namespace Discovery {


	ConnectServerHandler::ConnectServerHandler() :
		_thread("ConnectServerHandler"),
		asyncStartThread("AsyncStarter")
	{
		std::string homeDir(getenv("WM_BASE_DIR"));
		confFileName = homeDir + "/system_config/Connect.config";
	}


	ConnectServerHandler::~ConnectServerHandler()
	{
        signalStop();
		if(_thread.isRunning())
			_thread.join();
	}

	void ConnectServerHandler::stop()
	{
        signalStop();
		if(_thread.isRunning())
			_thread.join();
	}

    void ConnectServerHandler::signalStop()
    {
        std::unique_lock<std::mutex> lock{m_stopMutex};
        _stop = true;
        m_stopCondition.notify_all();
    }

	void ConnectServerHandler::start()
	{
		static const std::string confPath = confFileName;
		_fileConf = new Poco::Util::PropertyFileConfiguration(confPath);

        const std::string stationName = _fileConf->getString("Station.Name", "WM-QNX-PC");
        setenv("WM_STATION_NAME", stationName.c_str(), 0);

        if (stationName != "SIMULATION")
        {
            _fileConf->setBool("Simulation.Watch", false);
        }
        if (!_fileConf->has("Storage.Watch"))
        {
            _fileConf->setBool("Storage.Watch", false);
        }
        if (_fileConf->has("Communicator.Watch"))
        {
            _fileConf->setBool("Communicator.Watch", false);
        }

		// second read the ConnectConfig-File and init the Shared Memory
		_shMem = ConnectionConfiguration::instance().Init();

		ConnectionConfiguration::instance().saveConfig(_fileConf);
		ConnectionConfiguration::instance().mydump();

        auto oCameraHardware = SearchForCameraHardware();
        ModifyCameraKeys(oCameraHardware);

        S6K_ModifyVIConfigSymlink();

		_thread.start(*this);
		_ready.wait();
	}

	bool ConnectServerHandler::isRunning(int pid)
	{
		bool ret = false;
		const Poco::DirectoryIterator	oEndIt;
		Poco::DirectoryIterator		oPidIt			( Poco::Path("/proc/"));
		std::stringstream strstr;
		strstr << pid;
		std::string		oPidName = strstr.str();
		while (oPidIt != oEndIt)
		{
			const std::string oPathName	( oPidIt.path().getBaseName() );
			if(oPathName == oPidName)
			{
				ret = true;
				break;
			}
			oPidIt++;
		}
		return ret;
	}

	int ConnectServerHandler::enumPids(int &currRunAppsInPercent)
	{
		RunState ret = NotRunning;
		int nRunning=0;
		int expectedNumberOfApps = LAST_KEY_INDEX+1;
		currRunAppsInPercent = 0;
		nRunning = 0;
		for(int i=0; i<=LAST_KEY_INDEX; ++i)
		{
			size_t dotPos;
			dotPos = precitec::interface::pidKeys[i].rfind('.');
			if(dotPos==std::string::npos)
			{
				continue;
			}

			std::string appName = precitec::interface::pidKeys[i].substr( 0, dotPos );
			std::string appNameWatch = appName;
			std::string appNameIsAlive = appName;

			appNameWatch.append(".Watch");
			appNameIsAlive.append(".IsAlive");

			bool doWatch = ConnectionConfiguration::instance().getBool(appNameWatch,true);

			if( !doWatch )
			{
				expectedNumberOfApps--;
				currRunAppsInPercent = (int)((((double)nRunning / (double)expectedNumberOfApps)) * 100.0);
				continue;
			}

			int pid = ConnectionConfiguration::instance().getInt(precitec::interface::pidKeys[i],0);
			if(pid==0)
			{
				ConnectionConfiguration::instance().setBool(appNameIsAlive, false);
			}
			else
			{
				bool isAppRunning = isRunning(pid);
				if(isAppRunning)
				{
					nRunning++;
					currRunAppsInPercent = (int)((((double)nRunning / (double)expectedNumberOfApps)) * 100.0);
				}

				ConnectionConfiguration::instance().setBool(appNameIsAlive, isAppRunning);
			}
		}

		if(nRunning==0)
		{
			ret = NotRunning;
		}
		else if(nRunning<(expectedNumberOfApps))
		{
			ret = Error;
		}
		else if(nRunning>=(expectedNumberOfApps))
		{
			ret = Running;
		}

		return ret;
	}


	void ConnectServerHandler::run()
	{

		_ready.set();
		char hostname[MAXHOSTNAMELEN];
		gethostname(hostname, sizeof(hostname));

		if (ConnectionConfiguration::instance().getString("Station.UUID","").compare("") == 0)
		{
            Poco::UUIDGenerator& gen = Poco::UUIDGenerator::defaultGenerator();
            Poco::UUID uuid = gen.create();
			ConnectionConfiguration::instance().setString("Station.UUID", uuid.toString());
		}

		ConnectionConfiguration::instance().setString("Host.Name", hostname);
		ConnectionConfiguration::instance().setString("Service.Name", "QNX_ConnectServer");
        ConnectionConfiguration::instance().setInt("ConnectServerPid", getpid());

        startAll();
        int running = NotRunning;
        while (running != Running)
        {
            {
                // scoped, to ensure only the test is locked
                std::unique_lock<std::mutex> lock{m_stopMutex};
                if (_stop)
                {
                    break;
                }
            }
            Poco::Thread::sleep(1000);
            int dummy;
            running = enumPids(dummy);
        }
        std::unique_lock<std::mutex> lock{m_stopMutex};
        m_stopCondition.wait(lock, [this] { return _stop = true; });
	}

void ConnectServerHandler::stopAll()
{
    myAsyncStarterObj.setShuttingDown();
    for (int i = 0; i <= LAST_KEY_INDEX; i++)
    {
        const auto pid = ConnectionConfiguration::instance().getInt(pidKeys[i], 0);
        if (pid == 0)
        {
            continue;
        }
        kill(pid, SIGTERM);
        ConnectionConfiguration::instance().setInt(pidKeys[i], 0);
    }
}

void ConnectServerHandler::startAll()
{
    std::cout << "Before asyncStart..." << std::endl;
    myAsyncStarterObj.setConfigFileName(ConnectionConfiguration::instance().getString("Start.Config", "startup.config"));
    asyncStartThread.start(myAsyncStarterObj);

    std::cout << "After asyncStart..." << std::endl;
}

void ConnectServerHandler::restartSystem()
{
    stopAll();
    system("systemctl reboot --ignore-inhibitors");
}

void ConnectServerHandler::shutdownSystem()
{
    stopAll();
    system("systemctl poweroff --ignore-inhibitors");
}

CameraHardware ConnectServerHandler::SearchForCameraHardware(void)
{
    bool o10GigEIsPresent{false};
    bool oFramegrabberIsPresent{false};

    try
    {
        fs::path start{"/sys/devices"};
        fs::recursive_directory_iterator directoryIt{start}, end;
        while (directoryIt != end)
        {
            if (!fs::is_directory(directoryIt->path()))
            {
                if (directoryIt->path().filename() == fs::path("subsystem_vendor"))
                {
                    uint32_t oVendor{0x0000};
                    {
                        fs::path oVendorFile{directoryIt->path().parent_path()};
                        oVendorFile /= fs::path("vendor");
                        FILE* pFileHandle = fopen(oVendorFile.string().c_str(), "rb");
                        if (pFileHandle != nullptr)
                        {
                            char oBuffer[20];
                            fgets(oBuffer, sizeof(oBuffer), pFileHandle);
                            oVendor = strtol(oBuffer, nullptr, 16);
                        }
                        else
                        {
                            std::cerr << "Cannot open vendor file" << std::endl;
                        }
                        fclose(pFileHandle);
                    }
                    uint32_t oDevice{0x0000};
                    {
                        fs::path oVendorFile{directoryIt->path().parent_path()};
                        oVendorFile /= fs::path("device");
                        FILE* pFileHandle = fopen(oVendorFile.string().c_str(), "rb");
                        if (pFileHandle != nullptr)
                        {
                            char oBuffer[20];
                            fgets(oBuffer, sizeof(oBuffer), pFileHandle);
                            oDevice = strtol(oBuffer, nullptr, 16);
                        }
                        else
                        {
                            std::cerr << "Cannot open device file" << std::endl;
                        }
                        fclose(pFileHandle);
                    }
                    std::cout << "found PCI Device:" << std::hex << " 0x" << oVendor << " 0x" << oDevice << std::endl;

                    if ((oVendor == 0x1fc9) && (oDevice == 0x4027))
                    {
                        o10GigEIsPresent = true;
                        std::cout << "10GigE Cu Tehuti\n";
                    }
                    else if ((oVendor == 0x1fc9) && (oDevice == 0x4022))
                    {
                        o10GigEIsPresent = true;
                        std::cout << "10GigE Fiber Tehuti\n";
                    }
                    else if ((oVendor == 0x1d6a) && (oDevice == 0x07b1))
                    {
                        o10GigEIsPresent = true;
                        std::cout << "10GigE Cu Aquantia/Marvell\n";
                    }
                    else if ((oVendor == 0x1ae8) && (oDevice == 0x0a41))
                    {
                        oFramegrabberIsPresent = true;
                        std::cout << "SiSo me4\n";
                    }
                }
            }
            ++directoryIt;
        }
    }
    catch(fs::filesystem_error const& oFilesystemException)
    {
        std::cerr << "Identification of pci devices failed: " << oFilesystemException.code().message() << std::endl;
    }

    return CameraHardware{o10GigEIsPresent, oFramegrabberIsPresent};
}

void ConnectServerHandler::ModifyCameraKeys(CameraHardware p_oCameraHardware)
{
    bool oFoundHasCamera{false};
    SystemConfiguration::instance().getBool("HasCamera", false, &oFoundHasCamera);
    bool oFoundCameraInterfaceType{false};
    SystemConfiguration::instance().getInt("CameraInterfaceType", 0, &oFoundCameraInterfaceType);

    if (!p_oCameraHardware.m_10GigEIsPresent && !p_oCameraHardware.m_FramegrabberIsPresent) // no camera hardware present
    {
        if (!oFoundHasCamera) // key HasCamera is not present in SystemConfig.xml
        {
            SystemConfiguration::instance().setBool("HasCamera", false);
        }
    }
    else if (p_oCameraHardware.m_10GigEIsPresent && !p_oCameraHardware.m_FramegrabberIsPresent) // 10GigE hardware is present
    {
        if (!oFoundHasCamera) // key HasCamera is not present in SystemConfig.xml
        {
            SystemConfiguration::instance().setBool("HasCamera", true);
        }
        if (!oFoundCameraInterfaceType) // key CameraInterfaceType is not present in SystemConfig.xml
        {
            SystemConfiguration::instance().setInt("CameraInterfaceType", 1);
        }
    }
    else if (!p_oCameraHardware.m_10GigEIsPresent && p_oCameraHardware.m_FramegrabberIsPresent) // framegrabber is present
    {
        if (!oFoundHasCamera) // key HasCamera is not present in SystemConfig.xml
        {
            SystemConfiguration::instance().setBool("HasCamera", true);
        }
        if (!oFoundCameraInterfaceType) // key CameraInterfaceType is not present in SystemConfig.xml
        {
            SystemConfiguration::instance().setInt("CameraInterfaceType", 0);
        }
    }
    else // this should not be the case ! The PC contains a 10GigE board AND a framegrabber !
    {
        std::cerr << "The PC contains a 10GigE board AND a framegrabber !\n" ;
        std::cerr << "Please correct the hardware structure !\n" ;
        std::cerr << "Camera hardware is switched off !\n";
        SystemConfiguration::instance().setBool("HasCamera", false);
    }
}

void ConnectServerHandler::S6K_ModifyVIConfigSymlink(void)
{
    bool oIsSOUVIS6000_Application = SystemConfiguration::instance().getBool("SOUVIS6000_Application", false);

    if (oIsSOUVIS6000_Application)
    {
        try
        {
            SOUVIS6000MachineType oSOUVIS6000_Machine_Type = static_cast<SOUVIS6000MachineType>(SystemConfiguration::instance().getInt("SOUVIS6000_Machine_Type", 0) );

            std::string oHomeDir{getenv("WM_BASE_DIR")};

            std::string oVIConfigLinkName = oHomeDir + "/config/VI_Config.xml";
            std::cout << "oVIConfigLinkName:   " << oVIConfigLinkName << std::endl;
            fs::path VIConfigLink{oVIConfigLinkName};

            std::string oVIConfigTargetName{oHomeDir + "/config_templates/VI_Config_Standard.xml"};
            if (oSOUVIS6000_Machine_Type == SOUVIS6000MachineType::eS6K_SouRing)
            {
                oVIConfigTargetName = oHomeDir + "/config_souvis6000/VI_Config_S6K_SOURING.xml";
            }
            else if ((oSOUVIS6000_Machine_Type == SOUVIS6000MachineType::eS6K_SouSpeed) || (oSOUVIS6000_Machine_Type == SOUVIS6000MachineType::eS6K_SouBlate))
            {
                oVIConfigTargetName = oHomeDir + "/config_souvis6000/VI_Config_S6K_SOUSPEED.xml";
            }
            std::cout << "oVIConfigTargetName: " << oVIConfigTargetName << std::endl;
            fs::path VIConfigTarget{oVIConfigTargetName};

            fs::remove(VIConfigLink);
            std::cout << "Target: " << VIConfigTarget.string() << " Link: " << VIConfigLink.string() << std::endl;
            fs::create_symlink(VIConfigTarget, VIConfigLink);
        }
        catch (fs::filesystem_error const& oFilesystemException)
        {
            std::cerr << "Modification of link VI_Config.xml failed: " << oFilesystemException.code().message() << std::endl;
        }
    }
}

}}}

