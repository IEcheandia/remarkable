/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       23.11.2016
 *  @brief      Part of the EthercatOutputs Interface
 *  @details
 */

#ifndef ETHERCATOUTPUTS_INTERFACE_H_
#define ETHERCATOUTPUTS_INTERFACE_H_

#include "server/interface.h"
#include "module/interfaces.h" // wg appId
#include "protocol/protocol.info.h"

#include "event/ethercatOutputs.h"

/*
 * Hier werden die abstrakten Baisklassen, sowie die
 * Messages definiert. Dieser Header wird sowohl für
 * den lokalen (Implementation, MsgHandler) sowie den
 * remote-Teil (Proxyer) benötigt.
 */

namespace precitec
{

namespace interface
{
	using namespace  system;
	using namespace  message;
	//----------------------------------------------------------
	// Hier folgen die Template-Definitionen für die verschiedenen
	// Spezialisierungen  <Implementation> <MsgHandler> <Proxyer> <Messages>

	template <int mode>
	class TEthercatOutputs;

	//----------------------------------------------------------
	// Abstrakte Basis Klassen  = Interface
	// Die <Implementation> und <Proxy> Spezialisierungen leiten von diesen
	// Basisklassen ab.

	/**
	 * Signaler zeigt auf primitive Weise den Systemzustand an.
	 * Der State-Enum bietet drei Zustände an. Verschiedene
	 * Handler können diese Zustände unterschiedlich darstellen.
	 */
	template<>
	class TEthercatOutputs<AbstractInterface>
	{
	public:
		TEthercatOutputs() {}
		virtual ~TEthercatOutputs() {}
	public:
		/// Interface: EthercatOutputs : ecatDigitalOut
		virtual void ecatDigitalOut (EcatProductIndex productIndex, EcatInstance instance, uint8_t value, uint8_t mask) = 0;
		/// Interface: EthercatOutputs : ecatAnalogOut
		virtual void ecatAnalogOut (EcatProductIndex productIndex, EcatInstance instance, EcatChannel channel, uint16_t value) = 0;
		/// Interface: EthercatOutputs : ecatGatewayOut
		virtual void ecatGatewayOut (EcatProductIndex productIndex, EcatInstance instance, uint8_t size, stdVecUINT8 data, stdVecUINT8 mask) = 0;
		/// Interface: EthercatOutputs : ecatEncoderOut
		virtual void ecatEncoderOut (EcatProductIndex productIndex, EcatInstance instance, uint16_t command, uint32_t setCounterValue) = 0;
		/// Interface: EthercatOutputs : ecatAxisOut
		virtual void ecatAxisOut (EcatProductIndex productIndex, EcatInstance instance, EcatAxisOutput axisOutput) = 0;
		/// Interface: EthercatOutputs : ecatRequestSlaveInfo
		virtual void ecatRequestSlaveInfo () = 0;
        /**
         * Whether all data should be sent depending on @p enable.
         **/
        virtual void sendAllData(bool enable) = 0;
		/// Interface: EthercatOutputs : ecatFRONTENDOut
		virtual void ecatFRONTENDOut (EcatProductIndex productIndex, EcatInstance instance, EcatFRONTENDOutput frontendOutput) = 0;

	};

    struct TEthercatOutputsMessageDefinition
    {
		EVENT_MESSAGE(EcatDigitalOut, EcatProductIndex, EcatInstance, uint8_t, uint8_t);
		EVENT_MESSAGE(EcatAnalogOut, EcatProductIndex, EcatInstance, EcatChannel, uint16_t);
		EVENT_MESSAGE(EcatGatewayOut, EcatProductIndex, EcatInstance, uint8_t, stdVecUINT8, stdVecUINT8);
		EVENT_MESSAGE(EcatEncoderOut, EcatProductIndex, EcatInstance, uint16_t, uint32_t);
		EVENT_MESSAGE(EcatAxisOut, EcatProductIndex, EcatInstance, EcatAxisOutput);
		EVENT_MESSAGE(EcatRequestSlaveInfo, void);
        EVENT_MESSAGE(SendAllData, bool);
		EVENT_MESSAGE(EcatFRONTENDOut, EcatProductIndex, EcatInstance, EcatFRONTENDOutput);

		MESSAGE_LIST(
			EcatDigitalOut,
			EcatAnalogOut,
			EcatGatewayOut,
			EcatEncoderOut,
			EcatAxisOut,
			EcatRequestSlaveInfo,
            SendAllData,
			EcatFRONTENDOut
		);
    };

	//----------------------------------------------------------
	template <>
	class TEthercatOutputs<Messages> : public Server<Messages>, public TEthercatOutputsMessageDefinition
	{
	public:
		TEthercatOutputs<Messages>() : info(system::module::EthercatOutputs, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
		MessageInfo info;
	private:
		/// Konstanten wg Lesbarkeit, diese könnten auch in der Basisklasse stehen, würden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 500*Bytes, replyBufLen = 100*Bytes, NumBuffers=128 };
	};

} // namespace interface
} // namespace precitec

#endif /* ETHERCATOUTPUTS_INTERFACE_H_ */

