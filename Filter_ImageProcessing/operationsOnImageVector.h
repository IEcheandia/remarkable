#pragma once

#include "common/frame.h"

namespace precitec
{
namespace filter
{
    enum class Operations
	{
		ePixelSum = 0,
		ePixelDiff = 1,
		ePixelMin = 2,
		ePixelMax = 3,
		ePixelMean = 4,
		ePixelStd = 5,
		ePixelRange = 6,
		ePixelMedian = 7,
		eRepeat = 8,
		eInvalid = 9, 
        NumberValidOperations =10, 
	};
	
    class OperationsOnImageVector
    {
    public:
        OperationsOnImageVector (int width=0, int height=0);
        void clear(int width, int height);
        bool insertImage(const image::BImage & rImage);
        std::size_t numberOfImages() const ;
        int w() const;
        int h() const;
        void computeResultImage(image::TLineImage<byte> & rOutput, Operations p_operation, bool stretchContrast);
        
        bool resultWithMovingWindowAvailable(Operations p_operation, bool stretchContrast, unsigned int expectedWindow) const;
        //update results as in a sliding window (buffer still growing)
        bool tryUpdateResultWithMovingWindow(image::TLineImage<byte> & rOutput, const image::BImage & rImageToAdd, 
                                             Operations p_operation, bool stretchContrast, unsigned int expectedWindow);
        //update results as in a sliding window 
        bool tryUpdateResultWithMovingWindow(image::TLineImage<byte> & rOutput, const image::BImage & rImageToRemove, const image::BImage & rImageToAdd, 
                                             Operations p_operation, bool stretchContrast, unsigned int expectedWindow);
        

    private:
        class IntermediateResults 
        {
        public:
            void reset() {m_operation = Operations::eInvalid; m_window=0;};
            bool valid(Operations operation, unsigned int window) const
            {
                return operation == m_operation && window == m_window;
            }
            image::TLineImage<int> & ref(Operations operation, unsigned int window)
            {
                //track the reason before return a non-const reference
                m_operation = operation; m_window = window;
                return m_image;
            }
            const image::TLineImage<int> & get() const {return m_image;};
        private:
            image::TLineImage<int> m_image;
            Operations m_operation = Operations::eInvalid;
            unsigned int m_window = 0;
            
        };
        
        bool prepareNextRow();
        void updateRowPointers();
        void incrementX();
        const std::vector<image::BImage> & images() const;
        const std::vector<const byte *> & imagePixelsPointers() const;
        std::vector<image::BImage> m_Images;
        std::vector<const byte *> m_ImagePixelsPointers;
        IntermediateResults m_IntermediateResults; //needed only for certain operations that risk overflow, or to update result wiht moving window
        
        int m_x;
        int m_y;
        int m_w;
        int m_h;
        
        //function that performes the specific operation. The input argument is a vector of pointers to same pixel location in 
        // multiple images. The functions must be called in the inner loop, iterating on all pixel positions
        template<Operations t_operation, typename T> inline
        T applyOnPixel() const;
        
        //iterate on all the pixel and compute the resulting image
        template<Operations t_operation, typename T> inline
        void applyOnImage(image::TLineImage<T> & rOutputImage);
        
        //iterate on all the pixel and compute the resulting image, tracking minimum and maximum value
        template<Operations t_operation, typename T> 
        void applyOnImage(image::TLineImage<T> & rOutputImage, T & rMin, T & rMax);
        
        template<Operations t_operation> 
        void  applyOnImageAndStretchContrastInPlace(image::TLineImage<byte> & rOutput);
    
        
        template<Operations t_operation> 
        void  applyOnImageAndStretchContrast(image::TLineImage<byte> & rOutput);

    };
    
    
    
}
}
