#pragma once

#include <Poco/Bugcheck.h>
#include <Poco/SharedPtr.h>
#include <system/timer.h>
#include <common/product.h>
#include <common/triggerContext.h>
#include <event/trigger.h>
#include <event/trigger.proxy.h>
#include <message/device.h>
#include <message/device.server.h>
#include <message/device.proxy.h>
#include "qnxTimerNoHw.h"
#include "triggerControllerNoHw.h"
#include "triggerServerNoHw.h"


namespace precitec {
namespace trigger {

using interface::TTrigger;




class TriggerControllerNoHw
{

public:

    explicit TriggerControllerNoHw(grabber::TriggerServerNoHw* triggerServer);

    void setTriggerSource(TriggerSource source);

    void singleshot(const std::vector<int>& sensorIds, TriggerContext const& context);

    void burst(TriggerContext const& context, TriggerInterval const& interval);

    void burstAgain(unsigned int triggerDistanceSave, unsigned int nbTriggersSave);

    int stop();

    grabber::ImageFromDiskParametersNoHw getImageFromDiskHWROI() const;

    void onTrigger();

    void onTimerQnx();

    void resetImageFromDiskNumber();

    void setTestImagesPath(std::string const& testImagesPath);

    void setTestProductInstance(const Poco::UUID& productInstance);

    void setSimulation(bool onoff);

    void setTestImagesProductInstanceMode(bool set);

    void resetImageNumber();


private:

    void setupTrigger(unsigned int triggerDistance, unsigned int nbTriggers);

    void setEvent(TriggerContext const& context);

    void setEvent(TriggerContext const& context, TriggerInterval const& interval);

    grabber::TriggerServerNoHw* m_triggerServer;

    TriggerContext m_context;

    TriggerInterval m_interval;

    bool m_endless;

    long m_triggerPerInterval;

    long m_triggerDistance;

    long m_imagePerInterval;

    bool m_triggerStarted;

    system::Timer m_triggerTimer;
};

}
}

