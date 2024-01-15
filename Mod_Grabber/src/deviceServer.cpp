/**
 * @file
 * @brief  Schnittstelle auf Framegrabber und Kamera
 *
 *
 * @author JS
 * @date   20.05.10
 * @version 0.1
 * Erster Wurf
 */

#include <iostream>
#include <string>
#include <unistd.h>
#include <cstdio>
#include <memory>

// poco includes
#include "Poco/File.h"
#include "Poco/Path.h"
#include "Poco/AutoPtr.h"
#include "Poco/Util/XMLConfiguration.h"
#include "Poco/Exception.h"



#include "../include/grabber/deviceServer.h"

#include"module/moduleLogger.h"


//#define DEVICESERVER_LOG


namespace precitec
{
namespace grabber
{

Poco::FastMutex DeviceServer::m_mutex;

//checkt obe ein key zum grabber gehoert
bool checkGrabber(std::string keyString);
bool readCameraConfigXML(baseCameraConfig& pCamValues);
bool writeCameraConfigXML(baseCameraConfig& pCamValues);


DeviceServer::DeviceServer(DataAcquire &grabber,CommandServer &triggerCmdServer):m_rGrabber(grabber),m_rTriggerCmdServer(triggerCmdServer),deviceOK_(false)
{
std::cout<<"Device Sertver CTOR..."<<std::endl;
}



/**
 * @brief initialisiert die Me4 Framegrabberkarte
 *
 *
 * Mit den Konfigurationsparametern wird die Bildgroesse gesetzt und
 * das Applet geladen. Bei Erfolg wird die Kamera initialisiert: Die Kommunikationsports
 * werden gescannt und der Port mit der Kamera wird geoeffnet
 *
 * \param config  Konfigurationsvektor
 * \param subDevice Karte die initialisiert wird
 * \return noch nix
 */
int DeviceServer::initialize(Configuration const& config, int subDevice)
{
	int xoff = 0;
	int yoff = 0;
	int width =  0;
	int height = 0;
    int grabberStatus = 0;
    //int camStatus = -1;

	std::cout<<"Starte grabber initialisierung..."<<std::endl;

	m_rGrabber.getImageSize(xoff, yoff, width, height);

	//initialise most exportant base parameters
	m_BaseCamConfig.xstart=xoff;
	m_BaseCamConfig.ystart=yoff;
	m_BaseCamConfig.width =width;
	m_BaseCamConfig.height=height;


	 //Set Exp.time, HW ROI aus file -- falls file vorhanden
	 //xml lesen und auf schreiben
	 bool xmlStatus = readCameraConfigXML(m_BaseCamConfig);
     if(xmlStatus)
     {
    	 width  = m_BaseCamConfig.width;
    	 height = m_BaseCamConfig.height;
     }
	/// Initialisierung: Applet laden,
	/// Kamera initialisieren
	/// Kamera ROI auslesen
	/// Speicher anlegen, FPGA Parameter setzen
	/// Image Klasse auf dma buffer legen
    grabberStatus = m_rGrabber.initDataAcquire(m_BaseCamConfig);


    /// Bilderfassung starten: grab starten, image-get thread starten, warten auf Trigger setzen
    if( grabberStatus >= 0)
    {
        Poco::FastMutex::ScopedLock lock(m_mutex);

		std::string sValue;
		m_rGrabber.getCameraParameter("Trigger.Source",sValue);

		std::istringstream is(sValue);
		int val;
		is >> val;
		std::cout<<"Trigger.Source: "<<val<<std::endl;
        if(val!=1)
        {
        	int mode = 1; // Interface Trigger
        	m_rGrabber.setCameraParameter("Trigger.Source",mode);
        }

       //Basis Kameraparameter setzen
        // falls Werte aus file vorhanden..
        if(m_BaseCamConfig.status)
        {
        	int error =0;
        	error = m_rGrabber.setCameraParameter("Voltages.BlackLevelOffset",m_BaseCamConfig.offset);
        	error = m_rGrabber.setCameraParameter("ExposureTime",m_BaseCamConfig.exposuretime);
        	if(error<0)
        	{
        		std::cout<<"Werte konnten nicht in die Kamera geschrieben werden "<<std::endl;
        	}
        }

    	// frames 0: Im SW Triggermode werden die Angaben nicht beruecksichtigt
       	grabberStatus = m_rGrabber.startDataAcquire(0);
       	if(grabberStatus<0)
       	{
       		std::cout<<"Framegrabber Start Bildeinzug fehlgeschlagen..."<<std::endl;
       		wmFatal( eImageAcquisition, "Fatal.Grabber.ImageAcq", "Framegrabber Start Bildeinzug fehlgeschlagen\n");
       		return grabberStatus;
       	}
       //std::cout << "DeviceServer::initialize: startDataAckquire ok" << std::endl;
    }
    else
    {
    	std::cout<<"Kamerainitialisierung fehlgeschlagen..."<<std::endl;
    	wmFatal( eImageAcquisition, "Fatal.Grabber.CameraInit", "Kamerainitialisierung fehlgeschlagen\n");
    	setDeviceOK_(false);
    	return -1;

    }
    usleep(100 * 1000);

    setDeviceOK_(true);


    /// Konfiguration aus Kamera laden und in MirrorKonfiguration schreiben:
    ///  Halten der Parameter in einer "Mirror" Konfiguration
    Configuration dummyConfig =  get(0);

    m_rGrabber.setMirrorConfiguration(dummyConfig);

    //wmLog( eInfo, "Framegrabber und Kamerainitialisierung erfolgt\n");
    wmLogTr(eInfo,"QnxMsg.Grab.FGKamInitialized","Grabber und Kamera O.k.\n");
    std::cout<<"Initialisierung des Device servers erfolgt..."<<std::endl;

    return 1;
}



/**
 * @brief prueft einen key auf grabber oder Kamera zugehoerigkeit
 *
 *
 * Ein grabber key muss Device im String beinhalten
 * sonst wird ein Kamera key angenommen.
 * Die Kamera key striongs heissen beliebig, Photonfocus hat da keine Regeln ...
 *
 * \param keyString string in dem Device gesucht wird
  * \return boll true: grabber oder false: Kamera
 */
bool checkGrabber(std::string keyString)
{
	std::string ::size_type idx;
	idx=keyString.find("Device");
	if(idx == std::string::npos)
	{
	    //Zeichen "Device" kommt im string nicht vor
	     return false;
	}
	else
	{
		// Die Property des keys beinhaltet Device, also liegt ein
		// grabber key vor
		return true;

	}

	return false;
}


/**
 * @brief prueft einen key auf bestimmte Zeichen
 *
 *
 * Ein Kamera PWM key kann die Kamera im live Betrieb aufhaengen
 * \param keyString string in dem Device gesucht wird
  * \return bool true: PWM im string
 */
int checkDangerousKey (std::string keyString)
{
	std::string::size_type found; // idx,idy;

	//Die PWMs explizit unterscheiden...
	found=keyString.find("PWM");
	if(found != std::string::npos)
		return 1;

	found=keyString.find("Exp");
	if(found != std::string::npos)
		return 2;

	return 0;


}


bool readCameraConfigXML(baseCameraConfig& pCamValues )
{
	bool status = true;
	//XML File lesen oder erzeugen:
	std::string oBaseDir = std::string(getenv("WM_BASE_DIR")) + "/config/";
	std::string oCameraFilename = "CameraConfig.xml";
	std::string oConfigFilename = oBaseDir + oCameraFilename;
	const std::string dummy(oConfigFilename);


	pCamValues.exposuretime = 0.0;
	pCamValues.offset =0;
	pCamValues.xstart =0;
	pCamValues.ystart = 0;
	pCamValues.width =0;
	pCamValues.height =0;
	pCamValues.status = false;

	File oConfigFile(dummy);

	std::cout<<"load: CameraConfigFile lesen und entsprechende keyValues auslesen "<<oConfigFilename<<std::endl;
	if (oConfigFile.exists() == true)
	{
		std::cout<<"----- config file vorhanden ----- "<<std::endl;
		AutoPtr<Util::XMLConfiguration> pConfIn;

		try { // poco syntax exception might be thrown or sax parse exception
				pConfIn	= new Util::XMLConfiguration( oConfigFilename );
		} // try
		catch(const Exception &p_rException)
		{
			std::cout<<"XML configuration kann nicht erstellt werden... "<<std::endl;

			return -1;
		} // catch
		//Auslesen:
		try
		{
			pCamValues.offset= pConfIn->getInt("Offset");
			std::cout<<"Offset aus file: "<<pCamValues.offset<<std::endl;
			pCamValues.exposuretime =static_cast<float>(pConfIn->getDouble("ExposureTime"));
			std::cout<<"ExposureTime aus file: "<<pCamValues.exposuretime<<std::endl;
			pCamValues.xstart= pConfIn->getInt("xStart");
			std::cout<<"xstart aus file: "<<pCamValues.xstart<<std::endl;
			pCamValues.ystart= pConfIn->getInt("yStart");
			std::cout<<"ystart aus file: "<<pCamValues.ystart<<std::endl;
			pCamValues.width= pConfIn->getInt("Width");
			if(pCamValues.width<=0)
			{
				pCamValues.width = 128;
			}
			std::cout<<"width aus file: "<<pCamValues.width<<std::endl;
			pCamValues.height= pConfIn->getInt("Height");
			if(pCamValues.height<=0)
			{
				pCamValues.height = 128;
			}

			std::cout<<"height aus file: "<<pCamValues.height<<std::endl;
			pCamValues.status = true;

		}
		catch(const Exception &p_rException)
		{
			std::cout<<"Can not find element in CameraConfig "<<std::endl;
			status = -1;
			pCamValues.status = false;
			return status;
		}

	}
	else
	{
		std::cout<<"Werte aus Kamera uebernehmen "<<std::endl;
		pCamValues.status = false;
		return -1;
	}
	return status;

}


bool writeCameraConfigXML(baseCameraConfig& pCamValues)
{
	bool status = true;

	//XML File lesen oder erzeugen:
	std::string oBaseDir = std::string(getenv("WM_BASE_DIR")) + "/config/";
	std::string oCameraFilename = "CameraConfig.xml";
	std::string oConfigFilename = oBaseDir + oCameraFilename;
	const std::string dummy(oConfigFilename);

	File oConfigFile(dummy);



	std::cout<<"----- config file schreiben ----- "<<std::endl;
	//AutoPtr<Util::XMLConfiguration> pConfOut;
	AutoPtr<Util::XMLConfiguration> pConfOut(new Util::XMLConfiguration);
	pConfOut->loadEmpty("CameraConfig");

	try
	{
		//pConfOut	= new Util::XMLConfiguration( oConfigFilename );
		//pConfOut->loadEmpty("CameraConfig");

		//AutoPtr<Util::XMLConfiguration> pConfOut(new Util::XMLConfiguration);
		//pConfOut->loadEmpty("CameraConfig");


		pConfOut->setInt	("Offset",pCamValues.offset);
		pConfOut->setDouble ("ExposureTime",pCamValues.exposuretime);
		pConfOut->setInt    ("xStart",pCamValues.xstart);
		pConfOut->setInt    ("yStart",pCamValues.ystart);
		pConfOut->setInt    ("Width",pCamValues.width);
		pConfOut->setInt    ("Height",pCamValues.height);
	}
	catch(const Exception &p_rException)
	{
		std::cout<<"Exception: xml config konnte nicht angelegt werden "<<std::endl;
		return false;
	}


	if( oConfigFile.exists() == true )
	{
		try{
			std::cout<<"----- config file vorhanden ----- "<<std::endl;
			pConfOut->save(oConfigFile.path());
		}
		catch(const Exception &p_rException)
		{
			std::cout<<"in config file schreiben nicht moeglich "<<std::endl;
			return false;
		}
	}
	else
	{
		std::cout<<"config file anlegen "<<std::endl;
		try{
			status = oConfigFile.createFile();
			pConfOut->save(oConfigFile.path());
		}
		catch(const Exception &p_rException)
		{
			std::cout<<"can not create config file "<<std::endl;
			return false;
		}
	}

	return status;
}

bool DeviceServer::checkAndSetGlobalParameter(SmpKeyValue keyValue)
{
    ValueTypes 	keyType  = keyValue->type();
    std::string 		keyString =keyValue->key();

	if(keyString=="TestimagesPath")
    {
    	if (keyType == TString)
    	{
    		std::string pathName = keyValue->value<std::string>();
    		m_rTriggerCmdServer.setTestImagesPath(pathName);
   			return true;
    	}
    }
    else if (keyString == "TestProductInstance")
    {
        if (keyType == TString)
        {
            m_rTriggerCmdServer.setTestProductInstance(Poco::UUID{keyValue->value<std::string>()});
            return true;
        }
    }
	return false;
}

/**
 * @brief setzt einen Wert ueber das KeyValue Verfahren
 *
 *
 * keyValue enthaelt den Identifikationsstring ( = key), den typ und
 * in der abgeleiteten Klasse den Wert.
 * Mit dem key kann der Parameter auf die Kamera geschrieben werden.
 *
 * Fuer die Datenbank/GUI wird zwischen Kamera und Grabber nicht unterschieden.
 * Ueber den key kann an Kamera oder Grabber gesendet werden. Der Grabber key hat immer
 * FG ( standard ) oder Device (Applet) am Anfang
 *
 *
 * \param keyValue  objekt mit key und value
 * \param subDevice Karte die beschrieben wird
 * \return KeyHandle koennte der Zugriffhandle ( token ) der Kamera fuer den Parameter sein
 */
KeyHandle DeviceServer::set(SmpKeyValue keyValue, int subDevice)
{

    
    #ifdef DEVICESERVER_LOG
    std::cout << __FUNCTION__ << " " << std::string(  keyValue.isNull() ? std::string("NULL") :  keyValue->key() ) <<  " locking \n";
    #endif
    
    Poco::ScopedLockWithUnlock<Poco::FastMutex>  lock(m_mutex);
    
    #ifdef DEVICESERVER_LOG
    std::cout << __FUNCTION__ << " " << std::string(  keyValue.isNull() ? std::string("NULL") :  keyValue->key() ) <<  " locked \n";
    #endif
    
    ValueTypes 	keyType  = keyValue->type();
    std::string 		keyString =keyValue->key(); //string
    std::string      valueString;
    int 		error = 0;
    bool        liveStop=false;
    int         liveInterrupted = 0;

    std::ostringstream os;

    if( checkAndSetGlobalParameter(keyValue) )
    {
	  return KeyHandle(1);
    }

	if(!deviceOK_)
		return KeyHandle(); // setzt handle auf -1

	// grabber oder Kamera
    bool isGrabber = checkGrabber(keyString);

    //Falls ein property (-->key) sich mit dem livemode nicht vertraegt
	//wird der hier unterbrochen
	//aktuell PWMs:1 und Exptime: 2
	int dKey = checkDangerousKey(keyString);
	if( dKey>0 )
	{
		//std::cout<<"gesuchter string im key ..."<<std::endl;
		//timer stoppen ...
		liveInterrupted =  m_rTriggerCmdServer.cancelSpecial(std::vector<int>(1, 0));

		if(liveInterrupted==1)// live mode lief und wurde angehalten
		{
            usleep(70 * 1000); // gibt Zeit das letzte Bild zu uebertragen
			liveStop = true;
		}

	}

	// Die folgenden Kommandos sollen nicht in der Liste mit get auftauchen
	// und werden nicht in der keyValue Liste gehalten
	// --> sie werden direkt ausgefuehrt
	if(keyString=="StoreDefaults")
    {
    	if(keyType == TInt)
    	{
            int val = keyValue->value<int>();
            error = m_rGrabber.setCameraParameter(keyString,val);
        
            if (error < 0)
    			return KeyHandle();
    		else
    	    {
              lock.unlock(); // otherwise deadlock in getCameraBaseConfig 
              getCameraBaseConfig();
    	      bool status = writeCameraConfigXML(m_BaseCamConfig );
			  if(!status)
			  {
				wmLog( eInfo, "Could not write camera XML file\n");
				std::cout<<"XML file konnte nicht erzeugt werden..."<<std::endl;
			  }
    		  return KeyHandle(1);
    	    }
    	}
    }
    else if ( (keyString == "FTTestImage.Address") || (keyString == "FTTestImage.Data") ) // PhotonFocus Parameter fehlerhaft
    {

    	if (keyType == TInt)
    	{
    		int oVal = keyValue->value<int>();
    		error = m_rGrabber.setCameraParameter(keyString, oVal);
    		if (error < 0)
    		{
    			return KeyHandle();
    		}
    		return KeyHandle(1);
    	}
    }
    else if (keyString == "FTTestImage.Enable")
    {
    	if (keyType == TInt)
    	{
    		int oVal = static_cast<int>(keyValue->value<int>());
    		std::cout << "set FTTestImage.Enable, new value: " << oVal << std::endl;
    		error = m_rGrabber.setCameraParameter(keyString, oVal);
    		if (error < 0)
    		{
    			return KeyHandle();
    		}
    		return KeyHandle(1);
    	}
    }
    else if (keyString == "FileRead2")
    {
    	if (keyType == TString)
    	{
    		std::string oFilename = keyValue->value<std::string>();
    		error = m_rGrabber.setCameraParameter(keyString, oFilename);
    		if  (error < 0)
    		{
    			return KeyHandle();
    		}
    		return KeyHandle(1);
    	}
    }
    else if (keyString == "UserFlash.Clear")
    {
        //this key should not be set by the user in the device page
        wmLog(eDebug, "Clear UserFlash\n");

        error = m_rGrabber.setCameraParameter(keyString,0);

        if (error < 0)
            return KeyHandle();
        else
            return KeyHandle(1);
    }
    else if (keyString == "UserFlash.Addr")
    {
        int oVal = static_cast<int>(keyValue->value<int>());
        error = m_rGrabber.setCameraParameter(keyString, oVal);
        if (error < 0)
        {
            return KeyHandle();
        }
        return KeyHandle(1);
    }
    else if ( keyString.substr(0,14) == "UserFlash.Data" )
    {
        //this key should not be set by the user in the device page
        //string starting with UserFlash.Data
        //for convenience, the "virtual" property UserFlash.DataX performs SetAddr(X) and GetData()
        int userFlashAddr = -1;
        std::string userFlashAddrString = keyString.substr(14,4);
        if ( userFlashAddrString == "")
        {
            return KeyHandle();
        }

        try
        {
            userFlashAddr = std::stoi(userFlashAddrString);
        }
        catch (std::invalid_argument&)
        {
            return KeyHandle();
        }
        catch(std::out_of_range&)
        {
            return KeyHandle();
        }
        if (userFlashAddr < 0  || userFlashAddr > 2048) // system::CamGridData::mBytesMaxAddress
        {
            std::cout << keyString << ": cannot convert address " << userFlashAddrString << std::endl;
            return KeyHandle();
        }
        auto error = m_rGrabber.setCameraParameter("UserFlash.Addr",userFlashAddr);
        if(error<0)
        {
            std::cout << "Error when requesting UserFlash.Addr " << userFlashAddr << std::endl;
            return KeyHandle();
        }

        int userFlashData = static_cast<int>(keyValue->value<int>());
        error = m_rGrabber.setCameraParameter("UserFlash.Data", userFlashData);
        if (error < 0)
        {
            return KeyHandle();
        }
        return KeyHandle(1);
    }
    else if (keyString == "UserFlash.Data" )
    {
        //this key should not be set by the user in the device page
        wmLog(eDebug, "Write camera UserFlash\n");
        int oVal = static_cast<int>(keyValue->value<int>());
        error = m_rGrabber.setCameraParameter(keyString, oVal);
        if (error < 0)
        {
            return KeyHandle();
        }
        return KeyHandle(1);
    }

    // Falls alle Parameter im mirror gehalten werden (sensorConfiguration_)
	// Der neue Wert wird im mirror gehalten. Falls er dort nicht existiert
	// oder ausserhalb der GRenzwerte liegt kommt ein Fehler zurueck ( -1)
	error = m_rGrabber.replaceSensorConfiguration(keyValue );
	if(error<0) // keyValue nicht in der Sensorkonfiguration oder ausserhalb Bereich
	{
		// Live u.u. wieder einschalten...
  		wmLog( eInfo, "Replace Sensor Configuration failed\n");
    	return KeyHandle();
	}

	//type Abfrage
	switch(keyType)
	{
	case(TInt):
	{
		//const TKeyValue<int> &v = dynamic_cast<const TKeyValue<int>&> (keyValue);
		//int dummy = v.value();
		int val = keyValue->value<int>();


		if(isGrabber)
		{
			error = m_rGrabber.setGrabberParameter(keyString,val);
		} else
		{
			error = m_rGrabber.setCameraParameter(keyString,val);
		}

		//integer auf ostringstream schieben...
		os << val;
		valueString = os.str();
		//grabber_.setCameraParameter(keyString,valueString);
	}
	break;

	case(TFloat):
	{

		float val = keyValue->value<float>();
		if(isGrabber)
		{
			error = m_rGrabber.setGrabberParameter(keyString,(double)val);
		} else
		{

			error = m_rGrabber.setCameraParameter(keyString,val);
			//ExpTime Special Handling with a dummy grab
			if((liveStop==false) && (dKey==2))
			{
				m_rGrabber.setImageNoUse(true);
				m_rGrabber.trigger();
			}
		}

		//float auf ostringstream schieben...
		os << val;
		valueString = os.str();
		//error = grabber_.setCameraParameter(keyString,valueString);
	}
	break;

	case(TString):
	{
		// z.B. Kommando
		std::string valueString = keyValue->value<std::string>();
		if(isGrabber)
		{
			error = m_rGrabber.setGrabberParameter(keyString,valueString);
		} else
		{
			error = m_rGrabber.setCameraParameter(keyString,valueString);
		}
	}
	break;

	default:
		break;
	}//switch



	//std::cout<<" vor burst again: "<<liveStop<<std::endl;
	//false live mode unterbrochen, hier wieder einschalten
	if(liveStop)
	{
		//std::cout<<"burst again ..."<<std::endl;
		m_rTriggerCmdServer.burstAgain();
		liveStop = false;
	}


	if (error < 0)
	{
		//wmLog()...
		std::cout<<"Key Value " << keyString << "Setting failed: "<<error<<std::endl;
		wmLog( eInfo, "Key Value %s Setting failed\n", keyString.c_str());
		return KeyHandle(); // setzt handle auf -1
	}
	else
		return KeyHandle(1);
}



/**
 * @brief liefert einen Wert ueber das KeyValue Verfahren
 *
 *
 * key enthaelt den Identifikationsstring ( = key)
 * Mit dem key kann der Parameter aus der Kamera gelesen werden.
 * Mit dem Identifikationsstring wird in einer Liste min,nax und default ermittelt
 * und dem keyVaalue hinzugefuegt
 *
 * \param keyValue  objekt mit key und value
 * \param subDevice Karte die beschrieben wird
 * \return SmpKeyValue Pointer auf KeyValue liefert key und Value zurueck
 */
//std::auto_ptr<KeyValue> DeviceServer::get(Key key, int subDevice)
SmpKeyValue DeviceServer::get(Key key, int subDevice)
{
    if ( m_rGrabber.grabberHasError() )
    {
        SmpKeyValue kv {new TKeyValue<int>{}}; // invalid KeyValue
        return kv;
    }

    #ifdef DEVICESERVER_LOG
    std::cout << __FUNCTION__ << " " << key <<  " locking \n";
    #endif
    
    Poco::FastMutex::ScopedLock lock(m_mutex);

    #ifdef DEVICESERVER_LOG
    std::cout << __FUNCTION__ << " " << key <<  " locked \n";
    #endif
    
    bool ignoreMirrorConfiguration = true;
    ValueTypes keyType;
    std::string strValue;

    SensorConfigVector confVector;
    //std::cout<<"Device Server get key "<<key<<std::endl;


    // StoreDefaults ausgenommen...schicke gueltigen keyValue zurueck
    // Kommandos abzufragen hat keine Sinn
    if(key == "StoreDefaults")
    {
    	return SmpKeyValue (new KeyValue(TInt,"StoreDefaults",1) );
    }

    /*
    if(key == "ReOpen")
    	return SmpKeyValue (new KeyValue(TInt,"ReOpen",1));

    */

    if (key == "FTTestImage.Enable")
    {
    	std::cout << " getting Testimage enabled flag" << std::endl;
    	return SmpKeyValue (new KeyValue(TInt, "FTTestImage.Enable"));
    }

    //the sensor size is not provided directly by the hardware
    if (key == "Window.WMax")
    {
        SmpKeyValue kv (new TKeyValue<int>("Window.WMax", m_rGrabber.getMaxSensorWidth()) );
        kv->setReadOnly(true);
        return kv;
    }
    if (key == "Window.HMax")
    {
        SmpKeyValue kv (new TKeyValue<int>("Window.HMax", m_rGrabber.getMaxSensorHeight()) );
        kv->setReadOnly(true);
        return kv;
    }


    if ( key.substr(0,14) == "UserFlash.Data" )
    {
        //string starting with UserFlash.Data
        //for convenience, the "virtual" property UserFlash.DataX performs SetAddr(X) and GetData()
        int userFlashAddr = -1;
        std::string userFlashAddrString = key.substr(14,4);
        if ( userFlashAddrString != "")
        {            
            try 
            {
                userFlashAddr = std::stoi(userFlashAddrString);
            }
            catch (std::invalid_argument&)
            {}
            catch(std::out_of_range&)
            {}
            if (userFlashAddr < 0  || userFlashAddr > 2048) // system::CamGridData::mBytesMaxAddress
            {
                std::cout << key << ": cannot convert address " << userFlashAddrString << std::endl;
                return SmpKeyValue (new KeyValue(TInt,key,-1) );
            }
            auto error = m_rGrabber.setCameraParameter("UserFlash.Addr",userFlashAddr);
            if(error<0)
            {
                std::cout << "Error when requesting UserFlash.Addr " << userFlashAddr << std::endl;
                return SmpKeyValue (new KeyValue(TInt,key,-1  ) );
            }
        }
        
        //at this point the address has been set, let's continue like a normal camera property
        key = "UserFlash.Data";
        ignoreMirrorConfiguration = true; //do not try to read this value from the mirror xml file
    }

    //Falls die mirror Conf besteht, Abfrage aus der Liste beantwortn, geht schneller
    if(m_rGrabber.isMirrorConfigrationOK() && !ignoreMirrorConfiguration)
    {
    	SmpKeyValue keyMirrorVal = m_rGrabber.getMirrorKeyValue(key);
    	return keyMirrorVal;
    }

    bool isGrabber = checkGrabber(key);

   	//Types getCameraParameter(string const& key,string &value);
    if(isGrabber)
    {
    	keyType = m_rGrabber.getGrabberParameter(key,strValue);
    	//key in cameraProperty Liste suchen und min,max,def ermitteln
    	confVector = m_rGrabber.getGrabberProperties();
    	//std::cout<<"get grabber key: "<<key<<std::endl;
    }
    else if (deviceOK_)
    {
    	keyType = m_rGrabber.getCameraParameter(key,strValue);
    	//cameraProperty Liste holen
    	confVector = m_rGrabber.getCameraProperties();
    	//std::cout<<"get camera key: "<<key<<std::endl;
    }
    else
    {
        keyType = TChar;
    }

     if(keyType == TUnknown)
     {
    	 // KeyValue(ValueTypes type, Key key, int handle)
         std::cout << "Unknown" << std::endl;
    	return SmpKeyValue (new KeyValue(TInt,"?",-1 ) );
     }

     std::string dummy = strValue;


     switch(keyType){
     case(TInt):
     {
    	 int val;
    	 int minInt=0,maxInt=0,defInt=0;
    	 //string nach int
    	 std::istringstream iss(dummy);
         iss>>val;
         for(unsigned int i=0;i<confVector.size();++i)
         {
        	 if( confVector[i]->property == key)
        	 {
        		 TSensorProperty<int> *dummyIntProp = dynamic_cast<TSensorProperty<int>*>( confVector[i]);
        		 minInt = dummyIntProp->min;
        		 maxInt = dummyIntProp->max;
        		 defInt = dummyIntProp->defValue;
        		 i=confVector.size();
        		 //std::cout<<"Key: "<<key<<" value: "<<val<<" min: "<<minInt<<" max: "<<maxInt<<" default: "<<defInt<<std::endl;
        	 }
         }

         return SmpKeyValue ( new TKeyValue<int>(key,val,minInt,maxInt,defInt) );

     }
   	 break;
     case(TFloat):
     {
    	 float val;
    	 int minFloat=0.0,maxFloat=0.0,defFloat=0.0;
    	 std::istringstream iss(strValue);
         iss >> val;

         for(unsigned int i=0;i<confVector.size();++i)
         {
        	 if( confVector[i]->property == key)
             {
                 		 TSensorProperty<float> *dummyFloatProp = dynamic_cast<TSensorProperty<float>*>( confVector[i]);
                 		 minFloat = dummyFloatProp->min;
                 		 maxFloat = dummyFloatProp->max;
                 		 defFloat = dummyFloatProp->defValue;
                 		 i=confVector.size();
                 		 //std::cout<<"Key: "<<key<<" value: "<<val<<" min: "<<minFloat<<" max: "<<maxFloat<<" default: "<<defFloat<<std::endl;
             }
         }

         return SmpKeyValue (new TKeyValue<float> (key,val,minFloat,maxFloat,defFloat) );

     }
   	 break;
     case(TString):
     {
    	 //TKeyValue<string> testKey(key,strValue);

    	 // cast auf Basis
    	 //keyVal =  dynamic_cast<KeyValue*> (&testKey);

    	 //keyVal.pKeyValue_ =  dynamic_cast<KeyValue*> (testKey);
    	 return SmpKeyValue (new TKeyValue<std::string> (key,strValue) );

    	 //return *keyVal;
     }
     break;

     default:
    	 break;

     }//switch

	//instanzieren mit: KeyValue(ValueTypes type, Key key, int handle)
    //KeyValue kv;
	//return kv;
	std::cout << "unrecognized type" << std::endl;
	return SmpKeyValue (new KeyValue(TInt,"?",-1  ) ); //(new KeyValue );

}


/**
 * @brief Liesst eine Konfiguration aus der Kamera aus
 * in From eines Vektors von KeyValues
 * Dazu wird die Liste der Properties geholt.Diese Properties werden aus der Kamera gelesen
 * und in die Konfiguration geschrieben
 *
 * \param subDevice: Kamera
 * \return Configuration Vektor mit KeyValues
 */
Configuration DeviceServer::get(int subDevice)
{

    //no lock here, since internally get is called 
	Configuration config;


	//falls vorhanden, liefere mirror Konfiguration
    if(m_rGrabber.isMirrorConfigrationOK() )
    {
    	config = m_rGrabber.getSensorConfiguration();
    	return config;
    }

	//StringSet::const_iterator pos;
	precitec::system::Timer timer;
	timer.start();

	//Hole die Konfiguration, welche in den CameraKeys hinterlegt ist
	SensorConfigVector cameraConfVector = m_rGrabber.getCameraProperties();

	//int tt = cameraConfVector.size();
	for(unsigned int i=0;i<cameraConfVector.size();++i)
	{
		//std::cout<<"DeviceServer: get Key: " << cameraConfVector[i]->property << std::endl;
		SmpKeyValue tmp = get(cameraConfVector[i]->property,subDevice);

		// Achtung falls handle
		//dummy = (int)tmp->handle();
		//std::cout<<"key : "<<tmp->key()<<std::endl;
		//std::cout<<"key handle: "<<dummy<<std::endl;
		if(tmp->key()!= "?")
			config.push_back( tmp );
		else
			std::cout<<" "<<cameraConfVector[i]->property<<" - key aus der Configuration nicht in der Kamera gefunden"<<std::endl;
	}


    config.push_back(get("Window.WMax"));
    config.push_back(get("Window.HMax"));

	SensorConfigVector grabberConfVector = m_rGrabber.getGrabberProperties();
	for(unsigned int j=0;j<grabberConfVector.size();++j)
	{
			//std::cout<<"DeviceServer: get Key: " << grabberConfVector[j]->property << std::endl;
			SmpKeyValue tmp = get(grabberConfVector[j]->property,subDevice);

			// Achtung falls handle
			//dummy = (int)tmp->handle();
			//std::cout<<"key : "<<tmp->key()<<std::endl;
			//std::cout<<"key handle: "<<dummy<<std::endl;
			if(tmp->key()!= "?")
				config.push_back( tmp );
			else
				std::cout<<"key der Konfiguration nicht gefunden"<<std::endl;
	}

	//std::cout<<"deviceServer Configuration: "<<config<<std::endl;
	timer.stop(); //ca. 32 ms
	long long us = timer.us();
	std::cout << "Device server time[us] elapsed: " << us << std::endl;

	// spiegelt aktuelle Kamerakonfiguration
	m_rGrabber.setMirrorConfiguration(config);

	return config;
}



/**
 * @brief initialisiert die Me4 Framegrabberkarte
 *
 *
 * Kameraport schliessen, Image thread beenden,
 * Speicher freigeben
 */
void DeviceServer::uninitialize()
{

    m_rGrabber.closeCamera();
    m_rGrabber.closeDataAcquire();

    std::cout<<"Device Server schliessen..."<<std::endl;

}


/**
 * @brief write essential key values in the member configuration of the device server
 */
void DeviceServer::getCameraBaseConfig()
{

    SmpKeyValue keyValue=get("ExposureTime",0);
    if(keyValue->isValueValid())
    {
        TKeyValue<float> *dummy = dynamic_cast<TKeyValue<float>*>( keyValue.get());
        m_BaseCamConfig.exposuretime = dummy->value();
        std::cout<<"exptime "<<m_BaseCamConfig.exposuretime<<std::endl;
    }

    keyValue=get("Voltages.BlackLevelOffset",0);
    if(keyValue->isValueValid())
    {
            TKeyValue<int> *dummy = dynamic_cast<TKeyValue<int>*>( keyValue.get());
            m_BaseCamConfig.offset = dummy->value();
            std::cout<<"offset "<<m_BaseCamConfig.offset<<std::endl;
    }
    keyValue=get("Window.X",0);
    if(keyValue->isValueValid())
    {
            TKeyValue<int> *dummy = dynamic_cast<TKeyValue<int>*>( keyValue.get());
            m_BaseCamConfig.xstart = dummy->value();
            std::cout<<"offset "<<m_BaseCamConfig.xstart<<std::endl;
    }

    keyValue=get("Window.Y",0);
    if(keyValue->isValueValid())
    {
            TKeyValue<int> *dummy = dynamic_cast<TKeyValue<int>*>( keyValue.get());
            m_BaseCamConfig.ystart = dummy->value();
            std::cout<<"offset "<<m_BaseCamConfig.ystart<<std::endl;
    }

    keyValue=get("Window.W",0);
    if(keyValue->isValueValid())
    {
            TKeyValue<int> *dummy = dynamic_cast<TKeyValue<int>*>( keyValue.get());
            m_BaseCamConfig.width = dummy->value();
            std::cout<<"width "<<m_BaseCamConfig.width<<std::endl;
    }

    keyValue=get("Window.H",0);
    if(keyValue->isValueValid())
    {
            TKeyValue<int> *dummy = dynamic_cast<TKeyValue<int>*>( keyValue.get());
            m_BaseCamConfig.height = dummy->value();
            std::cout<<"offset "<<m_BaseCamConfig.height<<std::endl;
    }

}

/**
 * @brief Re-Initialisiert die Me4 Framegrabberkarte
 * Der Bildthread wird runtergefahren, der GRab wird gestopped, der Speicher wird freigegeben
 * Dann wird der Speicher mit neuer Bildgroesse neu auf den Shared Memory gesetzt
 */
void DeviceServer::reinitialize()
{
#ifdef DEVICESERVER_LOG
    std::cout << __FUNCTION__ << " locking \n";
#endif
    Poco::FastMutex::ScopedLock lock(m_mutex);


#ifdef DEVICESERVER_LOG
    std::cout << __FUNCTION__ << " locked \n";
#endif  
	std::cout<<"Grabber neu initialisieren und  starten..."<<std::endl;

	// Test - reinitialize wird missbraucht ...
	m_rGrabber.grabberReset();

}



DeviceServer::~DeviceServer()
{
}

} 	// namespace grabber
}	// namespace precitec
