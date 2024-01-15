#include <module/moduleLogger.h>
#include "../include/grabber/deviceServerNoHw.h"


namespace precitec
{
namespace grabber
{

DeviceServerNoHw::DeviceServerNoHw(CommandServerNoHw& triggerCmdserver):
    m_triggerCmdServer(triggerCmdserver)
{
    std::cout<<"Device Sertver CTOR..."<<std::endl;
}


int DeviceServerNoHw::initialize(Configuration const& config, int subDevice)
{
    return 1;
}

void DeviceServerNoHw::set(Configuration config, int subDevice)
{

}


SmpKeyValue DeviceServerNoHw::get(KeyHandle handle, int subDevice)
{
    return SmpKeyValue(new KeyValue(TInt,"?",-1) );
}



KeyHandle DeviceServerNoHw::set(SmpKeyValue keyValue, int subDevice)
{
    std::string keyString =keyValue->key();
    std::string valueString;
    bool liveStop = false;
    int liveInterrupted = 0;

    std::ostringstream os;

    if(checkAndSetGlobalParameter(keyValue))
    {
        return KeyHandle(1);
    }

    if(!m_deviceOK)
    {
        return KeyHandle(); // setzt handle auf -1
    }

    int dKey = checkDangerousKey(keyString);

    if(dKey > 0)
    {
        liveInterrupted = m_triggerCmdServer.cancelSpecial(std::vector<int>(1, 0));
        if(liveInterrupted==1)// live mode lief und wurde angehalten
        {
            usleep(70 * 1000); // gibt Zeit das letzte Bild zu uebertragen
            liveStop = true;
        }
    }

    if(liveStop)
    {
        //std::cout<<"burst again ..."<<std::endl;
        m_triggerCmdServer.burstAgain();
        liveStop = false;
    }
    return KeyHandle(1);
}


SmpKeyValue DeviceServerNoHw::get(Key key, int subDevice)
{
    SensorConfigVector confVector;

    if (key == "hasCamera")
    {
        SmpKeyValue kv {new TKeyValue<bool>{"hasCamera", false, true,  false, false}};
        kv->setReadOnly(true);
        return kv;
    }
    if (key == "ReloadImagesFromDisk" )
    {
        return SmpKeyValue{new TKeyValue<bool>{"ReloadImagesFromDisk", false, true, false, false}};
    }

    auto oHWROI = m_triggerCmdServer.getImageFromDiskHWROI();
    auto it = std::find(sKeysImageFromDiskParametersNoHw.begin(), sKeysImageFromDiskParametersNoHw.end(), key);
    if (it != sKeysImageFromDiskParametersNoHw.end())
    {
        int i = it - sKeysImageFromDiskParametersNoHw.begin();
        SmpKeyValue oKeyVal {new TKeyValue<int>{key, oHWROI[i], 0, 1024, 0}};
        oKeyVal->setReadOnly(true);
        return oKeyVal;
    }
    return SmpKeyValue (new KeyValue(TInt,key +"?",-1) );
}


Configuration DeviceServerNoHw::get(int subDevice)
{
    Configuration config;
    config.push_back(get("hasCamera", subDevice));
    config.push_back(get("ReloadImagesFromDisk", subDevice));
    for (auto& hwROIKey : sKeysImageFromDiskParametersNoHw)
    {
        config.push_back(get(hwROIKey, subDevice));
    }
    return config;
}


void DeviceServerNoHw::uninitialize()
{
}

void DeviceServerNoHw::reinitialize()
{
}

int DeviceServerNoHw::checkDangerousKey(std::string keyString)
{
    std::string::size_type found;
    found=keyString.find("PWM");
    if(found != std::string::npos)
    {
        return 1;
    }
    found=keyString.find("Exp");
    if(found != std::string::npos)
    {
        return 2;
    }
    return 0;
}

bool DeviceServerNoHw::checkAndSetGlobalParameter(SmpKeyValue keyValue)
{
    ValueTypes keyType  = keyValue->type();
    std::string keyString = keyValue->key();

    if(keyString == "TestimagesPath")
    {
        if (keyType == TString)
        {
            std::string pathName = keyValue->value<std::string>();
            m_triggerCmdServer.setTestImagesPath(pathName);
            return true;
        }
    }
    else if (keyString == "TestProductInstance")
    {
        if (keyType == TString)
        {
            m_triggerCmdServer.setTestProductInstance(Poco::UUID{keyValue->value<std::string>()});
            return true;
        }
    }
    else if(keyString == "SimulateCamera")
    {
        if (keyType == TBool)
        {
            bool yesno = static_cast<bool>(keyValue->value<bool>());
            m_triggerCmdServer.setSimulation(yesno);
            return true;
        }
    }
    else if (keyString == "TestimagesProductInstanceMode")
    {
        if (keyType == TBool)
        {
            m_triggerCmdServer.setTestImagesProductInstanceMode(keyValue->value<bool>());
        }
    }
    else if (keyString == "ReloadImagesFromDisk")
    {
        if (keyType == TBool)
        {
            m_triggerCmdServer.resetImageFromDiskNumber();
            return true;
        }
    }
    return false;
}


}
}
