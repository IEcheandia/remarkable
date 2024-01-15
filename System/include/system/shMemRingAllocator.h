/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Wolfgang Reichl (WoR), SB, HS
 * 	@date		2016
 * 	@brief		Shared-memory buffer for event interfaces.
 */

#ifndef SH_MEM_RING_ALLOCATOR_H_
#define SH_MEM_RING_ALLOCATOR_H_

// WM includes
#include "SystemManifest.h"

namespace precitec {
namespace system {
	class SharedMem;	// fwd decl
namespace message {


/**
 * CLASS NAME DEPRECATED: the current implementation is a FIFO queue
 * Ringallokator verwaltetn sein Speicherpool nicht selbst. Speicher wird weder allokiert noch
 * gefreed. Der SPeicher wird als ein Block angenommen.
 * Ringallokator ist fuer die SharedMem-Verwaltung fuer das Messaging (Pulse) unter QNX gedacht
 * worden. Der Speicher sollte fuer 'hinreichend' viele Events ausgelegt sein. Da grosse
 * Datenmengen sowieso ueber andere SharedMems laufen, sollte die Groesse des Spechers vertretbar
 * bleiben.
 */
class SYSTEM_API ShMemRingAllocator {
public:
	/// allocates fixed size fifo queue from shared memory
	ShMemRingAllocator( std::string p_oName, SharedMem const &p_rShMem, int p_oPayloadSize);

	~ShMemRingAllocator()=default;
	ShMemRingAllocator(const ShMemRingAllocator&)=delete;

	/// payload size, without header
	int payloadSize() const { return m_oPayloadSize; }

	/// header size
	static int headerSize();

	/// allocates a block of fixed size
	char* allocBlock(int numReaders);

	/// frees an allocated block
	void freeBlock(char* p_pPayloadMemPos);

protected:

	/// char* from shmem offset
	char* shMemOffsetToMemPos(int position) const { return  m_pShMemBegin + position; }

	/// shmem offset from char*
	intptr_t memPosToShMenOffset(char* p_pMemPos) const { return  p_pMemPos - m_pShMemBegin; }

private:
	struct Header;

	/// initialize headers for all blocks
	void init();

	/// cast to char* pointer
	char* headerToMemPos(Header* p_pHeader) const;

	/// cast from char* pointer
	Header* memPosToHeader(char* p_pMemPos) const;

	char* const			m_pShMemBegin;		///< mem pos of begin of shared memory buffer
	char* const			m_pShMemEnd;		///< mem pos of end of shared memory buffer
	const int 			m_oShMemSize;  		///< size of shared memory in bytes
	const int			m_oPayloadSize;		///< size of payload of one block
	const int			m_oBlockSize;		///< size of header plus size of payload
	const std::string 	m_oName; 			///< name of interface
	bool m_allocFailed = false;
};

} // namespace message
} // namespace system
} // namespace precitec

#endif // SH_MEM_RING_ALLOCATOR_H_
