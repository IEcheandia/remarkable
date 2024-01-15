#ifndef MESSAGEMACROS_H_
#define MESSAGEMACROS_H_

#include"module/moduleLogger.h"
#include"system/tools.h"
#include "system/exception.h"

#define INIT_EVENT(Message) \
	Poco::ScopedLock<Poco::FastMutex> lock(*senderMutex_); \
	std::lock_guard senderLock{signaler()}; \
	try \
	{ \
		initMessage( precitec::interface::messages::MessageIndex<Message, MessageList>::index ); \
	} catch (precitec::system::AllocFailedException &) \
	{ \
		return; \
	} \
	catch(...) \
	{ \
		system::logExcpetion(__FUNCTION__, std::current_exception()); \
		wmLog(eError, "'%s' failed. Event not sent.\n" ,__FUNCTION__); \
		/* initMessage failed - do not send the message */ \
		return; \
	} // catch

 #define INIT_MESSAGE(Message) \
 	Poco::ScopedLock<Poco::FastMutex> lock(*senderMutex_); \
 	initMessage( precitec::interface::messages::MessageIndex<Message, MessageList>::index );



/**
 * Neues Macro um die von Server<MsgHandler> abgeleiteten Klassen einfach zu halten
 * Verwendung:
 * 	template <>
 *	class TUserServer<MsgHandler> : public Server<MsgHandler>
 *	{
 *		MSG_HANDLER(TUserServer);
 * 	public:
 *		void registerCallbacks()
 *		{
 *			REGISTER(TUserServer, callback0, int, int);
 * 			... wweitere callback-Registrierungen
 *		}
 *		void CALLBACK(...) {}
 * 		... weitere Callbacks
 * 	};
 */

// man kann dem Server eine NamedMutex mitliefern (die, wenn gesetzt, den Server killt)
#define MSG_HANDLER(UServer) \
	public: \
		/** Std Kontruktor ohne Protokoll */ \
		UServer(UServer<AbstractInterface>* server) \
		: Server<MsgHandler>(UServer<precitec::interface::Messages>().info), \
			server_(server), dispatcher_(numMessages()) { \
			dispatcher_.clearCallbackList(numMessages(), Callback(&UServer<MsgHandler>::defaultHandler)); \
		} \
		/** Kontruktor mit hartkodiertem Protokoll (nicht MM gemanaged */ \
		UServer(UServer<AbstractInterface>* server, SmpProtocolInfo &protInfo) \
		: Server<MsgHandler>(UServer<precitec::interface::Messages>().info), server_(server), \
		dispatcher_(numMessages()) { \
			dispatcher_.clearCallbackList(numMessages(), Callback(&UServer<MsgHandler>::defaultHandler)); \
			activate(protInfo); /* ab jetzt laeuft der Server, wenn p!=NULL */ \
		} \
		~UServer() { UServer<MsgHandler>::dispose(); } \
	protected: \
		/** handler leitet nur noch an den dispacher weiter */ \
		virtual void handle(Receiver & receiver, int i) { dispatcher_.handle(receiver, this, i); } \
		/** Typdefinition der Callbacks, waere obsolet mit richtigen Delegates */ \
		typedef void (UServer<MsgHandler>::*Callback) (Receiver &); \
	private:	\
		/** Typdefinition der Callbacks, wandert in Server mit richtigen Delegates */ \
		void defaultHandler(Receiver&) { std::cout << "testLocal " #UServer "<MsgHandler>::defaultHandler" << std::endl;	} \
		/** wir brauchen natuerlich einen Localcall-Server, der die Arbeit erledigt */ \
		UServer<AbstractInterface> 			*server_; \
		/** erledigt das Callbackmanagement */ \
		Dispatcher<UServer<MsgHandler> >  dispatcher_;



#define EVENT_HANDLER( UServer ) \
	public: \
		/** Std-Kontruktor ohne Protokoll, das eigenstaendig (setProtocol(p) ) gesetzt wird */ \
		UServer(UServer<AbstractInterface> *server) : Server<EventHandler>(UServer<precitec::interface::Messages>().info), \
		server_(server), eventDispatcher_(numMessages()) { \
			eventDispatcher_.clearCallbackList(numMessages(), EventCallback(&UServer<EventHandler>::defaultHandler)); \
		} \
		virtual ~UServer() { Server<EventHandler>::stop(); } \
		virtual SmpProtocolInfo activate(SmpProtocolInfo &serverInfo) { \
			SmpProtocolInfo clientInfo = Server<EventHandler>::activateProtocol(serverInfo); \
			registerCallbacks(); \
			Server<precitec::interface::EventHandler>::start(serverInfo->type());	/* Messageloop fuer dieses Protokoll starten*/\
			return clientInfo; \
		}\
	protected:	\
		/** die private Server-Klasse ruft hiermit typ-richtig die Callbacks, mit richtigen Delegates ist das obsolet */ \
		virtual void handle(Receiver & receiver, int i ) { eventDispatcher_.handle(receiver, this, i); } \
	private: \
		/** wir brauchen natuerlich einen Implementation-Server, der die Arbeit erledigt */ \
		UServer<AbstractInterface> *server_; \
		/** Typdefinition der Callbacks, waere obsolet mit richtigen Delegates */ \
		typedef void (UServer<EventHandler>::*EventCallback) (Receiver&); \
		/** Typdefinition der Callbacks, wandert in Server mit richtigen Delegates */ \
		void defaultHandler(Receiver&) { \
			 std::cout << " " #UServer "<EventHandler>::defaultHandler" << std::endl;  \
		} \
		EventDispatcher<UServer<EventHandler> >  eventDispatcher_;



#define EVENT_HANDLER2( UServer ) \
	public: \
		/** Std-Kontruktor ohne Protokoll, das eigenstaendig (setProtocol(p) ) gesetzt wird */ \
		UServer(UServer<EventServer> *server) : Server<EventHandler>(UServer<precitec::interface::Messages>().info), \
		server_(server), eventDispatcher_(numMessages()) { \
			eventDispatcher_.clearCallbackList(numMessages(), EventCallback(&UServer<EventHandler>::defaultHandler)); \
		} \
		virtual ~UServer() { Server<EventHandler>::stop(); } \
		virtual SmpProtocolInfo activate(SmpProtocolInfo &serverInfo) { \
			SmpProtocolInfo clientInfo = Server<EventHandler>::activateProtocol(serverInfo); \
			registerCallbacks(); \
			Server<precitec::interface::EventHandler>::start(serverInfo->type());	/* Messageloop fuer dieses Protokoll starten*/\
			return clientInfo; \
		}\
	protected:	\
		/** die private Server-Klasse ruft hiermit typ-richtig die Callbacks, mit richtigen Delegates ist das obsolet */ \
		virtual void handle(Receiver & receiver, int i ) { eventDispatcher_.handle(receiver, this, i); } \
	private: \
		/** wir brauchen natuerlich einen Implementation-Server, der die Arbeit erledigt */ \
		UServer<EventServer> *server_; \
		/** Typdefinition der Callbacks, waere obsolet mit richtigen Delegates */ \
		typedef void (UServer<EventHandler>::*EventCallback) (Receiver&); \
		/** Typdefinition der Callbacks, wandert in Server mit richtigen Delegates */ \
		void defaultHandler(Receiver&) { \
			 std::cout << " " #UServer "<EventHandler>::defaultHandler" << std::endl;  \
		} \
		EventDispatcher<UServer<EventHandler> >  eventDispatcher_;


/**
 * Remote-Call-Macros
 * Dieses Macro erstetzt cut & past code fuer alle abgeleiteten Message-Server
 */
typedef precitec::system::message::ProtocolInfo precitecsystemmessageProtocolInfo;

#define PROXY_CTOR( UserServer ) Server<MsgProxy>(UserServer<precitec::interface::Messages>().info)

#define PROXY_CTOR1( UserServer, precitecsystemmessageProtocolInfo ) Server<MsgProxy>(UserServer<precitec::interface::Messages>().info, precitecsystemmessageProtocolInfo)

#define EVENT_PROXY_CTOR( UserServer ) \
	Server<EventProxy>(UserServer<precitec::interface::Messages>().info)


namespace precitec
{
namespace interface
{
namespace messages
{

/**
 * Definition for a Message.
 *
 * This struct provides typedefs for the reply type
 * and for all the arguments passed to the Message
 * as a tuple type.
 *
 * The MessageDefinition is normally defined as a public struct
 * of a sub-class of Server<Messages>.
 *
 * Example usage:
 * @code
 * struct MyMessage : public MessageDefinition<int, bool, std::string> {};
 * @endcode
 *
 * This example defines a message called MyMessage with an int as reply
 * type and taking two arguments of bool and std::string.
 *
 * @tparam Reply The reply type of the Message
 * @tparam Args... The argument types of the Message
 *
 * @see MessageListDefinition
 */
template <typename Reply, typename... Args>
struct MessageDefinition
{
    /**
    * The reply type of this Message
    */
    typedef Reply reply_type;
    /**
    * The arguments of the Message
    */
    typedef std::tuple<Args...> argument_types;
};

/**
 * Definition for the Messages provided by a Server<Messages>.
 *
 * The MessageListDefinition can be used to group and order the
 * MessageDefinitions provided by a Server<Messages> sub-class.
 *
 * The MessageListDefinition knows the number of messages and
 * can be used to determine the index of a given MessageDefinition
 * at compile time through the MessageIndex struct.
 *
 * An example usage:
 * @code
 * struct MyMessage : public MessageDefinition<int, bool, std::string> {};
 * struct OtherMessage : public MessageDefinition<void> {};
 *
 * struct MessageList : public MessageListDefinition<MyMessage, OtherMessage> {};
 * @endcode
 *
 * This defines a MessageList with two Messages: MyMessage and OtherMessage.
 *
 * To access the number of messages use:
 * @code
 * MessageList::NumMessages
 * @endcode
 *
 * To access the messages use:
 * @code
 * MessageList::messages
 * @endcode
 *
 * @see MessageDefinition
 * @see MessageIndex
 */
template <typename... Args>
struct MessageListDefinition
{
    /**
    * Type of the messages as a tuple type.
    */
    typedef std::tuple<Args...> messages;
    /**
    * The number of Messages in messages
    */
    static const size_t NumMessages = std::tuple_size<messages>::value;
};

/**
* Helper template for MessageIndex to calculate the index of a Message in Messages.
*
* Do not use directly, use MessageIndex instead.
*
* This is a variadic template comparing recursively from the last element of the
* Messages to the first element whether the type at the current index is the same
* as Message.
*/
template <size_t count, typename Message, typename Messages>
struct MessageIndexFinder;

/**
 * Special type for the case that the Message is not in the MessageList.
 */
struct MessageNotInList : std::integral_constant<int, -1>{};

/**
 * Recursive condition if Message is not part of the MessageList.
 */
template <size_t count, typename Message>
struct MessageIndexFinder<count, Message, std::tuple<>> : MessageNotInList{};

/**
 * Recursive step removing the first element of the tuple.
 */
template <size_t count, typename Message, typename T, typename... Args>
struct MessageIndexFinder<count, Message, std::tuple<T, Args...> > : MessageIndexFinder<count, Message, std::tuple<Args...>>{};

/**
 * Recursive step for the case that the index is found.
 */
template <size_t count, typename Message, typename... Args>
struct MessageIndexFinder<count, Message, std::tuple<Message, Args...>> : std::integral_constant<std::size_t, count - 1 - sizeof...(Args)>{};

/**
 * Calculates the index of a Message in the MessageList at compile time.
 *
 * For example:
 * @code
 * struct MyMessage : public MessageDefinition<int, bool, std::string> {};
 * struct OtherMessage : public MessageDefinition<void> {};
 *
 * struct MessageList : public MessageListDefinition<MyMessage, OtherMessage> {};
 *
 * const int myMessageIndex = MessageIndex<MyMessage, MessageList>::index;
 * const int otherMessageIndex = MessageIndex<OtherMessage, MessageList>::index;
 *
 * assert(myMessageIndex == 0);
 * assert(otherMessageIndex == 1);
 * @endcode
 *
 * This allows to get the index of a Message in a MessageList at compile time.
 *
 * @tparam Message The Message for which the index needs to be derived
 * @tparam MessageList The MessageList containing Message
 *
 * @see MessageDefinition
 * @see MessageList
 */
template <typename Message, typename MessageList>
struct MessageIndex
{
    typedef MessageIndexFinder<MessageList::NumMessages, Message, typename MessageList::messages> compare_type;
    typedef typename compare_type::value_type value_type;

    /**
    * The index of Message in MessageList
    */
    static const value_type index = compare_type::value;
    static_assert(!std::is_base_of<MessageNotInList, compare_type>::value, "MessageList must contain Message");
};

}
}
}

/**
 * Macro to simplify the definition of a MessageDefinition.
 *
 * Example usage:
 * @code
 * MESSAGE(int, MyMessage, bool, std::string);
 * @endcode
 *
 * This defines MyMessage with return type int and two arguments of bool and std::string.
 */
#define MESSAGE(ReturnType, Name, ...) \
struct Name : public precitec::interface::messages::MessageDefinition<ReturnType, __VA_ARGS__>{}

/**
 * Marco to simplify the definition of a MessageDefinition with void return type.
 *
 * @see MESSAGE
 **/
#define EVENT_MESSAGE(Name, ...) \
    MESSAGE(void, Name, __VA_ARGS__)

/**
 * Macro to simplify the definition of a MessageListDefinition.
 *
 * Example usage:
 * @code
 * MESSAGE(int, MyMessage, bool, std::string);
 * MESSAGE(void, OtherMessage);
 * MESSAGE_LIST(MyMessage, OtherMessage);
 * @endcode
 *
 * This defines a MessageList with two Messages: MyMessage and OtherMessage.
 */
#define MESSAGE_LIST(...) \
    struct MessageList : public precitec::interface::messages::MessageListDefinition< __VA_ARGS__ >{}

/**
 * Registers a callback for Class<Messages>::Message in the dispatcher.
 *
 * Normally used in a Server<MsgHandler>::registerCallbacks implementation.
 */
#define REGISTER_MESSAGE(Message, callback) \
    dispatcher_.addCallback(precitec::interface::messages::MessageIndex<Message, MessageList>::index, Callback(&std::remove_pointer<decltype(this)>::type::callback))
/**
 * Registers a callback for Class<Messages>::Message in the eventDispatcher.
 *
 * Normally used in a Server<EventProxy>::registerCallbacks implementation.
 */
#define REGISTER_EVENT(Message, callback) \
    eventDispatcher_.addCallback(precitec::interface::messages::MessageIndex<Message, MessageList>::index, EventCallback(&std::remove_pointer<decltype(this)>::type::callback))

#endif /*MESSAGEMACROS_H_*/
