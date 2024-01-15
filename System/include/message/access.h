#ifndef ACCESS_H_
#define ACCESS_H_
#pragma once

/**
 * System::marshal.h
 *
 *  Created on: 22.08.2010
 *      Author: WoX
 *   Copyright: Precitec Vision KG
 */

#include <cstring>
#include <iostream>

namespace precitec
{
namespace system
{
/**
 * die ShMemPtr Implementierung ist fuer den QNX-Mode anders
 * hier werden statt der Daten die Pointer geschickt. Sonst
 * werden ShMemPtr wie SmartPtr behandelt.
 */
template<class T> class ShMemPtr;

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
enum TMode{ StringMode, BinMode, InPlace, Shared };




/**
 * ab hier kann dan spezialisiert werden fuer verschiedene
 * Typen
 */
/**
 * Die generische Implementierung ist gleich der BinMode-
 * Implementierung. Daten werden im Binaerformat kopiert.
 */
template <class T, int Mode>
struct Access {
	static std::size_t read(T & value, PCChar mem) {
		value = *reinterpret_cast<T const*>(mem);
		//std::cout << "Access TG: read" << value << std::endl;
		return sizeof(value);
	}
	static std::size_t read(T & value, std::size_t length, PCChar mem) {
		std::memcpy(reinterpret_cast<void*>(&value), mem, length);
		return length;
	}
	static std::size_t write(T const& value, PChar mem) {
		//std::cout << "Access TG: write " << value << std::endl;
		*reinterpret_cast<T*>(mem) = value;
		return sizeof(value);
	}
	static std::size_t write(T const& value, std::size_t length, PChar mem) {
		//std::cout << "Access TG: write " << length/sizeof(T) << "* "<< value << std::endl;
		std::memcpy(mem, &value, length);
		return length;
	}
};

/// die String-Implementierung ist fuer alle Modi gleich
template<int Mode>
struct Access<PvString, Mode> {
	static std::size_t read(PvString & value, char* mem) {
		value = PvString(mem);
		//std::cout << "Access SG: read" << value << std::endl;
		return value.length()+1;
	}
	static std::size_t write(PvString const& value, char* mem) {
		//std::cout << "Access SG: write" << value << std::endl;
		std::strcpy(mem, value.c_str());
		return value.length()+1;
	}
};






template<class T>
struct Access<ShMemPtr<T>, Shared> {
	static std::size_t read(ShMemPtr<T> & value, char* mem) {
		value = *reinterpret_cast<T const*>(mem);
		//std::cout << "Access TS: read" << value << std::endl;
		return sizeof(value);
	}
	static std::size_t write(ShMemPtr<T> const& value, char* mem) {
		//std::cout << "Access TS: write" << value << std::endl;
		*reinterpret_cast<T*>(mem) = value;
		return sizeof(value);
	}
};

// hs: currently not used - attention, may not work with non pods due to inplace construction (eg Poco::UUID)

// Inplace ist nur sinnvoll fuer Pointer
//template <class PT>
//struct Access<PT*, InPlace> {
//	typedef typename DeRefPtr<PT>::RefType T;
//	typedef typename DeConst<T>::Type NCT;	// falls T bereits const ist, wird es entfernt
//	typedef const NCT *PCT;					// und nun fehlerfrei (wieder) hizugefuegt
//	static uInt read(PT & value, PCChar mem) {
//		value = new(mem) T; // inkonsequent: hier wird CTor ausgefuehrt, im Verktorfall nur reinterpret_cast ausgefuehrt ????
//		return sizeof(T);
//	}
//	static uInt read(PT & value, int length, PCChar mem) {
//		value = reinterpret_cast<PCT>(mem); // der Pointer muss nur uminterpretiert wrden, dann ist alles ok
//		return length;
//	}
//};






} // namespace message
} // namespace system
} // namespace precitec

#endif // ACCESS_H_
