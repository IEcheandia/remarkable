#pragma once

#include <event/trigger.h>
#include <event/trigger.interface.h>
#include <event/sensor.h>
#include <event/sensor.proxy.h>
#include <common/defines.h>
#include "trigger/sequenceProvider.h"
#include "trigger/imageDataHolder.h"
#include "grabber/sharedMemoryImageProvider.h"

namespace precitec{
namespace grabber{

using interface::TSensor;
using interface::TTrigger;
using interface::TriggerContext;
using interface::TriggerInterval;
using interface::EventProxy; 

typedef std::array<int,6> ImageFromDiskParametersNoHw;
    static const std::array<std::string, std::tuple_size<ImageFromDiskParametersNoHw>::value > sKeysImageFromDiskParametersNoHw {"Window.X","Window.Y","Window.W","Window.H","Window.WMax", "Window.HMax" };


class TriggerServerNoHw: public TTrigger<AbstractInterface>
{
public:

    explicit TriggerServerNoHw(TSensor<EventProxy>& sensorProxy);

    void trigger(TriggerContext const& context) override;

    void trigger(const std::vector<int>& sensorIds, TriggerContext const& context);

    void trigger(TriggerContext const& context, TriggerInterval const& interval) override;

    void triggerMode(int mode) override;

    void triggerStop(int flag) override;

    void loadImages();

    void loadImages(Poco::Path const& path); 

    void uninit(void);

    void resetImageFromDiskNumber();

    ImageFromDiskParametersNoHw getImageFromDiskHWROI(const TriggerContext& context);

    void setSimulation(bool onoff);

    bool isSimulationStation() const; 

    void setTestProductInstance(const Poco::UUID& productInstance);

    void setTestImagesProductInstanceMode(bool set);

    void TriggerServerNoHsetTestImagesPath(std::string const& testImagesPath);

    void setTestImagesPath(std::string const& testImagesPath);

    void prepareActivity(uint32_t productNumber);

private:

    TSensor<EventProxy>& m_sensorProxy;

    Poco::FastMutex* m_serverAccess;

    ImageDataHolder m_imageDataArray[MAXIMAGE];

    int m_idx;

    int m_imgLoaded;

    bool m_isSimulation;

    Timer m_hilfsTimer;

    SequenceProvider m_sequenceProvider;

    SharedMemoryImageProvider m_noGrabberSharedMemory;

};

}
}
