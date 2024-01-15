#ifndef SHMEMALLOCATOR_H_
#define SHMEMALLOCATOR_H_
#pragma once

/**
 * tesMsgBuffer::shMemAllocator.h
 *
 *  Created on: 11.08.2010
 *      Author: WoX
 *   Copyright: Precitec Vision KG
 */

#include "Poco/Random.h"
#include "system/timer.h"
#include "system/sharedMem.h"
#include "SystemManifest.h"

/// tesMsgBuffer::precitec::system::message::ShMemAllocator
namespace precitec
{
namespace system
{
namespace message
{

struct RingShMemChunk {
	typedef Timer::Time TimeStamp;
	/// lenght ist Datenlaenge inkl. Header
	size_t	length;
	/// ist Element NICHT alokiert
	bool	isFree;
	// = 0 fuer freie Bloecke; Allokationszeit fuer verwendete Blocks
	// gleichzeitig Frei-Marker und
	TimeStamp	timeStamp;
	/// Konsistenz-Test innerhalb des Headers
	int checkSum;

	// Zeiger auf naechstes/letztes Element ist nur fuer freie Bloecke relevant. Wg SharedMem-Zugriff
	// von verschiedenen Prozessen wird nur der Offset auf den Start des ShMem verwendet
	intptr_t		next;
	intptr_t		last;
	bool isChunkFree() const { return timeStamp ==0; }
	void print(std::ostream &os, PChar start, int index=-1) {
		os << "C";
		if (index !=-1) os << "[" << index << "]";
		os << "@" << int(PChar(this)-start) << (isFree?"-":"+")
			 << length << "(" << length-sizeof(RingShMemChunk) << ") ->" << next;
	}
};
/**
 * ShMemAllocator verwaltet ein 1-Block conti. Speicher-Pool.
 * ShMemAllocator hat O(1) alloc aber O(n) free
 * ShMemAllocator ist sinnvoll, wenn Speicher FIFO zurueckgegeben wird, dann ist auch free O(1)
 * Ringallokator verwaltetn sein Speicherpool nicht selbst. Speicher wird weder allokiert noch
 * gefreed. Der SPeicher wird als ein Block angenommen.
 * Ringallokator ist fuer die SharedMem-Verwaltung fuer das Messaging (Pulse) unter QNX gedacht
 * worden. Der Speicher sollte fuer 'hinreichend' viele Events ausgelegt sein. Da grosse
 * Datenmengen sowieso ueber andere SharedMems laufen, sollte die Groesse des Spechers vertretbar
 * bleiben.
 */
class SYSTEM_API ShMemAllocator {
public:
	/// Leerer CTor(mit setMem()) fuer verzoegerte initialisierung
	ShMemAllocator() : mem_(NULL), limSize_(0) {}
	/// Test-CTor mit normalem Mem
	ShMemAllocator(PChar p, int size)
	: mem_(p), limSize_(size)
//	, accessMutex_(NULL/*new Poco::NamedMutex(mem.name())*/)
	{ initFreeList(size); }
	/// Std-Allokator fuer ShMem
	ShMemAllocator(SharedMem const &mem)
		: mem_(PChar(mem.begin())), limSize_(mem.size())
//		,	accessMutex_(NULL/*new Poco::NamedMutex(mem.name())*/)
	{
		initFreeList(limSize_);
	}
	~ShMemAllocator() {
//		if (accessMutex_) delete accessMutex_;
	}
	typedef RingShMemChunk MemChunk;

public:
	/// Test-Funktion mit std-Memory
	void setMem(PChar p, int size, PvString const& name);

	/// std-Initialisierung mit SharedMem
	void setMem(SharedMem const &mem);

	/// Freelist erhaelt einen Eintrag mit kompletten Speicher
	void initFreeList(int totalSize);
	/// gibt Speicherblock der Groesse size zurueck: *(int*)(p-4) == size
	PChar allocBlock(int size, int recursionDepth=0);
	/// gibt Speicherblock wieder frei; ist nicht Echtzeit wg findFreeElement
	void freeBlock(PChar p);
public:
	/// die Freiliste muss immer geschlossen sein, test-only
	bool isFreeListARing() const;
	/**
	 * total-Speicher incl. Header wg Vollstaendigkeitskontrolle
	 * @return
	 */
	std::size_t getFragmentedFreeMemSize();


	/// gitb max USER-Speicherblock zurueck (ohne Header (was den User alleine interessiert)
	std::size_t getMaxFreeMemBlock();

	/**
	 * reine Testfunktion
	 * @return Speicher
	 */
	std::size_t getFreeMemSize();
	/// wir mergen alle mergebaren Elemente (wenn nix allokiert ist, bleibt einElementmit allem Speicher)
	void cleanList();

	/// wir gehen von Anfang durch alle Listenelemente und drucken sie aus
	void printFreeList(std::ostream &os);
	/// wir gehen von Anfang durch alle Listenelemente und drucken sie aus
	void printList(std::ostream &os);
	/// Headergroesse: muss zu benoetigtem Speicher addiertwerden
	static int headerSize() { return sizeof(MemChunk); }

protected:
	/// aus Offset Pointer berechnen
	PChar getReadPointer(int offset) const { return posToMem(offset); }
	/// aus Offset Pointer berechnen
	MemChunk * getRingBufferHeader(int offset) const { return posToElement(offset); }
	/// offset fuer Pointer in Ringpuffer berechnen
	intptr_t getWritePuffer(PChar mem) const { return  mem - mem_;	}
	/// Gesamtpufferlaenge in Bytes
	int buffSize() const { return limSize_; }
private:

	/// versucht Element mit naechstem Element zu mergen
	bool mergeElement(MemChunk *element) ;
	/// von Element werden von vorne size Bytes abgespalten und der Rest als Element zurueckgegeben
	MemChunk * splitElement(MemChunk *element, int length);

	///
	//void readLock() {}
	///
	//void writeLock() {}
	/// lock
	//void lockList() {}
	/// unlock
	//void unlockList() {}
private:
	// Pointer-Arithmetik
	/// ist Pointer gueltig
	bool isValid(MemChunk *element) const;
	/// Pointer auf PChar casten
	PChar elementToMem(MemChunk *element) const {	return reinterpret_cast<PChar>(element); }
	/// Pointer relativ zu Pufferanfang
	intptr_t elementToPos(MemChunk *element) const {	return reinterpret_cast<PChar>(element) - mem_;	}
	/// Element-Pointer aus PChar
	MemChunk *memToElement(PChar p) const {	return  reinterpret_cast<MemChunk*>(p); }
	/// Element-Pointer aus Abstand zu Pufferanfang
	MemChunk *posToElement(intptr_t position) const { return  reinterpret_cast<MemChunk*>(mem_ + position); }
	/// PChar aus Abstand zu Pufferanfang
	PChar posToMem(int position) const { return  mem_ + position;	}
	/// Abstand zu Pufferanfang aus PChar
	intptr_t memToPos(PChar mem) const { return  mem - mem_;	}
private:
	// Listen-Funktionen
	/// sucht in linked List nach freiem Element, kann nicht scheitern, da Ringliste + element frei
	MemChunk * findFreeElement(MemChunk *element);
	/// naechstes Element im Speicher
	MemChunk * next(MemChunk *element) const;
	/// naechstes Elementin Freiliste
	MemChunk * nextFree(MemChunk *element) const { return posToElement(element->next); }
	/// voriges Element in Freiliste
	MemChunk * lastFree(MemChunk *element) const { return posToElement(element->last); }
	/// Element aus Freiliste entfernen
	MemChunk* removeElement(MemChunk *element);
	/// Element an Kopf von Freiliste setzen
	void insertElement(MemChunk *element);
	/// Element mitten in Freiliste einbauen
	void insertBefore(MemChunk *newElement, MemChunk *element);

private:
	PChar							 mem_;
	MemChunk 		*freeList_;
	int 								limSize_;

//	Poco::NamedMutex	*accessMutex_;
//	Poco::FastMutex	*accessMutex_;
};

} // namespace message
} // namespace system
} // namespace precitec

#endif // SHMEMALLOCATOR_H_
