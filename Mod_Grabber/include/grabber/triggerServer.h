/**
* @file
* @brief Headerfile zum TrigeerServer
*
* @author JS
* @date   20.05.10
* @version 0.1
* Erster Wurf
*/


#ifndef TRIGGERSERVER_H_
#define TRIGGERSERVER_H_

#include "Poco/Mutex.h"
#include "Poco/Stopwatch.h"
#include "Poco/Path.h"
#include "Poco/Timestamp.h"
#include "event/trigger.h"
#include "event/trigger.interface.h"

#include "event/sensor.h"
#include "event/sensor.proxy.h"
#include "dataAcquire.h"
#include "trigger/imageDataHolder.h"
#include "trigger/sequenceProvider.h"
#include "grabber/sharedMemoryImageProvider.h"

namespace precitec
{
    using namespace image;
    using namespace interface;

namespace grabber
{
    /// Verarbeitet TriggerEvents vom Trigger Modul
    class TriggerServer : public TTrigger<AbstractInterface>
    {
        public:
            TriggerServer( TSensor<EventProxy>& sensorProxy, DataAcquire& grabber );

        public:
            /// typischerweise ein single trigger, welcher SW getriggert wird
            void trigger(TriggerContext const& context);
            // special overload for including sample data
            void trigger(const std::vector<int>& p_rSensorIds, TriggerContext const& context);

            /// burst trigger typischerweise grabber controlled
            void trigger(TriggerContext const& context, TriggerInterval const& interval);

            /// setze trigger Mode 1 grabber controlled, 2 sw controlled
            void triggerMode(int mode);

            /// stoppe den Grabber, falls es grabber controlled laeuft
            void triggerStop(int flag);

            /// Bildnummer zuruecksetzen
            void resetImageNumber();

            void setImageNumber(int imgNo);

            /// Bildnummer des gegrabbten Bildes
            int getImageNumberOfGrabber(void);

            /// ist die grabFunktion auf der Grabber Seite gestartet ?
            bool getGrabActive(void); // {return (m_rGrabber.getGrabActive());}

            int startDataAcquire(int frames); // m_rGrabber.startDataAcquire(frames);

            int triggerImageInfo(int&x,int&y,int&dx,int&dy);

            bool getDbgFlagFromDataAcquire(void);

            bool getAcquireBusyFlag(void);

            void uninit(void);

			void setTestImagesPath( std::string const & _testImagesPath );

            void setTestProductInstance(const Poco::UUID &productInstance);

        private:
            TSensor<EventProxy>& m_rSensorProxy;		///< event schnittstelle zum analyzer
            /// Server kann nur eine Triggeranforderung auf einmal erfuellen
            Poco::FastMutex *serverAccess_;			///< Mutex zum Schutz des Triggers

            //TLineImage in AnalyzerInterface/include/image
            // TLineImage abgeeleitet von TImage in in AnalyzerInterface/include/image/ipImage.h
            //typedef TLineImage<byte> LImage; ( AnalyzerInterface/include/image/image.h, Klasse selbst in ipLineImage.h )
            //typedef LImage Image; ( Interfaces/include/event/sensor.h )
            DataAcquire &m_rGrabber;					///< Kamera und Grabber
            Poco::Timestamp time_;					///< Poco timer
            system::Timer hilfsTimer;				///< System timer
            SequenceProvider m_SequenceProvider;
            SharedMemoryImageProvider m_noGrabberSharedMemory;
            bool m_hasCamera;
    };

}

}

#endif /*TRIGGERSERVER_H_*/
