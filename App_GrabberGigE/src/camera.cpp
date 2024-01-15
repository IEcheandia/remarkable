// project includes
#include "camera.h"
// poco includes
#include "Poco/Util/XMLConfiguration.h"
// wm includes
#include "common/defines.h"
#include "image/image.h"
#include "common/systemConfiguration.h"
#include "common/connectionConfiguration.h"
#include "system/timer.h"
#include "system/realTimeSupport.h"
// aravis includes
#include <arv.h>
// stl includes
#include <fstream>
#include <thread>

#include <sys/resource.h>
#include <sys/prctl.h>

namespace precitec {
namespace grabber  {

#define DEBUG_ENCODER_CONFIGURATION   0

static void arv_new_buffer_cb (ArvStream *stream, ApplicationData *data)
{    
    if ( data->camera->get("Debug")->value<bool>() ) { wmLog( eDebug, "arv_new_buffer_cb\n"); }
    
	ArvBuffer *buffer = arv_stream_try_pop_buffer (stream);
    
	if (ARV_IS_BUFFER (buffer)) 
    {
        switch( arv_buffer_get_status( buffer ) )
        {
            case ARV_BUFFER_STATUS_SUCCESS:
                data->buffer_count++;
                break;                     
            case ARV_BUFFER_STATUS_ABORTED:
                wmLog( eDebug, "New buffer: ABORTED!\n");
                break;
            case ARV_BUFFER_STATUS_CLEARED:
                wmLog( eDebug, "New buffer: CLEARED!\n");
                break;
            case ARV_BUFFER_STATUS_UNKNOWN:
                wmLog( eDebug, "New buffer: UNKNOWN!\n");
                break;
            case ARV_BUFFER_STATUS_TIMEOUT:
                wmLog( eDebug, "New buffer: TIMEOUT!\n");
                break;
            case ARV_BUFFER_STATUS_MISSING_PACKETS:
                wmLog( eDebug, "New buffer: MISSING PACKETS!\n");
                break;
            case ARV_BUFFER_STATUS_WRONG_PACKET_ID:
                wmLog( eDebug, "New buffer: WRONG PACKET ID!\n");
                break;
            case ARV_BUFFER_STATUS_SIZE_MISMATCH:
                wmLog( eDebug, "New buffer: SIZE MISMATCH!\n");
                break;
            case ARV_BUFFER_STATUS_FILLING:
                wmLog( eDebug, "New buffer: FILLING!\n");
                break;
        }            
        
        if ( arv_buffer_get_status( buffer ) == ARV_BUFFER_STATUS_SUCCESS ) 
        { 
            data->camera->queueImage( buffer );
        }
        
        if ( data->buffer_count >= data->buffer_max ) { data->camera->cancel(); }
	}
}


static void arv_stream_cb (void *user_data, ArvStreamCallbackType type, ArvBuffer *buffer)
{
	if (type == ARV_STREAM_CALLBACK_TYPE_INIT) 
    {
		if (!arv_make_thread_realtime (int(system::Priority::Sensors))) { wmLog(eDebug, "Failed to make stream thread realtime\n"); }
	}
}

static void control_lost_cb(UNUSED ArvDevice *device, void *user_data)
{
    wmLog(eError, "Control to camera lost\n");
    wmFatal( eImageAcquisition, "QnxMsg.Fatal.ImageAcquisition", "Camera does not produce images anymore\n");
    reinterpret_cast<Camera*>(user_data)->uninitialize();
}

static void watchDog( Camera* camera )
{    
    prctl(PR_SET_NAME, "watchdog");
    bool exit = false; 
    
    if ( camera )
    {
        if ( camera->get("Debug")->value<bool>() ) { wmLog( eDebug, "Camera::watchDog started\n" ); }
        
        int oldImageNum = camera->currentImageNumber();
        int cycle = camera->cycleNumber();
        
        while ( camera->isCapturing() && !exit )
        {            
            sleep( 1 );
            
            if ( camera->isCapturing() ) // while the thread was sleeping, the main thread could have stopped capturing images ...
            {                
                if ( cycle != camera->cycleNumber() ) // in this case the next cycle has already started, while this thread was sleeping - this means there is already a new watchDog thread, and we should exit here ...
                {
                    exit = true;
                }
                else if ( camera->currentImageNumber() > oldImageNum )
                {
                    oldImageNum = camera->currentImageNumber();
                } 
                else if ( camera->currentImageNumber() == oldImageNum )
                {
                    wmFatal( eImageAcquisition, "QnxMsg.Fatal.ImageAcquisition", "Camera does not produce images anymore\n");
                    camera->uninitialize();
                    exit = true;
                }
            }
        }
        
        if ( camera->get("Debug")->value<bool>() ) { wmLog( eDebug, "Camera::watchDog ended\n" ); }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

namespace
{
/**
 * Helper method for the unit test constructor.
 **/
TSensor<EventProxy>& getSensorProxy()
{
    static TSensor<EventProxy> s_sensorProxy{};
    return s_sensorProxy;
}
}

Camera::Camera()
    : m_sensorProxy{getSensorProxy()}
    , m_handleImageThread(&Camera::handleImageThread, this)
{
}

Camera::Camera( TSensor<EventProxy>& p_sensorProxy )
    : m_sensorProxy( p_sensorProxy )
    , m_wmBaseDir(getenv("WM_BASE_DIR") ? (std::string(getenv("WM_BASE_DIR")) + "/") : "/tmp/precitec/")
    , m_configDir( m_wmBaseDir )
    , m_capturing( false )
    , m_cycle( 0 )
    , m_frameTimer(std::make_unique<system::ElapsedTimer>())
    , m_handleImageThread(&Camera::handleImageThread, this)
{
    bool oHasCamera = true;
    char* oEnvStrg = getenv((char *)"WM_STATION_NAME");
    if (oEnvStrg != nullptr)
    {
        if (strcmp("WM-QNX-PC", oEnvStrg) == 0)
        {
            oHasCamera = SystemConfiguration::instance().getBool("HasCamera", true);
        }
        else
        {
            oHasCamera = false;
        }
    }
    else
    {
        oHasCamera = false;
    }

    m_activeForConfiguration = SystemConfiguration::instance().getInt("CameraInterfaceType", 0) == 1 && oHasCamera;

    if (!m_activeForConfiguration)
    {
        return;
    }

    {
        adjustPriority(uint32_t(system::Priority::Sensors) - 2);
    }

    // SystemConfig Switches for camera encoder interface
    m_oImageTriggerViaCameraEncoder = SystemConfiguration::instance().getBool("ImageTriggerViaCameraEncoder", false);
    wmLog(eDebug, "m_oImageTriggerViaCameraEncoder (bool): %d\n", m_oImageTriggerViaCameraEncoder);
    m_oCameraEncoderBurstFactor = SystemConfiguration::instance().getInt("CameraEncoderBurstFactor", 1);
    wmLog(eDebug, "m_oCameraEncoderBurstFactor (int): %d\n", m_oCameraEncoderBurstFactor);

    m_appData.buffer_count  = 0;
	m_appData.chunks        = nullptr;
	m_appData.chunk_parser  = nullptr;
    m_appData.camera        = this;

	m_configDir.pushDirectory("config");
    
    initialize();    

    if (m_vendor == Vendor::Baumer)
    {
        /* For some reason the Baumer camera is unable to be initialized properly during its initial bootup.
         * It does not produce any images during the first burst sequence, leading the system into "Not Ready".
         * To prevent this, after the init sequence, we need to additionaly create the aravis stream and then
         * destroy both the stream and camera. After this a repeated initialization performs as expected and
         * we can aqcuire images during burst as usual.
         */

        std::unique_lock<std::mutex> cameraLock{m_cameraMutex};

        const bool debug = get("Debug")->value<bool>();
        GError *error = nullptr;

        if (debug)
        {
            wmLog(eDebug, "Creating arv stream \n");
        }
        m_arvStream = arv_camera_create_stream ( m_arvCamera, arv_stream_cb, nullptr, &error );
        checkError("CreateStream", &error);

        if (debug)
        {
            wmLog( eDebug, "Camera::reset for Baumer \n" );
        }
        uninitialize();
        initialize();
    }
};

Camera::~Camera()
{
    m_shuttingDown = true;
    m_condition.notify_all();
    m_handleImageThread.join();

    cancel();
    
    if ( m_arvCamera )
    {
        g_object_unref(m_arvCamera);
        g_object_unref(m_arvStream);
    }
}

void Camera::initializeWork(void)
{
    buildPropCatalog("Root", true , 0);
    wmLog(eDebug, "Found %d genicam properties\n", m_parameters.m_paramConfig.size());

    parameterize();

    GError *error = nullptr;
    // set packet size and log some infos about the camera ...
    arv_camera_gv_set_packet_size(m_arvCamera, 8228, &error);

    wmLog( eDebug, "gv n_stream channels  = %d\n",       arv_camera_gv_get_n_stream_channels      (m_arvCamera, &error));
    checkError("GetNStreamChannels", &error);
    wmLog( eDebug, "gv packet delay       = %d ns\n",    arv_camera_gv_get_packet_delay           (m_arvCamera, &error));
    checkError("getPacketDelay", &error);
    wmLog( eDebug, "gv packet size        = %d bytes\n", arv_camera_gv_get_packet_size            (m_arvCamera, &error));
    checkError("GetPacketSize", &error);

    // the hw-roi unfortunately needs a special treatment, buffers need to be allocated, etc.
    int x,y, w, h;
    arv_camera_get_region(m_arvCamera, &x, &y, &w, &h, &error);
    checkError("GetRegion", &error);
    get("Window.X")->setValue( x );
    get("Window.Y")->setValue( y );
    get("Window.W")->setValue( w );
    get("Window.H")->setValue( h );
    arv_camera_set_region( m_arvCamera, x, y, w, h, &error );
    checkError("SetRegion", &error);
    bool mirrorX = SystemConfiguration::instance().getBool("ImageMirrorActive", false);
    bool mirrorY = SystemConfiguration::instance().getBool("ImageMirrorYActive", false);

    if (mirrorX)
    {
        if (mirrorY)
        {
            m_imageFillMode = image::ImageFillMode::Reverse;

        }
        else
        {
            m_imageFillMode = image::ImageFillMode::FlippedHorizontal;
        }
    }
    else
    {
        if (mirrorY)
        {
            m_imageFillMode = image::ImageFillMode::FlippedVertical;
        }
        else
        {
            m_imageFillMode = image::ImageFillMode::Direct;
        }

    }

    initializeAcquisition();
    initLedTrigger();
    if (isImageTriggerViaCameraEncoder())
    {
        initEncoderInterface();
    }
}

void Camera::adjustPriority(uint32_t priority)
{
    // first pass a new resource limit for RLIMIT_RTPRIO
    system::raiseRtPrioLimit();
    system::makeThreadRealTime(priority);
}

interface::KeyHandle Camera::set(interface::SmpKeyValue p_keyValue)
{
    std::unique_lock<std::mutex> cameraLock{m_cameraMutex};
    const auto &rKey		( p_keyValue->key() );
    auto keyValue = m_parameters.get( rKey );

	if ( keyValue->key() != "?" ) 
    {
        GError *error = nullptr;
        const auto special = isSpecialProperty(keyValue->key());
        // the keys are typically invalid??
		//if (p_keyValue->isValueValid() == false) 
        //{
		//	return -1;
		//}
		const Types oType( keyValue->type() );
		switch (oType) 
        {
		case TBool: 
        {
			const auto oValue	(p_keyValue->value<bool>());
			keyValue->setValue(oValue);
            if (!special)
            {
                arv_device_set_boolean_feature_value(m_arvDevice, keyValue->key().c_str(), keyValue->value<bool>(), &error );
                checkError(keyValue->key(), &error);
            }
            wmLog( eDebug, "TBOOL Camera::set( %s, %s )\n", keyValue->key().c_str(), oValue ? "TRUE" : "FALSE" );
			break;
        }
        case TInt: 
        {
			const auto oValue	(p_keyValue->value<int>());                                    
			keyValue->setValue(oValue);
            if (!special)
            {
                arv_device_set_integer_feature_value(m_arvDevice, keyValue->key().c_str(), keyValue->value<int>(), &error );
                checkError(keyValue->key(), &error);
            }
            wmLog( eDebug, "TINT Camera::set( %s, %i )\n", keyValue->key().c_str(), oValue );
            break;
        }
        case TFloat:
        {
			const auto oValue	(p_keyValue->value<float>());
			keyValue->setValue(oValue);
            if (!special)
            {
                arv_device_set_float_feature_value(m_arvDevice, keyValue->key().c_str(), keyValue->value<float>(), &error );
                checkError(keyValue->key(), &error);
            }
            wmLog( eDebug, "TFLOAT Camera::set( %s, %f )\n", keyValue->key().c_str(), oValue );
			break;
        }
        case TString:
        {
            const auto oValue{p_keyValue->value<std::string>()};
            keyValue->setValue(oValue);
            if (!special)
            {
                arv_device_set_string_feature_value(m_arvDevice, keyValue->key().c_str(), keyValue->value<std::string>().c_str(), &error);
                checkError(keyValue->key(), &error);
            }
            wmLog( eDebug, "TSTRING Camera::set( %s, %f )\n", keyValue->key().c_str(), oValue );
            break;
        }
        default:
			break;
		} 

        if (special)
        {
            // some keys need a special treatment ...
            updateSpecialProperties( rKey );
        }

        // write the current state of the properties to a file
        // only if the camera was initialized, otherwise default values overwrite values in camera.xml
        if (m_arvCamera && m_arvStream)
        {
            writeToFile( configFile().toString(), m_parameters.m_paramConfig );
        }

		return keyValue->handle();
	}

	return -1;
}
    

interface::SmpKeyValue Camera::get(interface::Key p_key) const
{
    return m_parameters.get( p_key );
}


interface::Configuration Camera::getConfig() const
{
    return m_parameters.m_paramConfig;
}



void Camera::initialize()
{
    m_sharedMemory.init();

    if ( get("Debug")->value<bool>() ) { wmLog( eDebug, "Camera::initialize()\n" ); }
    
    m_parameters.clear();

    GError *error = nullptr;
    if ( m_arvCamera != nullptr )
    {
        g_object_unref(m_arvCamera);
        m_arvCamera = nullptr;
    }

    // during system startup it is possible that the link to the camera is not yet ready
    // in that case the camera initialization fails and the whole system goes into not ready
    // But a short time afterwards (manual testing shows 2 s) the link to the camera becomes ready
    // Thus lets try to find the camera multiple times before going into not ready
    for (int i = 0; i < 5 && !m_arvCamera; i++)
    {
        // incrementally wait longer between the intervals to test
        // maximum wait time 10 s.
        Poco::Thread::sleep(i * 1000);
        m_arvCamera = arv_camera_new("192.168.255.2", &error);
        checkError("CameraNew", &error);
    }

    if ( m_arvCamera == nullptr )
    {
        wmFatal( eImageAcquisition, "QnxMsg.Fatal.CameraInit", "Cannot initialize camera ...\n");        
        return;
    }
    
    m_arvDevice = arv_camera_get_device( m_arvCamera );
    if ( m_arvDevice == nullptr )
    {
        g_object_unref(m_arvCamera);
        m_arvCamera = nullptr;
        wmFatal( eImageAcquisition, "QnxMsg.Fatal.CameraInit", "Cannot initialize camera ...\n");        
        return;
    }

    g_signal_connect(m_arvDevice, "control-lost", G_CALLBACK(control_lost_cb), this);
    
    m_arvGenicam    = arv_device_get_genicam( m_arvDevice );
        
    wmLog( eInfo, "Camera found!\n");
    detectVendor();
    wmLog( eInfo, "Camera model name: %s\n", arv_camera_get_model_name(m_arvCamera, &error) );
    checkError("GetModelName", &error);
    wmLog( eInfo, "Camera device id: %s\n", arv_camera_get_device_id(m_arvCamera, &error) );
    checkError("GetDeviceId", &error);

    downloadGenicamXml();

    // determine sensor size and set the maximum hw-roi size
    gint maxWidth, maxHeight;
    arv_camera_get_sensor_size(m_arvCamera, &maxWidth, &maxHeight, &error);
    checkError("GetSensorSize", &error);
    get("Window.H")->setMaximum( maxHeight );
    get("Window.Y")->setMaximum( maxHeight-10 );
    get("Window.HMax")->setValue( maxHeight );
    get("Window.HMax")->setMaximum( maxHeight );
    get("Window.HMax")->setMinimum( maxHeight );
    get("Window.X")->setMaximum( maxWidth-10 );
    get("Window.W")->setMaximum( maxWidth );
    get("Window.WMax")->setValue( maxWidth );
    get("Window.WMax")->setMaximum( maxWidth );
    get("Window.WMax")->setMinimum( maxWidth );
    wmLog( eInfo, "Camera resolution (%d,%d)\n", maxWidth, maxHeight );
}

void Camera::initializeAcquisition()
{
    switch (m_vendor)
    {
    case Vendor::Photonfocus:
        initializeAcquisitionPhotonfocus();
        break;
    default:
        // nothing
        break;
    }

    GError *error = nullptr;
    arv_camera_set_acquisition_mode( m_arvCamera, ARV_ACQUISITION_MODE_CONTINUOUS, &error);
    checkError("SetAcquisitionMode", &error);

    arv_device_set_string_feature_value (m_arvDevice, "ExposureMode", "Timed", &error );
    checkError("SetExposureMode", &error);
}

void Camera::initializeAcquisitionPhotonfocus()
{
    GError *error = nullptr;
    // one has to adjust the max frame rate property, otherwise the rate will get clipped...
    arv_device_set_float_feature_value(m_arvDevice, "AcquisitionFrameRateMax", 10000.0, &error );
    checkError("AcquisitionFrameRateMax", &error);

    if (get("StatusLine")->value<bool>())
    {
        arv_device_set_boolean_feature_value(m_arvDevice, "EnStatusLine",  true, &error);
        checkError("EnStatusLine", &error);
    }
}

void Camera::initLedTrigger()
{
    switch (m_vendor)
    {
    case Vendor::Photonfocus:
        initLedTriggerPhotonfocus();
        break;
    case Vendor::Basler:
        initLedTriggerBasler();
        break;
    case Vendor::Baumer:
        initLedTriggerBaumer();
        break;
    default:
        // nothing
        break;
    }
}

void Camera::initLedTriggerPhotonfocus()
{
    GError *error = nullptr;
    arv_device_set_string_feature_value (m_arvDevice, "LineOut2_LineMode",      "Output", &error );
    checkError("SetLineOut2_LineMode", &error);
    arv_device_set_string_feature_value (m_arvDevice, "LineOut2_LineSource",    "ExpReady", &error );
    checkError("SeLineOut2_LineSource", &error);
    arv_device_set_boolean_feature_value(m_arvDevice, "LineOut2_LineInverter",  true, &error);
    checkError("SetLineOut2_LineInverter", &error);
}

void Camera::initLedTriggerBasler()
{
    GError *error = nullptr;
    arv_device_set_string_feature_value(m_arvDevice, "LineSelector", "Line3", &error);
    if (checkError("LineSelector", &error))
    {
        return;
    }
    arv_device_set_string_feature_value(m_arvDevice, "LineMode", "Output", &error);
    checkError("LineMode", &error);
    arv_device_set_string_feature_value(m_arvDevice, "LineSource", "ExposureActive", &error);
    checkError("LineSource", &error);
    arv_device_set_boolean_feature_value(m_arvDevice, "LineInverter", true, &error);
    checkError("LineInverter", &error);
}

void Camera::initLedTriggerBaumer()
{
    GError *error = nullptr;
    arv_device_set_string_feature_value(m_arvDevice, "LineSelector", "Line5", &error);
    if (checkError("LineSelector", &error))
    {
        return;
    }
    arv_device_set_string_feature_value(m_arvDevice, "LineSource", "ExposureActive", &error);
    checkError("LineSource", &error);
    arv_device_set_boolean_feature_value(m_arvDevice, "LineInverter", false, &error);
    checkError("LineInverter", &error);
    arv_device_set_string_feature_value(m_arvDevice, "LineFormat", "PushPull", &error);
    checkError("LineFormat", &error);
}

void Camera::initEncoderInterface()
{
    switch (m_vendor)
    {
    case Vendor::Photonfocus:
        initEncoderInterfacePhotonfocus();
        if (CameraEncoderBurstFactor() == 1)
        {
            initAreaImageTriggerPhotonfocus(250, 1);
        }
        else if ((CameraEncoderBurstFactor() > 1) && (CameraEncoderBurstFactor() <= 100))
        {
            //initLineImageTriggerPhotonfocus(250, 1); // Trigger regime for line image mode not yet defined
            initAreaImageTriggerPhotonfocus(250, 1);
        }
        else
        {
            wmLogTr(eError, "QnxMsg.Grab.ABCDEF", "Burst factor for image trigger via encoder interface is not in valid range\n");
        }
        break;
    case Vendor::Basler:
        break;
    case Vendor::Baumer:
        break;
    default:
        // nothing
        break;
    }
}

void Camera::initEncoderInterfacePhotonfocus()
{
    if (m_arvDevice == nullptr) return;

#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "initEncoderInterfacePhotonfocus Start\n");
#endif

    GError *error = nullptr;
    guint32 oBuffer = 0;

    oBuffer = 5; // RS422
    arv_device_write_register(m_arvDevice, 0x00300C40, oBuffer, &error); // LineIn0_LineFormat
    checkError("LineIn0_LineFormat", &error);

    oBuffer = 5; // RS422
    arv_device_write_register(m_arvDevice, 0x00300C4C, oBuffer, &error); // LineIn1_LineFormat
    checkError("LineIn1_LineFormat", &error);

    arv_device_read_register(m_arvDevice, 0x00300C40, &oBuffer, &error); // LineIn0_LineFormat
    checkError("LineIn0_LineFormat", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "LineIn0_LineFormat: %x\n", oBuffer);
#endif

    arv_device_read_register(m_arvDevice, 0x00300C4C, &oBuffer, &error); // LineIn1_LineFormat
    checkError("LineIn1_LineFormat", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "LineIn1_LineFormat: %x\n", oBuffer);
#endif

    arv_device_set_string_feature_value (m_arvDevice, "LineIn0_LineMode", "Input", &error );
    checkError("LineIn0_LineMode", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "LineIn0_LineMode: %s\n", arv_device_get_string_feature_value(m_arvDevice, "LineIn0_LineMode", &error));
#endif

// returns an error ?
//    arv_device_set_string_feature_value (m_arvDevice, "LineIn0_LineFormat", "RS422", &error );
//    checkError("LineIn0_LineFormat", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "LineIn0_LineFormat: %s\n", arv_device_get_string_feature_value(m_arvDevice, "LineIn0_LineFormat", &error));
#endif

    arv_device_set_string_feature_value (m_arvDevice, "LineIn1_LineMode", "Input", &error );
    checkError("LineIn1_LineMode", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "LineIn1_LineMode: %s\n", arv_device_get_string_feature_value(m_arvDevice, "LineIn1_LineMode", &error));
#endif

// returns an error ?
//    arv_device_set_string_feature_value (m_arvDevice, "LineIn1_LineFormat", "RS422", &error );
//    checkError("LineIn1_LineFormat", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "LineIn1_LineFormat: %s\n", arv_device_get_string_feature_value(m_arvDevice, "LineIn1_LineFormat", &error));
#endif

    arv_device_set_string_feature_value (m_arvDevice, "EncoderSourceA", "LineIn0", &error );
    checkError("EncoderSourceA", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "EncoderSourceA: %s\n", arv_device_get_string_feature_value(m_arvDevice, "EncoderSourceA", &error));
#endif

    arv_device_set_string_feature_value (m_arvDevice, "EncoderSourceB", "LineIn1", &error );
    checkError("EncoderSourceB", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "EncoderSourceB: %s\n", arv_device_get_string_feature_value(m_arvDevice, "EncoderSourceB", &error));
#endif

    arv_device_set_string_feature_value (m_arvDevice, "EncoderMode", "HighResolution", &error );
    checkError("EncoderMode", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "EncoderMode: %s\n", arv_device_get_string_feature_value(m_arvDevice, "EncoderMode", &error));
#endif

   arv_device_set_integer_feature_value (m_arvDevice, "EncoderDivider", 100, &error );
    checkError("EncoderDivider", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "EncoderDivider: %d\n", arv_device_get_integer_feature_value(m_arvDevice, "EncoderDivider", &error));
#endif

    arv_device_set_string_feature_value (m_arvDevice, "EncoderOutputMode", "DirectionUp", &error );
    checkError("EncoderOutputMode", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "EncoderOutputMode: %s\n", arv_device_get_string_feature_value(m_arvDevice, "EncoderOutputMode", &error));
#endif

    arv_device_set_string_feature_value (m_arvDevice, "EncoderResetSource", "Off", &error );
    checkError("EncoderResetSource", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "EncoderResetSource: %s\n", arv_device_get_string_feature_value(m_arvDevice, "EncoderResetSource", &error));
#endif

    arv_device_set_string_feature_value (m_arvDevice, "EncoderResetActivation", "RisingEdge", &error );
    checkError("EncoderResetActivation", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "EncoderResetActivation: %s\n", arv_device_get_string_feature_value(m_arvDevice, "EncoderResetActivation", &error));
#endif

    arv_device_set_boolean_feature_value(m_arvDevice, "EncoderAcquisitionStartReset", true, &error );
    checkError("EncoderAcquisitionStartReset", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "EncoderAcquisitionStartReset: %d\n", arv_device_get_boolean_feature_value(m_arvDevice, "EncoderAcquisitionStartReset", &error));
#endif

    arv_device_set_integer_feature_value (m_arvDevice, "EncoderValueAtReset", 0, &error );
    checkError("EncoderValueAtReset", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "EncoderValueAtReset: %d\n", arv_device_get_integer_feature_value(m_arvDevice, "EncoderValueAtReset", &error));
#endif

    arv_device_set_integer_feature_value (m_arvDevice, "EncoderStartValue", 0, &error );
    checkError("EncoderStartValue", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "EncoderStartValue: %d\n", arv_device_get_integer_feature_value(m_arvDevice, "EncoderStartValue", &error));
#endif

    // manual encoder reset is needed for starting the encoder counter !
    arv_device_execute_command(m_arvDevice, "EncoderReset", &error);
    checkError("EncoderReset", &error);

#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "initEncoderInterfacePhotonfocus End\n");
#endif
}

void Camera::initAreaImageTriggerPhotonfocus(int p_oEncoderDivider, int p_oTriggerDivider)
{
    if (m_arvDevice == nullptr) return;

#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "initAreaImageTriggerPhotonfocus Start\n");
#endif
    GError *error = nullptr;

    arv_device_set_string_feature_value (m_arvDevice, "AcquisitionStart_TriggerMode", "Off", &error );
    checkError("AcquisitionStart_TriggerMode", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "AcquisitionStart_TriggerMode: %s\n", arv_device_get_string_feature_value(m_arvDevice, "AcquisitionStart_TriggerMode", &error));
#endif

    arv_device_set_boolean_feature_value(m_arvDevice, "EnAcquisitionFrameRate", false, &error );
    checkError("EnAcquisitionFrameRate", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "EnAcquisitionFrameRate: %d\n", arv_device_get_boolean_feature_value(m_arvDevice, "EnAcquisitionFrameRate", &error));
#endif

    arv_device_set_string_feature_value (m_arvDevice, "FrameStart_TriggerMode", "On", &error );
    checkError("FrameStart_TriggerMode", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "FrameStart_TriggerMode: %s\n", arv_device_get_string_feature_value(m_arvDevice, "FrameStart_TriggerMode", &error));
#endif

    arv_device_set_string_feature_value (m_arvDevice, "FrameBurstStart_TriggerMode", "Off", &error );
    checkError("FrameBurstStart_TriggerMode", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "FrameBurstStart_TriggerMode: %s\n", arv_device_get_string_feature_value(m_arvDevice, "FrameBurstStart_TriggerMode", &error));
#endif

    arv_device_set_string_feature_value (m_arvDevice, "ExposureStart_TriggerMode", "Off", &error );
    checkError("ExposureStart_TriggerMode", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "ExposureStart_TriggerMode: %s\n", arv_device_get_string_feature_value(m_arvDevice, "ExposureStart_TriggerMode", &error));
#endif

    arv_device_set_string_feature_value (m_arvDevice, "FrameStart_TriggerSource", "Encoder0", &error );
    checkError("FrameStart_TriggerSource", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "FrameStart_TriggerSource: %s\n", arv_device_get_string_feature_value(m_arvDevice, "FrameStart_TriggerSource", &error));
#endif

    arv_device_set_string_feature_value (m_arvDevice, "FrameStart_TriggerActivation", "RisingEdge", &error );
    checkError("FrameStart_TriggerActivation", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "FrameStart_TriggerActivation: %s\n", arv_device_get_string_feature_value(m_arvDevice, "FrameStart_TriggerActivation", &error));
#endif

    arv_device_set_float_feature_value (m_arvDevice, "FrameStart_TriggerDelay", 0.0, &error );
    checkError("FrameStart_TriggerDelay", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "FrameStart_TriggerDelay: %f\n", arv_device_get_float_feature_value(m_arvDevice, "FrameStart_TriggerDelay", &error));
#endif

    arv_device_set_integer_feature_value (m_arvDevice, "FrameStart_TriggerDivider", p_oTriggerDivider, &error );
    checkError("FrameStart_TriggerDivider", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "FrameStart_TriggerDivider: %d\n", arv_device_get_integer_feature_value(m_arvDevice, "FrameStart_TriggerDivider", &error));
#endif

    arv_device_set_integer_feature_value (m_arvDevice, "EncoderDivider", p_oEncoderDivider, &error );
    checkError("EncoderDivider", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "EncoderDivider: %d\n", arv_device_get_integer_feature_value(m_arvDevice, "EncoderDivider", &error));
#endif

#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "initAreaImageTriggerPhotonfocus End\n");
#endif
}

void Camera::initLineImageTriggerPhotonfocus(int p_oEncoderDivider, int p_oTriggerDivider)
{
    if (m_arvDevice == nullptr) return;

#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "initLineImageTriggerPhotonfocus Start\n");
#endif
    GError *error = nullptr;

    arv_device_set_string_feature_value (m_arvDevice, "AcquisitionStart_TriggerMode", "Off", &error );
    checkError("AcquisitionStart_TriggerMode", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "AcquisitionStart_TriggerMode: %s\n", arv_device_get_string_feature_value(m_arvDevice, "AcquisitionStart_TriggerMode", &error));
#endif

    arv_device_set_string_feature_value (m_arvDevice, "FrameStart_TriggerMode", "Off", &error );
    checkError("FrameStart_TriggerMode", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "FrameStart_TriggerMode: %s\n", arv_device_get_string_feature_value(m_arvDevice, "FrameStart_TriggerMode", &error));
#endif

    arv_device_set_string_feature_value (m_arvDevice, "FrameBurstStart_TriggerMode", "On", &error );
    checkError("FrameBurstStart_TriggerMode", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "FrameBurstStart_TriggerMode: %s\n", arv_device_get_string_feature_value(m_arvDevice, "FrameBurstStart_TriggerMode", &error));
#endif

    arv_device_set_string_feature_value (m_arvDevice, "ExposureStart_TriggerMode", "Off", &error );
    checkError("ExposureStart_TriggerMode", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "ExposureStart_TriggerMode: %s\n", arv_device_get_string_feature_value(m_arvDevice, "ExposureStart_TriggerMode", &error));
#endif

    arv_device_set_string_feature_value (m_arvDevice, "FrameBurstStart_TriggerSource", "Encoder0", &error );
    checkError("FrameBurstStart_TriggerSource", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "FrameBurstStart_TriggerSource: %s\n", arv_device_get_string_feature_value(m_arvDevice, "FrameBurstStart_TriggerSource", &error));
#endif

    arv_device_set_string_feature_value (m_arvDevice, "FrameBurstStart_TriggerActivation", "RisingEdge", &error );
    checkError("FrameBurstStart_TriggerActivation", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "FrameBurstStart_TriggerActivation: %s\n", arv_device_get_string_feature_value(m_arvDevice, "FrameBurstStart_TriggerActivation", &error));
#endif

    arv_device_set_float_feature_value (m_arvDevice, "FrameBurstStart_TriggerDelay", 0.0, &error );
    checkError("FrameBurstStart_TriggerDelay", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "FrameBurstStart_TriggerDelay: %f\n", arv_device_get_float_feature_value(m_arvDevice, "FrameBurstStart_TriggerDelay", &error));
#endif

    arv_device_set_integer_feature_value (m_arvDevice, "AcquisitionBurstFrameCount", 5, &error );
    checkError("AcquisitionBurstFrameCount", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "AcquisitionBurstFrameCount: %d\n", arv_device_get_integer_feature_value(m_arvDevice, "AcquisitionBurstFrameCount", &error));
#endif

    arv_device_set_boolean_feature_value(m_arvDevice, "EnAcquisitionFrameRate", true, &error );
    checkError("EnAcquisitionFrameRate", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "EnAcquisitionFrameRate: %d\n", arv_device_get_boolean_feature_value(m_arvDevice, "EnAcquisitionFrameRate", &error));
#endif

    arv_device_set_float_feature_value (m_arvDevice, "AcquisitionFrameTime", 2000.0, &error ); // us
    checkError("AcquisitionFrameTime", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "AcquisitionFrameTime: %f\n", arv_device_get_float_feature_value(m_arvDevice, "AcquisitionFrameTime", &error));
#endif

    arv_device_set_integer_feature_value (m_arvDevice, "FrameBurstStart_TriggerDivider", p_oTriggerDivider, &error );
    checkError("FrameBurstStart_TriggerDivider", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "FrameBurstStart_TriggerDivider: %d\n", arv_device_get_integer_feature_value(m_arvDevice, "FrameBurstStart_TriggerDivider", &error));
#endif

    arv_device_set_integer_feature_value (m_arvDevice, "EncoderDivider", p_oEncoderDivider, &error );
    checkError("EncoderDivider", &error);
#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "EncoderDivider: %d\n", arv_device_get_integer_feature_value(m_arvDevice, "EncoderDivider", &error));
#endif

#if DEBUG_ENCODER_CONFIGURATION
    wmLog(eDebug, "initLineImageTriggerPhotonfocus End\n");
#endif
}

void Camera::uninitialize()
{
    if ( m_arvStream != nullptr )
    {
        arv_stream_set_emit_signals (m_arvStream, FALSE);
        g_object_unref( m_arvStream );
        m_arvStream = nullptr;
    }
    if ( m_arvCamera != nullptr )
    {
        arv_camera_stop_acquisition (m_arvCamera, nullptr);
        m_arvDevice = nullptr;
        g_object_unref(m_arvCamera);
        m_arvCamera = nullptr;
    }
    
    m_capturing = false;
}
    
void Camera::burst(TriggerContext const& context, interface::TriggerInterval const& interval)
{
    std::unique_lock<std::mutex> cameraLock{m_cameraMutex};
    const bool debug = get("Debug")->value<bool>();
    if ( get("Debug")->value<bool>() ) { wmLog( eDebug, "Camera::burst()\n" ); }

    if (get("ReuseLastImage")->value<bool>())
    {
        if (!m_imagesInSharedMemory.empty())
        {
            auto &imageData{m_imagesInSharedMemory.back()};
            const auto &size = std::get<3>(imageData);
            image::TLineImage<byte> image{std::get<1>(imageData), size};
            image.setImageId(std::get<0>(imageData));

            m_triggerContext = context;
            m_triggerContext.HW_ROI_x0  = get( "Window.X" )->value<int>();
            m_triggerContext.HW_ROI_y0  = get( "Window.Y" )->value<int>();
            m_triggerContext.HW_ROI_dx0 = get( "Window.W" )->value<int>();
            m_triggerContext.HW_ROI_dy0 = get( "Window.H" )->value<int>();
            m_triggerContext.setImageNumber(0);

            sendImage(image, size);

            return;
        }
    }
    

    // case 1 - the camera was not available during boot-up, so initialize() has not completed successfully, m_arvCamera is null ...
    if ( m_arvCamera == nullptr )
    {  
        wmFatal( eImageAcquisition, "QnxMsg.Fatal.CameraInit", "Cannot initialize camera ...\n");        
        return;
    }
    
    GError *error = nullptr;
    auto payload = arv_camera_get_payload( m_arvCamera, &error );
    checkError("GetPayload", &error);
    bool createBuffers = false;
    if ( m_arvStream != nullptr )
    {
        if (debug)
        {
            wmLog(eDebug, "Reusing stream\n");
        }
        if (payload != m_payload)
        {
            if (debug)
            {
                wmLog(eDebug, "Start/stop streaming thread\n");
            }
            arv_stream_set_emit_signals (m_arvStream, FALSE);
            arv_stream_stop_thread(m_arvStream, true);
            arv_stream_start_thread(m_arvStream);
            arv_stream_set_emit_signals (m_arvStream, true);
            createBuffers = true;
        }
    }
    else
    {
        if (debug)
        {
            wmLog(eDebug, "Creating arv stream\n");
        }
        m_arvStream = arv_camera_create_stream ( m_arvCamera, arv_stream_cb, nullptr, &error );
        checkError("CreateStream", &error);

        // case 2 - camera was working once, but the network-connection was lost. m_arvCamera and others are not null, but not valid anymore, the stream will not get created ...
        if ( m_arvStream == nullptr )
        {
            initialize();
            m_arvStream = arv_camera_create_stream ( m_arvCamera, arv_stream_cb, nullptr, &error );
            checkError("CreateStream - try 2", &error);
            if ( m_arvStream == nullptr ) { return; }
        }

        g_signal_connect (m_arvStream, "new-buffer", G_CALLBACK (arv_new_buffer_cb), &m_appData);
        createBuffers = true;

        // disable any packet resends, as this seems to cause trouble at higher trigger frequencies ...
        g_object_set( m_arvStream, "packet-resend", ARV_GV_STREAM_PACKET_RESEND_NEVER, NULL);
        // timeout of 2 ms - image transmission is about 1 ms, so any value larger than 2 is already too late for us
        g_object_set( m_arvStream, "packet-timeout", 2000, NULL);
        arv_stream_set_emit_signals (m_arvStream, TRUE);

    }
    m_payload = payload;
    if (debug)
    {
        wmLog(eDebug, "Payload: %d\n", m_payload);
    }

    if (createBuffers)
    {
        if (debug)
        {
            wmLog(eDebug, "Creating buffers\n");
        }
        std::unique_lock<std::mutex> lock{m_sharedMemoryMutex};
        int offset = 0;
        if (!m_imagesInSharedMemory.empty())
        {
            offset = std::get<1>(m_imagesInSharedMemory.back()).offset();
        }
        lock.unlock();
        m_sharedMemory.resetOffset(offset);
        m_maxNumberImagesInSharedMemory = m_sharedMemory.size() / payload;
        for (std::size_t i = 0; i < m_maxNumberImagesInSharedMemory; i++)
        {
            auto sharedMem = m_sharedMemory.nextImagePointer(payload);
            arv_stream_push_buffer(m_arvStream, arv_buffer_new(payload, sharedMem.get()));
        }
    }
                
    m_triggerContext = context;
    m_triggerContext.HW_ROI_x0  = get( "Window.X" )->value<int>();
	m_triggerContext.HW_ROI_y0  = get( "Window.Y" )->value<int>();
    m_triggerContext.HW_ROI_dx0 = get( "Window.W" )->value<int>();
	m_triggerContext.HW_ROI_dy0 = get( "Window.H" )->value<int>();

    m_appData.buffer_count  = 0;
    m_appData.buffer_max = interval.nbTriggers();
    
    // image trigger via camera encoder only if it is configured and if WM is in automatic mode
    if ((isImageTriggerViaCameraEncoder()) && (interval.state() == workflow::eAutomaticMode))
    {
        uint32_t oTriggerDeltaUM = interval.triggerDelta();
        uint32_t oTriggerDeltaMM = oTriggerDeltaUM / 100;
        wmLog(eDebug, "oTriggerDeltaUM: %d oTriggerDeltaMM: %d\n", oTriggerDeltaUM, oTriggerDeltaMM);
        wmLog(eDebug, "CameraEncoderBurstFactor: %d\n", CameraEncoderBurstFactor());
        if ((oTriggerDeltaUM % 100) != 0)
        {
            wmLogTr(eError, "QnxMsg.Grab.ABCDEF", "Trigger Distance is not on a boundary of 0.1mm !\n");
        }
        uint32_t oEncoderDividerValue = oTriggerDeltaMM * 10; // this is valid for 100.000 pulses per meter, after quadrature decoding
        wmLog(eDebug, "oEncoderDividerValue 1: %d\n", oEncoderDividerValue);
        oEncoderDividerValue /= CameraEncoderBurstFactor();
        wmLog(eDebug, "oEncoderDividerValue 2: %d\n", oEncoderDividerValue);
        uint32_t oFrameTriggerDividerReg {1};
        uint32_t oEncoderDividerReg {255};
        if (oEncoderDividerValue > 255)
        {
            oFrameTriggerDividerReg = 10;
            oEncoderDividerReg = oEncoderDividerValue / 10;
            if ((oEncoderDividerValue % 10) != 0)
            {
                wmLogTr(eError, "QnxMsg.Grab.ABCDEF", "Encoder divider is not suitable for the divider register !\n");
            }
        }
        else
        {
            oFrameTriggerDividerReg = 1;
            oEncoderDividerReg = oEncoderDividerValue;
        }
        wmLog(eDebug, "oFrameTriggerDividerReg: %d\n", oFrameTriggerDividerReg);
        wmLog(eDebug, "oEncoderDividerReg: %d\n", oEncoderDividerReg);

        if (CameraEncoderBurstFactor() == 1)
        {
            initAreaImageTriggerPhotonfocus(oEncoderDividerReg, oFrameTriggerDividerReg);
        }
        else if ((CameraEncoderBurstFactor() > 1) && (CameraEncoderBurstFactor() <= 100))
        {
            //initLineImageTriggerPhotonfocus(oEncoderDividerReg, oFrameTriggerDividerReg); // Trigger regime for line image mode not yet defined
            initAreaImageTriggerPhotonfocus(oEncoderDividerReg, oFrameTriggerDividerReg);
        }
        else
        {
            wmLogTr(eError, "QnxMsg.Grab.ABCDEF", "Burst factor for image trigger via encoder interface is not in valid range\n");
        }
    }
    else
    {
        double rate = 1.0E9 / interval.triggerDistance();
        wmLog( eDebug, "Camera::burst - triggerDistance: %d\n", interval.triggerDistance() );
        wmLog( eDebug, "Camera::burst - frame-rate %f\n", rate );    

        arv_camera_set_frame_rate( m_arvCamera, rate, &error );
        checkError("SetFrameRate", &error);
        if ( fabs( rate - arv_camera_get_frame_rate( m_arvCamera, nullptr )) > 0.5 )
        {
            wmLog( eError, "Selected frame rate of %d Hz is not available for the camera and its current settings.\n", rate );
            wmLog( eError, "A frame-rate of %d Hz is being used.\n", arv_camera_get_frame_rate( m_arvCamera, nullptr ) );
        }
    }

    arv_camera_start_acquisition (m_arvCamera, &error);
    checkError("StartAcquisition", &error);
    
    m_capturing = true;
    if ((isImageTriggerViaCameraEncoder()) && (interval.state() == workflow::eAutomaticMode))
    {
        if (CameraEncoderBurstFactor() == 1)
        {
            m_frameDelta = std::chrono::nanoseconds{interval.triggerDistance()};
        }
        else if ((CameraEncoderBurstFactor() > 1) && (CameraEncoderBurstFactor() <= 100))
        {
            m_frameDelta = std::chrono::nanoseconds{100000};
        }
    }
    else
    {
        m_frameDelta = std::chrono::nanoseconds{interval.triggerDistance()};
    }
    m_frameTimer->restart();
    
    m_cycle++;

    if ( !isImageTriggerViaCameraEncoder() && get("AcquisitionMode")->value<std::string>() != std::string("SingleFrame"))
    {
        std::thread( watchDog, this ).detach();
    }
}

void Camera::cancel()
{
    std::unique_lock<std::mutex> cameraLock{m_cameraMutex};
    if ( get("Debug")->value<bool>() ) { wmLog( eDebug, "Camera::cancel\n" ); }
    
    if ( m_capturing )
    {
        GError *error = nullptr;
        if ( m_arvCamera != nullptr ) { arv_camera_stop_acquisition (m_arvCamera, &error); }
        checkError("StopAcquisition", &error);
        m_capturing = false;
    }

    m_triggerContext = {};

    std::unique_lock<std::mutex> lock{m_bufferQueueMutex};
    while (!m_bufferQueue.empty())
    {
        if (get("Debug")->value<bool>())
        {
            wmLog(eDebug, "Dropping buffer from queue\n");
        }
        auto *buffer = m_bufferQueue.front();
        m_bufferQueue.pop();
        arv_stream_push_buffer(m_arvStream, buffer);
    }
}

namespace
{
void printStatusLine(ArvBuffer *buffer)
{
    // Get pixel data pointer:
    uint8_t* pDataPointer = (uint8_t*)arv_buffer_get_data(buffer, nullptr);
    int width; int height;
    arv_buffer_get_image_region(buffer, NULL, NULL, &width, &height);

    // Select the last row of the image to get the Status Line
    uint8_t * pStatusLine = &(pDataPointer[width*(height - 1)]);

    // Get the Status Line preamble
    const int32_t preamble = *(int32_t*)pStatusLine;
    // Get the Counter_0 (4 Bytes). By default it is the FrameStart counter.
    const uint32_t counter0 = *(uint32_t*)(pStatusLine + 4);
    // Get the Counter_1 (4 Bytes). By default it is the MissedTrigger counter.
    const uint32_t counter1 = *(uint32_t*)(pStatusLine + 8);
    // Get the Counter_2 (4 Bytes). By default it is the TimestampTick counter.
    const uint32_t counter2 = *(uint32_t*)(pStatusLine + 12);
    // Get the Counter_3 (4 Bytes). By default it is disabled.
    const uint32_t counter3 = *(uint32_t*)(pStatusLine + 16);

    const uint32_t timer0 = *(uint32_t*)(pStatusLine + 20);
    const uint32_t timer1 = *(uint32_t*)(pStatusLine + 24);
    const uint32_t timer2 = *(uint32_t*)(pStatusLine + 28);
    const uint32_t timer3 = *(uint32_t*)(pStatusLine + 32);

    // Get the Integration Time in units of clock cycles (24 bit)
    const uint32_t integrationTime = (*(uint32_t*)(pStatusLine + 36)) & 0xFFFFFF;
    // Get Timer and Integration Time Clock Frequency (32 bit)
    const uint32_t clockFreq = *(uint32_t*)(pStatusLine + 44);
    // Get Image Average Value (12 bit)
    const uint32_t imageAverageValue = (*(uint32_t*)(pStatusLine + 48)) & 0xFFF;

    // Get image offsetX (12 bit)
    const uint16_t sLOffsetX = (*(uint32_t*)(pStatusLine + 52)) & 0xFFF;
    // Get image width (12 bit)
    const uint16_t sLWidth = (*(uint32_t*)(pStatusLine + 56)) & 0xFFF;
    // Get image offsetY (12 bit)
    const uint16_t sLOffsetY = (*(uint32_t*)(pStatusLine + 60)) & 0xFFF;
    // Get image height (12 bit)
    const uint16_t sLHeight = (*(uint32_t*)(pStatusLine + 64)) & 0xFFF;

    // Get Digital Gain (2 bit)
    const uint16_t digitalGain = (*(uint16_t*)(pStatusLine + 68)) & 0x3;
    // Get Digital Offset (12 bit)
    const int16_t digitalOffset = (*(int16_t*)(pStatusLine + 72)) & 0xFFFF;

    // Get Fine Gain (16 bit). FineGain. This is fixed a point value in the	format: 4 digits integer value, 12 digits fractional value.
    float fineGain = ((*(uint16_t*)(pStatusLine + 76)) & 0xFFFF);
    fineGain /= 4096;

    // Get Camera Type Code (16 bit)
    const uint16_t typeCode = (*(uint16_t*)(pStatusLine + 84)) & 0xFFFF;
    // Get Camera Serial Number (32 bit)
    const uint32_t serialNumber = *(uint32_t*)(pStatusLine + 88);
    // Get Custom value 0 (32 bit)
    const uint32_t customValue0 = *(uint32_t*)(pStatusLine + 92);
    // Get Custom value 1 (32 bit)
    const uint32_t customValue1 = *(uint32_t*)(pStatusLine + 96);

    wmLog(eDebug, "%d | %d | %d | %d | %d\n", preamble, counter0, counter1, counter2, counter3);
    wmLog(eDebug, "Timer0 = %d  Timer1 = %d  Timer2 = %d  Timer3 = %d\n", timer0, timer1, timer2, timer3);
    wmLog(eDebug, "OffsetX = %d  Width = %d  OffsetY = %d  Height = %d\n", sLOffsetX, sLWidth, sLOffsetY, sLHeight);
    wmLog(eDebug, "ClockFreq = %d IntegrationTime = %d (cycles) %f (us) ImageAverageValue = %d \n", clockFreq, integrationTime, ((float)integrationTime/clockFreq)*1000000, imageAverageValue);
    wmLog(eDebug, "DigitalGain = %d DigitalOffset = %u FineGain = %f\n", digitalGain, digitalOffset, fineGain);
    wmLog(eDebug, "TypeCode = %d SerialNumber = %d CustomValue0 = %d CustomValue1 = %d\n", typeCode, serialNumber, customValue0, customValue1);
}
}

void Camera::queueImage( ArvBuffer* p_buffer )
{
    std::unique_lock<std::mutex> lock{m_bufferQueueMutex};
    m_bufferQueue.push(p_buffer);
    lock.unlock();
    m_condition.notify_all();
}

void Camera::handleImageThread()
{
    prctl(PR_SET_NAME, "handle");
    adjustPriority(uint32_t(system::Priority::Sensors) - 1);
    std::unique_lock<std::mutex> lock{m_bufferQueueMutex};
    while (!m_shuttingDown)
    {
        if (!m_bufferQueue.empty())
        {
            auto *buffer = m_bufferQueue.front();
            m_bufferQueue.pop();
            lock.unlock();

            newImage(buffer);
            arv_stream_push_buffer(m_arvStream, buffer);

            lock.lock();
            continue;
        }

        m_condition.wait(lock);
    }
}
void Camera::newImage( ArvBuffer* p_buffer)
{
    if ( get("Debug")->value<bool>() ) { wmLog( eDebug, "Camera::newImage (%d)\n", m_appData.buffer_count ); }
    
    size_t bufferSize;
    unsigned char* bufferData = (unsigned char*)arv_buffer_get_data(p_buffer, &bufferSize);
    
    int width; int height;
   	arv_buffer_get_image_region(p_buffer, NULL, NULL, &width, &height);
    
    if ( width != get( "Window.W" )->value<int>() || height != get( "Window.H" )->value<int>() )
    {
        wmLog( eError, "Received image does not have the expected size (%dx%d) != (%dx%d)\n", width, height, get( "Window.W" )->value<int>(), get( "Window.H" )->value<int>() );
    }

    // todo - check that we really have only 8bits per pixel, and not 10 or 12 ...
    // int bitDepth = ARV_PIXEL_FORMAT_BIT_PER_PIXEL(arv_buffer_get_image_pixel_format(p_buffer));
    
   	m_triggerContext.setImageNumber(m_appData.buffer_count-1);

    if ( !isImageTriggerViaCameraEncoder() )
    {
        if (m_triggerContext.imageNumber() > 1 && m_frameTimer->elapsed() < (m_frameDelta / 2))
        {
            wmLog(eError, "Skipping image: delta of %d smaller than %d\n",
                std::chrono::duration_cast<std::chrono::milliseconds>(m_frameTimer->elapsed()).count(),
                std::chrono::duration_cast<std::chrono::milliseconds>(m_frameDelta).count());
            if (get("StatusLine")->value<bool>())
            {
                printStatusLine(p_buffer);
            }
            return;
        }
    }
    m_frameTimer->restart();

    auto sharedMem = m_sharedMemory.fromExistingPointer(bufferData);
    image::Size2d size{width, height};
    image::TLineImage<byte> image{sharedMem, size};

    updateImageInSharedMemory(sharedMem, image, bufferSize);

    sendImage(image, size);
}

void Camera::updateImageInSharedMemory(const system::ShMemPtr<byte>& sharedMem, image::TLineImage<byte>& image, std::size_t bufferSize)
{
    std::unique_lock<std::mutex> lock{m_sharedMemoryMutex};

    uint32_t imageId = 0;
    if (!m_imagesInSharedMemory.empty())
    {
        imageId = std::get<0>(m_imagesInSharedMemory.back()) + 1;
    }
    image.setImageId(imageId);

    // remove old images from cache which would be overwriten by new image
    while (!m_imagesInSharedMemory.empty())
    {
        const auto &ptr = std::get<1>(m_imagesInSharedMemory.front());
        if ((ptr.offset() + int(std::get<2>(m_imagesInSharedMemory.front())) > sharedMem.offset()) &&
            (ptr.offset() < sharedMem.offset() + int(bufferSize)))
        {
            m_imagesInSharedMemory.pop_front();
            continue;
        }
        break;
    }
    m_imagesInSharedMemory.emplace_back(imageId, sharedMem, bufferSize, image.size());
}

void Camera::sendImage(image::TLineImage<byte> &image, const image::Size2d &size)
{
    bool debug = get("Debug")->value<bool>();
    const auto * pStart = image.begin();
    const auto * pEnd = image.end();
    if (m_imageFillMode != image::ImageFillMode::Direct)
    {
        assert(image.isContiguos()); //otherwise image.data() does not work
        m_newImageTmpCopy.resize(image.size());
        //apply mirroring
        switch (m_imageFillMode)
        {
            case image::ImageFillMode::Direct:
                //should not be here, nothing to do
                __builtin_unreachable();
                break;
            case image::ImageFillMode::FlippedHorizontal:
                image::fillBImage<image::ImageFillMode::FlippedHorizontal>(m_newImageTmpCopy, image.data());
                break;
            case image::ImageFillMode::FlippedVertical:
                image::fillBImage<image::ImageFillMode::FlippedVertical>(m_newImageTmpCopy, image.data());
                break;
            case image::ImageFillMode::Reverse:
                image::fillBImage<image::ImageFillMode::Reverse>(m_newImageTmpCopy, image.data());
                break;
        }
        m_newImageTmpCopy.copyPixelsTo(image);
    }

    if (debug)
    {
        if (image.size() != size)
        {
            wmLog(eWarning, "Inconsistency in image size after copy\n");
        }
        if (image.begin() != pStart || image.end() != pEnd)
        {
            wmLog(eWarning, "Inconsistency in image address after copy\n");
        }
    }

    m_sensorProxy.data(CamGreyImage, m_triggerContext, image );
}



void Camera::buildPropCatalog(const char *feature, gboolean show_description, int level)
{
	ArvGcNode *node = arv_gc_get_node (m_arvGenicam, feature);
    
	if (ARV_IS_GC_FEATURE_NODE (node) && arv_gc_feature_node_is_implemented (ARV_GC_FEATURE_NODE (node), nullptr)) 
    {
        if (ARV_IS_GC_CATEGORY (node)) 
        {
			const GSList *features;
			const GSList *iter;

            wmLog( eDebug, "Category %s\n", feature );

            wmLog( eDebug, "Adding category %s to the list of properties.\n", feature );

            features = arv_gc_category_get_features (ARV_GC_CATEGORY(node));

            for (iter = features; iter != NULL; iter = iter->next)
            {
                buildPropCatalog((const char*)iter->data, show_description, level + 1);
            }
		} 
		else if ( ARV_IS_GC_NODE(node) )
        {
            auto featureNode = ARV_GC_FEATURE_NODE(node);
            const auto visibility = arv_gc_feature_node_get_visibility(featureNode);
            if (visibility != ARV_GC_VISIBILITY_BEGINNER)
            {
                // only list beginner nodes
                return;
            }
            if (arv_gc_feature_node_get_actual_access_mode(featureNode) == ARV_GC_ACCESS_MODE_WO)
            {
                return;
            }

            auto type = arv_dom_node_get_node_name(ARV_DOM_NODE(node));

            bool readOnly = false;
            if (arv_gc_feature_node_is_locked(featureNode, nullptr))
            {
                readOnly = true;
            }
            if (!arv_gc_feature_node_is_available(featureNode, nullptr))
            {
                readOnly = true;
            }
            if (!arv_gc_feature_node_is_implemented(featureNode, nullptr))
            {
                readOnly = true;
            }
            if (arv_gc_feature_node_get_actual_access_mode(featureNode) == ARV_GC_ACCESS_MODE_RO)
            {
                readOnly = true;
            }

            if ( g_strcmp0 (type, "Integer") == 0) 
            {                
                auto value = arv_device_get_integer_feature_value(m_arvDevice, feature, nullptr);
                gint64 min = -1000000;
                gint64 max = 1000000;
                arv_device_get_integer_feature_bounds(m_arvDevice, feature, &min, &max, nullptr);
                interface::TKeyValue<int> keyvalue( 
                                                    feature,    // key
                                                    value,		// value    
                                                    int(min),	// min
                                                    int(max),	// max
                                                    value		// default
                                                  );
                keyvalue.setReadOnly(readOnly);
                m_parameters.addParameter( keyvalue );
            }
            if ( g_strcmp0 (type, "Float") == 0) 
            {
                auto value = arv_device_get_float_feature_value(m_arvDevice, feature, nullptr);
                double min = -1.0E10f;
                double max = 1.0E10f;
                arv_device_get_float_feature_bounds(m_arvDevice, feature, &min, &max, nullptr);
                interface::TKeyValue<float> keyvalue( 
                                                    feature,    // key
                                                    value,		// value    
                                                    float(min),	// min
                                                    float(max),    // max
                                                    value		// default
                                                );
                keyvalue.setReadOnly(readOnly);
                m_parameters.addParameter( keyvalue );
            }
            if ( g_strcmp0 (type, "Boolean") == 0) 
            {
                auto value = arv_device_get_boolean_feature_value(m_arvDevice, feature, nullptr);
                interface::TKeyValue<bool> keyvalue( 
                                                    feature,    // key
                                                    value,		// value    
                                                    false,	    // min	
                                                    true,	    // max	
                                                    value		// default
                                                  );
                keyvalue.setReadOnly(readOnly);
                m_parameters.addParameter( keyvalue );
            }
            if (ARV_IS_GC_ENUMERATION (node))
            {
                auto value = arv_device_get_string_feature_value(m_arvDevice, feature, nullptr);

                std::string stringValue;
                if (value)
                {
                    stringValue = value;
                }
                interface::TKeyValue<std::string> keyvalue{
                                                    feature,    // key
                                                    stringValue,		// value
                                                    std::string{},	    // min
                                                    std::string{},	    // max
                                                    stringValue		// default
                                                    };
                keyvalue.setReadOnly(readOnly);
                m_parameters.addParameter( keyvalue );
            }
        }
	}
}



void Camera::parameterize() 
{
    GError *error = nullptr;
    std::vector<std::string> allowedList{
        std::string{"StatusLine"},
        std::string{"Gain"},
        std::string{"Debug"},
        std::string{"ExposureTime"},
        std::string{"Window.X"},
        std::string{"Window.Y"},
        std::string{"Window.W"},
        std::string{"Window.H"}

    };
    
    auto configFilePath = configFile();
    Poco::File oConfigFile(configFile());
    if (!oConfigFile.exists())
    {
        // check whether legacy camera.xml exists
        Poco::Path legacyPath = m_configDir;
        legacyPath.setFileName(m_legacyConfigFile);
        Poco::File legacyFile{legacyPath};
        // and rename to new file name
        if (legacyFile.exists())
        {
            legacyFile.moveTo(configFilePath.toString());
        }
    }
	if (oConfigFile.exists() == true) 
    {
		Poco::AutoPtr<Poco::Util::XMLConfiguration> pConfIn;
		try { 
            pConfIn = new Poco::Util::XMLConfiguration(configFilePath.toString());
		} 
		catch(const Poco::Exception &p_rException) 
        {
			wmLog(eDebug, "%s - '%s': %s\n", __FUNCTION__, p_rException.name(), p_rException.message().c_str());
		} 
		
		//for(auto it(std::begin(m_parameters.m_paramConfig)); it != std::end(m_parameters.m_paramConfig); ++it) 
		for(auto it : m_parameters.m_paramConfig )
        {
            if (it->isReadOnly())
            {
                continue;
            }
            if (!pConfIn->hasProperty(it->key()))
            {
                continue;
            }
			//std::cout << __FUNCTION__ << "reading key:\n" << it->second->key() << "\n"; // debug
            if (std::any_of(allowedList.begin(), allowedList.end(), [it] (const auto &key) { return key == it->key(); }))
            {
                auto start = std::chrono::system_clock::now();
                
                const Types oType	( it->type() );
                const auto special = isSpecialProperty(it->key());
                try 
                {
                    switch (oType) 
                    {
                    case TBool:
                    {
                        it->setValue( pConfIn->getBool( it->key(), it->defValue<bool>() ) ); // assign value
                        if (!special)
                        {
                            arv_device_set_boolean_feature_value(m_arvDevice, it->key().c_str(), it->value<bool>(), &error );
                            checkError("Parameterize " + it->key(), &error);
                        }
                        break;
                    }
                    case TInt:
                    {
                        it->setValue( pConfIn->getInt(it->key(), it->defValue<int>()) ); // assign value
                        if (!special)
                        {
                            arv_device_set_integer_feature_value(m_arvDevice, it->key().c_str(), it->value<int>(), &error );
                            checkError("Parameterize " + it->key(), &error);
                        }
                        break;
                    }
                    case TFloat:
                    {
                        it->setValue( (float)(pConfIn->getDouble( it->key(), it->defValue<float>() ) ) ); // assign value
                        if (!special)
                        {
                            arv_device_set_float_feature_value(m_arvDevice, it->key().c_str(), it->value<float>(), &error );
                            checkError("Parameterize " + it->key(), &error);
                        }
                        break;
                    }
                    case TString:
                    {
                        it->setValue(pConfIn->getString(it->key(), it->value<std::string>()));
                        if (!special)
                        {
                            arv_device_set_string_feature_value(m_arvDevice, it->key().c_str(), it->value<std::string>().c_str(), &error);
                            checkError("Parameterize " + it->key(), &error);
                        }
                        break;
                    }
                    default:
                        std::ostringstream oMsg;
                        oMsg << "Invalid value type: '" << oType << "'\n";
                        wmLogTr(eWarning, "QnxMsg.Vdr.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str());
                        break;
                    }
                }
                catch(const Poco::Exception &p_rException) 
                {
                    wmLog(eDebug, "%s - '%s': %s\n", __FUNCTION__, p_rException.name(), p_rException.message().c_str());
                }
                
                if (special && it->key().rfind("Window.", 0) == std::string::npos)
                {
                    updateSpecialProperties( it->key() );
                }
                
                auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now()-start).count(); 
                if ( elapsed > 5000 )
                {
                    wmLog(eDebug,"It took %dus to restore %s\n", elapsed, it->key().c_str() );
                }
            }
		}
	}
	else 
    {
		std::ostringstream oMsg;
		oMsg << "Configuration file '" << m_configDir.toString() << "' not found.\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
	}

	updateSpecialProperties( "Window.X" );
}


bool Camera::isSpecialProperty(const std::string &key)
{
    static const std::vector<std::string> s_specialProperties{
        {"Debug"},
        {"StatusLine"},
        {"Window.X"},
        {"Window.Y"},
        {"Window.W"},
        {"Window.H"},
        {"ExposureTime"},
        {"ReuseLastImage"},
    };
    return std::find(s_specialProperties.begin(), s_specialProperties.end(), key) != s_specialProperties.end();
}

// some properties need a special treatment, e.g. the hw-roi, for which we also need to allocate buffers ...
void Camera::updateSpecialProperties( std::string p_key )
{
    GError *error = nullptr;
    if ( p_key == std::string("Window.X") || p_key == std::string("Window.Y") || p_key == std::string("Window.W") || p_key == std::string("Window.H") )
    {
        int x,y,w,h;
        x = get( "Window.X" )->value<int>(); 
        y = get( "Window.Y" )->value<int>();
        w = get( "Window.W" )->value<int>(); 
        h = get( "Window.H" )->value<int>();
        arv_camera_set_region( m_arvCamera, x, y, w, h, &error );
        checkError("SetRegion", &error);
    }
    if ( p_key == std::string("ExposureTime") )
    {
        float exposure = get( "ExposureTime" )->value<float>() * 1000.0f;
        arv_camera_set_exposure_time (m_arvCamera, exposure, &error);
        checkError("SetExposureTime", &error);
    }
}

bool Camera::isImageInSharedMemory(uint32_t imageId)
{
    std::unique_lock<std::mutex> lock{m_sharedMemoryMutex};
    if (m_imagesInSharedMemory.empty())
    {
        return false;
    }
    if (std::get<0>(m_imagesInSharedMemory.front()) > imageId)
    {
        return false;
    }
    static const std::size_t s_safetyMargin{10};
    if (m_imagesInSharedMemory.size() > m_maxNumberImagesInSharedMemory - s_safetyMargin)
    {
        const auto difference = std::min(m_imagesInSharedMemory.size() - (m_maxNumberImagesInSharedMemory -s_safetyMargin), s_safetyMargin);
        // safety margin of 10 images
        if (std::get<0>(m_imagesInSharedMemory.front()) + difference > imageId)
        {
            return false;
        }
    }
    return std::get<0>(m_imagesInSharedMemory.back()) >= imageId;
}

bool Camera::checkError(const std::string &label, GError **error)
{
    if (!error || !(*error))
    {
        return false;
    }
    wmLog(eError, "%s failed with %s\n", label, (*error)->message);
    g_clear_error(error);
    return true;
}

void Camera::downloadGenicamXml()
{
    std::size_t size;
    const auto *data = arv_device_get_genicam_xml(m_arvDevice, &size);
    std::string data_string{data, size};
    std::ofstream stream{m_wmBaseDir.toString() + std::string{"/data/cameraGigEVision.xml"}, std::ios_base::out | std::ios_base::trunc};
    stream << data_string;
}

void Camera::detectVendor()
{
    GError *error = nullptr;
    const char *vendor = arv_camera_get_vendor_name(m_arvCamera, &error);
    wmLog( eInfo, "Camera vendor name: %s\n", vendor);
    checkError("GetVendorName", &error);

    if (strcmp(vendor, "Photonfocus AG") == 0)
    {
        m_vendor = Vendor::Photonfocus;
    }
    else if (strcmp(vendor, "Basler") == 0)
    {
        m_vendor = Vendor::Basler;
    }
    else if (strcmp(vendor, "Baumer") == 0)
    {
        m_vendor = Vendor::Baumer;
    }
}

Poco::Path Camera::configFile()
{
    Poco::Path path = m_configDir;
    if (!m_arvCamera)
    {
        path.setFileName(m_legacyConfigFile);
    }
    else
    {
        if (m_configFileName.empty())
        {
            m_configFileName = initConfigFileName();
        }
        path.setFileName(m_configFileName);
    }
    return path;
}

std::string Camera::initConfigFileName() const
{
    std::string fileName{"camera-"};
    switch (m_vendor)
    {
    case Vendor::Photonfocus:
        fileName.append(std::string{"PF-"});
        break;
    case Vendor::Baumer:
        fileName.append(std::string{"Baumer-"});
        break;
    case Vendor::Basler:
        fileName.append(std::string{"Basler-"});
        break;
    case Vendor::Unknown:
        fileName.append(std::string{"Unknown-"});
        break;
    }
    fileName.append(arv_camera_get_model_name(m_arvCamera, nullptr));
    fileName.append(std::string{".xml"});
    return fileName;
}

void Camera::queryPowerDown()
{
    GError *error = nullptr;
    if (arv_device_is_feature_available(m_arvDevice, "PowerDown_Status", &error))
    {
        const auto powerDown = arv_device_get_string_feature_value(m_arvDevice, "PowerDown_Status", &error);
        checkError("PowerDown_Status", &error);
        wmLog(eInfo, "Power down status of camera: %s\n", powerDown);
    }
    if (arv_device_is_feature_available(m_arvDevice, "PowerDown_CountSinceStartup", &error))
    {
        int countSinceStartup = arv_device_get_integer_feature_value(m_arvDevice, "PowerDown_CountSinceStartup", &error);
        checkError("PowerDown_CountSinceStartup", &error);
        wmLog(eInfo, "%d PowerDown events since startup\n", countSinceStartup);
    }
    if (arv_device_is_feature_available(m_arvDevice, "PowerDown_CountLog", &error))
    {
        int countLifeTime = arv_device_get_integer_feature_value(m_arvDevice, "PowerDown_CountLog", &error);
        checkError("PowerDown_CountLog", &error);
        wmLog(eInfo, "%d PowerDown events on this device\n", countLifeTime);
    }
}

} // namespache grabber
} // namespace precitec
