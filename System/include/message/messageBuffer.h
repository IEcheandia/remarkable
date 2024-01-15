#ifndef MESSAGE_BUFFER_H_
#define MESSAGE_BUFFER_H_

#include <iostream>
#include <string.h> // memcpy, memset

#include "SystemManifest.h" // wg Windows DLL

#include "system/types.h"
#include "system/templates.h" // u.a. wg iMax

#include "message/messageException.h"
#include "message/access.h"
/** Namespace fuer das Messageing-System
 */
namespace precitec
{
namespace system
{
namespace message
{

	/**
	 * Die Uebertragung von Variablen auf Puffer und zurueck, gibt es in mehreren
	 * Geschmacksrictungen. Die Default-Methode und Default-Implementierung ist
	 * das binaere Kopieren.
	 * Die Modi sind
	 * 			binaeres Kopieren: schnell, kompakt
	 * 			String-IO: menschenlesbar
	 * 			Inplace: optimal schnell, da nichtkopiert wird; nur fuer Pointer, und
	 * 								nur fuer besondere Faelle, da der Speicher/Puffer nicht freigegeben werden darf
	 * Es gibt Sonderloesungen fuer std::string.
	 * Typen
	 */
/*
	enum TMode{ StringMode, BinMode, InPlace };

	template <class T, int Mode=BinMode>
	struct ToValue {
		void operator () (T & value, PCChar mem) const {
			value = *reinterpret_cast<T const*>(mem);
		}
		void operator () (T & value, int length, PCChar mem) const {
			std::memcpy(&value, mem, length);
		}
	};


	template <class T, int Mode=BinMode>
	struct FromValue {
		void operator () (T const& value, PChar mem)  {
			*reinterpret_cast<T*>(mem) = value;
		}
		void operator () (T const& value, int length, PChar mem) {
			std::memcpy(mem, &value, length);
		}
	};
*/

	/**
	 * ab hier kann dan spezialisiert werden fuer verschiedene
	 * Typen
	 */
/*
	template <class T>
	struct ToValue<T, BinMode> {
		void operator () (T & value, PCChar mem) const {
			value = *reinterpret_cast<T const*>(mem);
		}
		void operator () (T & value, int length, PCChar mem) const {
			std::memcpy(&value, mem, length);
		}
	};

	template <class T>
	struct FromValue<T, BinMode> {
		void operator () (T const& value, PChar mem) {
			*reinterpret_cast<T*>(mem) = value;
		}
		void operator () (T const& value, int length, PChar mem) {
			std::memcpy(mem, &value, length);
		}
	};


	// die String-Implementierung ist fuer alle Modi gleich
	template<>
	struct ToValue<PvString, BinMode> {
		void operator () (PvString & value, char* mem) const { value = PvString(mem); }
	};

	template<>
	struct FromValue<PvString, BinMode> {
		void operator () (PvString const& value, char* mem) {
	  	std::strcpy(mem, value.c_str());
		}
	};



	// Inplace ist nur sinnvoll fuer Pointer
	template <class PT>
	struct ToValue<PT*, InPlace> {
		typedef typename DeRefPtr<PT>::RefType T;
		typedef typename DeConst<T>::Type NCT;	// falls T bereits const ist, wird es entfernt
		typedef const NCT *PCT;					// und nun fehlerfrei (wieder) hizugefuegt
		void operator () (PT & value, PCChar mem) const {
			value = new(mem) T;
		}
		void operator () (PT & value, int length, PCChar mem) const {
			value = reinterpret_cast<PCT>(mem);
		}
	};
*/



//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

/*
	struct SYSTEM_API Header {
		Header () : size(0), messageNum(0), isPulse(false) {}
		int 	size;			///< Messagesize
		int		messageNum;
		bool	isPulse; 	///< Notbehelf, bis QNX-Pulse mit Daten implementiert sind
		int 	priv;			///< Debug-Hilfe
		int 	checkSum;	///< Debug-Hilfe
	};

	class SYSTEM_API MessageBuffer {
	public:
		MessageBuffer() : rawData_(NULL), data_(NULL), cursor_(NULL), m_oMaxBufferSize(0) {}
		MessageBuffer(int size)
		: rawData_(new char [size+sizeof(Header)]),
			data_(rawData_+sizeof(Header)), cursor_(data_), m_oMaxBufferSize(sizeof(Header)+size) { header().size = 0; }
	 ~MessageBuffer() {	if (rawData_) { delete [] rawData_;} }
	 	// klar, der Header
	 	Header const& header() 	const { return *reinterpret_cast<Header*>(rawData_); }
	 	Header &header() { return *reinterpret_cast<Header*>(rawData_); }
	 	char* rawData() const { return rawData_; }
	 	// internen Puffer neu setzen
		void 	setData(int size);
	 	PChar	data() 			const { return data_; }
	 	PChar	cursor() 		const { return cursor_;}
	 	/// Groesse derNutzdaten
		int  	msgSize() 	const { return (rawData_) ? header().size : 0; }
		// Groesse der Gesamtdaten (inkl. Header)
		int  	dataSize() const { return cursor_-rawData_; }
		/// Notnagel bis QNX-Pulse implementiert sind
		void 	makePulse() { if (rawData_) header().isPulse = true; }
		/// Notnagel bis QNX-Pulse implementiert sind
		void 	makeMessage() { if (rawData_) header().isPulse = false; }
		/// Notnagel bis QNX-Pulse implementiert sind
		bool 	isPulse() const { return (rawData_) && header().isPulse == true; }
		/// die MsgNummer
		int messageNum() const { return header().messageNum; }
		/// die MsgNummer
		void setMessageNum(int num) { header().messageNum = num; }
		/// puffer wird gelesen: cursor-update, msgSize bleibt konstant
		void 	skip(int delta) const { cursor_+=delta; }
		/// move while writing to buffer: update  cursor + msgSize
		void 	move(int delta) { cursor_+=delta; header().size+=delta; }
		/// cursor  wird zurueck gesetzt; header->Size = MsgSize bleibt erhalten
		void 	rewind() const { cursor_=data_; }
		/// reset for creating a new message: msgSize = 0
		void 	clear() { cursor_=data_; header().size = 0; }
		/// maximal msgSize
		int 	limMsgSize() const { return m_oMaxBufferSize-sizeof(Header); }
		/// fuer NoProtocol Puffer werden einfach ineinander kopiert
		void copyFrom(MessageBuffer &from);
		/// Restlaengenkontrolle des Puffers
		bool isFree(int size) const {	return rawData_+m_oMaxBufferSize > cursor_+size; }
		/// debug-Tool Typisiertes auslesen aus Puffer, ohne cursor-Bewegung
		/// eine einfache CheckSumme
		int calcCheckSum() const;
		int checkSum() const { return header().checkSum; }
		void setCheckSum() { header().checkSum = calcCheckSum(); }
		bool queryCheckSum() { return header().checkSum == calcCheckSum(); }
		// debug
		int priv() const { return header().priv; }
		// debug
		void setPriv(int p) { header().priv = p; }
		//template <class T>
	 	//void peek(T& value) const { return ToValue<T, BinMode>(value, cursor()); }
		//template <class T>
	 	//void peek(T& value, int n) const { return ToValue<T, BinMode>(value, n, cursor()); }
		friend  std::ostream &operator <<(std::ostream &os, MessageBuffer const& b) {
			os << "{" << b.m_oMaxBufferSize << "}"; return os;
		}
	public:
		template <class T>
	 	void dump(std::ostream &os);
	 	void dump()	{ dump<char>(std::cout); std::cout << std::endl; }
	private:
		char 		*rawData_;
		//Header	*header_;
		char		*data_;	// zeigt auf Daten anfang
		mutable char 		*cursor_;	// read/Write-Zeiger
		int			 m_oMaxBufferSize;		// zur  Buff-Overflow-Abfrage
	}; // MessageBuffer

	template <class T>
 	void MessageBuffer::dump(std::ostream &os) {
 		os 	<< "<<" << "#" << header().messageNum << ":"
				<< msgSize() << "Bytes "
				<< (header().isPulse ? "Pulse" : "Message")
				<< ">> [";
 		os.flush();
 		T* msgStart = (T*)(data_);
 		uInt i;
		for (i=0; i < (msgSize()/sizeof(T))-1; ++i) {
				os << int(msgStart[i]) << ":" ;
		}
		if (msgSize()>0) os << int(msgStart[i]); // und das letzte auch noch
		os << "]";
 	}

 	inline int MessageBuffer::calcCheckSum() const {
 		int cs = 0;
		for (int i=0; i< msgSize(); ++i) {
			cs += data_[i] + i; // i wird hinzugefuegt, damit bei genullten Klassen die Laenge geprueft wird
		}
 		return cs;
 	}
*/
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------



	/*

	template <class T>
	void deMarshalBuffer(T & value, MessageBuffer const& buffer) {
		value = *reinterpret_cast<T const*>(buffer.cursor());
		//std::cout << "demarshalling<T>  " << value << "  at: " << int(buffer.cursor()-buffer.data()) << std::endl;
		buffer.skip(sizeof(T));
	}

	template <class T>
	void deMarshalBuffer(T & value, int length, MessageBuffer const& buffer) {
		std::memcpy(&value, buffer.cursor(), length);
		//std::cout << "deMarshalling " << length/sizeof(T) << " : " ;
		//for (int i=0; i<length; ++i) {
		//	std::cout << (&value)[i] <<  std::endl;
		//}
		//std::cout  <<  std::endl;		buffer.skip(length);
		buffer.skip(length);
	}

	template <class T>
	void marshalBuffer(T const& value, MessageBuffer &buffer) {
		if (!buffer.isFree(sizeof(T))) throw MessageException("insufficient MessageBuffer size");
		*reinterpret_cast<T*>(buffer.cursor()) = value;
		//std::cout << "marshalling " << value << " at: " <<  int(buffer.cursor()-buffer.data()) << std::endl;
		buffer.move(sizeof(T));
	}

	template <class T>
	void marshalBuffer(T const& value, int length, MessageBuffer &buffer) {
		if (!buffer.isFree(length)) throw MessageException("insufficient MessageBuffer size");
		std::memcpy(buffer.cursor(), &value, length);
		//std::cout << "marshalling " << length << " : " ;
		//for (int i=0; i<length; ++i) {
		//	std::cout << (&value)[i] <<  std::endl;
		//}
			//std::cout  <<  std::endl;
		buffer.move(length);
	}
	*/
/*
	template<>
	void deMarshalBuffer<PvString>(PvString & value, MessageBuffer const&buffer);

	template<>
	void marshalBuffer<PvString>(PvString const& value, MessageBuffer &buffer);
*/
/*
	template<>
	inline void deMarshalBuffer<PvString>(PvString & value, MessageBuffer const&buffer) {
	   value = std::string(buffer.cursor());
	 	//	std::cout << "marshalling buff:" << (char*)buffer.cursor() << " with len: " << value.length() << " to: " << value << std::endl;
	   buffer.skip(value.length()+1); //  update cursor, leave msgSize
	}


	template<>
	inline void marshalBuffer<PvString>(PvString const& value, MessageBuffer &buffer) {
		 //std::cout << "pre-marshalling str:" << value << " with len: " << value.length() << " to: " << (char*)buffer.cursor() << std::endl;
		if (!buffer.isFree(value.length())) throw MessageException("insufficient MessageBuffer size");
	   std::strcpy(buffer.cursor(), value.c_str());
		 //std::cout << "marshalling str:" << value << " with len: " << value.length() << " to: " << (char*)buffer.cursor() << std::endl;
	   buffer.move(value.length()+1); //  update cursor + msgSize
	}
	*/
} // message
} // system
} // precitec




	using precitec::system::message::MessageException;

namespace precitec
{
namespace system
{
namespace message
{


	struct SYSTEM_API Header {
		Header() : messageNum(0), size(0), checkSum(0) {}
		byte	messageNum;		///< definiert Aufgabe [0..255]
		std::size_t 	size;			///< Messagesize
		int		checkSum;		///< soll verhidern, dass ungueltige Puffer ausgewertet werden

	};

	/**
	 * MessagebufferBase erledigt die Pufferverwaltung, ohne den Speicher bereitzustellen.
	 *
	 */
	class SYSTEM_API MessageBuffer {
	public:
		MessageBuffer() : buffer_(NULL), cursor_(NULL), limSize_(0), multiUse_(0)	{}
		MessageBuffer(PChar p, int size)
		: buffer_(p),	cursor_(buffer_+sizeof(Header)), limSize_(size), multiUse_(0)	{ 
			//std::cout << "cTor::MessageBuffer(" << std::hex << int(p) << std::dec << ", " << size << ")" << std::endl;
			header().size = 0; // die aktuelle Messagegroesse
		}
	  virtual ~MessageBuffer() {}
	public:
		// evnt-Locking-Interface: darf nichts tun, was nicht mit Locking zu tun hat, da die Message-
		// Routinen dieses Interface nicht verwenden. Die defaultImplementierung tut nichts
		/// (Write)Locks fuer N Leser setzen (writelock wird aufgehoben
		virtual void lockForReading(int n=1) {}
		/// ein WriteLock freigeben
		virtual void freeBuffer() {	std::cout << "MessageBuffer::freeBuffer() should not be called!!!" << std::endl;	}
		/// ist Puffer frei
		virtual bool lockForWriting() {
			//std::cout << "MessageBuffer::lockForWriting: should not be called" << std::endl;
			return true;
		}
		/// is es eine SharedMem Buffer oder nicht
		virtual bool supportsShMemAccess() const { return false; }

		virtual int bufferToOffset() const { return 1; }
	public:
		/// puffer wird gelesen: cursor-update, msgSize bleibt konstant
		void 	skip(std::size_t delta) const { cursor_+=delta; }
		/// move while writing to buffer: update  cursor + msgSize
		//void 	move(int delta) { cursor_+=delta; header().size+=delta; }
		/// cursor  wird zurueck gesetzt; header->Size = MsgSize bleibt erhalten
		void 	rewind() const { cursor_=msgStart(); }
		/// reset for creating a new message: msgSize = 0
		virtual void 	clear() { cursor_=msgStart(); /*header().size = 0;*/ }
		/// fuer NoProtocol Puffer werden einfach ineinander kopiert
		void copyFrom(MessageBuffer const&from);
		/// Restlaengenkontrolle des Puffers
		bool isFree(int size) const {	return rawData()+limSize_ > cursor_+size; }
		/// Nach-Initialisieren - ??? redundant mit resize, oder !?
	 	void setData(int size);

		/// debug-Tool Typisiertes auslesen aus Puffer, ohne cursor-Bewegung
		/// debug-Hilfe
		template <class T>
	 	void dump(std::ostream &os);
	 	void dump()	{ dump<char>(std::cout); std::cout << std::endl; }
		virtual int currentElement() const { return -1; }

	public:
		/// neuen Puffer (Groesse size) holen oder alten reinitialisieren
		virtual void reset(int size=0) {
			//std::cout << "MessageBuffer::reset" << std::endl;
			// wenn aktueller Puffer schon wieder frei ist (nicht der Normalfall) wiederverwenden
			if (this->lockForWriting()&&(size<=limSize_)) this->resetCursor(size);
			// sonst neuen Puffer aus grossem Puffer anfordern
			else 										this->getNewBuffer(size);
			if (rawData()==nullptr) {
				std::cout << "MessageBuffer::reset: failed" << std::endl;
			}
		}
		/// Restlaengenkontrolle des Puffers (berechtet Soll- vs. Ist-Datenende um Vorzeichenprobleme zu vermeiden)
		bool hasSpace(std::size_t size) { return testRemainingBuffSize(size); }

		/// multiUse bestimmt, wie oft ein Puffer(virtuell) vom Publisher gelockt wird, damit jeder Subscriber ihn freigeben kann)
		void addMultiUse() { ++multiUse_; }
		/// multiUse bestimmt, wie oft ein Puffer(virtuell) vom Publisher gelockt wird, damit jeder Subscriber ihn freigeben kann)
		void removeMultiUse() { --multiUse_; }
		int multiUse() { return multiUse_; }
		void resetMultiUse() { multiUse_=0; }

	protected:
	 	/// wenn moeglich neuen Puffer mit passender Groesse holen, bisherige Daten kopieren
		virtual void resize(int size) {};
	 	/// neuen Puffer mit passender Groesse holen
		virtual void getNewBuffer(int size) {};
	 	/// wenn moeglich neuen Puffer mit passender Groesse holen, bisherige Daten kopieren
		virtual bool testRemainingBuffSize(std::size_t size) { return false; };
		/// cursor initialisieren (nur debug??)
		virtual void resetCursor(int size=0) {
			//std::cout << "MessageBuffer::resetCursor: should NOT be called" << std::endl;
			cursor_ = msgStart();
		}
	public:
		// Accessoren
		/// maximal moegliche msgSize
		int 	limMsgSize() const { return iMax(0, int(limSize_-sizeof(Header))); }
		Header  &header() const { return *reinterpret_cast<Header*>(buffer_); }
		// nach clear-Aufruf sind diese Funktionen sinnvoll
		/// Pufferanfang
		PChar rawData() const { return buffer_; }
	//protected:
		///
		virtual void setRawData(PChar p, int size) { buffer_ = p; limSize_ = size; }
		///
		int 	limSize() const { return limSize_; }
	public:
		/// Anfang  des MessageTiels (nach Header)
		PChar	msgStart() 	const { return buffer_ + sizeof(Header); }
	 	/// Groesse der Nutzdaten, Achtung nur gueltig, wenn Header  cont upgedated ist, oder bei received Message
		std::size_t  	msgSize() 	const { return buffer_ ? header().size : 0; }
		/// Messaagegroesse in Header schreiben
		//void 	setMsgSize(int s) { header().size = s; }
		/// aktuelle Position des Cursors
		intptr_t currentPos() const { return cursor() - msgStart(); }
		/// aus aktuellem Cursor die MsgSize berechnen und in Header schreiben
		void 	setMsgSize() { header().size = currentPos(); }

		/// Groesse der Gesamtdaten (inkl. Header) (klappt auch bei buffer_ == NULL)
		intptr_t dataSize() const { return cursor_-buffer_; }

		/// die MsgNummer
		int messageNum() const { return header().messageNum; }
		/// die MsgNummer
		void setMessageNum(int num) { header().messageNum = num; }
		/// debug-Info setzen
		void setNames(PvString const& serverName, PvString const& messageName) {
			memcpy(interfaceName_, serverName.c_str(), iMax(std::size_t(DebugStringSize), serverName.length()));
			memcpy(messageName_, messageName.c_str(), iMax(std::size_t(DebugStringSize), messageName.length()));
		}
	public:
		// cursor-Befehler
		/// Datenzeiger zum Lesen-Schreiben einer Message
	 	PChar	cursor() 		const { return cursor_; }
		/// MessageEnde neu setzen
		void 	moveCursor(std::size_t delta) const { cursor_ += delta;	}
		friend
		std::ostream &operator << (std::ostream &os, MessageBuffer const&b) {
			os << "[[" << std::hex << long (b.buffer_) << "->" << long (b.cursor_) << "]] ";
			os << "msg: " << b.messageNum();
#ifndef NDEBUG
			os << " " << b.interfaceName_ << " " << b.messageName_;
#endif
			return os;
		}
	protected:
		/// aktueller Puffer, kann "Konstante" sein muss aber nicht
		char 						*buffer_;
		/// Verwaltungsinfo kann veraendert werden, auch wenn Puffer ro ist
		mutable char 		*cursor_;
		/// Groesse ist nur bei statischen Buffern konstant
		int 						limSize_;
		/// Number of Subscribers to a Event!-Buffer (for access-counting)
		int							multiUse_;
//#ifndef NDEBUG // wir lassen erstmal die Headergroesse konstant, um Probleme zu vermeiden
		enum { DebugStringSize = 32 };
		char						interfaceName_[DebugStringSize];
		char						messageName_[DebugStringSize];
//#endif
	};

	template <class T>
 	void MessageBuffer::dump(std::ostream &os) {
 		os 	<< "<<" << "#" << messageNum() << ":"
				<< msgSize() << "Bytes "
				<< ">> [";
 		os.flush();
 		T* msg_Start = (T*)(msgStart());
 		uInt i(0);
 		if (msgSize()==0) return;
 		if (msgSize()>1) {
			for (i=0; i < (msgSize()/sizeof(T))-1; ++i) {
					os << int(msg_Start[i]) << ":" ;
			}
 		}
		if (msgSize()>0) os << int(msg_Start[i]); // und das letzte auch noch
		os << "]"; os.flush();
 	}

 	inline void MessageBuffer::setData(int size) {
		if (rawData()) delete [] rawData();
		try {
			setRawData(new char [size+sizeof(Header)], size);
			memset( rawData(), 0, limSize_ );
		} catch (std::exception &e) {
			std::cout << "MessageBuffer::setDate threw: " << e.what() << std::endl;
		}
		//header_  = new(rawData_) Header; // in-place Allokation
		//data_		 = rawData() + sizeof(Header);
		cursor_  = msgStart();
		limSize_ = size+sizeof(Header);
	}

	// fuer Null-Protocol -> \todo sollte ueber Copy-Ctor erledigbar sein, oder????
 	inline void MessageBuffer::copyFrom(MessageBuffer const&from) {
		// Buffer-overflow vermeiden
		int xferSize = iMin(limSize_, from.limSize());
		//std::cout << "copyFrom: " << from.msgSize() << " to: " << m_oMaxBufferSize << std::endl;
		memcpy(rawData(), from.rawData(), xferSize);
		// \todo muss das hier rein????
		cursor_ = msgStart();
	}


	/**
	 * Einfachste Implementierung, Puffergroesse muss von Anfang an feststehen und
	 * ausreichen, es gibt keine Nachallokierung (waehrend der Serialisierung)
	 * buffer_ der Basisklasse haelt die Daten, wird aber von dieser Klasse verwaltet
	 */
	class SYSTEM_API StaticMessageBuffer : public MessageBuffer {
	public:
		/// Default CTor wg Array[MsgBuffer]
		StaticMessageBuffer() : MessageBuffer() {}
		/// std-CTor (nicht Qnx-Protokol) ohne ShMem Support
		StaticMessageBuffer(int size)
			try :  MessageBuffer(new char[size], size) , supportsShMemAccess_(false)
		{
			memset( rawData(), 0, size );
		}
		catch (...) { std::cout << "ctor::StaticMessageBuffer(" << size << ") failed " << std::endl; }

		/// std-CTor (Qnx-Protokol) mit optionalem Support fuer ShMemPtr (=Qnx-Protocol)
		StaticMessageBuffer(int size, bool support)
		: MessageBuffer(), supportsShMemAccess_(support) {
			setData(size);
		}
		/// Puffer der Basisklase wird hier verwaltet
		virtual ~StaticMessageBuffer() { if (rawData() ) { delete [] rawData(); } }
	public:

		/// \todo PerformanceBug!!! ggf buffEnd als Variable
		virtual bool testRemainingBuffSize(std::size_t size) {
			return msgStart() + limMsgSize() > cursor() + size;
		}
		/// is es eine SharedMem Buffer oder nicht
		virtual bool supportsShMemAccess() const { return supportsShMemAccess_; }
		/// wird von Sender/Receiver bei QNX-Protokoll gesetzt
		virtual void setShMemAccess(bool supported) { supportsShMemAccess_ = supported; }
		/// neuen Puffer (Groesse size) holen oder alten reinitialisieren
//		void reset(int size=0) {
//			if (lockForWriting()&&(size<=limSize())) this->resetCursor();
//			else this->getNewBuffer(size);
//		}
		/// Restlaengenkontrolle des Puffers (berechtet Soll- vs. Ist-Datenende um Vorzeichenprobleme zu vermeiden)
		bool hasSpace(std::size_t size) { return testRemainingBuffSize(size); }
	 	/// neuen Puffer mit passender Groesse holen, bisherige Daten kopieren
		virtual void resize(int size) {
			PChar tmpP(rawData());
			int oldSize(limSize());
			try {
				setRawData( new char[size], size ); // RAII
			} catch (...) {
				// sollte new werfen, wollen wir das mitbekommen, damit pTemp nicht leckt
				setRawData(NULL, 0);
				// throw;
			}
			if (tmpP) {
				memcpy(rawData(), tmpP, oldSize);
				delete [] tmpP;
			}
		}
		// ist fuer das Freigeben von Locks vorgesehen
		virtual void freeBuffer() {}

	protected:
	 	/// neuen Puffer mit passender Groesse holen
		virtual void getNewBuffer(int size) {
			//std::cout << "StaticMessageBuffer::getNewBuffer: should NOT be called " << std::hex << int(rawData()) << std::dec << std::endl;
			if (rawData()) { delete [] rawData(); }
			setRawData( new char[size], size ); // RAII
		}
	private:
		/// Flag, das gesetzt wird bei QNX-Messaging. Interprozess-Kommunikation von ShPtr kann ueber Ptr erfolgen
		bool supportsShMemAccess_;
	};

	//typedef StaticMessageBuffer MessageBuffer;



	/**

	 */
	template <class T>
	void deMarshalBuffer(T & value, MessageBuffer const& buffer) {
		buffer.moveCursor(Access<T, BinMode>::read(value, buffer.cursor()));
	}

	template <class T>
	void deMarshalBuffer(T & value, std::size_t length, MessageBuffer const& buffer) {
		buffer.moveCursor(Access<T, BinMode>::read(value, length, buffer.cursor()));
	}

	template <class T>
	void marshalBuffer(T const& value, MessageBuffer &buffer) {
		if (!buffer.hasSpace(sizeof(T))) throw MessageException("insufficient MessageBuffer size");
		//std::cout << "marshalling " << value << " at: " <<  int(buffer.cursor()-buffer.data()) << std::endl;
		buffer.moveCursor(Access<T, BinMode>::write(value, buffer.cursor()));
	}

	template <class T>
	void marshalBuffer(T const& value, std::size_t length, MessageBuffer &buffer) {
		if (!buffer.hasSpace(length)) throw MessageException("insufficient MessageBuffer size");
		//std::cout << "marshalling " << length/sizeof(T) << "* "<< value[0] << " at: " <<  int(buffer.cursor()-buffer.msgStart()) << std::endl;
		buffer.moveCursor(Access<T, BinMode>::write(value, length, buffer.cursor()));
	}

	/*
	template<>
	void deMarshalBuffer<PvString>(PvString & value, MessageBuffer const&buffer);

	template<>
	void marshalBuffer<PvString>(PvString const& value, MessageBuffer &buffer);
	*/

	/*
	template<>
	inline void deMarshalBuffer<PvString>(PvString & value, MessageBuffer const&buffer) {
	   value = std::string(buffer.cursor());
	 	//	std::cout << "marshalling buff:" << (char*)buffer.cursor() << " with len: " << value.length() << " to: " << value << std::endl;
	   buffer.moveCursor(value.length()+1); //  update cursor, leave msgSize
	}


	template<>
	inline void marshalBuffer<PvString>(PvString const& value, MessageBuffer &buffer) {
		 //std::cout << "pre-marshalling str:" << value << " with len: " << value.length() << " to: " << (char*)buffer.cursor() << std::endl;
		if (!buffer.isFree(value.length())) throw MessageException("insuffizient MessageBuffer size");
	   std::strcpy(buffer.cursor(), value.c_str());
		 //std::cout << "marshalling str:" << value << " with len: " << value.length() << " to: " << (char*)buffer.cursor() << std::endl;
	   buffer.moveCursor(value.length()+1); //  update cursor + msgSize
	}
	*/

} // namespace message
} // namespace system
} // namespace precitec

#endif /*MESSAGE_BUFFER_H_*/
