/**
*
* @defgroup Framegrabber Framegrabber
*
* \section sec VDR File Loading
*
*
* @file
* @brief  Workerclass to load previously recorded images and/or samples corresponding to product, seamseries and seam
* @copyright    Precitec GmbH & Co. KG
* @author GG
* @date   17.05.17
*
*
*/

#include <string>
#include <sstream>
#include <cstdlib>
#undef min
#undef max
#include <limits>
#include <iostream>
#include <vector>
#include <Poco/Delegate.h>
#include <Poco/Environment.h>
#include <Poco/Path.h>
#include <Poco/File.h>
#include <Poco/DirectoryIterator.h>
#include <Poco/SortedDirectoryIterator.h>
#include <Poco/Exception.h>
#include <Poco/RegularExpression.h>
#include <Poco/String.h>
#if !defined(__QNX__)
#include <Poco/NumericString.h>
#endif
#include <Poco/RWLock.h>
#include <common/connectionConfiguration.h>
#include <common/bitmap.h>
#include <system/types.h>
#include "module/moduleLogger.h"
#include "trigger/sequenceLoader.h"
#include "../Mod_VideoRecorder/include/videoRecorder/types.h"
#include "grabber/sharedMemoryImageProvider.h"

namespace precitec
{
SequenceLoader::SequenceLoader():
    m_IsInitialized(false),
    m_Shutdown(false),
    m_DoRefreshFolders(false),
    m_LastImageFolderIndex(0),
    m_LastSampleFolderIndex(0),
    m_Workerthread("VdrFileLoader"),
    m_ThreadAdapter(*this, &SequenceLoader::work),
    m_ImageCache(MaxCacheSize),
	m_SampleCache(MaxCacheSize),
    m_memory(nullptr)
{
}

SequenceLoader::~SequenceLoader()
{
}

void SequenceLoader::init(grabber::SharedMemoryImageProvider *memory)
{
    if( ! m_IsInitialized )
    {
    	m_DoRefreshFolders=true;
        m_memory = memory;
        startWorkerThread();
        m_IsInitialized = true;
    }
}

void SequenceLoader::close(void)
{
    if( m_IsInitialized )
    {
        if (m_Workerthread.isRunning())
        {
            m_Shutdown=true;
            m_WorkerEventStart.set();
            wmLog(eDebug, "%s: Shutdown as soon as possible...\n", __FUNCTION__);
            const bool joined( m_Workerthread.tryJoin(10000) );
            if (! joined)
            {
                std::ostringstream oMsg;
                oMsg << __FUNCTION__ << "\t: Failed to join thread '" << m_Workerthread.name() << "'.\n";
                wmLog(eWarning, oMsg.str());
            }
        }

        m_LastImageFolderIndex=0;
        m_SequenceInformation.ImageFolderMap().clear();
        m_ImageCache.clear();
        m_SequenceInformation.ImageFolderVector().clear();
        m_LastSampleFolderIndex=0;
        m_SequenceInformation.SampleFolderMap().clear();
        m_SampleCache.clear();
        m_SequenceInformation.SampleFolderVector().clear();
        m_IsInitialized = false;
    }
}

void SequenceLoader::reload(uint32_t _productNumber)
{
	try
	{
		{
			Poco::ScopedWriteRWLock oWriteLock(m_ImageQueueMutex);
			m_LastImageFolderIndex=0;
			m_SequenceInformation.ImageFolderMap().clear();
			m_ImageCache.clear();
			m_SequenceInformation.ImageFolderVector().clear();
		}
		{
			Poco::ScopedWriteRWLock oWriteLock(m_SampleQueueMutex);
			m_LastSampleFolderIndex=0;
			m_SequenceInformation.SampleFolderMap().clear();
			m_SampleCache.clear();
			m_SequenceInformation.SampleFolderVector().clear();
		}
		m_SequenceInformation.scanFolders();
		updateCachingStrategy(_productNumber);
        std::ostringstream oMsg;
        oMsg  << __FUNCTION__ << " Productnumber=" << _productNumber << " Last Image Index=" << m_LastImageFolderIndex << "\n";
        wmLog(eWarning, oMsg.str());
		m_WorkerEventReady.reset();
		m_WorkerEventStart.set();
		m_WorkerEventReady.wait();
	}
    catch(Poco::TimeoutException const & _ex)
    {
    	//we do expect this exception
    }
    catch(Poco::Exception const & _ex)
    {
        std::ostringstream oMsg;
        oMsg  << __FUNCTION__ << " " << _ex.what() << " - " << _ex.message() << "\n";
        wmLog(eWarning, oMsg.str());
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
}

void SequenceLoader::setBasepath(std::string const & _path)
{
    m_SequenceInformation.setBasepath(_path);
}

void SequenceLoader::setTestProductInstance(const Poco::UUID &productInstance)
{
    m_SequenceInformation.setInstanceId(productInstance);
}

void SequenceLoader::setTestImagesProductInstanceMode(bool set)
{
    m_SequenceInformation.setOperationMode(set ? SequenceInformation::OperationMode::ProductInstance : SequenceInformation::OperationMode::SerialNumber);
}

bool SequenceLoader::getImage( const Poco::UUID &productInstance, uint32_t _productnumber, uint32_t _seamseriesnumber, uint32_t _seamnumber, uint32_t _imagenumber, ImageDataHolder & _imageDataHolder )
{
    if (productInstance == Poco::UUID{})
    {
        return getImage(_productnumber, _seamseriesnumber, _seamnumber, _imagenumber, _imageDataHolder);
    } else
    {
        return getImage(productInstance, _seamseriesnumber, _seamnumber, _imagenumber, _imageDataHolder);
    }
}

bool SequenceLoader::getImage( const Poco::UUID &productInstance, uint32_t _seamseriesnumber, uint32_t _seamnumber, uint32_t _imagenumber, ImageDataHolder & _imageDataHolder )
{
    return getImage(m_SequenceInformation.makeVdrFileKey(productInstance, _seamseriesnumber, _seamnumber, _imagenumber), _imageDataHolder);
}

bool SequenceLoader::getImage( uint32_t _productnumber, uint32_t _seamseriesnumber, uint32_t _seamnumber, uint32_t _imagenumber, ImageDataHolder & _imageDataHolder )
{
    std::string imageKey;
    m_SequenceInformation.makeVdrFileKey( _productnumber, _seamseriesnumber, _seamnumber, _imagenumber, imageKey );
    return getImage(imageKey, _imageDataHolder);
}

bool SequenceLoader::getImage( const std::string &imageKey, ImageDataHolder & _imageDataHolder )
{
	try
	{
		VdrFolderMap_t::Iterator inProducts = m_SequenceInformation.ImageFolderMap().find(imageKey);
		if( inProducts == m_SequenceInformation.ImageFolderMap().end() )
		{
			std::ostringstream oMsg;
			oMsg  << __FUNCTION__ << " imagekey NOT found " << "  " << imageKey << "\n";
			wmLog(eDebug, oMsg.str());
			return false;
		}
		else
		{
			if( m_ImageCache.has(imageKey) )
			{
				Poco::ScopedWriteRWLock oWriteLock(m_ImageQueueMutex);
				_imageDataHolder = *m_ImageCache.get(imageKey);
                loadImage(_imageDataHolder);
				m_ImageCache.remove(imageKey);
				m_WorkerEventStart.set();
				return true;
			}
			else
			{
				std::ostringstream oMsg;
				oMsg  << __FUNCTION__ << " imagekey Cache Miss " << "  " << imageKey << "\n";
				wmLog(eDebug, oMsg.str());
				updateCachingStrategy(imageKey, ImageVdrFileType);
				{
					Poco::ScopedWriteRWLock oWriteLock(m_ImageQueueMutex);
					m_ImageCache.clear();
				}
				addCache( inProducts->second, ImageVdrFileType );
				if( m_ImageCache.has(imageKey) )
				{
					Poco::ScopedWriteRWLock oWriteLock(m_ImageQueueMutex);
					_imageDataHolder = *m_ImageCache.get(imageKey);
                    loadImage(_imageDataHolder);
					m_ImageCache.remove(imageKey);
					m_WorkerEventStart.set();
					return true;
				}
			}

			return false;
		}
	}
    catch(Poco::Exception const & _ex)
    {
        std::ostringstream oMsg;
        oMsg  << __FUNCTION__ << " " << _ex.what() << " - " << _ex.message() << "\n";
        wmLog(eWarning, oMsg.str());
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

    return false;
}

bool SequenceLoader::getSamples( const Poco::UUID &productInstance, uint32_t _productnumber, uint32_t _seamseriesnumber, uint32_t _seamnumber, uint32_t _imagenumber, fileio::SampleDataHolder & _sampleDataHolder )
{
    if (productInstance == Poco::UUID{})
    {
        return getSamples(_productnumber, _seamseriesnumber, _seamnumber, _imagenumber, _sampleDataHolder);
    } else
    {
        return getSamples(productInstance, _seamseriesnumber, _seamnumber, _imagenumber, _sampleDataHolder);
    }
}

bool SequenceLoader::getSamples( const Poco::UUID &productInstance, uint32_t _seamseriesnumber, uint32_t _seamnumber, uint32_t _imagenumber, fileio::SampleDataHolder & _sampleDataHolder )
{
    return getSamples(m_SequenceInformation.makeVdrFileKey(productInstance, _seamseriesnumber, _seamnumber, _imagenumber), _sampleDataHolder);
}

bool SequenceLoader::getSamples( uint32_t _productnumber, uint32_t _seamseriesnumber, uint32_t _seamnumber, uint32_t _imagenumber, fileio::SampleDataHolder & _sampleDataHolder )
{
    std::string sampleKey;
    m_SequenceInformation.makeVdrFileKey( _productnumber, _seamseriesnumber, _seamnumber, _imagenumber, sampleKey );
    return getSamples(sampleKey, _sampleDataHolder);
}

bool SequenceLoader::getSamples( const std::string &sampleKey, fileio::SampleDataHolder & _sampleDataHolder )
{
	try
	{
		VdrFolderMap_t::Iterator inProducts = m_SequenceInformation.SampleFolderMap().find(sampleKey);
		if( inProducts == m_SequenceInformation.SampleFolderMap().end() )
		{
			std::ostringstream oMsg;
			oMsg  << __FUNCTION__ << " samplekey NOT found " << "  " << sampleKey << "\n";
			wmLog(eDebug, oMsg.str());
			return false;
		}
		else
		{
			if( m_SampleCache.has(sampleKey) )
			{
				Poco::ScopedWriteRWLock oWriteLock(m_SampleQueueMutex);
				_sampleDataHolder = *m_SampleCache.get(sampleKey);
				m_SampleCache.remove(sampleKey);
				m_WorkerEventStart.set();
				return true;
			}
			else
			{
				std::ostringstream oMsg;
				oMsg  << __FUNCTION__ << " samplekey Cache Miss " << "  " << sampleKey << "\n";
				wmLog(eDebug, oMsg.str());
				updateCachingStrategy(sampleKey, SampleVdrFileType);
				{
					Poco::ScopedWriteRWLock oWriteLock(m_SampleQueueMutex);
					m_SampleCache.clear();
				}
				addCache( inProducts->second, SampleVdrFileType );
				if( m_SampleCache.has(sampleKey) )
				{
					Poco::ScopedWriteRWLock oWriteLock(m_SampleQueueMutex);
					_sampleDataHolder = *m_SampleCache.get(sampleKey);
					m_SampleCache.remove(sampleKey);
					m_WorkerEventStart.set();
					return true;
				}
			}

			return false;
		}
	}
    catch(Poco::Exception const & _ex)
    {
        std::ostringstream oMsg;
        oMsg  << __FUNCTION__ << " " << _ex.what() << " - " << _ex.message() << "\n";
        wmLog(eWarning, oMsg.str());
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

    return false;
}


void SequenceLoader::startWorkerThread()
{
    if (! m_Workerthread.isRunning())
    {
        m_Shutdown=false;
        m_WorkerEventStart.set();
        m_Workerthread.start(m_ThreadAdapter);
    }
}

void SequenceLoader::work()
{
    try
    {
        std::ostringstream oMsg;
        while(true)
        {
            m_WorkerEventStart.wait();
            {
                if (m_Shutdown)
                {
                    break;
                }
                else
                {
                	if( m_DoRefreshFolders )
                    {
                		m_SequenceInformation.scanFolders();
                    	m_DoRefreshFolders=false;
                    }

                    if( m_SequenceInformation.ImageFolderMap().size()>0 )
                    {
                        doCache(ImageVdrFileType);
                    }

                    if( m_SequenceInformation.SampleFolderMap().size()>0 )
                    {
                        doCache(SampleVdrFileType);
                    }
                }
            }
            m_WorkerEventReady.set();
        }

        oMsg.str("");
        oMsg << "-- Leaving thread TID " << m_Workerthread.currentTid() << " '" << m_Workerthread.name() << "'. --\n";
        wmLog(eDebug, oMsg.str());
    }
    catch(Poco::Exception const & _ex)
    {
        std::ostringstream oMsg;
        oMsg  << __FUNCTION__ << " " << _ex.what() << " - " << _ex.message() << "\n";
        wmLog(eWarning, oMsg.str());
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
}

void SequenceLoader::doCache(VdrFileType _vdrFileType)
{
	if( _vdrFileType==ImageVdrFileType)
	{
		if( m_ImageCache.size()<MaxCacheSize && m_LastImageFolderIndex < m_SequenceInformation.ImageFolderVector().size() )
		{
			for( ; m_LastImageFolderIndex<m_SequenceInformation.ImageFolderMap().size() && m_ImageCache.size()<MaxCacheSize; m_LastImageFolderIndex++ )
			{
				VdrFileInfo imageInfo = m_SequenceInformation.ImageFolderVector().at(m_LastImageFolderIndex);
				std::string strKey = imageInfo.getKey();
				addCache(imageInfo, _vdrFileType);
			}
		}
	}
	else if( _vdrFileType==SampleVdrFileType)
	{
		if( m_SampleCache.size()<MaxCacheSize && m_LastSampleFolderIndex < m_SequenceInformation.SampleFolderVector().size() )
		{
			for( ; m_LastSampleFolderIndex<m_SequenceInformation.SampleFolderVector().size() && m_SampleCache.size()<MaxCacheSize; m_LastSampleFolderIndex++ )
			{
				VdrFileInfo sampleInfo = m_SequenceInformation.SampleFolderVector().at(m_LastSampleFolderIndex);
				std::string strKey = sampleInfo.getKey();
				addCache(sampleInfo, _vdrFileType);
			}
		}
	}
}

void SequenceLoader::updateCachingStrategy(std::string const & vdrFileKey, VdrFileType _vdrFileType)
{
	if( _vdrFileType==ImageVdrFileType )
	{
		updateCachingStrategy(vdrFileKey, m_SequenceInformation.ImageFolderMap(), m_SequenceInformation.ImageFolderVector(), m_LastImageFolderIndex);
	}
	else if( _vdrFileType==SampleVdrFileType )
	{
		updateCachingStrategy(vdrFileKey, m_SequenceInformation.SampleFolderMap(), m_SequenceInformation.SampleFolderVector(), m_LastSampleFolderIndex);
	}
}

void SequenceLoader::updateCachingStrategy(std::string const & vdrFileKey, VdrFolderMap_t const & _vdrFolderMap, std::deque<VdrFileInfo> const & _vdrVector, uint32_t & _lastFolderIndex)
{
	VdrFolderMap_t::ConstIterator inProducts = _vdrFolderMap.find(vdrFileKey);
    if( inProducts == _vdrFolderMap.end() )
    {
        return;
    }

    for( uint32_t idx=0; idx<_vdrVector.size(); idx++ )
    {
        VdrFileInfo vdrFileInfo = _vdrVector.at(idx);
        std::string otherKey = vdrFileInfo.getKey();
        if( vdrFileKey==otherKey )
        {
            _lastFolderIndex=idx;
            break;
        }
    }
}

void SequenceLoader::updateCachingStrategy(uint32_t _productNumber)
{
	updateCachingStrategy( _productNumber, m_SequenceInformation.ImageFolderVector(), m_LastImageFolderIndex);
	updateCachingStrategy( _productNumber, m_SequenceInformation.SampleFolderVector(), m_LastSampleFolderIndex);
}

void SequenceLoader::updateCachingStrategy(uint32_t _productNumber, std::deque<VdrFileInfo> const & _vdrVector, uint32_t & _lastFolderIndex)
{
    for( uint32_t idx=0; idx<_vdrVector.size(); idx++ )
    {
        VdrFileInfo vdrFileInfo = _vdrVector.at(idx);
        std::string otherKey = vdrFileInfo.getKey();
    	std::string::size_type pos = otherKey.find_first_of('.');
    	if(pos==std::string::npos)
    	{
    		continue;
    	}

    	std::string otherProductKey = otherKey.substr(0,pos);
    	uint32_t otherProductNumber;
#if !defined(__QNX__)
    	otherProductNumber = std::stoul(otherProductKey);
#else
		if( ! m_SequenceInformation.stringToUint(otherProductKey.c_str(), otherProductNumber))
		{
			return;
		}
#endif
    	if( _productNumber==otherProductNumber )
        {
    		_lastFolderIndex=idx;
            break;
        }
    }
}


void SequenceLoader::addCache( VdrFileInfo & _vdrFileInfo, VdrFileType _vdrFileType )
{
    try
    {
    	if( _vdrFileType==ImageVdrFileType)
    	{
			std::string strKey = _vdrFileInfo.getKey();
			if( ! m_ImageCache.has(strKey) )
			{

				ImageDataHolder imageDataHolder;
				imageDataHolder.setPath(_vdrFileInfo.getPath());
				imageDataHolder.setKey(_vdrFileInfo.getKey());
				Poco::ScopedWriteRWLock oWriteLock(m_ImageQueueMutex);
				m_ImageCache.add( strKey, imageDataHolder );
			}
    	}
    	else if( _vdrFileType==SampleVdrFileType)
    	{
			std::string strKey = _vdrFileInfo.getKey();
			if( ! m_SampleCache.has(strKey) )
			{
				fileio::SampleDataHolder sampleDataHolder;
				fileio::Sample sample( _vdrFileInfo.getPath() );
				if( ! sample.readAllData(sampleDataHolder))
				{
					throw( Poco::Exception(" could not load sample file ", _vdrFileInfo.getPath()));
				}

				Poco::ScopedWriteRWLock oWriteLock(m_SampleQueueMutex);
				m_SampleCache.add( strKey, sampleDataHolder );
			}
    	}
    }
    catch(Poco::Exception const & _ex)
    {
        std::ostringstream oMsg;
        oMsg  << __FUNCTION__ << " " << _ex.what() << " - " << _ex.message() << "\n";
        wmLog(eWarning, oMsg.str());
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
}

void SequenceLoader::loadImage(ImageDataHolder &imageDataHolder)
{
    fileio::Bitmap bmp( imageDataHolder.getPath() );
    image::Size2d size (bmp.width(), bmp.height());
    auto sharedMem = m_memory->nextImagePointer(bmp.fileSize());
    image::TLineImage<byte> image{sharedMem, size};
    std::vector<unsigned char> oAdditionalData;
    if( ! bmp.load(image.begin(), oAdditionalData))
    {
        throw( Poco::Exception(" could not load image file ", imageDataHolder.getPath()));
    }
    imageDataHolder.setByteImage(image);
    if( oAdditionalData.size()>=vdr::add_data_indices::eNbBytes )
    {
        unsigned short sValue;
        sValue = *(reinterpret_cast<unsigned short*>(&oAdditionalData[vdr::add_data_indices::eHwRoiX]));
        imageDataHolder.setHardwareRoiOffsetX(sValue);
        sValue = *(reinterpret_cast<unsigned short*>(&oAdditionalData[vdr::add_data_indices::eHwRoiY]));
        imageDataHolder.setHardwareRoiOffsetY(sValue);
        sValue = *(reinterpret_cast<unsigned short*>(&oAdditionalData[vdr::add_data_indices::eImgNb]));
        imageDataHolder.setImageNumber(sValue);
        imageDataHolder.setIsExtraDataValid(true);
    }
    else
    {
        imageDataHolder.setIsExtraDataValid(false);
    }
}


} // namespace precitec
