#pragma once

// wm includes
#include "message/device.interface.h"
// project includes
#include "camera.h"
// stl includes
#include <iostream>

namespace precitec {
namespace grabber  {


class DeviceServer : public TDevice<AbstractInterface>
{
public:
    
	DeviceServer(Camera &p_rCamera) :
		m_rCamera( p_rCamera )
	{}

	/*virtual*/ int initialize(const Configuration &p_rConfig, int p_oSubDevice = 0) 
    {
		return -1;
	} 

	/*virtual*/ void uninitialize() 
    {
	} // uninitialize

	/*virtual*/ void reinitialize() 
    {
	} 

	/*virtual*/ KeyHandle set(SmpKeyValue p_oSmpKeyValue, int p_oSubDevice=0) 
    {
		return m_rCamera.set(p_oSmpKeyValue);
	} 

	/*virtual*/ void set(Configuration p_rConfig, int p_oSubDevice=0) 
    {
        for( auto smpKeyValue : p_rConfig ) { m_rCamera.set( smpKeyValue ); }
	}

	/*virtual*/  SmpKeyValue get(Key p_oKey, int p_oSubDevice=0) 
    {
		return m_rCamera.get(p_oKey);
	} 

	/*virtual*/ SmpKeyValue get(KeyHandle p_oKeyHandle, int p_oSubDevice=0) 
    {
		return SmpKeyValue(new KeyValue(TInt,"?",-1) );
	}

	/*virtual*/ Configuration get(int p_oSubDevice=0) 
    {
		return m_rCamera.getConfig();
	}

private:
    
	Camera& m_rCamera;
	
};


}	// grabber
}	// precitec
