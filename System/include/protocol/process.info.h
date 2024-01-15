#ifndef PROCESS_INFO_H_
#define PROCESS_INFO_H_
/*
 * ProcesInfo.h wurde aus protocol.info.h ausgelagert,
 * damit die ProcessInfo die LocalNode definieren kann.
 * Dies braucht einen Qnx-System-Header:
 */
#if defined __QNX__ || defined __linux__

#include <stdint.h> // uint32_t
#if defined __QNX__
#include <sys/netmgr.h> // ND_LOCAL_NODE
#endif
#endif

#include <cstdlib>
#include "Poco/SharedPtr.h"

#include "message/serializer.h"
#include "system/typeTraits.h"
#include "system/templates.h"
#include "protocol/protocol.info.h"

namespace precitec
{
namespace system
{
namespace message
{
#if defined __QNX__ || defined __linux__
	/**
	 * ProcessInfo
	 *  Spezialisieung der protokollinfo fuer das QNX-Messaging-Protokoll
	 */
	class ProcessInfo : public ProtocolInfo {
	public:
		/// enums fuer spezifische Nodes
#if defined __QNX__
		enum { 	InvalidNode = 0xfffffff0, InvalidPid = -1,
						InvalidChannel = -1,
						LocalNode = ND_LOCAL_NODE, CallerPid=0 };
#else
		enum { 	InvalidNode = 0xfffffff0, InvalidPid = -1,
						InvalidChannel = -1,
						LocalNode = 0, CallerPid=0 };
#endif
	public:
		/// erzeugt ungueltige PI fuer Tests -> fuehrt zu keiner Verbindung
		ProcessInfo() : ProtocolInfo(Qnx), chId_(InvalidChannel), pId_(InvalidPid), node_(InvalidNode), m_oServerUUIDStrg(), shMemName_()  {
			//std::cout << "ProcessInfo(): should not be called" << std::endl;
		}
		/// erzeugt PI fuer Server
		ProcessInfo(uint32_t node) : ProtocolInfo(Qnx), chId_(InvalidChannel), pId_(CallerPid), node_(node), m_oServerUUIDStrg(), shMemName_() {
			//std::cout << "ProcessInfo CTor: " << (isValid() ? "valid" : "invalid") << std::endl;
			//std::cout << "...             : " << (isServerInfo() ? "server" : "non-server") << std::endl;

		}
		/// (innerhalb ServerInitialisierung) erzeugt gueltige PI fuer Client
		ProcessInfo(int c, int p, uint32_t node, PvString shMemName=PvString()) : ProtocolInfo(Qnx), chId_(c), pId_(p), node_(node), m_oServerUUIDStrg(), shMemName_(shMemName) {
		}
		/// (innerhalb ServerInitialisierung) erzeugt gueltige PI fuer Client
		ProcessInfo(int c, int p, uint32_t node, PvString p_oServerUUIDStrg, PvString shMemName=PvString()) :
			ProtocolInfo(Qnx), chId_(c), pId_(p), node_(node), m_oServerUUIDStrg(p_oServerUUIDStrg), shMemName_(shMemName) {}
		/// copy-CTor
		ProcessInfo(ProcessInfo const& rhs)
		: ProtocolInfo(rhs), chId_(rhs.chId_), pId_(rhs.pId_), node_(rhs.node_), m_oServerUUIDStrg(rhs.m_oServerUUIDStrg), shMemName_(rhs.shMemName_) {}
		/// deserialisierungs CTor
		ProcessInfo(MessageBuffer const&buffer );

		virtual ~ProcessInfo() {}
	public:
		// der Serialize-Block
		virtual void serialize	( MessageBuffer &buffer ) const;
		virtual void deserialize( MessageBuffer const&buffer );
		/// wird von ProtocollInfo aus Fabrik-Tabelle heraus aufgerufen
		static ProtocolInfo* create(int , MessageBuffer const& buffer) { return new ProcessInfo(buffer);}
		void swap(ProcessInfo & rhs) {
			ProtocolInfo::swap(rhs); std::swap(chId_, rhs.chId_); std::swap(pId_,rhs.pId_);
			std::swap(node_,rhs.node_); m_oServerUUIDStrg.swap(rhs.m_oServerUUIDStrg); shMemName_.swap(rhs.shMemName_); }

		/// MM setzt hiermit das SharedMemory;
		void setSharedMem(PvString const& name) { shMemName_ = name; }
		virtual PvString destination() const { return std::to_string(chId_); }

	public:
		// Accessoren
		int processId() const { return pId_;  }
		int channelId() const { return chId_; }
		int node() 			const { return node_; }
		PvString const serverUUIDStrg() 	const { return m_oServerUUIDStrg; }
		/// wichtig fuer die Unterscheidung PI fuer Server(ungueltig), Client (gueltig, da von Server erzeugt)
		bool isValid() const { return pId_ != InvalidPid; }
		/// der Server bekommt eine 'leere' $PI beim Co
		bool isServerInfo() const { return /*(node_ != InvalidNode) && */(pId_==CallerPid); }
		/// erzeugt ungültige PI, da Server-Initialisierung automatisch neuen Channel bereitstellt (hoffentlich)
		virtual SmpProtocolInfo generateDerived(int ) const;
		PvString const& shMemName() const { return shMemName_; }
	private:
		/// Vergleichsfunktion erlaubt  den == operator sicher u. public in der Baisklasse zu halten
		virtual bool isEqual(ProtocolInfo const& rhs) const;
	private:
		/// connection ID (ChannelCreate <-> ConnectAttach)
		int chId_;
		/// Prozess ID (ConnectAttach)
		int pId_;
		/// Netzwerk-id
		int node_;
		/// UUID String fuer Verbindung ueber SIMPL
		PvString m_oServerUUIDStrg;
		/// ShMemName fuer MessagePuffer
		PvString shMemName_;
	};

	inline ProcessInfo::ProcessInfo(MessageBuffer const& buffer )
		: ProtocolInfo(Qnx),
			chId_(Serializable::deMarshal<int>(buffer)),
			pId_(Serializable::deMarshal<int>(buffer)),
			node_(Serializable::deMarshal<int>(buffer)),
			m_oServerUUIDStrg(Serializable::deMarshal<PvString>(buffer)),
			shMemName_(Serializable::deMarshal<PvString>(buffer))
	{
		//std::cout << "ProcessInfo::deserialize-CTor " << *this << std::endl;
	}

	/// Funktion wird nur fuer richtigen Typ aufgerufen, dynamic_cast klappt
	inline bool ProcessInfo::isEqual(ProtocolInfo const& rhs) const {

		ProcessInfo const&pi(dynamic_cast<ProcessInfo const&>(rhs));
		//std::cout << "ProcessInfo::isEqual( (" << chId_ << ", " << pId_ << ")," <<
		//																" (" << pi.chId_ << ", " << pi.pId_ << ") )" << std::endl;
		return ((chId_==pi.chId_) && (pId_==pi.pId_));
	}

	inline void ProcessInfo::serialize( MessageBuffer &buffer ) const	{
		//std::cout << "ProcessInfo::serialize: " << *this << std::endl;
		marshal(buffer, chId_);
		marshal(buffer, pId_);
		marshal(buffer, node_);
		marshal(buffer, m_oServerUUIDStrg);
		marshal(buffer, shMemName_);
	}

	inline void ProcessInfo::deserialize( MessageBuffer const&buffer ) {
		ProcessInfo tmp(buffer); swap(tmp);
		/*
		std::cout << "ProcessInfo::deserialize: " << type() << std::endl;
		deMarshal(buffer, chId_);
		deMarshal(buffer, pId_);
		deMarshal(buffer, node_);
		std::cout << "ProcessInfo::deserialized: " << *this << std::endl;
		*/
	}

	/// impl in protocolInfo.h
	std::ostream &operator << (std::ostream &os, ProcessInfo const&i);

#else
	class ProcessInfo : public ProtocolInfo {
	public:
		/// enums fuer spezifische Nodes
		enum { 	InvalidNode = 0xfffffff0, InvalidPid = -1,
						InvalidChannel = -1,
						LocalNode = 0, CallerPid=0 };
	public:
		/// erzeugt ungueltige PI fuer Tests -> fuerhrt zu keiner Verbindung
		ProcessInfo() {}
		/// erzeugt PI fuer Server
		ProcessInfo(int node) {}
		/// (innerhalb ServerInitialisierung) erzeugt gueltige PI fuer Client
		ProcessInfo(int c, int p, int node) {}
		/// copy-CTor
		ProcessInfo(ProcessInfo const& rhs) {}
		/// deserialisierungs CTor
		ProcessInfo(MessageBuffer const&buffer ) {}

		virtual ~ProcessInfo() {}
	public:
		// der Serialize-Block
		virtual void serialize	( MessageBuffer &buffer ) const {}
		virtual void deserialize( MessageBuffer const&buffer ) {}
		/// wird von ProtocollInfo aus Fabrik-Tabelle heraus aufgerufen
		static ProtocolInfo* create(int , MessageBuffer const& buffer) { return new ProcessInfo(buffer);}
		void swap(ProcessInfo & rhs) { }
	public:
		// Accessoren
		int processId() const { return 0;  }
		int channelId() const { return 0; }
		int node() 			const { return 0; }
		/// wichtig fuer die Unterscheidung PI fuer Server(ungueltig), Client (gueltig, da von Server erzeugt)
		bool isValid() const { return false; }
		/// der Server bekommt eine 'leere' $PI beim Co
		bool isServerInfo() const { return false; }
		/// erzeugt ungueltige PI, da Server-Initialisierung automatisch neuen Channel bereitstellt (hoffentlich)
		virtual SmpProtocolInfo generateDerived(int ) const { return SmpProtocolInfo(); }
		PvString const& shMemName() const { return name_; }
	private:
		/// Vergleichsfunktion erlaubt  den == operator sicher u. public in der Baisklasse zu halten
		virtual bool isEqual(ProtocolInfo const& rhs) const { return false; }
		PvString name_;
	};
	inline std::ostream &operator << (std::ostream &os, ProcessInfo const&i) { return os;}

#endif
} // message
} // system
} // precitec

#endif /*PROCESS_INFO_H_*/
