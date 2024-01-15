#include <cstdlib>
#include <iostream>
#include <string>
#include <sys/neutrino.h>

#include <spawn.h>
#include "Poco/Pipe.h"
#include "Poco/Process.h"
#include "Poco/PipeStream.h"

#include "protocol/protocol.udp.h"
#include "protocol/protocol.qnxMsg.h"
#include "protocol/protocol.null.h"

#include "message/testServer.proxy.h"
#include "message/testServer.server.h"
#include "test.h"

namespace precitec
{	
namespace test
{
	using namespace system;
	using namespace message;
	using namespace test;
	using interface::AbstractInterface;
	using interface::MsgProxy;
	using interface::MsgHandler;
	using interface::MsgServer;
	using interface::TTestServer;
	// Hier wird der eigentliche Test durchgefuehrt
	// der Server wird mitgeliefert
	void testServer(TTestServer<AbstractInterface> &testServer) {

		int a=2, b=3;
		int result = testServer.add(a, b);
		std::cout << "testServer.add(" << a << ", " << b << ") = " << result << std::endl;
	
		double c=2.1, d=3.4;
		PvString bResult = testServer.compare(a, b, c, d) ? "equal" : "unequal";
		std::cout << "testServer.compare(" << a << ", " << b << ", " << c << ", " << d << ") = " << bResult << std::endl;
		bResult = testServer.compare(a, a, d, d) ? "equal" : "unequal";
		std::cout << "testServer.compare(" << a << ", " << a << ", " << d << ", " << d << ") = " << bResult << std::endl;

		PvString hallo("Welt");
		PvString sResult = testServer.copy(hallo); 
		std::cout << "testServer.copy(" << hallo << ") = " << sResult << std::endl;
		
		PvString welt("hallo");
		sResult = testServer.cat(welt, hallo);  
		std::cout << "testServer.cat(" << welt << ", " << hallo << ") = " << sResult << std::endl;
	}

	/// eher trivial der Lokale Server
	void testServer()
	{
		TTestServer<MsgServer>   local;
		testServer(local);
	}
	
	/// der Server mit QXN-Message-Protokoll
	void testServer(ProcessInfo &procInfo)
	{
		//std::cout << "Qnx Client: remoting: " << procInfo << std::endl;
		TTestServer<MsgProxy>   remote(QnxProtocol::sender(procInfo));
		//std::cout << "Client: RemoteProxy created: " << procInfo.channelId() << std::endl;
		testServer(remote);
		//std::cout << "Client: QnxTest done: " << procInfo.channelId() << std::endl;
	}
	
	
	/// der Server mit Udp-Message-Protokoll
	void testServer(SocketInfo &sockInfo)
	{
		//std::cout << "Udp Client: remoting: " << sockInfo << std::endl;
		TTestServer<MsgProxy>   remote(UdpProtocol::create(sockInfo));
		//std::cout << "Client: RemoteProxy created: " << std::endl;
		testServer(remote);
		//std::cout << "Client: QnxTest done: " << std::endl;
	}

	/// der Server mit Udp-Message-Protokoll
	void testServerTcp(SocketInfo &sockInfo)
	{
		//std::cout << "Udp Client: remoting: " << sockInfo << std::endl;
		TTestServer<MsgProxy>   remote(TcpProtocol::create(sockInfo));
		//std::cout << "Client: RemoteProxy created: " << std::endl;
		testServer(remote);
		//std::cout << "Client: QnxTest done: " << std::endl;
	}

	/// der Server mit Null-Message-Protokoll
	void testServer(NullInfo &nullInfo)
	{
		//std::cout << "Client: nulling: " << std::endl;
		SmpProtocol nullProtocol(NullProtocol::create(nullInfo));
		//std::cout << "Client: prot: " << std::endl;
		TTestServer<MsgServer> 	local;
		//std::cout << "Client: 2: " << std::endl;
		TTestServer<MsgHandler> handler(&local, nullProtocol); 
		//std::cout << "Client: 3: " << std::endl;
		TTestServer<MsgProxy>   remote(nullProtocol);
		//std::cout << "Client: 4: " << std::endl;
		testServer(remote);
		//std::cout << "Client: NullTest done: " << std::endl;
		
	}

	
} // namespace test
} // namespace precitec


