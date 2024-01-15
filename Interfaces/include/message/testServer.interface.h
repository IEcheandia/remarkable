#ifndef TEST_SERVER_INTERFACE_H_
#define TEST_SERVER_INTERFACE_H_

#include "system/types.h"				// wg PvString
#include "server/interface.h"
#include "module/interfaces.h" 	// wg module::TestServer
#include "message/serializer.h"
#include "common/testClass.h" 	// wg Field

/**
 * Hier findes sich die von remote und local gemeinsam genutzen Daten
 * Dazu gehoeren die abstrakte Basisklasse und die Messages.
 */
namespace precitec
{
	using namespace  system;
	using namespace  message;
	using system::module::TestServer;

namespace interface
{

	/// Vorwaertsdeklaration des generellen Templates das hier Spezialisiert wird
	template <int CallType>	class TTestServer;

	/**
	 * Nun wird die abstrakte Basisklasse unseres Servers definiert
	 * Diese Klasse darf Memberfunktionen mit Inhalt habe, aber keine
	 * nicht-statischen Membervariable.
	 */
	template <>
	class TTestServer<AbstractInterface>
	{
	public:
		TTestServer() {}
		virtual ~TTestServer() {}
	public:
		/// addiert zwei Zahlen
		virtual int add(int a, int b) = 0;
		/// vergleicht 2 Ints und zwei Floats ist wahr, wenn beide Paare gleich sind
		virtual bool compare(int a, int b, float c, float d) = 0;
		/// gibt lokal einen PvString aus
		virtual void output(void) = 0;
		/// kopiert einen PvString
		virtual PvString copy(PvString const& s) = 0;
		/// verbindet zwei PvStrings miteinander
		virtual PvString cat(PvString const& s, PvString const& t) = 0;
		/// schickt n Bytes
		virtual Field multiplyField(Field const& f, int factor) = 0;

		virtual Field getField(int size) = 0;

		virtual void sendField(Field const& f) = 0;

	};


	/**
	 * die 5 des MESSAGE_LIST-Macros bezieht sich auf die 5 Messages
	 * die Zahhlen dach MESSGE_NAME (hier 0..4) gibt die Anzahl der
	 * 	Parameter der Memberfunktion
	 * In der MessageList wird nichts anderes gemacht, als ein Enum zu
	 * 	erzeugen das jeder Message eine eindeutige nummer zuordnet, die
	 * 	sich aus der Position in der Lsite ergibt. Die Parameter der
	 *  Memberfunktion sind noetig, da ueberladene Funktionen mit unterschiedlichen
	 * 	Parametern erlaubt sind. Daher sind hier auch die Rueckgabewerte nicht  gefragt.
	 */
	template <>
	class TTestServer<Messages> : public Server<Messages>
	{
	private:
		/// Konstanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		/**
		 * die Puffergroessen richten sich nach den Argumenten. Bei etwa Strings ist die Gesamtlaenge der
		 * 		Argumente/Returnwerte nicht automatisch zu ermitteln. Daher muss der Anwender diese
		 * 		Konstanten, mit Bedacht selbst waehlen.
		 * Im konkreten'Beispiel wird die cat-Funktion als Puffer-kritischste Funktion angesehen.
		 */
		enum { sendBufLen  = 1020*KBytes, replyBufLen = 1020*KBytes };
	public:
		// die naechstn Zeilen sind bis auf den Klassennamen cut 'n' paste
		/** die Message-Klasse haelt ein paar Daten, auf die die anderen Klassen zugreifen
		 * 	NumMessages wird von den Macros gesetzt und hier der Basisklasse zur Verfuegung gestellt
		 *  module::TestServer erlaubt Client/Server bzw. Subscriber/Publisher auf den Interfacenamen zuzugreifen
		 */
		TTestServer() : info(TestServer, sendBufLen, replyBufLen, NumMessages) {}
		MessageInfo info;
	public:
		/**
		 * die 5 des MESSAGE_LIST-Macros bezieht sich auf die 5 Messages
	 	 * die Zahhlen dach MESSAGE_NAME (hier 0..4) gibt die Anzahl der
		 * 	Parameter der Memberfunktion
		 * In der MessageList wird im Wesentlsichen ein Enum erzeugt,
		 * 	das jeder Message eine eindeutige Nummer zuordnet, die
		 * 	sich aus der Position in der Liste ergibt. Der Enum generiert sich
		 *  aus Namen der Memberfunktion und seiner Signatur (seine Argument-Typen)
		 * 	So sind ueberladene Funktionen moeglich. Der Return-Wert wird nicht genommen, da C++ ihn auch ignoriert
		 */
		MESSAGE_LIST8(
			TestServer,	// Am Anfang stand das Wort (der Name der Schnittstelle fuer den MuduleManager)
			MESSAGE_NAME2(add, int, int),  // Komma!!! als Abschluss kein Semikolon
			MESSAGE_NAME4(compare, int, int, float, float),
			MESSAGE_NAME0(output),
			MESSAGE_NAME1(copy, PvString),
			MESSAGE_NAME2(cat, PvString, PvString),
			MESSAGE_NAME2(multiplyField, Field, int),
			MESSAGE_NAME1(getField, int),
			MESSAGE_NAME1(sendField, Field)	// Achtung!!! Hier nicht einmal ein Komma!!!
		);
	public:
		// Mit DEFINE_MSG verwendt intern MASSAGE_NAME generiert also die gleichen Namen wie in der MessageList
	 	// Hier sind die Rueckgabewerte noetig, da die Message hier erzeugt wird.
		DEFINE_MSG2(int, add, int, int);
		DEFINE_MSG4(bool, compare, int, int, float, float);
		DEFINE_MSG0(void, output);
		DEFINE_MSG1(PvString, copy, PvString);
		DEFINE_MSG2(PvString, cat, PvString, PvString);
		DEFINE_MSG2(Field, multiplyField, Field, int);
		DEFINE_MSG1(Field, getField, int);
		DEFINE_MSG1(void, sendField, Field);
	};
} // interface
} // precitec


#endif /*TEST_SERVER_INTERFACE_H_*/
