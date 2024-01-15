/**
 * @file
 * @brief  Implementierung der Klasse TriggerController
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

#include "message/device.h"

#include "message/device.server.h"
#include "message/device.proxy.h"
#include "system/realTimeSupport.h"


#if defined __QNX__ || defined __linux__
#ifdef __QNX__
#include <sys/neutrino.h>
#endif
#include <sys/prctl.h>
#include <sys/timerfd.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <semaphore.h>
#endif // defined __QNX__ || defined __linux__

#include "system/timer.h"

#include "Poco/SharedPtr.h"
#include "Poco/Thread.h"
#include "Poco/Timer.h"

#include"module/moduleLogger.h"

#include "../include/trigger/triggerController.h"


using Poco::SharedPtr;
using Poco::Timer;
using Poco::TimerCallback;
using Poco::Thread;


#define __logTimer 0


namespace precitec
{
	using namespace system;
	using namespace grabber;
	using namespace Poco;
	using interface::TTrigger;

namespace trigger
{


#if defined __QNX__ || defined __linux__

	static void* messageRcvThread ( void* triggerController);


	/// Infrastruktur fuer qnx timer und puls
	typedef union {
#ifdef __QNX__
   		struct _pulse   pulse;
#else
   		int pulse;
#endif
	} my_message_t;


#ifdef __QNX__
	static struct sigevent         event;
	static int                     chid;
	static int                     conID;
	static int                     rcvid;
	static my_message_t            msg;
	static timer_t                 timer_id;
#endif
	static struct itimerspec       itime;
	static pthread_t			   our_thread_id=0;
    static int                     timerFd = -1;

#ifdef __QNX__
	#define MY_PULSE_CODE    _PULSE_CODE_MINAVAIL
	#define END_TIMER_THREAD _PULSE_CODE_MINAVAIL+1
#else
	#define MY_PULSE_CODE    10
	#define END_TIMER_THREAD 111
#endif

	///Init Timer
	bool QnxTimer::initTimer( void )
	{
#ifdef __QNX__
		chid  = ChannelCreate(0);
		event.sigev_notify = SIGEV_PULSE;
		event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0,
	                                    chid,
	                                    _NTO_SIDE_CHANNEL, 0);
		event.sigev_priority = getprio(0);
		event.sigev_code = MY_PULSE_CODE;
		timer_create(CLOCK_REALTIME, &event, &timer_id);
		conID = (int)event.sigev_coid;
#endif
#ifdef __linux__
        timerFd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
        if (timerFd == -1)
        {
            std::cout << "Creating timerfd failed with: " << strerror(errno) << std::endl;
        }
#endif
		return true;
	} // initTimer


	/// Start or stop the timer
	void QnxTimer::setTimer(int bRun, long liveTimens)
	{
		long sec  = liveTimens/1000000000;
		long nsec = liveTimens%1000000000;
		std::cout<<"softtrigger timer sec: "<<sec<<" , "<<nsec<<std::endl;
		//itime.it_value.tv_sec = sec;
		//itime.it_value.tv_nsec = bRun * nsec; //liveTimens;
		itime.it_value.tv_sec = 0;
		itime.it_value.tv_nsec = bRun * 100000; //liveTimens;
		//Interval
		itime.it_interval.tv_sec  = sec;
		itime.it_interval.tv_nsec = bRun * nsec; //liveTimens;
#ifdef __QNX__
		timer_settime(timer_id, 0, &itime, NULL);
#elif __linux__
        timerfd_settime(timerFd, 0, &itime, NULL);
#endif
	}


	/// starte thread mit einem message receive
	void  QnxTimer::startThread(void* arg)
	{
		// Init the timer ...
		initTimer();
		if(our_thread_id == 0)
			std::cout<<"timer thread ID ist NULL..."<<std::endl;
		else
			std::cout<<"thread ID ist: "<<our_thread_id<<std::endl;

		std::cout<<"Creating thread now..."<<std::endl;
		pthread_create(&our_thread_id, NULL,&messageRcvThread ,(void*)arg);
		std::cout<<"Thread was created..."<<std::endl;
	}

#endif //ifdef QNX







TriggerController::TriggerController(grabber::TriggerServer* p_pTriggerServer) :
		m_pTriggerServer(p_pTriggerServer),
		m_oTriggerMode(swTrigger),
		m_oContext(),
		m_oInterval(),
		m_oEndless(false),
		m_oTriggerPerInterval(0),
		m_oImagePerInterval(0),
		m_oTriggerStarted(false)

{
}

TriggerController::~TriggerController()
{
    close(timerFd);
}

void TriggerController::copyTriggerServerAddress(TriggerServer* triggerServer)
{
	m_pTriggerServer = triggerServer;
}


void TriggerController::setTriggerSource(precitec::interface::TriggerSource source)
{

	//check the dbg output variable from the dataAcquire object
	m_oDbgFlag = m_pTriggerServer->getDbgFlagFromDataAcquire();

	if (source == ExternalTrigger)
	{
		m_oTriggerMode =  exTrigger;
	}
	else if (source == GrabControlledTrigger)
	{
		//std::cout<<"3. grabber controlled ..."<<std::endl;
		m_oTriggerMode = grabCtrTrigger;
	}
    else //if(source == SoftTrigger )
	{
		//std::cout<<" sw controlled ..."<<std::endl;
		if( our_thread_id != 0) // timerthread existiert
		{
			//std::cout<<" 3. sw Trigger existiert schon ..."<<std::endl;
			m_oTriggerMode =  swTrigger;
		}
		else
		{
			m_oTriggerMode =  swTrigger;
#if defined __QNX__ || __linux__
			//std::cout<<"3. qTimer thread starten..."<<std::endl;
			m_oQnxSoftTimer.startThread(this); //qTimer thread starten
#endif
		}
	}
}


void TriggerController::singleshot(const std::vector<int>& p_rSensorIds, TriggerContext const& context)
{
	std::cout << " Recieved singleshot " << std::endl;
    setEvent( context );
}


void TriggerController::burst(TriggerContext const& context, TriggerInterval const& interval)
{
	m_oContext  = context;
	m_oInterval = interval;
	m_oEndless  = interval.nbTriggers() <= 0;	// Verarbeitung ist endlos, wenn keine Laenge definiert wird

	if(!m_pTriggerServer->getAcquireBusyFlag() )
	{
		// Anzahl TriggerSignale pro Interval (= Bilder Pro Interval)
		m_oTriggerPerInterval = m_oInterval.nbTriggers();
		m_oTriggerDistance = m_oInterval.triggerDistance();

		// Bildnummer zuruecksetzen
		m_oImagePerInterval 	= 0;

		// HW ROI in Trigger Kontext setzen
		m_pTriggerServer->triggerImageInfo(m_oContext.HW_ROI_x0,m_oContext.HW_ROI_y0,m_oContext.HW_ROI_dx0,m_oContext.HW_ROI_dy0);

		// Trigger starten mit distance und number of triggers
		setupTrigger(m_oTriggerDistance,m_oTriggerPerInterval);
	}
	else
	{
		std::cout<<"can not start live mode, grab is busy by memory reallocation"<<std::endl;
	}
}

void TriggerController::burstAgain(unsigned int oTriggerDistanceSave, unsigned int oNbTriggersSave)
{

	// Trigger starten
	if(!m_pTriggerServer->getAcquireBusyFlag() )
		setupTrigger(oTriggerDistanceSave,oNbTriggersSave );
	else
		std::cout<<"can not start live mode, grab is busy by memory reallocation"<<std::endl;


}


int TriggerController::stop()
{
	std::cout << "trigger Controller - stop trigger" << std::endl;
	int status = 0;

	if(m_oTriggerMode == swTrigger)
	{
#if defined __QNX__ || defined __linux__
		//std::cout<<"Timer Stop vom sw controlled Trigger ausfuehren..."<<std::endl;

		if(m_oTriggerStarted)
		{
			std::cout<<"trigger controller timer laeuft: stoppen "<<std::endl;
			m_oQnxSoftTimer.setTimer(0,0);
			m_oTriggerStarted = false;
			status = 1;
		}
		else
			std::cout<<"trigger controller timer steht: nicht mehr stoppen "<<std::endl;
#else
		if (m_pTimer.get() != NULL) // POCO Timer getriggert
		{
			m_pTimer->restart(0);  // Aus einem Callback (onTimer) darf nie Stop aufgerufen werden! Es entsteht sonst ein Deadlock
			std::cout<<"timer stop"<<std::endl;
		}
#endif
	}
	else
    {
        m_pTriggerServer->triggerStop(1);
    }

	//status == 0: trigger war schon angehalten
	//status == 1: trigger wurde angehalten

	return(status);
}



void TriggerController::resetImageNumber()
{
	m_pTriggerServer->resetImageNumber( );
	m_oContext.setImageNumber(0);
}


void TriggerController::setImageNumber(int imgNo)
{
	m_pTriggerServer->setImageNumber(imgNo);
	m_oContext.setImageNumber(imgNo);
}


/// Aufruf nach qnx Timer
void TriggerController::onTrigger(void)
{
	// Signal Trigger
	static system::Timer timer("TriggerTimer");
	timer.restart();
	++m_oImagePerInterval;
	// doing the next line before setEvent(context) results in m_oContext.imageNumber = dataAcquire.frameCnt_+1
	//m_oContext.setImageNumber( m_oContext.imageNumber() + 1 );

	int dummyContext = m_oContext.imageNumber();
	int dummyGrabber = m_pTriggerServer->getImageNumberOfGrabber();

	if(m_oDbgFlag)
		std::cout <<"SW trigger/onTrigger: "<<timer<<" image number: " << dummyContext<<" grabber image: "<<dummyGrabber<<std::endl; //Debug Ausgabe

	// beim allerersten timer wurde noch kein Bild aufgenommen: dummyContex=0 und dummyGrabber = 0
	//Test auf Uebertriggerung
	if(dummyGrabber>-1) // bei -1 : keine Kamera konfiguriert ...
	{
		if(dummyGrabber < (dummyContext-1 ) )
		{
			std::cout <<"SW trigger/onTrigger - last grabbed image: "<<dummyGrabber<<std::endl; //Debug Ausgabe


			wmLogTr(dummyGrabber < (dummyContext-10) ? eError: eWarning,"QnxMsg.Grab.Overtriggered ","Grabber uebertriggert: %d,%d\n",dummyContext,dummyGrabber);

			if(dummyGrabber < (dummyContext-10))
			{
				wmFatal( eImageAcquisition, "QnxMsg.Fatal.Overtriggered", "Frame Grabber uebertriggert\n");
			}
		}
	}
#if __logTimer
	std::cout <<"onTrigger: "<<timer<<" TriggerController: " << m_oContext.imageNumber()  << std::endl; //Debug Ausgabe
	std::cout <<"m_oImagePerInterval: "<<m_oImagePerInterval <<" m_oTriggerPerInterval: " << m_oTriggerPerInterval  << std::endl;
#endif

	//Test trigger Kontext
	//HW ROI in trigger Context
	//int x,y,dx,dy;
	//m_pTriggerServer->triggerImageInfo(x,y,dx,dy);

	//m_pTriggerServer->triggerImageInfo(m_oContext.HW_ROI_x0,m_oContext.HW_ROI_y0,m_oContext.HW_ROI_dx0,m_oContext.HW_ROI_dy0);

	// std::cout<<"Trigger Kontex HW_ROI_x0 in triggerController: "<<x<<std::endl;
	// HW ROI in trigger Context
	//m_oContext.HW_ROI_x0 = x;
	//m_oContext.HW_ROI_y0 = y;

	setEvent( m_oContext );

	m_oContext.setImageNumber( m_oContext.imageNumber() + 1 );

	if (!m_oEndless && m_oImagePerInterval >= m_oTriggerPerInterval)
	{
		stop();
	}

}

void TriggerController::onGrabCtrTrigger(void)
{
	//std::cout<<"Grab Controlled burst starten.. "<<std::endl;
	setEvent(m_oContext,m_oInterval);
}


void TriggerController::onTimer(Poco::Timer& timer)
{
	// TriggerCallback aufrufen
	// timer messen
	m_triggerTimer.restart();
#if __logTimer
	std::cout<<m_triggerTimer<<std::endl;
	std::cout<<std::endl;
#endif
	onTrigger();
}



#if defined __QNX__ || defined __linux__
	/// Diese Methode wird aufgerufen, wenn der QNX Timer aufschlaegt
	void TriggerController::onTimerQnx(void)
	{
		// TriggerCallback aufrufen
		// timer messen
		m_triggerTimer.restart();
#if __logTimer
		std::cout<<"onTimerQnx "<< m_triggerTimer<<std::endl;
		std::cout<<std::endl;
#endif
		onTrigger();
	}
#endif




void TriggerController::setupTrigger(unsigned int oTriggerDistance,unsigned int oNbTriggers )
{
//std::cout << "4 .start trigger [distance]: " << oTriggerDistance<< "trigger per interval: " << oNbTriggers<< std::endl;

	if(m_oTriggerMode == swTrigger)
	{
		// Timer zur Zeitausgabe starten
		m_triggerTimer.start();
	#if defined __QNX__ || defined __linux__
	   //std::cout<<"5. sw triggered mode und qnxtimer setzen..."<<std::endl;
	   m_pTriggerServer->triggerMode(2);

	   //laeuft die grab Funktion schon ?
	   if(!m_pTriggerServer->getGrabActive()  )
	   {
		  //std::cout<<"grab ist nicht aktiv ? "<<m_pTriggerServer->getGrabActive()<<std::endl;
		  // Grab starten
		  m_pTriggerServer->startDataAcquire(0);

	   }

	   // timer mit der uebergebener distance starten - falls timr nicht laeuft
	   if(m_oTriggerStarted == false)
	   {
		   std::cout<<"trigger controller timer steht: timer starten "<<std::endl;
		   m_oQnxSoftTimer.setTimer(1,oTriggerDistance ); // qnx timer setzen
	   	   m_oTriggerStarted = true;                      // flag - timer gestartet
	   }
	   else
		   std::cout<<"trigger controller timer laeuft schon: timer nicht mehr starten "<<std::endl;

	#else
		/// triggerAbstand von nSec in mSec umwandeln
		long periodicalInterval = long(oTriggerDistance / 1000000);
		/// Timer aufsetzten. Timer startet sofort
		m_pTimer = new Poco::Timer(0, periodicalInterval);

		std::cout<<"Pocotimer Initialisierung: "<<periodicalInterval<<std::endl;
		//m_pTimer->start(&SoftTrigger::onTimer), Thread::PRIO_HIGH);
		m_pTimer->start(&onTimer), Thread::PRIO_HIGH);
	#endif
	 }
	else   //(m_oTriggerMode == grabCtrTrigger )
	{
        // hier wird der grabber auf grabber controlled gesetzt: 1 grabberControlled
		//std::cout<<"5. grabber controlled mode setzen und grab starten..."<<std::endl;
		m_pTriggerServer->triggerMode(1);
		// grab controlled trigger starten
		onGrabCtrTrigger();
	}

}

void TriggerController::setTestImagesPath( std::string const & _testImagesPath )
{
	m_pTriggerServer->setTestImagesPath( _testImagesPath );
}

void TriggerController::setTestProductInstance(const Poco::UUID &productInstance)
{
    m_pTriggerServer->setTestProductInstance(productInstance);
}

void TriggerController::setEvent(TriggerContext const& context)
{
	poco_assert( m_pTriggerServer );
	//std::cout << "triggerController - triggerEvent "  << context.imageNumber() << std::endl; Debug ausgabe
	m_pTriggerServer->trigger( context );
}

void TriggerController::setEvent(TriggerContext const& context, TriggerInterval const& interval)
{
	poco_assert( m_pTriggerServer );
	//std::cout << "triggerController - triggerEvent burst"  << context.imageNumber() << std::endl; Debug ausgabe
	m_pTriggerServer->trigger( context, interval );
}


#if defined __QNX__ || defined __linux__

	/// QNX timer thread ...
	static void* messageRcvThread ( void* triggerController)
	{

            prctl(PR_SET_NAME, "trigger");

            system::makeThreadRealTime(system::Priority::Sensors);

	   		//SoftTrigger *trigger = ((SoftTrigger*) softTrigger);
	        TriggerController  *trigger = ((TriggerController*) triggerController);
            bool loop = true;

	   		//int dummy = trigger->status;

	   		int i= 0;
	  		//std::cout<<"messageThread active, dummy= "<<dummy<<std::endl;
	  		//for (;;)	  
	  		while(loop)
	  		{
#ifdef __QNX__
	  		    rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
	  		     if (rcvid == 0)
	  		     {    // pulse
	  		          if (msg.pulse.code == MY_PULSE_CODE)
	  		          {
	  			       	//timerHandler();
	  		        	//std::cout<<"qnxTimer in thread  arrived..."<<i<<std::endl; //Debug ausgabe
	  			       	i++;
	  			        trigger->onTimerQnx();//onTimerQnx();    //->onTimerQnx();
	   			       } // else other pulses
	  		          else
	  		          {
	  		        	  //kill thread
	  		        	  // std::cout<<"kill timer receive thread..."<<std::endl; Debug ausgabe
	  		        	  loop = false;
	  		        	  //pthread_exit( NULL );

	  		          }
	  			  } // else messages ...
#endif
#ifdef __linux__
                 uint64_t expired = 0;
                 const std::size_t size = read(timerFd, &expired, sizeof(uint64_t));
                 if (size != sizeof(uint64_t))
                 {
                     std::cout << "Failed to read timerFd" << std::endl;
                     continue;
                 }
                 if (expired != 1)
                 {
                     std::cout << "Timer expired " << expired << " times" << std::endl;
                 }
	  		     i++;
	  		     trigger->onTimerQnx();//onTimerQnx();    //->onTimerQnx();
#endif
	  		 }
	   		// std::cout<<"timer messageThread finished...n"<<std::endl; Debug ausgabe
	  		return(NULL);
  	}
#endif //QNX





}//namespace trigger
}//namesopace precitec

