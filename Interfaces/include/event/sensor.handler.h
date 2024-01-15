#ifndef SENSOR_HANDLER_H_
#define SENSOR_HANDLER_H_

#include "event/sensor.interface.h"
#include "server/eventHandler.h"

#include "module/moduleLogger.h"

namespace precitec
{
namespace interface
{
	using image::Sample;
	using namespace  message;

	template <>
	class TSensor<EventHandler> : public Server<EventHandler>, public TSensorMessageDefinition
	{
	public:
		//EVENT_HANDLER( TSensor );
		/* Std-Kontruktor ohne Protokoll, das eigenstaendig (setProtocol(p) ) gesetzt wird */
		TSensor(TSensor<AbstractInterface> *server) : Server<EventHandler>(TSensor<Messages>().info),
		server_(server), eventDispatcher_(numMessages()) {
			eventDispatcher_.clearCallbackList(numMessages(), EventCallback(&TSensor<EventHandler>::defaultHandler));
		}
		virtual ~TSensor() { Server<EventHandler>::stop(); }
		virtual SmpProtocolInfo activate(SmpProtocolInfo &serverInfo) {
			SmpProtocolInfo clientInfo = Server<EventHandler>::activateProtocol(serverInfo);
			registerCallbacks();
			start(serverInfo->type());	/* Messageloop fuer dieses Protokoll starten*/
			return clientInfo;
		}
	protected:
		/* die private Server-Klasse ruft hiermit typ-richtig die Callbacks, mit richtigen Delegates ist das obsolet */
		virtual void handle(Receiver & receiver, int i ) { eventDispatcher_.handle(receiver, this, i); }
	private:
		/* wir brauchen natuerlich einen Implementation-Server, der die Arbeit erledigt */
		TSensor<AbstractInterface> *server_;
		/** Typdefinition der Callbacks, waere obsolet mit richtigen Delegates */
		typedef void (TSensor<EventHandler>::*EventCallback) (Receiver&);
		/* Typdefinition der Callbacks, wandert in Server mit richtigen Delegates */
		void defaultHandler(Receiver&) {}
		EventDispatcher<TSensor<EventHandler> >  eventDispatcher_;


	public:
		void registerCallbacks()
		{
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
            REGISTER_EVENT(Imagedata, data<image::BImage>);
            REGISTER_EVENT(Sampledata, data<image::Sample>);
            REGISTER_EVENT(SimulationDataMissing, simulationDataMissing);
		}

		template <typename T>
		void data(Receiver &receiver)
		{
            int sensorId; receiver.deMarshal(sensorId);
            TriggerContext context; receiver.deMarshal(context);
            T data; receiver.deMarshal(data);
          
			try {
                server_->data(sensorId, context, data);
			} catch(Poco::Exception& p_rException) { // fliplib exceptions like the GraphBuilderException are all poco exceptions
				wmLog( eError, "Poco::Exception: %s:\n", p_rException.what() );
				wmLog( eError, "'%s'\n", p_rException.message().c_str() );
			} // catch	
            catch (...) {
                wmLog( eError, "Unhandled TSensor<EventHandler>::data(Image) exception\n" );
            } // catch
		}

        void simulationDataMissing(Receiver &receiver)
        {
            TriggerContext context;
            receiver.deMarshal(context);
            server_->simulationDataMissing(context);
        }

	};

} // namespace interface
} // namespace precitec


#endif /*SENSOR_HANDLER_H_*/
