/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2012
 *  @brief			GrabberStatus Interface
 */

#ifndef GRABBERSTATUS_PROXY_H_
#define GRABBERSTATUS_PROXY_H_


#include "server/proxy.h"
#include "message/grabberStatus.interface.h"


namespace precitec
{
namespace interface
{

	template <>
	class TGrabberStatus<MsgProxy> : public Server<MsgProxy>, public TGrabberStatus<AbstractInterface>, public TGrabberStatusMessageDefinition
	{
	public:
		/// Der CTor braucht die Klasse, die die Arbeit erledigt, also den eigentlichen Server
		TGrabberStatus() : PROXY_CTOR(TGrabberStatus),
				TGrabberStatus<AbstractInterface>() {}

		TGrabberStatus(SmpProtocolInfo &p) : PROXY_CTOR1(TGrabberStatus,  p),
				TGrabberStatus<AbstractInterface>()	{}

		virtual ~TGrabberStatus() {}

	public:
		/*virtual*/ bool isImgNbInBuffer(uint32_t p_oImageNb)
		{
			INIT_MESSAGE(IsImgNbInBuffer);
			sender().marshal(p_oImageNb);
			sender().send();
			bool oImgNbIsInBuffer	(false);
			sender().deMarshal(oImgNbIsInBuffer);
			return oImgNbIsInBuffer;
		}

		virtual void preActivityNotification(uint32_t p_oProductNb)
		{
			INIT_MESSAGE(PreActivityNotification);
			sender().marshal(p_oProductNb);
			sender().send();
		}


	};

} // interface
} // precitec


#endif /*GRABBERSTATUS_PROXY_H_*/
