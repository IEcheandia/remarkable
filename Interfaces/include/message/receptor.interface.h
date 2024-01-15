#ifndef RECEPTOR_INTERFACE_H_
#define RECEPTOR_INTERFACE_H_

#include <iostream>
#include  "server/interface.h"
#include  "module/interfaces.h" // wg appId
#include  "protocol/protocol.info.h"
#include  "protocol/protocol.udp.h"

#include  "message/module.h"
#include  "message/registrar.iterators.h"

namespace precitec
{
	using system::message::ProtocolInfo;
	using system::message::Message;
	using system::message::UdpProtocol;
	using system::message::SmpProtocolInfo;
	using system::module::AnyModule;
	using system::module::Receptor;

namespace interface
{

/*
	struct TwoProtocols : public  Serializable
	{
		SmpProtocolInfo registrarProtocol;
		SmpProtocolInfo moduleProtocol;
		virtual void serialize	( MessageBuffer &buffer ) const {

			marshal(buffer, registrarProtocol);
			marshal(buffer, moduleProtocol);
		}
		virtual void deserialize( MessageBuffer const&buffer ) {
			deMarshal(buffer, registrarProtocol);
			deMarshal(buffer, moduleProtocol);
		}
	};
*/


	template <int CallType>
	class TReceptor;

	/**
	 *	AbstrakteBasisklasse des Module Servers
	 * 	Der Modul-Server kommuniziert mit dem Module-Manager,
	 * 		d.h. er wartet auf Befehler dessselben
	 */
	template <>
	class TReceptor<AbstractInterface> {
	public:
		TReceptor() {}
		virtual ~TReceptor() {}
	public:
		// Interface zum Modul

		/// anderer ModulManager Registriert sich
		virtual void 	registerModuleManager(int interfaceId, SmpProtocolInfo & protocol) = 0;

		/// Modul registriert sich
		virtual SmpProtocolInfo registerModule(int moduleHandle, ModuleSpec spec, ModuleType type) = 0;

		/// Modul de-registriert sich
		virtual void unregisterModule(int moduleHandle) = 0;
	};

    struct TReceptorMessageDefinition
    {
		MESSAGE(void, RegisterModuleManager, int, SmpProtocolInfo );
		MESSAGE(SmpProtocolInfo, RegisterModule, int, ModuleSpec, int);
		MESSAGE(void, UnregisterModule, int);

		MESSAGE_LIST(
			RegisterModuleManager,
			RegisterModule,
			UnregisterModule
		);
    };

	template <>
	class TReceptor<Messages> : public Server<Messages>, public TReceptorMessageDefinition {
	public:
		TReceptor() : info(Receptor, sendBufLen, replyBufLen, MessageList::NumMessages) {}
		MessageInfo info;
	private:
		/// Konstanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 500*Bytes, replyBufLen = 500*Bytes };
	};

} // namespace interface
} // namespace precitec


#endif /*RECEPTOR_INTERFACE_H_*/
