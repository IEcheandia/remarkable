#ifndef SHAREDMEM_H_
#define SHAREDMEM_H_
#pragma once

#include <vector>
#include "system/types.h"
#include "message/serializer.h"
#include "message/messageBuffer.h"

//#ifndef __QNX__
//#include "Poco/SharedMemory.h"
//#endif
/**
 * Analyzer_Interface::sharedMem.h
 *
 *  Created on: 01.06.2010
 *      Author: Wolfgang Reichl
 *   Copyright: Precitec Vision KG
 */

/// Analyzer_Interface::precitec::system::SharedMem
namespace precitec
{

namespace system
{

using message::Serializable;
using message::MessageBuffer;

#if defined __QNX__ || defined __linux__

	/**
	 * SharedMemControl enthaelt einige statische Verwaltungsinformation fuer
	 * alle SharedMem-Blocke
	 * Erst einmal gegeh wir davon aus dass die SharedMemories ueber gemeinsame
	 * Header 'von Hand' verwaltet werden. Die automatische Verwaltung (so sie je gewuenscht wird)
	 * korreliert die Tabellen verschiedener Prozesse selbst ueber ein Shared-Memory)
	 */
	class SharedMem;
	class SharedMemControl {
	public:
		struct ShMemEntry {
			ShMemEntry() : handle(NoHandle), memory(NULL) {}
			//ShMemEntry(int i, int s) : handle(i), size(s), memory(NULL) {}
			/// der erst mal Haupt-CTor: Index handverwaltet, Groesse vorgegeben
			ShMemEntry(int i, int s, SharedMem &mem, const std::string &n) : handle(i), size(s),	memory(&mem), name(n) {}
			ShMemEntry(ShMemEntry const&rhs) { handle=rhs.handle; memory=rhs.memory; name=rhs.name; }
			void operator =(ShMemEntry const&rhs) { handle=rhs.handle; memory=rhs.memory; name=rhs.name; }
			/// globaler Wert, 'hash' fuer Zugriff
			int 			 handle;
			/// ist wesentlich fuer Konsistenz
			int 			 size; // remote aus fd erhaeltlich
			/// lokales ShMem-Objekt
			SharedMem *memory;
            std::string name;
		};
		typedef std::vector<ShMemEntry> MemoryList;
	public:
		enum { NoHandle = -1 };

	private:
		/// der einzige CTor
		SharedMemControl();
	public:
		~SharedMemControl() {}

		static SharedMemControl& instance();

		/// handverwaltetes (handle-Vorgabe) ShMem (ohne Namen e.g. PhysMem)
		//int registerMemory(int handle, int size, PhysMem &mem);
		/// handverwaltetes (handle-Vorgabe) ShMem mit Namen
		int registerMemory(int handle, PvString name, SharedMem &mem, int size);
		/// Controllerverwaltetes (handle wird erzeugt) ShMem (ohne Namen e.g. PhysMem)
		//int  registerMemory(PhysMem &mem);
		/// der haeufigste Zugriff ist von Handle auf SharedMem
		SharedMem& operator [] (int handle) const;
		/// handle zu name
		PvString name (int handle) const;
		/// name zu handle
		int findHandle (PvString name) const;
	private:
		/// am SharedMemory sollst Du es erkennen
		bool isUsed(int handle) const;
	private:
		MemoryList memoryList_;
	};

	/**
	 * SharedMem ist eine Wrapperklasse um einen SharedMemory-Block
	 * Das Ziel ist es einfahc mehrere benannte Shared-Memories in
	 * verschiedenen Prozessen zu haben, deren Initialisierung,
	 * Zerstoerung automatisch geregelt ist.
	 * Die Umrechnung von Pointern wird so geloest, dass in einem lokalen Pointer,
	 * ausserhalb des eigentlichen Shared-Memories ein Pointer auf den ShMem-Anfang hinterlegt wird.
	 * Ein Smart-Shared-Memory-Pointer-Typ haelt eine Referenz auf das Shared Memory und
	 * die Differenz zu dem Inhalt dieses Anfang-Pointers.
	 * Das Shared Memory serialiseirt sich auf einen globalen Index, der vom
	 * ModulManager verwaltet wird.
	 */

	template <class T>	class ShMemPtr;
	class SharedMem {
	public:
		enum ShMemMode {
			FailNew   = 0,	 // Mem muss vorhanden sein
			CreateNew = 1,   // wenn nicht da, dann wird es erzeugt
			RecreateOld = 2, // Server loescht vorhandenes Mem erst (i.a. problematisch)
			KillOnClose = 4, // 'server'-Version Besitzer raeumt auf
			WriteAccess = 8,  // sonst ReadOnly Zugriff
			Locked = 16, // MAP_LOCKED
			StdClient = CreateNew | WriteAccess,
            StdLockedClient = CreateNew | WriteAccess | Locked,
			StdServer = CreateNew | WriteAccess | KillOnClose
		};
	public:
		/// leeres ShMem, etwa fuer Arrays
		SharedMem();
		/// unverwaltetes ShMem; kann nicht mit ShMemPtr verwendet werden!!
		SharedMem(PvString const& fileName, ShMemMode mode, int size);
		/// selbstverwaltetes ShMem, server==true: ShMem wird erzeugt/vernichtet
		SharedMem(int handle, PvString const& fileName, ShMemMode mode, int size);
		/// selbstverwaltetes ShMem, server==true: ShMem wird erzeugt/vernichtet
		SharedMem(int handle, PvString const& fileName);
		/// \todo ueber das virtual muss man noch reden
		virtual ~SharedMem();
		//void operator = (SharedMem const& mem);
		/// fd_ ist die "Handle" des Shmem
		//operator int() { return fd_; }
	public:
		static bool failNew(ShMemMode mode) { return mode & FailNew; }
		static bool createNew(ShMemMode mode) { return mode & (CreateNew |	RecreateOld); }
		static bool writeAccess(ShMemMode mode) { return mode & WriteAccess; }
		static bool recreateOld(ShMemMode mode) { return mode & RecreateOld; }
	public:
		/// verzoegertes Anlegen des ShMem, size ist z.Zt. Pflicht // optional, ggf aus fd
		void set(ShMemMode mode, int size);
		/// unverwaltetes ShMem; kann nicht mit ShMemPtr verwendet werden!!
		void set(PvString const& fileName, ShMemMode mode, int size);
		/// verzoegertes Anlegen des ShMem, size ist z.Zt. Pflicht // optional, ggf aus fd
		void set(int handle, PvString const& fileName, ShMemMode mode, int size);
		/// Zugriff auf den Basis-Pointer
		PVoid begin()  	const { return basePtr_; }
		/// der Name
		PvString const& name() const { return name_; }
		/// die Groesse in Bytes
		int size() const { return size_; }
		/// nullPtr-Pruefung
		bool isValid() const { return basePtr_!=NULL; }
		/// aus einem normalo-Ptr wird ein ShMemPtr, oder Funktion wirft
		template <class T>
		ShMemPtr<T> toShMemPtr(T *p) { return ShMemPtr<T>(handle, p-begin()); }

		friend  std::ostream &operator <<(std::ostream &os, SharedMem const& m) {
			os << "ShMem: " << m.name_
					<< " #" << m.fd_
					<< " @" << std::hex << long(m.basePtr_) << std::dec
					<< "(" << m.size_ << ")";
			return os;
		}
	public:
		// ShMemController-Schnittstelle
		/// wird bei erfolgreicher Registrierung gesetzt
		//void 	setHandle(int h) { handle_ = h; }
		///	debug-Schnittstelle + intern
		int 	handle() const { return handle_; }
	private:
		// wird von CTor'en aufgerufen um basePtr_ zu setzen
		void handleError(PvString const& baseText, int errNo) const;
		/// Filedescriptor holen
		int open(PvString const& fileName, ShMemMode mode);
		/// basePtr mappen
		PVoid map(int size, ShMemMode mode);
	private:
		/// der ShMem-Name
		PvString  name_;
		/// der Filedescriptor des ShMem = Handle
		int fd_;
		/// von Hand oder von MM gesetzt
		int handle_;
		/// zeigt auf das erste mit mmap angelegte byte
		PVoid basePtr_;
		/// Groesse in Bytes
		int size_;
	};


	/**
	 * ShMemPtr
	 */
	template <class T>
	class ShMemPtr {
	public:
		/// fuer Arrays
		ShMemPtr() : memory_(SharedMemControl::NoHandle), offset_(0) {}
		/// copy-CTor
		ShMemPtr(ShMemPtr const& rhs) : memory_(rhs.memory_), offset_(rhs.offset_) {}
		/// zeigt auf Basis-Pointer
		ShMemPtr(SharedMem const& mem) : memory_(mem.handle()), offset_(0) {}
		/// berechnet aus realem Pointer den Offset
		ShMemPtr(SharedMem const& mem, T *p) : memory_(mem.handle()), offset_(PByte(p)-PByte(shMem().begin())) {}
		/// auf Mehrfachzuweisung wird verzichtet (kein Rueckgabewert)
		void operator = (ShMemPtr const& rhs)  {	memory_=rhs.memory_; offset_=rhs.offset_;	}
	public:
		void set(SharedMem const& mem, int o) { memory_ = mem.handle(); offset_ = o; }
		void set(int index, int o) { memory_ = index; offset_ = o; }
		void set(T *p) { offset_ = (PByte(p)-PByte(shMem().begin())); }
		void set(SharedMem const& mem, T *p) { memory_ = mem.handle(); offset_ = (PByte(p)-PByte(shMem().begin())); }
		// !=0 und in SharedMem.size
		bool isValid() const { return !isNull() && (offset_ < shMem().size); }
		// das Standard-Zeug, was ein Pointer so koennen sollte
		T* operator -> ()	{	return checkedDeref();	}
		const T* operator -> () const	{ return checkedDeref();	}
		T& operator * () { return *checkedDeref();	}
		T& operator () (int i) { return *(i+unCheckedDeref());	}
		const T& operator * () const { return *checkedDeref();	}
		T* get() { return deref(); }
		const T* get() const { return deref(); }
		//operator T* () { return deref(); }
		//operator const T* () const { return deref(); }
		bool operator ! () const { return isNull();	}
		bool isNull() const {	return (memory_>=0) && (!shMem().isValid()); }
		void setNull() { memory_ = -1; }
		/// ???? sollte hier auf PhysPtr verglichen werden ????
		bool operator == (const ShMemPtr<T>& ptr) const	{	return get() == ptr.get(); }
		bool operator != (const ShMemPtr<T>& ptr) const	{	return get() != ptr.get(); }
		bool operator <  (const ShMemPtr<T>& ptr) const	{	return get() <  ptr.get();}
		bool operator <= (const ShMemPtr<T>& ptr) const	{	return get() <= ptr.get(); }
		bool operator >  (const ShMemPtr<T>& ptr) const	{	return get() >  ptr.get();}
		bool operator >= (const ShMemPtr<T>& ptr) const	{	return get() >= ptr.get(); }
		void swap(ShMemPtr<T> &p) { std::swap(memory_, p.memory_); std::swap(offset_, p.offset_);}

		int offset() const
		{
			return offset_;
		}
	public:
		void init(T *t) { offset_ =  PChar(t) - shMem().basePtr_; }
		friend  std::ostream &operator <<(std::ostream &os, ShMemPtr const& p) {
			os << "[[=" << p.shMem().name() << ": " << p.offset_ << "]]"; return os;
		}
		//void serialize(MessageBuffer  &buffer) { serialize(memory_); serialize(offset_); }
	private:
		/// kann Null-Pointer zurueckgeben
		T* deref() const { return isNull() ? NULL : unCheckedDeref(); }
		/// wirft bei Null-Ptr, gibt immer gueltigen Ptr zurueck
		T* checkedDeref() const { if (isNull()) throw /*NullPointerException()*/; return unCheckedDeref(); }
		/// erledigt nur die Pointer-Arithmetik
		T* unCheckedDeref() const { return (T*)(PByte(shMem().begin()) + offset_);}
		SharedMem &shMem() const {return SharedMemControl::instance()[memory_]; }
	private:
		int memory_;
		int offset_;
	};

	template <class T>
	inline void swap(ShMemPtr<T>& p1, ShMemPtr<T>& p2) { p1.swap(p2); }

	/**
	 * ShMemMiniPtr ist ein ShMemPtr ohne BasisPtr also einfacher Integer.
	 * Der ShMemMiniPtr ist als Zeiger-Typ innerhalb eine Listen-Struktur auf einem ShMem sinnvoll.
	 * ShMemMiniPtr uebernimmt die unuebersichtlichen Typkonversionen.
	 */
	template <class T>
	class ShMemMiniPtr {
	public:
		typedef T *PT;
		/// fuer Arrays
		ShMemMiniPtr() : offset_(0) {}
		/// aus ShMem-Basis-Pointer und realem Pointer
		ShMemMiniPtr(PChar base, T *p) : offset_(reinterpret_cast<PChar>(p)-base) {}
		/// copy-CTor
		ShMemMiniPtr(ShMemMiniPtr const& rhs) : offset_(rhs.offset_) {}
		/// auf Mehrfachzuweisung wird verzichtet (kein Rueckgabewert)
		void operator = (ShMemMiniPtr const& rhs)  { offset_=rhs.offset_;	}
	public:
		/// wir nehemn an, T sei eine Listenstruktur mit den int-Elementen next_, last_
		T* next(PChar base) { return get(get(base)->next_); }
		/// wir nehemn an, T sei eine Listenstruktur mit den int-Elementen next_, last_
		T* last(PChar base) { return get(get(base)->last_); }
		/// wir nehemn an, T sei eine Listenstruktur mit den int-Elementen next_, last_
		ShMemMiniPtr nextPtr(PChar base) { return get(base)->next_; }
		/// wir nehemn an, T sei eine Listenstruktur mit den int-Elementen next_, last_
		ShMemMiniPtr lastPtr(PChar base) { return get(base)->last_; }
	public:
		void set(PChar base, T *p) { offset_ = reinterpret_cast<PChar>(p)-base; }
		void set(int o) { offset_ = o; }
		// das Standard-Zeug, was ein Pointer so koennen sollte
		T& operator () (PChar base)	{	return *deref(base); }
		const T& operator () (PChar base) const	{ return *deref(base); }
		T* get(PChar base) { return deref(base); }
		const T* get(PChar base) const { return deref(base); }
		bool isNull() const {	return offset_!=0; }
		/// ???? sollte hier auf PhysPtr verglichen werden ????
		bool operator == (const ShMemMiniPtr<T>& ptr) const	{	return offset_ == ptr.offset_; }
		bool operator != (const ShMemMiniPtr<T>& ptr) const	{	return get() != ptr.offset_; }
		bool operator <  (const ShMemMiniPtr<T>& ptr) const	{	return get() <  ptr.offset_;}
		bool operator <= (const ShMemMiniPtr<T>& ptr) const	{	return get() <= ptr.offset_; }
		bool operator >  (const ShMemMiniPtr<T>& ptr) const	{	return get() >  ptr.offset_;}
		bool operator >= (const ShMemMiniPtr<T>& ptr) const	{	return get() >= ptr.offset_; }
	private:
		/// kann Null-Pointer zurueckgeben
		T* deref(PChar base) const { return reinterpret_cast<PT>(base+offset_); }
	private:
		int offset_;
	};

#else
	typedef std::string PvString;

	template <class T>	class ShMemPtr;
	class SharedMem {
	public:
		enum ShMemMode {
			FailNew   = 0,	 // Mem muss vorhanden sein
			CreateNew = 1,   // wenn nicht da, dann wird es erzeugt
			RecreateOld = 2, // Server loescht vorhandenes Mem erst (i.a. problematisch)
			KillOnClose = 4, // 'server'-Version Besitzer raeumt auf
			WriteAccess = 8,  // sonst ReadOnly Zugriff
			StdClient = CreateNew | WriteAccess,
			StdServer = CreateNew | WriteAccess | KillOnClose
		};		/// leeres ShMem, etwa fuer Arrays
		SharedMem() {}
		/// controllerverwaltetes Shared-Mem, server==true: ShMem wird erzeugt/vernichtet
		SharedMem(PvString const& , ShMemMode , int ) {}
		/// selbstverwaltetes ShMem, server==true: ShMem wird erzeugt/vernichtet
		SharedMem(int , PvString const& , ShMemMode , int ) {}
		/// controllerverwaltetes Shared-Mem, server==true: ShMem wird erzeugt/vernichtet
		SharedMem(PvString const& ) {}
		/// selbstverwaltetes ShMem, server==true: ShMem wird erzeugt/vernichtet
		SharedMem(int , PvString const& ) {}
		/// \todo ueber das virtual muss man noch reden
		virtual ~SharedMem() {}
		/// groesse des ShMEm
		int size() const { return 0; }
	public:
		static bool failNew(ShMemMode mode) { return false; }
		static bool createNew(ShMemMode mode) { return false; }
		static bool writeAccess(ShMemMode mode) { return false; }
		static bool recreateOld(ShMemMode mode) { return false; }
	public:
		/// verzoegertes Anlegen des ShMem, size ist optional, ggf aus fd
		void set(ShMemMode , int size) {}
		/// verzoegertes Anlegen des ShMem, size ist optional, ggf aus fd
		void set(int index, PvString const& fileName, ShMemMode mode, int size) {}
		void set(PvString name, ShMemMode mode, int Size) {}
		/// Zugriff auf den Basis-Pointer
		PVoid begin()  	const { return NULL; }
		/// nullPtr-Pruefung
		bool isValid() const { return false; }
		/// aus einem normalo-Ptr wird ein ShMemPtr, oder Funktion wirft
		template <class T>
		ShMemPtr<T> toShMemPtr(T *) { return ShMemPtr<T>(); }
	public:
		///	debug-Schnittstelle + intern
		int 	handle() const { return 0; }
	};

	class SharedMemControl {
	public:
		enum ShMemMode {
			FailNew   = 0,	 // Mem muss vorhanden sein
			CreateNew = 1,   // wenn nicht da, dann wird es erzeugt
			RecreateOld = 2, // Server loescht vorhandenes Mem erst (i.a. problematisch)
			KillOnClose = 4, // 'server'-Version Besitzer raeumt auf
			WriteAccess = 8,  // sonst ReadOnly Zugriff
			StdClient = CreateNew | WriteAccess,
			StdServer = CreateNew | WriteAccess | KillOnClose
		};		/// der einzige CTor
		SharedMemControl();
		~SharedMemControl() {}
	public:
		/// handverwaltetes (handle-Vorgabe) ShMem mit Namen
		int registerMemory(int , PvString , SharedMem &, int ) { return 0; }
		/// handverwaltetes (handle-Vorgabe) leeres ShMem
		int registerMemory(int , PvString ) { return 0; }
		/// Controllerverwaltetes (handle wird erzeugt) ShMem mit Namen
		int registerMemory(PvString , SharedMem &, int ) { return 0; }
		/// Controllerverwaltetes leeres ShMem mit Namen
		int registerMemory(PvString ) { return 0; }
		/// der haeufigste Zugriff ist von Handle auf SharedMem
		SharedMem& operator [] (int )  { return shMem_; }
		/// SharedMem per name geht auch
		SharedMem& operator [] (PvString )  { return shMem_; }
		/// handle zu name
		PvString name (int ) const { return PvString(""); }
		/// name zu handle
		int findHandle (PvString ) const { return  0; }
	public:
		// Test-Schnittstelle
		int numMemEntries() const { return 0; }
		int numNameEntries() const { return 0; }
	private:
		SharedMem shMem_;
	};

	template <class T>
	class ShMemPtr {
	public:
		/// fuer Arrays
			ShMemPtr()  {}
			/// copy-CTor
			ShMemPtr(ShMemPtr const&) {}
			/// zeigt auf Basis-Pointer
			ShMemPtr(SharedMem const& ) {}
			/// deserialize
			//ShMemPtr(MessageBuffer const& ) {}
			/// berechnet aus realem Pointer den Offset
			ShMemPtr(SharedMem const& , T ) {}
			/// auf Mehrfachzuweisung wird verzichtet (kein Rueckgabewert)
			void operator = (ShMemPtr const& )  {		}
			// !=0 und in SharedMem.size
			bool isValid() const { return false; }
			// das Standard-Zeug, was ein Pointer so koennen sollte
			T* operator -> ()	{	return NULL;	}
			const T* operator -> () const	{ return NULL;	}
			T& operator * () { return *checkedDeref();	}
			const T& operator * () const { return *checkedDeref();	}
			T* get() { return deref(); }
			const T* get() const { return deref(); }
			operator T* () { return deref(); }
			operator const T* () const { return deref(); }
			bool operator ! () const { return isNull();	}
			bool isNull() const {	return true; }
			void setNull() {}
			/// ???? sollte hier auf PhysPtr verglichen werden ????
			bool operator == (const ShMemPtr<T>& ptr) const	{	return false; }
			bool operator != (const ShMemPtr<T>& ptr) const	{	return false; }
			bool operator <  (const ShMemPtr<T>& ptr) const	{	return false;}
			bool operator <= (const ShMemPtr<T>& ptr) const	{	return false; }
			bool operator >  (const ShMemPtr<T>& ptr) const	{	return false;}
			bool operator >= (const ShMemPtr<T>& ptr) const	{	return false; }
			void swap(ShMemPtr<T> &p) {}
		public:
			void init(T const &t) {}
			void init(T *t) {}
		private:
			/// kann Null-Pointer zurueckgeben
			T* deref() const { return unCheckedDeref(); }
			/// wirft bei Null-Ptr, gibt immer gueltigen Ptr zurueck
			T* checkedDeref() const { return unCheckedDeref(); }
			/// erledigt nur die Pointer-Arithmetik
			T* unCheckedDeref() const { return (T*)(NULL);}
			SharedMem &shMem() const {return shMem_; }
		private:
			SharedMem shMem_;
	};

#endif // __QNX__||__linux__

} // namespace system
} // namespace precitec

#endif // SHAREDMEM_H_
