/**
 * @file
 * @brief DataAcquire realisiert den Framegrabber und Kamerazugriff.
 *
 * @author JS
 * @date   20.05.10
 * @version 0.1
 * Erster Wurf
 *
 */

#ifndef DATAACQUIREIMPL_H_
#define DATAACQUIREIMPL_H_
#include <string>

#include "event/sensor.interface.h"
#include "event/sensor.proxy.h"
#include "dataAcquire.h"
#include "framegrabber.h"
#include "camera.h"

#include "Poco/BasicEvent.h"
#include "Poco/Mutex.h"

#include "event/trigger.h"
#include "event/trigger.interface.h"
#include "event/inspectionCmd.interface.h"

#include "common/triggerContext.h"
#include "common/defines.h"

#include "event/sensor.h"
//#include "image/ipLineImage.h"

#include "system/timer.h"

#include "system/types.h"

#include "message/device.h"

#include <deque>

//#define CAMERA_GRAYIMAGE	1
//#define CAMERA_LASERLINE	2

enum CameraType{CamGreyImage = 1,CamLaserLine = 2};

/// kleine Hilfe
template<class T>
T value(const std::string& s)
{
std::istringstream is(s);
T val;
is >> val;
return val;
}


struct BaseCamConf
{
		float  exposuretime;
		int    offset;
		int    xstart;
		int    ystart;
		int    width;
		int    height;
		bool   status;
};
typedef   BaseCamConf baseCameraConfig;



namespace precitec
{
	using namespace interface;

	/// Zugriff auf Kamera/Framegrabber hinter dem key Value und Trigger Interface
	class DataAcquire
	{
	public:
		DataAcquire(TSensor<EventProxy> &sensorProxy);
		//DataAcquire(TSensor<AbstractInterface> &sensorProxy);
		virtual ~DataAcquire(); //{};

		/// Setze Parameter int
		int setGrabberParameter(std::string const& key, int const & value);

		/// Setze Parameter double
		int setGrabberParameter(std::string const& key, double const & value);

		/// Setze Parameter double
		int setGrabberParameter(std::string const& key, std::string const & value);

		/// liefere Parameter
		//int getGrabberParameter(std::string const & key,std::string &value);

		/// Bilderfassung initialisieren auf Bildgroesse
		int initDataAcquire(baseCameraConfig& pBaseConfig);

		int closeDataAcquire(void);


		/// Kamera initialisieren
		int initCamera();


		///Setze Kamera Parameter mittels std::string
		int setCameraParameter(std::string const& key,std::string const& value);

		///Setze Kamera Parameter mittels int
		int setCameraParameter(std::string const& key,int& value);

		///Setze Kamera Parameter mittels float
		int setCameraParameter(std::string const& key,float& value);

		///Lese Kamera Parameter mittles std::string
		Types getCameraParameter(std::string const& key,std::string &value);

		///Lese Grabber Parameter mittels std::string
		Types getGrabberParameter(std::string const& key,std::string &value);

		bool getGrabActive(){return(frameGrabber_.grabActive);}

		/// Kamera Port schliessen
		int closeCamera();

		/// Kamera ROI setzen
		int setROICamera(int x, int y, int dx, int dy, bool useOffset );

		/// Grabber ROI bzw. Grabber Image Buffer Groesse setzen
		int  setROIGrabber(int x, int y, int dx, int dy,int offset );

		/// Hole Bildgroesse
		int getImageSize(int& x, int& y, int& dx, int& dy);

		/// Bilderfassungsthread initialisieren
		int initImageThread();

		/// Acquire auf der Me4 starten, thread starten
		int startDataAcquire(int frames); // Anzahl Frames mitgeben

		/// Acquire auf der Me4, thread stoppen
		int stopDataAcquire ();

		/// Bilderfassungsthread starten
		int startImageThread();

		/// grabber stoppen, thread runterfahren, DMA freigeben
		int stopAcquireMem();

		/// DMA neu anlegen, thread starten,grabber starten
		int startAcquireMem(int frames);

		/// Bilderfassung stoppen, DMA freigeben - DMA neu anlegen Bilderfassung neustarten
		int grabberReset(void);

		/// Einzelbild ohne Kontext antriggern
		int trigger (void);

		/// Einzelbild antriggern
		int trigger (TriggerContext const& context );

		/// grabbercontrolled triggern
		int trigger (TriggerContext const& context, TriggerInterval const& interval);

		/// image thread generiert event, welches onImageEvent aufruft
		void onImageEvent(const void* pSender, int& i);

		///reset frameCounter to zero
		void resetImageNumber();

		/// set frameCounter to imgNo
		void setImageNumber(int imgNo);

		/// image number from framegrabber side
		int getImageNumberFromFrameGrabber(void);


		/// liefere die Liste mit den Kamera Keys inkl. min,max und def ( = Properties)
		SensorConfigVector getCameraProperties(void){return camera_.getProps(); }

		/// liefere die Liste mit den Grabber Keys inkl. min,max und def ( = Properties)
		SensorConfigVector getGrabberProperties(void){ return frameGrabber_.getProps();}

		/// liefert die von der dataAcquire Klasse gehaltene Sensorkonfiguration
		Configuration getSensorConfiguration(void){return sensorConfiguration_;}

		/// schreibt die aktuelle Sensorkonfiguration aus der Hardware in die Variable der dataacquire Klasse
		void setMirrorConfiguration(Configuration &config){sensorConfiguration_ = config ;configOK_ = true;}

		/// ersetze einzelnen keyValue in der sensor Konfiguration
		int  replaceSensorConfiguration(SmpKeyValue keyValue );

		/// Gibt es eine Sensor Konfiguration
		bool isMirrorConfigrationOK(void) {return configOK_;}

		/// Hole key aus der gespiegelten Konfiguration
		SmpKeyValue getMirrorKeyValue(std::string const& key);

		/// befinden wir uns im grabber controlled mode
		bool grabberControlledMode(void){return frameGrabber_.getGrabberModeFlag(); }

		/// grabber trigegr mode setzen
		void setModeFlag(bool flag){ frameGrabber_.setGrabberModeFlag(flag);  }

		/// set dbg Flag with start of the App
		void setDbgOut(const int& dbgOut);

		///check the dbg flag
		bool getDbgOut(){return dbgOut_;}

		/// tell the framegrabber that a grabbed  image should not used
		void setImageNoUse(bool flag){frameGrabber_.setImageNoUse(flag); }

		///check the acquire busy flag
		bool getAcquireBusy(){return acquireBusy_;}

		///signal a framegrabber error
		bool grabberHasError() {if(fgSuccess_ < 0) return true; else return false;}


		/**
		 * @brief				returns if the given image number references an image in the grabber dma buffer (true), or if it is expired (false)
		 * @param	p_oImageNb	image number from image context (in grabber also denominated 'frame counter')
		 * @return	bool		if the given image number references an image in the grabber dma buffer
		 */
		bool isImgNbInBuffer(uint32_t p_oImageNb);

        void setInspectionCmd(const std::shared_ptr<TInspectionCmd<AbstractInterface>> &inspectionCmd)
        {
            m_inspectionCmd = inspectionCmd;
        }

        unsigned int getMaxSensorWidth() const
        {
            return camera_.getMaxSensorWidth();
        }

        unsigned int getMaxSensorHeight() const
        {
            return camera_.getMaxSensorHeight();
        }

	private:
		TSensor<EventProxy> &m_rSensorProxy;  ///<Schnittstelle zum Analyzer
		int          	fgSuccess_;			///<grabber ok
		int          	camSuccess_;		///<Camera ok
		FrameGrabber 	frameGrabber_;		///grabber Klasse
		Camera       	camera_;			///<Camera Klasse
		int          	imageWidth_;		///< on chip Bildbreite - HW ROI
		int          	imageHeight_;		///< on chip Bildhoehe  - HW ROI
		int          	imageXOff_;			///< on chip x Offset   - HW ROI
		int          	imageYOff_;			///< on chip y Offset   - HW ROI
		int				frameCnt_;			///<Bildzaehler
		TriggerContext 	triggerContext_;	///<Kontext zum Bildtrigger
		mutable Poco::FastMutex imageGrabMutex_;

		/// Image ist vom Typ BImage
		/// TLineImage<byte> BImage;


		//Image ist vom Typ LImage: typedef LImage Image; --Analyzer_Interface/include/image
		// --> typedef TLineImage<byte> LImage;
		// also ist dmaImage ein TLineImage<byte>
		//Image dmaImage_[MAX_IMAGE_BUFFERS];		///<Array mit den DMA Bildadressen

		//Image *pDmaImg;
		std::vector<image::BImage> dmaList_;    ///< Liste mit den dma Speichen Adressen der Bilder im dma

		Poco::FastMutex mutex_;					///< Poco Mutex
		system::Timer grabTimer_;				///<timer fuer den grabber

		bool configOK_;                         ///<Konfiguration vorhanden
		bool grabberCtrMode_;                   ///<ist der grabber controlled mode aktiv ?
        bool dbgOut_;                           ///<sorgt fuer zusaetzliche Meldungen in der Konsole  pro Bild
        bool acquireBusy_;						///<acquire busy, f.E. at memory reallocations
        int  reOpenFlag_;                        ///<control for CamPort reinitialise
        int flushPortFlag_;                      ///<control for Cam Communication Port buffer

		//typedef std::vector<SmpKeyValue> Configuration;
		Configuration sensorConfiguration_;		///<Haelt die aktuelle Sensorkonfiguration mit property,value, min, max und default

        // counter of all images received in onImageEvent, used as imageId in BImage
        uint32_t overallImageCounter_;
        // the imageIds currently hold in the frame grabber
        std::deque<uint32_t> imagesInBuffer_;

        std::shared_ptr<TInspectionCmd<AbstractInterface>> m_inspectionCmd;

	};

} // namespace precitec


#endif /*DATAACQUIREIMPL_H_*/
