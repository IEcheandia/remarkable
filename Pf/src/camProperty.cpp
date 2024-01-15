
#include <iostream>
#include <string>
#include <list>

//#include "system/types.h"
//#include "system/typeTraits.h"
#include "pf/photonFocus.h"
#include "pfCamera.h"
#include "camProperty.h"


namespace precitec
{
namespace ip
{
	using std::cout;
	using std::endl;
	using std::string;

	/**
	 * std-CTor
	 */
	Property::Property(std::string name, PropertyType type) 
	 : name_(name), type_(type), 
	camera_(NULL), handle_(No<uInt>::Value) 
	{
		//cout << "CTor0 Property: " << handle_ << std::endl;
		valueCurrent_ = false;
	}

	/**
	 * std-CTor
	 */
	Property::Property(std::string name, PropertyType type, 
										 BaseCamera * camera) 
	 : name_(name), type_(type), 
	camera_(camera), handle_(No<uInt>::Value) 
	{
		validateAccess(camera_);
		//cout << "CTor1 Property: " << name_ << std::endl;
		valueCurrent_ = false;
	}
	
	Property::Property(Camera & cam, pf::Handle prop)
	 : name_(pfProperty_GetName(cam.port().handle(), prop)),
	type_(pfProperty_GetType(cam.port().handle(), prop)),
	camera_(&cam), handle_(prop), valueCurrent_(false)
	{
		//cout << "CTor2 Property: " << name_ << " c-handle:" << cam.port().handle() << std::endl;
	}

	/**
	 * macht alle Ueberpruefungen, die notwendig sind
	 * bevor man auf eine pf-Funktion zugreift.
	 * Setzt ggf. handle_.
	 * /result bool ok dann ist handle_ auch richtig gesetzt
	 */
	pf::Handle Property::validateAccess(BaseCamera const* camera)
	{
		//cout << "validateAccess: " /*<< camera << ".." << handle_*/ << std::endl;
		// Schnellabhandlung des Normalfalls
		if (isHandleValid()) return true;
		// erst scheller Test auf Kamera-Handle
		// dann Stringvergleich des Namen
		if (isConnected(camera) && isValid() )
		{
			//cout << "isConnected: && isValid--" << name_; // << std::endl;
			// Kamera und Namen sind ok
			// versuchen Property-Handle zu erstellen
			handle_ = camera->findProperty(name_);
			//cout << "--handle " << handle_ << std::endl;
			
			if (isHandleValid())
			{
				//cout << "handleValid " << handle_ << std::endl;
				//uInt realType = pfProperty_GetType(&camera, handle_);
				int realType = camera->getType(handle_);
				//Typ pruefen/setzen
				if (type_==PF_INVALID) type_ = realType;
				else
				{ // wenn Typ vorgegeben ist muss er auch stimmen, sonst ist alles nix
					//cout << "type_: " << type_ << "(" << realType << ")" << std::endl;
					// Typ Mode soll als int behandelt werden
					if (realType==PF_MODE) realType = PF_INT;
					else if (type_!=realType) handle_ = No<uInt>::Value;
				}
			}
			else
			{
					cout << "property handle invalid" << std::endl;
			}
		} // if isConnected && isValid
		else
		{
			cout << "!isConnected:" << std::endl;
		}
		return isHandleValid();
	}

	/// ist Property mit Kamera verbunden
	bool Property::isConnected(BaseCamera const* camera) const 
	{ 
		return camera && camera->isValid(); 
	}
	
	
	/// hat die Kamera diese Eigenschaft
	bool Property::isImplementedBy(BaseCamera const& camera) const 
	{ 
		return isConnected(&camera) && camera.implements(name_); 
	}
	
	
	/**
	 * ungecachtes Setzen des Kamera-Property aus String
	 * ???? von camera ???
	 */
	bool Property::write(BaseCamera const* camera, std::string const& value) 
	{
		if (!validateAccess(camera)) return false;
		int error = camera->propertyFromString(handle_, value);
		return (error>=0);
	}

	
	/**
	 * ungecachtes Auslesen des Kamera-Property aus String
	 */
	bool Property::read(BaseCamera const* camera, std::string & value) 
	{
		//cout << "Property::read(string) " << std::endl;
		if (!validateAccess(camera) ) return false;
		value = camera->propertyToString(handle_);
		return (value!=""/*No<std::string>::Value*/);
	}

	
       bool Property::isHandleValid() const
       { 
           return handle_ != No<uInt>::Value; 
       }
    
	
	/**
	 * Zuweisung von Property auf String (liefert Wert)
	 */
	/// Zuweisung auf String
 /*
	std::string operator << (std::string& str, Property & property)
	{ 
		//cout << "std::operator <<" << std::endl;
		// da wird const& zurueckgeben muss der String statisch sein
		static std::string noValue="No Value";
		bool ok = property.read(property.camera_, str);
		return ok ? str : "No Value";
	}
*/
	
	/// Zuweisung von String
 /*
	Property& operator << (Property& prop, std::string const& value)
	{ 
		prop.write(prop.camera_, value);
		return prop;
	}
*/
	Property::operator std::string () { 
		//cout << "Property::operator std::string " << std::endl;
		validateAccess(camera_);
		if (isHandleValid()) return camera_->propertyToString(handle_); 
		else return "unconnectd";
	}

	/**
	 * Zuweisung von string auf Property (liefert Wert)
	 */
#ifndef NDEBUG
  Property & Property::operator = (std::string const& value)  
	{ 
		std::cout << "Property (" << name_ << "-" << handle_ << ") = ";
		if (validateAccess(camera_))
		{
			if (!camera_->propertyFromString(handle_, value))
			{
				/// throw Whatever Error
			 std::cout << "Error: value not set" << std::endl;
			}
			else
			{
				 //std::cout << value << "set" << std::endl;			
			}
		}
		else
		{
				/// throw Whatever Error
			 std::cout << "invalid property" << std::endl;
		}
		return *this;
	}
#else
  Property & Property::operator = (std::string const& value)  
	{ 
		if (validateAccess(camera_))
		{
			if (!camera_->propertyFromString(handle_, value))
			{
				/// throw Whatever Error
			}
		}
		else
		{
			 std::cout << "invalid property" << std::endl;
		}
		return *this;
	}
#endif

	std::ostream& operator << (std::ostream &os, Property &p)
	{
		//cout << "std::ostream op << : " << std::endl;

		os << p.name() << " ";
		switch (p.type())
		{
		case PF_INVALID:
			cout << "<invalid>";
			break;
		case PF_ROOT:
			cout << "<Kamera>";
			break;
		case PF_INT:
			cout << "<int>" << std::string(p);
			break;
		case PF_FLOAT:
			cout << "<float>"  << std::string(p);
			break;
		case PF_BOOL:
			cout << "<bool>" << std::string(p);
			break;
		case PF_MODE:
			cout << "<mode>" << std::string(p);
			break;
		case PF_REGISTER:
			cout << "<reg>" << std::string(p);
			break;
		case PF_STRING:
			cout << "<string>" << std::string(p);
			break;
		case PF_BUFFER:
			cout << "<buffer>";
			break;
		case PF_STRUCT:
			cout << "<struct>";
			break;
		case PF_ARRAY:
			cout << "<array>";
			break;
		case PF_COMMAND:
			cout << "<command>";
			break;
		case PF_EVENT:
			cout << "<event>";
			break;
		}
		return os;
	}
	
	//------------------------------------------------------------------	
	//------------------------------------------------------------------	
	//------------------------------------------------------------------	
	//------------------------------------------------------------------	


} // ip 
} // precitec
