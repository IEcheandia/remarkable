#ifndef SENSOR_REMOTE_H_
#define SENSOR_REMOTE_H_

#include "Poco/TypeList.h"
#include "server.h"
#include "message/message.h"
#include "message/messageReceiver.h"
#include "message/messageMacros.h"
#include "message/protocol.h"
#include <unistd.h> // sleep

namespace precitec
{
	using namespace  system;
	using namespace  message;

	namespace system
	{
		namespace server
		{
			using Poco::TypeListType;
			using message::Message;
			//using message::Sender;
			using message::MessageBuffer;


			template <>
			class TSensor<EventServer>
			{
			public:
				TSensor(){}
				virtual ~TSensor() {}
			public:
				// grabt N Bilder
				void grabImages(int nImages);

				// grabt 1 Bild
				void grabSingleImage(void);
			private:
			};

			template <>
			class TSensor<Messages>
			{
			public:
				MESSAGE_LIST2(
					system::module::Sensor,
					MESSAGE_NAME1(grabImages, int),
					MESSAGE_NAME0(grabSingleImage)
				);
			public:
				DEFINE_MSG1(bool, grabImages, int);
				DEFINE_MSG0(bool, grabSingleImage);
				// Werte werden so festgelegt, dass alle Parameter und Ergebnisse Platz finden
				enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
				enum { sendBufLen  = 500*Bytes, replyBufLen = 100*Bytes };
			};

			template <>
			class TSensor<EventProxy> : public Server<EventProxy>
			{
			public:
				/// Der CTor braucht die Klasse, die die Arbeit erledigt, also den eigentlichen Server
				TSensor() : Server<Proxy>() {
					sender().setBuffers(TSensor<Messages>::sendBufLen, TSensor<Messages>::replyBufLen);
				}
				virtual ~TSensor() {}

			public:

				bool grabImages(int nImages)
				{
					typedef TSensor<Messages>::MESSAGE_NAME1(grabImages, int) Msg;
					sender().initMessage(Msg::index);
					sender().marshal(nImages);
					sender().send();
					bool ret; sender().deMarshal(ret);
					return ret;
				}
			};

	template <>
	class TSensor<MsgHandler> : public Server<MsgHandler>
	{
	public:
		/// aus dem
		//typedef TSensor<Implementation> DCServer;
		typedef void (TSensor<MsgHandler>::*Callback) ();
	public:
		TSensor(TSensor<Implementation> *server, Protocol &p)
		: Server<MsgHandler>(new Receiver(p, TSensor<Messages>::sendBufLen, TSensor<Messages>::replyBufLen)),
			numMessages_(TSensor<Messages>::NumMessages), cbs_(numMessages_), server_(server),
			messageLoop_(this, &TSensor::messageLoop)
		{
			init();
		}

		/// diese Routine wird aufgerugen falls eine ungueltige Message aufgerufen wird, sollte iegentlich selten vorkommen
		void defaultHandler()	{	wmLog(eError("default Handler\n"); }

		virtual ~TSensor() {}
	public:
		bool init()
		{
			// DefaultHanlder setzen
			for(int i=0; i<numMessages_; i++ ) {	cbs_[i] = &TSensor::defaultHandler; }
			// nun die richtigen
			REGISTER1(TSensor, grabImages, int );

			messageLoop_.start();
			sleep(0);
			return true;
		}
		/// beendet die Message-Loop, wartet ggf. auf die Terminierung von Threads und Prozessen
		virtual bool stop() { messageLoop_.stop(); return true; }

		void CALLBACK1(grabImages, int)()
		{
			TSensor<Messages>::MESSAGE_NAME1(grabImages, int) msg();
			int  arg0; receiver().deMarshal(arg0);
			receiver().marshal(server_->grabImages(arg0));
			receiver().reply();
		}
	private:
		void messageLoop()
		{
			while(true)
			{
				int msgNum = receiver().receive();
				if ((msgNum>=0)&&(msgNum<numMessages_))
				{
					(this->*cbs_[msgNum])();
				}
			}
		}
		private:
		void addCallback(int msgNum, Callback cb)
		{
			cbs_[msgNum] = cb;
		}
				/// wir brauchen natuerlich einen Localcall-Server, der die Arbeit erledigt
		int 				numMessages_;
		std::vector<Callback> cbs_;
		TSensor<Implementation> *server_;
		Poco::Activity<TSensor> messageLoop_;	// Thread-Wrapper um Messageloop
	};

} // sensor
} // inspect
} // precitec


#endif /*SENSOR_REMOTE_H_*/
