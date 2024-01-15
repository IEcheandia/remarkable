#pragma once

#include <message/grabberStatus.interface.h>
#include <iostream>
#include "../trigger/triggerServerNoHw.h"

namespace precitec {
namespace grabber {


class GrabberStatusServerNoHw: public TGrabberStatus<AbstractInterface>
{
public:

    GrabberStatusServerNoHw(TriggerServerNoHw& triggerServer):
    m_triggerServer(triggerServer)
    {}

    bool isImgNbInBuffer(uint32_t imageNb) override
    {
        return true;
    }

    void preActivityNotification(uint32_t productNumber) override
    {
        m_triggerServer.prepareActivity(productNumber);
    }

private:
    TriggerServerNoHw& m_triggerServer;
};

}
}
