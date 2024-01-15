/**
 * @file
 * @brief  frameGrabber realisiert den Me4 Zugriff.
 *
 * @author JS
 * @date   20.05.10
 * @version 0.1
 * Erster Wurf
 *
 */



#ifndef _BVHW_H_
#define _BVHW_H_

#include <iostream>
#include <string>
//#include "wrapper/queueProxy.h" // QueueProxy
#include "Poco/BasicEvent.h"

#include "system/sharedMem.h"
#include "fgrab_prototyp.h"
#include "fgrab_struct.h"
#include "fgrab_define.h"

//#include "clser.h"

#include "event/imageShMem.h" // wg Bild-Konstanten Hoehe, Breite, Pufferzahl, ...
#include "sensor.h"

#undef BYTE
#undef DWORD

//#include "defines.h"

namespace precitec {

const int MAX_IMAGE_BUFFERS = greyImage::NumImageBuffers;
const int LOTSOFIMAGES = 0x7fffffff;



typedef Poco::BasicEvent<int> imageEventType;
//typedef std::set<std::string> StringSet;



/**
*  FrameGrabber Klasse
*  Zweck: Zugriff zur Framegrabber Karte uebr den Silicon Software SDK
*
*  Stellt direkt implementierte Zugriffsfunktion
*  und die allgemeinen Zugriffsfunktion SetProperty und GerProperty
*  zur Verfuegung
*
 */
///Zugriff auf Me4
class FrameGrabber
{
public:

	/// Konstruktor mit Message ID
	FrameGrabber(int conID);

	/// Destruktot
	FrameGrabber();

	~FrameGrabber();

	/// setzt nur interne ROI Variablen iAnzImagePixels_  und iAnzImageLines
	void setROI( int width, int height );

	/// setzt nur interen offset Variablen xoff_ und yoff_
	void setOffset( int xOff, int yOff );
    /// Grabber Initialisierung
	int init( int deviceNo );

	/// ROI setzen
	int	 setGrabberRoi(int x, int y, int dx, int dy, bool useOffset);

	/// Roi Werte lesen
	int  getGrabberRoi(int& x, int& y, int& dx, int& dy);



	/// Einen sw trigger an die kamera senden
	int live( int blocknr, int triggerCode );
	int snap( int blocknr, int triggercode );
	/// regulaerer Ausstieg
	int end();

	/**
	* Setzt die Belichtungszeiten bei Belichtungszeitwechsel
	* Funktioniert nur im Mode Pulsweitenmodulation der Kamera
	*
	*/
	void setTimings(int time0, int time1)
		{
			timings[0] = time0;
			timings[1] = time1;
			//std::cout<<"timings: "<< timings[0]<<","<<timings[1]<<std::endl;
		}

	/// der std-Fehler-Ausstieg
	int freeGrabber(const char* functionName=NULL);

	/// die Globalabfrage, kein HW-Zugriff, wenn false
	bool 			bGrabberOnboard;

	/// Tabelle mit Umrechnung phys/log Adressen
	std::vector<unsigned long> logAddr;
	std::vector<unsigned long> physAddr;
	std::vector<unsigned long> nbytes;

	/// thread welcher auf Bilder wartet und an die Analyse einen Puls sendet
	static void* imageGetter(void* frameGrabber);

	///startet den Grabbetrieb, ruft nur die siso grab Funktion auf, initialisiert die grabber flags auf 0,
	/// set grabActive auf true und gibt den gettter thread via sem frei
	int AcquireStart(int frames);

	///stoppt den Grabbetrieb
	int AcquireStop(void);


	/// Bildaufnahme mit thread stoppen
	int stop();

	/// Bildaufnahme mit thread starten
	/// mit frames
	int start(int frames);

	/// set DMA buffers to shared mem
	int setDMA(){ initDMAImageMem(ShMem_ ); return 1; };

	/// setzt alle FGPA-Parameter
	int initFGPAParameter();

	/// set DMA buffers to shared mem, initialize FPGA parameters
	int initMemory( int deviceNo );


	/// gibt Bildpseicher wieder frei
	void freeImageMem();

	/// Initialisiert den Thread der die Kamera-Bilder abholt und Pulse schickt
	int initImageThread();

	/// Startet den Thread
	int startImageThread();

	//int onBoard(){return 0;}
	//int test(void){return 0;}

	/// setzt einzelnen Parameter vom Typ T
	template <class T>
	inline int setParameter(const char* name, T value);

	/// liest einzelnen Parameter vom Typ T
	template <class T>
	inline int getParameter(const char* name, T* value);

	/// liefert die Grabber Properties
	SensorConfigVector getProps(void){return grabberProps_; }

	/// liefert die Anzahl von DMA Puffern
	int getBufferNumber() const;

	int getAnzImagePixels(void){return iAnzImagePixels_;}
	int getAnzImageLines(void){return iAnzImageLines_;}
	int getActualImgNb() const {return imgCounter_ + 1;}

    bool getGrabberModeFlag(void){return grabberControlledMode_;}
    void setGrabberModeFlag(bool flag){grabberControlledMode_ = flag;}

    void setImageNoUse(bool flag){imageNoUseFlag_ = flag;}

    int portFlush();

	volatile bool grabActive;



	imageEventType imageEvent_;
	/// Accessor fuer die SharedMemPtr
	precitec::system::ShMemPtr<byte> imagePtr(int i) { return pImage_[i]; }



private:

	int initBoard(int deviceNo);		///< laedt Applet
	int initSouvisImageMem();			///< Allokiert ImageMem in Hauptspeicher, falls kein SiSo board vorhanden ist
	int initSisoImageMem();				///< Allokiert ImageMem ueber Siso-API
	int initUserImageMem();				///< Allokiert ImageMem ueber User memory
	precitec::system::SharedMem& initSharedImageMem();				///< Puffer aus Shared Memory generieren
	int initDMAImageMem(precitec::system::SharedMem &imageShMem );	///< DMA Speicher auf shared mem setzen
	void freeUserMem();					///< Gibt des DMA wieder frei, shared mem wird nicht deallokiert

	bool isCamClockPresent();			///< Kamera-Detektion ueber clock
	int triggerImageSW();				///< wirft SW-kontrolliert die Aufnahme eines Bildes an
	void setFlashExposure(int time);	///< setzt die Belichtungszeit, wenn der Grabbber diese steuert
	void error(Fg_Struct *fgData);		///<FehlerausgabeRoutine
	volatile int 	imgCounter_;        ///<durchlaufende Nummer
	volatile int    actualBufferNumber_;///<Bildnummer im DMA Speciehr
	Fg_Struct		*fg;				///< Pointer/Handle auf den Grabber
	dma_mem* bufPtr;					///< Puffer mit allen Bildern, Achtung Bild-Ptr muessen extra ermittelt werden
	int     iAnzImagePixels_;			///< Breite des Bildes
	int		iAnzImageLines_;			///< Hoehe des Bildes
	int     iMaxImageBuffers_; 			///< Bildanzahl im DMA Speicher
	int 	xoff_; 						///< x offset
	int 	yoff_;						///< y offset
	int 	mirror_;					///< applet mirror flag
	int timings[2]; 					///< timing-Parameter fuer Pulsweitensteuerung
	int   conID_;						///< Prozess ID
    bool grabberControlledMode_;  		///<true if grabbercontrolled mode
    bool imageNoUseFlag_;               ///<true if image should not used
    int frames_; 						///< Anzahl bilder welche im burst mode aufgenommen werden sollen
	pthread_t threadId_;				///< id des imageGetterThreads
	int SoftwareTrgPulse_Id_;			///< sw trigger id
	pthread_attr_t attr_; 				///<thread attribute
	precitec::system::ShMemPtr<byte> *pImage_; 	///< Shared Memory-Pointer
	SensorConfigVector grabberProps_;			///< da stehen die nach aussen zugaenglichen Properties vom Grabber drin
	precitec::system::SharedMem  ShMem_; 		///< shared memory Klasse


};


/**
 * @brief setzt Parameter auf FPGA
 *
 */
template <class T>
int FrameGrabber :: setParameter(const char*  name, T value)
{
	if (fg==NULL)
		return -1;
	int id = Fg_getParameterIdByName(fg, name);
	int rc = Fg_setParameter(fg, id, &value, 0);
	if (rc<0)
	{
		std::cout << "Siso::set " << name << " failed : " << rc << " : " << Fg_getLastErrorDescription(fg) << std::endl;
	    return rc;
	}
	else
		return 0;
}


/**
 * liesst Parameter aus Framegrabber FPGA
 */
template <class T>
int FrameGrabber :: getParameter(const char*  name, T *value)
{
	if (fg==NULL)
		return -1;

	int id = Fg_getParameterIdByName(fg, name);
	int rc = Fg_getParameter(fg,id,value,0);
	if (rc<0)
	{
		std::cout << "Siso::get " << name << " failed : " << rc << " : " << Fg_getLastErrorDescription(fg) << std::endl;
		return -1;
	}
	else
		return 0;
}





} // namespace precitec

#endif //_BVHW_H_
