/***
*	@file
*	@copyright		Precitec Vision GmbH & Co. KG
*	@author			LB
*	@date			2017
*	@brief			Performs operations useful for cross correlation (convolution, template computation)
*/


#ifndef CrossCorrelationImpl_H_
#define CrossCorrelationImpl_H_
#include <string>
#include "common/frame.h"
#include "geo/size.h"
#include "math/descriptiveStats.h"
#include <numeric> //accumulate
#include "filter/algoImage.h"

namespace precitec 
{
namespace filter 
{
namespace crosscorrelation 
{
		using namespace image;
		using namespace geo2d;
		using precitec::math::DescriptiveStats;		
		using std::numeric_limits;
		
		enum eKernelSource {
			Disabled, FileFromConfig, Box, Gaussian, Checkered, FileFromSystemGraphs,
			ImageToResampleFromConfig, ImageToResampleFromSystemGraphs,
            SparseKernelFromBmp
		};
		enum eCorrelationMethod {
			CrossCorrelation,
			NormedCrossCorrelation
		};

		//template generation methods (without normalization)
		/*
		* Compute a Gaussian Template
		*/
		void calcGaussianTemplate(const int radius, const double sigma, DImage & newTemplate, double & kernelSum);
		void calcGaussianTemplate(const int radius, const double sigma, DImage & rHorizontalComponent, DImage & rVerticalComponent, double & kernelSum);

		/*
		* Compute Sobel Kernel
		*/
		void calcSobelKernel(bool horizontal, DImage & rHorizontalComponent, DImage & rVerticalComponent, double & kernelSum);



		/*
		* Compute a Checkered Template (as in a checkered tablecloth, not as a chessboard)
		L = light intensity
		D = dark intensity
		M = mid intensity sqrt(L*D)

		+-+-+
		|L|M|
		+-+-+
		|M|D|
		+-+-+

		*/
		void calcCheckeredTemplate(const unsigned int pTileWidth, const unsigned int pTileHeight,
			const unsigned int pNumberHorizontalRepetitions, const unsigned int  pNumberVerticalRepetitions,
			const double pLowIntensity, const double pHighIntensity,
			DImage & rHorizontalComponent, DImage & rVerticalComponent, double & rKernelSum);

		/*
		* Dot product (m,1) *(1,n) = (m,n)
		*/
		void multiplyHorizontalAndVerticalMatrix(const DImage & pHorizontalElement, const DImage & pVerticalElement, DImage & rResultMatrix);

		/*
		* Compute a box (square) template
		*/
		void calcBoxTemplate(const unsigned int width, const unsigned int height, DImage & newTemplate, double & kernelSum);
		/**
		* Read bmp image from file
		*/
		BImage readTemplateFromDisk(const std::string & pFilename);



		/**
		* Computes a magnified image via pixel replication
		*/
		template <typename T>
		TLineImage<T> magnifyImage(const image::TLineImage<T> & pImage, int pMagnificationFactor)
		{
			BImage oMagnifiedImage(geo2d::Size(pMagnificationFactor*pImage.width(), pMagnificationFactor*pImage.height()));
			
			for ( int y = 0; y < pImage.height(); ++y )
			{
				auto inputPixel = pImage.rowBegin(y);
				for ( int x = 0; x < pImage.width(); ++x, ++inputPixel )
				{
					auto pixelVal = *inputPixel;

					int i0 = x*pMagnificationFactor;
					int j0 = y*pMagnificationFactor;

					for ( int j = j0; j < j0 + pMagnificationFactor; ++j )
					{
						for ( auto oOutputPixel = oMagnifiedImage.rowBegin(j) + i0, outputPixelEnd= oOutputPixel+pMagnificationFactor;
								oOutputPixel!= outputPixelEnd; 
								++oOutputPixel )
						{
							(*oOutputPixel) = pixelVal;
						}
					}
				}
				assert(inputPixel == pImage.rowEnd(y));
			}
			
			return oMagnifiedImage;
		}//magnifyImage




		//like trasformPixelIntensities for the simpler case when the trasofrmation is ax+b
		template <typename TInput, typename TOutput>
		void rescalePixelIntensities(
			const image::TLineImage<TInput> & pReferenceImage,
			image::TLineImage<TOutput> & pOutputImage,
			double pFactor, double pOffset)
		{
			assert(pOutputImage.size() == pReferenceImage.size());
            
            pReferenceImage.transformTo(pOutputImage, [&pFactor, &pOffset] (TInput pixel)
				{
					return pixel*pFactor + pOffset;
				});
			
		}//rescalePixelIntensities



		/**
		* Computes the sum of pixel intensities in a region of interest of pImage (given by pOrigin, pSize).
		* No range checking is performed
		*/

		template <typename TInput>
		double calcSumIntensitiesOfRegion(
			const image::TLineImage<TInput> & pImage, geo2d::Point pOrigin, geo2d::Size pSize)
		{
            
			int xMin = pOrigin.x;
			int yMin = pOrigin.y;
            
			int xMax = xMin + pSize.width;
			int yMax = yMin + pSize.height;
            
			assert(pImage.isValid());
			if ( xMin == 0 && xMax == pImage.width() && pImage.isContiguos() )
			{
				auto begin = pImage.rowBegin(yMin);
				auto end = pImage.rowEnd(yMax - 1);
				return std::accumulate(begin, end, 0.0);
			}
			
			
			//image or region not contigous, proceed line by line
			double oSum = 0.0;
            pImage.for_each([&oSum](TInput intensity){ oSum += intensity;}, pOrigin.x, pOrigin.y, pSize.width, pSize.height);
            
            return oSum;
		}


		/**
		* Computes the convolution betwen p_rImageIn and p_rTemplate, and saves the result to p_rImageOut.
		* No padding or normalization is performed here.
		*/
		template <typename TPixelType, typename TKernelType, typename TOutputType>
		void calcConvolution(
			const image::TLineImage<TPixelType> & p_rImageIn,
			const image::TLineImage<TKernelType> & p_rTemplate,
			image::TLineImage<TOutputType> & p_rImageOut)
		{

			const auto oTemplateWidth = p_rTemplate.width();
			const auto oTemplateHeight = p_rTemplate.height();
			const auto oImgWidth = p_rImageIn.width();
			const auto oImgHeight = p_rImageIn.height();

			geo2d::Size oSizeOut(oImgWidth - oTemplateWidth + 1, oImgHeight - oTemplateHeight + 1);
			p_rImageOut.resize(oSizeOut);

			assert(p_rTemplate.isContiguos());

			for ( int y = 0; y < oSizeOut.height; ++y )
			{
				auto pixelOut = p_rImageOut.rowBegin(y);
				for ( int x = 0; x < oSizeOut.width; ++x )
				{
					//at the current point (x,y), multiplicate template and input image values
					double result = 0;
					auto pixelTemplate = p_rTemplate.begin();
					for ( int j = 0; j < oTemplateHeight; j++ )
					{
						auto inputPixel = p_rImageIn.rowBegin(y + j)+x;
						const auto inputPixelEnd = inputPixel + oTemplateWidth;
						//pixelTemplate = isTemplateContiguous ? pixelTemplate : p_rTemplate.rowBegin(j); //not needed, in this application the template is always contiguous

						for ( ; inputPixel != inputPixelEnd; ++pixelTemplate, ++inputPixel)
						{
							auto partialResult = (*pixelTemplate) * (*inputPixel);
							result += partialResult;
						}
					} // end template iteration
					assert(pixelTemplate == p_rTemplate.end());
					(*pixelOut) = static_cast<TOutputType> (result);
					pixelOut++;
				}//end line iteration
			}//end image iteration
		} // calcConvolution


		template <typename TInputPixelType, typename TKernelType, typename TOutputPixelType, bool t_normalized>
		void computeResultImage(
			image::TLineImage<TOutputPixelType> & rOutputImage, DescriptiveStats & rStats,
			const image::TLineImage<TInputPixelType> & rImage,
			const image::TLineImage<TKernelType>& p_oKernel1, const image::TLineImage<TKernelType>& p_oKernel2,
			const geo2d::Size p_oKernelSize, double p_oKernelRootSquaredSum,
			const bool p_oFillOutputBorder, 
			bool computeStats,
			TOutputPixelType p_outputMin, TOutputPixelType p_outputMax,
            TLineImage<int> & rIntegralImage            
        )
		{
			//input sanity check should be done before calling this function, these asserts are for debug only
			assert(rImage.size().area() > 0); 
			assert(p_oKernelRootSquaredSum >= 0); //squaredSum = 0 is an edge case, when the template is empty

			//determine output size and padding if necessary
			int w = p_oKernelSize.width;
			int h = p_oKernelSize.height;
			const image::Size2d oComputedSize(rImage.width() - w + 1, rImage.height() - h + 1);
			const Size oOutputSize = p_oFillOutputBorder ? rImage.size() : oComputedSize;

			auto oOutputBorder = Point((oOutputSize.width - oComputedSize.width) / 2, (oOutputSize.height - oComputedSize.height) / 2);

			rOutputImage.resize(oOutputSize);
			
			if ( t_normalized && p_oKernelRootSquaredSum <= 0 )
			{
				//normalization with an empty kernel
				rOutputImage.fill(0);
				return;
			}

			if ( p_oFillOutputBorder )
			{
				rOutputImage.fill(0);
			}

			// compute cross-correlation
			DImage  oCrossCorrelationResult;
            SummedAreaTable< byte,int> squaredImageIntegral;
            if (t_normalized)
            {
                //shallow copy (avoid allocating new memory for m_IntegralImage if rIntergralImage is big enough)
                squaredImageIntegral.m_IntegralImage = rIntegralImage;
                squaredImageIntegral.init<SummedAreaTableOperation::sumSquaredValues>(rImage);
                
                //shallow copy (so that a  rIntegralImage of the correct size is possibly available for the next cycle)
                rIntegralImage = squaredImageIntegral.m_IntegralImage;
            };

			bool useSeparableKernel = p_oKernel2.isValid();
	
			if ( useSeparableKernel )
			{
				assert(p_oKernelSize.width == p_oKernel1.width() && p_oKernelSize.height == p_oKernel2.height());
				DImage oCCorr1;
				calcConvolution<TInputPixelType, double, double>(rImage, p_oKernel1, oCCorr1);
				calcConvolution<double, double, double>(oCCorr1, p_oKernel2, oCrossCorrelationResult);
			}
			else
			{
				assert(p_oKernelSize == p_oKernel1.size());
				calcConvolution<TInputPixelType, double, double>(rImage, p_oKernel1, oCrossCorrelationResult);
			}

			//write output
			poco_assert_dbg(oComputedSize == oCrossCorrelationResult.size());

			TOutputPixelType outputRange = p_outputMax - p_outputMin;
			//iterate again on all the pixel to finish to compute the formula, at the same time write the resultValues and compute stats

			const double expectedMin = rStats.getRangeMin();
			const double expectedMax = rStats.getRangeMax();
			const double expectedRange = expectedMax - expectedMin;
			
			assert(oCrossCorrelationResult.isContiguos());
			auto crossCorrelationPixel = oCrossCorrelationResult.begin();
			for ( int y = 0; y < oComputedSize.height; y++ )
			{
				auto outputPixel = rOutputImage.rowBegin(y + oOutputBorder.y) + oOutputBorder.x;
				for ( int x = 0; x < oComputedSize.width; ++x, ++outputPixel, ++crossCorrelationPixel)
				{
					double val = *crossCorrelationPixel;

					if (t_normalized) //known at compile time
					{
						int xmax = x + w;
						int ymax = y + h;
						auto squareSum = squaredImageIntegral.calcSum(x, xmax, y, ymax);
						val = squareSum > 0 ? val / (p_oKernelRootSquaredSum * sqrt(squareSum)) : 0;
					}

					TOutputPixelType val_out;
					
					//rescale pixel
					if ( val < expectedMax )
					{
						if ( val > expectedMin )
						{    //expectedMin <= val < expectedMax
							val_out = static_cast<TOutputPixelType>(outputRange*((val - expectedMin) / expectedRange) + p_outputMin);
						}
						else
						{	 //val <= expectedMin
							val_out = p_outputMin;
						}
					}
					else
					{	//val >= expectedMax
						val_out = p_outputMax;
					}
					(*outputPixel) = val_out;

					if (computeStats)
					{
						rStats.addIntensityValue(val);
					}//end computeStats
				} // end for y 
			} //end for x
		}

	

} //end namespace crosscorrelation
} //end namespace filter
} //end namespace precitec

#endif /*CrossCorrelationImpl_H_*/
