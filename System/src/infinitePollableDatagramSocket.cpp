#include "protocol/infinitePollableDatagramSocket.h"
#include <cstring>

#include <sys/epoll.h>

namespace precitec
{
namespace system
{
namespace message
{

InfinitePollableDatagramSocket::InfinitePollableDatagramSocket()
    : Poco::Net::DatagramSocket()
    , m_epollFd(epoll_create1(EPOLL_CLOEXEC))
{
}

InfinitePollableDatagramSocket::~InfinitePollableDatagramSocket()
{
    ::close(m_epollFd);
}

bool InfinitePollableDatagramSocket::infinitePoll()
{
    if (m_epollFd == -1)
    {
        return false;
    }
    if (!m_initialized)
    {
        const int fd = sockfd();
        if (fd == -1)
        {
            return false;
        }
        struct epoll_event ev;
        memset(&ev, 0, sizeof(ev));
        ev.data.fd = fd;
        ev.events = EPOLLIN;
        if (epoll_ctl(m_epollFd, EPOLL_CTL_ADD, fd, &ev) == -1)
        {
            return false;
        }
        m_initialized = true;
    }

    struct epoll_event evlist;
    memset(&evlist, 0, sizeof(evlist));
    if (epoll_wait(m_epollFd, &evlist, 1, -1) <= 0)
    {
        return false;
    }

    return true;
}

}
}
}
