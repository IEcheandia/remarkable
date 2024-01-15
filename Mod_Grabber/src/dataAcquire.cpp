/**
 *
 * @defgroup Framegrabber Framegrabber
 * \section sec Dieses Modul stellt die Kamera und Grabber Funktionalitaet zur Verfuegung
 * Kamera und Grabber werden als eine Einheit gesehen, die Ansteuerung geschieht ueber ein key Value Verfahren
 *
 *
 * @file
 * @brief  Zugriff auf Framegrabber und Kamera
 * @copyright    Precitec Vision GmbH & Co. KG
 * @author JS
 * @date   20.05.11
 *
 *
 */

#include <iostream>
#include <vector>
#include "../include/grabber/dataAcquire.h"
#include "event/imageShMem.h" // wg Bild-Konstanten Hoehe, Breite, Pufferzahl, ...
#include "Poco/Delegate.h"
#include "common/connectionConfiguration.h"
#include "common/systemConfiguration.h"
#include "system/types.h"
#include "common/defines.h" // CameraType

#include"module/moduleLogger.h"

#define GRABBERRESET 1


namespace precitec
{

/// bis die Konfiguration steht, wird die Bildgroesse hier definiert
const int ANZ_IMAGE_PIXELS = greyImage::ImageWidth; //Interfaces/INclude/event/imageShMem.h
const int ANZ_IMAGE_LINES  = greyImage::ImageHeight;
const int X_OFFSET = 0;
const int Y_OFFSET = 0;




///DataAcquire initialisieren...
DataAcquire::DataAcquire(TSensor<EventProxy> &sensorProxy) :
		m_rSensorProxy(sensorProxy),
		camera_(0),
		imageWidth_(ANZ_IMAGE_PIXELS),
		imageHeight_(ANZ_IMAGE_LINES),
		imageXOff_(X_OFFSET),
		imageYOff_(Y_OFFSET),
		frameCnt_(0),
		grabTimer_("DataAcquisitionTimer"),
		configOK_(false),
		grabberCtrMode_(false),
		dbgOut_(false),
		acquireBusy_(false),
		overallImageCounter_(0)
{
    bool hasCamera{false};
    char* oEnvStrg = getenv((char *)"WM_STATION_NAME");
    if (oEnvStrg != nullptr)
    {
        if (strcmp("WM-QNX-PC", oEnvStrg) == 0)
        {
            hasCamera = SystemConfiguration::instance().getBool("HasCamera", true);
        }
    triggerContext_.HW_ROI_x0=imageXOff_;//KIH BAUSTELLE
    triggerContext_.HW_ROI_y0=imageYOff_;//KIH BAUSTELLE
    }
    if(hasCamera)
    {
        //imageWidth_ = 512; imageHeight_ = 512;
        imageWidth_ = ConnectionConfiguration::instance().getInt("CameraWidth",ANZ_IMAGE_PIXELS);
        imageHeight_= ConnectionConfiguration::instance().getInt("CameraHeight",ANZ_IMAGE_LINES);

        std::cout<<"Data Acquire anlegen mit "<<imageWidth_<<" mal "<<imageHeight_<<" Pixel"<<std::endl;
        frameGrabber_.imageEvent_+=Poco::delegate(this, &DataAcquire::onImageEvent);

        /// Bildgroesse muss mit Konstruktor festgelegt werden
        /// Auf dem grabber brauchen wir momentan keinen offset
        /// Es werden nur die grabber member width und height gesetzt
        frameGrabber_.setROI(imageWidth_ ,imageHeight_ );

        //std::cout<<"FrameGrabber Nr "<<frameGrabber_.onBoard()<<" angelegt"<<std::endl;
        std::cout<<"dbgOut_ CTOR "<<dbgOut_<<" angelegt"<<std::endl;
    }
}

DataAcquire::~DataAcquire()
{
	// onImageEvent-thread stoppen
	frameGrabber_.imageEvent_-=Poco::delegate(this, &DataAcquire::onImageEvent);
	// Siso-Grabber freigeben -> sollte die Wiederstartbarkeit herstellen
	frameGrabber_.freeGrabber();
}


/**
 *  @brief Bildnummer auf 0 setzen
 */
void DataAcquire::resetImageNumber()
{
	Poco::FastMutex::ScopedLock lock(imageGrabMutex_);
	frameCnt_ = 0;
}


/**
 *  @brief Bildnummer  setzen
 *  \param imgNo Bildnummer
 */
void DataAcquire::setImageNumber(int imgNo)
{
	Poco::FastMutex::ScopedLock lock(imageGrabMutex_);
	frameCnt_ = imgNo;
}



int DataAcquire::getImageNumberFromFrameGrabber(void)
{

	return(frameCnt_);

	//return(triggerContext_.imageNumber() );
}

/**
 *   @brief Poco event aus imageGetter thread
 *   onImageEvent sendet via den sensorProxy das Bild und den triggerContext
 *   an den Analyzer
 *
 * 	\param  pSender Klasse aus der der Event kommt
 *  \param  i Bild nummer im dma Speicher, vono 1 bis Anzahl Bilder im dma
 */
void DataAcquire::onImageEvent(const void* pSender, int& i)
{

	Poco::FastMutex::ScopedLock lock(imageGrabMutex_);

    //make a copy of the triggerContext and update image number (otherwise DataAcquire::trigger 
    //could change the image number in the meantime)
    
    TriggerContext oTriggerContext = triggerContext_;
	oTriggerContext.setImageNumber(frameCnt_++);
	oTriggerContext.sync(grabTimer_.us());

	if(dbgOut_)
		std::cout<< "onImageEvent: "<<grabTimer_ << " triggered image number: " <<triggerContext_.imageNumber()
        << "sending image number " << oTriggerContext.imageNumber()  <<std::endl; //Debug ausgabe

    imagesInBuffer_.push_back(overallImageCounter_);
    // safety buffer of 10 images so that it is unlikely that Grabber writes into images currently written by VideoRecorder
    while (imagesInBuffer_.size() >= std::size_t(frameGrabber_.getBufferNumber() - 10))
    {
        imagesInBuffer_.pop_front();
    }
    dmaList_[i].setImageId(overallImageCounter_);
    overallImageCounter_++;

	m_rSensorProxy.data(CamGreyImage, oTriggerContext, dmaList_[i] );

	if(dbgOut_)
		std::cout<<std::endl; // Debug Ausgabe

	grabTimer_.restart();

}


/// Hole die Bildgroesse
/**
 *  @brief Bildnummer auf 0 setzen
 *  \param x offset x
 *  \param y offset y
 *  \param dx Bildbreite
 *  \param dy Bildhoehe
 *  \return 1 kein Fehler
 */
int DataAcquire::getImageSize(int& x, int& y, int& dx, int& dy)
{
	x  = imageXOff_; y  = imageYOff_;
	dx = imageWidth_; dy = imageHeight_;
	return 1;
}


/**
 *  @brief grabber mit Breite und Hoehe initialisieren,
 *  FG Karte initialisieren, shared memeory anlegen und mappen,
 *  FPGA Parameter setzen, noch nichts starten -
 *  shared mem pointer auf dma Speicher setzen, mit diesem shared pointer image
 *  konstruieren
 *
 *  \param pBaseConfig exposuretime, offset, width and height read out of the camera config file
 *  \return fgSuccess_  kleiner 0 im Fehlerfall
 */
int DataAcquire:: initDataAcquire(baseCameraConfig& pBaseConfig)
{

	//wichtig fuer Speicherreservierung
	// setzen der Bildgroesse -- setzt nur member...
	 frameGrabber_.setROI(pBaseConfig.width,pBaseConfig.height);
	 frameGrabber_.bGrabberOnboard = true;
	 fgSuccess_ = 0;

	 //std::cout<<"grabber onboard: "<<frameGrabber_.bGrabberOnboard<<std::endl;
	 const int BoardNum = 0;
	 if (frameGrabber_.bGrabberOnboard)
	 {
		 // Hinter init verbirgt sich :
		 // board init
		 // shared mem init
		 fgSuccess_ = frameGrabber_.init(BoardNum);
		 switch (fgSuccess_)
		 {
			case -1:
			{
				 wmFatal( eImageAcquisition, "QnxMsg.Fatal.AppletInit", "Applet Initialisierung fehlerhaft\n");
				 std::cout<<"no grabber on board"<<std::endl;
			}
		 	break;
			case -2:
			{
				 wmFatal( eImageAcquisition, "QnxMsg.Fatal.NoGrabber", "Kein Framegrabber vorhanden, user mem. angelegt\n");
				 std::cout<<"mem allocation without grabber in user mem"<<std::endl;
			}
			break;

		 	case -3:
		 	{
		 		 wmFatal( eImageAcquisition, "QnxMsg.Fatal.SharedMem", "Shared memory kann nicht verwendet werden\n");
				 std::cout<<"shared memory allocation failure in grabber init"<<std::endl;
		 	}
			 break;

		 	default:
		 	{
		 		//wmLog( eInfo, "Framegrabber Initialisierung erfolgt\n");
		 		wmLogTr(eInfo,"QnxMsg.Grab.FGInitialized","Framegrabber Initialisierung erfolgt\n");
				std::cout<<"framegrabber initialized ok - fgSuccess: "<<fgSuccess_<<std::endl;
		 	}
		 	break;
		 }
	} else	{
		//Bildspeicher (kein DMA von Siso) anlegen
		fgSuccess_ = frameGrabber_.init(BoardNum);
		std::cout<<" kein DMA anlegen, fgSuccess: "<<fgSuccess_<<std::endl;
		fgSuccess_ = -2;
	}

	 // Kamera initialisieren
	 // ROI holen
	 // Speicher setzen
	 // FPGA PArameter setzen
	 if(fgSuccess_>=0)
	 {
		 // init camera,bei Fehler -1
		 camSuccess_ = initCamera();
		 //std::cout<<"Initialisierung camSuccess_ : "<<camSuccess_<<std::endl;
		 // ROI von XML file oder aus Kamera holen:
		 if(camSuccess_ > 0)
		 {

			//Kamera Name im Logger ausgeben:
			 char val[256];
			 camera_.GetPropertyString(0,"CameraName",val);
		     wmLog(eInfo,"Camera Type: %s\n",val);

		    //the cameraConfile file is checked if the width or height is smaller than 16
			if(pBaseConfig.status)
			{
				 camera_.setRoi(pBaseConfig.xstart,pBaseConfig.ystart,pBaseConfig.width,pBaseConfig.height,1);
				 imageXOff_   = pBaseConfig.xstart;
				 imageYOff_   =  pBaseConfig.ystart;
				 imageWidth_  = pBaseConfig.width;
				 imageHeight_ = pBaseConfig.height;
			}
			else
			{
				//if there is no camera config file, we check the values out of the camera
				camera_.getRoi(imageXOff_,imageYOff_,imageWidth_,imageHeight_);
			 	pBaseConfig.xstart=imageXOff_;
			 	pBaseConfig.ystart=imageYOff_;

			 	pBaseConfig.width=imageWidth_;
			 	if(pBaseConfig.width<=0)
			 	{
			 		pBaseConfig.width= 128;
			 	}
			 	pBaseConfig.height=imageHeight_;
			 	if(pBaseConfig.height<=0 )
			 	{
			 		pBaseConfig.height= 128;
			 	}

			}
			// initialize the camera TMP Roi
            camera_.setCameraTmpROI(imageXOff_,imageYOff_,imageWidth_,imageHeight_);
            std::ostringstream os;
            os<<imageXOff_<<","<<imageYOff_<<","<<imageWidth_<<","<<imageHeight_;
            std::string outstring =os.str();
            wmLogTr(eInfo,"QnxMsg.Grab.CamInitialized","Kamera Initialisierung erfolgt %s\n",outstring.c_str());
            std::cout<<"ROI on camera: "<<imageXOff_<<","<<imageYOff_<<","<<imageWidth_<<","<<imageHeight_<<std::endl;

			//grabber Variable setzen, Offset hier unnoetig
			frameGrabber_.setROI(imageWidth_,imageHeight_ );
			fgSuccess_ = frameGrabber_.setDMA(); // Fehler -1
			fgSuccess_ = frameGrabber_.initFGPAParameter(); // Fehler -1


		 }
		 else
		 {
			 imageWidth_ = pBaseConfig.width;
		     imageHeight_ = pBaseConfig.height;
		     wmFatal( eImageAcquisition, "QnxMsg.Fatal.CameraInit", "Kamera konnte nicht initialisiert werden\n");
			 return camSuccess_; // Zurueck mit -1
		 }
	 }

 	//**********************************************************************

     // shared pointer auf dma setzen, images konstruieren
	 if (fgSuccess_>=0)
	 {
		 image::TLineImage<unsigned char> dummy;
		 int bufferCtr = frameGrabber_.getBufferNumber();
		 image::Size2d size (imageWidth_,imageHeight_ );
		 for(int i=0;i<bufferCtr;++i )
		 {
			 ShMemPtr<byte> shPtr(frameGrabber_.imagePtr(i));
             dummy = image::TLineImage<unsigned char>(shPtr, size);
			 dmaList_.push_back(dummy);
		 }
		 grabTimer_.start();
		 std::cout << "initDataAcquire: ok" << std::endl;
	 }
	 else
	 {
		 wmFatal( eImageAcquisition, "QnxMsg.Fatal.GrabberDMAFGPA", "Framegrabber DMA/FPGA Fehler  \n");
		 std::cout<<"Framegrabber DMA/FPGA Fehler "<<std::endl;
	 }

	 return fgSuccess_;

};


int DataAcquire::closeDataAcquire(void)
{
	//thread beenden
	std::cout<<"thread beenden ..."<<std::endl;

	frameGrabber_.grabActive= false;
	frameGrabber_.live(0,0);
	usleep(100 * 1000);
	frameGrabber_.freeGrabber();

	return 1;
}



/// Kamera initialisieren
int DataAcquire:: initCamera()
{
	int defPort = 0;
	 // Kamera initialisieren
     if(fgSuccess_)
     {
		camSuccess_ = camera_.init(defPort);
		if(camSuccess_ >=0)
			camSuccess_ = camera_.test();
     }
	 return camSuccess_;
}


/// Kamera initialisieren
int DataAcquire:: closeCamera()
{
	 camera_.close();
     return 1;

}

/// Kamera ROI setzen
int  DataAcquire::setROICamera(int x, int y, int dx, int dy, bool useOffset )
{
	camSuccess_= camera_.setRoi(x, y, dx, dy, useOffset);

	//
	imageWidth_  = dx;
	imageHeight_ = dy;
	imageXOff_=x; // KIH BAUSTELLE
	imageYOff_=y; // KIH BAUSTELLE

	std::cout<<"setROICamera imageXOff,imageYOff,imageWidth,imageHeight: "<<imageXOff_ <<" "<<imageYOff_<<" "<<imageWidth_<<" "<<imageHeight_<<std::endl;

	return camSuccess_;
}

/// Bilderfassungsthread initialisieren
int DataAcquire:: initImageThread()
{
	int status = frameGrabber_.initImageThread();

	return status;
}


/**
 *  @brief Bildaufnahme mit get thread starten
 *  \param frames Anzahl Bilder
 *  \param framerate Bildrate
 *  \return error <0 Fehler
 */
int DataAcquire:: startDataAcquire(int frames)
{
	return frameGrabber_.start(frames);  // thread hochfahren, acquire starten
}


/**
 *  @brief Bildaufnahme stoppen
 *  thread stoppen bzw. verlassen, Grab stoppen
 *  \return error <0 Fehler
 */
int DataAcquire:: stopDataAcquire()
{
	// bild getter thread runterfahren
	// Fg_StopAcquireEx aufrufen
	frameGrabber_.stop();
	return 1;
}


/// Bilderfassungsthread starten
int DataAcquire:: startImageThread()
{
	frameGrabber_.startImageThread();
	return 1;
}


/// Bilderfassungsthread stoppen, grab stoppen, DMA freigeben
int DataAcquire:: stopAcquireMem()
{
	std::cout<<" Stop Acquire "<<std::endl;

	// jetzt soll kein live zugelassen werden
	acquireBusy_ = true;

	// bild getter thread runterfahren
	// Fg_StopAcquireEx aufrufen
	frameGrabber_.stop();

	// DMA Speicher freigeben, Shared mem bleibt
	frameGrabber_.freeImageMem();

	std::cout<<" Stop Acquire durchgefuehrt"<<std::endl;

	return 1;
}


/// DMA Speicher setzen,Bilderfassungsthread starten,Grab starten
int DataAcquire:: startAcquireMem(int frames)
{
	std::cout<<" Start Acquire "<<std::endl;

	frameGrabber_.setDMA();

	std::cout<<" Set DMA durchgefuehrt "<<std::endl;
    usleep(200 * 1000);

	frameGrabber_.start(frames);

	std::cout<<" Grabber Start durchgefuehrt "<<std::endl;

	// jetzt soll kann live zugelassen werden
	acquireBusy_ = false;

	return 1;
}



/// Einzelbild ohne Kontext antriggern - dummy Bild
int DataAcquire:: trigger (void)
{
	int ret = frameGrabber_.live(0,0);

    if(ret<0) //else
    {
        wmFatal( eImageAcquisition, "QnxMsg.Fatal.ImageAcquisition", "Frame Grabber liefert keine Bilder\n");
    }
	return ret;
}


/// Einzelbild antriggern
int DataAcquire:: trigger (TriggerContext const& context )
{
    if (context.imageNumber() < frameCnt_  )
    {
        wmLog(eWarning, "DataAcquire::Trigger: possible error in resetting counter for ImageContext: input triggerContext = %d, frameCnt = %d \n", context.imageNumber(), frameCnt_ );
    }
//	Poco::FastMutex::ScopedLock lock(mutex_);
	int ret = frameGrabber_.live(0,0);
	//if(ret>=0)
	//{
	//	std::cout<< "frameGrabber_.live(0,0) return code: " << ret << std::endl;
	//}
	if(ret<0) //else
	{
		wmFatal( eImageAcquisition, "QnxMsg.Fatal.ImageAcquisition", "Frame Grabber liefert keine Bilder\n");
	}
	triggerContext_ = context;
//	grabTimer_.restart();
//	std::cout<< grabTimer_ << " " << triggerContext_.imageNumber() << std::endl; Debug Ausgabe
	return 1;
}

/// grabbercontrolled triggern
int DataAcquire::trigger (TriggerContext const& context, TriggerInterval const& interval)
{
  return 1;
}


///liefert data type des Parameter des keys und den wert als String zurueck
Types DataAcquire::getGrabberParameter(std::string const & key, std::string &strKeyValue)
{
	int error = 0;

	Types type = TUnknown;
    std::ostringstream os;

    if(key=="Device1_Process0_Debug_Output")
    {
        type = TInt;
        int val=0;
        if(getDbgOut())
	    {
            val = 1;
	    }
        else
        {
            val = 0;
        }
        std::cout<<"dbgOut_: "<<val<<std::endl;
        os<<val;
        strKeyValue = os.str();

        return type;
    }

	// Parametertyp zurueckgeben -- SiSo kennt die Parameter nicht,
	// also in der Konfiguration nachschauen
	SensorConfigVector grabberConfig = getGrabberProperties();

	 for(unsigned int i=0;i<grabberConfig.size();++i)
	 {
		 if( grabberConfig[i]->property == key)
	     {
	       type =  grabberConfig[i]->propertyType;
	     }
	 }


	const char *cKey = key.c_str();

	 if(type == TInt)
	 {
		 int val =0;
		 //error = frameGrabber_.test();
	 	 error = frameGrabber_.getParameter(cKey,&val);      //getParameter(cKey,&val);
	 	 if(error<0)
	 		 std::cout<<"frameGrabber_.getParameter Error"<<std::endl;
	 	 //integer auf ostringstream schieben...
	 	 os << val;
	 	 strKeyValue = os.str();
	 }
	 else if (type==TFloat )
	 {
		float val = 0.0;
		//int getParameter(const char* name,T &value);
		error = frameGrabber_.getParameter(cKey,&val);
		if(error<0)
			 std::cout<<"frameGrabber_.getParameter Error"<<std::endl;

		//float auf ostringstream schieben...
		os << val;
		strKeyValue = os.str();
	 }

	 return type;
}


/// Setze Grabber ROI bzw. Image Buffer im Applet
// Parameter werden hier nicht mehr geprueft
// die keys haengen vom Applet ab...
int  DataAcquire::setROIGrabber(int x, int y, int dx, int dy,int offset )
{
	return frameGrabber_.setGrabberRoi(x, y, dx, dy, offset);
}




/// setze int Parameter
int DataAcquire::setGrabberParameter(std::string const& key, int const & value)
{
	if(key=="Device1_Process0_Debug_Output")
	{
	    setDbgOut(value);
        bool dummy =getDbgOut();
        std::cout<<"set GrabberParameter dbgOut: "<<dummy<<std::endl;
        return(1);
	}

	// Achtung Trigger Mode Umstellung abfangen:
	// Image get thread beenden, Fg_Acquire beenden
	// Trigger Mode aendern, Thread starten. Fg_Acquire darf bei grabber controlled mode erst mit dem start
	// aufgerufen werden.
	// enum TriggerMode {GrabberControlled=1,ExternSw_Trigger=2};
	if(key == "Device1_Process0_Trigger_TriggerMode")
	{
		std::cout<<"dataAcquire - Trigger Mode umstellen ...."<<std::endl;
		if ( value ==1 )
		{
			grabberCtrMode_ = true;
			//thread und grab stoppen
			frameGrabber_.stop();
			frameGrabber_.setGrabberModeFlag(true);
		}
		else if (value == 2 )
		{
			grabberCtrMode_ = false; // dann SW controlled
			//thread und grab stoppen
			frameGrabber_.stop();
			frameGrabber_.setGrabberModeFlag(false);
		}
	}

	//string auf char* konvertieren
	const char *cKey = key.c_str();
	return frameGrabber_.setParameter(cKey, value);
}

///setze double Parameter
int DataAcquire::setGrabberParameter(std::string const& key, double const & value)
{
	const char *cKey = key.c_str();
	return frameGrabber_.setParameter(cKey, value);
}

///setze string
int DataAcquire::setGrabberParameter(std::string const& key, std::string const & value)
{
	const char *cKey = key.c_str();
	return frameGrabber_.setParameter(cKey, value);
}




///liefert data type des Parameter des keys und den wert als String zurueck
Types DataAcquire::getCameraParameter(std::string const & key, std::string &strKeyValue)
{
	int error = -1;

	char val[256];
	//Kommandos abfangen ...
	if( camera_.isCommand(key))
	{
		  std::cout<<" Kommando Abfrage ..."<<std::endl;
		  strKeyValue = "1";
		  return TInt;
	}
	if(key=="CameraReOpen")
	{
		std::stringstream sstr;
		sstr<<reOpenFlag_;
		strKeyValue = sstr.str();
		std::cout<<"CameraReOpen Flag: "<<strKeyValue<<std::endl;
		return TInt;

	}
	if(key=="FlushPort")
	{
			std::stringstream sstr;
			sstr<<flushPortFlag_;
			strKeyValue = sstr.str();
			std::cout<<"Flush Port Flag: "<<strKeyValue<<std::endl;
			return TInt;

	}


	//string auf char* konvertieren
	const char *cKey = key.c_str();

	// Parametertyp zurueckgeben-- photonfocus arbeitet mit char* nicht mit string ...
	error = camera_.GetPropertyString(0, cKey,val);

    if(error<0)
    {
    	return TUnknown;
    }

	//char pointer wieder richtig auf string konvertieren...
	 std::istringstream iss(val);

	 //iss>>val;
	 //strKeyValue = val;
	 iss >> strKeyValue;

	// return type ist PropertyType von Photonfocus ???
	Types keyType = camera_.GetPropertyType(0,cKey);

	if(keyType == TOpMode)
	{
		//string Manipulation
		//Mode liefert einen string, mit einem "#" davor,
		//gesetzt wird der Mode aber ueber einen int
		//--> "#" wird entfernt:
		std::string ::size_type idx;
		idx=strKeyValue.find("#");
		if(idx == std::string::npos)
		{
		   //Zeichen "#" kommt im string nicht vor
            //std::cout<<"Mode ohne # "<<std::endl;
            return TUnknown;
		}
		else
		{
		 if(idx == 0)
			 strKeyValue.erase(0,1);
		}
		keyType = TInt;
	}

	return keyType;
}



/**
 * @brief setzt einen Wert ueber das KeyValue Verfahren mit
 * dem pf SDK ueber strings
 * \param key  keystring
 * \param value valuestring
 * \return error token
 */
int DataAcquire::setCameraParameter(std::string const& key, std::string const& value)
{
	//string auf char* konvertieren
	const char *cKey = key.c_str();
    const char *cValue = value.c_str();

	// der Port muss noch flexibel werden ...
    std::ostringstream sstr;
    sstr << key << ", " << value; // << std::endl;
    std::string str(sstr.str());
    wmLog(eInfo, "Camera Parameter changed %s\n",str.c_str() );


	return camera_.SetPropertyString(0,(char*)cKey,(char*)cValue);
}




int DataAcquire::grabberReset(void)
{

	std::cout<<"Grabber mit "<<imageWidth_<<"x"<<imageHeight_<<" neu initialisieren und  starten..."<<std::endl;
	std::cout<<"fg hat intern "<<frameGrabber_.getAnzImagePixels()<<"x"<<frameGrabber_.getAnzImageLines()<<std::endl;

	stopAcquireMem();

	startAcquireMem(0); // mit der 0 wird acquire mit lots of images gestartet

	// dmaList loeschen und neu setzen:
	if( dmaList_.size()> 0)
	{
	   std::cout<<"dmaList size: "<<dmaList_.size()<<std::endl;
	   dmaList_.clear(); //clear richtig ??
	}

	image::BImage dummy;
    int bufferCtr = frameGrabber_.getBufferNumber();

    std::cout<<"bufferCtr: "<<bufferCtr<<" "<<std::endl;

    image::Size2d size (imageWidth_,imageHeight_ );
	for(int i=0;i<bufferCtr;++i ) {
		 ShMemPtr<byte> shPtr(frameGrabber_.imagePtr(i));
		 dummy = image::BImage(shPtr, size);
		 dmaList_.push_back(dummy);
	 }
	std::cout<<"Neue dmaList size: "<<dmaList_.size()<<std::endl;

	return 0;
}




/**
 * \param key  keystring
 * \param value integer
 * \return error token
 */
int DataAcquire::setCameraParameter(std::string const& key, int& value)
{
	int error = 0;

	//Achtung falls am ROI gedreht wird, muessen einige Werte abgefangen werden
	//error = -3 :Hier wird bei X,Y und W nichts gemacht, nur bei H wird dann wirklich der ROI veraendert.
	//error = -2 : Roi setzen ging schief
	//error = -1 : kein Roi Parameter
		
	error = camera_.isRoiParameter(key,value);

	// error : -2 ROI setzen ging schief
	// error : -3 War nicht H, also muessen wir (noch) nix machen.
	
	if( error >= 0 ) // Roi wurde umgestellt, oder zumindest versucht ...
	{
        if (m_inspectionCmd)
        {
            m_inspectionCmd->signalNotReady(eImageAcquisition);
        }
		// grabber image Speicher anpassen...
		int x,y,dx,dy;
		//interne Werte holen
		camera_.getCamRoi(x,y,dx,dy);

        // ROI Werte immer aktuell halten
		imageWidth_ 	= dx;
		imageHeight_ 	= dy;
		imageXOff_		= x;
		imageYOff_		= y;

		std::ostringstream sstr;
		sstr <<imageXOff_<<", "<<imageYOff_<<","<<imageWidth_<<","<<imageHeight_;
		std::string str(sstr.str());
		wmLog(eInfo, "ROI changed %s\n",str.c_str() );
		std::cout << "ROI wurde umgestellt: " <<imageXOff_<<","<<imageYOff_<<" - "<< imageWidth_ << " x " << imageHeight_ << std::endl;


		error = setROIGrabber( 0, 0, dx, dy, 1 ); //keinen offset auf dem grabber setzen

		// Beim ROI sicherheitshalber die mirror Konfiguration anpassen:
		for( unsigned int i=0; i<sensorConfiguration_.size(); ++i )
		{
			if( sensorConfiguration_[i]->key() == "Window.X" )
			{
				sensorConfiguration_[i]->setValue<int>(x);
				if(dbgOut_)
				    std::cout << "ROI gesetzt: replace in mirror configuration - keyValue: " << sensorConfiguration_[i]->key() << " - " << key << std::endl;
			}

			if( sensorConfiguration_[i]->key() == "Window.Y" )
			{
				sensorConfiguration_[i]->setValue<int>(y);
				if(dbgOut_)
				    std::cout << "ROI gesetzt: replace in mirror configuration - keyValue: " << sensorConfiguration_[i]->key() << " - " << key << std::endl;
			}

			if( sensorConfiguration_[i]->key() == "Window.W" )
			{
				sensorConfiguration_[i]->setValue<int>(dx);
				if(dbgOut_)
				    std::cout << "ROI gesetzt: replace in mirror configuration - keyValue: " << sensorConfiguration_[i]->key() << " - " << key << std::endl;
			}

			if( sensorConfiguration_[i]->key() == "Window.H" )
			{
				sensorConfiguration_[i]->setValue<int>(dy);
				if(dbgOut_)
				    std::cout << "ROI gesetzt: replace in mirror configuration - keyValue: " << sensorConfiguration_[i]->key() << " - " << key << std::endl;
			}
		}

#ifdef GRABBERRESET
		// changing the image size on the sensor must be followed by an reorganization of the dma memory
        system::ElapsedTimer timer;
		grabberReset();
        wmLog(eDebug, "Grabber reset took %i ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(timer.elapsed()).count());
#endif
        if (m_inspectionCmd)
        {
            m_inspectionCmd->signalReady(eImageAcquisition);
        }

		return error;
	}
    if(error == -2)
    {
        wmLog(eInfo, "Could not change ROI\n");
        return error;
    }


    // ROI wurde noch nicht umgesetzt
	if(error == -3)
	  return 0;

	//ReOpen it ?
	if(key=="CameraReOpen")
	{
		std::cout<<"get Camera ReOpen Command: "<<value<< std::endl;
		if(value==1)
		{
			error = 0;
			int error =	camera_.reOpen();
			if(error<0)
		    {
			    std::cout<<" ReOpen failed"<<std::endl;
			    wmLog(eInfo, "ReOpen failed\n");
			    reOpenFlag_ = 0;
		    }
		    else
		    {
		    	reOpenFlag_ = 1;
		        std::cout<<"ReOpen successfull"<<std::endl;
		    }
		}
		else if(value!=1)
		{
			std::cout<<"No ReOpen"<<std::endl;
			error = 0;
		}

		return error;
	}

	if(key=="FlushPort")
	{
		std::cout<<"get Camera FlushPort Command: "<<value<< std::endl;
		if(value==1)
		{
			error = 0;
			int error =	camera_.flushPort();
			if(error<0)
		    {
			    std::cout<<" FlushPort failed"<<std::endl;
			    wmLog(eInfo, "FlushPort failed\n");
			    flushPortFlag_ = 0;
		    }
		    else
		    {
		    	flushPortFlag_ = 1;
		        std::cout<<"FlushPort successfull"<<std::endl;
		    }
		}
		else if(value!=1)
		{
				std::cout<<"No FlushPort"<<std::endl;
				error = 0;
		}

		return error;
	}


	//ansonsten weiter ...
	//string auf char* konvertieren
	const char *cKey = key.c_str();

	error = camera_.SetProperty(0,(char*)cKey,value);
	if(dbgOut_)
	{
		std::ostringstream osstr;
		osstr <<key<<", "<<value<<", error: "<<error;
		std::string str(osstr.str());
		wmLog(eInfo, "Changed %s\n",str.c_str() );
	}
	return error;
}

/**
 * \param key  keystring
 * \param value integer
 * \return error token
 */
int DataAcquire::setCameraParameter(std::string const& key,float& value)
{
	int error = 0;

	//string auf char* konvertieren
	const char *cKey = key.c_str();

	//via template:
	//std::cout<<"dataAcquire SetProperty key: "<<key<<" value: "<<value<<std::endl;
	error = camera_.SetProperty(0,(char*)cKey,value);
	if(dbgOut_)
	{
	    std::ostringstream osstr;
		osstr <<key<<", "<<value<<",error: "<<error;
		std::string str(osstr.str());
		wmLog(eInfo, "Changed %s\n",str.c_str() );
	}
	return error;
}


/**
 * Ersetze einzelnen keyValue in der mirror Sensorkonfiguration
 * \param keyValue Adresse(SmartPointer) auf einen keyValue
 *
 * \return error <0 Fehler: der key liegt nicht in der Konfiguration
 */
int DataAcquire::replaceSensorConfiguration(SmpKeyValue keyValue )
{
	int error = -1;

	for(unsigned int i=0;i<sensorConfiguration_.size();++i)
	{
		//std::cout<<"replace in configuration - keyValue: "<<sensorConfiguration_[i]->key() <<" - "<<keyValue->key()<<std::endl;
		if( sensorConfiguration_[i]->key() == keyValue->key() )
		{
			// Achtung min, max default nicht ueberschreiben ...
			// Zugriff auf value: void value(T const& value) { value_ = value; }

			if(dbgOut_)
			{
				std::stringstream sstr;
			    std::cout<<"replace in configuration "<<keyValue->key()<<std::endl;

		        sstr<<keyValue->key(); // << std::endl;
			    std::string str(sstr.str());
			    wmLog(eInfo, "Replace in Mirror Configuration %s\n",str.c_str() );
			}
			if(keyValue->type()==TInt )
			{
				const int dummy = keyValue->value<int>();

				if(     ( dummy <  sensorConfiguration_[i]->minima<int>() )
					 || ( dummy >  sensorConfiguration_[i]->maxima<int>() )
					 )
				{
					return error;
				}
				if(dbgOut_)
				{
                    std::cout<<"new value: "<<dummy<<std::endl;
                }
                sensorConfiguration_[i]->setValue<int>(dummy);
			}
			else if (keyValue->type()==TFloat)
			{
				const float dummy = keyValue->value<float>();

				if(     ( dummy <  sensorConfiguration_[i]->minima<float>() )
					 || ( dummy >  sensorConfiguration_[i]->maxima<float>() )
				)
				{
					return error;
				}
				if(dbgOut_)
				{
				    std::cout<<"new value: "<<dummy<<std::endl;
				}
				sensorConfiguration_[i]->setValue<float>(dummy);
			}
			else
				// im string Fall kein Vergleich
				sensorConfiguration_[i] = keyValue;


			return 0;
		}

	}

	std::cout<<"replace Mirror Value: "<<keyValue->key()<<" - Key nicht in sensorKonfiguration..."<<std::endl;
	wmLog(eInfo,"Key was not in the mirror configuration \n");


	return error;
}

/**
 * Set debug Outputr variable, logmessages and cout depends on the variable
 * \param key Property string
 * \return SmpKeyValue Adresse(SmartPointer) auf einen keyValue
  */
void DataAcquire::setDbgOut(const int& dbgOut)
{
	if(dbgOut>0)
	{
		dbgOut_ = true;
	}
	else
	{
		dbgOut_ = false;
	}
}

/**
 * Liefere keyValue aus mirror Konfiguration
 * \param key Property string
 * \return SmpKeyValue Adresse(SmartPointer) auf einen keyValue
  */
SmpKeyValue DataAcquire::getMirrorKeyValue(std::string const& key)
{
	for(unsigned int i=0;i<sensorConfiguration_.size();++i)
	{
		//Suche key in der Liste...
		if( sensorConfiguration_[i]->key() == key )
		{
			if( sensorConfiguration_[i]->type() == TInt)
			{
				if( dbgOut_)
				{
			        std::cout<<"getMirrorKey "<<key<<" value<int> "<<sensorConfiguration_[i]->value<int>()<<std::endl;
				}
			}
			if( sensorConfiguration_[i]->type() == TFloat)
			{
				if(dbgOut_)
				{
				    std::cout<<"getMirrorKey "<<key<<" value<float> "<<sensorConfiguration_[i]->value<float>()<<std::endl;
				}
			}
			return (sensorConfiguration_[i] );
		}

	}
	// Typ in der Liste nicht vorhanden...
	std::cout<<"get MirrorValueKeyValue: "<<key <<"- Key nicht in sensorKonfiguration..."<<std::endl;
	return SmpKeyValue (new KeyValue(TInt,"?",-1 ) );
}



bool DataAcquire::isImgNbInBuffer(uint32_t p_oImageNb)
{
    Poco::FastMutex::ScopedLock lock(imageGrabMutex_);
    return std::find(imagesInBuffer_.begin(), imagesInBuffer_.end(), p_oImageNb) != imagesInBuffer_.end();
} // isImgNbInBuffer



} // namespace precitec
