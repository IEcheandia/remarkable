#pragma once

#include <string>
#include <message/device.h>
#include <message/device.interface.h>
#include <grabber/dataAcquire.h>
#include "../trigger/commandServerNoHw.h"


namespace precitec {
namespace grabber {

using namespace trigger; 

class DeviceServerNoHw: public TDevice<AbstractInterface>
{
public:

    explicit DeviceServerNoHw(CommandServerNoHw& triggerCmdserver);

public:

    int initialize(Configuration const& config, int subDevice) override;

    void uninitialize() override;

    void reinitialize() override;

    KeyHandle set(SmpKeyValue keyValue, int subDevice=0) override;

    void set(Configuration config, int subDevice=0) override;

    SmpKeyValue get(Key key, int subDevice=0) override;

    SmpKeyValue get(KeyHandle handle, int subDevice=0) override;

    Configuration get(int subDevice) override;

    int checkDangerousKey(std::string keyString);

    bool checkAndSetGlobalParameter(SmpKeyValue keyValue);

private:

    CommandServerNoHw& m_triggerCmdServer;
    bool m_deviceOK;

};

}
}
