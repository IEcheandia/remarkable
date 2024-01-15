/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2012
 *  @brief			GrabberStatus Interface
 */

#ifndef GRABBERSTATUS_HANDLER_H_
#define GRABBERSTATUS_HANDLER_H_


#include "message/grabberStatus.h"
#include "message/grabberStatus.interface.h"
#include "server/handler.h"


namespace precitec
{
namespace interface
{
	using namespace  message;

	template <>
	class TGrabberStatus<MsgHandler> : public Server<MsgHandler>, public TGrabberStatusMessageDefinition
	{
	public:
		MSG_HANDLER( TGrabberStatus );
	public:
		void registerCallbacks()
		{
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
			REGISTER_MESSAGE(IsImgNbInBuffer, isImgNbInBuffer);
			REGISTER_MESSAGE(PreActivityNotification, preActivityNotification);
		}

		void isImgNbInBuffer(Receiver &p_rReceiver)
		{

			uint32_t oImageNumber	(0);
			p_rReceiver.deMarshal(oImageNumber);
			p_rReceiver.marshal(server_->isImgNbInBuffer(oImageNumber));
			p_rReceiver.reply();
		}

		void preActivityNotification(Receiver &receiver)
		{
			uint32_t oProductNumber	(0);
			receiver.deMarshal(oProductNumber);
			server_->preActivityNotification(oProductNumber);
			receiver.reply();
		}

	};

} // namespace interface
} // namespace precitec



#endif /*GRABBERSTATUS_HANDLER_H_*/
