#pragma once

#include "event/controlSimulation.interface.h"
#include "Poco/UUID.h"
namespace precitec
{

namespace interface
{
    
    template<>
    class TControlSimulation<EventServer> : public TControlSimulation<AbstractInterface>
    {
    public:
        TControlSimulation(){};
        virtual ~TControlSimulation(){};
        
    public:
        virtual void activateControlSimulation (bool p_oState) = 0;
    virtual void setInspectionCycle (bool p_oState, uint32_t p_oProductType, uint32_t oProductNumber) = 0;
    virtual void setSeamSeries (bool p_oState, uint32_t p_oSeamSeries) = 0;
    virtual void setSeam (bool p_oState, uint32_t p_oSeam) = 0;
    virtual void setCalibration (bool p_oState, uint32_t p_oMode) = 0;
    virtual void genPurposeDigIn (uint8_t p_oAddress, int16_t p_oDigInValue) = 0;
    virtual void quitSystemFault (bool p_oState) = 0;
    //Post processing testing
    virtual void productChanged(Poco::UUID productUUID)= 0;
    virtual void seamChanged(Poco::UUID productUUID, Poco::UUID seamUUID)= 0;
    };
}
}

