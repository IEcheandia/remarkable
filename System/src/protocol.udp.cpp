#include "protocol/protocol.udp.h"

namespace precitec
{
namespace system
{
namespace message
{


UdpProtocol::UdpProtocol() : Protocol(Udp), socketInfo_(Udp), timeout_(250*Milliseconds), initialized_(false) {
    //std::cout << "UdpProtocol::CTor() ok :" << std::endl;
}

UdpProtocol::UdpProtocol(SocketInfo const& info)
    : Protocol(Udp), socketInfo_(info), timeout_(250*Milliseconds), initialized_(false) 	{
#if defined UDPTRACE
    std::cout << "UdpProtocol::CTor(SocketInfo)1 ok : " << info << " " << socketInfo_ << "   thread " << ThreadID() << std::endl;
#endif
}

UdpProtocol::UdpProtocol(SmpProtocolInfo const& info)
    : Protocol(Udp), socketInfo_(*dynamic_cast<PSI>(&*info)), timeout_(250*Milliseconds), initialized_(false) 	{
#if defined UDPTRACE
    std::cout << "UdpProtocol::CTor(SocketInfo)2 ok :" << info << " " << socketInfo_ << "   thread " << ThreadID()<< std::endl;
#endif
}

UdpProtocol::~UdpProtocol() {
#if defined UDPTRACE
    std::cout << "UdpProtoco:.DTor: " << std::endl;
#endif
    socket_.close();
}

bool UdpProtocol::isValid(){ return socketInfo_.isValid(); }
/// gibt die ProtocolInfo zurueck (die erst in der abgeleiteten Klasse definiert ist)
ProtocolInfo const& UdpProtocol::protocolInfo()  const { return socketInfo_; }

void UdpProtocol::initSender()	{
    if (isValid() && !initialized_) {
        initialized_ = true;
        SocketAddress  sockadr(socketInfo_.ip(),  socketInfo_.portString() );
        socket_.connect(sockadr);
#if defined UDPTRACE
        std::cout << " Udp initSender " << socketInfo_ << "   thread " << ThreadID() << std::endl;
#endif
    }
}

void UdpProtocol::initReceiver() {
    if (isValid() && !initialized_) {
        try {
            //std::cout << "Server: trying to bind to: " << socketInfo_ << std::endl;
            socket_.bind(SocketAddress(socketInfo_.ip(),  socketInfo_.portString()), true);
            initialized_ = true;
        }	catch(Poco::IOException&)	{
            // ggf ein ReThrow
            std::printf("bind failed\n");
            return ;
        }
#if defined UDPTRACE
        std::cout << " Udp initReceiver " << socketInfo_ << "   thread " << ThreadID() << std::endl;
#endif
    }
}

/**
    * receive wartet auf ein Datagramm und lest dann die Daten ein
    * receive wirft (direkt oder ueber RawRead) Poco::Exception& (timeout)
    * receive ruft rawRead fuer das eigentliche (ggf. mehrphasige) Lesen
    * \param MessageBuffer buffer hierhin werden die Daten kopiert und der Header gesetzt
    * \return Nummer der Message (fuer die Callback-Tabelle)
    */
int UdpProtocol::getMessage(MessageBuffer &buffer) {
    try {
        if (socket_.infinitePoll()) {
            return rawReceiveFrom(buffer, sender_);
        } else {
            return TimeoutMessage;
        }
    } catch (...) {
        return TimeoutMessage;
    }

}

void UdpProtocol::reply(MessageBuffer &replyBuffer) {
    rawReply(replyBuffer, sender_);
}

void UdpProtocol::send(MessageBuffer &sendBuffer, MessageBuffer &replyBuffer) {
    if (!isValid()) { return ; }
    /// Abfrage auf ReturnTyp = PulseType fehlt noch
    // vermoeglicht meherere Message- und Reply-Teile
    try {
        rawSend(sendBuffer);
        replyBuffer.header().messageNum = sendBuffer.header().messageNum;
        rawReceive(replyBuffer);
    } catch (...)	{
        std::cout << "UdpProtocol::send caught unknown exception "  << socketInfo_<< std::endl;
        throw;
    }
}

void UdpProtocol::sendPulse(MessageBuffer &sendBuffer, module::Interfaces ) {
    if (!isValid()) { return ; }
    try {
        rawSend(sendBuffer);
    } catch (...) {
        throw;
    }
}

void UdpProtocol::sendQuitPulse(MessageBuffer &sendBuffer, module::Interfaces interfaceId)
{
    if (!isValid() || !initialized_)
    {
        return;
    }
    rawReply(sendBuffer, SocketAddress{socketInfo_.ip(),  socketInfo_.portString()});
}

void UdpProtocol::rawSend(MessageBuffer &buffer) {
    // Abfrage auf ReturnTyp = PulseType fehlt noch
    // die Message kommt in mehreren Teilen, das muss irgendwo kodiert werden
#if defined UDPTRACE
    int msgSize = buffer.msgSize(); // + sizeof(Header);
    int chkSum 		 = buffer.checkSum();
    std::cout << "Udp::rawSend 	size " << msgSize << "  checkSum " << chkSum << "   " <<  socketInfo_ << "   thread " << ThreadID() << std::endl;
#endif
    int stint = 0;
    std::size_t bytesToSend 	 = 0;
    int bytesSent;
    try	{
        bytesSent			 = socket_.sendBytes(buffer.rawData(), sizeof(Header));
    }	catch (Poco::Exception &e)	{
        std::cout << "udp rawSend Exception @header " << e.what() <<std::endl;
        throw;
    }

    std::size_t restSize = buffer.msgSize();
    buffer.rewind();
    while (restSize>0) {
        bytesToSend = min(std::size_t(UDPSizeLimit), restSize);
        try	{
            bytesSent = socket_.sendBytes(buffer.cursor(), int(bytesToSend));
            restSize -= bytesSent;
            buffer.skip(bytesSent); // only move cursor, leave msgSize intact
            ++stint;
        }	catch (Poco::Exception &/*e*/)	{
            Poco::Thread::sleep(1); // ???? ist neu zu testen, ob wirklich immer noch noetig
        }
    }
}

void UdpProtocol::rawReply(MessageBuffer &buffer, const SocketAddress & sender) {
        // Abfrage auf ReturnTyp = PulseType fehlt noch
    // die Message kommt in mehreren Teilen, das muss irgendwo kodiert werden
    int stint = 0;
    std::size_t bytesToSend  = 0;
    int bytesSent;
#if defined UDPTRACE
    int msgSize = buffer.msgSize();
    int chkSum 		 = buffer.checkSum();
    std::cout << "Udp::rawReply 	size " << msgSize << "  checkSum " << chkSum << "   " <<  socketInfo_ << "   thread " << ThreadID() << std::endl;
#endif
    try	{
        bytesSent = socket_.sendTo(buffer.rawData(), sizeof(Header), sender);
    } catch (...)	{
        std::cout << "udp rawReply Exception @header" << std::endl;
        throw;
    }
    std::size_t restSize = buffer.msgSize(); // + sizeof(Header);
    buffer.rewind(); // curosr zuruecksetzen
    while (restSize>0) {
        bytesToSend = min(std::size_t(UDPSizeLimit), restSize);
        try	{
            bytesSent = socket_.sendTo(buffer.cursor(), int(bytesToSend), sender);
            restSize -= bytesSent;
            buffer.skip(bytesSent);
            ++stint;
        } catch (...)	{
            Poco::Thread::sleep(1); // ist das noch noetig ????
        }
    }
}

int UdpProtocol::rawReceive(MessageBuffer &buffer) {
    buffer.clear(); // cursor + dataSize zurueckgesetzt
    try {
        // erst lesen wir den Header
        std::size_t	byteToRead = sizeof(Header);
        int bytesRead = socket_.receiveBytes(buffer.rawData(), int(byteToRead));

        // im header steht, wie gross die Message tatsaechlich ist
        std::size_t restSize 		 = buffer.msgSize();
#if defined UDPTRACE
        int chkSum 		 = buffer.checkSum();
        std::cout << "Udp::rawReceive 	size " << restSize << "  checkSum " << chkSum << "   " <<  socketInfo_ << "   thread " << ThreadID() << std::endl;
#endif
        // ggff wird solange gelesen bis alles da ist
        while (restSize>0) {
            byteToRead = min(restSize, std::size_t(UDPSizeLimit));
            bytesRead =  socket_.receiveBytes(buffer.cursor(), int(byteToRead));
            restSize -= bytesRead;
            buffer.skip(bytesRead);
        }
    }	catch (...) {
        std::cout << "udp raw receive Exception " << std::endl;
        throw;
    }

    buffer.rewind();
    return buffer.header().messageNum;
}

int UdpProtocol::rawReceiveFrom(MessageBuffer &buffer, SocketAddress & sender)	{
    buffer.rewind(); // cursor + dataSize zurueckgesetzt
    try {
        // erst lesen wir den Header
        std::size_t	byteToRead = sizeof(Header);
        int bytesRead = socket_.receiveFrom(buffer.rawData(), int(byteToRead), sender);

        // im header steht, wie gross die Message tatsaechlich ist
        std::size_t restSize = buffer.msgSize();
#if defined UDPTRACE
        int chkSum 		 = buffer.checkSum();
        std::cout << "Udp::rawReceiveFrom " << sender.toString() << "   size "  << restSize  << "  checkSum " << chkSum << "   " <<  socketInfo_ << "   thread " << ThreadID() << std::endl;
#endif

        // ggff wird solange gelesen bis alles da ist
        while (restSize>0) {
            byteToRead = min(restSize, std::size_t(UDPSizeLimit));
            bytesRead =  socket_.receiveBytes(buffer.cursor(), int(byteToRead));
            restSize -= bytesRead;
            buffer.skip(bytesRead);
        }

    }	catch  (...) {
        std::cout << "udp raw receiveFrom Exception " << std::endl;
        throw;
    }
    buffer.rewind(); // fuers deMarshalling cursor wieder zuruecksetzen
    return buffer.header().messageNum;
}

}
}
}
