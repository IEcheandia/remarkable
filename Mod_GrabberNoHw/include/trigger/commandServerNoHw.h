#pragma once

#include <vector>
#include <Poco/Bugcheck.h>
#include <common/systemConfiguration.h>
#include <system/exception.h>
#include <event/triggerCmd.h>
#include <event/triggerCmd.handler.h>
#include <event/trigger.h>
#include <event/trigger.proxy.h>
#include <event/sensor.h>
#include "triggerControllerNoHw.h"
#include "commandServerNoHw.h"
#include "triggerServerNoHw.h"


namespace precitec
{
namespace trigger
{

class CommandServerNoHw: public TTriggerCmd<AbstractInterface>
{

public:
    explicit CommandServerNoHw(grabber::TriggerServerNoHw& triggerServer):
    m_triggerController(&triggerServer)
    , m_triggerDistanceSave(0)
    , m_nbTriggersSave(0)
    {
    }

    void single(const std::vector<int>& sensorIds, TriggerContext const& context) override
    {
        resetImageNumber(0); // reset image number (counter)
        m_triggerController.singleshot(sensorIds, context);
    }

    void burst(const std::vector<int>& sensorIds, TriggerContext const& context, int source, TriggerInterval const& interval) override
    {
        m_triggerDistanceSave = interval.triggerDistance();
        m_nbTriggersSave  = interval.nbTriggers();
        m_triggerController.setTriggerSource(TriggerSource(source));
        m_triggerController.burst(context, interval);
    }

    void burstAgain(void)
    {
        m_triggerController.burstAgain(m_triggerDistanceSave, m_nbTriggersSave);
    }

    void cancel(const std::vector<int>& sensorIds) override
    {
        std::cout<<"Trigger stoppen"<<std::endl;
        m_triggerController.stop();
    }

    int cancelSpecial(const std::vector<int>& sensorIds)
    {
        std::cout<<"Trigger stoppen"<<std::endl;
        const int status = m_triggerController.stop();
        return status;
    }

    void resetImageNumber(int id)
    {
        std::cout << "CommandServer<Server>::resetImageNumber id: " << id << std::endl;
        m_triggerController.resetImageNumber();
    }

    grabber::ImageFromDiskParametersNoHw getImageFromDiskHWROI() const
    {
        return m_triggerController.getImageFromDiskHWROI();
    }

    void resetImageFromDiskNumber()
    {
        m_triggerController.resetImageFromDiskNumber();
    }

    void setTestProductInstance(const Poco::UUID& productInstance)
    {
        m_triggerController.setTestProductInstance(productInstance);
    }

    void setSimulation(bool onoff)
    {
        m_triggerController.setSimulation(onoff);
    }

    void setTestImagesProductInstanceMode(bool set)
    {
        m_triggerController.setTestImagesProductInstanceMode(set);
    }

    void setTestImagesPath(std::string const& testImagesPath)
    {
        m_triggerController.setTestImagesPath(testImagesPath);
    }

private:

    TriggerControllerNoHw m_triggerController;
    unsigned int m_triggerDistanceSave;
    unsigned int m_nbTriggersSave;
};

}
}

