#ifndef PFCAMERA_H
#	define PFCAMERA_H

#include <stdlib.h>

#	include <string>
#	include <list>

typedef std::string String;
typedef unsigned char 	byte;
typedef unsigned int	uInt;
#	include "system/types.h"

#	include "pf/photonFocus.h"
#	include "camPort.h"

namespace precitec
{
namespace ip
{
	class CommandParam;
	
	/**
	 * Generische Kamera
	 * nicht ganz sauber umgesetz, es gibt ein paar Abh‰ngigkeiten von pf
	 */
	class BaseCamera 
	{
	 public:
		 BaseCamera();
		 virtual ~BaseCamera();
	 public:
		// die oeffentliche Schnittstelle ist nichtvirtuell
		// dies bedingt, dass diese Funktionen typischerweise
		// nichts tun als ihre virtuellen Genossen aufzurufen
		 
		 /// Funktion, um eine Kamera am Port zu oeffnen
		 void open(int portNum) { port().open(portNum);}
		 /// schlieﬂt Kamera (Port)
		 void close() { port().close(); }
		 /// ist Kamera (und damit letztlich der Port) gueltig
		 bool isValid() const { return isPortValid(); }

		 /// Key-Value-Coding ist pf-Spezifisch ???		 
		 pf::Handle findProperty(std::string const& name) const { return findCamProperty(name); }

		 /// gibt Typ der Eigenschaft zurueck : PF-Zaehlung
		 uInt getType(pf::Handle property) const { return getPropType(property); }
		 /// ist Eigenschaft vorhanden
		 bool implements(std::string name) const { return implementsProp(name); }
		 /// typisierter Lesezugriff auf Eigenschaft
		 template <class T>
		 int getProperty(pf::Handle property, T &value) const { return getCamProperty(property, value);}
		 /// typisierter Schreibzugriff auf Eigenschaft
		 template <class T>
		 int setProperty(pf::Handle property, T value) const { return setCamProperty(property, value); }
		 /// String-Lesezugrif auf Eigenschaft
		 std::string propertyToString(pf::Handle property) const { return getPropertyAsString(property); }
		 /// String-Schreibzugrif auf Eigenschaft
		 bool propertyFromString(pf::Handle property, std::string const& value) const { return setPropertyFromString(property, value); }
		 friend std::ostream& operator << (std::ostream &os, BaseCamera const &c);

		 int flags(pf::Handle property) const { return getFlags(property);}
		 
	 private:
	 	// die private virtuelle Schnittstelle, hier spielt die Musik
		 virtual int getFlags(pf::Handle property) const { return 0;}
	 	
		 virtual bool isPortValid() const { 
				 std::cout << "BaseCamera::isPortValid()" << std::endl;
			 return false; }
//		 int setProperty(pf::Handle property, T value) const { setCamProperty(property, value); }
		 virtual std::string getPropertyAsString(pf::Handle property) const;
		 virtual bool setPropertyFromString(pf::Handle property, std::string const& value) const;
		 virtual pf::Handle findCamProperty(std::string const& name) const;
		 virtual uInt getPropType(pf::Handle property) const;
		 virtual bool implementsProp(std::string name) const;
		 virtual int getCamProperty(pf::Handle property, bool  &value) const;
		 virtual int getCamProperty(pf::Handle property, int   &value) const;
		 virtual int getCamProperty(pf::Handle property, float &value) const;
		 virtual int getCamProperty(pf::Handle property, std::string &value) const;
		 virtual int setCamProperty(pf::Handle property, bool  value) const;
		 virtual int setCamProperty(pf::Handle property, int   value) const;
		 virtual int setCamProperty(pf::Handle property, float value) const;
		 virtual int setCamProperty(pf::Handle property, std::string const& value) const;
		 virtual int setCamProperty(pf::Handle property, CommandParam) const;

		 virtual CamPort const& port() const { 
			 //std::cout << "BaseCamera: port()const" << std::endl;
			 static const CamPort camPort;
			 return camPort; }
		 virtual CamPort& port() { 
			 //std::cout << "BaseCamera: port()" << std::endl;
			 static CamPort camPort;
			 return camPort; }
	 protected: public:
		 //int handle() { port().handle(); }
		 int  handle() const { 
			 //std::cout << "Camera::handle()" << std::endl;
			 //std::cout << "Camera::handle()" << port() << std::endl;
			 return port().handle(); 
		 }
		 //int handle() const;
	 protected:
		 std::string errorMsg_; ///< letzte Fehlermeldung
	}; // BaseCamera
	
	//std::ostream& operator << (std::ostream &os, Camera const &c);
/*	
	namespace pf
	{*/
	class Property;
	class Camera : public BaseCamera
	{
	 public:
		 Camera() : port_() { // invalid Port erzeugen
			 //std::cout << "Camera: " << std::endl;
		 }
		 Camera(Camera const&c) : port_(c.port_) {}
		 Camera(int portNum) : port_(portNum) {}
		 Camera(pf::Port & port);
		 //Camera(pf::Port const& port) : port_(port) {}
		 //Camera(pf::Port const& port) : BaseCamera(), port_(port) {}
		 virtual ~Camera();
		 /// best mˆgliche Geschwindigkeit einstellen
		 void negotiateComSpeed();
		 /// aktuelle Geschwindigkeit herausfinden 
		 int  getComSpeed();
		 
		 void getDllVersion(int &vMajor, int &vMinor);
		 void listAllProperties(std::list<pf::Handle> &propList, bool recurse=true, 
		 												pf::Handle root = pf::NoHandle);
		 pf::Handle getChildProperty(pf::Handle parentProp) const;
	 private:			 
		 pf::Handle getRootProperty() const;
		 pf::Handle getNextProperty(pf::Handle parentProp, pf::Handle currProp) const;
		 
		 void addAllSiblings(pf::Handle parentProp, pf::Handle firstProp, 
		 											std::list<pf::Handle> &propList, int i, bool recurse);
		 void addAllChildren(pf::Handle parentProp, std::list<pf::Handle> &propList, int i, bool recurse);
		 
	 private:
		 virtual int getFlags(pf::Handle property) const { return pfProperty_GetFlags(port().handle(), property); }
		 virtual bool isPortValid() const { 
			 //std::cout << "Camera::isPortValid()" << std::endl;
			 return port().isValid(); }
		 virtual std::string getPropertyAsString(pf::Handle property) const;
		 virtual bool setPropertyFromString(pf::Handle property, std::string const& value) const;
		 virtual pf::Handle findCamProperty(std::string const& name) const;
		 virtual uInt getPropType(pf::Handle property) const;
		 virtual bool implementsProp(std::string name) const;
		 virtual int getCamProperty(pf::Handle property, bool  &value) const;
		 virtual int getCamProperty(pf::Handle property, int   &value) const;
		 virtual int getCamProperty(pf::Handle property, float &value) const;
		 virtual int getCamProperty(pf::Handle property, std::string &value) const;
		 virtual int setCamProperty(pf::Handle property, bool  value) const;
		 virtual int setCamProperty(pf::Handle property, int   value) const;
		 virtual int setCamProperty(pf::Handle property, float value) const;
		 virtual int setCamProperty(pf::Handle property, std::string const& value) const;
		 virtual int setCamProperty(pf::Handle property, CommandParam) const;
		 /*		 
				uInt operator () () { return port_();}
 
 
				uInt type = pfProperty_GetType(handle_);
				int error = camera_.setProperty(handle_, std::string &value);
				int error = camera_.setProperty(handle_, float &val);
				int error = camera_.setProperty(handle_, int &val);
				int error = camera_.setProperty(handle_, bool &val);
 
				int error = camera_.getProperty(handle_, std::string &value);
				int error = camera_.getProperty(handle_, float &val);
				int error = camera_.getProperty(handle_, int &val);
				int error = camera_.getProperty(handle_, bool &val);
				*/		 
	 private:
		 std::string handlePfError(int error) const;
//			 void clearPfError();
	 public:
		 virtual pf::Port const& port() const { 
			 //std::cout << "Camera: port()const " << port_ << std::endl;
			 return port_; }
		 virtual pf::Port &port() { 
			 //std::cout << "Camera: port()" << std::endl;
			 return port_; }
	 private:
		 pf::Port port_;  ///< ???
	}; // Camera
	//std::ostream& operator << (std::ostream &os, Camera const &c);
//	} // pf
} // ip
} // precitec

#endif // PFCAMERA_H
