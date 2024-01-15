/** \file 	Range.cpp
 *  \author	Wolfgang Reichl
 *  \date 	02.08.06
 */


#include "system/types.h"
#include "system/typeTraits.h"


namespace geo2d
{
	/**
	 * \return true wenn start_ <= end_, false sonst
	 * ein Leeres Intervall ist gueltig
	 */
	template <class T>
	bool TRange<T>::isValid() const
	{
		return ( start() <= end() );
	}

	/**
	 * \return Range(min(a, e), max(a, e)), Intervall ist immer gueltig
	 * Ausnahme wenn T=double und a oder e NaN
	 */
	template <class T>
	TRange<T> TRange<T>::makeValid() const
	{
		return ( start() <= end() ) ? TRange<T>(*this) :  TRange<T>(end(), start());
	}

	/**
	 * \return enthaelt Intervall keine Punkte
	 * Hier sei definiert ein  ungueltiges Intervall enthaelt keine Punkte
	 */
	template <class T>
	bool	TRange<T>::isEmpty() const {
		return start() >= end();
	}

	/**
	 * \return enthaelt Intervall keine Punkte -> No<T>::Value sonst obere inklusive Grenze
	 * Hier sei definiert ein  ungueltiges Intervall enthaelt keine Punkte
	 */
	template <class T>
	T	TRange<T>::maxValue() const {
		return isEmpty() ? No<T>::Value :  end();
	}

	/**
	 * Achtung!!! geschlossenes Intervall; beide Intervallgernzen sind inklusiv
	 * \return true wenn start <= value < end, false sonst
	 */
	template <class T>
	bool TRange<T>::contains( T value )  const {
		return ( start() <= value) && (value <= end() );
	}

	/**
	 * ein Intervall enthaelt sich selbst
	 * \return true wenn r vollstaendig in TRange enthalten
	 */
	 template <class T>
	bool TRange<T>::contains(TRange<T> r) const	{
		return (start()<=r.start()) && (end()>=r.end());
	}

	template <class T>
	std::string TRange<T>::toString() const {
		std::stringstream ss;
		ss << "[" << start() << ".." << end() << "]";
		return ss.str();
	}

	/**
	 * \return neues Intervall ggf. leeres/ungueltiges Intervall
	 * \param border sollte positive sein (ansonst ist dilate ein "erode")
	 * dilate erhaelt ungueltiges Intervall
	 * dilate erzeugt aus gueltigem Intervall nie ungueltiges Intervall, bleibt
	 * ggf. (neg. border) in der Mitte stehen
	 */
	template <class T>
	TRange<T> TRange<T>::dilate(T border) const {
		if ((!isValid()) || (border==0))
			return *this;
		else if (start()-2*border < end())
			return TRange(start()-border, end()+border);
		else
			return TRange(center(), center());
	}

	/**
	 * Spezialisierung fuer streng positiven Typ UInt
	 * Da Rand>0 ist, wird hier erodiert, aber dafuer wird bei 0 geklammert
	 */
	 /*
	template <>
	TRange<UInt> TRange<UInt>::dilate(UInt border) const
	{
		if ((!isValid()) || (border==0)) return *this;
		return TRange<UInt>(start() > border	? start() - border : 0,   end + border);
	}
	*/

	/**
	 * \return neues Intervall ggf. leeres/ungueltiges Intervall
	 *	\param border sollte positiv sein (ansonst ist close ein "open")
	 * erode erhaelt ungueltiges Intervall
	 * erode erzeugt aus gueltigem Intervall nie ungueltiges Intervall, bleibt
	 * ggf. (bos. border) in der Mitte stehen
	 */
	template <class T>
	TRange<T> TRange<T>::erode(T border) const
	{
		if ((!isValid()) || (border==0))
			return *this;
		else if (start()+2*border < end())
			return TRange(start()+border, end()-border);
		else
			return TRange(center(), center());
	}

	template <class T>
	TRange<T> TRange<T>::close(T border) const
	{
	//#warning function TRange::close(b) is obsolete, use range.erode(b)
		if (border==0) return *this;
		else return erode(border);
	}

	/**
	 * \return neues Intervall ggf. leeres/ungueltiges Intervall
	 *	\param Skalierungs-Faktor
	 * negativer Faktor macht ein gueltige Intervall ungueltig und umgekehrt
	 */
	template <class T>
	TRange<T> TRange<T>::scale(double iScaleFaktor)
	{
		return TRange(T(start()* iScaleFaktor), T(end()*iScaleFaktor));
	}

	/**
	 * \return bei Integer Anzahl der Punkte im Intervall, bei float echte Laenge
	 */
	template <class T>
	T TRange<T>::length() const
	{
		return isValid() ? end() - start() : No<T>::Value;
	}

	/**
	 * \return mittlerer Punkt des Intervalls
	 */
	template <class T>
	T	TRange<T>::center() const
	{
		return !isValid() ? No<T>::Value : isEmpty() ? start() :  (start() + maxValue()) / 2;
		// return interpolate(0.5);
	}

	/**
	 * \return Position in Intervall mit relativem Abschnitt factor
	 *	\param wenn Factor in [0..1] liegt wird interpoliert, sonst extrapoliert
	 */
	template <class T>
	double	TRange<T>::interpolate(double factor) const
	{
		return !isValid() ? No<double>::Value : start()  +  factor * length();
	}

	/**
	 * \param beiliebiger Wert
	 * \return naechster Punkt im intervall
	 */
	template <class T>
	T	TRange<T>::project(T value) const
	{
		return (value < start()) ? start() : (value > maxValue()) ? maxValue() : value;
	}

	/**
	 * \param beliebiges Intervall
	 * \return Schnitt-Intervall der beiden Intervalle
	 */
	template <class T>
	TRange<T>	TRange<T>::project(TRange<T> range) const
	{
		return TRange<T>(project(range.start()), project(range.end()));
	}

	/**
	 * \param Verschiebewert: positiv schiebt rechts
	 * verschiebt Intervall selbst
	 */
	template <class T>
	TRange<T> TRange<T>::operator+=(T shift)
	{
		start() += shift;
		end()	+= shift;
		return *this;
	}

	/**
	 * \param Verschiebewert: positiv schiebt links
	 * verschiebt Intervall selbst
	 */
	template <class T>
	TRange<T> TRange<T>::operator-=(T shift)
	{
		start() -= shift;
		end()		-= shift;
		return *this;
	}


	//---------------------------------------------------------------------------
	//---------------------------------------------------------------------------
	//---------------------------------------------------------------------------
	//  externe Funktionen

	template <class T>
	bool operator==( const TRange<T>& r1, const TRange<T>& r2 )
	{
		return   r1.isValid() && r2.isValid()
						 && (r1.start() == r2.start() )
						 && (r1.end()   == r2.end() );
	}

//	template <>
//	bool operator==( const TRange<float>& r1, const TRange<float>& r2 );
//	template <>
//	bool operator==( const TRange<double>& r1, const TRange<double>& r2 );
//
	/*
	template <class T>
	Bool equal( const TRange<T>& r1, const TRange<T>& r2, T epsilon )
	{
		return   r1.isValid() && r2.isValid()
			 && equal(r1.start()  r2.start(), epsilon)
			 && equal(r1.end,   r2.end, 	epsilon);
	}
*/

	template <class T>
	bool operator!=( const TRange<T>& r1, const TRange<T>& r2 )
	{
		return !(r1==r2);
	}


	template <class T>
	std::ostream& operator <<( std::ostream& os, const TRange<T>& r )	{
		os << "[" << r.start_ << "," << r.end_ << "]";
		return os;
	}



	template <class T>
	std::istream& operator >>( std::istream& is, TRange<T>& r )	{
		is.ignore(5, '['); is >> r.start_;
		is.ignore(5, ','); is >> r.end_;
		is.ignore(5, ']');
		return is;
	}

	template <class T>
	TRange<T> operator |( const TRange<T>& r1, TRange<T> const& r2 )
	{
		return TRange<T>(std::min(r1.start(), r2.start()), std::max(r1.end(), r2.end()));
	}

	template <class T>
	TRange<T> operator&( const TRange<T>& r1, TRange<T>const& r2 )
	{
		return TRange<T>(std::max(r1.start(), r2.start()), std::min(r1.end(), r2.end()));
	}

	/**
	 * \param Verschiebewert: positiv schiebt rechts
	 * \return verschobenes Intervall
	 */

	template <class T>
	TRange<T> operator +(const TRange<T>& r1, TRange<T> const& r2)
	{
		return TRange<T>(std::min(r1.start(), r2.start()), std::max(r1.end(), r2.end()));
	}

	template <class T>
	TRange<T> operator+(TRange<T> const& r, T shift)
	{
		return TRange<T>(r.start()+shift, r.end()+shift);
	}

	template <class T>
	TRange<T> operator+(T shift, TRange<T> const& r)
	{
		return TRange<T>(r.start()+shift, r.end()+shift);
	}


	/**
	 * \param Verschiebewert: positiv schiebt links
	 * \return verschobenes Intervall
	 */

	template <class T>
	TRange<T> operator-( const TRange<T>& r1, TRange<T>const& r2 )
	{
		return TRange<T>(max(r1.start(), r2.start()), min(r1.end(),   r2.end()));
	}

	template <class T>
	TRange<T> operator-(TRange<T> const& r, T shift)
	{
		return TRange<T>(r.start()-shift, r.end()-shift);
	}

	template <class T>
	TRange<T> operator-(T shift, TRange<T> const& r)
	{
		return TRange<T>(r.start()-shift, r.end()-shift);
	}


}; // namespace geo2d

