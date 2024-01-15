#ifndef TRIGGERCONTROLLER_H_
#define TRIGGERCONTROLLER_H_


/**
 * @file
 * @brief  beinhaltet die Klasse TriggerController
 * Der TriggerController implementiert alle notwendigen Trigger Methoden
 *
 * @author JS
 * @date   01.03.13
 * @version 0.1
 *
 *
 */

#include "Poco/Bugcheck.h"
#include "Poco/SharedPtr.h"
#include "system/timer.h"

#include "common/product.h"
#include "common/triggerContext.h"
#include "event/trigger.h"
#include "event/trigger.proxy.h"

#include "message/device.h"

#include "message/device.server.h"
#include "message/device.proxy.h"


#include "system/timer.h"

#include "Poco/SharedPtr.h"
#include "Poco/Thread.h"
#include "Poco/Timer.h"

#include "triggerController.h"


using Poco::SharedPtr;
using Poco::Timer;
using Poco::TimerCallback;
using Poco::Thread;


#include "grabber/triggerServer.h"


namespace precitec
{
	using namespace system;
	using namespace grabber;
	using namespace Poco;
	using interface::TTrigger;


namespace trigger
{



    /**
   	 *   @brief QNX Timer, benoetigt einen thread, der Timer erzeugt einen Puls der im event Fall abgefangen wird
   	 */
   	/// QNX Timer Methoden
   	class QnxTimer
   	{
   		public:
    		//pthread_t	our_thread_id;
   		QnxTimer(){}
   		virtual ~QnxTimer() {std::cout<<"QnxTimer DTor..."<<std::endl;}

   		/// Init timer handler for live mode, no error is generated
   		bool	initTimer( void );

   		/**
   		 * Start or stop the timer
   		 */
   		void setTimer(int bRun, long liveTimens);


   		/**
   		 * starte thread mit einem message receive
   		 */
   		void startThread(void* arg);

    }; //QnxTimer




	/// Der TriggerController setzt die Kommandos des command servers um
	/// und verwaltet den QNX timerthread oder, abhaengig vom grabber mode,
   	/// verwaltet den grabber controlled mode
	class TriggerController
	{

		enum TriggerMode {swTrigger,exTrigger,grabCtrTrigger};

		public:
            
            virtual ~TriggerController();

            
			TriggerController(grabber::TriggerServer* p_pTriggerServer);

			/// Weist dem TriggerController den TriggerServer zu. Fuer alle Controller gleich
			void copyTriggerServerAddress(TriggerServer* triggerServer);

			/// In Abhaengigkeit der TriggerSource wird der trigger Mode gesetzt, bzw.
			/// ein qnx timer thread gestartet
			void setTriggerSource(precitec::interface::TriggerSource source);


			/// Diese Methode loest sofort ein TriggerEvent aus.
			void singleshot(const std::vector<int>& p_rSensorIds, TriggerContext const& context);


			/// Loest eine Reihe von Intervals aus.
			void burst(TriggerContext const& context, TriggerInterval const& interval);


			/// starte den Trigger nach Unterbrechung erneut
			void burstAgain(unsigned int oTriggerDistanceSave, unsigned int oNbTriggersSave);

			/// Stopt eine Triggerquelle, wenn vorhanden
			int stop();


			///reset imageNumber
			void resetImageNumber();

			/// Bildnummer setzen
			void setImageNumber(int imgNo);

			void setTestImagesPath(std::string const & _testImagesPath);

            void setTestProductInstance(const Poco::UUID &productInstance);

			void setSimulation( bool _onoff );

			/// Diese Methode wird aufgerufen, wenn ein TriggerEvent ansteht.
			void onTrigger(void);


			/// grabber controlled burst starten
			void onGrabCtrTrigger(void);

			/// Diese Methode wird aufgerufen, wenn der Poco Timer aufschlaegt
			void onTimer(Poco::Timer& timer);


#if defined __QNX__ || defined __linux__
			/// Diese Methode wird aufgerufen, wenn der QNX Timer aufschlaegt
			void onTimerQnx(void);
#endif


		private:

			void setupTrigger(unsigned int oTriggerDistance,unsigned int oNbTriggers);

			/// Einzeltrigger auf dem grabber Modl aufrufen
			void setEvent(TriggerContext const& context);

			/// Trigger(burst) auf dem grabber Modul aufrufen: Startet eine Bildsequenz mit dem
			/// HW Timer des grabbers
			void setEvent(TriggerContext const& context, TriggerInterval const& interval);



		private:

			TriggerServer 				*m_pTriggerServer;    		///<  triggerServer
			TriggerMode					m_oTriggerMode; 			///<  enum mit den Trigger Modi
			TriggerContext 				m_oContext;               	///<  trigger context: bildnummer, hw ROI,
			TriggerInterval		 		m_oInterval;				///< Intervall: Anzahl Bilder und zeitl. Abstand
			bool 						m_oEndless;					///< endlose Bildaufnahme ?

			long 						m_oTriggerPerInterval;
			long 						m_oTriggerDistance;
			long 						m_oImagePerInterval;

			bool						m_oTriggerStarted;
			bool                        m_oDbgFlag;

#if defined __QNX__ || defined __linux__
			QnxTimer 					m_oQnxSoftTimer;			///< qnx Timer
#endif
			Poco::SharedPtr<Poco::Timer> m_pTimer;				    ///< Poco timer
			system::Timer 				 m_triggerTimer;			///< Trigger Timer


	};





}//namespace trigger
}//namespace precitec

#endif /*TRIGGERCONTROLLER_H_*/
