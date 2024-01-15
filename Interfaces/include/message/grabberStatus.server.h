/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2012
 *  @brief			GrabberStatus Interface
 */

#ifndef GRABBERSTATUS_SERVER_H_
#define GRABBERSTATUS_SERVER_H_



#include "message/videoRecorderCmd.interface.h"

namespace precitec
{
	using namespace  system;
	using namespace  message;

namespace interface
{

	template <>
	class TGrabberStatus<MsgServer> : public TGrabberStatus<AbstractInterface>
	{
	public:
		TGrabberStatus(){}
		virtual ~TGrabberStatus() {}
	public:

		// start recording
		/*virtual*/ bool isImgNbInBuffer(uint32_t p_oImageNb){ return false; }

		virtual void preActivityNotification(uint32_t p_oProductNb)
		{
		}

	};


} // interface
} // precitec



#endif /*GRABBERSTATUS_SERVER_H_*/
