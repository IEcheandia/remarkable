#ifndef SMART_PTR_H_
#define SMART_PTR_H_

//#include "Poco/ReferenceCounter.h"
//#include "Poco/ReleasePolicy.h"
#include "Poco/SharedPtr.h"
#include "Poco/SharedMemory.h"

#include "system/policies.h"
#include "system/sharedMem.h" // wg ShMemPtr

namespace precitec
{
namespace system
{

	/// Pimpl-Implementierung wird vorwaerts deklareirt
	template <class T> class TSmartPtrBase;




	/**
	 * Pimpl-Implementierungs-Basisklasse (TSmartPtr ist der Pimpl-Kopf)
	 * Diese Klasse findet im Wesentlichen im Container Anwendung. Hiermit
	 * kann ich verschiedenste Shared-Pointer (mit unterschiedlichem Speichermanagement)
	 * in einer Klasse halten. Das Problem ist, dass Poco seine Shared-Ptr
	 * ohne Klassenhierarchie ueber Templates definiert. Also Pointer mit verschiedenem
	 * Speichermanagement voellig inkompatibel zueinander sind.
	 * Es gibt folgende Pointer-Typen:
	 *  Poco::Shared-Ptr auf Arrays - DTor mit delete []
	 *  SharedMemory-Ptr - DTor tut nichts, Ptr serialisiert anders als normale Ptr
	 *  Raw-Ptr - Dtor tut nichts, User muss selbst Laufzeit verwalten, etwa fuer Stack(auto)-Ptr
	 *  Poco::Shared-Ptr auf EinzelObjekte - nicht unterstuetzt, da Unterbringung in ShMem nicht sinnvoll
	 *
	 * Die ueber die Pimple-Implementierung verurachte Ineffizient (doppelte Indirektion,
	 * Mutexe) wird durch Cachen des C-Pointers (im Container) erreicht. Der SmartPtr wird
	 * nur fuer Verwaltungsarbeiten (Erzeugen/Vernichten) verwendet. Das entspricht der Vorstellung,
	 * dass Pointer-Zugriffe (via Cache) viel haeufiger sind, als Verwaltungsoperatrionen.
	 * Der Pimpl-Kopf hat CToren fuer alle abgeleiteten Klassen von TSmartPtrBase sowie deren
	 * interne SharedPtr also Poco::SharedPtr die SharedMemPtr-Klasse
	 * sowie Raw-Pointer.
	 */
	template <class T>
	class TSmartPtrBase	{
	public:
		enum { ArrayType, UnManagedType, SharedMemType, NumTypes };
		/// Herausgabe des Roh-Zeigers
		virtual T * get() = 0;
		/// Herausgabe des Roh-Zeigers
		virtual const T * get() const  = 0;
		/// NullPointer-Abfrage
		virtual bool isNull() const = 0;
		/// NullPointer setzen, etwa um deallokation zu erzwingen
		virtual void setNull() = 0;
		/// swap ist nicht abstrakt, weil es wohl!!??!! bei der Zusweisung/copy-CTor der abgeleiteten Klassen aufgerufen wird
		virtual void swap(TSmartPtrBase &) {}
		virtual ~TSmartPtrBase() {}
		static Poco::SharedPtr<TSmartPtrBase<T> > create(message::MessageBuffer const&buffer, int length);
		virtual void serialize	( MessageBuffer &buffer, int numElements) const	{}
		/// Zuweisung eines anderen SmartPtrs
		//virtual TSmartPtrBase &operator =(TSmartPtrBase &rhs)  = NULL;
	}; // TSmartPtrBase

	/**
	 * SmartPtr-Implementierung fuer Pointer auf Arrays
	 * ueber die ArrayReleasePolicy wird fuer sauberes Aufraeumen gesorgt
	 */
	template <class T>
	class TSmartArrayPtr : public TSmartPtrBase<T> {
	public:
		typedef Poco::SharedPtr<T, Poco::ReferenceCounter, ArrayReleasePolicy<T> > ShArrayPtr;

		/// NuLL-Ptr-Allokater wofuer auch immer
		TSmartArrayPtr() {}
		/// Std-Allokator mit Elementzahl: Speicher wird angelegt
		TSmartArrayPtr(int numElements) : smartPtr_(new T[numElements]) {}

		//TSmartArrayPtr(TSmartArrayPtr const &rhs) : smartPtr_(rhs.smartPtr_) {}

		TSmartArrayPtr(ShArrayPtr &sp) : smartPtr_(sp) {}
		/// wg SharedPtr braucht der DTor nichts zu tun
		virtual ~TSmartArrayPtr() {}
	protected:
		/// Allokator mit Roh-Pointer nur fuers deserialisieren
		TSmartArrayPtr(T* p) : smartPtr_(p) {}
	public:
		/// Herausgabe des Roh-Zeigers
		T * get() { return smartPtr_.get(); }
		/// Herausgabe des Roh-Zeigers
		const T * get() const { return smartPtr_.get(); }
		/// NullPointer-Abfrage
		virtual bool isNull() const { return smartPtr_.isNull(); }
		/// NullPointer setzen, etwa um deallokation zu erzwingen
		virtual void setNull() { smartPtr_= NULL; }
		/// Zuweisung eines anderen SmartPtrs
		virtual TSmartArrayPtr &operator =(TSmartArrayPtr const &rhs) { smartPtr_ = rhs.smartPtr_; return *this; }
		/// swap wg Zuweisung abgeleiteter Klassen
		virtual void swap(TSmartArrayPtr &p) { smartPtr_.swap(p.smartPtr_); }
		static TSmartPtrBase<T>* create(message::MessageBuffer const& buffer, int length) {
			// SmartArrayPtr verwendet den Prt im CTor, also p darf P keine auto-Variable sein (muss Heap-Var sein)
			//int length;
			//deMarshalBuffer(length, buffer);
			T *p(new T[length]);
			deMarshalBuffer(*p, length*sizeof(T), buffer);
			return new TSmartArrayPtr<T>(p);
		}
		virtual void serialize	( MessageBuffer &buffer, int length ) const	{
			int type(TSmartPtrBase<T>::ArrayType);
			marshalBuffer(type, buffer);
			const T * p(smartPtr_.get());
			marshalBuffer(*p, length*sizeof(T), buffer);
		}
	private:
		/// im Poco::SharedPtr steckt die restliche Intelligenz
		ShArrayPtr smartPtr_;
	};

	/**
	 * SmartPtr-Implemetierung fuer Pointer auf Share Memory
	 * Der Speicher wird nie deallokiert, auck kein Destruktor aufgerufen
	 * Die Objekte leben ewig, da eine Synchronisation ueber Prozessgrenzen
	 * via SharedMem (impliziert neue RefCounter-Klasse, die auf SharedMem lebt)
	 * zwar moeglich, aber im Moment nicht vorgesehen ist.
	 * Der eigentliche SharedMemory-Ptr system::ShMemPtr hat einen Index
	 * auf das verwendete Shared Memory und den Offset davon. Die Indices
	 * werden von einer Singleton-Klasse verwaltet (in einer Tabelle gepeichert).
	 */
	template <class T>
	class TSmartSharedPtr : public TSmartPtrBase<T>	{
	public:

		/// NuLL-Ptr-Allokater wofuer auch immer
		TSmartSharedPtr() {}
		/// Std-Allokator mit SharedMem-Pointer
		TSmartSharedPtr(ShMemPtr<T> &p) : smartPtr_(p) {}
		/// wg SharedPtr braucht der DTor nichts zu tun
		virtual ~TSmartSharedPtr() {}
		static TSmartPtrBase<T>* create(message::MessageBuffer const&buffer, int ) {
			// SmartSharedPtr verwendet keine Ptr, also darf p auto-Variable sein
			ShMemPtr<T> p;
			deMarshalBuffer(p, buffer);
			return new TSmartSharedPtr<T>(p);
		}
		virtual void serialize( MessageBuffer &buffer, int length) const	{
			// falls Puffer ShMem-Verwendung unterstuetzt, als solches serialisieren, sonst, tun als waere
			// es ein normaler SmartPtr gewesen
			if (buffer.supportsShMemAccess()) {
				// nur der ShMemPtr wird in den Puffer serialisiert
				int type(TSmartPtrBase<T>::SharedMemType);
				marshalBuffer(type, buffer);
				marshalBuffer(smartPtr_, buffer);
			} else {
				// der Pointer wird auf Array-Pointer gesetzt, damit beim Empfaenger ein solcher erzeugt wird
				// der Inhalt des ShMemPtr wird serialisiert
				int type(TSmartPtrBase<T>::ArrayType);
				marshalBuffer(type, buffer);
				marshalBuffer(*(smartPtr_.get()), length*sizeof(T), buffer);
			}
		}
	public:
		/// NullPointer-Abfrage
		virtual bool isNull() const { return smartPtr_.isNull(); }
		/// NullPointer setzen (etwa fuer Debug, um DTor zu erzwingen)
		virtual void setNull() { smartPtr_.setNull(); }
		/// Zuweisung eines anderen SmartPtrs
		virtual TSmartSharedPtr &operator =(TSmartSharedPtr const &rhs) { smartPtr_ = rhs.smartPtr_; return *this; }
		/// swap wg Zuweisung abgeleiteter Klassen
		virtual void swap(TSmartSharedPtr &rhs) {	smartPtr_.swap(rhs.smartPtr_); }

	private:
		/// Herausgabe des Roh-Zeigers
		T * get() { return smartPtr_.isNull() ? NULL : smartPtr_.get(); }
		/// Herausgabe des Roh-Zeigers
		T const* get() const { return smartPtr_.isNull() ? NULL : smartPtr_.get(); }
	private:
		/// im Poco::SharedPtr steckt die restliche Intelligenz
		ShMemPtr<T> smartPtr_;
	};

	/**
	 * Poco::SharedPtr ohne Deallokation also ein Dumb-Pointer.
	 * Diese Klasse existiert fuer Test und damit Container/Bilder
	 * mit normalen Poitern initialisiert werden koennen.
	 * Der Speicher wird nie deallokiert, wohl aber ein Destruktor aufgerufen
	 */
	template <class T>
	class TSmartUnmanagedPtr : public TSmartPtrBase<T>	{
		typedef Poco::SharedPtr<T, Poco::ReferenceCounter, NoReleasePolicy<T> > SharedUnmanagedPtr;
	public:

		/// NuLL-Ptr-Allokater wofuer auch immer
		TSmartUnmanagedPtr() {}
		/// Std-Allokator mit User-Mem
		TSmartUnmanagedPtr(T *p) : smartPtr_(p) {}
		/// copy cTor
		TSmartUnmanagedPtr(TSmartUnmanagedPtr &p) : smartPtr_(p.smartPtr_) {}
		/// Std-Allokator mit SharedMem-Pointer
		TSmartUnmanagedPtr(SharedUnmanagedPtr &p) : smartPtr_(p) {}
		/// wg SharedPtr braucht der DTor nichts zu tun
		virtual ~TSmartUnmanagedPtr() {}
	private:
		// diesen Ptr-Typ remote zu verwenden scheint all zu frivol, also sind die entsprechendne
		// Funktoinen gesperrt
		static TSmartUnmanagedPtr<T>* create(message::MessageBuffer const&, int) {
			return NULL;
		}
		virtual void serialize	( MessageBuffer &buffer, int length ) const	{

		}

	public:
		/// NullPointer-Abfrage
		virtual bool isNull() const { return smartPtr_.isNull(); }
		/// NullPointer setzen (etwa fuer Debug, um DTor zu erzwingen)
		virtual void setNull() { smartPtr_= NULL; }
		/// Zuweisung eines anderen SmartPtrs
		virtual TSmartUnmanagedPtr &operator =(TSmartUnmanagedPtr const &rhs) { smartPtr_ = rhs.smartPtr_; return *this; }
		/// swap wg Zuweisung abgeleiteter Klassen
		virtual void swap(TSmartUnmanagedPtr &rhs) {
			if (smartPtr_.isNull()) {
				if (rhs.smartPtr_.isNull()) {}
				else {rhs.smartPtr_.swap(smartPtr_);}
			} else {
				smartPtr_.swap(rhs.smartPtr_);
			}
		}
	private:
		/// Herausgabe des Roh-Zeigers
		T * get() { return smartPtr_.isNull() ? NULL : smartPtr_.get(); }
		/// Herausgabe des Roh-Zeigers
		T const* get() const { return smartPtr_.isNull() ? NULL : smartPtr_.get(); }
	private:
		/// im Poco::SharedPtr steckt die restliche Intelligenz
		SharedUnmanagedPtr smartPtr_;
	};

	//**
	// * Pimpl-Kopf fuer die SmartPtr-Klassen
	// * Ueber diese Indirektion kann etwa die Container-Klasse
	// * ueber eeineni Pointer ungewisser Groesse initialisiert werden.
	// * Eine auf Poco::SharedPtr aufbauende SmartPtr-Klasse die sowohl auf
	// * Normale Array-Ptr als auch auf Shared-Memory-Ptr zeigen kann
	// */
	template <class T>
	struct TSmartPointer : public message::Serializable {
	public:
		// dieser Poco::Pointertyp wird exportiert
		typedef Poco::SharedPtr<T, Poco::ReferenceCounter, ArrayReleasePolicy<T> > ShArrayPtr;

	public:
		/// Null-Ptr CTor wofuer auch immer
		TSmartPointer() : numElements_(0), smartPtr_(new TSmartArrayPtr<T>()) {}
		/// legt Speicher an (als SmartArray-Ptr auf Heap)
		TSmartPointer(int numElements) : numElements_(numElements), smartPtr_(new TSmartArrayPtr<T>(numElements)) {}
		/// copy CTor
		TSmartPointer(TSmartPointer const& p, int l) : numElements_(l), smartPtr_(p.smartPtr_) {}
		/// Erzeugen aus den SmartPtr-Implementierungen
		TSmartPointer(ShArrayPtr &p, int l) : numElements_(l), smartPtr_(new TSmartArrayPtr<T>(p)) {}
		/// Normalo-Ptr: hier wird ein Unmanaged-SmartPtr erzeugt
		TSmartPointer(T* p, int l) : numElements_(l), smartPtr_(new TSmartUnmanagedPtr<T>(p)) {}
		/// SharedMem-Ptr
		TSmartPointer(ShMemPtr<T> &p, int l) : numElements_(l), smartPtr_(new TSmartSharedPtr<T>(p)) {}		/// hier wird ein Shared-Memory-SmartPtr erzeugt

		TSmartPointer(message::MessageBuffer const&buffer)
		: numElements_(Serializable::deMarshal<int>(buffer)), smartPtr_(TSmartPtrBase<T>::create(buffer, numElements_)) {}

		virtual void serialize	( MessageBuffer &buffer) const	{
			//std::cout << "Container serialize: numElements_: " << numElements_ << " value: " << data_[numElements_-1] << std::endl;
			TSmartPointer::marshal(buffer, numElements_);
			smartPtr_->serialize(buffer, numElements());
		}
		virtual void deserialize( MessageBuffer const&buffer ) {
			TSmartPointer tmp(buffer); swap(tmp);
		}
		int numElements() const { return numElements_; }
		/// NullPointer setzen
		virtual bool isNull() const { return smartPtr_.isNull(); }
		/// NullPointer setzen
		virtual void setNull() { smartPtr_= NULL; }
		/// DTor Angelegenheiten werden an die Implementierung delegiert
		virtual ~TSmartPointer() {}
	public:
		/// swap
		//void swap(TSmartPointer &p) {	std::swap(p.smartPtr_, smartPtr_); }
		void swap(TSmartPointer &p) {
			using std::swap;
			swap(numElements_, p.numElements_);
			if (smartPtr_.isNull()) {
				if (p.smartPtr_.isNull()) {}
				else {p.smartPtr_.swap(smartPtr_);}
			} else {
				smartPtr_.swap(p.smartPtr_);
			}
		}

		/// Herausgabe des Roh-Zeigers
		T * get() { return isNull() ? NULL : smartPtr_->get(); }
		/// Herausgabe des Roh-Zeigers
		const T * get() const { return smartPtr_.isNull() ? NULL : smartPtr_->get(); }
		/// Zuweisung eines anderen SmartPtrs
		//TSmartPointer &operator =(TSmartPointer const &rhs) { smartPtr_ = rhs.smartPtr_; }
	private:
		int numElements_;
		Poco::SharedPtr<TSmartPtrBase<T> > smartPtr_;
		//TSmartPtrBase<T> *smartPtr_;
	};




	template <class T>
	Poco::SharedPtr<TSmartPtrBase<T> > TSmartPtrBase<T>::create(message::MessageBuffer const&buffer, int length) {
		int type;
		deMarshalBuffer(type, buffer);
		typedef TSmartPtrBase* (*FactoryFun)(message::MessageBuffer const& buffer, int length);
		FactoryFun factoryTable[NumTypes] = {
				TSmartArrayPtr<T>::create,
				TSmartArrayPtr<T>::create,		// unmanaged Ptr werden als Smart-Ptr deserialisiert
				TSmartSharedPtr<T>::create
		};
		return Poco::SharedPtr<TSmartPtrBase<T> >(factoryTable[type](buffer, length));
		//std::cout << "Container CTor(buffer): numElements_: " << numElements_ << " data: " << std::hex
		//			<< data_  << std::dec << " value: " << data_[numElements_-1] << std::endl;
	}

} // system
} // precitec


#endif /*SMART_PTR_H_*/

