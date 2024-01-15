#ifndef DERIVEDMESSAGEBUFFER_H_
#define DERIVEDMESSAGEBUFFER_H_
#pragma once

/**
 * System::derivedMessageBuffer.h
 *
 *  Created on: 22.08.2010
 *      Author: WoX
 *   Copyright: Precitec Vision KG
 */

#include "message/messageBuffer.h"
#include "system/sharedMem.h"
#include "system/shMemRingAllocator.h"
#include "system/templates.h" // wg iMin

namespace precitec
{
namespace system
{
namespace message
{

#if defined __QNX__ || defined __linux__


/**
 * Puffer wird nach Bedarf ueber Allokator bereitgestellt.
 * Bei jeder Message wird eine neuer Puffer genommen.
 * Beim Serialisieren wird bei Speichermangel, neuer Puffer genommen und Daten vom alten uebernommen.
 * Bei kleinen Puffergroessen sollten keine Laufzeitprobleme auftauchen.
 * Annahme SharedMem ist gross genug!
 * 			Ringpuffer voll  -> Exception
 * 			kein intelligenter Allokations-Algo., da Ringpuffer sparce bevoelkert ist (gross genug)
 * Speicher wird 'schnell' allokiert und wieder freigegeben
 * 	-> Puffergroessen-Probleme sehr selten,
 *  -> Performance allokation ist Echtzeit (determiniert)
 *  		dies gilt immer wenn der Ringpuffer gross genug ist
 *	-> Performance Deallokation ist schnell aber nicht streng Echtzeit
 */
class SYSTEM_API SharedMessageBuff : private ShMemRingAllocator, public MessageBuffer {
public:
	//SharedMessageBuff( std::string p_oName ) : ShMemRingAllocator( p_oName ), MessageBuffer() {}
	SharedMessageBuff( std::string p_oName, SharedMem const& shMem, int elementSize)
	: ShMemRingAllocator(p_oName, shMem, elementSize), MessageBuffer() {
		//std::cout << "SharedMessageBuff0: " << shMem << " : " << elementSize << std::endl;

		// erstes Element allokieren ?? wirklich schon jetzt???
		//setRawData(allocBlock(elementSize, multiUse()), shMem.size()-headerSize());
		//std::cout << interfaceName_ << " multiUse " << multiUse() << std::endl;
	}

	/// ShMemAllocator raeumt sich selbst auf, MessageBuffer braucht nix tu tun
	~SharedMessageBuff() {}
protected:
	/// wenn moeglich neuen Puffer mit passender Groesse holen, bisherige Daten kopieren: Aufruf aus Serialisieren heraus
	virtual void resize(int size) {	testRemainingBuffSize(size); }
	/// neuen Puffer mit passender Groesse holen: Aufruf bei InitMessage
	virtual void getNewBuffer(int size) {
		//std::cout << "SharedMessageBuff::getNewBuffer: " << size << std::endl;
		//std::cout << "SharedMessageBuff::getNewBuffer currElement: " << ShMemRingAllocator::currentElement() << std::endl;
		// wir allokieren etwas mehr, um exzessiver Fragmentierung vorzubeugen
		const int ExtraMem = 100; // bytes
		// wir wollen nicht aus Versehen mehr Bytes anfordern, als der Puffer hat
		uInt totalMem = iMin(uInt(size+ExtraMem), uInt(limSize()/*-headerSize()*/));
		//std::cout << "totalMem: " << totalMem << " " << uInt(size+ExtraMem) << " " << limSize() << " " << headerSize() << std::endl;
		//std::cout << "SharedMessageBuff::getNewBuffer1 currElement: " << ShMemRingAllocator::currentElement() << std::endl;
		resetCursor(totalMem); // ??? Rekursion??
		if (rawData()==NULL) {
			std::cout << "SharedMessageBuff::getNewBuffer: failed" << size << std::endl;
		}
	}
	/// is es eine SharedMem Buffer oder nicht
	virtual bool supportsShMemAccess() const { return true; }
	virtual void reset(int size=0) {
		//std::cout << "SharedMessageBuff::reset(" << size << ") currElement: " << ShMemRingAllocator::currentElement() << std::endl;
		// wenn aktueller Puffer schon wieder frei ist (nicht der Normalfall) wiederverwenden
		if (this->lockForWriting()&&(size<=limSize())) MessageBuffer::resetCursor(size);
		// sonst neuen Puffer aus grossem Puffer anfordern
		else 										this->getNewBuffer(size);
		if (rawData()==NULL) {
			std::cout << "MessageBuffer::reset: failed" << std::endl;
		}
	}

public:
	/// (nach RcvPulse) Puffer zum Auslesen auf gegebenen Offset setzen
	void offsetToBuffer(int offset) {
		// jetzt wird er Offset in einenPointer umgerechtnet (basisPointer + offset)
		// ... und dem uebergebenen Puffer als neue Start-Adresse untergschoben
		setRawData(shMemOffsetToMemPos(offset), payloadSize());
		// jetzt kann aus dem Puffer gelesen werden
	}
	/// fuer sendPulse Puffer in Offset umwandeln
	int bufferToOffset() const { return memPosToShMenOffset(rawData()); }
	void copyBuff(PChar start, PChar end, PChar newBuff) {
		memcpy(newBuff, start, end-start);
	}
	/// prueft ob noch size Bytes frei sind, wenn nicht wird neue Puffer angelegt und die Daten kopiert
	virtual bool testRemainingBuffSize(std::size_t size) {
		return true;
	}
	/// gibt den Puffer frei
	//virtual void clear() { MessageBuffer::clear();/*freeBuffer();*/ }
	/// ein WriteLock freigeben
	virtual void freeBuffer() {
		//std::cout << "SharedMessageBuff::unlockRead() " << std::endl;
		freeBlock(rawData());
		MessageBuffer::clear();
		//std::cout << "SharedMessageBuff::freeBuffer currElement: " << ShMemRingAllocator::currentElement() << std::endl;
	}
private:
	// event-Locking-Interface: darf nichts tun, was nicht mit Locking zu tun hat, da die Message-
	// Routinen dieses Interface nicht verwenden. Die defaultImplementierung tut nichts
	/// (Write)Locks fuer setzen (geht nur wenn kein writeLock existiert)
	virtual void lockForReading() {  }
private:
	/// ist Puffer frei?; erst mal soll immer ein neuer Puffer verwendet werden
	virtual bool lockForWriting() {	return false; }
	/// internen Puffer neu setzen
	virtual void resetCursor(int size=0) {
		//std::cout << "SharedMessageBuff::resetCursor currElement: " << ShMemRingAllocator::currentElement() << std::endl;
		const auto  oAllocated	=	allocBlock(multiUse());
		//std::cout << interfaceName_ << " multiUse " << multiUse() << std::endl;
		//std::cout << "SharedMessageBuff::resetCursor currElement: ok "  << std::endl;
		if (oAllocated != nullptr)
		{
			setRawData(oAllocated, size);
		}
		//std::cout << "SharedMessageBuff::resetCursor1 currElement: " << ShMemRingAllocator::currentElement() << std::endl;
		//setRawData(allocBlock(size), size);
	} // resetCursor

	//void getReadAccess() { /*Poco::ScopedLock<Poco::FastMutex> lock(mutex);*/ if (access_!= -1) ++access_; }
	//void getWriteAccess() { /*Poco::ScopedLock<Poco::FastMutex> lock(mutex);*/ if (access_==0) --access_; }
};

/**
 * Puffer liegt in einem recht grossen SharedMem-Ringpuffer und wird dort dynamisch
 * nachallokiert. Die zu allokierende Groesse richtet sich nach dem bisher maximalen Wert.
 * Wenn die nicht reicht wird die Puffergroesse erhoeht. Die kann am Pufferende und vor blockierten
 * Segmenten des Ringpuffers schiefgehen; dann muessen Daten kopiert werden.
 */
/*
class SYSTEM_API DynamicMessageBuffer : public MessageBuffer {
private:
	DynamicMessageBuffer() {}
	DynamicMessageBuffer(int size) : MessageBuffer(new PChar[size], size) {}
	~DynamicMessageBuffer() {}
public:
private:
};
*/

#else


class SYSTEM_API SharedMessageBuff : public MessageBuffer {
public:
	SharedMessageBuff() : MessageBuffer() {}
	//SharedMessageBuff(SharedMem const& shMem, int elementSize): MessageBuffer() {}
	SharedMessageBuff(PVoid p, int totalSize, int elementSize) : MessageBuffer() {}
	/// ShMemAllocator raeumt sich selbst auf, MessageBuffer braucht nix tu tun
	~SharedMessageBuff() {}
protected:
	/// wenn moeglich neuen Puffer mit passender Groesse holen, bisherige Daten kopieren: Aufruf aus Serialisieren heraus
	virtual void resize(int size) {	}
	/// neuen Puffer mit passender Groesse holen: Aufruf bei InitMessage
	virtual void getNewBuffer(int size) {}
	/// is es eine SharedMem Buffer oder nicht
	virtual bool supportsShMemAccess() const { return true; }
	virtual void reset(int size=0) {}

public:
	/// (nach RcvPulse) Puffer zum Auslesen auf gegebenen Offset setzen
	void offsetToBuffer(int offset) {}
	/// fuer sendPulse Puffer in Offset umwandeln
	int bufferToOffset() const { return 0; }
	void copyBuff(PChar start, PChar end, PChar newBuff) {}
	/// prueft ob noch size Bytes frei sind, wenn nicht wird neue Puffer angelegt und die Daten kopiert
	virtual bool testRemainingBuffSize(std::size_t size) { return 0;}
private:
	// event-Locking-Interface: darf nichts tun, was nicht mit Locking zu tun hat, da die Message-
	// Routinen dieses Interface nicht verwenden. Die defaultImplementierung tut nichts
	/// (Write)Locks fuer setzen (geht nur wenn kein writeLock existiert)
	virtual void lockForReading() {  }
	/// ein WriteLock freigeben
	virtual void freeBuffer() {}
private:
	/// ist Puffer frei?; erst mal soll immer ein neuer Puffer verwendet werden
	virtual bool lockForWriting() {	return false; }
	/// internen Puffer neu setzen
	virtual void resetCursor(int size=0) {}

	//void getReadAccess() { /*Poco::ScopedLock<Poco::FastMutex> lock(mutex);*/ if (access_!= -1) ++access_; }
	//void getWriteAccess() { /*Poco::ScopedLock<Poco::FastMutex> lock(mutex);*/ if (access_==0) --access_; }
};


#endif // __QNX__||__linux__


} // namespace message
} // namespace system
} // namespace precitec
#endif // DERIVEDMESSAGEBUFFER_H_
