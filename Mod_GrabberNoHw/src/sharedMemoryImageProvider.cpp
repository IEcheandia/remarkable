/**
*
* @defgroup Framegrabber Framegrabber
*
* \section sec Image Loading
*
*
* @file
* @brief  Shared memory image provider to store images in a shared memory segment.
* @copyright    Precitec GmbH & Co. KG
* @author GM
*
*
*/
#include "grabber/sharedMemoryImageProvider.h"
#include "event/imageShMem.h"

namespace precitec
{
using namespace greyImage;
namespace grabber
{

SharedMemoryImageProvider::SharedMemoryImageProvider():
    m_memory()
    , m_offset(0)
{
}

void SharedMemoryImageProvider::init(bool locked, bool simulation)
{
    m_memory.set(sharedMemoryHandle(), sharedMemoryName(), locked ? precitec::system::SharedMem::StdLockedClient : precitec::system::SharedMem::StdClient , sharedMemorySize(simulation));
}

precitec::system::ShMemPtr<byte> SharedMemoryImageProvider::nextImagePointer(int bytes)
{
    precitec::system::ShMemPtr<byte> sharedMemoryPointer;
    static int counter = 0;
    counter++;
    if (m_offset + bytes > m_memory.size())
    {
        std::cout << "Shared memory wrapped around at image " << counter << std::endl;
        m_offset = 0;
    }
    sharedMemoryPointer.set(m_memory, m_offset);
    m_offset += bytes;

    return sharedMemoryPointer;
}

int SharedMemoryImageProvider::size() const
{
    return m_memory.size();
}

precitec::system::ShMemPtr<byte> SharedMemoryImageProvider::fromExistingPointer(byte *address)
{
    return precitec::system::ShMemPtr<byte>{m_memory, address};
}

}
}
