#ifndef REGISTRAR_HANDLER_H_
#define REGISTRAR_HANDLER_H_

#include  "message/registrar.interface.h"
#include  "server/handler.h"

namespace precitec
{
namespace interface
{

	template <>
	class TRegistrar<MsgHandler> : public Server<MsgHandler>, public TRegistrarMessageDefinition {
	public:
		MSG_HANDLER(TRegistrar);

		void registerCallbacks() {
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!
			REGISTER_MESSAGE(Publish, publish);
			REGISTER_MESSAGE(Subscribe, subscribe);
			REGISTER_MESSAGE(AutoSubscribe, autoSubscribe);
			REGISTER_MESSAGE(Activation, activation);
			REGISTER_MESSAGE(Kill, kill);
			REGISTER_MESSAGE(Unpublish, unpublish);
			REGISTER_MESSAGE(Unsubscribe, unsubscribe);
		}

		void publish(Receiver &receiver) {
			int 	moduleHandle; 	receiver.deMarshal(moduleHandle);
			int 	interfaceId; 	receiver.deMarshal(interfaceId);
			int 	numEvents; 		receiver.deMarshal(numEvents);
			int 	pubAppId; 		receiver.deMarshal(pubAppId);
			int 	subAppId; 		receiver.deMarshal(subAppId);
			PvString subPath; 		receiver.deMarshal(subPath);
			receiver.marshal(server_->publish(moduleHandle, interfaceId, numEvents, pubAppId, subAppId, subPath));
			// der Message-Reply mit dem Returnwert wird abgesetzt
			receiver.reply();
		}

		void subscribe(Receiver &receiver) {
			int 	moduleHandle; receiver.deMarshal(moduleHandle);
			int 	interfaceId; 	receiver.deMarshal(interfaceId);
			int 	numEvents; 		receiver.deMarshal(numEvents);
			int 	subAppId; 		receiver.deMarshal(subAppId);
			int 	pubAppId; 		receiver.deMarshal(pubAppId);
			PvString 	path; 			receiver.deMarshal(path);
			server_->subscribe(moduleHandle, interfaceId, numEvents, subAppId, pubAppId, path);
			// der Message-Reply mit dem Returnwert wird abgesetzt
			receiver.reply();
		}

		void autoSubscribe(Receiver &receiver) {
			int 	moduleHandle; receiver.deMarshal(moduleHandle);
			int 	interfaceId; 	receiver.deMarshal(interfaceId);
			int 	numEvents; 		receiver.deMarshal(numEvents);
			int 	subAppId; 				receiver.deMarshal(subAppId);
			int 	pubAppId; 				receiver.deMarshal(pubAppId);
			PvString 	path; 			receiver.deMarshal(path);
			server_->autoSubscribe(moduleHandle, interfaceId, numEvents, subAppId, pubAppId, path);
			// der Message-Reply mit dem Returnwert wird abgesetzt
			receiver.reply();
		}

		//virtual void subscribe(int appId, int interfaceId);
		void activation(Receiver &receiver) {
			int 	handle; receiver.deMarshal(handle);
			int 					eventId; 			receiver.deMarshal(eventId);
			bool					activate; 		receiver.deMarshal(activate);
			server_->activation(handle, eventId, activate);
			// der Message-Reply mit dem Returnwert wird abgesetzt
			receiver.reply();
		}

		void kill(Receiver &receiver) {
			// falls der Server etwas tun muss, kann er das (bisher tut er nichts)
			server_->kill();
			/// handler kill sich selbst
			stop();
			// der Message-Reply mit dem Returnwert wird abgesetzt ???? kommt er da noch an??
			receiver.reply();
		}

		void unpublish(Receiver &receiver) {
			int 	moduleHandle; 	receiver.deMarshal(moduleHandle);
			int 	interfaceId; 	receiver.deMarshal(interfaceId);
			int 	numEvents; 		receiver.deMarshal(numEvents);
			int 	pubAppId; 		receiver.deMarshal(pubAppId);
			server_->publish(moduleHandle, interfaceId, numEvents, pubAppId);
			// der Message-Reply mit dem Returnwert wird abgesetzt
			receiver.reply();
		}

		void unsubscribe(Receiver &receiver) {
			int 	moduleHandle; receiver.deMarshal(moduleHandle);
			int 	interfaceId; 	receiver.deMarshal(interfaceId);
			int 	numEvents; 		receiver.deMarshal(numEvents);
			int 	subAppId; 		receiver.deMarshal(subAppId);
			server_->subscribe(moduleHandle, interfaceId, numEvents, subAppId);
			// der Message-Reply mit dem Returnwert wird abgesetzt
			receiver.reply();
		}
	}; // class TRegistrar<MsgHandler>

} // namespace interface
} // namespace precitec

#endif /*REGISTRAR_LOCAL_H_*/
