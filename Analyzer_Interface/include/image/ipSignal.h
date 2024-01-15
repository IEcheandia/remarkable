#ifndef IP_SIGNAL_H_
#define IP_SIGNAL_H_

#include "system/types.h"

#include "geo/range.h"

#include "memory/container.h"
#include "memory/smartPtr.h"

#include "message/messageBuffer.h"
#include "message/serializer.h"

#include <cassert>

namespace precitec
{
namespace image
{
	/**
	 * Datenhaelter mit 1D-Iteratoren
	 * Die Daten liegen im Container, Singal liefert als neues Element den Datenzugriff.
	 */
	template <class ValueT>
	class TSignal : public system::message::Serializable
	{
	public:
		typedef typename system::TSmartPointer<ValueT>::ShArrayPtr ShArrayPtr;

		// nun die Konstruktoren
		/// TSignal der Groesse 0, ohne Speicher
		TSignal() : start_(NULL), length_(0) {}

		/// copy-Construktor shallow
		TSignal(TSignal const& rhs)
		: container_(rhs.container_), start_(container_.data()), length_(rhs.numElements()) {}
		
		/// Range-sample-Construktor (uses shallow Copy-CTor of system::Container)
		TSignal(TSignal const& rhs, geo2d::Range const& range)
		: container_(rhs.container_, range.end()), start_(container_.data() + range.start()), length_(range.length()) 
		{
			assert(container_.numElements() >= length_);
		}

		/// CTor aus nacktem Ptr (egal welcher Art) und Datengroesse
		TSignal(ValueT* dataPtr, int numElements)
		: container_(dataPtr, numElements), start_(container_.data()), length_(numElements) {}

		/// CTor aus existierendem Smartptr (egal welcher Art) und Datengroesse
		TSignal(ShArrayPtr &dataPtr, int numElements)
		: container_(dataPtr, numElements), start_(container_.data()), length_(numElements) {}

		/// CTor aus existierendem ShMemPtr(egal welcher Art) und Datengroesse
		TSignal(system::ShMemPtr<ValueT> &dataPtr, int numElements)
		: container_(dataPtr, numElements), start_(container_.data()), length_(numElements) {}

		/// CTor der Speicher anlegt
		TSignal(int numElements, ValueT const& value = ValueT{})
		: container_(numElements), start_(container_.data()), length_(numElements) { fill(value); }

		/// Deserialisierungs-CTor
		TSignal(system::message::MessageBuffer const&buffer);

		// virtual ist noetig, da Image abgeleitet wird
		virtual ~TSignal() {}

		TSignal& fill(ValueT const& value) { container_.fill(value); return *this; }

		/// data getter
		ValueT const * data() const {	return start_;	}
		ValueT* data() { assert(start_ != nullptr); return start_; }

		/// size getter
		std::size_t getSize() const { return length_; }
		/// Bildzugriff value = image[ 234 ]; Zugriff wird ueber Container erledigt
		inline ValueT &operator[](int offset) { return start_[offset];	}
		/// const Bildzugriff value = image[ offset ];
		inline ValueT operator[](int offset) const { assert(start_ != nullptr); return start_[offset]; }
		/// Bildzugriff value = image(1, 3);
		/// Bildzugriff value = image[ 234 ];
		ValueT &operator()(int offset) { return start_[offset]; }
		/// const Bildzugriff value = image[ offset ];
		ValueT operator()(int offset) const { return start_[offset]; }

		/// ist Speicher vorhanden
		bool isValid() const { return container_.isValid(); }
		
		/// Bild kopieren ggf. nach resize; Pendant zu Copy-CTor
		TSignal & operator = (TSignal const& rhs) { TSignal tmp(rhs); swap(tmp); return *this; }
		///	Bild neu dimensionieren; neue Elemente mit Wert fuellen
		void resizeFill(int newSize, ValueT const& value)	{ 
			if (newSize != container_.numElements()) { 
				container_.resizeFill(newSize, value); 
				start_ = container_.data();
				length_ = newSize;
			}
			else {
				container_.fill(value);
			}
			assert(length_ == newSize);
			assert(container_.numElements() == newSize);assert(container_.numElements() == newSize);
			assert( newSize == 0 || ( *start_ == value && *(start_ + newSize -1) == value));
		}
		///	Bild neu dimensionieren
		void resize(int newSize)	{
			if (newSize != container_.numElements()) {
				container_.resize(newSize);
				start_ = container_.data();
				length_ = newSize;
			}
			assert(length_ == newSize);
			assert(container_.numElements() == newSize);
		}
		/// interne Daten tauschen
		void swap(TSignal & rhs );
		friend std::ostream& operator << ( std::ostream& os, const TSignal& s )
		{
			os << "Sig(" << s.length_ << ") @" << std::hex << int(s.start_) << " : " << s.container_;	return os;
		}

		/// die Containergroesse
		int numElements() const { return length_;/*container_.numElements();*/ }
		/// in einen Buffer serialisieren
		void serialize( system::message::MessageBuffer &buffer ) const;
		/// aus einem Buffer deserialisieren; diese Loesung wg Exceptionsicherheit
		void deserialize( system::message::MessageBuffer const&buffer ) {	TSignal tmp(buffer); swap(tmp); }


	private:
		system::Container<ValueT> container_;
		/// relevant fuer Sub-Ranges, sonst == container_.data_
		ValueT*				start_;
		/// relevant fuer Sub-Ranges, sonst == container_.numElements_
		int					length_;
	}; // TSignal

	template<class ValueT>
	void TSignal<ValueT>::swap(TSignal & rhs ) {
		container_.swap(rhs.container_);
		std::swap(start_, rhs.start_);
		std::swap(length_, rhs.length_);
	}

	/**
	 * da beim Serialisieren ggf komprimiert wurde, fangend die Daten immer bei Position 0 an
	 */
	template<class ValueT>
	TSignal<ValueT>::TSignal(system::message::MessageBuffer const&buffer)
		: container_(buffer), start_(container_.data()), length_(Serializable::deMarshal<int>(buffer))
	{
	}

	/**
	 * beim Serialisieren wird komprimiert: unnoetiger Speicher wird nicht serialisiert
	 */
	template<class ValueT>
	void TSignal<ValueT>::serialize( system::message::MessageBuffer &buffer ) const
	{
		container_.serialize(buffer);
		marshal(buffer, length_);
	}

	typedef TSignal<int> 		Sample;
	typedef TSignal<double> Reading;


} // image
} // precitec


#endif /* IP_SIGNAL_H_*/
