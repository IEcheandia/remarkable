/**
 * tesMsgBuffer::shMemAllocator.cpp
 *
 *  Created on: 11.08.2010
 *      Author: WoX
 *   Copyright: Precitec Vision KG
 */

#include "system/shMemAllocator.h"
#include "system/templates.h" // wg iMax

namespace precitec
{
namespace system
{
namespace message
{

	/// Test-Funktion mit std-Memory
	void ShMemAllocator::setMem(PChar p, int size, PvString const& name) {
		mem_ = p;
		limSize_ = size;
		//accessMutex_ = new Poco::NamedMutex(name);
		initFreeList(size);
	}

	/// std-Initialisierung mit SharedMem
	void ShMemAllocator::setMem(SharedMem const &mem) {
		mem_ = PChar(mem.begin());
		limSize_ = mem.size();
		//accessMutex_ = new Poco::NamedMutex(mem.name());
		initFreeList(limSize_);
	}

	/// Freelist erhaelt einen Eintrag mit kompletten Speicher
	void ShMemAllocator::initFreeList(int totalSize) {
		freeList_ 				= memToElement(mem_);
		freeList_->length = totalSize;
		freeList_->last 	= elementToPos(freeList_);
		freeList_->next 	= elementToPos(freeList_);
		freeList_->isFree	= true;
	}

	/// gibt Speicherblock der Groesse size zurueck: *(int*)(p-4) == size
	PChar ShMemAllocator::allocBlock(int size, int recursionDepth) {
		//std::cout << "\tallocBlock(" << size << ")" << std::endl;
		if (size==0) { return NULL;	}
		int length = size + headerSize();
		MemChunk *usedBlock = freeList_; // den probieren wir zu nehmen
		if (int(length+headerSize())<int(freeList_->length) ) { // Achtung! Platz fuer neuen Header muss beruecksichtigt werden! -> dabher das +sizeof(...)
			// usedBlock wird aufgeteilt
			freeList_ = splitElement(usedBlock, length);
			usedBlock->length = length;
		} else if (length>int(freeList_->length)) {
			if (recursionDepth>10) {
				std::cout << "\trecursing allocBlock(" << size << ") giving up" << std::endl;
				printFreeList(std::cout); std::cout << std::endl;
				return NULL;
			}
			//std::cout << "\trecursing allocBlock(" << size << ") found free: ";	freeList_->print(std::cout, mem_);	std::cout << std::endl;
	//				printList(std::cout);

			// Rekursion sollt nicht wirklich problematisch sein;
			// wg Performance ggf Endrekursion in Schleife wandeln
			if (!mergeElement(freeList_)) {
				freeList_ = nextFree(freeList_);
				mergeElement(freeList_);
			}
			return allocBlock(size, recursionDepth+1);
		} else {
			// einfachster Fall: angeforderter Speicher passt genau ins naechste Freielement
			// used Block ist alte freeList_; daher aus Liste entfernen und zurueckgeben
			freeList_ = removeElement(usedBlock);
		}

		usedBlock->isFree = false;
		// um Fragmentierung zu vermeiden, versuchen wir FreiListe mit naechstem Element zu mergen
		mergeElement(freeList_);
		return elementToMem(usedBlock) + headerSize();
	}
	/// gibt Speicherblock wieder frei; ist nicht Echtzeit wg findFreeElement
	void ShMemAllocator::freeBlock(PChar p) {
		//std::cout << "\tfree(@" << p-mem_-headerSize() << ")" << std::endl;

		MemChunk *element = memToElement(p-headerSize());
		element->isFree = true;
		if (!isValid(element)) { std::cout << "free: invalid element " << std::endl; return; }
		insertElement(element);

		bool merged = mergeElement(element);
		if (merged) mergeElement(element);
		if (merged) mergeElement(element);
	}

	/// die Freiliste muss immer geschlossen sein, test-only
	bool ShMemAllocator::isFreeListARing() const {
		int i=0;
		//std::cout << "\t\tisFreeListARing" << std::endl;
		MemChunk *element = nextFree(freeList_);
		while (isValid(element) && (element!=freeList_) && (++i<50)) { element = nextFree(element);}
		//std::cout << "\t\tisFreeListARing ->" << (element==freeList_?"true":"false") << " ok" << std::endl;
		return element==freeList_;
	}

	/**
	 * total-Speicher incl. Header wg Vollstaendigkeitskontrolle
	 * @return
	 */
	std::size_t ShMemAllocator::getFragmentedFreeMemSize() {
		if (!isValid(freeList_)) return -1;
		std::size_t size = freeList_->length;
		int i=0;
		for (MemChunk *element = nextFree(freeList_);	isValid(element) && (element !=freeList_) && (++i<50); element = nextFree(element)) {
			size += element->length;
		}
		return  size;
	}


	/// gitb max USER-Speicherblock zurueck (ohne Header (was den User alleine interessiert)
	std::size_t ShMemAllocator::getMaxFreeMemBlock() {
		std::size_t size = 0;
		for (MemChunk *element = freeList_;	element !=freeList_; element = nextFree(element)) {
			size = iMax(size, element->length);
		}
		return  size-headerSize();
	}

	/**
	 * reine Testfunktion
	 * @return Speicher
	 */
	std::size_t ShMemAllocator::getFreeMemSize() {
		//std::cout << "\t\tgetFreeMemSize" << std::endl;
		cleanList();
		//printList(std::cout);
		//std::cout << "\t\tgetFreeMemSize -> " << freeList_->length << " ok" << std::endl;
		return freeList_->length;
	}

	/// wir mergen alle mergebaren Elemente (wenn nix allokiert ist, bleibt einElementmit allem Speicher)
	void ShMemAllocator::cleanList() {
		//std::cout << "\t\tcleanList" << std::endl;
		MemChunk *element;
		for (element = freeList_; nextFree(element) !=freeList_; element = nextFree(element)) {
			bool merged = true;
			for ( ; merged==true; ) {
				merged = mergeElement(element);
			}
		}
		mergeElement(element);
		//std::cout << "\t\tcleanList ok" << std::endl;
	}

	/// wir gehen von Anfang durch alle Listenelemente und drucken sie aus
	void ShMemAllocator::printFreeList(std::ostream &os) {
		//std::cout << "pfl: " << reinterpret_cast<PChar>(freeList_)-mem_ << std::endl;
		MemChunk *element = freeList_;
		element->print(os, mem_, 0);
		element = nextFree(element);
		for (int i=1;	element !=freeList_; element = nextFree(element), ++i) {
			element->print(os, mem_, i);
			os << " :: ";
			if (! isValid(element)) {	os << " invalid!!!!"; return;	}
		}
		os << std::endl;
	}

	/// wir gehen von Anfang durch alle Listenelemente und drucken sie aus
	void ShMemAllocator::printList(std::ostream &os) {
		MemChunk *element = reinterpret_cast<MemChunk *>(mem_);
		for (int i=0;  ;++i) {
			element->print(os, mem_, i);
			os << " :: ";
			element = next(element);
			if (!isValid(element)) break;
			if (i>30) break; // Bug-Bremse
		}
	}

	bool ShMemAllocator::isValid(MemChunk *element) const {
		return uInt(elementToPos(element)) < uInt(limSize_);
	}

	/// versucht Element mit naechstem Element zu mergen
	bool ShMemAllocator::mergeElement(MemChunk *element) {
		//std::cout << "\t\t\tmergeElement(" ;
	//			element->print(std::cout, mem_);
	//			std::cout << " with ";
	//			next(element)->print(std::cout, mem_);
	//			std::cout << ")" << std::endl;
		MemChunk *nextElement = next(element);
		if (	 !isValid(nextElement) 			// Pufferende
				|| !nextElement->isFree ) {		// nicht frei
			//std::cout << "\t\t\tmergeElement(" ;
			//element->print(std::cout, mem_);
			//std::cout << " -> false " << " ok" << std::endl;
			return false; // nicht gemerged
		}

	//			lastFree(element)->print(std::cout, mem_);	std::cout << " :: ";
	//			element->print(std::cout, mem_); std::cout << " :: ";
	//			element->next->print(std::cout, mem_); std::cout << " :: ";
	//			element->next->next->print(std::cout, mem_); std::cout << std::endl;

		element->length += nextElement->length;
		if (	 !isValid(nextElement)) {
			std::cout << "merge failed wg length" << std::endl;
		}

		//std::cout << "\t\t\tmergeElement: nextElement " ;	nextElement->print(std::cout, mem_);	std::cout  << std::endl;
		// nextElement aus seiner Listenumgebung streichen
		removeElement(nextElement);

	//			lastFree(element)->print(std::cout, mem_);	std::cout << " :: ";
	//			element->print(std::cout, mem_); std::cout << " :: ";
	//			element->next->print(std::cout, mem_); std::cout << std::endl;

		// wenn wir freeList_ wegmergen, muessen wir das korrigieren
		if (nextElement==freeList_) {
			freeList_ = element;
		}

	//			std::cout << "\t\t\tmergeElement(" ;
	//			element->print(std::cout, mem_);
	//			std::cout << ") -> true" << " ok" << std::endl;
		return true;
	}

	/// von Element werden von vorne size Bytes abgespalten und der Rest als Element zurueckgegeben
	ShMemAllocator::MemChunk * ShMemAllocator::splitElement(MemChunk *element, int length) {
		//std::cout << "\t\t\tsplitElement(" ;
		//element->print(std::cout, mem_);
		//std::cout << ", " << size + headerSize() << ")" << std::endl;
		// da length-Member in Block erhalten bleibt, muessen ein paar Bytes mehr allokiet werden
		// liegt naechstes Element direkt hinter diesem; wir verschieben element size Bytes nach hinten
		std::size_t newLength = element->length - length;
		element->length = length;

		MemChunk *newElement = next(element);
		newElement->length = newLength;
		newElement->isFree = true;

		insertBefore(newElement, element);


		if (	 !isValid(newElement) ){
			std::cout << "split failed invalid newElement: " << std::endl;
		}
		if (	 !isValid(element) ){
			std::cout << "split failed invalid element: " << std::endl;
		}

		//bool merged =  mergeElement(newElement);
		//if (merged) mergeElement(newElement);

//		std::cout << "\t\t\tsplitElement(" ;
//		element->print(std::cout, mem_);
//		std::cout << ", " << length << ") -> ";
//		element->print(std::cout, mem_);
//		std::cout << " :: ";
//		newElement->print(std::cout, mem_);
//		std::cout << " ok" << std::endl;
		return newElement;
	}

	/// sucht in linked List nach freiem Element, kann nicht scheitern, da Ringliste + element frei
	ShMemAllocator::MemChunk * ShMemAllocator::findFreeElement(MemChunk *element) {
		MemChunk *nextElement(next(element));
		while (!nextElement->isFree && (nextElement!=element)) { nextElement = next(nextElement);	}
		return nextElement;
	}

	/**
	 * Next geht zum naechsten Listenelement indem es
	 *
	 * ohne den ListenPointer zu verwenden
	 * @param element
	 * @return
	 */
	ShMemAllocator::MemChunk * ShMemAllocator::next(MemChunk *element) const {
		intptr_t position = elementToPos(element) + element->length;
		return memToElement(position>limSize_ ? mem_ : mem_+position);
	}


	// Listen-Funktionen
	ShMemAllocator::MemChunk* ShMemAllocator::removeElement(MemChunk *element) {
		MemChunk *n = nextFree(element);
		MemChunk *l = lastFree(element);
		n->last = element->last;
		l->next = element->next;
		return n;
	}

	void ShMemAllocator::insertElement(MemChunk *element) {
		element->next = elementToPos(freeList_);
		element->last = freeList_->last;
		freeList_->last = elementToPos(element);
		lastFree(element)->next = elementToPos(element);
	}

	void ShMemAllocator::insertBefore(MemChunk *newElement, MemChunk *element) {
		MemChunk *l = lastFree(element);
		newElement->last = element->last;
		l->next = elementToPos(newElement);

		MemChunk *n = nextFree(element);
		newElement->next = element->next;
		n->last = elementToPos(newElement);
	}

} // namespace message
} // namespace system
} // namespace precitec

