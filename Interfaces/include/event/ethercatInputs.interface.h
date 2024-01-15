/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       19.11.2016
 *  @brief      Part of the EthercatInputs Interface
 *  @details
 */

#ifndef ETHERCATINPUTS_INTERFACE_H_
#define ETHERCATINPUTS_INTERFACE_H_

#include "server/interface.h"
#include "module/interfaces.h" // wg appId
#include "protocol/protocol.info.h"

#include "event/ethercatInputs.h"

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
	class TEthercatInputs;

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
	class TEthercatInputs<AbstractInterface>
	{
	public:
		TEthercatInputs() {}
		virtual ~TEthercatInputs() {}
	public:
        virtual void ecatData(const EtherCAT::EcatInData &data) = 0;

	};

    struct TEthercatInputsMessageDefinition
    {
        EVENT_MESSAGE(EcatData, EtherCAT::EcatInData);
		MESSAGE_LIST(
            EcatData
		);
    };

	//----------------------------------------------------------
	template <>
	class TEthercatInputs<Messages> : public Server<Messages>, public TEthercatInputsMessageDefinition
	{
	public:
		TEthercatInputs<Messages>() : info(system::module::EthercatInputs, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
		MessageInfo info;
	private:
		/// Konstanten wg Lesbarkeit, diese könnten auch in der Basisklasse stehen, würden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 2000*Bytes, replyBufLen = 100*Bytes, NumBuffers=128 };

	};

} // namespace interface
} // namespace precitec

#endif /* ETHERCATINPUTS_INTERFACE_H_ */

