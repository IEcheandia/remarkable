/**
 * @file
 * @brief  Schnittstelle zum Triggern der Bildaufnahme
 *
 * Der TriggerServer stellt alle notwendigen funktionen zur Bildaufnahme zur Verfuegung
 *
 * @author JS / AB (nsec statt usec wg. reellem Trigger und Genauigkeit)
 * @date   20.05.10 / 2012-10
 * @version 0.1
 * Erster Wurf
 */

#include "grabber/triggerServer.h"
#include "Poco/DirectoryIterator.h"
#include "common/bitmap.h"
#include "common/connectionConfiguration.h"
#include "Poco/Mutex.h"
#include "Poco/ScopedLock.h"
#include "module/moduleLogger.h"
#include "../Mod_VideoRecorder/include/videoRecorder/types.h"

#include <algorithm>

#include "common/systemConfiguration.h"

//#include <ctime>
//#include <sstream>

using namespace fileio;

namespace precitec
{

namespace grabber
{

using namespace Poco;


TriggerServer::TriggerServer( TSensor<EventProxy>& sensorProxy, DataAcquire& grabber ) :
	m_rSensorProxy( sensorProxy ),
	serverAccess_(new Poco::FastMutex()),
	m_rGrabber(grabber)
{
    m_hasCamera = false;
    char* oEnvStrg = getenv((char *)"WM_STATION_NAME");
    if (oEnvStrg != nullptr)
    {
        if (strcmp("WM-QNX-PC", oEnvStrg) == 0)
        {
            m_hasCamera = SystemConfiguration::instance().getBool("HasCamera", true);
        }
        else
        {
            m_hasCamera = false;
        }
    }
    else
    {
        m_hasCamera = false;
    }
}


void TriggerServer::trigger(TriggerContext const& context)
{
    if (SystemConfiguration::instance().getBool("ImageTriggerViaEncoderSignals", false) == true)
    {
        TriggerContext oTriggerContext = context;
        triggerImageInfo(oTriggerContext.HW_ROI_x0,oTriggerContext.HW_ROI_y0,oTriggerContext.HW_ROI_dx0,oTriggerContext.HW_ROI_dy0);
        if (oTriggerContext.imageNumber() == 0)
        {
            m_rGrabber.resetImageNumber();
        }
        m_rGrabber.trigger(oTriggerContext );
    }
    else
    {
        m_rGrabber.trigger(context );
    }
}

void TriggerServer::trigger(const std::vector<int>& p_rSensorIds, TriggerContext const& context)
{
#if !defined(NDEBUG)
    wmLog(eInfo, "\tTriggerServer:trigger() image number: %d\n", context.imageNumber() );
#endif
    TriggerContext hilfsContext;
    hilfsContext = context;
    hilfsContext.setProductInstance(context.productInstance());
    hilfsTimer.elapsed();
    hilfsContext.sync(hilfsTimer.us());
    hilfsTimer.restart();

    BImage byteImage;
    fileio::SampleDataHolder sampleDataHolder;
    const bool hasImage = m_SequenceProvider.getImage(hilfsContext, byteImage);
    const bool hasSample = m_SequenceProvider.getSamples(hilfsContext, sampleDataHolder);

    if (!p_rSensorIds.empty())
    {
        bool allValid = true;
        for (int sensorId : p_rSensorIds)
        {
            if (sensorId <= interface::eImageSensorMax)
            {
                if (!hasImage)
                {
                    allValid = false;
                    break;
                }
                continue;
            }
            if (std::none_of(sampleDataHolder.allData.begin(), sampleDataHolder.allData.end(), [sensorId] (const auto &sample) { return sample.sensorID == sensorId; }))
            {
                allValid = false;
                break;
            }
        }
        if (!allValid)
        {
            m_rSensorProxy.simulationDataMissing(hilfsContext);
            return;
        }
    }

    if (hasImage)
    {
        m_rSensorProxy.data(CamGreyImage, hilfsContext, byteImage);
    }
    if (!hasImage && !hasSample)
    {
        wmLog(eError, "imageDataArray not found");
//         ImageDataHolder imageDataHolder = imageDataArray[m_idx];
//         image::BImage byteImage = imageDataHolder.getByteImage();
//         if(imageDataHolder.getIsExtraDataValid())
//         {
//             hilfsContext.HW_ROI_x0 = imageDataHolder.getHardwareRoiOffsetX();
//             hilfsContext.HW_ROI_y0 = imageDataHolder.getHardwareRoiOffsetY();
//             hilfsContext.HW_ROI_dx0 = byteImage.width();
//             hilfsContext.HW_ROI_dy0 = byteImage.height();
//             // uint32_t imageNumberExt = imageDataHolder.getImageNumber(); //we will not use the external image number here
//         }
//
//         m_rSensorProxy.data(CamGreyImage, hilfsContext, byteImage);
    }

    if (hasSample)
    {
        int nSensors = sampleDataHolder.allData.size();
        for( int sensorIdx=0; sensorIdx<nSensors; sensorIdx++ )
        {
            fileio::SampleDataHolder::OneSensorData oneSensorData = sampleDataHolder.allData.at(sensorIdx);
            int nSamples = oneSensorData.dataVector.size();
            int sensorID = oneSensorData.sensorID;
            TSmartArrayPtr<int>::ShArrayPtr* pIntPointer = new TSmartArrayPtr<int>::ShArrayPtr[1];
            pIntPointer[0] = new int[nSamples];

            for( int sampleIdx=0; sampleIdx<nSamples; sampleIdx++ )
            {
                int sampleValue = oneSensorData.dataVector.at(sampleIdx);
                pIntPointer[0][sampleIdx] = sampleValue;
            }

            try
            {
                const precitec::image::Sample sample(pIntPointer[0],nSamples);
                m_rSensorProxy.data(sensorID, hilfsContext, sample);
                std::ostringstream oMsg;
            }
            catch(const std::exception &p_rException)
            {
                std::ostringstream oMsg;
                oMsg  << __FUNCTION__ << " " << p_rException.what() << "\n";
                wmLog(eWarning, oMsg.str());
            }
            catch(...)
            {
                std::ostringstream oMsg;
                oMsg  << __FUNCTION__ << " Unknown exception encountered.\n";
                wmLog(eWarning, oMsg.str());
            }

            pIntPointer[0] = 0; // decrement smart pointer reference counter
            delete[] pIntPointer;
            pIntPointer = NULL;
        }
    }
}

/**
 * @brief Bildsequenz anstossen, grabberControlled
 * \param context  Triggercontext
 * \param interval Triggerinterval
 *
 * Achtung: Das Property "Device1_Process0_Trigger_ExsyncFramesPerSec" muss existieren
 * \return error <0 falls Fehler
 */
void TriggerServer::trigger(TriggerContext const& context, TriggerInterval const& interval)
{
		int error = 0;
		std::cout << "AppGrabber::trigger burst grabber controlled: " << context.imageNumber() << std::endl;
		bool dummy = m_rGrabber.grabberControlledMode();


		//Abfrage muss noch verwendet werden
		error = m_rGrabber.stopDataAcquire();
		 if(error<0)
		        	std::cout<<"grabber konnte nicht gestoppt werden..."<<std::endl;


        if( dummy )
        {
        	// framerate: triggerdistance : Triggerabstand in nSek.
        	// frames triggeranzahl

        	int frames = (int)interval.nbTriggers();
        	double framerate = 1000000000.0 / (double)interval.triggerDistance(); // ns -> Hz (=1/sec)

        	//framerate setzen: int setGrabberParameter(string const& key, int const & value);
        	error = m_rGrabber.setGrabberParameter("Device1_Process0_Trigger_ExsyncFramesPerSec",framerate);

        	wmLog(eInfo, "triggerserver burst: Triggernumber %d, framerate (Hz): %d\n", interval.nbTriggers(), (int)framerate);
        	m_rGrabber.startDataAcquire(frames);
        }
        else
        {
        	wmLog(eInfo, "triggerserver burst: grabber not in controlled mode.. \n");
        }


}


/**
 * @brief der tgrigger server bietet den zugriff auf ein starten des Grabbers
 * \param int frames
  */
int TriggerServer::startDataAcquire(int frames)
{
	int error = 0;
	error = m_rGrabber.startDataAcquire(frames);

	return (error);
}

/**
 * @brief Grabber controlled modus vom trigger server setzen
 * \param mode 1 grabber controlled
  */
void TriggerServer::triggerMode(int mode)
{
	int error = 0;

	bool dummy = m_rGrabber.grabberControlledMode();

 	if((mode==1)&&(!dummy)) // grabber controlled angefordert, nicht gesetzt
	{
		 std::cout<<"triggerServer triggerMode auf triggerControlled setzen"<<std::endl;
		 error = m_rGrabber.setGrabberParameter("Device1_Process0_Trigger_TriggerMode",mode);
	}
    else if( (mode==2)&&(dummy)   )//sw controlled aangefordert, grabber controlled gesetzt
	{
		std::cout<<"triggerServer triggerMode auf sw Controlled setzen"<<std::endl;
		error = m_rGrabber.setGrabberParameter("Device1_Process0_Trigger_TriggerMode",mode);
	}
    if(error<0)
    {
    	std::cout<<"dummy,error: "<<dummy<<" "<<error<<std::endl;
    	wmLogTr(eWarning, "QnxMsg.Workflow.BadGrabberMode", "Grabber could not be set to mode %d\n", mode);
    } else
    {
    	wmLog(eInfo, "Trigger server: trigger set to mode %d\n", mode);
    }
}


/// stoppe den Grabber, falls es grabber controlled laeuft
void TriggerServer::triggerStop(int flag)
{
	m_rGrabber.stopDataAcquire();
}


/**
 * @brief Bildnummer zuruecksetzen
 *
 */
void TriggerServer::resetImageNumber()
{
	m_rGrabber.resetImageNumber();
}

void TriggerServer::setImageNumber(int imgNo)
{
    m_rGrabber.setImageNumber(imgNo);
}

int TriggerServer::getImageNumberOfGrabber(void)
{
    int dummy = m_rGrabber.getImageNumberFromFrameGrabber();
    return(dummy);
}


int TriggerServer::triggerImageInfo(int&x,int&y,int&dx,int&dy)
{
	m_rGrabber.getImageSize(x,y,dx,dy);
	return 1;
}

bool TriggerServer::getDbgFlagFromDataAcquire(void)
{
	return(m_rGrabber.getDbgOut());
}

bool TriggerServer::getAcquireBusyFlag(void)
{
	return(m_rGrabber.getAcquireBusy());
}


bool TriggerServer::getGrabActive(void)
{
	return(m_rGrabber.getGrabActive());
}

void TriggerServer::uninit(void)
{
    m_SequenceProvider.close();
}

void TriggerServer::setTestImagesPath( std::string const & _testImagesPath )
{
    m_SequenceProvider.setBasepath(_testImagesPath);
}

void TriggerServer::setTestProductInstance(const Poco::UUID &productInstance)
{
    m_SequenceProvider.setTestProductInstance(productInstance);
}


} 	// grabber
}	// precitec
