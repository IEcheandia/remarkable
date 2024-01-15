#ifndef RING_H_
#define RING_H_
#pragma once

/**
 *  math::ring.h
 *	@file
 *	@copyright      Precitec Vision GmbH & Co. KG
 *	@author         Wolfgang Reichl (WoR)
 *	@date           30.09.2011
 *	@brief					Ring stellt einen funktionierenden Modulo-Zaehler zur Verfuegung
 */

#include <assert.h>
#include "system/types.h"

namespace precitec
{
namespace math
{

	/**
	 *  Klasse fuer korrekte Moduloarithmetik bei Kompilezeit-Modulo
	 *  Es stehen Grundoperatoren +, - und * zur Verfuegung. Increment ++ und Dekre-
	 *  ment -- sind vorhanden.
	 */
	/*
	template <int Modulo>
	class TRing {
		/// Correktoren sorgen dafuer, dass der Wert den TRing nicht ueberschreitet
		typedef void (*Corrector)();
	public:
		/// Default-CTor
		TRing() : value_(0) {}
		/// copy-CTor (nur von gleichem TRing)
		TRing(TRing const& r) :	value_(r.value_) {}
		/// CTor mit Wert und dem Modul, Wert muss passen
		TRing(uInt value) : value_(value) { assert(value<Modulo);	}

	private:
		/// CTor mir Korrektor, Resultat liegt garantiert in [0..TRing)
		TRing(uInt v0, Correcter corrector) : value_(v0) { corrector(); }

	public:
		///  Accessor modulo
		//static uInt size() const { return Modulo; }
		/// Typecastoperator
		operator uInt() const {	return value_; }
		/// fuer std-Abfragen
		operator bool() const {	return value_!=0; }

	public:
		/// Wertzuweisung
		void operator=(TRing r) {	value_ = r.value_; }
		/// Wertzuweisung
		void operator=(uInt value) { assert(value<Modulo); value_ = value; }

		/// Pre-Inkrement Operator
		TRing& operator++() {	++value_; topCheck(); return *this; }

		/// Post-Inkrement Operator
		TRing& operator++(int) { ++value_; topCheck(); return *this; }

		/// Pre-Dekrement Operator
		TRing& operator--() {	--value_; botCheck(); return *this; }

		/// Post-Dekrement Operator
		TRing& operator--(int) { --value_; botCheck(); return *this; }


		/// Addition
		inline friend	TRing operator+(TRing const& r, uInt v) { assert(v<Modulo); return TRing(r.value_+v, topCheck); }
		/// Addition
		inline friend	TRing operator+(uInt v, TRing const& r) { assert(v<Modulo); return TRing(r.value_+v, topCheck); }
		/// Addition
		void operator+=(uInt v) { value_+=v; topCheck(); }

		/// Subtraktion
		inline friend	TRing operator-(TRing const& r, uInt v) { assert(v<Modulo); return TRing(r.value_-v, botCheck); }
		/// Subtraktion
		inline friend	TRing operator-(uInt v, TRing const& r) { assert(v<Modulo); return TRing(v-r.value_, botCheck); }
		/// Subtraktion
		void operator-=(uInt v) { value_-=v; botCheck(); }

	private:
		/// value liegt garantiert in [0..2*modulo_)
		void topCheck() { value_ -= (value_>Modulo) ? Modulo : 0;	}
		/// value liegt garantiert in [-modulo..modulo)
		void botCheck() {	value_ += (int(value_)<0) ? Modulo : 0; }
	private:
		uInt value_; ///< Zahlenwert
	};
*/
	/**
	 *  Klasse fuer korrekte Moduloarithmetik bei Laufzeit-Modulo
	 *  Es stehen Grundoperatoren +, - und * zur Verfuegung. Increment ++ und Dekre-
	 *  ment -- sind vorhanden.
	 */
	class Ring {
	private:
		/// Correktoren sorgen dafuer, dass der Wert den TRing nicht ueberschreitet
		typedef void (*Corrector)(void);
		/// value liegt garantiert in [0..2*modulo_)
		void topCheck() {
			value_ -= (value_>=modulo_) ? modulo_ : 0;
		}
		/// value liegt garantiert in [-modulo..modulo)
		void botCheck() {	value_ += (int(value_)<0) ? modulo_ : 0; }
	public:
		/// Default-CTor
		Ring() : modulo_(1), value_(0) {}
		/// copy-CTor
		Ring(Ring const& r) :	modulo_(r.modulo_), value_(r.value_) {}
		/// CTor mit Wert und dem Modul, Wert muss passen
		Ring(uInt size, uInt value) : modulo_(size), value_(value) { assert(size>0), assert(value<modulo_);	}
		/// CTor von TRing
		//template <int Modulo>
		//Ring(TRing<Modulo> r) :	modulo_(Modulo), value_(r.value_) {}

	private:
		/// CTor mir Korrektor, Resultat liegt garantiert in [0..TRing)
		Ring(uInt size, uInt v0, Corrector &corrector) : modulo_(size), value_(v0) { corrector(); }
		/// CTor mit Wert und dem Modul, Wert muss passen
		Ring(uInt size, uInt value, bool goingUp) : modulo_(size), value_(value) { assert(size>0), assert(value<modulo_);	goingUp?topCheck():botCheck();}

	public:
		///  Accessor modulo
		//uInt size() const { return modulo_; }
		/// Typecastoperator (erspart Vergleichsoperatoren, da direkt mit uInt verglichen werden kann)
		operator uInt() const {	return value_; }
		/// fuer std-Abfragen
		operator bool() const {	return value_!=0; }

	public:
		/// Wertzuweisung
		void operator=(const Ring& r) {	modulo_ = r.modulo_; value_ = r.value_; }
		/// Wertzuweisung
		void operator=(uInt value) {	assert(value<modulo_); value_ = value; }

		/// Pre-Inkrement Operator
		Ring& operator++() {	++value_; topCheck(); return *this; }
		/// Post-Inkrement Operator
		//Ring& operator++(int) {	++value_; topCheck(); return *this; }

		/// Pre-Dekrement Operator
		Ring& operator--() {	--value_; botCheck(); return *this; }
		/// Post-Dekrement Operator
		//Ring& operator--(int) {	--value_; botCheck(); return *this; }


		/// Addition
		inline friend	Ring operator+(Ring const& r, uInt v) {assert(v<r.modulo_); return Ring(r.modulo_, r.value_+v, true); }
		/// Addition
		inline friend	Ring operator+(uInt v, Ring const& r) { assert(v<r.modulo_); return Ring(r.modulo_, r.value_+v, true); }
		/// Addition
		void operator+=(uInt v) { value_+=v; topCheck(); }

		/// Subtraktion
		inline friend	Ring operator-(Ring const& r, uInt v) { assert(v<r.modulo_); return Ring(r.modulo_, r.value_-v, false); }
		/// Subtraktion
		inline friend	Ring operator-(uInt v, Ring const& r) { assert(v<r.modulo_); return Ring(r.modulo_, v-r.value_, false); }
		/// Subtraktion
		void operator-=(uInt v) { value_-=v; botCheck(); }



	private:
		uInt modulo_;  ///< Modulo-Wert
		uInt value_; ///< Zahlenwert
	};

} // namespace math
} // namespace precitec

#endif // RING_H_
