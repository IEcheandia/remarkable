#ifndef CONTAINER_H
#define CONTAINER_H

//#include <string.h>
#include <memory>
#include <iostream>
#include <cstring>

#include "memory/smartPtr.h"
#include "system/sharedMem.h"
#include "message/messageBuffer.h"
#include "message/serializer.h"


namespace precitec
{
namespace system
{
	using namespace message;

	/**
	 * TODO
	 *  - Container wirft (wg Poco::SmartPtr) NullPointerException. Es fehlt noch
	 *   		eine Strategie mit dieser Exception umzugehen
	 *  - Serialisierung fuer Smartpointer muss noch  in Verbindung mit MsgWrapper
	 *
	 */

	/**
	 * Member-Klasse fuer alle Container-Klassen (diese leiten also nicht von Container ab,
	 * sondern enthalten einen oder mehrere Container)
	 * Von container sollte regulaer nicht abgeleitet werden.
	 * Container erlaubt Initialisierung mit regulaeren, ShMem-, und Smart-Pointern
	 * Container selbst benutzt intern Smart-Pointer, kann also formell als Value-Type
	 * behandelt werden.
	 * Container erlaubt erzeugen von Container, aber kein Element-Zugriff
	 */
	template <class ValueT>
	class Container : public system::message::Serializable	{
	public:
		typedef ValueT 					*Pointer;
		typedef const ValueT		*CPointer;
		typedef typename system::TSmartPointer<ValueT>::ShArrayPtr ShArrayPtr;

	public:
		/// default CTor fuer leeres Objekt
		Container() : smartPtr_(), data_(NULL) {}
		/// Copy-CTor : shallow
		Container( Container const& rhs)
		: smartPtr_(rhs.smartPtr_), data_(smartPtr_.get()) {}
		/// Copy-CTor : shallow
		Container( Container const& rhs, int numElements)
		: smartPtr_(rhs.smartPtr_, numElements), data_(smartPtr_.get()) {}
		/// CTor aus existierendem Smartptr (egal welcher Art) und Datengroesse = Alias CTor
		// CTor aus existierendem Smartptr und Datengroesse
		Container(ShArrayPtr &dataPtr, int numElements)
		: smartPtr_(dataPtr, numElements), data_(smartPtr_.get()) {}
		// CTor aus SharedMemPtr + groesse
		Container(system::ShMemPtr<ValueT> &dataPtr, int numElements)
		: smartPtr_(dataPtr, numElements), data_(smartPtr_.get()) {}
		//Container(typename TSmartArrayPtr<ValueT>::SmartArrayPtr &dataPtr, int numElements)
		//: smartPtr_(dataPtr, numElements), data_(smartPtr_.get()) {}

		/// CTor der Speicher anlegt
		Container(int numElements)
		: smartPtr_(numElements), data_(smartPtr_.get()) {}

		/// CTor mit externem Speicher !!! Achtung gefaehrlich !!! SmartPtr wird irgendwannn delete aufrufen!!!!
		Container(Pointer data, int numElements)
		: smartPtr_(data, numElements), data_(data) {}
		/// Deserialier-CTor aus MessageBuffer
		//Container(MessageBuffer const& buffer)
		//	: smartPtr_(deMarshal<TSmartPointer<ValueT> >(buffer)), data_(smartPtr_.get()) {}
		Container(message::MessageBuffer const& buffer) {
#if !defined(NDEBUG)
			//std::cout << "container::deserialize-CTor: @" << (buffer.cursor()-buffer.rawData()) << std::endl;
#endif
			smartPtr_ = deMarshal<TSmartPointer<ValueT> >(buffer);
			data_ = smartPtr_.get();
		}
		/// DTor tut nix der SmartPtr regelt alles
		virtual ~Container() {}
	public:
		/// Exception-sichere Zuweisung (kein Deep-copy)
		Container & operator=(const Container& rhs);
		// Vergleich auf Gleichheit der Pointer
		bool operator == (Container const& rhs) const { return data_==rhs.data_; }
		/// Vergleich auf Ungleichheit
		bool operator != (Container const& rhs) const { return !( data_==rhs.data_); }
		/// Halbordnung wg. STL-Listen - ??? noetig
		bool operator < (Container const& rhs) const { return data_ < rhs.data_; }
		/// accessor Anzahl Elemente
		int numElements() const { return smartPtr_.numElements(); }
		/// accessor Groesse in Bytes
		int dataSize() const { return numElements()*baseSize(); }
		/// Groesse des Basistyps
		int baseSize() const { return sizeof(ValueT); }

		/// 'fast' deep-copy
		void fastCopy(Container const& rhs) { memcpy(data_, rhs.data_, dataSize()); }
		/// mit konstantem Wert fuellen (klappt fuer alle Typen)
		void fill(ValueT value=ValueT()) { std::uninitialized_fill_n(data(), numElements(), value); }
		/// mit konstantem Wert fuellen, optimierter Fall
		void fastFill(unsigned char value) { std::memset(data_, value, dataSize()); }
		/// Inhalt tauschen
		void swap(Container & rhs);

		///	Bild neu dimensionieren; neue Elemente mit Wert fuellen
		void resizeFill(int newSize, ValueT const& value) { resize(newSize); fill(value); }
		///	Container neu dimensionieren
		void resize(int newSize);
		/// Neu-Allokation und deep-copy bei unpassender Groesse
		void resizeCopy(TSmartPointer<ValueT> data, int dataSize);
		/// Neu-Allokation und deep-copy bei unpassender Groesse
		void resizeCopy(Container const& rhs)  { resizeCopy(rhs.data_, rhs.dataSize()); }
		friend std::ostream& operator << (std::ostream& os, Container const& t)
		{
			os << std::hex << int(t.smartPtr_.get()) << std::dec;	return os;
		}

		// data getter
		CPointer data() const { return data_; }
		Pointer data() { return data_; }
		/// chache auffrischen
		void refreshCache() { data_ = smartPtr_.get(); }
		/// Speicher vorhanden
		bool isValid() const { return data_!=NULL; }

	private:
		/// Anzahl der T-Objekte in Container
		//int										numElements_;
		/// private Ref-Counter, falls der Speicher vom Container selbst angelegt wird
		//ReferenceCounter	rc_;
		/// der verwaltete Smart-Pointer, wird nur fuer Speicherverwaltung gebraucht
		TSmartPointer<ValueT>	smartPtr_;
		/// gecachter Pointerwert, wg Performance (doppelte Indirektion mit SmartPtr + Mutexe)
		Pointer								data_;
	public:
		virtual void serialize	( message::MessageBuffer &buffer ) const {
#if !defined(NDEBUG)
			//std::cout << "container::serialize: @" << (buffer.cursor()-buffer.rawData()) << std::endl;
#endif
			marshal(buffer, smartPtr_);
		}
		virtual void deserialize( message::MessageBuffer const&buffer ) {
#if !defined(NDEBUG)
			//std::cout << "container::deserialize: @" << (buffer.cursor()-buffer.rawData()) << std::endl;
#endif
		 	Container tmp(buffer);
		 	swap(tmp);
		}
	}; // Container

	/**
	 * Bei gleicher Groesse wird seicht kopiert, sonst tief
	 */
	template <class ValueT>
	void Container<ValueT>::resizeCopy(TSmartPointer<ValueT> ptr, int nElements) {
	  if (numElements() == nElements ) {
	  	// shallow copy: nur ptr, ...  werden kopiert
	  	smartPtr_  		= ptr;
			data_ 		 		= smartPtr_.get();
			//numElements_ 	= nElements;
	  } else {
		  using std::swap;
	  	// exception-safe-copy erzeugt immer ein SmartArrayPtr
			Container<ValueT> temp(nElements);
			swap(*this, temp);
	  }
	  return;
	}

	/**
	* Bei ungleicher Groesse wird neu alloziert
	*/
	template <class ValueT>
	void Container<ValueT>::resize(int nElements) {
		if (numElements() != nElements) {
			// resize
			Container<ValueT> temp(nElements);
			this->swap(temp);
		}
	}

	/**
	 * ggf spaeter durch ein einziges Mogelswap ersetzen
	 */
	template <class ValueT>
	inline void Container<ValueT>::swap(Container<ValueT> & rhs)	{
		using std::swap;
		smartPtr_.swap(rhs.smartPtr_);
		swap(data_, rhs.data_); // alt: beide caches refreshen, ist sicher langsamer
	}

} // system
} // precitec


#endif // CONTAINER_H

