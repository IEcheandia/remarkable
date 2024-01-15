#ifndef EVENT_TEST_INTERFACE_H
#define EVENT_TEST_INTERFACE_H

#include  "server/interface.h"
#include  "message/serializer.h"
#include "common/testClass.h" // wg Field

/*
 * Hier werden die abstrakten Baisklassen, sowie die
 * Messages definiert. Dieser Header wird sowohl fuer
 * den lokalen (Implementation, MsgHandler) sowie den
 * remote-Teil (Proxyer) benoetigt.
 */
namespace precitec
{
namespace interface
{
	using namespace  system;
	using namespace  message;



	/// Vorwaertsdeklaration des generellen Templates das hier Spezialisiert wird
	template <int mode>
	class TEventTest;

	/**
	 * PartTransport ist eine primitive PartTransport-Abstraktion. Sie kann bisher nur
	 * ein paar (ver)einfach(te) Befehle.
	 */
	template <>
	class TEventTest<AbstractInterface>
	{
	public:
		TEventTest() {}
		virtual ~TEventTest() {}
	public:
		virtual void trigger() = 0;
		/// uebertragen eines ints
		virtual void trigger(int a) = 0;
		/// a, b, a+b
		virtual void add(int a, int b, int sum) = 0;
		/// schickt einf Feld mit aufsteigenden Elemeten*Factor
		virtual void iota(Field const& f, int factor) = 0;
	};


	//----------------------------------------------------------
	template <>
	class TEventTest<Messages> : public Server<Messages>
	{
	public:
		TEventTest() : info(system::module::EventTest, sendBufLen, replyBufLen, NumMessages, NumBuffers) {}
		MessageInfo info;
	private:
		/// Konstanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024, NumBuffers=5};
		enum { sendBufLen  = 200*KBytes, replyBufLen = 200*KBytes };
	public:
		MESSAGE_LIST4(
			system::module::EventTest,
			MESSAGE_NAME0(trigger),
			MESSAGE_NAME1(trigger, int),
			MESSAGE_NAME3(add, int, int, int),
			MESSAGE_NAME2(iota, Field, int)
		);
	public:
		DEFINE_MSG0(void, trigger);
		DEFINE_MSG1(void, trigger, int);
		DEFINE_MSG3(void, add, int, int, int);
		DEFINE_MSG2(void, iota, Field, int);
	};


} // namespace events
} // namespace precitec

#endif /*EVENT_TEST_INTERFACE_H*/
