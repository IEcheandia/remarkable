#ifndef TEMPLATES_H
#define TEMPLATES_H

#include <string>
#include <sstream>
#include <vector>
#include <memory>

#include "system/policies.h"
#include "system/types.h"
#include "Poco/SharedPtr.h"
#include "system/typeTraits.h"
//#include "system/sharedMem.h"

/**
 * Falls der Wert v !=  default CTor des Typs ist wird v
 * zurueckgegeben sonst der mitgelieferte Defaultwert
 * @param v aktueller Wert
 * @param d Default Wert
 * @return v (wenn v!=T()) sonst d
 */
template <class T>
T valueOrDefault(T v, T d) { return v!=T() ? v : d; }

/**
 * aehnlich wie valueOnDefault, gibt Ersatzwert zuruec
 * wenn der Wert No<T>::Value ist.
 * @param v aktueller Wert
 * @param r Ersatz-Wert
 * @return v (wenn v!=No<T>::Value) sonst r
 */
template <class T>
T replaceNoValue(T const& v, T const& r) { return (v != precitec::No<T>::Value) ? v : r; }

/**
 * Ersatz fuer min, das es zwischen QNX und Win Probleme
 * mit std:: - Namespace gibt
 */
template <class T>
T iMin(T const&a, T const& b) { return a<b ? a : b; }

/**
 * Ersatz fuer max, das es zwischen QNX und Win Probleme
 * mit std:: - Namespace gibt
 */
template <class T>
T iMax(T const&a, T const& b) { return b<a ? a : b; }

/**
 * Typ aus String auslesen
 * hat Typinferenz aber bloede Syntax: T t; fromString(s, t);
 */
template <class T>
void fromString(PvString const&s, T &t) {
	std::stringstream ss(s);
	ss >> t;
}

/**
 * Typ aus String auslesen
 * keine Typinferenz aber bessere Syntax: T t(fromString<T>(s));
 */
template <class T>
T fromString(PvString const&s) {
	std::stringstream ss(s);
	T t;
	ss >> t;
	return t;
}
// specialized version for bool which treats bool as 'no' and 'false'
template <>
inline bool fromString<bool>(PvString const&s) {
	std::stringstream ss(s);
	bool t;
	ss >> std::boolalpha >> t;
	return t;
}

/** Daten-Konversion in String
 * alle PODs und Klassen mit <<-Operator
 * koennen nun direkt in Strings geschiftet werden
 * !!!! Achtung bei char/Byte-Pointer (werden wie Strings behandelt)
 */

template <class T>
PvString& operator << (PvString &s, T const & t)
{
	std::stringstream ss; ss << t;
	s += ss.str();
	return s ;
}


/// swap fuer Pods oder Klassen
template <class T, int isPod>
struct Swap {
	static void swap(T &l, T &r) { l.swap(r); }
};

template <class T>
struct Swap<T, 1> {
	static void swap(T &l, T &r) { std::swap(l, r); }
};

/** Hilfstemplate
 * wenn ein Template A<T> und A<T&> erzeugt werden koennenn soll
 * kann es manchmal zu &&T kommen, was boese ist. Ref loes t dieses
 * problem
 */
 /*
template <class T>
class DeRef
{
	typedef T Type;
};

template <>
class DeRef<class T&>
{
	typedef T Type;
};
*/

/**
 * 'Sortiere' zwei Zahlen aufsteigend
 * Basis verschiedener Sortier-Algorithmen
 */
template <class Item>
void compXchg(Item *a, Item *b) {
	if (*b<*a) { std::swap(a, b); }
}

/**
 * 'Sortiere' zwei Zahlen aufsteigend
 * Basis verschiedener Sortier-Algorithmen
 */
template <class Item>
void compXchg(Item &a, Item &b) {
	if (b<a) { std::swap(a, b); }
}

/**
 * 'Sortiere' zwei Zahlen aufsteigend, mit externem Vergleichs-Operator
 * Basis verschiedener Sortier-Algorithmen
 */
template <class Item, class Comp>
void compXchgComp(Item &a, Item &b, Comp const& smaller) {
	if (smaller(b,a)) { std::swap(a, b); } // is faster
}

/// depricated : use std::swap
template <class Item>
inline void xchg(Item &a, Item &b) { std::swap(a, b); }

/**
 * sort values in place by value highest first
 * \ingroup ipDll
 *
 * \version 1.0
 *
 * \date 02-07-2005
 *
 * \author WoR
 *
 */
template <class Item>
inline void sort(Item a[], uInt l, uInt r)
{
	uInt i, j;
	Item v;
	for (i=r; i>l; i--) { compXchg(a[i-1], a[i]); }
  for (i=l+2; i<=r; i++) {
		j = i;
		v =a[i];
		while(v<a[j-1]) {	a[j] = a[j-1]; j--;	}
		a[j] = v;
	} // for
} // sort


/**
 * sort values by index highest first
 * \ingroup ipDll
 *
 * \version 1.0
 *
 * \date 02-07-2005
 *
 * \author WoR
 *
 */
template <class Item>
inline void iSort(Item a[], uInt vI[], uInt l, uInt r)
{
	uInt i, j;
	uInt v;
	for (i=r; i>l; i--) {
		if (a[vI[i]] < a[vI[i-1]]) xchg(vI[i-1], vI[i]);
	}
	for (i=l+2; i<=r; i++) {
		j = i;
		v =vI[i];
		while(a[v]<a[vI[j-1]]) { vI[j] = vI[j-1]; j--; }
		vI[j] = v;
	}
} // sort

/**
 * sort values in place by value highest first
 * \ingroup ipDll
 *
 * \version 1.0
 *
 * \date 02-07-2005
 *
 * \author WoR
 *
 */
template <class Item>
inline void inverseSort(Item a[], uInt l, uInt r)
{
	uInt i, j;
	Item v;
	for (i=r; i>l; i--) { invCompXchg(a[i-1], a[i]); }
  for (i=l+2; i<=r; i++) {
		j = i;
		v =a[i];
		while(v>a[j-1]) {	a[j] = a[j-1]; j--;	}
		a[j] = v;
	} // for
} // sort

/**
 * sort values by index lowest first
 * \ingroup ipDll
 *
 * \version 1.0
 *
 * \date 02-07-2005
 *
 * \author WoR
 *
 */
template <class Item>
inline void inverseISort(Item a[], uInt vI[], uInt l, uInt r)
{
	uInt i, j;
	uInt v;
	for (i=r; l<i; i--) {
		if (a[vI[i]] > a[vI[i-1]]) xchg(vI[i-1], vI[i]);
	}
	for (i=l+2; i<=r; i++) {
		j = i;
		v =vI[i];
		while(a[v]>a[vI[j-1]]) { vI[j] = vI[j-1]; j--; }
		vI[j] = v;
	}
} // sort


/**
 * sort values in place by value highest first
 * \ingroup ipDll
 *
 * \version 1.0
 *
 * \date 02-07-2005
 *
 * \author WoR
 *
 */
template <class Item, class Comp>
inline void sortComp(Item a[], uInt l, uInt r, Comp const& smaller)
{
	uInt i, j;
	Item v;
	for (i=r; i>l; i--) { compXchgComp(a[i-1], a[i], smaller); }
  for (i=l+2; i<=r; i++) {
		j = i;
		v =a[i];
		while(smaller(v, a[j-1])) {	a[j] = a[j-1]; j--;	}
		a[j] = v;
	} // for
} // sort

/**
 * sort values by uInt highest first
 * \ingroup ipDll
 *
 * \version 1.0
 *
 * \date 02-07-2005
 *
 * \author WoR
 *
 */
template <class Item, class Comp>
inline void iSortComp(Item a[], uInt vI[], uInt l, uInt r, Comp const& smaller)
{
	uInt i, j;
	uInt v;
	for (i=r; i>l; i--) {
		if (smaller(a[vI[i]], a[vI[i-1]])) xchg(vI[i-1], vI[i]);
	}
	for (i=l+2; i<=r; i++) {
		j = i;
		v =vI[i];
		while(smaller(a[v], a[vI[j-1]])) { vI[j] = vI[j-1]; j--; }
		vI[j] = v;
	}
} // sort


/**
 * Wert wird auf Intervall [low .. high) projeziert
 * es ist _Absicht_, dass alle drei Typen gleich sind
 * und nicht gecastet wird
 */
template <class Item>
void clamp(Item low, Item &val, Item high) {
	if      (val<low)   val = low;
	else if (val>high)  val = high;
} // clamp

/**
 *  berechnet das Quadrat eines Ausdrucks
 */
template <class T>
T sqr(T const a) { return a*a; }



/*
 * Hier fangen einige TypeTraists und TMP-Funktioinen auf,
 * die zur Idntifizierung der Typen bei der Serialisierung
 * dienen.
 */


namespace precitec {
namespace system{
template <class T>
class ShMemPtr;
}
}

/**
 * kann T in U umgewandelt werden; Resultat in IsConvertibleTo::Value
 * Achtung!!!: braucht _mindestens_ gcc 3.5 (selbst dann noch Warnungen)
 */
template <class T, class U>
class IsConvertibleTo
{
protected:
	typedef char Converted;            ///< Rueckgabewert bei erfolgreicher Konversion
	class Default { char mUnused[2]; };///< sizeof(Default) != sizeof Converted
	static Converted test(U const&);   ///< wird ausgewaehlt, wenn T auf U konvertiert werden kann
	static Default   test(...);        ///< wird ausgewaehlt, wenn T inkompatibel zu U
	static T makeT();                  ///< erzeugt Typ T selbst wenn T() privat ist, wird nie erzeugt
public:
	const static bool Value =
				sizeof(test(makeT())) ==
				sizeof(Converted);  ///< Value==true wenn resultat von makeT in U umgewandelt wrden kann
};

/**
 * Ueber die Pointer kann man std-Konversionen verhindern, es bleiben also
 * nur noch echte Ableitungs-Beziehungen uebrig
 * Achtung IsSubClassof<X, X> == true
 */
template <class T, class U>
struct IsSubClassOf
{
	const static bool Value = IsConvertibleTo<T*, U*>::Value;
};

template <class T, class U>
struct IsSubClassOf<T[], U>
{
	const static bool Value = true;
};

template <class T, class U>
struct IsEqual
{
	static const bool Value = false;
};

template <class T>
struct IsEqual<T, T>

{
	static const bool Value = true;
};

template <class T>
struct IsStdVector
{
	static const bool Value = false;
};

template <class T>
struct IsStdVector<std::vector< T > >

{
	static const bool Value = true;
};


template <class T>
class IsSharedPtr
{
	public:
		const static bool Value = false;
};

template <class T >
class IsSharedPtr<Poco::SharedPtr<T, Poco::ReferenceCounter, precitec::system::NoReleasePolicy<T> > >
{
	public:
		const static bool Value = true;
} ;

template <class T >
class IsSharedPtr<Poco::SharedPtr<T, Poco::ReferenceCounter, precitec::system::ArrayReleasePolicy<T> > >
{
	public:
		const static bool Value = true;
} ;

template <class T >
class IsSharedPtr<Poco::SharedPtr<T> >
{
	public:
		const static bool Value = true;
} ;

template <class T >
class IsSharedPtr<std::shared_ptr<T> >
{
	public:
		const static bool Value = true;
} ;

template <class T, class U>
struct IsSharedPtrFrom
{
	const static bool Value = false;
};

template <class T, class U>
struct IsSharedPtrFrom<Poco::SharedPtr<T>, U >
{
	const static bool Value = IsConvertibleTo<T*, U*>::Value;
};

template <class T, class U>
struct IsSharedPtrFrom<std::shared_ptr<T>, U >
{
	const static bool Value = IsConvertibleTo<T*, U*>::Value;
};

template <class T, class U>
struct IsSharedPtrFrom<Poco::SharedPtr<T, Poco::ReferenceCounter, precitec::system::NoReleasePolicy<T> >, U >
{
	const static bool Value = IsConvertibleTo<T*, U*>::Value;
};

template <class T, class U>
struct IsSharedPtrFrom<Poco::SharedPtr<T, Poco::ReferenceCounter, precitec::system::ArrayReleasePolicy<T> >, U >
{
	const static bool Value = IsConvertibleTo<T*, U*>::Value;
};


template <class T>
class SharedPtrType
{
};

template <class T>
class SharedPtrType<Poco::SharedPtr<T> >
{
	public :
		typedef T Type;
};

template <class T>
class SharedPtrType<std::shared_ptr<T> >
{
	public :
		typedef T Type;
};

template <class T>
class SharedPtrType<Poco::SharedPtr<T, Poco::ReferenceCounter, precitec::system::NoReleasePolicy<T> > >
{
	public :
		typedef T Type;
};

template <class T>
struct SharedPtrType<Poco::SharedPtr<T, Poco::ReferenceCounter, precitec::system::ArrayReleasePolicy<T> > >
{
	typedef T Type;
};

template <class T>
class SharedPtrType<precitec::system::ShMemPtr<T> >
{
	public :
		typedef T Type;
};


template <class T>
struct DeRefPtr {
	typedef T RefType;
};

template <class T>
struct DeRefPtr<T*> {
	typedef T RefType;
};

template <class T, size_t N>
struct DeRefPtr<T[N]> {
	typedef T RefType;
};



template <class T>
struct DeConst {
	typedef T Type;
};

template <class T>
struct DeConst<const T> {
	typedef T Type;
};

// Primary template
template < typename T >
struct CompoundType {
  enum { IsPointer = false, IsReference = false, IsArray = false,
         IsFunction = false, IsPointerToMember = false };
  typedef T base_type;
};

// Partial specialization for references
template < typename T >
struct CompoundType<T&> {
  enum { IsPointer = false, IsReference = true, IsArray = false,
         IsFunction = false, IsPointerToMember = false };
  typedef T base_type;
};

// Partial specialization for pointers
template < typename T >
struct CompoundType<T*> {
  enum { IsPointer = true, IsReference = false, IsArray = false,
         IsFunction = false, IsPointerToMember = false };
  typedef T base_type;
};

// Partial specialization for arrays
template < typename T, size_t N >
struct CompoundType<T[N]> {
  enum { IsPointer = false, IsReference = false, IsArray = true,
         IsFunction = false, IsPointerToMember = false };
  typedef T base_type;
};

// Partial specialization for empty arrays
template < typename T >
struct CompoundType<T[]> {
  enum { IsPointer = false, IsReference = false, IsArray = true,
         IsFunction = false, IsPointerToMember = false };
  typedef T base_type;
};

// Partial specialization for pointers to members
template < typename T, typename C >
struct CompoundType<T C::*> {
  enum { IsPointer = false, IsReference = false, IsArray = false,
         IsFunction = false, IsPointerToMember = true };
  typedef T base_type;
};


template <class T>
struct IsSharedMemPtr {
	const static bool Value = false;
};

template <class T >
struct IsSharedMemPtr<precitec::system::ShMemPtr<T> >	{
	const static bool Value = true;
} ;


template <class T, class U>
struct IsSharedMemPtrFrom	{
	const static bool Value = false;
};

template <class T, class U>
struct IsSharedMemPtrFrom<precitec::system::ShMemPtr<T>, U >	{
	const static bool Value = IsConvertibleTo<T*, U*>::Value;
};

/*
 // sollte nicht mehr benoetigt werden (war vonn access.h verwendet)
template <class T>
struct IsString {
	const static int Value = 0;
};

template <>
struct IsString<std::string>	{
	const static int Value = 2;
};

template <>
struct IsString<char*>	{
	const static int Value = 2;
};
*/
#endif // TEMPLATES_H
