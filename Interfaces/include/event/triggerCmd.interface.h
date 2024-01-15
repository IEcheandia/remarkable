/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR, AL, AB, HS
 * 	@date		2009
 * 	@brief		Trigger command interface. Signals sensors to take a single measurement or several measurements over time (burst).
 */

#ifndef TRIGGERCMD_INTERFACE_H_
#define TRIGGERCMD_INTERFACE_H_

/**
 *  @file
 *  @copyright    Precitec Vision GmbH & Co. KG
 *  @author       Rapl Kirchner, Andreas Beschorner (AB)
 *  @date         2011 (last update)
 *  @brief        TriggerCommand Interface.
 */

#include "server/interface.h"
#include "module/interfaces.h" // wg appId
#include "protocol/protocol.info.h"

#include "event/triggerCmd.h"

/*
 * Hier werden die abstrakten Basisklassen, sowie die
 * Messages definiert. Dieser Header wird sowohl fuer
 * den lokalen (Implementation, MsgHandler) sowie den
 * remote-Teil (Proxyer) benoetigt.
 */
namespace precitec {
namespace interface {
	//----------------------------------------------------------
	// Hier folgen die Template-Definitionen fuer die verschiedenen
	// Spezialisierungen  <Implementation> <MsgHandler> <Proxyer> <Messages>

	template <int mode>
	class TTriggerCmd;

	//----------------------------------------------------------
	// Abstrakte Basis Klassen  = Interface
	// Die <Implementation> und <Proxy> Spezialisierungen leiten von diesen
	// Basisklassen ab.

	/**
	 * Commandos an Triggerquellen
	 */
	template<>
	class TTriggerCmd<AbstractInterface>
	{
	public:
		TTriggerCmd() {}
		virtual ~TTriggerCmd() {}
	public:
		virtual std::string name() const { return "BaseClass"; }
		// fordert ein einzelnes TriggerSignal an
		virtual void single(const std::vector<int>& p_rSensorIds, TriggerContext const& context) = 0;
		// initalisiert und startet die Triggerquelle ueber mehrere Abschnitte
		virtual void burst (const std::vector<int>& p_rSensorIds, TriggerContext const& context, int source, TriggerInterval const& interval) = 0;
		// unterbricht die laufende Triggerquelle, wenn noch Triggersignale versendet werden
		virtual void cancel(const std::vector<int>& p_rSensorIds) = 0;
	};

    struct TTriggerCmdMessageDefinition
    {
		EVENT_MESSAGE(Single, std::vector<int>, TriggerContext);
		EVENT_MESSAGE(Burst,  std::vector<int>, TriggerContext, int, TriggerInterval);
		EVENT_MESSAGE(Cancel, std::vector<int>);

		MESSAGE_LIST(
			Single,
			Burst,
			Cancel
		);
    };

	//----------------------------------------------------------
	template <>
	class TTriggerCmd<Messages> : public Server<Messages>, public TTriggerCmdMessageDefinition
	{
	public:
		TTriggerCmd<Messages>() : info(system::module::TriggerCmd, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
		MessageInfo info;

		typedef	std::vector<int>	vector_int_t;
	private:
		/// Konstanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 5*KBytes, replyBufLen = 100*Bytes, NumBuffers = 32 };
	};


} // namespace interface
} // namespace precitec


#endif /*TRIGGERCMD_INTERFACE_H_*/
