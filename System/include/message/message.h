#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <string>
#include <iostream>

#include "SystemManifest.h"

#include "Poco/TypeList.h"
#include "message/messageBuffer.h"

/** Namespace fuer das Messageing-System
 */
namespace precitec
{
namespace system
{
namespace message
{
	/**
	 *  Basisklasse fuer Message
	 * sie dient nur dazu dass es einen gemeinsamen Pointer
	 * auf alle Messages geben kann
	 */
	struct SYSTEM_API MessageBase {
		MessageBase() {}
		virtual ~MessageBase() {};
		virtual int getIndex() const { return -1; }
		friend bool operator == (MessageBase const& lhs, MessageBase const& rhs)  { return (lhs == rhs) ;	}
	};

	/** Message
	 * \param ReturnData Typ des Returnertes
	 * \param SendDataTupel Typliste der Argumente
	 * Die Message haelt im Wesentlichen Typ-Informationen
	 * Zusaetzlich gibt es in den abgeleiteten Klasse noch einen Enum (index), der
	 * die Messages eindeutig Indiziert
	 * Im Grunde sollte selbst dieser index redundant sein, ist aber z.Zt. wg Konsistentabfragen
	 * noch vorhanden
	 *
	 */
	class SYSTEM_API VoidData {};
	template<class ReturnData=VoidData, class SendDataTupel=Poco::NullTypeList>
	struct Message : public MessageBase {
		/**
		 * Konstruktor des in Implementation-Template, hier wird die Message
		 * initialisiert und registriert. Dies findet mindestens einmal statt und darf etwas dauern.
		 */
		Message() : MessageBase() {}
		virtual ~Message() {};
		typedef SendDataTupel SendTypes; /// Sendedatentyp
		typedef ReturnData  	ReturnType;  /// Empfangsdatentyp
	};
	enum { ShutdownMessage = -2, TimeoutMessage = -1};


} // message
} // system
} // precitec

#endif /*MESSAGE_H_*/
