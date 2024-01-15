#ifndef MEMPOOL_H_
#define MEMPOOL_H_

#include <list>

#include "system/types.h"

namespace precitec
{
namespace system
{
	/**
	 * MemChunk ist ein Speicherbereich aus einem Poco::MemoryPool
	 * ein Byte-Ptr wird intern verwendet, um einfache Ptr-Arithmetik zu erlauben.
	 */
	class MemChunk
	{
	public:
		/// wird etwa wg std::Container benoetigt (Nice-Class)
		MemChunk() : memPtr_(NULL) {}
		/// der normale CTor mit einem nacten Pointer, einem Mem-Chunk
		explicit MemChunk(PVoid p) : memPtr_(PByte(p)) {}
		/// der copy-CTor mit Value-Semantic
		MemChunk(MemChunk const& sc) : memPtr_(sc.memPtr_) {}
		/// DTor tut nichts, der SPeicher ist unter eines anderen Verwaltung
		virtual ~MemChunk() {}
		/// wg Nice-Class
		void operator = (MemChunk const& p) { memPtr_ = p.memPtr_; }
		/// wg Nice-Class
		bool operator == (MemChunk const& p) const { return memPtr_ == p.memPtr_; }
		/// wg Nice-Class
		bool operator < (MemChunk const& p) const { return memPtr_ < p.memPtr_; }
		/// Initialisierung mit nacktem Ptr
		void operator = (void *p) { memPtr_ = PByte(p); }
		/// MemChunk ist ein sehr duenner Wrapper um den Pointer
		operator PVoid () const { return PVoid(memPtr_); }
		/// MemChunk ist ein sehr duenner Wrapper um den Pointer
		operator PByte () const { return PByte(memPtr_); }
		/// Pointer + Offset = Pointer
		friend PByte operator + (MemChunk p, int offset) { return &p.memPtr_[offset]; }
		/// Offset  + Pointer = Pointer
		friend PByte operator + (int offset, MemChunk p) { return &p.memPtr_[offset]; }
		/// Offset  + Pointer = Pointer
//		friend bool operator == (PByte p, MemChunk sp) { return sp.memPtr_==p; }
//		friend bool operator == (MemChunk sp, PByte p) { return sp.memPtr_==p; }
	protected:
		PByte memPtr_;
	}; // MemChunk

	typedef MemChunk *ChunkPtr;
	typedef std::list<MemChunk> Chunks;
	typedef Chunks::iterator ChunkIter;
	typedef Chunks::const_iterator ChunkCIter;

} // system
} // precitec

#endif /*MEMPOOL_H_*/
