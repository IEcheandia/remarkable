/***
*	@file			
*	@copyright		Precitec Vision GmbH & Co. KG
*	@author			LB
*	@date			2019
*	@brief			Arithmetic on Images Stacks
*/

#include "frameBuffer.h"

namespace precitec {
namespace filter {
using namespace image;



CachedFrame::CachedFrame()
{
	reset();
}

geo2d::Size CachedFrame::internalSize(const geo2d::Size & rInputSize) const
{
	return geo2d::Size((rInputSize.width - 1) / m_resolutionX + 1, (rInputSize.height - 1) / m_resolutionY + 1);
}

void CachedFrame::set(const BImage & rImage, const interface::SmpTrafo & rTrafo, unsigned int rResolutionX, unsigned int rResolutionY )
{
	m_resolutionX = rResolutionX;
	m_resolutionY = rResolutionY;

	m_image = BImage(internalSize(rImage.size()));
	m_trafo = rTrafo;
	
	if ( m_resolutionX == 1 )
	{
        if (m_resolutionY == 1)
        {
            rImage.copyPixelsTo(m_image);            
        }
        else
        {
            for (unsigned int y = 0, h = m_image.size().height; y < h; ++y )
            {
                auto yInput = y * rResolutionY;
                auto inPixel = rImage.rowBegin(yInput);
                const auto inPixel_end = rImage.rowEnd(yInput);
                auto outPixel = m_image.rowBegin(y);
                std::copy(inPixel, inPixel_end,outPixel);
            }
        }
	}
	else
	{
		for ( unsigned int y = 0, h = m_image.size().height; y < h ; ++y )
		{
			auto inPixel = rImage.rowBegin(y * rResolutionY);
			const auto inPixel_end = rImage.rowEnd(y*rResolutionY);
			auto outPixel = m_image.rowBegin(y);

			for ( ; inPixel < inPixel_end;
				inPixel += m_resolutionX, ++outPixel )
			{
				*outPixel = *inPixel;
			}
		}
	}
}


void CachedFrame::upsample(const image::BImage &rSampledImage,  image::BImage & rDestinationImage, unsigned int resolutionX, unsigned int resolutionY)
{
    //compare CachedFrame::set : the sampling starts from 0,0, the remainder is at the end
  
    assert( int((rDestinationImage.width()-1)/double(resolutionX)+1) == rSampledImage.width());
    assert( int((rDestinationImage.height()-1)/double(resolutionY)+1) == rSampledImage.height());
    
    //we can safely upsample until the last element
    int  xLastColumn = rSampledImage.width()-1;
    //number of times the last column must be repeated
    auto lastXRepeat =  rDestinationImage.width() - xLastColumn*resolutionX; 
    
    const unsigned int heightDestination = rDestinationImage.height();
    
    for (unsigned int ySource = 0, ySourceEnd = rSampledImage.height(); ySource < ySourceEnd; ++ySource)
    {
        unsigned int yDestination = ySource * resolutionY;
        unsigned int yDestinationEnd =  yDestination + resolutionY;
        
        if (yDestinationEnd > heightDestination)
        {
            //last line, can't be always repeated resolutionY times
            assert(int(ySource) ==  rSampledImage.height()-1);
            yDestinationEnd = heightDestination;
        }
        
        auto pRowSourceLastColumn = rSampledImage.rowEnd(ySource)-1; 
        
        for (; yDestination < yDestinationEnd; ++yDestination)
        {
            assert( int(yDestination) < rDestinationImage.height());

            auto pRowSource = rSampledImage.rowBegin(ySource);
            auto pDestination = rDestinationImage.rowBegin(yDestination);
            
            for (; pRowSource != pRowSourceLastColumn; ++pRowSource)
            {
                for (unsigned int i = 0 ;  i < resolutionX;  ++i)
                {
                    *pDestination = *pRowSource;
                    
                    #ifndef NDEBUG
                    int xDestination = pDestination -  rDestinationImage.rowBegin(yDestination);
                    int xSource = pRowSource - rSampledImage.rowBegin(ySource);
                    assert(xSource == int(xDestination / resolutionX));
                    assert( int(ySource) == int(yDestination / resolutionY));
                    #endif                        
                    
                    ++pDestination;
                } //for xDestination                   
            }//for xSource
            
            //now upsample the last column
            assert(pDestination ==  rDestinationImage.rowBegin(yDestination)+xLastColumn * resolutionX);
            
            static_assert(sizeof(*pRowSource) ==  sizeof(byte), "");
            static_assert(sizeof(*pDestination) ==  sizeof(byte), "");
            std::memset( pDestination, *pRowSource, lastXRepeat*sizeof(byte));
            
    } //for yDestination
} //for ySource
}


void CachedFrame::reset()
{
	set(BImage(), nullptr,1,1);
}

bool CachedFrame::initialized() const
{
	return !(m_trafo.isNull());
}

void FrameBuffer::resetState(unsigned int window)
{
	resetState(window, m_ConstraintsOnInputSize, m_oResolutionX, m_oResolutionY);
}

void FrameBuffer::resetState(unsigned int window, ConstraintsOnInputSize oConstraintsOnInputSize, unsigned int resolutionX, unsigned int resolutionY)
{
	m_ConstraintsOnInputSize = oConstraintsOnInputSize;
	m_oResolutionX = std::max(1u, resolutionX);
	m_oResolutionY = std::max(1u, resolutionY);
	m_oRingbuffer.assign(window, CachedFrame());
	m_oLastFilledPosition = decrementIndex(window,1);
	poco_assert_dbg(! (getCachedElement(m_oLastFilledPosition).initialized()) );
}

FrameBuffer::InsertInputFrameState FrameBuffer::insertInputFrame(const interface::ImageFrame& rFrame)
{
	const BImage &rImage = rFrame.data();

    InsertInputFrameState ret = evaluateFrameInsertion(rFrame);
    if (ret == InsertInputFrameState::eFrameSkipped)
    {
        return ret;
    }    
    if (ret == InsertInputFrameState::eBufferReset)
    {
        resetState(m_oRingbuffer.size());
    }

	m_oLastFilledPosition = incrementIndex(m_oLastFilledPosition);
	m_oRingbuffer[m_oLastFilledPosition].set(rImage, rFrame.context().trafo(), m_oResolutionX, m_oResolutionY);

	poco_assert_dbg( getCachedElement(m_oLastFilledPosition).initialized());
	return ret;
}

bool FrameBuffer::isValidIndex(FrameBuffer::index i) const
{
	return i < m_oRingbuffer.size();
}

bool FrameBuffer::isBufferPositionInitialized(FrameBuffer::index i) const
{
	return  !(isValidIndex(i)) || getCachedElement(i).initialized();
}

FrameBuffer::index FrameBuffer::incrementIndex(index i, unsigned int delta) const
{
	i += delta;
	return i % m_oRingbuffer.size();
}

FrameBuffer::index FrameBuffer::decrementIndex(index i, unsigned int delta) const
{
	//index is unsigned, it can't become negative
	return incrementIndex(i, m_oRingbuffer.size() - delta);
}


CachedFrame & FrameBuffer::getCachedElement(FrameBuffer::index i)
{
	assert(isValidIndex(i));
	return m_oRingbuffer[i];
}


const CachedFrame & FrameBuffer::getCachedElement(FrameBuffer::index i) const
{
	assert(isValidIndex(i));
	return m_oRingbuffer[i];
}

CachedFrame & FrameBuffer::getLastCachedFrame()
{
	return getCachedElement(m_oLastFilledPosition);
}

const CachedFrame & FrameBuffer::getLastCachedFrame() const
{
	return getCachedElement(m_oLastFilledPosition);
}


//get last "numberOfFrames" images (eventually skipping the first "recentIndexesToSkip" ), from the most recent to the older
//the result is limited to the buffer size
//TODO: check for contiguous elements?
std::vector<FrameBuffer::index> FrameBuffer::getLastIndexes(unsigned int numberOfFrames, unsigned int recentIndexesToSkip) const
{
	std::vector<FrameBuffer::index> result;
	index lastProcessedIndex = decrementIndex(m_oLastFilledPosition, recentIndexesToSkip);
	numberOfFrames = numberOfFrames < m_oRingbuffer.size() ? numberOfFrames : m_oRingbuffer.size() ;
    result.reserve(numberOfFrames);
	for (unsigned int processedIndexes = recentIndexesToSkip; processedIndexes < numberOfFrames; ++processedIndexes )
	{
		auto & cachedElement = getCachedElement(lastProcessedIndex);
		if ( cachedElement.initialized() )
		{
			result.push_back(lastProcessedIndex); 
			assert(isBufferPositionInitialized(lastProcessedIndex));
			assert(m_ConstraintsOnInputSize == ConstraintsOnInputSize::eKeepEveryInput 
				|| getCachedElement(lastProcessedIndex).m_image.size() == getCachedElement(m_oLastFilledPosition).m_image.size());
			assert(getCachedElement(lastProcessedIndex).m_resolutionX == m_oResolutionX);
			assert(getCachedElement(lastProcessedIndex).m_resolutionY == m_oResolutionY);
		}
		lastProcessedIndex = decrementIndex(lastProcessedIndex, 1);
	}
	return result;
}

unsigned int FrameBuffer::getResolutionX() const
{
	return m_oResolutionX;
}
unsigned int FrameBuffer::getResolutionY() const
{
	return m_oResolutionY;
}

bool  FrameBuffer::hasSampling() const
{
	return m_oResolutionX != 1 || m_oResolutionY != 1;
}

void FrameBuffer::upsample(const image::BImage& rSampledImage, image::BImage& rDestinationImage)
{
    assert((int)((rDestinationImage.width()-1)/double(m_oResolutionX)) + 1 == rSampledImage.width());
    assert((int)((rDestinationImage.height()-1)/double(m_oResolutionY))+ 1 == rSampledImage.height()); 
    CachedFrame::upsample(rSampledImage, rDestinationImage, m_oResolutionX, m_oResolutionY);
}

FrameBuffer::BufferState FrameBuffer::getBufferState() const
{
    unsigned int bufferSize = m_oRingbuffer.size();
    
    unsigned int initializedElements = 0; 
    
    const auto reverseIt_atLastFilledPosition = m_oRingbuffer.rend() - m_oLastFilledPosition -1;
    const auto reverseIt_before0 = m_oRingbuffer.rend();
    
    //count initialized elements from  m_oRingbuffer[m_oLastFilledPosition] to m_oRingbuffer[0] (included)
    auto reverseIt = reverseIt_atLastFilledPosition;
    while (reverseIt != reverseIt_before0 && reverseIt->initialized())
    {
        ++initializedElements;
        ++reverseIt;
    }
    
    if (reverseIt == reverseIt_before0)
    {
        //continue: count initialized elements from m_oRingbuffer[size - 1] to m_oRingbuffer[m_oLastFilledPosition+1] 
        
        reverseIt = m_oRingbuffer.rbegin();        
        while (reverseIt != reverseIt_atLastFilledPosition && reverseIt->initialized())
        {
            ++initializedElements;
            ++reverseIt;
        }
    }
    
    assert(initializedElements <= bufferSize);
    assert(initializedElements == getLastIndexes(m_oRingbuffer.size()).size());
    return {initializedElements, bufferSize};
}



} // namespace filter
} // namespace precitec

