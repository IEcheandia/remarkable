/*
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
#pragma once

#include "system/sharedMem.h"

namespace precitec
{
namespace grabber
{

class SharedMemoryImageProvider
{
public:
    explicit SharedMemoryImageProvider();

    /**
     * Delayed initialization of the shared memory section
     **/
    void init(bool locked = false, bool simulation = false);

    /**
     * @param bytes The number of bytes for the new ShMemPtr
     * @returns a new ShMemPtr in the shared memory section for no grabber mode.
     * Please note that the pointer wrapps around, so existing memory will be overwritten
     **/
    precitec::system::ShMemPtr<byte> nextImagePointer(int bytes);

    /**
     * Creates a ShMemPtr from the raw @p address in case the address was generated from
     * the same shared memory segment.
     **/
    precitec::system::ShMemPtr<byte> fromExistingPointer(byte *address);

    int currentOffset() const
    {
        return m_offset;
    }

    /**
     * Resets the offset for generating image pointers to @p offset.
     * Use with caution, it can overwrite existing pointers
     **/
    void resetOffset(int offset)
    {
        m_offset = offset;
    }

    /**
     * @returns The size of the shared memory segment
     **/
    int size() const;

private:
    precitec::system::SharedMem  m_memory;
    int m_offset;
};

}
}
