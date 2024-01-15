#pragma once

// project includes
#include "camera.h"
// wm includes
#include "event/triggerCmd.h"
#include "event/triggerCmd.handler.h"


namespace precitec {
namespace grabber  {


class TriggerCommandServer : public TTriggerCmd<AbstractInterface>
{

public:
    
    TriggerCommandServer(Camera &p_rCamera)
        : m_rCamera( p_rCamera )
    {
    }

public:
    
    virtual void single(const std::vector<int>& p_rSensorIds, TriggerContext const& context)
    {
        if (isImageSensorMissing(p_rSensorIds))
        {
            return; // this is not the appropriate receiver - abort
        }
        m_rCamera.burst( context, TriggerInterval( 100000000, 1 ) );        
    }

    virtual void burst(const std::vector<int>& p_rSensorIds, TriggerContext const& context, int source, TriggerInterval const& interval)
    {  
        
        if (isImageSensorMissing(p_rSensorIds))
        {
            return; // this is not the appropriate receiver - abort
        }
        m_rCamera.burst( context, interval );
    }

    virtual void cancel(const std::vector<int>& p_rSensorIds)
    {
        // todo - pay attention to sensorIds  and sensor -1 (cancel all sensors)
        m_rCamera.cancel();
    }
        
private:
    
    static bool isImageSensorMissing(const std::vector<int>& p_rSensorIds)
    {
        const auto oSensorIdIsAnImageId	=	[](int p_oSensorId) {
                return p_oSensorId >= interface::eImageSensorMin && p_oSensorId <= interface::eImageSensorMax; };

        return std::none_of(std::begin(p_rSensorIds), std::end(p_rSensorIds), oSensorIdIsAnImageId);
    }
	Camera& m_rCamera;

};

} // namespace grabber
} // namespace precitec


