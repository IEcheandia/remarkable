#ifndef CONNECTSERVER_H_
#define CONNECTSERVER_H_
#include "Poco/Thread.h"
#include "Poco/Event.h"
#include "Poco/Util/PropertyFileConfiguration.h"
#include "Poco/AutoPtr.h"
#include "Poco/SharedMemory.h"
#include "AsyncSystemStarter.h"

#include <condition_variable>

namespace Poco
{
namespace Util
{
class XMLConfiguration;
}
}

namespace Precitec {

namespace Service {
namespace Discovery {


typedef struct
{
    bool m_10GigEIsPresent{false};
    bool m_FramegrabberIsPresent{false};
} CameraHardware;

class ConnectServerHandler : public Poco::Runnable
{

	typedef enum { Loading = 2, Running	= 1, NotRunning = 0, Error = -1 } RunState;


public:

	ConnectServerHandler();
	virtual ~ConnectServerHandler();
	void stop();
	void start();
	void run();
    void stopAll();
    void restartSystem();
    void shutdownSystem();
	std::string confFileName;

private:
	void dump();
	bool isRunning(int pid);
	int enumPids(int &currRunAppsInPercent);
	//void saveConfig(Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> conf);

    void startAll();
    void signalStop();

	void WriteTempConfToShMem(Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> myConf);
	Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> GetTempConfFromShMem();

    CameraHardware SearchForCameraHardware(void);
    void ModifyCameraKeys(CameraHardware p_oCameraHardware);
    void S6K_ModifyVIConfigSymlink(void);

	Poco::SharedMemory _shMem;
	Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> _fileConf ;
	Poco::Thread _thread;
	Poco::Event  _ready;
    std::mutex m_stopMutex;
    std::condition_variable m_stopCondition;
	bool         _stop = false;

    Poco::Thread asyncStartThread;
    AsyncSystemStarter myAsyncStarterObj;
};


}}}

#endif /*CONNECTSERVER_H_*/
