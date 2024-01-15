/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief		Implements the grabberStatus interface.
 */

#ifndef GRABBERSTATUSSERVER_H_20121017_INCLUDED
#define GRABBERSTATUSSERVER_H_20121017_INCLUDED

// project includes
#include "message/grabberStatus.interface.h"
#include "dataAcquire.h"
#include "triggerServer.h"
// stl includes
#include <iostream>

namespace precitec {
namespace grabber {


/**
 * @brief	Server implementation for video recorder commands.
 */
class GrabberStatusServer : public TGrabberStatus<AbstractInterface>
{
public:
	/**
	 * @brief CTOR.
	 * @param p_rGrabber Reference to grabber that executes actions.
	 */
	GrabberStatusServer(DataAcquire &p_rGrabber, TriggerServer &p_rTriggerServer) :
		m_rGrabber( p_rGrabber ),
		m_rTriggerServer( p_rTriggerServer )
	{} // GrabberStatusServer

	/**
	 * @brief				returns if the given image number belongs to an image in the grabber dma buffer (true), or if it is expired (false)
	 * @param	p_oImageNb	image number
	 * @return	bool		if the given image number belongs to an image in the grabber dma buffer
	 */
	/*virtual*/ bool isImgNbInBuffer(uint32_t p_oImageNb) {
		return m_rGrabber.isImgNbInBuffer(p_oImageNb);
	} // isImgNbInBuffer

	/**
	 * @brief				prepare anything for automatic mode
	 */
	void preActivityNotification(uint32_t _productNumber)
	{
	}


private:
	DataAcquire             &m_rGrabber;		   ///< Grabber/Kamera
	TriggerServer           &m_rTriggerServer;	   ///< TriggerServer for loadImages
}; // GrabberStatusServer


}	// grabber
}	// precitec

#endif // GRABBERSTATUSSERVER_H_20121017_INCLUDED
