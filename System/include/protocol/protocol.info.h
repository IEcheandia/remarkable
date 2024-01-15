#ifndef PROTOCOL_INFO_H_
#define PROTOCOL_INFO_H_

#include <stdlib.h>

#include "SystemManifest.h"

#include "Poco/SharedPtr.h"
#include "Poco/Semaphore.h"

#include "message/serializer.h"
#include "system/typeTraits.h"
#include "system/templates.h"

namespace precitec
{
namespace system
{
namespace message
{
	class Protocol;
	class ProtocolInfo;

	/// die verschiedenen Protokoll-Manifestationen
	enum ProtocolType { Udp, Qnx, Tcp, NullPCol,
										NumActiveProtocols, // actually used in Messaging and not MetaProtocols
											// QnxVector,
										NumProtocols // number of defined Protocols incl MetaProtocols
									};

	std::ostream& operator << (std::ostream&os, ProtocolType const&p);

	typedef Poco::SharedPtr<ProtocolInfo, Poco::ReferenceCounter, Poco::ReleasePolicy<ProtocolInfo> > SmpProtocolInfo;
	typedef Poco::SharedPtr<Protocol, Poco::ReferenceCounter, Poco::ReleasePolicy<Protocol> > SmpProtocol;

	/**
	 * Die dynamische Generierung der ProtokollInfos geschieht ueber eine Tabelle
	 * von statischen Fabrikfunktionen. So kann typsicher gearbeitt werden.
	 */
	template <int ProtocolType> ProtocolInfo* CreateProtocolInfo();


	/**
	 * Basisklasse fuer alle Infostrukturen
	 * Sie dient erstmal nur, um gemeinsame Zeiger fuer alle dies Strukturen zu haben.
	 * Spaeter kommen vvllt. noch abstrakte Memberfunktionen dazu.
	 */
	class SYSTEM_API ProtocolInfo : public precitec::system::message::Serializable {
	protected: // 'abstrakte' Basisklasse
		/// erzeugt ungueltiges Protokol (etwa fuer Tests)
		ProtocolInfo(): type_(NumProtocols) {}
		/// std-CTor
		ProtocolInfo(ProtocolType t) : type_(t) {}
		/// auch Zuweisungen sind nur ueber die Subklasse moeglich
		ProtocolInfo(ProtocolInfo const&rhs) : type_(rhs.type_) {}
		/// Deserialisierungs-CTor, verwendet bei direkter Deserialisierung (nicht polymorph)
		//ProtocolInfo( MessageBuffer const&buffer )
		//	: type_ (ProtocolType(Serializable::deMarshal<int>(buffer))) {}
	public:
		/// muss wg SmartPtr public sein
		virtual ~ProtocolInfo() {}
	public:
		virtual bool isValid() const { return false; }
		/// Acessor, gesetzt wird der Typ nur im CTor
		ProtocolType type() const { return type_; }
		/// debug-Ausgabe des Protokol-Ziels als String
		virtual PvString destination() const { return "unknown destination"; }
	public:
		/// liefert fuer jedes i eine neues Protokoll
		virtual SmpProtocolInfo generateDerived(int i) const;

		// der Serialize-Block
		virtual void serialize	( MessageBuffer &buffer ) const { marshal(buffer, type_);	}
		//virtual void deserialize( MessageBuffer const&buffer ) { deMarshal(buffer, type_); }
		virtual void deserialize( MessageBuffer const&buffer ) {	std::cout << "protocolInfo::serialize(): didn't wanna call" << std::endl;; }
		//virtual void deserialize( MessageBuffer const&buffer ) {	ProtocolInfo tmp(buffer); swap(tmp); }
		/// swap wg deserialize + allg. wichtige Funktion
		void swap(ProtocolInfo & rhs) { std::swap(type_, rhs.type_); }
		/// Erzeugt eine neue ProtocolInfo und deserialisiert den MessageBuffer in das neue Objekt
		static ProtocolInfo * create(int type, MessageBuffer const&buffer);

		/// Erzeugt eine neue ProtocolInfo in Abhaengigkeit von ProtocolType
		//static ProtocolInfo * create(ProtocolType type) { return protocolFactoryList[type](); }
		/// der Typ muss stimmen, der Rest wird and die isEqual-Funktion der abgeleiteten Klasse uebergeben
		bool operator == (ProtocolInfo const& rhs) const;
	private:
		/// die Basisfunktion sollte nie aufgerufen werden
		virtual bool isEqual(ProtocolInfo const& rhs) const { return false; }

	private:
		/// unterscheidet die verschiedenen Protokolle (nicht nur fuer die Serialisierung)
		ProtocolType type_;
		/// \todo wg debug interfaceId einbauen
	};
	std::ostream &operator << (std::ostream &os, ProtocolInfo const&i);

	/// subtyp-uebergreifender Protokollvergleich ueber virtuelles isEqual
	inline bool ProtocolInfo::operator == (ProtocolInfo const& rhs) const {
		return (type()== rhs.type()) && (isEqual(rhs));
	}




/*
 * Die nachfolgenden Klassen spezialisierne die Protokollinfo fuer die verschiedenen Protokolle
	 */


	/** SocketInfo Info
	 * Socketinformationen TCPAdr:Port
	 */
	class SYSTEM_API SocketInfo : public ProtocolInfo {
	public:
		/// praktisch fuer dynamic-down-casts
		typedef SocketInfo const* PSI;
		typedef Poco::SharedPtr<SocketInfo> SmpSocketInfo;

		/// erzeugt leeres Protcol, das zu keiner Verbindung fuehrt, wg Debug/Test-Zwecke
		SocketInfo(ProtocolType socketType)
			: ProtocolInfo(socketType), serverIP_(), serviceName_() {}
		/// der STd-CTor
		SocketInfo(ProtocolType socketType, PvString const &ip, PvString const &name)
			: ProtocolInfo(socketType), serverIP_(ip), serviceName_(name) {
			//std::cout << "SocketInfo-CTor: (" << ip << ", " << name << ") " << *this << std::endl;
		}
		/// copy-CTor
		SocketInfo(SocketInfo const& rhs)
			: ProtocolInfo(rhs), serverIP_(rhs.serverIP_), serviceName_(rhs.serviceName_) {}
		/// Deserialisierungs-CTor aufgerufen von ProtocolInfo::create
		SocketInfo(int socketType, MessageBuffer const&buffer )
			: ProtocolInfo(ProtocolType(socketType)),
				serverIP_(Serializable::deMarshal<PvString>(buffer)),
				serviceName_(Serializable::deMarshal<PvString>(buffer)) {
			//std::cout << "SocketInfo::deserialize-CTor: " << ip() << " " << portString() << std::endl;

		}
		virtual ~SocketInfo() {}
	public:
		/// fuer SIs im Wesentlichen fuer Debug-Zwecke (leeres PRotokol, ohne Verbindung)
		bool isValid() const { return !serverIP_.empty(); }
		/// von einem Basisport aus wird i zur Protnummer dazuaddiert
		virtual SmpProtocolInfo generateDerived(int i) const;
		/// ein Hack, damit nicht PI-Paar (Server/Client) uebermittelt wrden muss ClientPort==1+ServerPort
		virtual SmpProtocolInfo generateNextDerived() const;
		/// Adress-Accessor
		PvString 	ip() 	const { return serverIP_; }
		PvString 	portString() 	const { return serviceName_; }
		uInt 			port() const { uInt p(atoi(serviceName_.c_str()));/*fromString(serviceName_, p);*/ return p; }
		/// nor really destination , but we don't know more
		virtual PvString destination() const { return portString(); }
	private:
		/// Funktion wird nur fuer richtigen Typ aufgerufen, dynamic_cast klappt
		virtual bool isEqual(ProtocolInfo const& rhs) const;
	public:
		// Serialize Block
		virtual void serialize	( MessageBuffer &buffer ) const;
		//virtual void deserialize( MessageBuffer const&buffer );
		virtual void deserialize( MessageBuffer const&buffer );
		static ProtocolInfo* create(int type, MessageBuffer const& buffer) { return new SocketInfo(type, buffer);}
	public: // \todo wird noch privat, wenn auf Accessoren umgstellt ist
		PvString serverIP_;
		PvString serviceName_;
	};

	inline bool SocketInfo::isEqual(ProtocolInfo const& rhs) const {
		SocketInfo const&si(dynamic_cast<SocketInfo const&>(rhs));
		return ((ip()==si.serverIP_) && (portString()==si.serviceName_));
	}

	inline void SocketInfo::serialize	( MessageBuffer &buffer ) const {
		//std::cout << "SocketInfo::serialize: " << ip() << " " << portString() << std::endl;
		marshal(buffer, ip());
		marshal(buffer, portString());
	}

	inline void SocketInfo::deserialize( MessageBuffer const&buffer ) {
		SocketInfo tmp(type(), buffer); swap(tmp);
	}

	std::ostream &operator << (std::ostream &os, SocketInfo const&i);



	/**
	 * NullInfo ist ein reinen Test-Protokoll.
	 * Es finden keine Netzwerkzugriffe statt. Statt dessen werden Puffer
	 * seemaphorengeschuetzt hin- und her-kopiert
	 */
	class SYSTEM_API NullInfo : public ProtocolInfo {
	public:
		// Semaphore muessen mit 1 initalisiert werden, da 0 nicht erlaubt ist. Deshalb wird im Protokoll im Konstruktor ein Wait aufgerufen
		NullInfo() : ProtocolInfo(NullPCol), proxySync_(new Poco::Semaphore(1)), serverSync_(new Poco::Semaphore(1, Max<int>::Value)) {}
		virtual ~NullInfo() {}
	public:
		typedef NullInfo const* PNI;
		virtual bool isValid() const { return true; }
	public:
		Poco::SharedPtr<Poco::Semaphore> proxySync_;
		Poco::SharedPtr<Poco::Semaphore> serverSync_;
	public:
		virtual void serialize	( MessageBuffer &buffer ) const	{}
		virtual void deserialize( MessageBuffer const&buffer )	{}
		/// wird von ProtocollInfo aus Fabrik-Tabelle heraus aufgerufen
		static ProtocolInfo* create(int, MessageBuffer const&) { return new NullInfo(); }
	};

	//std::ostream &operator << (std::ostream &os, SocketInfo const&i);

	/// hiermit kann zu Jedem beliebigen Protocol ein BasisProtocoll definieren
	///  aus dem  BasisProtokoll werden die individuellen Protokolle pro Server generiert

	template <ProtocolType type>
	SmpProtocolInfo createBaseProtocolInfo() { return SmpProtocolInfo(); }
	// fuer diese Protokolle gibt es richtige Implementierungen
 	template <>
 	SmpProtocolInfo createBaseProtocolInfo<Tcp>();
	template <>
 	SmpProtocolInfo createBaseProtocolInfo<Udp>();
 	template <>
 	SmpProtocolInfo createBaseProtocolInfo<Qnx>();
 	template <>
 	SmpProtocolInfo createBaseProtocolInfo<NullPCol>();
/*
	template <ProtocolType type>
	SmpProtocolInfo createBaseProtocol() { return SmpProtocolInfo(); }
	// fuer diese Protokolle gibt es richtige Implementierungen
	template<> SmpProtocolInfo createBaseProtocol<Tcp>();
	template<> SmpProtocolInfo createBaseProtocol<Udp>();
	template<> SmpProtocolInfo createBaseProtocol<Qnx>();
	template<> SmpProtocolInfo createBaseProtocol<NullPCol>();
*/


} // message
} // system
} // precitec

#endif /*PROTOCOL_INFO_H_*/
