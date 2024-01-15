/**
*
* @defgroup Framegrabber Framegrabber
*
* \section sec Image Loading
*
*
* @file
* @brief  Workerclass to load previously recorded images and/or Samples corresponding to product, seamseries and seam
* @copyright    Precitec GmbH & Co. KG
* @author GG
* @date   17.05.17
*
*
*/

#pragma once

#include <vector>
#include "Poco/RWLock.h"
#include "Poco/Event.h"
#include "Poco/RunnableAdapter.h"
#include "Poco/Thread.h"
#include "Poco/ListMap.h"
#include "Poco/HashMap.h"
#include "Poco/LRUCache.h"
#include "Poco/UUID.h"
#include "common/triggerContext.h"
#include "vdrFileInfo.h"
#include "imageDataHolder.h"
#include "sequenceInformation.h"
#include "common/sample.h"

namespace precitec
{
using namespace interface;
using Poco::ListMap;
using Poco::HashMap;

namespace grabber
{
class SharedMemoryImageProvider;
}

class SequenceLoader
{
public:

    SequenceLoader();
    ~SequenceLoader();

    void init(grabber::SharedMemoryImageProvider *memory);
    void close(void);
    void setBasepath(std::string const & _path);
    void setTestProductInstance(const Poco::UUID &productInstance);
    bool getImage( const Poco::UUID &productInstanceId, uint32_t _productnumber, uint32_t _seamseriesnumber, uint32_t _seamnumber, uint32_t _imagenumber, ImageDataHolder & _imageDataHolder );
    bool getImage( const Poco::UUID &productInstanceId, uint32_t _seamseriesnumber, uint32_t _seamnumber, uint32_t _imagenumber, ImageDataHolder & _imageDataHolder );
    bool getImage( uint32_t _productnumber, uint32_t _seamseriesnumber, uint32_t _seamnumber, uint32_t _imagenumber, ImageDataHolder & _imageDataHolder );
    bool getSamples( const Poco::UUID &productInstanceId, uint32_t _productnumber, uint32_t _seamseriesnumber, uint32_t _seamnumber, uint32_t _imagenumber, fileio::SampleDataHolder & _sampleDataHolder );
    bool getSamples( const Poco::UUID &productInstanceId, uint32_t _seamseriesnumber, uint32_t _seamnumber, uint32_t _imagenumber, fileio::SampleDataHolder & _sampleDataHolder );
    bool getSamples( uint32_t _productnumber, uint32_t _seamseriesnumber, uint32_t _seamnumber, uint32_t _imagenumber, fileio::SampleDataHolder & _sampleDataHolder );
    void reload(uint32_t _productNumber);
    void setTestImagesProductInstanceMode(bool set);

private:

#if !defined(__QNX__)
    using SequenceLoader_Adapter_t=Poco::RunnableAdapter<SequenceLoader>;
	using VdrFolderMap_t = HashMap<std::string, VdrFileInfo>;
    using ImageCache_t=Poco::LRUCache<std::string, ImageDataHolder>;
    using SampleCache_t=Poco::LRUCache<std::string, fileio::SampleDataHolder>;
#else
    typedef Poco::RunnableAdapter<SequenceLoader> SequenceLoader_Adapter_t;
    typedef ListMap<std::string, VdrFileInfo> VdrFolderMap_t;
    typedef Poco::LRUCache<std::string, ImageDataHolder> ImageCache_t;
    typedef Poco::LRUCache<std::string, fileio::SampleDataHolder> SampleCache_t;
#endif

#if !defined(__QNX__)
    const uint32_t MaxCacheSize=128;
#else
    const static uint32_t MaxCacheSize=128;
#endif

    void startWorkerThread();
    void work();
    void doCache( VdrFileType _vdrFileType );
    void addCache( VdrFileInfo & _imageInfo, VdrFileType _vdrFileType );
    void updateCachingStrategy(std::string const & vdrFileKey, VdrFileType _vdrFileType );
    void updateCachingStrategy(std::string const & vdrFileKey, VdrFolderMap_t const & _vdrFolderMap, std::deque<VdrFileInfo> const & _vdrVector, uint32_t & _lastFolderIndex);
    void updateCachingStrategy(uint32_t _productNumber);
    void updateCachingStrategy(uint32_t _productNumber, std::deque<VdrFileInfo> const & _vdrVector, uint32_t & _lastFolderIndex);
    bool getImage( const std::string &sampleKey, ImageDataHolder & _imageDataHolder );
    bool getSamples( const std::string &sampleKey, fileio::SampleDataHolder & _sampleDataHolder );
    /**
     * Loads the image referenced by @p imageDataHolder and fills the additional data and creates the bimage.
     **/
    void loadImage(ImageDataHolder &imageDataHolder);
    bool m_IsInitialized;
    bool m_Shutdown;
    bool m_DoRefreshFolders;
    uint32_t m_LastImageFolderIndex;
    uint32_t m_LastSampleFolderIndex;
    Poco::Thread m_Workerthread;
    SequenceLoader_Adapter_t m_ThreadAdapter;
    mutable Poco::RWLock m_ImageQueueMutex;
    mutable Poco::RWLock m_SampleQueueMutex;
    mutable Poco::Event m_WorkerEventStart;
    mutable Poco::Event m_WorkerEventReady;
    ImageCache_t m_ImageCache;
    SampleCache_t m_SampleCache;
    grabber::SharedMemoryImageProvider *m_memory;
    SequenceInformation m_SequenceInformation;
};
} // namespace precitec
