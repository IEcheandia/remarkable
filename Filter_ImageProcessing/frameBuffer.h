#pragma once

#include "common/frame.h"

namespace precitec
{
namespace filter
{
    class CachedFrame
    {
    public:
        CachedFrame();
        geo2d::Size internalSize(const geo2d::Size & rInputSize) const;
        void set(const image::BImage & rImage, const interface::SmpTrafo & rTrafo,
            unsigned int rResolutionX, unsigned int rResolutionY);
        void reset();
        bool initialized() const;
        image::BImage m_image;
        interface::SmpTrafo m_trafo;
        unsigned int m_resolutionX;
        unsigned int m_resolutionY;
        static void upsample(const image::BImage &rSampledImage,  image::BImage & rDestinationImage,
            unsigned int resolutionX,  unsigned int resolutionY);
    };

    typedef std::vector<CachedFrame>  TCachedFrameVector;

    class FrameBuffer
    {
    public:
        enum ConstraintsOnInputSize
        {
            eResetStateOnDifferentSize, eSkipInsertionOnDifferentSize, eKeepEveryInput
        };
        enum class InsertInputFrameState
        {
            eFrameSkipped, eFrameAdded, eBufferReset
        };
        struct BufferState
        {
            unsigned int initializedElements; 
            unsigned int bufferSize;
        };
        typedef unsigned int index;

        void resetState(unsigned int window);
        void resetState(unsigned int window, ConstraintsOnInputSize oConstraintsOnInputSize, unsigned int resolutionX, unsigned int resolutionY);
        InsertInputFrameState insertInputFrame(const interface::ImageFrame& rFrame);
        ConstraintsOnInputSize getConstraintOnInputSize() const 
        {
            return m_ConstraintsOnInputSize;
        }
        //compatibleSize: true if size compatible with last buffer element (of buffer empty), false if different (independent from current constraints on inputsize)
        bool compatibleSize(const interface::ImageFrame& rFrame) const 
        {
            auto rLastElement = getCachedElement(m_oLastFilledPosition);
            if (!rLastElement.initialized())
            {
                return true; //buffer is empty, any image is compatible
            }
            return rLastElement.m_image.size() == rLastElement.internalSize(rFrame.data().size());
        }
        
        InsertInputFrameState evaluateFrameInsertion(const interface::ImageFrame& rFrame) const
        {
            if ( m_ConstraintsOnInputSize == ConstraintsOnInputSize::eKeepEveryInput
                || compatibleSize(rFrame) )
            {
                return InsertInputFrameState::eFrameAdded;
            }

            switch ( m_ConstraintsOnInputSize )
            {
                case ConstraintsOnInputSize::eResetStateOnDifferentSize:
                    return InsertInputFrameState::eBufferReset;
                    
                case ConstraintsOnInputSize::eSkipInsertionOnDifferentSize:
                    return InsertInputFrameState::eFrameSkipped;
                    
                default:
                case ConstraintsOnInputSize::eKeepEveryInput:
                    assert(false && "case should be already handled");
                    return InsertInputFrameState::eFrameSkipped; //return some kind of error
            }
            
        }
        
        CachedFrame & getCachedElement(index i);
        const CachedFrame & getCachedElement(index i) const;
        CachedFrame & getLastCachedFrame();
        const CachedFrame & getLastCachedFrame() const;
        bool isValidIndex(index i) const;
        bool isBufferPositionInitialized(index i) const;
        std::vector<index> getLastIndexes(unsigned int numberOfFrames, unsigned int recentIndexesToSkip = 0) const;
        unsigned int getResolutionX() const;
        unsigned int getResolutionY() const;
        bool hasSampling() const;
        void upsample(const image::BImage &rSampledImage,  image::BImage & rDestinationImage);
        BufferState getBufferState() const;

    private:
        ConstraintsOnInputSize m_ConstraintsOnInputSize;
        unsigned int m_oLastFilledPosition;				///< counter for ringbuffer
        TCachedFrameVector m_oRingbuffer;			///< Ringbuffer for  values
        unsigned int m_oResolutionX;        ///< "subsample" operate on every m_oResolutionX pixels in x direction
        unsigned int m_oResolutionY;		  ///<  "subsample" operate on every m_oResolutionX pixels in x direction

        index incrementIndex(index i, unsigned int delta = 1) const;
        index decrementIndex(index i, unsigned int delta) const;

    };

}
}
