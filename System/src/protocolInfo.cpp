
#include "SystemManifest.h"

#include "protocol/protocol.info.h"
#include "protocol/process.info.h"
#include "protocol/protocol.h"


namespace precitec
{
namespace system
{
namespace message
{

	template <>
	SYSTEM_API ProtocolInfo* CreateProtocolInfo<Udp>() 			{ return new SocketInfo(Udp); }
	template <>
	SYSTEM_API ProtocolInfo* CreateProtocolInfo<NullPCol>() 	{ return new NullInfo(); }
	template <>
	SYSTEM_API ProtocolInfo* CreateProtocolInfo<Qnx>() 			{ return new ProcessInfo(); }


	/// Fabrikfunktion fuer die untenstehende Tabelle, wg Polymorpher Deserialisierung
	ProtocolInfo * ProtocolInfo::create(int type, MessageBuffer const&buffer)
//	{
//		//std::cout << "ProtocolInfo create " << type << std::endl;
//		ProtocolInfo* info;
//		if (type>NumActiveProtocols) { type = Udp; std::cout << "wrong protocol deserialized" << std::endl; }
//		info = create(ProtocolType(type));
//		//std::cout << "ProtocolInfo create created" << type << std::endl;
//		info->deserialize( buffer );
//		//std::cout << "ProtocolInfo deserialized" << type << std::endl;
//		return info;
//	}
	{
		typedef ProtocolInfo* (*Factory) (int type, MessageBuffer const&buffer);
		/// Achtung!!!! Performance-Bug??!!???? Liste wird erst bei erstem Aufruf erzeugt
		if (type>NumActiveProtocols) {
			type = Udp;
			std::cout << "wrong protocol deserialized" << std::endl;
			throw;
		}
		/// die Static-Tabelle wird einmal initialisiert, danach ist das ganze recht effizient
		static Factory factoryList[NumProtocols] = {
				SocketInfo::create,
				ProcessInfo::create,
				SocketInfo::create,
				NullInfo::create
		};
		return factoryList[type](type, buffer);
	}

	template <>
 	SmpProtocolInfo createBaseProtocolInfo<Udp>() {
 		return SmpProtocolInfo(new SocketInfo(Udp, "127.0.0.1", "50000"));
 	}

 	template <>
 	SmpProtocolInfo createBaseProtocolInfo<Qnx>() {
 		//std::cout << "createBasePInfo<Qnx>: "  << std::endl;
 		return SmpProtocolInfo(new ProcessInfo(ProcessInfo::LocalNode));}

 	template <>
 	SmpProtocolInfo createBaseProtocolInfo<NullPCol>() {
 		return SmpProtocolInfo(new NullInfo);
	}

 	/// wir verwendne im MM ganze Serien von Protokollen, hier werden seie generiert
	SmpProtocolInfo ProtocolInfo::generateDerived(int i) const {
		std::cout << "ProtocolInfo::generateDerived should not be called: " << std::endl;
		return SmpProtocolInfo();
	}

#if defined __QNX__ || defined __linux__
 	/// wir verwenden im MM ganze Serien von Protokollen, hier werden seie generiert
	SmpProtocolInfo ProcessInfo::generateDerived(int ) const {
		//std::cout << "ProcessInfo::generateDerived: " << ProcessInfo(ProcessInfo::InvalidChannel, ProcessInfo::CallerPid, node_)<< std::endl;
		return SmpProtocolInfo(new ProcessInfo(ProcessInfo::InvalidChannel, ProcessInfo::CallerPid, node_) );
	}
#endif

	/// der MM braucht viele verschiedene Verbindunge, hier werden sie systematisch generiert
	SmpProtocolInfo SocketInfo::generateDerived(int i) const {
		if (!isValid()) return SmpProtocolInfo(new SocketInfo(type()));
		//std::cout << "generate derived<" << type() << " " << ip() << ">: (" << i << ") " << toString(i+	port()) << std::endl;
		return SmpSocketInfo(new SocketInfo(type(), ip(), std::to_string(i+	port() )) );
	}

	SmpProtocolInfo SocketInfo::generateNextDerived()	const {
		if (!isValid()) return SmpProtocolInfo(new SocketInfo(type()));
		//std::cout << "generate next: () " << type() << " " << ip() << toString(1+	port()) << std::endl;
		//std::cout << "generate next: " << *this << std::endl;
		return SmpSocketInfo(new SocketInfo(type(), ip(), std::to_string(1+	port() )) );
	}

	/**
	 * dank switch kann man auch ie SmpProtocolInfos ausgeben
	 */
	std::ostream &operator << (std::ostream &os, ProtocolInfo const&i) {
		switch (i.type()) {
		case Tcp: case Udp:
			os << *dynamic_cast<const SocketInfo*>(&i); break;
		case Qnx:
#if defined __QNX__ || defined __linux__
			os << *dynamic_cast<const ProcessInfo*>(&i); break;
#else
			os << "***QnxInfo***"; break;
#endif
		case NullPCol:
			os << "NullInfo"; break;
		default:
			os << "invalid ProtocolInfo-Type"; break;
		}
		return os;
	}

 	std::ostream &operator << (std::ostream &os, SocketInfo const&i) {
 		os << "SocketInfo<" << i.type() << ">["<< i.ip() << ":" << i.port() <<  "-" << i.portString() << "]";
 		return os;
 	}


#if defined __QNX__ || defined __linux__
	std::ostream &operator << (std::ostream &os, ProcessInfo const&i) {
		os << "ProcessInfo[ CHID: ";
		if (i.channelId()==ProcessInfo::InvalidChannel) { os << " InvalidChannel "; }
		else if (i.isServerInfo()) { os << " ServerInfo (uninited) "; }
		else { os << i.channelId();}
		os << " - PID: ";
		if (!i.isServerInfo()) {
			if (i.processId()==ProcessInfo::InvalidPid) { os << " InvalidPid "; }
			else { os << i.processId(); }
		}
		os << " - Node: ";
		os << (i.node()==int(ProcessInfo::InvalidNode) ? "invalidNode" :
					 i.node()==int(ProcessInfo::LocalNode)   ? "localNode" :
																											std::to_string(i.node())) << std::dec << "]";
		if (!i.shMemName().empty()) os << " using: " << i.shMemName();
		return os;
	}

#endif

}
}
}
