#ifndef PART_TRANSPORT_INTERFACE_H
#define PART_TRANSPORT_INTERFACE_H

#include  "server/interface.h"

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
	class TPartTransport;

	/**
	 * PartTransport ist eine primitive PartTransport-Abstraktion. Sie kann bisher nur
	 * ein paar (ver)einfach(te) Befehle.
	 */
	template <>
	class TPartTransport<AbstractInterface> 
	{
	public:
		TPartTransport() {}
		virtual ~TPartTransport() {}
	public:
		virtual void moveto(int position) = 0;
		virtual void init() = 0;
		virtual void pause() = 0;
		virtual void resume() = 0;
		virtual void stop() = 0;
	};


	//----------------------------------------------------------
	template <>
	class TPartTransport<Messages> : public Server<Messages>
	{
	public:
		TPartTransport() : info(system::module::PartTransport, sendBufLen, replyBufLen, NumMessages, NumBuffers) {}
		MessageInfo info;
	private:
		/// Konstanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 10*Bytes, replyBufLen = 10*Bytes };
	public:
		MESSAGE_LIST5(
			system::module::PartTransport,
			MESSAGE_NAME1(moveto, int),  
			MESSAGE_NAME0(init), 
			MESSAGE_NAME0(pause),
			MESSAGE_NAME0(resume), 
			MESSAGE_NAME0(stop) 
		);
	public:
		DEFINE_MSG0(void, init);
		DEFINE_MSG0(void, pause);
		DEFINE_MSG0(void, resume);
		DEFINE_MSG0(void, stop);
		DEFINE_MSG1(void, moveto, int);
	};


} // namespace events
} // namespace precitec

#endif /*PART_TRANSPORT_INTERFACE_H*/
