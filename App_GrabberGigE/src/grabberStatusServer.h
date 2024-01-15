#pragma once
#include "camera.h"

// wm includes
#include "message/grabberStatus.interface.h"
// stl includes
#include <iostream>

namespace precitec {
namespace grabber  {

class GrabberStatusServer : public TGrabberStatus<AbstractInterface>
{
public:

    GrabberStatusServer( )
	{} 

	/*virtual*/ bool isImgNbInBuffer(uint32_t p_oImageNb) 
    {
        if (!m_camera)
        {
            return false;
        }
        return m_camera->isImageInSharedMemory(p_oImageNb);
	}

	void preActivityNotification(uint32_t _productNumber)
	{
		//m_rTriggerServer.prepareActivity(_productNumber);
	}

    void setCamera(Camera *camera)
    {
        m_camera = camera;
    }

private:
    Camera *m_camera = nullptr;

};


}	// grabber
}	// precitec
