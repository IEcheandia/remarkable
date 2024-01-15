#pragma once

#include <Poco/Net/DatagramSocket.h>

namespace precitec
{
namespace system
{
namespace message
{

class InfinitePollableDatagramSocket : public Poco::Net::DatagramSocket
{
public:
    explicit InfinitePollableDatagramSocket();
    ~InfinitePollableDatagramSocket() override;

    bool infinitePoll();
private:
    int m_epollFd;
    bool m_initialized = false;
};

}
}
}
