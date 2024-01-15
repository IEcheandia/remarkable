

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

	//-----------------------------------------------------------
	// die BaseCamera- Member sind fast alle Dummies, die nie
	// aufgerufen werden sollten, ggf. BaseCamera voll virtualisieren 
	// diese Member  zu NULl setzen
	
	BaseCamera::BaseCamera() {}
	BaseCamera::~BaseCamera() {}
	
	//	int  BaseCamera::handle() const { return port().handle(); }
	pf::Handle BaseCamera::findCamProperty(std::string const& name) const
	{
	 	cout << "using BaseCamera::getPropType: " << std::endl;
		return pf::NoHandle;
	}
	uInt BaseCamera::getPropType(pf::Handle property) const
	{
	 //cout << "BaseCamera::getPropType: " << std::endl;
		return PF_INVALID;
	}
	bool BaseCamera::implementsProp(std::string name) const 
	{
		return false;
	}

	std::string BaseCamera::getPropertyAsString(pf::Handle) const
	{
	 //cout << "getPropertyAsString: " << std::endl;
		return No<String>::Value;
	}

	bool BaseCamera::setPropertyFromString(pf::Handle property, std::string const& value) const
	{
		//std::cout << "BaseCamera:: setpropFromstring" << std::endl;		
		return false;
	}
	
	int BaseCamera::getCamProperty(pf::Handle property, bool  &value) const
	{
		return No<int>::Value;
	}

	int BaseCamera::getCamProperty(pf::Handle property, int  &value) const
	{
		return No<int>::Value;
	}

	int BaseCamera::getCamProperty(pf::Handle property, float  &value) const
	{
		//cout << "BaseCamera::getCamProperty<float>: " << std::endl;
		return No<int>::Value;
	}

	int BaseCamera::getCamProperty(pf::Handle property, std::string &value) const
	{
		return No<int>::Value;
	}

	int BaseCamera::setCamProperty(pf::Handle property, bool  value) const
	{
		return No<int>::Value;
	}

	int BaseCamera::setCamProperty(pf::Handle property, int value) const
	{
		return No<int>::Value;
	}

	int BaseCamera::setCamProperty(pf::Handle property, float value) const
	{
		return No<int>::Value;
	}

	int BaseCamera::setCamProperty(pf::Handle property, std::string const& value) const
	{
		return No<int>::Value;
	}


	int BaseCamera::setCamProperty(pf::Handle property, CommandParam) const
	{
		return No<int>::Value;
	}
//	namespace pf
//	{
		
		using std::cout;
		using std::endl;
		using std::string;
		
		
	//------------------------------------------------------------------
	//------------------------------------------------------------------
	//------------------------------------------------------------------
	//------------------------------------------------------------------	

	//-----------------------------------------------------------
	// die Camera-Member sind die eigentlichen Implementierngen
	// von Kamera-verhalten 
	
	Camera::Camera(pf::Port & port) : port_(port) {}
	
	Camera::~Camera() { /*std::cout << "Camera::XTor" << std::endl;*/ }

	/**
	 * ermittelt nud setzt die maximale Port-Geschwindigkeit
	 * fuer die serielle Verbindng Grabber-Kamera
	 */	
	void Camera::negotiateComSpeed()
	{
		// bisher werden diese Baudraen (bei manchen Kameras) unterstützt
		const int baudRate[] =  {  9600, 19200, 38400, 57600 };
		const int NumBaudRates = sizeof(baudRate)/sizeof(int);
		
		if (isValid())
		{ // ohne offenen port is natuerlich nix
			
			// finde maximale Baudrate (Theorie=nach Kamera-Angaben)
			// wir probieren von unten solnage alle Raten aus bis 
			// wir auf eine nicht unterstütze treffen
			int error = 0;
			int rate;
			for (rate=0; (rate<NumBaudRates)&& !error; ++rate)
			{
				//cout << " testing supported Rate " << baudRate[rate] << std::endl;
				//cout << "port: " << handle() << std::endl;
				error = pfIsBaudRateSupported(handle(), baudRate[rate]);
				//cout << "error: " << error << std::endl;
			}
			--rate; // wir sind eins zu weit gegangen
			
			if (rate<0) 
			{ // nicht mal Minimal-Rate wird unterstützt 
				// sollte eingentlich nicht wirklich vorkommen 
				// keine Kommunikation ist moeglich
				// Totalausstieg unumgänglich
				cout << "no Communication possible. Rate:" << rate << std::endl;
				handlePfError(error);
				port().close();
				return;
			}
			else if (rate==0)
			{
				// nur eine Baudrate (die ist fest eingestellt also nichts tun)
				// tatsaechlich scheint bei solchen Kameras das Setzen der
				// Baudrate immer zu scheitern, auch die der Defaultrate
				return;
			}
			
			// finde maximale Baudrate (Praxis)
			// da wir Rate um eins reduzier haben muesste es sofort klappen
			// also von oben nach unten probieren, bis wir gut sind
			do
			{
				//cout << " testing actual Rate " << baudRate[rate] << std::endl;
				//cout << "port: " << handle() << std::endl;
				error = pfSetBaudRate(handle(), baudRate[rate]);
				handlePfError(error);
				cout << "no reliable Communication possible: " << error << std::endl;
			}	while ((--rate!=-1) && error);
			
			if(error < 0)
			{
				// Totalausstieg unumgänglich, sollte auch nie
				// vorkommen, wir haben ja zuvor erfolgreich miteinander
				// geredet
				cout << "shit happens. rate: " << rate << std::endl;
				handlePfError(error);
				port().close();
				// return; (kann entfallen da auch in allen anderen Faellen nichts mehr geschieht 
			} // if error < 0

			// die erste Baudrate, die wir einstellen konnten ist die maximale
			// wir haben also unser Ziel erreicht (sofern wir nicht quer 
			// ausgestiegen sind
		} // if valid
	} // negotiateSpeed

	/// liest aktuelle Kommunikationsgeschwindigkeit (Baud) aus
	int Camera::getComSpeed()
	{
		static int baudRate = 0;
		pfGetBaudRate(handle(), &baudRate);
		return baudRate;
	}

	
//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------
	/**
	 * zum ersten Test, ob weitermachen sinnvoll ist
	 * ??? es ist noch nicht klar, genau wo dies aufgerufen werden soll
	 */
	void Camera::getDllVersion(int &vMajor, int &vMinor)
	{
		int maj, min;
		/*cout <<*/ pfDeviceGetDllVersion(handle(), &maj, &min); // << std::endl;
		vMajor = maj;
		vMinor = min;
	}
	
	/**
	 * DIE Funktion um eine Eigenschaft zu finden, gibt die handle zurueck
	 */
	pf::Handle Camera::findCamProperty(std::string const& name) const
	{
		pf::Handle prop = pfProperty_ParseName(handle(), name.c_str());
		//std::cout << "find Property:: " << handle() << " " << name << " " << int(prop) << std::endl;
		return (prop != pf::NoHandle) ? prop : No<uInt>::Value;
	}
	/* rekursives Vorgehen
	pf::Handle Camera::findCamProperty(std::string const& name) const
	{
		int node = pfDevice_GetRoot(handle());
		int newNode = 0;
		int foundPosition = name.find('.');
		
		while (foundPosition>=0)
		{
			String rootName = name.substr(0, foundPosition);
			newNode = pfProperty_Select(handle(), node, child);
		} 
		String root = 
		pf::Handle prop = pfProperty_ParseName(handle(), name.c_str());
		return (prop != pf::NoHandle) ? prop : No<uInt>::Value;
	}*/
	
	/**
	 * gibt den Typ nach PF-Zaehlung urueck
	 */
	uInt Camera::getPropType(pf::Handle property) const
	{
		return pfProperty_GetType(handle(), property);
	}

	/**
	 * gibt es die Eigenschft ueberhaupt
	 * ??? man kann acuh findCamProperty nehmen und auf NoHandle prüfen ???
	 */
	bool Camera::implementsProp(std::string name) const
	{
		return findCamProperty(name.c_str()) != No<uInt>::Value;
	}

	std::string Camera::getPropertyAsString(pf::Handle property) const
	{
	 //cout << "getPropertyAsString: " << std::endl;
		char value[pf::MaxStringLen];
		int error = pfDevice_GetProperty_String(handle(), property, value, pf::MaxStringLen);
		handlePfError(error);
		return std::string(value);
	}

	bool Camera::setPropertyFromString(pf::Handle property, std::string const& value) const
	{
		//std::cout << "setPropertyFromString: " << property << " to " << value; 
		int error = pfDevice_SetProperty_String(handle(), property, (char*)value.c_str());
		handlePfError(error);
		//std::cout << (error < 0 ? " nok" : " ok" ) << std::endl;
		return (error >= 0);
	}

	int Camera::getCamProperty(pf::Handle property, bool &value) const
	{
		PFValue val;
		val.type = PF_BOOL;
		int error = pfDevice_GetProperty(handle(), property, &val);
		handlePfError(error);
		value = val.value.i;
		return error;
	}

	int Camera::getCamProperty(pf::Handle property, int &value) const
	{
		PFValue val;
		val.type = PF_INT;
		int error = pfDevice_GetProperty(handle(), property, &val);
		handlePfError(error);
		value = val.value.i;
		return error;
	}

	int Camera::getCamProperty(pf::Handle property, float &value) const
	{
		//cout << "Camera::getCamProperty<float>: " << std::endl;
		PFValue val;
		val.type = PF_FLOAT;
		int error = pfDevice_GetProperty(handle(), property, &val);
		handlePfError(error);
		value = val.value.f;
		//cout << "value: " << value << std::endl;
		//cout << "Camera::getCamProperty<float>: end" << std::endl;
		return error;
	}

	int Camera::getCamProperty(pf::Handle property, std::string &value) const
	{
	 //cout << "Camera::getCamProperty(string): " << std::endl;
		char buffer[pf::MaxStringLen];
		int error = pfDevice_GetProperty_String(handle(), property, 
																				buffer, sizeof(buffer));	
		handlePfError(error);
		value = std::string(buffer);
		return error;
	}

	int Camera::setCamProperty(pf::Handle property, bool  value) const
	{
		PFValue val;
		val.type    = PF_BOOL;
		val.value.i = value;
		int error   = pfDevice_SetProperty(handle(), property, &val);
		handlePfError(error);
		return error;
	}

	int Camera::setCamProperty(pf::Handle property, int value) const
	{
		PFValue val;
		val.type    = PF_INT;
		val.value.i = value;
		int error   = pfDevice_SetProperty(handle(), property, &val);
		handlePfError(error);
		return error;
	}

	int Camera::setCamProperty(pf::Handle property, float value) const
	{
		PFValue val;
		val.type    = PF_FLOAT;
		val.value.f = value;
		int error   = pfDevice_SetProperty(handle(), property, &val);
		handlePfError(error);
		return error;
	}

  int Camera::setCamProperty(pf::Handle property, std::string const& value) const
	{
		int error = pfDevice_SetProperty_String(handle(), property, (char*)value.c_str());
		handlePfError(error);
		return error;
	}
	
	int Camera::setCamProperty(pf::Handle property, CommandParam) const
	{
		PFValue val;
		val.type    = PF_COMMAND;
		val.value.i = 0;
		int error   = pfDevice_SetProperty(handle(), property, &val);
		handlePfError(error);
		return error;
	}

	
	
//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------
	pf::Handle Camera::getRootProperty() const
	{
		//cout << "getRootProperty()" << std::endl;
		return pfDevice_GetRoot(port().handle());
	}
	
	pf::Handle Camera::getNextProperty(pf::Handle parentProp, pf::Handle currProp) const
	{
		//cout << "getNextProperty()" << std::endl;
		return pfProperty_Select(port().handle(), parentProp, currProp);
	}

	pf::Handle Camera::getChildProperty(pf::Handle parentProp) const
	{
		//cout << "getChildProperty()" << std::endl;
		return pfProperty_Select(port().handle(), parentProp, parentProp);
	}

	void Camera::addAllChildren(pf::Handle parentProp, std::list<pf::Handle> &propList, int n, bool recurse)
	{
		//cout << "addAllChildren() " << parentProp << std::endl;
		pf::Handle childProp = getChildProperty(parentProp);
		//cout << "childProp: " << childProp << std::endl;
		if (childProp != pf::NoHandle)
		{
			propList.push_back(childProp);
			// for (int i=0; i<n; ++i) cout << '\t';
			//Property p(*this, childProp);
			//cout << p << std::endl;
			addAllSiblings(parentProp, childProp, propList, n, recurse);
		}
	}

	void Camera::addAllSiblings(pf::Handle parentProp, pf::Handle firstProp, 
	std::list<pf::Handle> &propList, int n, bool recurse)
	{
		//cout << "addAllSiblings() " << parentProp << std::endl;
		pf::Handle currProp = firstProp;
		pf::Handle nextProp;
		while (currProp != pf::NoHandle)
		{
			nextProp = getNextProperty(parentProp, currProp);
			//cout << "nextProp: " << nextProp << std::endl;
			if (nextProp != pf::NoHandle)
			{
				propList.push_back(nextProp);
				//for (int i=0; i<n; ++i) cout << '\t';
				//Property p(*this, nextProp);
				//cout << p << std::endl;
				if (recurse) addAllChildren(nextProp, propList, n+1, recurse);
			}
			currProp = nextProp;
		}
	}

	void Camera::listAllProperties(std::list<pf::Handle> &propList, bool recurse, pf::Handle root)
	{
		if (root==pf::NoHandle) root = getRootProperty();
		//Property p(*this, root);
		//cout << "root: " << p << std::endl;
		addAllChildren(root, propList, 1, recurse);
	}
	
//	pf::Handle pfProperty_Select(pf::Handle handle, pf::Handle node, pf::Handle node) const

	//APIFUNC(unsigned long, Property_GetFlags, (int port, TOKEN p))

		
	 
	std::string Camera::handlePfError(int error) const
	{
		std::string errorMsg;
		if (error<0)
		{
			errorMsg = pfGetErrorString(error);
			cout << "Fehler: " << errorMsg << std::endl;
		}
		else if  (error>0)
		{
//#ifndef NDEBUG
			errorMsg = pfGetErrorString(error);
			cout << "Warning: " << errorMsg << std::endl;
//#endif
		}
		return errorMsg;
	}
	/*
	void Camera::clearPfint error()
	{
		//			errorMsg_.empty();
	}
*/	
	std::ostream& operator << (std::ostream &os, BaseCamera const& c)
	{
		os << "[]< " << c.port();
		return os;
	}
	
//	} // pf
} // ip 
} // precitec

