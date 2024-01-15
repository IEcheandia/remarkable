#ifndef CAM_PROPERTY_H
#define CAM_PROPERTY_H


#include <iostream>
#include <string>
#include <list>

//#include "system/types.h"
//#include "system/typeTraits.h"
#include "pfCamera.h"
#include "pf/photonFocus.h"

namespace precitec
{

namespace ip
{
	template <class T>
	struct PfType
	{
		static const PropertyType Value = PF_INVALID;
	};
	template <> struct PfType<int>   { static const PropertyType Value = PF_INT;   };
	template <> struct PfType<float> { static const PropertyType Value = PF_FLOAT; };
	template <> struct PfType<bool>  { static const PropertyType Value = PF_BOOL;  };
	template <> struct PfType<std::string> { static const PropertyType Value = PF_STRING; };
	

	/** 
	 * Wrapper um Pfoton-Focus Property
	 * umfasst eine einzige Eigenschaft
	 * anders als Property, das z.B. mit 
	 * ROI auch mehrere PfProperties
	 * zusammenfassen kann
	 */
	
	/**
	 * Basisklasse fuer die konkreten Eigenschaften
	 */
	class Property
	{
	 public:
		 /// std-CTor
		 Property(std::string name, PropertyType type=PF_INVALID);
		 /// std-CTor
		 Property(std::string name, PropertyType type, BaseCamera * camera);
		 /// wrapper-CTor
		 Property(Camera &cam, pf::Handle prop);
		 virtual ~Property() {}
	 public:
		 /// gibt den Typ der Eigenschaft wieder
		 int  type() const { return type_; }
		 /// Name der Eigenschaft
		 std::string const& name() const  { return name_; }
		 /// liest Cache neue.g. wenn Kamera ausgefallen war
//		 void flush() { flush(*camera_); }

		 // Basis-Klasse hat nur String-Interface zu Werten
		 // dieses Interface ist nicht gecacht 

		 /// ist Property gehasht oder hat zumindest Namen
		 bool isValid() 	 const { return isHandleValid() || !name_.empty(); }
		 bool isWritable() { int f=flag(); return (f & F_WO) || !(f & (F_WO|F_RW)); }
		 bool isActive() 	 { int f=flag(); return !(f & F_INACTIVE); }
		 
		 /// Zuweisungen
		 Property &operator = (std::string const& value);
		 operator int() { std::string value; read(camera_, value); return atoi(value.c_str()); }
		 operator float() { std::string value; read(camera_, value); return atof(value.c_str()); }
		 //friend std::string operator << (std::string& str, Property & property);
		 //friend Property& operator << (Property& prop, std::string const& value);
		 friend std::ostream& operator << (std::ostream &os, Property &p);
	 protected:
	 	 int flag(){ validateAccess(camera_); return isHandleValid() ?  camera_->flags(handle_) : 0; }
		 virtual operator std::string ();
		 /// liest Cache neue.g. wenn  Kamera ausgefallen war, da BaseProperty nicht cacht, tut das hier nichts
//		 virtual void flush(BaseCamera const& camera) { }
		 /// validate all handles
		 pf::Handle validateAccess(BaseCamera const* camera);
		 
		 /// String-Wertzuweisung e.g. falls Typ unbekannt
		 bool write(BaseCamera const* camera, std::string const& value);
		 /// String-Wertabfrage e.g. falls Typ unbekannt
		 bool read(BaseCamera const* camera, std::string & value);
	 protected:
		 /// e.g: Bildnummer wird dynamisch von Kamera veraendert
		 virtual bool isVolatile() { return false; }
		 /// ist Wert in-Range
		 virtual bool isValueInRange() { return true; }
		 /// muss Wert von Kamera gelesen werden
		 bool isValueCurrent() { return valueCurrent_; }
		 /// hat die Kamera diese Eigenschaft
		 bool isImplementedBy(BaseCamera const& camera) const;
		 /// ist die Property-Handle ok
		 bool isHandleValid() const;
		 /// ist Property mit Kamera verbunden
		 bool isConnected(BaseCamera const* camera) const;
		 
	 protected:
		 std::string const  name_;
		 int                type_;
		 BaseCamera  *      camera_;
		 pf::Handle         handle_;
		 bool               valueCurrent_;
	private:
	//	Property(Property const& );
		Property &operator = (Property const& );
	}; // Property

	/// Zuweisung auf String
	//std::string operator << (std::string& str, Property & property);
	/// Zuweisung von String
	//Property& operator << (Property&  prop, std::string const& value);
	
	std::ostream &operator << (std::ostream &os, Property &p);
	
	/// Auslesen ueber String ist generisch
	//std::string operator = (std::string value, Property const& property) ;
	/// Einlesen ueber String ist generisch
	//std::string operator = (Property property, std::string const& value);
	
	 /**
	  *  eine Klasse die nur dazu dient eine spezifische 
	  * Template-Spezialisierung zu erzwingen
	  */	
	 class CommandParam {};
	 
	/** 
	 * hiervon sind die meisten einfachen Properties abgeleitet
	 * die Klasse haelt den Wert, die abgeleitete Klasse muss 
	 * nur noch den Namen definieren, und ggf. die Extremwerte initialisieren
	 */
	 
	 class Command : public Property
	{
	 public:
		 /// Konstruktor mit fester Kamera
		 Command(std::string name, BaseCamera & camera) : Property(name, PF_INT, &camera) {}
		 Command(std::string name) : Property(name, PF_INT) {}
	 public:
		 /// ausführung des Proramms 
		 bool execute() 
		 { 
			 validateAccess(camera_);
			 if (!isHandleValid()) return false;
			 CommandParam param; // wer weiss, ob er nicht doch, benutzt wird
			 return (camera_->setProperty(handle_, param) == 0);
		 }
	 protected:
	 private:
		 Property &operator = (std::string const& value) { execute(); return *this; }
		 Property &operator = (int value) { execute(); return *this; }
	}; // Command

	
/*		 
		 
		void saveConfig(int iOffset,int iSkimValue,int iLinLog1,int iLinLog2,int iTeiler);
		bool isOk(void) const { return valid_; }
		int  importSettings(const char *filename, int& endCounter);
		int  exportSettings(const char *filename, int& endCounter) const;
	
	
	
		void setExposure(int exposure);
		int  getExposure(void) const;
	
		int  setSkimValue(int iSkimValue, int min_dacval);
		int  getSkimValue(int min_dacval) const;
	
		int  setOffset(int iOffset);
		int  getOffset(void) const;
	
		int  setFrameTime(int iFrameTime);
		int  getFrameTime(void) const;
	
		void reset(void);
		int  setExposureMode( int flag);
		int  setSkimFlag(int flag);
		int  setLinLog(int linlog1,int linlog2,int percent);	
		int  getLinLog(int &linlog1,int &linlog2,int &percent) const;

		int  setRotateFlag(int flag);
		int  setConstFrameRate(int flag);
		int  getSkimFlag(void) const;
		int  getRotateFlag(void) const;
		int  getConstFrameRate(void) const;
		int  getExposureMode(void) const;
		int  setLUT( unsigned char* lut,int number);
	
	
	static const unsigned short EEPROM_BUFLEN  = 0x0800;
	static const unsigned short MAXDACVAL      = 0xffff;
	static const double 		SKIM_FACTOR    = 1.666 / 1.80;
	static const unsigned short SKIM_HEADROOM  = 0x1400;
	
	// Hilfsfunktionen
	/// \todo komplett auf obj-basis schreiben (string, fstream, iomanip,exceptions )
	void initDumpCache(unsigned char *cache, unsigned char *dirty, int n);
	int  EepromToFile(unsigned char *buf, int buflen, const char *filename, int& endCounter) const;
	int  FileToEeprom(unsigned char *buf, unsigned char *dirty, int buflen, const char *filename, int& endCounter);
*/		

} // ip
} // precitec

// Tempate Implementierungen
//#include "camPropertyT.h"

#endif // CAM_PROPERTY_H
