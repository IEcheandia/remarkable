#include "memory/memoryFile.h"

#include <sys/mman.h>
#include <unistd.h>

namespace precitec
{
namespace system
{
MemoryFile::MemoryFile() = default;

MemoryFile::MemoryFile(const std::string& filename)
    : m_fileDescriptor(memfd_create(filename.c_str(), MFD_ALLOW_SEALING))
{
}

MemoryFile::~MemoryFile()
{
    if (hasFileDescriptor())
    {
        close(m_fileDescriptor);
    }
}

int MemoryFile::setSize(std::size_t size)
{
    return ftruncate(m_fileDescriptor, size);
}

int MemoryFile::write(const std::string& line)
{
    return ::write(m_fileDescriptor, line.data(), line.size());
}

std::pair<std::string, int> MemoryFile::read(std::size_t size)
{
    std::string buffer;
    buffer.reserve(size);
    int readResult = ::read(m_fileDescriptor, &buffer[0], size);
    return std::make_pair(buffer, readResult);
}

long MemoryFile::seekStart()
{
    return lseek(m_fileDescriptor, 0, SEEK_SET);
}
}
}