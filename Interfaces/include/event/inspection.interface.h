#ifndef INSPECTION_INTERFACE_H_
#define INSPECTION_INTERFACE_H_


#include "server/interface.h"
#include "module/interfaces.h" // wg appId
#include "protocol/protocol.info.h"

#include "event/inspection.h"

/*
 * Hier werden die abstrakten Basisklassen, sowie die
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
	//----------------------------------------------------------
	// Hier folgen die Template-Definitionen fuer die verschiedenen
	// Spezialisierungen  <Implementation> <MsgHandler> <Proxyer> <Messages>

	template <int mode>
	class TInspection;

	//----------------------------------------------------------
	// Abstrakte Basis Klassen  = Interface
	// Die <Implementation> und <Proxy> Spezialisierungen leiten von diesen
	// Basisklassen ab.

	/**
	 * Signaler zeigt auf primitive Weise den Systemzustand an.
	 * Der State-Enum bietet drei Zustaende an. Verschiedene
	 * Handler koennen diese Zustaende unterschiedlich darstellen.
	 */
	template<>
	class TInspection<AbstractInterface>
	{
	public:
		TInspection() {}
		virtual ~TInspection() {}
	public:
		//Automatikbetrieb Start - Inspektion Bauteil aktiv, Grafen werden Produktspezifisch aufgebaut
		virtual void startAutomaticmode(uint32_t producttype, uint32_t productnumber, const std::string& p_rExtendedProductInfo) = 0;
		//Automatikbetrieb Stop
		virtual void stopAutomaticmode() = 0;
		// Inspection Start -> Naht aktiv
		virtual void start (int seamnumber ) = 0;
		// Inspection Ende -> Naht ! aktiv
		virtual void end ( int seamnumber ) = 0;
		// Nahtfolge uebernehmen
		virtual void info( int seamsequence ) = 0;
		// linelaser ein/aus
		virtual void linelaser (bool onoff) = 0;
		// Kalibration Start
		virtual void startCalibration() = 0;
		// Kalibration Ende
		virtual void stopCalibration() = 0;
		// Naht Vor-Start
		virtual void seamPreStart (int seamnumber ) = 0;
	};

    struct TInspectionMessageDefinition
    {
		EVENT_MESSAGE(StartAutomaticmode, uint32_t, uint32_t, std::string);
		EVENT_MESSAGE(StopAutomaticmode, void);
		EVENT_MESSAGE(Start, int);
		EVENT_MESSAGE(End, int);
		EVENT_MESSAGE(Info, int);
		EVENT_MESSAGE(Linelaser, bool);
		EVENT_MESSAGE(StartCalibration, void);
		EVENT_MESSAGE(StopCalibration, void);
		EVENT_MESSAGE(SeamPreStart, int);

		MESSAGE_LIST(
			StartAutomaticmode,
			StopAutomaticmode,
			Start,
			End,
			Info,
			Linelaser,
			StartCalibration,
			StopCalibration,
			SeamPreStart
		);
    };

	//----------------------------------------------------------
	template <>
	class TInspection<Messages> : public Server<Messages>, public TInspectionMessageDefinition
	{
	public:
		TInspection<Messages>() : info(system::module::Inspection, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
		MessageInfo info;
	private:
		/// Konstanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 500*Bytes, replyBufLen = 100*Bytes, NumBuffers=128 };
	};


} // namespace interface
} // namespace precitec


#endif /*INSPECTION_INTERFACE_H_*/
