#include "protocol/protocol.info.h"
#include "protocol/protocol.h"
#include "protocol/protocol.udp.h"
#include "protocol/protocol.null.h"
#if defined __QNX__ || defined __linux__
	// das Qnx-Protokoll mit ProcessInfo wird nur unter QNX verwendet
#include "protocol/protocol.qnxMsg.h"
#endif

namespace precitec
{
namespace system
{
namespace message
{
	template <ProtocolType type>
	SYSTEM_API SmpProtocol createTProtocol(SmpProtocolInfo & info) { return SmpProtocol(); }

	SmpProtocol createProtocol(SmpProtocolInfo & info) {
		//std::cout << "createProtocol " << info->type() << "->" << *info << std::endl;
		switch (info->type()) {
			case Udp: return createTProtocol<Udp>(info);
			case Qnx: return createTProtocol<Qnx>(info);
			case NullPCol: return createTProtocol<NullPCol>(info);
			default: { std::cout << "createProtocol invalid type" << std::endl; throw; return NULL; }
		}
	}

	std::ostream& operator << (std::ostream&os, ProtocolType const&p) {
		switch (p) {
			case Udp: os << "Udp"; break;
			case Qnx: os << "Qnx"; break;
			case NullPCol: os << "NullPCol"; break;
			default: os << "invalid Protocol"; break;
		}
		return os;
	}

	/// statischer Puffer fuer UnitTests (noch nicht wirklich optimal dat Janze) \todo, der Test-Puffer gehoert nun wirklich nicht in die Bibliothek!!!!!
	StaticMessageBuffer	NullProtocol::buffer_(1024*1024); // 1 MB sollte fuer erste Tests ausreichen

} // namespace message

} // namespace system
} // namespace precitec
