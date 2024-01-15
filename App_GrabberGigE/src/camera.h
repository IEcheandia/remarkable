#pragma once

// wm includes
#include "message/device.h"
#include "common/triggerInterval.h"
#include "common/triggerContext.h"
#include "event/sensor.proxy.h"
#include "grabber/sharedMemoryImageProvider.h"
// poco includes
#include "Poco/File.h"
// project includes
#include "parameter.h"
// aravis includes
#include <arv.h>
#include <memory>
#include <thread>
#include <condition_variable>

class TestSharedMemoryRing;


namespace precitec {
    
    using namespace interface;

namespace system
{
    class ElapsedTimer;
}

namespace grabber  {

    class Camera;
    
typedef struct 
{
	GMainLoop*         main_loop;
	int                buffer_count;
    int                buffer_max;
	ArvChunkParser*    chunk_parser;
	char**             chunks;
    Camera*            camera;
} ApplicationData;
    
    
class Camera
{
public:

    Camera(TSensor<EventProxy>& p_sensorProxy);
    virtual ~Camera();

    void initializeWork(void);

    interface::KeyHandle set(interface::SmpKeyValue p_smpKeyValue);
    
   	interface::SmpKeyValue get(interface::Key p_key) const;

   	interface::Configuration getConfig() const;

    void initialize();

    void uninitialize();
    
    void queueImage( ArvBuffer* p_buffer );
    void newImage( ArvBuffer* p_buffer);
    
    void burst(TriggerContext const& context, interface::TriggerInterval const& interval);
    
    void cancel();
    
    void buildPropCatalog(const char *feature, gboolean show_description, int level);
    
    void parameterize();
    
    void updateSpecialProperties( std::string p_key );
    
    bool isCapturing() const { return m_capturing; };
    
    int currentImageNumber() const { return m_triggerContext.imageNumber(); };
    
    int cycleNumber() const { return m_cycle; };

    bool isImageInSharedMemory(uint32_t imageId);

    Poco::Path configFile();

    /**
     * Whether the system configuration specifies that GigE camera should be used.
     **/
    bool isActiveForConfiguration() const
    {
        return m_activeForConfiguration;
    }

    
protected:
    
    
    ApplicationData         m_appData;
    Parameter               m_parameters;
	ArvCamera*              m_arvCamera = nullptr;
    ArvStream*              m_arvStream = nullptr;
    ArvDevice*              m_arvDevice = nullptr;
    ArvGc*                  m_arvGenicam = nullptr;
    TSensor<EventProxy>&    m_sensorProxy;
	TriggerContext 	        m_triggerContext;  
    
    const Poco::Path		m_wmBaseDir;
    Poco::Path              m_configDir;
    
    bool                    m_capturing;
    int                     m_cycle;
    std::unique_ptr<system::ElapsedTimer> m_frameTimer;
    std::chrono::nanoseconds m_frameDelta;

private:
    bool checkError(const std::string &label, GError **error);
    void handleImageThread();
    void adjustPriority(uint32_t priority);
    void downloadGenicamXml();
    void detectVendor();
    void initializeAcquisition();
    void initializeAcquisitionPhotonfocus();
    void initLedTrigger();
    void initLedTriggerPhotonfocus();
    void initLedTriggerBasler();
    void initLedTriggerBaumer();
    void initEncoderInterface();
    void initEncoderInterfacePhotonfocus();
    void initAreaImageTriggerPhotonfocus(int p_oEncoderDivider, int p_oTriggerDivider);
    void initLineImageTriggerPhotonfocus(int p_oEncoderDivider, int p_oTriggerDivider);
    bool isSpecialProperty(const std::string &key);
    std::string initConfigFileName() const;
    void queryPowerDown();
    void sendImage(image::TLineImage<byte> &image, const image::Size2d &size);
    void updateImageInSharedMemory(const system::ShMemPtr<byte>& sharedMem, image::TLineImage<byte>& image, std::size_t bufferSize);

    bool isImageTriggerViaCameraEncoder(void) { return m_oImageTriggerViaCameraEncoder; }
    int CameraEncoderBurstFactor(void) { return m_oCameraEncoderBurstFactor; }

    enum class Vendor
    {
        Unknown,
        Photonfocus,
        Basler,
        Baumer
    };
    Vendor m_vendor = Vendor::Unknown;

    SharedMemoryImageProvider m_sharedMemory;
    std::deque<std::tuple<uint32_t, system::ShMemPtr<byte>, std::size_t, image::Size2d>> m_imagesInSharedMemory;
    std::mutex m_cameraMutex;
    std::mutex m_sharedMemoryMutex;
    std::queue<ArvBuffer*> m_bufferQueue;
    image::ImageFillMode m_imageFillMode;
    image::BImage m_newImageTmpCopy;
    std::mutex m_bufferQueueMutex;
    std::thread m_handleImageThread;
    std::condition_variable m_condition;
    bool m_shuttingDown = false;
    unsigned int m_payload = 0;
    const std::string m_legacyConfigFile{"camera.xml"};
    std::string m_configFileName;

    bool m_oImageTriggerViaCameraEncoder;
    int m_oCameraEncoderBurstFactor;
    bool m_activeForConfiguration{false};
    std::size_t m_maxNumberImagesInSharedMemory{0};

    friend TestSharedMemoryRing;
    /**
     * Constructor for autotest which does not setup aravis at all
     **/
    Camera();
};


} // namespace grabber
} // namespace precitec
