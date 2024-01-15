#ifndef PART_TRANSPORT_SERVER_H
#define PART_TRANSPORT_SERVER_H

#include <string> // std::string

#include  "server/interface.h"
#include  "event/partTransport.interface.h"
#include  "module/interfaces.h" // wg appId


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
	/**
	 * TPartTransport ist eine primitive TPartTransport-Abstraktion. Sie kann bisher nur
	 * ein paar (ver)einfach(te) Befehle.
	 */
	template <>
	class TPartTransport<EventServer> : public TPartTransport<AbstractInterface> 
	{
	public:
		TPartTransport() {}
		virtual ~TPartTransport() {}
	public:
		virtual void moveto(int position) {}
		virtual void init() {}
		virtual void pause() {}
		virtual void resume() {}
		virtual void stop() {}
	};


} // namespace system
} // namespace precitec

#endif /*PART_TRANSPORT_SERVER_H*/
