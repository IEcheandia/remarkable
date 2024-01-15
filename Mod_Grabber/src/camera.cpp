/**
 * @file
 * @brief  camera Klasse realisiert Kamera Zugriff, Zugriff auf Photonfocus SDK
 *
 * @author JS
 * @date   20.05.11
 * @version 0.1
 *
 *
 */





#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "camera.h"
//#include "templates.h"
//#include "photonfocusFile.h"

// -----------------------------------------------------------------------------
// System includes
// -----------------------------------------------------------------------------


using namespace precitec;

// definition of static class members

/*static*/ const unsigned short Camera::EEPROM_BUFLEN  = 0x0800;		///<EEPROM Laenge
/*static*/ const unsigned short Camera::MAXDACVAL      = 0xffff;		/// Maximaler DAC Wert
/*static*/ const double 		Camera::SKIM_FACTOR    = 1.666 / 1.80; 	///< Umrechnungsfaktor fuer skimming
/*static*/ const unsigned short Camera::SKIM_HEADROOM  = 0x1400;		///<skim Wertebereich


/**
* @brief Fehlertext holen
*
* \param    error - Fehlernummer
* \return   error Fehlernummer
*
*/
int Camera::handleError(int error)
{
	const char *s;
	s = pfGetErrorString(error);
     if (error < 0) {
    	 std::printf("Error: %s\n", s);
     }else {
    	 std::printf("Warning: %s\n", s);
     }
     return error;
}

/**
* @brief Lege die nach aussen in der GUI sichtbaren Properties fest
*
* \return   error Fehlernummer
*
*/
int Camera::createCameraProperties(void)
{
	//verfuegbar gestellte Kamerakeys festlegen:


	if(m_oCamName =="MV1-D2048x1088-160")
	{
		MaxSensorWidth = 2048;
		MaxSensorHeight = 1088;

        //Properties der MV1-D2048 unterscheiden sich wesentlich von der MV1-D1024Serie
		std::cout<<"cam Props fuer MV1-D2048x1088-160 erzeugen"<<std::endl;

		cameraProps_.push_back( new TSensorProperty<float>("ExposureTime",0.0,120.0,2.0,TFloat)  );
		cameraProps_.push_back( new TSensorProperty<int>("Voltages.BlackLevelOffset",0,16380,16300,TInt) );
		//Trigger Source
		cameraProps_.push_back( new TSensorProperty<int>("Trigger.Source",0,1,1,TInt) );

		// Store Setup to defaults - hide
		//	cameraProps_.push_back( new TSensorProperty<int>("StoreDefaults",0,1,1,TInt) );

		//Multislope Values
		// LinLog gibt es bei CMOSIS CMV2000 nicht ! Dafuer Multislope !
		cameraProps_.push_back( new TSensorProperty<int>("Multislope.Mode",0,4,0,TInt)  );
		cameraProps_.push_back( new TSensorProperty<int>("Multislope.NrSlopes",2,3,2,TInt)  );
		cameraProps_.push_back( new TSensorProperty<int>("Multislope.Value1",64,127,64,TInt)  );
		cameraProps_.push_back( new TSensorProperty<int>("Multislope.Value2",64,127,64,TInt )  );
		cameraProps_.push_back( new TSensorProperty<int>("Multislope.Time1",0,1000,900,TInt)  );
		cameraProps_.push_back( new TSensorProperty<int>("Multislope.Time2",0,1000,1000,TInt)  );

		// ROI
		cameraProps_.push_back( new TSensorProperty<int>("Window.X",0,2047,256,TInt )  ); 	//xoffset
		cameraProps_.push_back( new TSensorProperty<int>("Window.Y",0,1087,256,TInt)  );   	//yoffset
		cameraProps_.push_back( new TSensorProperty<int>("Window.W",8,2048,512,TInt)  );   	// xwidth
		cameraProps_.push_back( new TSensorProperty<int>("Window.H",1,1088,512,TInt) );		//yheight

		/*
		//Line Hopping Properties
		cameraProps_.push_back( new TSensorProperty<int>("Decimation.Y",1,15,1,TInt) );		//Line Hopping
		 */

		// Trigger.LevelControlled scheint sich manchmal selbststaendig auf 1 zu aendern ...
		cameraProps_.push_back( new TSensorProperty<int>("Trigger.LevelControlled",0,1,0,TInt) );
        // For LED-Calib
        cameraProps_.push_back( new TSensorProperty<int>("BlackLevelOffset",0,4000,3333,TInt) );

	}
	else
	{
		MaxSensorWidth = 1024;
		MaxSensorHeight = 1024;

		//pCameraKey p = new TCamProperty<float>("ExposureTime",0.0,120.0,2.0);
		//Property,min,max,default,DataType

		std::cout<<"cam Props erzeugen"<<std::endl;
		cameraProps_.push_back( new TSensorProperty<float>("ExposureTime",0.0,120.0,2.0,TFloat)  );
		cameraProps_.push_back( new TSensorProperty<int>("Voltages.BlackLevelOffset",0,4000,3334,TInt) );
		//Trigger Source
		cameraProps_.push_back( new TSensorProperty<int>("Trigger.Source",0,1,1,TInt) );

		// Store Setup to defaults - hide
		//	cameraProps_.push_back( new TSensorProperty<int>("StoreDefaults",0,1,1,TInt) );

		//LinLog Values
		cameraProps_.push_back( new TSensorProperty<int>("LinLog.Mode",0,4,1,TInt)  );
		cameraProps_.push_back( new TSensorProperty<int>("LinLog.Value1",0,30,10,TInt)  );
		cameraProps_.push_back( new TSensorProperty<int>("LinLog.Value2",0,30,5,TInt )  );
		cameraProps_.push_back( new TSensorProperty<int>("LinLog.Time1",0,1000,900,TInt)  );
		cameraProps_.push_back( new TSensorProperty<int>("LinLog.Time2",0,1000,1000,TInt)  );

		// ROI
		cameraProps_.push_back( new TSensorProperty<int>("Window.X",0,1024,256,TInt )  ); 	//xoffset
		cameraProps_.push_back( new TSensorProperty<int>("Window.Y",0,1024,256,TInt)  );   	//yoffset
		cameraProps_.push_back( new TSensorProperty<int>("Window.W",0,1024,512,TInt)  );   	// xwidth
		cameraProps_.push_back( new TSensorProperty<int>("Window.H",0,1024,512,TInt) );		//yheight

		//MROI Properties
		/*
		cameraProps_.push_back( new TSensorProperty<int>("MROI.Enable",0,1,0,TInt )  ); 	 //MROI Enable
		cameraProps_.push_back( new TSensorProperty<int>("MROI.MROI0_Y",0,1024,256,TInt)  ); //MROI 0 Start Y
		cameraProps_.push_back( new TSensorProperty<int>("MROI.MROI0_H",0,1024,256,TInt)  ); //MROI 0 Hoehe
		cameraProps_.push_back( new TSensorProperty<int>("MROI.MROI1_Y",0,1024,512,TInt) );	 //MROI 1 Start y
		cameraProps_.push_back( new TSensorProperty<int>("MROI.MROI1_H",0,1024,256,TInt) );	 // MROI 1 Hoehe

		//Line Hopping Properties
		cameraProps_.push_back( new TSensorProperty<int>("Decimation.Y",0,15,0,TInt) );		//Line Hopping
		 */

		//Sensor IOs fuer die Beleuchtung
		cameraProps_.push_back( new TSensorProperty<int>("PWM.Enable",0,1,1,TInt )  ); 	//Linienlaser Ausgang enable
		cameraProps_.push_back( new TSensorProperty<int>("PWM.PWM1",0,255,125,TInt)  );  //Linienlaser Ausgang1 dimmen
		cameraProps_.push_back( new TSensorProperty<int>("PWM.PWM2",0,255,125,TInt)  );  //Linienlaser Ausgang2 dimmen
		cameraProps_.push_back( new TSensorProperty<int>("PWM.PWMBel",0,255,125,TInt) ); //Beleuchtungs Ausgang dimmen

		// Trigger.LevelControlled scheint sich manchmal selbststaendig auf 1 zu aendern ...
		cameraProps_.push_back( new TSensorProperty<int>("Trigger.LevelControlled",0,1,0,TInt) );

		//Properties fuer die Bildkorrektur
		//1:Offset,2:Offset Hot Pixel; 3:Hot Pixel; 4: Offset,Gain; 5:Offset,Gain,Hot Pixel
		cameraProps_.push_back( new TSensorProperty<int>("Correction.Mode",0,5,4,TInt )  );

		//Skim Feature Enable
		cameraProps_.push_back( new TSensorProperty<int>("Skim",0,8,0,TInt )  );

		//New Fiberloss Propery in ms
		cameraProps_.push_back( new TSensorProperty<float>("PWM.FiberLossTimeout",0.0,3400.0,0.0,TFloat )  );
		//ReOpen Feature
		cameraProps_.push_back( new TSensorProperty<int>("CameraReOpen",0,1,0,TInt )  );
		cameraProps_.push_back( new TSensorProperty<int>("FlushPort",0,1,0,TInt )  );
        // For LED-Calib
        cameraProps_.push_back( new TSensorProperty<int>("BlackLevelOffset",0,4000,3333,TInt) );
        //For simultaneous readout
        cameraProps_.push_back( new TSensorProperty<int>("Trigger.Interleave",0,1,0,TInt) );


	}//else

	//pCameraKey p = new TCamProperty<float>("ExposureTime",0.0,120.0,2.0,TFloat);
	//TCamProperty<float> *t =   dynamic_cast< TCamProperty<float>* > (p);

	//Test: Ausgabe
	/*
		for(unsigned int i=0;i<cameraProps_.size();++i)
		{
			std::cout<<"cameraProps: " << cameraProps_[i]->property<< std::endl;
			if( cameraProps_[i]->propertyType == TInt )
			{
				TSensorProperty<int> *testIntProp = dynamic_cast< TSensorProperty<int>* >(cameraProps_[i]);
				std::cout<<"Min/Max: "<<testIntProp->min<<","<<testIntProp->max<<std::endl;
			}
			else if ( cameraProps_[i]->propertyType == TFloat )
			{
				TSensorProperty<float> *testFloatProp = dynamic_cast<TSensorProperty<float>*>( cameraProps_[i]);
				std::cout<<"Min/Max: "<<testFloatProp->min<<","<<testFloatProp->max<<std::endl;
			}
		}
	*/

	return 1;
}

/**
* @brief Kamera Konstruktor
*
* Initialisierung mit der port Nummer
* Das set "cameraKeys_ wird mit den freigegeben Properties fuer die
* Kamera initialisiert
*
*/
Camera :: Camera(int portNum) :
	port_(portNum),
	MaxSensorWidth(1024),
	MaxSensorHeight(1024)

{


	//createCameraProperties();

}


Camera :: ~Camera()
{

	//cameraProps loeschen
		std::cout<<"deleting "<<cameraProps_.size()<<" cam probs"<<std::endl;
		for(unsigned int i=0;i<cameraProps_.size();++i)
		{
			if(cameraProps_[i])
		     delete 	cameraProps_[i];
		}
		cameraProps_.clear();
 	//cout << "Camera::DTor" << endl;
}



/**
*  @brief Kamera initialisieren
*
* \param   portNum - Kameraport
* \return  error Fehlernummer
*
 */
int Camera::init(int portNum)
{

	int 	numOfPorts = 0;
	char 	manu[256];
	//int 	mBytes, nBytes, version, type;
    port_ = 0; // Eine Me4 im System
    int error = 0;
    int status = 1;

	// Falls sich noch eine Me3 im System befindet wird diese nicht erkannt, deshalb port 0
	error = pfPortInit(&numOfPorts);
    std::cout << "Portinit" << numOfPorts<< std::endl;


    error = pfDeviceOpen(0);  // 0 ist port_;
    std::cout << "Camera DeviceOpen ";
    if (error < 0)
    {
       std::cout << "Camera Device Open  - " << pfGetErrorString(error) << std::endl;
       return -1;
    }
     else
       std::cout << "Camera Device opened " <<std::endl;


    TOKEN t = pfProperty_ParseName(port_, "Header.Pixelclock");
    if(t == INVALID_TOKEN)
    {
    	std::cout<<"Property Header.Pixelclock not found or bad index"<<std::endl;
    	error = -1;
    }
    else
    {
    	pfDevice_GetProperty_String(port_, t, manu, 50);
    	std::cout<<"Pixelclock: "<<manu<<std::endl;
    }

    //Camera Name setzen
    if(error>=0)
    {
    	 t = pfProperty_ParseName(0, "CameraName");
    	 if(t == INVALID_TOKEN)
    		std::cout<<"Property CameraName not found or bad index"<<std::endl;
    	 else
    	 {
    		 error = pfDevice_GetProperty_String(port_, t, manu, 50);
    		 //char pointer wieder richtig auf string konvertieren...
    		 if(error >= 0)
    		 {
    			 std::istringstream iss(manu);
    			 iss >> m_oCamName;
    			 std::cout <<"Camera Name: "<< m_oCamName << std::endl;

    			 //erstelle die Property Liste
    			 createCameraProperties();
    		 }
    	 }
    }

    if(error >= 0)
    	status = 1;
    else
    	status = -1;

    return status;
}


/**
 * @brief Kamerakomunikation schliessen
 */
void Camera :: close(void)
{
	std::cout << "Communication with camera closed" << std::endl;
	int error  = pfDeviceClose(port_);
	std::cout << "DeviceClose: " << error << "  - " << pfGetErrorString(error) << std::endl;
}


/**
 * @brief Kamerakomunikation schliessen und wieder oeffnen
 */
int Camera :: reOpen(void)
{
	std::cout<<"Cam Close Port"<<std::endl;
	int error  = pfDeviceClose(port_);
	if(error<0)
	{
	   std::cout << "Camera Close CamPort  - " << pfGetErrorString(error) << std::endl;
	   return -1;
	}
	else
	{
		std::cout<<"Cam Port closed"<<std::endl;
	}


    error = pfDeviceOpen(port_);
	if (error < 0)
	{
	    std::cout << "Camera Device Open  - " << pfGetErrorString(error) << std::endl;
	    return -1;
	}
	else
	    std::cout << "Camera Device reopened " <<std::endl;

	return(0);


}

/**
 * @brief Port der Kamerakommunikation leeren
 */
int Camera :: flushPort(void)
{
    int error = 0;
	std::cout<<"Flush Communication Port"<<std::endl;
    error  = pfFlushPort(port_);
	std::cout<<"Return Value pfFlushPort: "<<error<<std::endl;
	if(error<0)
	{
	   std::cout << "Flush Communication Port  - " << pfGetErrorString(error) << std::endl;
	   return -1;
	}
	else
	{
		std::cout<<"Communication Port flushed"<<std::endl;
	}

  	return(0);

}


/**
* @brief property ueber String an Kamera senden
*
* \param   port - Kameraport
* \param   property - Eigenschaft
* \param  val - Wert
* \return  error Fehlernummer
*
 */

int Camera::SetPropertyString(int port, char* property, char* val)
{
	TOKEN t;
	int error=0;

	//printf("Set property %s: %s\n", property, val);
	t = pfProperty_ParseName(port, property);
	if(t == INVALID_TOKEN){
		std::cout << "SetPropertyString: Property not found or bad index: (" << property << ")\n" << std::endl;
		error = -1;
	}
	else{
		error = pfDevice_SetProperty_String(port, t, val);
		if(error < 0)
		{
			handleError(error);
			//Test, Anweisung Hofmann, Photonfocus:
			//PortFlush(); ist siso Function...

		}
	}
	return error;
}

/**
* @brief property ueber String aus Kamera lesen
*
* \param   port - Kameraport
* \param   property - Eigenschaft
* \param  val - Wert
* \return  error Fehlernummer
*
 */
int Camera::GetPropertyString(int port, const char* property, char* val)
{
	TOKEN t;
	int error=0;

	std::string propertyString = property;
	
	//printf("Get property %s\n", property);
	t = pfProperty_ParseName(port, property);
	if(t == INVALID_TOKEN){
		std::cout << "GetPropertyString: Property not found or bad index: (" << property << ")\n" << std::endl;
		error = -1;
	}
	else{
		error = pfDevice_GetProperty_String(port, t, val, 64);
		std::string valueString=val;

		//std::cout<<"GetPropertyString: "<<propertyString<<" value: "<<valueString<<std::endl;

		if(error < 0){
			handleError(error);
		}
	}
	return error;
}




/**
* @brief liefert typ des properties
* photonfocus kennt jede Menge Datentypen, wir nur wenige,
* deshal bmappen wir die pf typen auf unsere
*
* \param   port - Kameraport
* \param   property - Eigenschaft
* \param  val - Wert
* \return  propertytyp als int
*
 */
Types Camera::GetPropertyType(int port, const char* property)
{
	TOKEN t;
	int error=0;
	PropertyType pfType=PF_INT;
	Types type=TInt;

//	typedef enum {
//		PF_INVALID,
//		PF_ROOT,     ///< ROOT NODE TYPE
//		PF_INT,      ///< A 32 bit signed integer
//		PF_FLOAT,    ///< A 4 byte float, single precision
//		PF_BOOL,     ///< A boolean value (1: true, 0: false)
//		PF_MODE,     ///< A mode value. Only the values in the choice are valid.
//		PF_REGISTER, ///< A register value (direct register export)
//		PF_STRING,   ///< A string value (constant or fixed lengh, 0 terminated)
//		PF_BUFFER,   ///< A buffer value. The length is specified in the len field.
//		// Meta types:
//		PF_STRUCT,   ///< A struct value containing other properties
//		PF_ARRAY,    ///< An array value, containing a struct or property
//		// Special:
//		PF_COMMAND,  ///< A command
//		PF_EVENT    ///< An event node.
//	} PropertyType;

	//printf("get type of property %s:\n", property);
	t = pfProperty_ParseName(port, property);
	if(t == INVALID_TOKEN){
		std::cout << "GetPropertyType: Property not found or bad index: (" << property << ")\n" << std::endl;
		error = -1;
	}
	else{
		pfType = pfProperty_GetType(port, t);
	}

	if(error < 0)
		handleError(error);

	switch(pfType)
	{
	case(PF_INT):
	{
		type = TInt;
	}
	break;
	case(PF_FLOAT):
	{
		type = TFloat;
	}
	break;
	case(PF_STRING):
	{
		type = TString;
	}
	break;
	case(PF_MODE):
	{
		type = TOpMode;
	}

	break;

	default:
		break;

	}//switch


	return type;
}

/**
* @brief int property an Kamera uebertragen
*
* \param   port - Kameraport
* \param   property - Eigenschaft
* \param  val - Wert
* \return  error Fehlernummer
*
 */
int Camera::SetPropertyInt(int port, char* property, int val)
{
	TOKEN t;
	int error=0;
	PFValue v;

	//printf("Set property %s: %d\n", property, val);
	t = pfProperty_ParseName(port, property);
	if(t == INVALID_TOKEN){
		std::cout << "SetPropertyInt: Property not found or bad index: (" << property << ")\n" << std::endl;
		error = -1;
	}
	else{
		v.type = pfProperty_GetType(port, t);
		v.value.i = val;
		error = pfDevice_SetProperty(port, t, &v);
		if(error < 0){
			handleError(error);
		}
	}
	return error;
}


/**
* @brief float property an Kamera uebertragen
*
* \param   port - Kameraport
* \param   property - Eigenschaft
* \param  val - Wert
* \return  error Fehlernummer
*
 */
int Camera::SetPropertyFloat(int port, char* property, float val)
{
	TOKEN t;
	int error=0;
	PFValue v;

	//printf("Set property %s: %f\n", property, val);
	t = pfProperty_ParseName(port, property);
	if(t == INVALID_TOKEN){
		std::cout << "SetPropertyFloat: Property not found or bad index: (" << property << ")\n" << std::endl;
		error = -1;
	}
	else{
		v.type = pfProperty_GetType(port, t);
		v.value.i = val;
		error = pfDevice_SetProperty(port, t, &v);
		if(error < 0){
			handleError(error);
		}
	}
	return error;
}




/**
* @brief int property aus Kamera lesen
*
* \param   port - Kameraport
* \param   property - Eigenschaft
* \param  val - Wert
* \return  error Fehlernummer
*
 */
int Camera::GetPropertyInt(int port, char* property, int* val)
{
	TOKEN t;
	int error;
	PFValue v;

	error = 0;
	//printf("Get property %s\n", property);
	t = pfProperty_ParseName(port, property);
	if(t == INVALID_TOKEN){
		std::cout << "GetPropertyInt: Property not found or bad index: (" << property << ")\n" << std::endl;
	}
	else{
		error = pfDevice_GetProperty(port, t, &v);
		if(error < 0){
			handleError(error);
		}
		*val = v.value.i;
	}
	return error;
}


/**
*  @brief float property aus Kamera lesen
*
* \param   port - Kameraport
* \param   property - Eigenschaft
* \param  val - Wert
* \return  error Fehlernummer
*
 */
int Camera::GetPropertyFloat(int port, char* property, float* val)
{
	TOKEN t;
	int error;
	PFValue v;

	error = 0;
	//printf("Get property %s\n", property);
	t = pfProperty_ParseName(port, property);
	if(t == INVALID_TOKEN){
		std::cout << "GetPropertyFloat: Property not found or bad index: (" << property << ")\n" << std::endl;
	}
	else{
		error = pfDevice_GetProperty(port, t, &v);
		if(error < 0){
			handleError(error);
		}
		*val = v.value.i;
	}
	return error;
}





/**
 * @brief Kamerakommunikationsstatus
 * Kamera status der Kommunikation
 */
class Completion
{
private:
	const std::string function_;
	const std::string working_;
	const int max_;
		  int c;

public:
	Completion(const std::string& function, int max)
	:	function_(function),
		working_("/-\\|"),
		max_(max),
		c(0)
	{
	}

	int set(int state)
	{
		const int percent = 100 * state / max_;
		std::cerr << function_.c_str() << "..." << working_[c] << "   " << percent << "%\r";
		c = (c+1) % 4;
		return percent;
	}

	void done(void)
	{
		std::cerr << "Done.                100%\r" << std::endl;
	}
}; // class




/**
 * @brief Testen ob und welche Kamera angeschlossen ist
 * Kamera Namen ausgeben
 */
int Camera ::test()
{

 	int error = 1;

 	char manu[256];

 	std::string camName;
 	TOKEN t = pfProperty_ParseName(0, "CameraName");
 	if(t == INVALID_TOKEN)
 		std::cout<<"Property CameraName not found or bad index"<<std::endl;
 	else
 	{

 		error = pfDevice_GetProperty_String(port_, t, manu, 50);
 		//char pointer wieder richtig auf string konvertieren...
 		if(error >= 0)
 		{
 			std::istringstream iss(manu);
 			iss >> camName;
 			//std::cout <<"Camera Name: "<< camName << std::endl;
 			if(camName == "FastTrack-160")
 			{
 				std::cout<<"Fast Track 160 Kamera angeschlossen"<<std::endl;
 			}
 			else if ((camName == "MV1-D1024E-C026-160") ||
			         (camName == "MV1-D1024E-C059-160"))
 			{
 				std::cout<<"Fibercam 160 Kamera angeschlossen"<<std::endl;
 			}
 			else if (camName == "MV1-D2048x1088-160")
 			{
 				std::cout<<"CMOSIS CMV2000 Kamera angeschlossen"<<std::endl;
 			}
 			else
 			{
 				std::cout<<"Achtung: Keine 160-FT Kamera angeschlossen"<<std::endl;
 			}

 			error = 1;

 		}
 		else
 		{
 			handleError(error);
 			return -1;
 		}
 	}
	// to do: gueltigen Namen vorraussetzen:
	// "MV-D1024-80"
	// "FastTrack-160"
	// "MV-D1024-80E"
	// vermutlich hat die fibercam einen neuen Namen

 	return error;
}



/**
* @brief Aktuelle Kamera FPGA Werte ins EEPROM schreiben
*
*/
void Camera :: saveConfig(void)
{

	char manu[256];
	int error = 0;
	TOKEN t = pfProperty_ParseName(port_, "StoreDefaults");
	if(t == INVALID_TOKEN)
	{
		 std::cout<<"Property StoreDefaults not found or bad index"<<std::endl;
	     error = -1;
	}
	else
	{
		 std::sprintf(manu,"%d",1);
		 error = pfDevice_SetProperty_String(port_,t, manu);
	}
	 if(error < 0)
	      handleError(error);

}



/**
* @brief Belichtungszeit setzen
*
* \param   usesPulseWidthExposure: Belichtungsmode
* \param   zeitInMikroSekunden : Belichtungszeit
*
*/
void Camera :: setExposure(bool usesPulseWidthExposure,int zeitInMikroSekunden)
{
	//exposure setzen mit einfacher Methode:

    if (usesPulseWidthExposure)
	{
		// Workaround, da Grab vom Produktwechsel nichts mitbekommt, oder??
	 	setExposureMode(1); // 1 = pulsweitenkontrollierte Belichtung
	}
	else
	{
		// Workaround, da Grab vom Produktwechsel nichts mitbekommt, oder??
	 	setExposureMode(0);
	 	char manu[256];
	 	TOKEN t = pfProperty_ParseName(0, "ExposureTime");
	 	if(t == INVALID_TOKEN) std::cout<<"Property ExposureTime not found or bad index"<<std::endl;

	 	//std::cout<<"Zeit vom mmi(mikroSekunden) : "<<zeitInMikroSekunden<<std::endl;

	 	float zeitInMiliSekunden = (float)zeitInMikroSekunden / 1000.0;

	 	std::sprintf(manu,"%f",zeitInMiliSekunden);
	 	int error = pfDevice_SetProperty_String(port_,t, manu);
	 	 if(error < 0)
	 		      handleError(error);

	 	//std::cout <<"Exposure token: " << t <<"Time: "<<manu<<" return value: "<<error<< std::endl;
    }

}

/**
 * @brief Belichtungszeit holen
 */
int Camera :: getExposure(void)
{

	char manu[256];
	TOKEN t = pfProperty_ParseName(0, "ExposureTime");
	if(t == INVALID_TOKEN) std::cout<<"Property ExposureTime not found or bad index"<<std::endl;
	pfDevice_GetProperty_String(port_, t, manu, 50);
	//std::cout<<"Exposure time :"<< manu << std::endl;

	float expTime = atof(manu);
	int zeitInMikroSekunden = int(float(expTime)* 1000.0);
	return  zeitInMikroSekunden;



}


/**
* @brief skim value setzen, Verstaerkung am Pixel AD Wandler
*
* \param   iSkimValue: skim Wert
* \return  0
*
*/
int Camera :: setSkimValue(int iSkimValue)
{
	//std::cout << " ip::FloatProperty set SkimValue to |" <<  iSkimValue << std::endl;
	// SkimValue existiert in FastTrack-Kameras nicht und wurde auch bisher nicht wirklich parametriert
	char manu[256];
	TOKEN t = pfProperty_ParseName(0, "Skim");
	if(t == INVALID_TOKEN) std::cout<<"Property  Skim not found or bad index"<<std::endl;

	std::sprintf(manu,"%d",iSkimValue);
 	int error = pfDevice_SetProperty_String(port_,t, manu);
 	if(error < 0)
 		handleError(error);

 	return 0;
}

/**
*  @brief skim Wert holen
*  \return  skim Wert
*
*/
int Camera :: getSkimValue(void)
{

	//PFValue val;
    // int von 0.0 bis 8.0
	char manu[256];

    TOKEN t = pfProperty_ParseName(0, "Skim");
	if(t == INVALID_TOKEN) std::cout<<"Property  Skim not found or bad index"<<std::endl;

    pfDevice_GetProperty_String(port_, t, manu, 50);
    //std::cout<<"skim voltage: "<< manu << std::endl;

    int skim = atoi(manu);

    return skim;
}


/**
 * @brief Offset setzen
 * Schwarzabgleich
 * Wert zwischen 0 und 4095, default wert liegt bei etwa 3345
 * return error
 */
int Camera :: setOffset(int iOffset)
{
	 char manu[256];
	 TOKEN t = pfProperty_ParseName(0, "Voltages.BlackLevelOffset");
	 if(t == INVALID_TOKEN) std::cout<<"Property  Voltages.BlackLevelOffset not found or bad index"<<std::endl;

	 std::sprintf(manu,"%d",iOffset);
	 int error = pfDevice_SetProperty_String(port_,t, manu);
	 if(error < 0)
      handleError(error);

	 //std::cout << "Offset token : " << t <<" Offset: "<<manu<<" return: "<<error<< std::endl;


    return error;
}

/**
 * @brief Offset auslesen
 * liefert Schwarzabgleich ( Global offset )
 * wert zwischen 0 und 4095
 */
int Camera :: getOffset(void)
{

	char manu[256];
	TOKEN t = pfProperty_ParseName(0, "Voltages.BlackLevelOffset");
	if(t == INVALID_TOKEN) std::cout<<"Property  Voltages.BlackLevelOffset not found or bad index"<<std::endl;

	pfDevice_GetProperty_String(port_, t, manu, 50);
	//std::cout<<"Offset: "<< manu << std::endl;

	int offset = atoi(manu);
	//string wandeln: atoi


	return int(offset);
}

/**
 * @brief Helligkeitsabgleich vom Hersteller auslesen
 * liefert Schwarzabgleich ( Global offset )
 * wert zwischen 0 und 4095
 */
int Camera :: getCalibOffset(void)
{

	char manu[256];
	TOKEN t = pfProperty_ParseName(0, "BlackLevelOffset");
	if(t == INVALID_TOKEN) std::cout<<"Property  BlackLevelOffset not found or bad index"<<std::endl;

	pfDevice_GetProperty_String(port_, t, manu, 50);
	//std::cout<<"Offset: "<< manu << std::endl;

	int offset = atoi(manu);
	//string wandeln: atoi


	return int(offset);
}
/**
*  @brief Framezeit setzen
*  macht nur Sinn falls Kamera im const. frame mode
*  wir haben CFR auf false
*  \param  iFrameTime Zeit eines Frames in ms
*  \return error
*/
int Camera :: setFrameTime(int iFrameTime)
{
	char manu[256];
	int error = 0;
	TOKEN t = pfProperty_ParseName(0, "FrameTime");
	if(t == INVALID_TOKEN)
	{
		std::cout<<"Property FrameTime not found or bad index"<<std::endl;
		error = -1;
	}
	else
	{
		std::sprintf(manu,"%d",iFrameTime);
		error = pfDevice_SetProperty_String(port_,t, manu);
	}
	 if(error < 0)
		 handleError(error);
	 //std::cout << "Offset token : " << t <<" Offset: "<<manu<<" return: "<<error<< std::endl;


    return error;
}

/**
 * liefert Frame Zeit in ms
 *
 */
int Camera :: getFrameTime(void)
{

	char manu[256];
	TOKEN t = pfProperty_ParseName(port_, "FrameTime");
	if(t == INVALID_TOKEN) std::cout<<"Property FrameTime not found or bad index"<<std::endl;


	pfDevice_GetProperty_String(port_, t, manu, 50);
	//std::cout<<"Frametime: "<<manu << std::endl;

	//milisekunden
	float ftime = atof(manu);

	//mikrosekunden
	int itime = int(ftime * 1.0e3);

	return (itime);
}


/**
 * z.Zt. nicht implementiert
 */
int Camera :: importSettings(const char *filename, int& endCounter )
{

    //unsigned char buf[EEPROM_BUFLEN];
    //unsigned char dirty[EEPROM_BUFLEN];

    std::cout << "import filename: " << filename << std::endl;

    // write eeprom from file
    std::cout << "reading EEPROM from file is not supported" << std::endl;

    return 0; //FileToEeprom( buf, dirty, EEPROM_BUFLEN, filename, endCounter );
}


/**
 * z.Zt. nicht implementiert
 */
int Camera :: exportSettings(const char *filename, int &endCounter) const
{
    //unsigned char buf[EEPROM_BUFLEN];

    std::cout << "export filename: " << filename << std::endl;

   	// dump eeprom in file oder dump fpga register in file
    std::cout << "Dumping to EEPROM is not supported" << std::endl;

    return 0; //EepromToFile( buf, EEPROM_BUFLEN, filename, endCounter );
}


/**
 * skim flag setzen, aus Kopatibilitaetsgruenden noch vorhanden
 * \param flag
 * \return 0
 */
int Camera :: setSkimFlag(int flag)
{
	// SkimFlag: Skim > 0 --> skimming ist an
	setSkimValue(flag);

	return 0;
}


/**
 * liefert skim flag
 */
int Camera :: getSkimFlag(void)
{
	// SkimFlag: 0 --> skim ist deaktiviert
	int dummy = getSkimValue();
	    if (dummy > 0)
		    return dummy;
	    else
	    	return 0;

}


/**
 * z.Zt. nicht implementiert
 */
int Camera :: setRotateFlag(int flag)
{
	// RotateFlag ??
	std::cout << "flag in camSetRotate: " << flag << std::endl;
    return 0;
}


/**
 * Setze const. Bildrate
 * \param flag - 1: true d.h. Bildrate konstant
 * \return error
 */
int Camera :: setConstFrameRate(int flag)
{
	char manu[256];
    int error = 0;

	 TOKEN t= pfProperty_ParseName(0, "Trigger.CFR");
	 if(t == INVALID_TOKEN)
	 {
		 std::cout<<"Property Trigger.CFR not found or bad index"<<std::endl;
         error = -1;
	 }
	 else
	 {
		 if( flag == 1)
		 {
			 std::cout << "set constant framerate" << std::endl;
			 std::sprintf(manu,"%d",1);
			 error = pfDevice_SetProperty_String(port_,t, manu);

		 }
		 else if(flag == 0)
		 {
			 std::cout << "set constant framerate off" << std::endl;
			 std::sprintf(manu,"%d",0);
			 error = pfDevice_SetProperty_String(port_,t, manu);
		 }
	 }
	 if(error < 0)
		 handleError(error);
	 //std::cout << "Offset token : " << t <<" Offset: "<<manu<<" return: "<<error<< std::endl;

	 return error;

}

/**
 * Belichtungsmode setzen
 * \param flag - 1: pulsweitenmoduliert; 0: Belichtinsgzeit konstant
 * \return error
 */
int Camera :: setExposureMode( int flag )
{
	 char manu[256];
	 TOKEN t=0;
	 int error = 0;
	 t = pfProperty_ParseName(0, "Trigger.LevelControlled");
	 if(t == INVALID_TOKEN)
	 {
		 std::cout<<"Property Trigger.LevelControlled not found or bad index"<<std::endl;
		 error = -1;
	 }
	 else
	 {
    	 if( flag == 1)
    	 {
    		 std::cout << "set pulswidth modulation" << std::endl;
    		 std::sprintf(manu,"%d",1);
    		 error = pfDevice_SetProperty_String(port_,t, manu);
    	 }
    	 else if(flag == 0)
    	 {
    		 std::cout << "set const. exposure time" << std::endl;
    		 std::sprintf(manu,"%d",0);
    		 error = pfDevice_SetProperty_String(port_,t, manu);
    	 }
	 }
	 if(error < 0)
		 handleError(error);
	 //std::cout << "Offset token : " << t <<" Offset: "<<manu<<" return: "<<error<< std::endl;

	 return error;
}



/**
 * liefert Belichtungsmodus:
 *  0 : camera Belichtungszeit
 *  1 : pulsweitenmoduliert
 */
int Camera :: getExposureMode(void)
{

	 char manu[256];
	 TOKEN t=0;
	 int mode = 0;
	 int error = 0;

	 t = pfProperty_ParseName(0, "Trigger.LevelControlled");
	  if(t == INVALID_TOKEN)
		  std::cout<<"Property Trigger.LevelControlled not found or bad index"<<std::endl;

	 error = pfDevice_GetProperty_String(port_, t, manu, 50);

	 if(error<0)
	 {
		 handleError(error);
		 return error;
	 }
	 else
	 {
		 mode =atoi(manu);
	 }
	return(mode);

}

/**
 * nicht implementiert
 */

int Camera :: getRotateFlag(void)
{
	// RotateFlag existiert in FastTrack-Kameras nicht und wurde auch bisher nicht wirklich parametriert
 	return 0;
}


/**
 * liefert das CFR  Flag
 * 0: const. framerate aus
 * 1: const. framerate an
 */
int Camera :: getConstFrameRate(void)
{
	 char manu[256];
	 TOKEN t=0;
	 t = pfProperty_ParseName(0, "Trigger.CFR");
	 if(t == INVALID_TOKEN) std::cout<<"Property Trigger.CFR not found or bad index"<<std::endl;
	 int error = 0;

	 error = pfDevice_GetProperty_String(port_, t, manu, 50);

	 if(error<0)
	 {
		 handleError(error);
		 return error;
	 }

	//std::cout<<"Constant frame rate "<<manu << std::endl;
	int mode =atoi(manu);

	return(mode);

}

/**
 * to do
 */

int Camera :: setLUT( unsigned char* lut,int number)
{
//	PFCameraValue value;
/*
    int status = 0;
    int lutdepth,nluts;
    unsigned int tmp;
*/
    std::cout << "camSetLUT" << std::endl;
/*
    status = pfReadBitMode(mCamera);

    switch(status)
    {
    	case(PFMODE_8BIT):
			cout << "8 bit mode" << endl;
            break;
		case(PFMODE_10BIT):
			cout << "10 bit mode" << endl;
            break;
        case(PFMODE_LUT):
        	cout << "LUT mode" << endl;
            break;
        case(PFMODE_2X):
			cout << "High Gain mode" << endl;
            break;
       	case(PFMODE_LFSR):
            cout << "Test mode" << endl;
            break;

        default: // hier verlassen wir einfach sang und klanglos die funktion ...
			return(0);
    }


    // switch to lut mode
    pfSetBitMode(mCamera,PFMODE_LUT);

    status= pfGetProperty(mCamera,"LUTFlags",&value);

    tmp      = (unsigned int)(value.value.i);
    lutdepth = (tmp & 0xf);
    nluts    = (tmp & 0xf0)>>4;

    cout << "lutFlags: " << value.value.i << endl
    	 << "lutdepth: " << lutdepth << endl
    	 << "nluts   : " << nluts << endl
    	 << "slotnumber: " << number << endl;

    // location :
    // sel = 1,2,3,4; sel = 0 --> PFLUT_RAM
    // where = PFLUT_RAM | (sel -1)

    int where=0;
    if(number==0)
    {
		where = PFLUT_RAM;
    }
	else if( number > 0)
    {
		where = PFLUT_EEPROM | number-1;  // die eeprom slots gehen von 0 bis 3
    }

	// number = 1,2,3,4
    // schreibe auf EEPROM Bank 1 ????
    //int where  = PFLUT_RAM | (number -1);

    pfWriteLUT(mCamera,where,lut);


	// Umschalten der Eepromn LUT Bank  in RAM ??

    // slotnumber mindestens 1 ... 4  ( intern 0...3 )
    //      value.value.i = slotnumber-1;
    //     pfSetProperty(c,"ActiveLUT",&value);
*/
	return 0;
}


/**
 * linlog1 linlog1 wert
 * linlog2 linlog2 wert
 * time1   linlog time 1 in 1/1000 der exp. time
 * time2   linlog time 2 in 1/1000 der exp. time
 *
 * return error
 *
 */
int Camera :: setLinLog(int linlog1,int linlog2,int time1, int time2)
{
	 char manu[256];
     int error = 0;

	 std::cout<<"linlog Werte: "<<linlog1<<" "<<linlog2<<" "<<time1<<" "<<time2<<std::endl;

	 // Neue Grenzwerte sinnvoll

	TOKEN t = pfProperty_ParseName(port_, "LinLog.Mode");
	if(t == INVALID_TOKEN) std::cout<<"Property LinLog.Mode not found or bad index"<<std::endl;
	std::sprintf(manu,"%d",4); //UserMode
	error = pfDevice_SetProperty_String(port_,t, manu);


	t = pfProperty_ParseName(port_,"LinLog.Value1");
	if(t == INVALID_TOKEN) std::cout<<"Property LinLog.Value1 not found or bad index"<<std::endl;
	std::sprintf(manu,"%d",linlog1);
	error = pfDevice_SetProperty_String(port_,t, manu);


	t = pfProperty_ParseName(port_,"LinLog.Value2");
	if(t == INVALID_TOKEN) std::cout<<"Property LinLog.Value2 not found or bad index"<<std::endl;
	std::sprintf(manu,"%d",linlog2);
	error = pfDevice_SetProperty_String(port_,t, manu) ;
	//ip::IntProperty value2("LinLog.Value2", camera_);


	t = pfProperty_ParseName(port_,"LinLog.Time1");
	if(t == INVALID_TOKEN) std::cout<<"Property LinLog.Time1 not found or bad index"<<std::endl;
	std::sprintf(manu,"%d",time1);
	error = pfDevice_SetProperty_String(port_,t, manu);


	t = pfProperty_ParseName(port_,"LinLog.Time2");
	if(t == INVALID_TOKEN) std::cout<<"Property LinLog.Time2 not found or bad index"<<std::endl;
	std::sprintf(manu,"%d",time2); //UserMode
	error = pfDevice_SetProperty_String(port_,t, manu);

	 if(error < 0)
		 handleError(error);

	 std::cout <<"set linlog beendet mit: " <<error << std::endl;

	 return error;

}

/**
 * Liefert Linlog Werte
 * \param linlog1 linlog1 wert
 * \param linlog2 linlog2 wert
 * \param time1   linlog time 1 in 1/1000 der exp. time
 * \param time2   linlog time 2 in 1/1000 der exp. time
 *
 * \return error
 *
 */
int Camera :: getLinLog(int &linLog1,int &linLog2,int &time1, int &time2)
{
	char manu[256];
	int error = 0;

	TOKEN t = pfProperty_ParseName(port_, "LinLog.Mode");
	if(t == INVALID_TOKEN) std::cout<<"Property LinLog.Mode not found or bad index"<<std::endl;
	error = pfDevice_GetProperty_String(port_, t, manu, 50);
	//std::cout << manu << std::endl;


	t = pfProperty_ParseName(port_, "LinLog.Value1");
	if(t == INVALID_TOKEN) std::cout<<"Property LinLog.Value1 not found or bad index"<<std::endl;
	error = pfDevice_GetProperty_String(port_, t, manu, 50);
	//std::cout << manu << std::endl;
	linLog1 = atoi(manu);



	t = pfProperty_ParseName(port_, "LinLog.Value2");
	if(t == INVALID_TOKEN) std::cout<<"Property LinLog.Value2 not found or bad index"<<std::endl;
	error = pfDevice_GetProperty_String(port_, t, manu, 50);
	//std::cout << manu << std::endl;
	linLog2 = atoi(manu);



	t = pfProperty_ParseName(port_, "LinLog.Time1");
	if(t == INVALID_TOKEN) std::cout<<"Property LinLog.Time1 not found or bad index"<<std::endl;
	error = pfDevice_GetProperty_String(port_, t, manu, 50);
	//std::cout << manu << std::endl;
	time1 = atoi(manu);


	t = pfProperty_ParseName(port_, "LinLog.Time2");
	if(t == INVALID_TOKEN) std::cout<<"Property LinLog.Time2 not found or bad index"<<std::endl;
	error = pfDevice_GetProperty_String(port_, t, manu, 50);
	//std::cout << manu << std::endl;
	time2 = atoi(manu);

	 if(error < 0)
			 handleError(error);

	return error;
}



/**
 *  checkt ob ein ROI Parameter geaendert wird,
 *  setzt dann den kompletten ROI
 * \param key  Parameter
 * \return true oder false
 *
 */
int Camera :: isRoiParameter(std::string const &key,int val)
{
	int error =0;
	if( key == "Window.X" )
	{
		camTmpRoiX_ = val;
		return -3;
	}
	if ( key == "Window.Y" )
	{
		camTmpRoiY_ = val;
		return -3;
	}
	if ( key == "Window.W" )
	{
		 camTmpRoiDX_ = val;
		 return -3;
	}
	if ( key == "Window.H" )
	{
		camRoiX_ 	= camTmpRoiX_;
		camRoiY_ 	= camTmpRoiY_;
		camRoiDX_ 	= camTmpRoiDX_;
		camRoiDY_ 	= val;
		error= setRoi( camRoiX_, camRoiY_, camRoiDX_, camRoiDY_, 1 );
		return error;
	}
	else
		return -1; // kein Roi Parameter
}


void Camera ::setCameraTmpROI(int &x,int &y, int&dx, int&dy)
{
	camTmpRoiX_ 	=x ;
	camTmpRoiY_ 	=y ;
	camTmpRoiDX_ 	=dx ;
	camTmpRoiDY_ 	=dy ;

}
/**
 * setRoi setzt den Kamera ROI
 * Das Flag useOffset regelt, ob der Offset-Parameter genutzt werden,
 * oder das  ROi zentriert wird.
 * Der Rueckgabe wert ist im Moment ein Dummy
 */
int Camera :: setRoi(int x, int y, int dx, int dy, bool useOffset)
{

	int error = 0;


	// einige Konsistenz-Checks

	// letzte beiden Bits loeschen da Breite % 4 == 0 sein muss
	dx = dx & (~3);

	//  Hoehe / Breite = 0  laesst auf Fehleingabe schliessen
	if (dx==0) dx = MaxSensorWidth;
	if (dy==0) dy = MaxSensorHeight;

	// wenn nicht aktiviert, dann Bild zentrieren
	if (!useOffset)
	{
		x = (MaxSensorWidth - dx) / 2;
		y = (MaxSensorHeight - dy) / 2;
	}

	// Offset - Breiten Konsistenz herstellen
	if (dx  > int(MaxSensorWidth) )
	{
		x = 0;
		dx = MaxSensorWidth;
	}
	else if (x+dx > int(MaxSensorWidth) )
		x = MaxSensorWidth - dx;

	if (dy  > int(MaxSensorHeight))
	{
		y = 0;
		dy = MaxSensorHeight;
	}
	else if (y+dy> int(MaxSensorHeight))
		y = MaxSensorHeight - dy;


	// ROI setzen, reihenfolge beachten, sonst kann u.U. nichts gesetzt werden
	char manu[256];
	TOKEN t = pfProperty_ParseName(port_, "Window.X");
	if(t == INVALID_TOKEN) std::cout<<"Property Window.X not found or bad index"<<std::endl;
	std::sprintf(manu,"%d",x); //UserMode
	error = pfDevice_SetProperty_String(port_,t, manu);

	t = pfProperty_ParseName(port_, "Window.Y");
	if(t == INVALID_TOKEN) std::cout<<"Property Window.Y not found or bad index"<<std::endl;
	std::sprintf(manu,"%d",y); //UserMode
	error = pfDevice_SetProperty_String(port_,t, manu);

	t = pfProperty_ParseName(port_, "Window.W");
	if(t == INVALID_TOKEN) std::cout<<"Property Window.W not found or bad index"<<std::endl;
	std::sprintf(manu,"%d",dx); //UserMode
	error  =pfDevice_SetProperty_String(port_,t, manu);

	t = pfProperty_ParseName(port_, "Window.H");
	if(t == INVALID_TOKEN) std::cout<<"Property Window.H not found or bad index"<<std::endl;
	std::sprintf(manu,"%d",dy); //UserMode
	error = pfDevice_SetProperty_String(port_,t, manu);


	if(error < 0)
	{
		 handleError(error);
		 return -1;
	}

	// tatsaechlich gesetzten Werte merken
	camRoiX_ = x;
	camRoiY_ = y;
	camRoiDX_= dx;
	camRoiDY_= dy;

	return error;
}

/**
 * getRoi liefert den Kamera Roi
 * Der Rueckgabe wert ist im Moment ein Dummy
 */
int Camera :: getRoi(int& x, int& y, int& dx, int& dy)
{

	int error = 0;
	char manu[256];
 	std::cout << "Camera::GetRoi"<< std::endl;



	TOKEN t = pfProperty_ParseName(port_, "Window.W");
	if(t == INVALID_TOKEN) std::cout<<"Property Window.W not found or bad index"<<std::endl;
	error = pfDevice_GetProperty_String(port_, t, manu, 50);
	dx = atoi(manu);

	t = pfProperty_ParseName(port_, "Window.H");
	if(t == INVALID_TOKEN) std::cout<<"Property Window.H not found or bad index"<<std::endl;
    error = pfDevice_GetProperty_String(port_, t, manu, 50);
    dy = atoi(manu);

	t = pfProperty_ParseName(port_, "Window.X");
	if(t == INVALID_TOKEN) std::cout<<"Property Window.X not found or bad index"<<std::endl;
	error = pfDevice_GetProperty_String(port_, t, manu, 50);
	x = atoi(manu);

	t = pfProperty_ParseName(port_, "Window.Y");
	if(t == INVALID_TOKEN) std::cout<<"Property Window.Y not found or bad index"<<std::endl;
	error = pfDevice_GetProperty_String(port_, t, manu, 50);
    y = atoi(manu);



	if(error < 0)
		 handleError(error);

 	std::cout << "Camera::getRoi: " << x << " : " <<  y << " : " <<  dx << " : " << dy << std::endl;
 	// interne member setzen:
 	camRoiX_ = x;
 	camRoiY_ = y;
 	camRoiDX_ = dx;
    camRoiDY_ = dy;

    //temporaere Roi Wert setzen
    camTmpRoiX_ = camRoiX_;
    camTmpRoiY_ = camRoiY_;
    camTmpRoiDX_ = camRoiDX_;
    camTmpRoiDY_ = camRoiDY_;
   

	return error;
}

/**
 * Laden der EEPROM Werte
*/
void Camera :: reset(void)
{

	char manu[256];
	TOKEN t = pfProperty_ParseName(port_, "Reset");
	if(t == INVALID_TOKEN) std::cout<<"Property Reset not found or bad index"<<std::endl;
	//std::cout << "token : " << t << std::endl;

	std::sprintf(manu,"%d",1); //UserMode
	int error = pfDevice_SetProperty_String(port_,t, manu);
	 if(error < 0)
			 handleError(error);


}


/**
 *    Beinhaltet das Property key ein Kommando ?
 *    Hier muessen zwingend alle Kommandos der Kamera, welche in der
 *    Kamera Eingenschaften liste hinterlegt sind abgefragt werden
 *    \param key Property
 *    \return true or false
*/
bool Camera::isCommand(std::string const & key)
{

  if(      (key == "StoreDefaults")
		  ||  (key == "Reset")
		  ||  (key == "FactoryReset")
  	  )
	  return true;
  else
	  return false;
}



/**
 * Setzen auf Auslieferungszustand
*/
void Camera :: factoryReset(void)
{

	char manu[256];
	TOKEN t = pfProperty_ParseName(port_, "FactoryReset");
	if(t == INVALID_TOKEN) std::cout<<"Property FactoryReset not found or bad index"<<std::endl;
	//std::cout << "token : " << t << std::endl;

	std::sprintf(manu,"%d",1);
	int error = pfDevice_SetProperty_String(port_,t, manu);
	 if(error < 0)
		 handleError(error);


}


/**
 * Trigger Modus setzen
 * \param trig 1: grabber Controlled Modus;0: free running
 */
void Camera::setTrigger(TriggerType trig)
{

	char manu[256];
	TOKEN t = 0;
    int error = 0;

	t = pfProperty_ParseName(port_, "Trigger.Source");
	if(t == INVALID_TOKEN) std::cout<<"Property Trigger.Source not found or bad index"<<std::endl;
	if( trig == 1)
	{
			std::cout << "grabber controlled" << std::endl;
			std::sprintf(manu,"%d",1);
			error = pfDevice_SetProperty_String(port_,t, manu);


	}
	 else if(trig == 0)
	 {
		std::cout << "free running" << std::endl;
		std::sprintf(manu,"%d",0);
		error = pfDevice_SetProperty_String(port_,t, manu);
	 }
	 if(error < 0)
		 handleError(error);
		 //std::cout << "Offset token : " << t <<" Offset: "<<manu<<" return: "<<error<< std::endl;

}

/**
 * z.Zt. nicht verfuegbar
*/
void Camera :: initDumpCache(unsigned char *cache, unsigned char *dirty, int n)
{/*
	memset(cache, 0xff, n);
    memset(dirty, DIRTY_SKIP, n);
    */
}


/**
 * z.Zt. nicht verfuegbar
*/
int Camera :: EepromToFile(unsigned char *buf, int buflen, const char *filename, int &endCounter ) const
{
/*    Completion competion("reading EEPROM", buflen);
    for(int i = 0;i < buflen;i++)
    {
		int val = pfReadEeprom(mCamera, i);
		if(val < PFOK)
		{
			printf("MReadEEPromAF->MReadEEPromAF=%x\n", val);
			return(val);
		}
		endCounter = competion.set(i);

		buf[i] = val;
	};
	competion.done();

    if (savePhotonFocusToFile( filename, buflen, buf))
    {
    	return PFOK;
    }
    else
    {
    	return PFERROR;
    };
 */
 return 0;
} // Camera :: EepromToFile



/**
 * z.Zt. nicht verfuegbar
*/
int Camera :: FileToEeprom(unsigned char *buf, unsigned char *dirty, int buflen, const char *filename, int &endCounter)
{

    std::cout<<"FileToEeprom not supported"<<std::endl;


    return 1;

} // Camera :: FileToEeprom


std::vector<std::uint8_t> Camera::getUserFlash(int startBit, int numBits)
{
    /*
     WRITE: 
     Um ein Byte zu schreiben, musst Du zuerst das UserFlash.Addr mit der richtigen Adresse schreiben und dann UserFlash.Data mit dem entsprechenden 8Bit Wert.
     Diese Prozedur muss Du für jedes neue Byte wiederholen.
     
     READ:
     Um ein Byte zu lesen, musst Du zuerst das UserFlash.Addr mit der richtigen Adresse schreiben und dann das Property UserFlash.Data lesen.
     Diese Prozedur muss Du für jedes neue Byte wiederholen.

 */
    std::vector<std::uint8_t> oResult;

    const int PORT = 0;
    TOKEN tokenAddress = pfProperty_ParseName(PORT, "UserFlash.Addr");
    TOKEN tokenData = pfProperty_ParseName(PORT, "UserFlash.Data");
    
    if (tokenAddress == INVALID_TOKEN || tokenData == INVALID_TOKEN)
    {
        std::cout << "UserFlash properties not found" << std::endl;
        return oResult;
    }
    
    PFValue pfvalAddress;
    pfvalAddress.type = pfProperty_GetType(PORT, tokenAddress);
    
    PFValue pfvalData;
    pfvalData.type = pfProperty_GetType(PORT, tokenData);
    
    std::cout << "getUserFlash from " << startBit << " to " << startBit + numBits -1 << std::endl;
    
    oResult.reserve(numBits);
    
    for (unsigned int counter = startBit, end = std::min(startBit + numBits, 2048); counter < end; ++counter)
    {
        pfvalAddress.value.i = counter;
        int error = pfDevice_SetProperty(PORT, tokenAddress, &pfvalAddress);
        if (error < 0)
        {
            std::cout << "Error when setting UserFlash address at position " << counter << std::endl;
            return std::vector<std::uint8_t> ();
        }
    
        error = pfDevice_GetProperty(PORT, tokenData, &pfvalData);
        if (error < 0)
        {
            std::cout << "Error when getting UserFlash data at position " << counter << std::endl;
            return std::vector<std::uint8_t> ();
        }
        
        std::uint8_t currentBitValue (pfvalData.value.i);
        oResult.push_back(currentBitValue);
    }
    
    return oResult;
    
}


bool Camera::setUserFlash(const std::vector<std::uint8_t> & rContent)
{
    /*
     WRITE: 
     Um ein Byte zu schreiben, musst Du zuerst das UserFlash.Addr mit der richtigen Adresse schreiben und dann UserFlash.Data mit dem entsprechenden 8Bit Wert.
     Diese Prozedur muss Du für jedes neue Byte wiederholen.
     
     READ:
     Um ein Byte zu lesen, musst Du zuerst das UserFlash.Addr mit der richtigen Adresse schreiben und dann das Property UserFlash.Data lesen.
     Diese Prozedur muss Du für jedes neue Byte wiederholen.

 */
    
    const int PORT = 0;
    TOKEN tokenAddress = pfProperty_ParseName(PORT, "UserFlash.Addr");
    TOKEN tokenData = pfProperty_ParseName(PORT, "UserFlash.Data");
    TOKEN tokenClear = pfProperty_ParseName(PORT, "UserFlash.Clear");
    
    if (tokenAddress == INVALID_TOKEN || tokenData == INVALID_TOKEN || tokenClear == INVALID_TOKEN)
    {
        std::cout << "UserFlash properties not found" << std::endl;
        return false;
    }

    //clear memory  
    PFValue pfvalClear;
    pfvalClear.type = pfProperty_GetType(PORT, tokenClear);
    pfvalClear.value.i = 1;
    pfDevice_SetProperty(PORT, tokenClear, &pfvalClear); 
   
    //write content
    PFValue pfvalAddress;
    pfvalAddress.type = pfProperty_GetType(PORT, tokenAddress);
    
    PFValue pfvalData;
    pfvalData.type = pfProperty_GetType(PORT, tokenData);
    auto * pContent = rContent.data();
    for (unsigned int counter = 0, end = rContent.size(); counter < end; ++counter, ++pContent)
    {
        pfvalAddress.value.i = counter;
        int error = pfDevice_SetProperty(PORT, tokenAddress, &pfvalAddress);
        if (error < 0)
        {
            std::cout << "Error when setting UserFlash address at position " << counter << std::endl;
            return false;
        }
    
        pfvalData.value.i = *pContent;
        error = pfDevice_SetProperty(PORT, tokenData, &pfvalData);
        if (error < 0)
        {
            std::cout << "Error when setting UserFlash data at position " << counter << std::endl;
            return false;
        }
            
    }
    return true;
}

