#ifndef _RANGE_H_
#define _RANGE_H_
/**
 * @file
 * @author	Wolfgang Reichl
 * @date 	18.07.06
 */

#include <iostream>
#include <sstream>
#include <algorithm>

#include "Analyzer_Interface.h"
#include "system/typeTraits.h"
#include "system/types.h"

namespace precitec
{
namespace geo2d
{
	/// Intervallklasse
	/** Stellt ein mathematisches Intervall dar. Die Grenzen sind bei allen
	 *  Operationen eingeschlossen.
	 */
	template<class T>
	class TRange {
	public:
		/// Nullintervall [0,0[
		TRange() : start_(T()), end_(T()) {}
		/// Intervall aus unterer und oberer Grenze [start,end[
		TRange(T start, T end) : start_(start), end_(end) {}

		/// Commodity-CTor mit nur Endwert
		TRange(T end) :	start_(T()), end_(end) {}
		TRange(TRange const& rhs) :	start_(rhs.start_), end_(rhs.end_) {}
		void operator =(TRange  const& rhs) {	start_=rhs.start_; end_=rhs.end_; }

		/// erzeugt gueltiges Intervall auch wenn s > e
		static TRange validRange(int s, int e) { return (s<e) ? TRange(s, e) : TRange(e, s); }
		/// Accessoren
		T & start() {	return start_; }
		T const& start() const { return start_; }
		T & end() {	return end_; }
		T const& end() const { return end_;	}


		/// Null-Start-Intervall mit oberer Grenze [0,end[
		//TRange(T end) : start_(0), end_(end)	{}

		bool init(T start, T end);

		/// Prueft ob Intervall korrekt ist start_ <= end_
		bool isValid() const;

		/// Prueft ob Intervall Punkte enthaelt
		bool isEmpty() const;

		/// tauscht Intervallgrenzen, falls Intervall ungueltig
		TRange makeValid() const;

		/// gibt den maximalen erlaubten Wert an, Intetvall muss gueltig sein
		T maxValue() const;

		/// Prueft ob ein Wert im Intervall liegt
		bool contains(T value) const;

		/// Prueft ob ein anderes Intervall vollstaendig im Intervall liegt
		bool contains(TRange r) const;

		/// vergroessert das Intervall beidseitig
		TRange dilate(T border) const;
		/// vergroessert Intervall symmetrisch um border Pixel

		/// verkleinert das Intervall beidseitig
		TRange erode(T border) const;
		TRange close(T border) const;

		/// skaliert Anfang und Ende des Intervalls
		TRange scale(double scaleFaktor);
		void swap(TRange &r) { std::swap(start_, r.start_); std::swap(end_, r.end_); }

		/// gibt Laenge des Intervalls zurueck
		T length() const;

		std::string toString() const;

		/// interpolation 0->start 1.0->end
		double interpolate(double factor) const;

		/// liefert die Intervall-Mitte
		T center() const;

		/// projeziert Wert auf Intervall
		T project(T value) const;
		/// projeziert anderes Intervall auf Intervall
		TRange<T> project(TRange<T> range) const;

		TRange operator+=(T shift);
		TRange operator-=(T shift);

	public:
		T start_; ///< untere Intervallgrenze inklusiv
		T end_; ///< obere Intervallgrenze inklusiv

	}; // TRange


	// -----------------------------------------------------------------------------
	// ------------------------------ Funktionen -----------------------------------
	// -----------------------------------------------------------------------------

	/// Gleichheit
	/** ein ungueltiges Intervall vergleicht - aehnich wie NAN -
	 *  immer false
	 */
	template<class T>
	bool operator==(const TRange<T>& r1, const TRange<T>& r2);

	/// Ungleichheit
	/** ein ungueltiges Intervall vergleicht - aehnich wie NAN -
	 *  immer false
	 */
	template<class T>
	bool operator!=(const TRange<T>& r1, const TRange<T>& r2);

	template<class T>
	std::ostream& operator<<(std::ostream& os, const TRange<T>& r);

	template<class T>
	std::istream& operator>>(std::istream& is, TRange<T>& r);

	/**
	 * Vereinigt zwei TRanges zu Einer
	 * Dabei gilt: newStart = min( r1.start, r2.start )
	 *  		   newEnd   = max( r1.end  , r2. end  )
	 * Return Vereinigungsmenge (existiert immer)
	 */
	template<class T>
	TRange<T> operator|(const TRange<T>& r1, const TRange<T>& r2);

	/// Schnittmenge
	/** Liefert den Schnitt aus zwei TRanges, sofern er existiert.
	 *  Existiert kein Schnitt, liefert die Funktion eine NullTRange zurueck.
	 *  Return Schnittmenge (NullTRange, wenn kein Schnitt existiert)
	 */
	template<class T>
	TRange<T> operator&(const TRange<T>& r1, const TRange<T>& r2);

	template <class T>
	TRange<T> operator +( const TRange<T>& r1, TRange<T> const& r2 );

	template<class T>
	TRange<T> operator +(const TRange<T>& r, T offset);

	template<class T>
	TRange<T> operator +(T offset, const TRange<T>& r);

	template <class T>
	TRange<T> operator-( const TRange<T>& r1, TRange<T>const& r2 );

	template<class T>
	TRange<T> operator -(const TRange<T>& r, T offset);

	template<class T>
	TRange<T> operator -(T offset, const TRange<T>& r);


	/**
	 * Beschneidet einen Wert auf ein Intervall
	 * Intervall ist oben geschlossen!!!
	 * Dass T != U ist, soll es (wf expliziter casts ohne Warnungen) erlauben,
	 * dass ein Int etwa auf eine UInt-Range geclampt wird.
	 *
	 * @param value Der zu beschneidende Wert
	 * @param TRange Das Interval auf das der Wert benitten werden soll
	 */
	template<class T, class U>
	T clamp(T value, TRange<U> const& range) {
		if (U(value) < range.start()) {
			value = T(range.start());
		} else if (U(value) > range.end()) {
			value = T(range.end());
		}
		return value;
	} // clamp

	typedef TRange<int> Range;
	typedef TRange<uInt> Range1u;
	typedef TRange<float> Range1f;
	typedef TRange<double> Range1d;
} // namespace geo2d

	template<> struct No<geo2d::TRange<int> > {
		static const geo2d::TRange<int> Value;
	};

	//template <> struct No<geo2d::Range>  { static const geo2d::Range Value; };
	template<> struct No<geo2d::Range1u> {
		static const geo2d::Range1u Value;
	};
	template<> struct No<geo2d::Range1f> {
		static const geo2d::Range1f Value;
	};
	template<> struct No<geo2d::Range1d> {
		static const geo2d::Range1d Value;
	};

	#include "range.hpp"

} // namespace precitec

#endif //_Range_H_
