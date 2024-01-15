/**
 * @file
 * @brief  framegrabber Klasse mit Me4 Zugriff.
 * Diese framegrabber klasse verwendet direkt die Funktionen der SiSo API RT5
 *
 * @author JS
 * @date   20.05.11
 * @version 0.1
 *
 *
 */



/*************************************************************************/
/* * System headers                                                     */
/*************************************************************************/
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <math.h>

// wg mmap, mem_offset
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#ifdef __QNX__
#include <sys/neutrino.h> // send
#endif
#include <pthread.h>
#include <semaphore.h>

#include "event/imageShMem.h" // wg Bild-Konstanten Hoehe, Breite, Pufferzahl, ...

#include "common/connectionConfiguration.h"
#include "common/defines.h"

#if DEBUGDISPLAY
#include "debugDisplay.h"
#endif

#include "common/systemConfiguration.h"

/************************************************************************
* Application headers                                                   *
************************************************************************/
#include "framegrabber.h"

#include "applEnums.h" // die Siso-konstanten, die Appletentitaeten beschreiben


#include <unistd.h> // usleep

namespace precitec {

using namespace interface;

#ifndef NOFD
#define NOFD (-1)
#endif

namespace CameraGrayAreaFull {
enum DvalMode {
	DVAL_Enabled	= 1,
	DVAL_Disabled	= 0
};
}

// Semaphore anlegen
// global zur Kommunikation zwischen Prozess und thread
sem_t  eventSem;

// -----------------------------------------------------------------------------
// Framegrabber Klasse
// -----------------------------------------------------------------------------


/**
 * Konstruktor fuer die Framegrabberklasse
 * Initialisiert nur interne variablen, keine HW INitialisierung
 */

FrameGrabber :: FrameGrabber()
						: imgCounter_(-1),
						  actualBufferNumber_(0),
						  fg(NULL),
						  bufPtr(NULL),
						  iAnzImagePixels_(0),
						  iAnzImageLines_(0),
						  xoff_(0),
						  yoff_(0),
						  mirror_(false),
						  conID_(0),
						  grabberControlledMode_(false),
						  imageNoUseFlag_(false),
						  frames_(LOTSOFIMAGES),
						  threadId_(0),
						  SoftwareTrgPulse_Id_(0)
{
    // SystemConfig Switch for Image Mirror
    mirror_ = SystemConfiguration::instance().getBool("ImageMirrorActive", false);
    wmLog(eDebug, "mirror_ (bool): %d\n", mirror_);

    bool hasCamera{false};
    char* oEnvStrg = getenv((char *)"WM_STATION_NAME");
    if (oEnvStrg != nullptr)
    {
        if (strcmp("WM-QNX-PC", oEnvStrg) == 0)
        {
            hasCamera = SystemConfiguration::instance().getBool("HasCamera", true);
        }
    }
    if (hasCamera && SystemConfiguration::instance().getBool("ImageMirrorYActive", false) == true)
    {
        wmLog( eFatal, "The FrameGrabber camera does not support mirroring the image! Please disable this option and reboot.\n" );
    }
    grabActive = false; // sorgt dafuer, dass ueberfluessige Bilder waehrend der Initialisierung keinen Schaden anrichten
}



/**
 * Konstruktor fuer die Framegrabberklasse
 * Initialisiert nur interne Variablen, mit Message ID
 */
FrameGrabber :: FrameGrabber(int conID)
						: imgCounter_(-1),
						  actualBufferNumber_(0),
						  fg(NULL),
						  bufPtr(NULL),
						  iAnzImagePixels_(0),
						  iAnzImageLines_(0),
						  iMaxImageBuffers_(0),
						  xoff_(0),
						  yoff_(0),
						  conID_(conID),
						  threadId_(0),
						  SoftwareTrgPulse_Id_(0)
{
  grabActive = false; // sorgt dafuer, dass ueberfluessige Bilder waehrend der Initialisierung keinen Schaden anrichten
}




FrameGrabber :: ~FrameGrabber()
{
	//grabberProps loeschen
	std::cout<<"deleting "<<grabberProps_.size()<<" probs"<<std::endl;
	for(unsigned int i=0;i<grabberProps_.size();++i)
	{
		if(grabberProps_ [i])
		delete 	grabberProps_[i];
	}
	grabberProps_.clear();

	std::cout<<"deleting pImage_"<<std::endl;
	if (pImage_) delete [] pImage_;
}


/**
 *   @brief Setzen der ROI Groesse auf die Grabber Variablen
 *   \param width  Bildbreite
 *   \param height Bildhoehe
 */
void FrameGrabber :: setROI( int width, int height )
{
	iAnzImagePixels_ = width;
	iAnzImageLines_  = height;

	std::cout << "frameGrabber, setRoi: " <<iAnzImagePixels_<<" "<<iAnzImageLines_ << std::endl;


}

/**
 *   @brief Setzen der Offset Grabber Variablen
 *   \param xoff Offset in x Richtung
 *   \param yoff Offset in y Richtung
 */
void FrameGrabber :: setOffset( int xOff, int yOff )
{
	xoff_ = xOff;
	yoff_ = yOff;
}



/**
 *  @brief Setzen der Offset Werte im Bildspeicher des Framegrabbers
 * \param x x-offset
 * \param y y-offset
 * \param dx Breite
 * \param dy Hoehe
 *
 * \return error  <0 Fehler
 */
int	 FrameGrabber::setGrabberRoi(int x, int y, int dx, int dy, bool useOffset)
{
	int error = 0;
	error = setParameter("Device1_Process0_ImageBuffer_XOffset",x);
	error = setParameter("Device1_Process0_ImageBuffer_YOffset",y);
	error = setParameter("Device1_Process0_ImageBuffer_XLength",dx);
	error = setParameter("Device1_Process0_ImageBuffer_YLength",dy);

    if (mirror_)
    {
        int adressEndValue = (dx / 4 - 1); // last address for 4 pixel
        setParameter("Device1_Process0_AdressEnd_Value", adressEndValue);

        int xLength = dx / 4;
        setParameter("Device1_Process0_LineMemory_XLength", xLength);
    }

	if(error >= 0)
	{
		//Werte intern ablegen
		iAnzImagePixels_ = dx;
		iAnzImageLines_ = dy;
		xoff_ = x;
		yoff_ = y;
	}
	return error;
}



/**
 *   @brief Grabber schliessen, Speicher und image thread schliessen
 *   Grabber runterfahren:
 *   thread beenden, Grab stoppen, Speicher freigeben, Handle freigeben
 * 	\param  functionName Ursache beim Runterfahren kann mitgegegben werden
 *
 * 	\return error  <0 Fehler

 */
int FrameGrabber :: freeGrabber(const char* functionName)
{
	if (functionName)
	{
		std::fprintf(stderr, "Error in %s: %s\n", functionName, Fg_getLastErrorDescription(fg));
	}
	else
	{
		std::fprintf(stderr, "%s\n", Fg_getLastErrorDescription(fg));
	}
	// alles runterfahren
	int dma = 0; // = CameraPort = PORT_A

	//thread runterfahren:
	grabActive = false;
	triggerImageSW();

	Fg_stopAcquireEx(fg, dma, bufPtr,0);



	// Speicher freigeben
	freeImageMem();

	Fg_FreeGrabber(fg);

#if DEBUGDISPLAY
	CloseDisplay();
#endif


	grabActive = false;
	fg = NULL;
	return -1;
}


/**
 *   @brief cameralink port leeren
 *   Verhindert eventuell ein Aufhaengen der Kommunikationsschnittstelle
 *   nach einem Abbruch der Kommunikation
 * 	\param  no params
 *
 * 	\return error  <0 Fehler

 */
int FrameGrabber :: portFlush()
{
    int error =0;
	//Test -
	std::cout<<"clFlushPort() Aufruf "<<std::endl;

	//clFlushPort (&portNum); don't do that // (void* serialRef);

	return error;
}



/**
 * @brief Image Thread, reagiert, sobald Bild im DMA Speicher
 * diese Routine wird als thread gestartet und wartet immer auf Bilder
 * wenn ein Bild ankommt wird ein Puls an den Calc-abgesetzt
 *
 * 	\param  frameGrabber frameGrabber wird dem thread mitgegeben
 */
void *FrameGrabber :: imageGetter(void *frameGrabber)
{
	FrameGrabber &grabber = *((FrameGrabber *)frameGrabber);

	int  actualImageNumber = 0;
	int frameNumber = 0;


	grabber.imgCounter_ = 0;
	grabber.actualBufferNumber_= 0;

	std::cout <<"image thread gestartet" << std::endl;

	// hier warten wir auf Freigabe, d.h. Fg_Acquire muss gestartet sein ...
	std::cout<<"thread: in thread auf sem_wait... "<<std::endl;
	sem_wait(&eventSem);
	std::cout<<"thread: sem erhalten... "<<std::endl;
	std::cout<<" active: "<<grabber.grabActive<<" actNr: "<<actualImageNumber<<" frames: "<<grabber.frames_<<std::endl;


	while( (grabber.grabActive) && (actualImageNumber < grabber.frames_ ) )
	{

		const int timeout = 0x7FFFEFFE; //  0; //50;
		const int TimeoutError = -2120;
		do{
		  actualImageNumber = Fg_getLastPicNumberBlockingEx(grabber.fg, grabber.imgCounter_+1, 0, timeout, grabber.bufPtr);
		  if(actualImageNumber != grabber.imgCounter_+1)
		  {
#if !defined(NDEBUG)
  			std::cout<<"imagGetter - uebertriggert..."<<actualImageNumber<<" "<< grabber.imgCounter_+1<<std::endl;
  			std::cout << " imageGetter thread wird mit uebertriggerung verlassen ... " << actualImageNumber << std::endl;
  			pthread_exit(NULL);
#endif
		  }
		  if (grabber.grabActive == false)
		  {
			std::cout << " imageGetter thread wird mit false beendet ... " << actualImageNumber << std::endl;
		    pthread_exit(NULL);
		  }
		  else
		  {
#if !defined(NDEBUG)
			std::cout <<"actualImageNumber: "<< actualImageNumber<<" imagCounter+1: "<<grabber.imgCounter_+1<< std::endl;
#endif
			grabber.imgCounter_ = actualImageNumber;
		  }
		}while((actualImageNumber == TimeoutError)&&(actualImageNumber<grabber.frames_));


		grabber.actualBufferNumber_ = (actualImageNumber-1)%grabber.iMaxImageBuffers_; //   MAX_IMAGE_BUFFERS;
		frameNumber = grabber.actualBufferNumber_;

#if DEBUGDISPLAY
		// Adressen sind unterschiedlich :
		std::printf("im image getter phys addr. und size: %lx   %lx\n",grabber.physAddr[frameNumber],grabber.nbytes[frameNumber]);
		// hier werden die Adressen nach dem allokieren abgelegt:
		unsigned long* iPtr0 = (unsigned long*)grabber.logAddr[frameNumber];
		DrawBuffer(0,(unsigned long*)iPtr0,0,0);
#endif
        if(!grabber.imageNoUseFlag_)
        {
        	grabber.imageEvent_.notify(frameGrabber, frameNumber);
        }
        else
        {
        	std::cout<<"dummy image not sent"<<std::endl;
        	grabber.imageNoUseFlag_ = false;
        }
	} //while

	std::cout << " imageGetter thread ist beendet ... " <<std::endl;

	grabber.threadId_  = 0;
	grabber.grabActive = false;

	grabber.AcquireStop();

	pthread_exit(NULL);

}

/**
* @brief Berechne phys addr. aus log. addr.
*
* \param    addr  virtuelle Adresse*
* \param    Speicher(Bild)groesse
*
*  \return   phys. adresse , null pointer im Fehlerfall
*
*/

off64_t logAddressToPhysAddress(void *addr,size_t imgSize)
{
#ifdef __QNX__
  off64_t                 offset;

  // when filed-id = NOFD; the phys address of addr is calculated
    //**** original me3 :
    //if (mem_offset64(imgPtr, NOFD, 1, &offset, 0) == -1)

  if (mem_offset64(addr, NOFD, imgSize, &offset, 0) == EACCES)
  {
  	// -1 = AccessError
  	// physical memory is not found
    return 0; // ein NULL-Ptr scheint hier angemessen
  }
  return offset;
#endif
  return 0;
}

/**
 * @brief Initialisierung der Me4: Laedt Applet fuer ME4-Karte
 * \param Grabber Nummer
 * \return error <0 falls Fehler
 */
int FrameGrabber :: initBoard(int deviceNo)
{
	int error = 1;

    if (mirror_)
    {
        std::cout<<" Lade hap file Mirror_RT5_AquisitionLinux64.hap"<<std::endl;
        fg = Fg_InitEx("Mirror_RT5_AquisitionLinux64.hap",0,0 );
    }
    else
    {
        std::cout<<" Lade hap file Simple_RT5_AquisitionLinux64.hap"<<std::endl;
        fg = Fg_InitEx("Simple_RT5_AquisitionLinux64.hap",0,0 );
    }

	std::cout << "Fg_InitEx: "<< fg <<" Description " << Fg_getLastErrorDescription(0) << std::endl;

	if(fg == NULL)
	{
		std::cout << "Error in Fg_Init: " << Fg_getLastErrorDescription(0) << std::endl;
		return -2;
	}
	//std::cout << "bvhw: Framegrabberseriennr: " << std::hex
	//					<< Fg_getSerialNumber(fg) << std::dec << std::endl;

	int nrOfParameter = Fg_getNrOfParameter(fg);
	std::fprintf(stdout,"Nr of Parameter: %d\n",nrOfParameter);

	// schreibe die Properties, auf die Zugriff gestattet wird,in eine Liste
	//Trigger Modi:  enum TriggerMode {	GrabberControlled=1,ExternSw_Trigger=2};

	std::cout<<"creating probs "<<std::endl;
	grabberProps_.push_back( new TSensorProperty<int>("Device1_Process0_Trigger_TriggerMode",1,3,2,TInt)  );
	grabberProps_.push_back( new TSensorProperty<float>("Device1_Process0_Trigger_ExsyncFramesPerSec",1.0,1000.0,10.0,TFloat)  );

	//Laenge exposure signal
	grabberProps_.push_back( new TSensorProperty<float>("Device1_Process0_Trigger_ExsyncExposure",10.0,1000.0,400.0,TFloat)  );

		//ExSync Delay
	grabberProps_.push_back( new TSensorProperty<float>("Device1_Process0_Trigger_ExsyncDelay",0.0,10000.0,0.0,TFloat)  );
        
    	// Flash delay
	grabberProps_.push_back( new TSensorProperty<float>("Device1_Process0_Trigger_FlashDelay",0.0,10000.0,0.0,TFloat)  );
       

	// Exsync Polaritaet
	int ExsyncPolarity = TrgPortArea::HighActive;
	grabberProps_.push_back( new TSensorProperty<int>("Device1_Process0_Trigger_ExsyncPolarity",0,1,ExsyncPolarity,TInt)  );



	// Die Grabber ROIS sollte von aussen nicht verstellt werden
	//grabberProps_.push_back( new TSensorProperty<int>("Device1_Process0_ImageBuffer_XOffset",0,1024,0,TInt)  );
	//grabberProps_.push_back( new TSensorProperty<int>("Device1_Process0_ImageBuffer_YOffset",0,1024,0,TInt)  );
	//grabberProps_.push_back( new TSensorProperty<int>("Device1_Process0_ImageBuffer_XLength",0,1024,1024,TInt)  );
	//grabberProps_.push_back( new TSensorProperty<int>("Device1_Process0_ImageBuffer_YLength",0,1024,1024,TInt )  );


	// Flash an
	int FlashEnable = TrgPortArea::ON;
	grabberProps_.push_back( new TSensorProperty<int>("Device1_Process0_Trigger_FlashEnable",0,1,FlashEnable,TInt)  );

    //Debug flag packed in the grabber properties to switch dynamic debug output- 0: less output
	grabberProps_.push_back( new TSensorProperty<int>("Device1_Process0_Debug_Output",0,1,0,TInt)  );


//	grabActive = true;  der grab setzt

	return error;
}


/**
 * @brief Kamera vorhanden/ok Test
 * Clock sollte immer anliegen
 */
bool FrameGrabber :: isCamClockPresent()
{
	if (fg == NULL) return false;
	int clock=0;
	int status = Fg_getParameter(fg,FG_CAMSTATUS, &clock, 0);
	std::cout << (clock >0 ? "Kamera Clock vorhanden" : "Keine Kamera Clock ") << clock << std::endl;
	std::cout<<"Status: "<<status<<std::endl;
	return clock > 0;
}
/**
 * @brief Dynamisches Setzen der Applet Parameter
 * setzt alle Parameter-Satz fuer ein ganz spezifisches Applet: Simple_RT5_AquisitionQNX.hap
 */
int FrameGrabber :: initFGPAParameter()
{


	if (fg==NULL) return -1;


	 setParameter("Device1_Process0_Aquisition_UseDval",int(CameraGrayAreaBase::DVAL_Enabled));
	 setParameter("Device1_Process0_Aquisition_Format",int(CameraGrayAreaBase::DualTap8Bit) );


	/*============ ImageBuffer : buffer ============== */

	int Process0_buffer_XOffset=xoff_;;
	setParameter("Device1_Process0_ImageBuffer_XOffset",Process0_buffer_XOffset);

	int Process0_buffer_XLength = iAnzImagePixels_; //   width;
	setParameter("Device1_Process0_ImageBuffer_XLength",Process0_buffer_XLength);

	int Process0_buffer_YOffset=yoff_;
	setParameter("Device1_Process0_ImageBuffer_YOffset",Process0_buffer_YOffset );

	int Process0_buffer_YLength =  iAnzImageLines_; //   height;
	setParameter("Device1_Process0_ImageBuffer_YLength",Process0_buffer_YLength);


	/*============ Zugriff aus TrgPort Area ============== */
	// Vorraussetzung ist der Triggeroperator !!
	// --> simpleAquisition
	// Trigger Mode
	int Trigger_Mode = TrgPortArea::ExternSw_Trigger;  //GrabberControlled waere 1
	setParameter("Device1_Process0_Trigger_TriggerMode",Trigger_Mode);


    // ExSync enable
	int ExsyncEnable = TrgPortArea::ON;
	setParameter("Device1_Process0_Trigger_ExsyncEnable",ExsyncEnable  );


	// Flash an
	int FlashEnable = TrgPortArea::ON;
	setParameter("Device1_Process0_Trigger_FlashEnable",FlashEnable);


	// SW Trigger ID hohlen
	SoftwareTrgPulse_Id_ = Fg_getParameterIdByName(fg, "Device1_Process0_Trigger_SoftwareTrgPulse");
	if (SoftwareTrgPulse_Id_ < 0 )
			std::cout << "Siso::get SoftwareTrigegr ID  failed : " << SoftwareTrgPulse_Id_<< " : " << Fg_getLastErrorDescription(fg) << std::endl;


	// Trigger Source --- unnoetig  bei grabberControlled
	//signal quelle fuer das trigger signal, hier sw trigger
	int ImgTrgInSource = TrgPortArea::SoftwareTrigger;
	setParameter("Device1_Process0_Trigger_ImgTrgInSource", ImgTrgInSource );



	// Trigger Polaritaet
	int ImgTrgInPolarity = TrgPortArea::HighActive;
	setParameter("Device1_Process0_Trigger_ImgTrgInPolarity", ImgTrgInPolarity);

	// Trigger downscale
	unsigned int ImgTrgDownscale = 1;
	setParameter("Device1_Process0_Trigger_ImgTrgDownscale", ImgTrgDownscale );

	// Trigger Genauigkeit
	double Accuracy = 10.0;
	setParameter("Device1_Process0_Trigger_Accuracy",Accuracy);



	// Framerate
	double ExsyncFramesPerSec = 10.0;
	setParameter("Device1_Process0_Trigger_ExsyncFramesPerSec", ExsyncFramesPerSec);

	// Exsync Exposure
	double ExsyncExposure= 400.0;
	setParameter("Device1_Process0_Trigger_ExsyncExposure", ExsyncExposure);

	//ExSync Delay
	double ExsyncDelay=0.0;
	setParameter("Device1_Process0_Trigger_ExsyncDelay",ExsyncDelay );

	// Exsync Polaritaet
	int ExsyncPolarity = TrgPortArea::HighActive;
	setParameter("Device1_Process0_Trigger_ExsyncPolarity", ExsyncPolarity );

			// Flash delay
	double FlashDelay = 0.0;
	setParameter("Device1_Process0_Trigger_FlashDelay",FlashDelay  );


	// SW Trigger Timeout
	double SoftwareTrgDeadTime = 10000.0;
	setParameter("Device1_Process0_Trigger_SoftwareTrgDeadTime",SoftwareTrgDeadTime );


		// Flash Polaritaet
	int FlashPolarity = TrgPortArea::HighActive;
	setParameter("Device1_Process0_Trigger_FlashPolarity",FlashPolarity );

	// Ende Trigger Operator ******************************************************************


	//timeout setzen
	 int generalTimeOut = 0x7FFFFFFE; //    2,147,483,646
	 int statusTimeOut = Fg_setParameter(fg,FG_TIMEOUT,&generalTimeOut ,0);
	 std::cout<<"Status Timeout: "<<statusTimeOut<<std::endl;

    if (mirror_)
    {
        int adressEndValue = (iAnzImagePixels_ / 4 - 1); // last address for 4 pixel
        setParameter("Device1_Process0_AdressEnd_Value", adressEndValue);

        int xLength = iAnzImagePixels_ / 4;
        setParameter("Device1_Process0_LineMemory_XLength", xLength);
    }

	  // Im debug Fall den Display auf QNX anwerfen ...
#if DEBUGDISPLAY
				// ---------------------------------------------------------------------------
				// Test Display anschmeissen ...
				int drawWidth  = iAnzImagePixels_;
				int drawHeight = iAnzImageLines_;

				CreateDisplay(8,drawWidth,drawHeight);
#endif




	 return 1;

}

/**
 * Legt Shared Speicher selbst an, falls kein Siso-Board vorhanden/aktiv ist
 */
int FrameGrabber :: initSouvisImageMem()
{
	int width	= iAnzImagePixels_;    // Achtung: Wert muss durch 4 teilbar sein;
	int height	= iAnzImageLines_;
	int imgSize = (width * height + 4095) & 0xfffff000; // auf Pagegroesse aufrunden

	// Shared memory selbst anlegen ??? ein einziges ShMem wuerde ausreichen
	//for(int i = 0; i < MAX_IMAGE_BUFFERS; i++)
	for(int i = 0; i < iMaxImageBuffers_; i++)
	{
		void *virtual_address = mmap64(0, imgSize,
//								 PROT_READ | PROT_WRITE | PROT_NOCACHE,
//								 MAP_PHYS | MAP_ANON,
								 PROT_READ | PROT_WRITE,
								 MAP_ANON,
								 NOFD, 0);

		if (virtual_address == MAP_FAILED)
		{

			std::cout<<"frameGrabber,initSouvisImage: Allocation error with mmap64"<<std::endl;
			return -1;
		}

		off64_t  physical_address = 0;

#ifdef __QNX__
		// **** me3 code:
		// mem_offset64(virtual_address, NOFD, imgSize, &physical_address, 0)) == -1)
		if (mem_offset64(virtual_address, NOFD, imgSize, &physical_address, 0) == -1)
		{
			std::cout<<"frameGrabber,initSouvisImage: Allocation error with mem_offset64"<<std::endl;
			return -1;
		}
#endif

		//logAddr[i] = (unsigned long)virtual_address;
		logAddr.push_back((unsigned long)virtual_address);
		//physAddr[i] = (unsigned long)physical_address;
		physAddr.push_back((unsigned long)physical_address );
		//nbytes[i] = imgSize;
        nbytes.push_back(imgSize);

		// GUR 1.7.04
	}
	std::printf("initSouvisImageMem: physical memory allocated without grabber \n");
	/// es wird immer ein Fehler (= keine Siso-Karte) zurueckgegeben
	return -2;
}

/**
 * Allokiert Bildspeicher mit Siso-API, flls ME4 vorhanden ist
 */
int FrameGrabber :: initSisoImageMem()
{


	int width		= iAnzImagePixels_ ;    // Achtung: Wert muss durch 4 teilbar sein;
	int height	    = iAnzImageLines_ ;


	// Speicherallokierung
	size_t bufSize = greyImage::ImageShMemSize; // gesamte Speichergroese


	int imgSize = (width * height + 4095) & 0xfffff000; // sub buffer size
	//bufPtr = Fg_AllocMemEx(fg,bufSize,MAX_IMAGE_BUFFERS);iMaxImageBuffers_
	bufPtr = Fg_AllocMemEx(fg,bufSize,iMaxImageBuffers_);

	if(bufPtr == NULL)
	{
		freeGrabber("Fg_AllocMemEx");
		std::cout<<"frameGrabber,initSisoImage: Allocation error with siso Fg_Alloc"<<std::endl;
		return -1;
	}

  int dma = 0;

  //for(int i = 0; i < MAX_IMAGE_BUFFERS; i++)
  for(int i = 0; i <iMaxImageBuffers_ ; i++)
  {
  	// get pointer to individual image within buffer
	 void *imgPtr 	= Fg_getImagePtrEx(fg, i+1, dma, bufPtr);

	 // **** me3 code:
		 //if (mem_offset64(imgPtr, NOFD, 1, &offset, 0) == -1)
		 //{
		 //	 printf("bvhw: Failure in mem_offset64\n");
		 //}
		 //logAddr[i] = (unsigned long)imgPtr;
		 //physAddr[i] = (unsigned long)offset;
		 //nbytes[i] = imgSize;

     //logAddr[i] 	= (unsigned long)imgPtr;
     //physAddr[i]    = (unsigned long)logAddressToPhysAddress(imgPtr,greyImage::ImageShMemSize); //mem_offset64(addr, NOFD, imgSize, &offset, 0)
     //nbytes[i] 	    = greyImage::ImageShMemSize;  ----- ????

     logAddr.push_back((unsigned long)imgPtr);
     physAddr.push_back( (unsigned long)logAddressToPhysAddress(imgPtr,imgSize  )     );
     nbytes.push_back(imgSize);


     // Siso-Allokation und Phys-Adress-Konversion muessen geklappt haben
     if(imgPtr)
     {

      std::cout << "bvhw Memory for image " << i << std::hex
      					<< "- log: " << logAddr[i]
      					<< "- phys: " << physAddr[i]
      					<< "- bytes: " << nbytes[i] << std::dec << std::endl;
     }
      else
      {
    	  std::cout<<"Allocation error ..."<<std::endl;
    	  std::cout<<"frameGrabber,initSisoImage: Allocation error with siso Fg_getImagePtr"<<std::endl;
    	  return -1;
      }
  }
  // if (!allokOk) deallocBuffers();
  //std::cout << "bvhw: Memory Allocation " << (allocOk ? "ok" : "nok") << std::endl;
  return 1;
}

/**
 * Legt DMA Speicher ueber User Memory an
 */
int FrameGrabber :: initUserImageMem()
{
	int bufferIndex = 0;
	int width	= iAnzImagePixels_;    // Achtung: Wert muss durch 4 teilbar sein;
	int height	= iAnzImageLines_;
	int imgSize = (width * height + 4095) & 0xfffff000; // auf Pagegroesse aufrunden
    //size_t bufSize = imgSize * MAX_IMAGE_BUFFERS;       // Gesamte Speichergroesse
	size_t bufSize = imgSize * iMaxImageBuffers_; // Gesamte Speichergroesse


    //bufsize: Size of all image memory in byte
    //MAX_IMAGE_BUFFERS: Number of subbuffers - Integer on 32 bit systems
	//bufPtr = Fg_AllocMemHead(fg,bufSize,MAX_IMAGE_BUFFERS);
	bufPtr = Fg_AllocMemHead(fg,bufSize,iMaxImageBuffers_);

	// User memory anlegen
	//for(int i = 0; i < MAX_IMAGE_BUFFERS; i++)
	for(int i = 0; i < iMaxImageBuffers_; i++)
	{
		void *virtual_address = mmap64(0, imgSize,
//								 PROT_READ | PROT_WRITE | PROT_NOCACHE,
//								 MAP_PHYS | MAP_ANON,
								 PROT_READ | PROT_WRITE,
								 MAP_ANON,
								 NOFD, 0);

		if (virtual_address == MAP_FAILED)
		{
			 std::cout<<"frameGrabber,initUserImageMem: Allocation error with mmap64"<<std::endl;
			return -1;
		}

		// *** siso example: Fg_AddMem(Fg, bufmem[i], buflen, i, head)
		// virtual address: pointer to user memory
		// imgSize: Size of user memory
		// i : Index of subbuffers
		bufferIndex = Fg_AddMem(fg,virtual_address,imgSize,i,bufPtr);
		if(bufferIndex<0)
			std::cout<<"frameGrabber: Fg_AddMem Error"<<std::endl;

		off64_t  physical_address = 0;

#ifdef __QNX__
	    // *** me code:	mem_offset64(virtual_address, NOFD, imgSize, &physical_address, 0)
		if (mem_offset64(virtual_address, NOFD, imgSize, &physical_address, 0) == -1)
		{

			std::cout<<"frameGrabber,initUserImageMem: Allocation error with mem_offset64"<<std::endl;
			return -1;
		}
#endif

		//logAddr[i] = (unsigned long)virtual_address;
		//physAddr[i] = (unsigned long)physical_address;
		//nbytes[i] = imgSize;
		 logAddr.push_back((unsigned long)virtual_address);
		 physAddr.push_back((unsigned long)physical_address );
		 nbytes.push_back(imgSize);


	}

	std::printf("initUserImageMem: physical memory allocated via user memory\n");
	/// es wird immer ein Fehler (= keine Siso-Karte) zurueckgegeben
	return 1;
}

/**
*  @brief Initialisiert shared Memory
*  Die gesamte Groesse des verfuegbaren Bildspeichers wird hier initialisiert
*/
precitec::system::SharedMem& FrameGrabber :: initSharedImageMem()
{
	using namespace precitec::greyImage;
	// ShMem festlegen :
	// Shared mem size wird in interfaces/include/event/imageShMem.h festgelegt:
	//  const int	  	NumImageBuffers		= 256;
	//	const int	  	ImageWidth			= 256;
	//	const int	  	ImageHeight			= 256;
	//	const int	  	ImageSize			= ((ImageWidth * ImageHeight + 4095)) & 0xfffff000; // auf Pagegroesse aufrunden
  	//	const int		ImageShMemSize		= NumImageBuffers * ImageSize;


	static precitec::system::SharedMem imageShMem(sharedMemoryHandle(), sharedMemoryName(), precitec::system::SharedMem::StdClient, ImageShMemSize);

	return imageShMem;
}


/**
 *  @brief Initialisiert shared Memory und mappt den SiSo Speicher in den shared Mem
 *  Die Anzahl Bilder sollte von der shMem- und der BildGroesse abhaengen
 *  Legt DMA Speicher ueber User shared Memory an
 */
int FrameGrabber :: initDMAImageMem(precitec::system::SharedMem &imageShMem  )
{



	using namespace precitec::greyImage;
	// ShMem-Daten aus Header

	int bufferIndex = 0;
	int width	= iAnzImagePixels_;    // Achtung: Wert muss durch 4 teilbar sein;  wg Kamera
	int height	= iAnzImageLines_;


	int imgSize = (width * height + 4095) & 0xfffff000; // auf Pagegroesse aufrunden

	// soviele Bilder passen in den shared mem hinein
	iMaxImageBuffers_ = ImageShMemSize / imgSize;

	std::cout<<"Moegliche Anzahl Bilder im DMA Speicher bei "<<ImageShMemSize<<" Shared mem size und "<<imgSize<<" Einzelbildgroesse"<<std::endl;
	std::cout<<"width: "<<width<<" height: "<<height<<std::endl;
	std::cout<<"---> "<<iMaxImageBuffers_  <<std::endl;

	const int ImagesInBuffer = iMaxImageBuffers_ ; //  MAX_IMAGE_BUFFERS; //      ImageShMemSize / imgSize;

	// Gesamtspeicher der Bilder

	const int FinalBuffSize  =  ImagesInBuffer * imgSize;      //greyImage::ImageShMemSize;

	bufPtr = Fg_AllocMemHead(fg, FinalBuffSize, ImagesInBuffer);
	if(bufPtr==NULL)
	{
		std::cout<<"frameGrabber: Fg_AllocMemHead: "<< Fg_getLastErrorDescription(fg)  <<std::endl;
		return -1;
	}

	std::cout<<"creating pImage "<<std::endl;
	pImage_ = new precitec::system::ShMemPtr<byte>[ImagesInBuffer];

	//Einzelpointer an Fg_AddMem uebergeben
	for(int i = 0; i < ImagesInBuffer; i++)
	{
		pImage_[i].set(imageShMem, i*imgSize);

        // pImage[i].get() wg c-Pointer!!
		bufferIndex = Fg_AddMem(fg, pImage_[i].get(), imgSize, i, bufPtr);
		if(bufferIndex<0)
		{
			std::cout<<"frameGrabber: Fg_AddMem Error: "<< Fg_getLastErrorDescription(fg)  <<std::endl;
			return -1;
		}
		off64_t  physical_address = 0;

#ifdef __QNX__
	    // *** me code:	mem_offset64(virtual_address, NOFD, imgSize, &physical_address, 0)
		if (mem_offset64(pImage_[i].get(), NOFD, imgSize, &physical_address, 0) == -1)
		{

			std::cout<<"frameGrabber,initUserImageMem: Allocation error with mem_offset64"<<std::endl;
			return -1;
		}
#endif
		logAddr.push_back((unsigned long)pImage_[i].get() );
		physAddr.push_back((unsigned long)physical_address );
		nbytes.push_back(imgSize);
	}

	std::printf("initUserImageMem: physical memory allocated via user shared memory\n");

	return 1;
}





/**
 *  @brief Gibt SiSo DMA frei
 *  Image-Speicher freigeben, ShMem Speicher bleibt bestehen !
 */
void FrameGrabber :: freeImageMem()
{
	if (!bGrabberOnboard)
	{
		// shared Mem freigeben!!!
	}
	else
	{
		if (fg==NULL)
			return;
		if (bufPtr) // hier koennte unterschieden werden, wie der SPeicher angelegt wurde
			freeUserMem();

	}
}

/**
 * @brief Ruft SiSo Speicher Deallokation auf
 * User Image-Speicher freigeben
 */
void FrameGrabber::freeUserMem()
{


	// Die vectoren logAddr, phyAddr und nbytes
	// sowie der sharedMem Pointer Array muessen ebenfalls freigegeben
	// werden

    std::cout<<"Es werden "<<iMaxImageBuffers_<<" freigegeben"<<std::endl;

	//for(int i = 0; i < MAX_IMAGE_BUFFERS; i++)
	for(int i = 0; i < iMaxImageBuffers_; i++)
	{
		//bufPtr: handle on framebuffer:
		//i: index of subbuffer
		if( Fg_DelMem(fg,bufPtr,i) != FG_OK)
		{
			std::cout<<"Fg_DelMem:Invalid Paraameter..."<<std::endl;
		}

	}


	// free ShMemPtr Array
	std::cout<<"deleting pImage_"<<std::endl;
	if (pImage_)
		delete[] pImage_;

	// Vektoren leeren
	 logAddr.clear();
	 physAddr.clear();
	 nbytes.clear();

	 if( Fg_FreeMemHead(fg, bufPtr) != FG_OK)
	{
		std::cout<<"Fg_FreeMemHead: Speicher kann nicht freigegeben werden..."<<std::endl;
	}

}



/**
 *  @brief Image Thread anlegen
 *  Initialisiere den Image thread
 */
int FrameGrabber :: initImageThread()
{

//	int threadId;
//	pthread_attr_t attr;
//	pthread_attr_init( &attr );
//	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	int status=0;

	if (threadId_!=0)
	{
		//pthread_abort(threadId_);
		//std::cout<<"frameGrabber::initImagethread aborted"<<std::endl;
		std::cout<<"frameGrabber::thread ID already exists "<<std::endl;
		//return -1;
	}
	status = pthread_attr_init( &attr_ );
	status = pthread_attr_setdetachstate(&attr_, PTHREAD_CREATE_DETACHED);

	//Semaphore anlegen
	sem_init( &eventSem,
	          0,    // not shared
	          0);   //SEAM_VALUE_MAX, 0 locked - binaer


	///setprio(threadId_, getprio(0)+1);
	//std::cout<<"frameGrabber::initImagethread initialised"<<std::endl;
	return status;

}


/*
 *  @brief Starte den Image thread
 */
int FrameGrabber :: startImageThread()
{

	//grabActive = false; // image-Getter darf nicht einfach  loslaufen
	int ok = pthread_create(&threadId_, &attr_, FrameGrabber::imageGetter, this);

	return ok;


}



/**
 *  @brief Framegrabber Initialisierung zusammengefasst:
 *  Erstes Hochfahren des Systems: Applet laden ,shared memory anlegen, DMA anlegen, Applet Parameter setzen
 *  Appletname  ist hartkodiert
 * \param deviceNo Grabber Nummer
 */
int FrameGrabber :: init( int deviceNo )
{
	    /// DMA Speicher ohne grabber anlegen
		if (!bGrabberOnboard)
		{
			initSouvisImageMem();
			return -2;
		}


		/// Applet laden
		if( initBoard(deviceNo) < 0)
		{
			bGrabberOnboard = false;
		    return -1;
		}

		// SharedMem aus interface/event/imageSharedMem.h anlegen
		//if(	initSharedImageMem() < 0)
		ShMem_ = initSharedImageMem();
        if(!ShMem_.isValid())
        	return -3;


	return 1;

}





/**
 *  @brief Framegrabber Speicher anlegen auf shared mem
 *  Nach dem ersten Hochfahren des Systems: Applet laden ,shared memory anlegen, DMA anlegen, Applet Parameter setzen
 *  Appletname  ist hartkodiert
 * \param deviceNo Grabber Nummer
 */
int FrameGrabber :: initMemory( int deviceNo )
{

	//int dummy = deviceNo;
	if (deviceNo<0)
		std::cout<<"ungueltige device Famegrabber nummer"<<std::endl;
	// DMA Speicher auf shared mem setzen
	if ( initDMAImageMem(ShMem_ )< 0 )
	{
		return -1;
	}


	/// FPGA Parameter setzen
	if( initFGPAParameter()< 0)
	{
		return -1;
	}

    // Im debug Fall den Display auf QNX anwerfen ...
#if DEBUGDISPLAY
		// ---------------------------------------------------------------------------
		// Test Display anschmeissen ...
		int drawWidth  = iAnzImagePixels_;
		int drawHeight = iAnzImageLines_;

		CreateDisplay(8,drawWidth,drawHeight);
#endif

		return 1;

}


/**
 *  @brief Bilderfassung des Framegrabbers anstossen, der Grabber wartet dann nur noch auf Trigger
 *  Dahinter verbirgt sich Fg_AcquireEx
 */
int FrameGrabber :: AcquireStart(int frames)
{

	if (fg==NULL)
		return -1;

	// LotsOfImages is a work around,, GRAB_INFINITE desn't work
	if(frames == 0)
		frames = LOTSOFIMAGES;

	std::cout<<"Fg_AcquireEx... "<<std::endl;
	if((Fg_AcquireEx(fg, PORT_A, frames, ACQ_STANDARD, bufPtr))<0)//ACQ_STD
	{
	   std::cout<<"framegrabber:return AcquireStart, grabActive Flag: "<<grabActive<<std::endl;
	   return freeGrabber("Fg_AcquireEx");
	}
	std::cout<<"Fg_AcquireEx durchgef�hrt... "<<std::endl;

	// Jetzt sem posten, damit der thread auf Bilder wartet
	// gebe thread frei...

	 grabActive = true;
	 imgCounter_ = 0;
	 actualBufferNumber_ = 0;
	 frames_ = frames;
	 std::cout<<"AcquireStart: poste sem..."<<std::endl;
	 sem_post(&eventSem);


	// flags fuer thread

	 return 1; // > 0 : ok


	//return start();

}


/**
 *  @brief Bilderfassung des Framegrabbers stoppen
 *  Dahinter verbirgt sich Fg_AcquireEx
 */
int FrameGrabber :: AcquireStop(void)
{
	int status = 0;
	if (fg==NULL)
		return -1;

	//stop continuous grab - flags fuer thread
	 grabActive = false;
	 status = Fg_stopAcquireEx(fg, PORT_A, bufPtr, 0);

	 if(status < 0)
		 error(fg);
	 //if (threadId_!=0) pthread_abort(threadId_);

    return status;
}



/**
 *  @brief SW Trigger absetzen
 *  ein einzelnes Bild anstossen
 */
int FrameGrabber :: triggerImageSW()
{
	//std::cout<<"framegrabber.cpp - triggerImageSW..."<<std::endl;
	//const int timeout = 0x7FFFEFFE;
	int SoftwareTrgPulse = 1;
	int status = Fg_setParameter(fg, SoftwareTrgPulse_Id_, &SoftwareTrgPulse, 0);


//#if !defined(NDEBUG)
	if(status){
		std::cout<<"framegrabber::trigger Image, status: "<<status<<std::endl;
	}
//#endif

	while(status < 0)
	{
		if (status==FG_SOFTWARE_TRIGGER_BUSY)
		{
			// keine Loops hier !!
			// ... Software Trigger nochmals senden (siehe Softwaredoku)
			std::printf("FG_SOFTWARE_TRIGGER_BUSY\n");
			//status = Fg_setParameter(fg, SoftwareTrgPulse_Id_, &SoftwareTrgPulse, 0);
		}
		else
		{
			error(fg);
	  	return -1; //  freeGrabber("send-SoftwareTrgPulse)"); // freeGrabber ist vllt doch zu hart
		}
	}

	return (status);
}



/**
 *  @brief Bild antriggern
 *  \param toggle Flag fuer Belichtungszeitaenderung
 *  \param triggercode Blitz an oder aus kann am code gelesen werden
 *  \return error <0 Fehler
 */

int FrameGrabber :: live(int toggle,int triggercode)
{
	// falls zuvor irgendetwas passiert ist: raus
	if (!grabActive)
		return -1;

//   int SoftwareTrgPulse = 1;
//   int status = Fg_setParameter(fg, SoftwareTrgPulse_Id_, &SoftwareTrgPulse, 0);
   return triggerImageSW();
}

/**
 *  @brief EXSYNC Zeit veraendern
 *  \param time EXSYNC Puls Zeit Laenge
 *   Fuer Flashmodus die Belichtungszeit (durch Siso gesteuert) setzen
 */
void FrameGrabber :: setFlashExposure(int time)
{
	// falls zuvor irgendetwas passiert ist: raus
	if (!grabActive)
		return;
	if (Fg_setParameter(fg, FG_EXPOSURE, &time, PORT_A)<0)
	{
		std::cout << "setFlashExposure failed" << std::endl;
		// freeGrabber("set-Exposure");
	}

}


/**
 *  @brief Einzelbildaufmnahme, nimmt ein Bild via einem Trigger auf
 *  \param toggle Flag Belichtungszeit Wechsel
 *  \param code fuer flash an- oder aus
 *  \return error <0 Fehler
 */
int FrameGrabber :: snap(int toggle,int triggercode)
{
	int status = live(toggle,triggercode);
	return status;
}

/**
 * @brief Hardware schliessen, Ressourcen freigeben
*  \return 0 OK -1   Error
 */
int   FrameGrabber :: end()
{
	// thread runterfahren
	// acquire stop
	// Speicher freigeben
	// grabber freigeben
	return freeGrabber();
}


/**
* @brief Fehlerbehandlung
* \param fg Grabber struktur
*
*/
void  FrameGrabber :: error(Fg_Struct *fg)
{
  int error;
  const char* str;

  str=Fg_getLastErrorDescription(fg);
  error=Fg_getLastErrorNumber(fg);
  std::printf("siso function: Error(%d): %s\n",error,str);
}

/**
 * @brief true falls ein Siso Grabber detektiert werden konnte 0 sonst.
 */
bool checkForSisoGrabber()
{
	return true;
}

/**
* @brief Akquisse stoppen
* Bildaufnahme auf der Me4 stoppen
*/
int FrameGrabber :: stop()
{

	int status = 0;
	//int dma = 0;      // = CameraPort = PORT_A

	if(!grabActive)
	{
		std::cout<<"framegrabber: grabber schon gestoppt..."<<std::endl;
		return status;
	}
	//thread runterfahren:
	grabActive = false;
	triggerImageSW();

	// Aquisition stoppen
	status = AcquireStop();

	//status = Fg_stopAcquireEx(fg, dma, bufPtr,0);

	if(status < 0)
		error(fg);

	return status;
	// Speicher freigeben
	//freeImageMem();


}

/**
 * @brief Akquisse starten
 * bild thread starten, grab starten
 * Bildaufnahem mit Anzahl Bilder = unendlich starten
*/
int FrameGrabber :: start(int frames)
{
	int status = 0;
	//const int LotsOfImages = 0x7fffffff;

	// grab noch aktiv
	if(grabActive)
	{
	    std::cout<<"frameGrabber.start: grab noch active ..."<<std::endl;
		return status;
	}
	if(frames == 0)
		frames_ = LOTSOFIMAGES;

	frames_=frames;

	// thread initialisieren
	status = initImageThread();

	// thread starten
	status = startImageThread();

	//Fg_Acquire von Siso starten
	status = AcquireStart(frames);

    return status; // > 0 : ok
}


int FrameGrabber::getBufferNumber() const {

	return iMaxImageBuffers_;
} // getBufferNumber


} // namespace precitec
