#ifndef CAMTYPEDPROPERTY_H_
#define CAMTYPEDPROPERTY_H_

#include "camProperty.h"

namespace precitec
{
namespace ip
{

	/**
	 * abstrakte Basisklasse für alle realen Properties
	 * hat bereits einen Typ aber noch keinen Wert,
	 * dies erleubt es Basisklassen die Werte auf eigene Weise
	 * zu Speichern, oder u.U. zu berechnen
	 * 
	 * bei AutoRange werden die Extremwerte aud der Kamera eingelesen
	 * AutoRange ist nich nicht implelentiert!!
	 */	
	template<class T, bool AutoRange=false>
	class TypedProperty : public Property
	{
	 public:
		 /// Konstruktor mit fester Kamera
		 TypedProperty(std::string name, BaseCamera & camera);
		 TypedProperty(std::string name);
		 /// copy-CTor
		 virtual ~TypedProperty() {}
	 public:
		 /// Konversion zu Basis-Typ, gecacht
		 operator T();
		 /// Zuweisung auf Property
		 //TypedProperty<T> operator = (T value);
		 //void operator = (T value);
		 /// Wertzuweisung mit Kamera
		 T get(BaseCamera const& camera);
		 /// Wertabfrage mit Caching mit Caching und optionaler Kamera
		 bool set(T value, BaseCamera const& camera);
		
	 	//BaseCamera const * camera() const {return camera_; } 
		 
	 protected:
		 /// innerer Cache-Zugriff 
		 virtual T  value() const { return No<T>::Value; }
		 /// min-wert fuer Range-Abfrage 
		 virtual T  minValue() { return No<T>::Value; }
		 /// max-wert fuer Range-Abfrage 
		 virtual T  maxValue() { return No<T>::Value; }
		 /// muss ueberschrieben werden
		 virtual void setValue(T value) {}
		 /// bei festem intervall, Ftk. ok, ohne interrvall Ftk mit false ueberschreibe, bei AutoRange Gueltigkeitsabfrage
		 virtual bool isRangeValid() { return !AutoRange; }
		 /// legt fuer AutoRange ungueltige Range an
		 virtual void invalidateRange() {}
	 protected:
		 virtual operator std::string ();
		 /// liest Cache neu e.g. wenn  Kamera ausgefallen war
//		 virtual void flush(BaseCamera const& camera) { valueCurrent_ = false; invalidateRange(); read(camera); }
		 /// hier wird ungecacht mittels pf... von Kamera gelesen
		 bool read(BaseCamera const& camera);
		 /// hier wird ungecacht mittels pf... auf Kamera geschrieben
		 bool write(BaseCamera const& camera, T value);
		 /// Konkretisierung erfolgt ggf. in abgeleiteter Klasse
		 protected:
		 virtual bool isValueInRange();
	 private:
		 TypedProperty(TypedProperty const&prop);
	}; // TypedProperty

	/** 
	 * hiervon sind die meisten einfachen Properties abgeleitet
	 * die Klasse haelt den Wert, die abgeleitete Klasse muss 
	 * nur noch den Namen definieren, und ggf. die Extremwerte initialisieren
	 */
	template<class T, bool AutoRange>
	 class ValProperty : public TypedProperty<T, AutoRange>
	{
	 public:
		 /// Konstruktor mit fester Kamera
		 ValProperty(std::string name, BaseCamera & camera, T value=No<T>::Value);
		 ValProperty(std::string name, T value=No<T>::Value);
		 void operator = (T value) { write(*camera_, value); }
	 protected:
		 /// innerner Cache-Zugriff 
		 virtual T value() const { return value_; }
		 void setValue(T value) { value_ = value; }
	 protected:
		 T value_;
		 T minValue_;
		 T maxValue_;
		 bool initRangeOnNextAccess_;
	}; // ValProperty

	typedef ValProperty<int, false> IntProperty;
	typedef ValProperty<float, false> FloatProperty;


//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------





	
	/**
	 * gecachtes lesen
	 * falls Property invalid (e.g. kein Name)
	 * oder Wert volatile (von Kamera generierter Wert (BildZahl)
	 * wird der Wert von Kamera gelesen
	 * falls irgendetwas scheiter ist der Wert invalid
	 */
	template <class T, bool AutoRange>
	 TypedProperty<T, AutoRange>::operator T() 
	 { 
	 	
	 	return isValueCurrent() ? value() : isConnected(camera_) ? get(*camera_) : No<T>::Value; 
	 }
	
	template <class T, bool AutoRange>
	 T TypedProperty<T, AutoRange>::get(BaseCamera const& camera)
	 {
		 //std::cout << "TypedProperty::get: " << std::endl;
		 bool ok = true; // kann nur bei read zu false gesetzt werden
		 if (isVolatile() || !isValueCurrent() || !isValueInRange()) ok = read(camera);
//		 std::cout << "TypedProperty::get: post read: " << value() << std::endl;
		 return ok ? value() : No<T>::Value;
	 }
	
	template <class T, bool AutoRange>
	 bool TypedProperty<T, AutoRange>::set(T val, BaseCamera const& camera)
	 {
		 //std::cout << "TypedProperty::set: " << std::endl;
		 bool ok = true; // kann nur bei write zu false gesetzt werden
		 if (isVolatile() || val!=value()) ok = write(camera, val);
		 return ok;
	 }

	template <class T, bool AutoRange>
	TypedProperty<T, AutoRange>::operator std::string () { 
		//std::cout << "TypedProperty::operator std::string " << std::endl;
		if (isValueCurrent()) 
		{
			return std::string();// std::string s; toString(s, value()); return s;
		}
		else
		{
//			std::cout << "camera_: " << camera_ << std::endl;
			validateAccess(camera_);
			if (!isHandleValid() ) return "unconnected";
			return camera_->propertyToString(handle_); 
		}
	}
 
	
 	/**
	 * ungecachtes Lesen eines T-Wertes von der Kamera.
	 * Wird von allen abgeleiteten Klassen verwendet.
	 * Rueckgabewert ist: Operation gelungen.
	 * Wenn das Kamera-Objekt ungueltig ist
	 * oder kein gueltiges Token/Handle erzeugt wird
	 * oder die pf-Funktion einen Fehler bringt, scheitert write.
	 * Die Handle wird nicht gecacht.
	 */
	template <class T, bool AutoRange>
	 bool TypedProperty<T, AutoRange>::read(BaseCamera const& camera)
	 {
//		 std::cout << "TypeProp::read<T>: ";
		 validateAccess(&camera);
		 if (!isHandleValid()) return false;
		 T val;
		 valueCurrent_ = ( camera.getProperty(handle_, val) == 0);
		 setValue(val);
//		 std::cout << "TypedProp::read post camera.setProperty: " << value() << std::endl;
		 return valueCurrent_;
	 }
	
	
 	/**
	 * ungecachtes Schreiben eines T-Wertes auf der Kamera.
	 * Wird von allen abgeleiteten Klassen verwendet.
	 * Rueckgabewert ist: Operation gelungen.
	 * Wenn das Kamera-Objekt ungueltig ist
	 * oder kein gueltiges Token/Handle erzeugt wird
	 * oder die pf-Funktion einen Fehler bringt, scheitert write.
	 * Die Handle wird nicht gecacht.
	 */
	template <class T, bool AutoRange>
	 bool TypedProperty<T, AutoRange>::write(BaseCamera const& camera, T value)
	 {
		 //std::cout << "TypeProp::write<T>: " << value << std::endl;
		 validateAccess(&camera);
		 if (!isHandleValid()) return false;
		 //std::cout << "\t\t handle " << handle_ << " valid ";
		 valueCurrent_ = (camera.setProperty(handle_, value) == 0);
		 //std::cout << (valueCurrent_ ? "ok" : "nok") << std::endl;
		 // wenn Property setzen geklappt hat neuen Wert in Cache übernehmen
		 if (valueCurrent_) setValue(value);
		 return valueCurrent_;
	 }

	/**
	 * Konkretisierung erfolgt ggf. in abgeleiteter Klasse
	 * Jedes Property hat u.U. eigene Gueltigkeitsbereiche.
	 * Hier wird der T-Default gesetzt.
	 * Wg include-Problem mit <limits> noch nicht aktiv
	 */

	template <class T, bool AutoRange>
	 bool TypedProperty<T, AutoRange>::isValueInRange() 
	 { 
		 return value() != No<T>::Value;
	 }

	
	/**
	 * Konstruktor mit Kamera, mit Wert
	 * mit  Kamera: ein Property fuer eine Kamera
	 *    sollte der Normalfall sein
	 * mit  Wert: fuer 'konstante' Werte sinnvoll
	 */
	template <class T, bool AutoRange>
	 TypedProperty<T, AutoRange>::TypedProperty(std::string name, BaseCamera & camera) 
		: Property(name, PfType<T>::Value, &camera)
	{
		//std::cout << "CTor0 TypedProperty: " << value() << T(0.3) << std::endl;
		// neuen Wert auf Kamera (so vorhanden) schreiben
	}
	
	template <class T, bool AutoRange>
	 TypedProperty<T, AutoRange>::TypedProperty(std::string name) 
	 	: Property(name, PfType<T>::Value, NULL)
	{
		 valueCurrent_ = false;
		//std::cout << "CTor1 TypedProperty: " << value() << T(0.3) << std::endl;
	}
	
	
	/**
	 * Konstruktor mit Kamera, mit Wert
	 * mit  Kamera: ein Property fuer eine Kamera
	 *    sollte der Normalfall sein
	 * mit  Wert: fuer 'konstante' Werte sinnvoll
	 */
	template <class T, bool AutoRange>
	 ValProperty<T, AutoRange>::ValProperty(std::string name, BaseCamera &camera, T val) 
	 	: TypedProperty<T, AutoRange>(name, camera), value_(val)
	{
		//std::cout << "CTor0 ValProperty: " << value() << T(0.3) << std::endl;
		 valueCurrent_ = false;
	 
		bool ok = true;
		if (isHandleValid() && camera.isValid() && val!= No<T>::Value) { ok = write(*camera_, value()); }
		valueCurrent_ = ok;
		
	}
	
	template <class T, bool AutoRange>
	 ValProperty<T, AutoRange>::ValProperty(std::string name, T value) 
	 	: TypedProperty<T, AutoRange>(name), value_(value)
	{
		//cout << "CTor0 ValProperty: " << value() << T(0.3) << std::endl;
		valueCurrent_ = false;
	}
	
	
} // namesapce ip
} // precitec


#endif /*CAMTYPEDPROPERTY_H_*/
