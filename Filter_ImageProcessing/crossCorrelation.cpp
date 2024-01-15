/***
*	@file
*	@copyright		Precitec Vision GmbH & Co. KG
*	@author			LB
*	@date			2017
*	@brief			Convolution With Template
*/

#include "crossCorrelation.h"
#include "crossCorrelationImpl.h"
#include "system/tools.h"
#include "module/moduleLogger.h"
#include "overlay/overlayPrimitive.h"
#include "common/bitmap.h"
#include "filter/algoImage.h"
#include "filter/algoPoint.h"

#include <fliplib/TypeToDataTypeImpl.h>


namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
	namespace filter {
		using namespace crosscorrelation;

		const std::string	CrossCorrelation::m_oFilterName = std::string("CrossCorrelation");
		const std::string	CrossCorrelation::m_oPipeOutImage = std::string("ImageFrame");
		const std::string	CrossCorrelation::m_oPipeOutTemplate = std::string("Template");


		CrossCorrelation::CrossCorrelation() :
			TransformFilter(CrossCorrelation::m_oFilterName, Poco::UUID{"43c86086-19e0-437c-bb18-95c82e44a945"}),
			m_pPipeInImageFrame(nullptr),
			m_oPipeImageFrame(this, CrossCorrelation::m_oPipeOutImage),
			m_oPipeTemplateFrame(this, CrossCorrelation::m_oPipeOutTemplate),
			m_oSource(eKernelSource::Box),
			m_oMethod(eCorrelationMethod::CrossCorrelation),
			m_oTemplateNumber(0),
			m_oTemplateWidth(3),
			m_oTemplateHeight(3),
			m_oSigma(0.5),
			m_oIntensityUpperLeftCorner(1),
			m_oIntensityLowerRightCorner(255),
			m_oSquareLength(2),
			m_oKernelParameter1(-1), //backcompatibility
			m_oKernelParameter2(-1), //backcompatibility
			m_oFillOutputBorder(false),
			m_oRangeInputMinPerc(0.0),
			m_oRangeInputMaxPerc(100.0),
			m_skipPaint(false)
		{
			parameters_.add("Source", fliplib::Parameter::TYPE_int, m_oSource);
			parameters_.add("Method", fliplib::Parameter::TYPE_int, m_oMethod);
			parameters_.add("TemplateNumber", fliplib::Parameter::TYPE_UInt32, m_oTemplateNumber);
			parameters_.add("TemplateWidth", fliplib::Parameter::TYPE_UInt32, m_oTemplateWidth);
			parameters_.add("TemplateHeight", fliplib::Parameter::TYPE_UInt32, m_oTemplateHeight);

			parameters_.add("Sigma", fliplib::Parameter::TYPE_double, m_oSigma );
			parameters_.add("IntensityUpperLeftCorner", fliplib::Parameter::TYPE_UInt32, m_oIntensityUpperLeftCorner );
			parameters_.add("IntensityLowerRightCorner", fliplib::Parameter::TYPE_UInt32, m_oIntensityLowerRightCorner );
			parameters_.add("SquareLength", fliplib::Parameter::TYPE_UInt32, m_oSquareLength);
			parameters_.add("KernelParameter1", fliplib::Parameter::TYPE_double, m_oKernelParameter1); //backcompatibility
			parameters_.add("KernelParameter2", fliplib::Parameter::TYPE_double, m_oKernelParameter2); //backcompatibility
			parameters_.add("FillOutputBorder", fliplib::Parameter::TYPE_bool, m_oFillOutputBorder);
			parameters_.add("RangeInputMinPerc", fliplib::Parameter::TYPE_double, m_oRangeInputMinPerc);
			parameters_.add("RangeInputMaxPerc", fliplib::Parameter::TYPE_double, m_oRangeInputMaxPerc);

            setInPipeConnectors({{Poco::UUID("319b06ec-785f-48af-8836-13e9864f8cef"), m_pPipeInImageFrame, "ImageIn", 0, "ImageIn"}});
            setOutPipeConnectors({{Poco::UUID("fb017e8a-da69-4bac-a3b8-a8ab885e06ee"), &m_oPipeImageFrame, "ImageFrame", 0, "ImageFrame"},
            {Poco::UUID("b37411c5-cc72-4f70-832a-2578ecdd0614"), &m_oPipeTemplateFrame, "Template", 0, "Template"}});
            setVariantID(Poco::UUID("fdd30629-cba3-4d19-8f44-ee03b7a441a9"));
		}


        bool CrossCorrelation::TemplateFromFileInfo::update(int source, int templateNumber)
        {
            if (m_source == source && templateNumber == m_templateNumber)
            {
                return false;
            }
            switch(source)
            {
                case FileFromConfig:
                case FileFromSystemGraphs:
                    m_resampleImage = false;
                    break;
                case ImageToResampleFromConfig:
                case ImageToResampleFromSystemGraphs:
                case SparseKernelFromBmp:
                    m_resampleImage = true;
                    break;
                default:
                    m_originalTemplate.clear();
                    return false;
            }

            m_originalTemplate.clear();
            m_lastUsedSamplingX = 1.0;
            m_lastUsedSamplingY = 1.0;

            std::ostringstream oFilename;
            oFilename << system::wmBaseDir();

            switch(source)
            {
                case FileFromConfig:
                case ImageToResampleFromConfig:
                case SparseKernelFromBmp:
                    oFilename << "/config/graphs/pattern/" ;
                    break;
                case FileFromSystemGraphs:
                case ImageToResampleFromSystemGraphs:
                    oFilename << "/config/graphs/pattern/" ;
                    break;
                default:
                    assert(false);
                    return false;
            }
            oFilename << std::setfill('0') << std::setw(3) << templateNumber << ".bmp";
            m_originalTemplate = readTemplateFromDisk(oFilename.str());
            m_resampledTemplate = m_originalTemplate;
            if (source == SparseKernelFromBmp )
            {
                m_oSparseKernel = {m_resampledTemplate};
            }
            else
            {
                m_oSparseKernel = {};
            }
            return true;
        }


        bool CrossCorrelation::TemplateFromFileInfo::needsToResampleTemplate(double samplingX, double samplingY) const
        {
            return m_originalTemplate.isValid() && m_resampleImage && !(samplingX == m_lastUsedSamplingX && samplingY == m_lastUsedSamplingY);
        }

        void CrossCorrelation::TemplateFromFileInfo::updateTemplates(double samplingX, double samplingY)
        {
            if (needsToResampleTemplate(samplingX, samplingY))
            {
                m_resampledTemplate.clear(); // to avoid writing into m_originalTemplate if it's a shallow copy
                if (samplingX == 1 && samplingY == 1)
                {
                    m_resampledTemplate = m_originalTemplate;
                }
                else if (samplingX >= 1 && samplingY >= 1)
                {
                    upsampleImage(m_resampledTemplate, m_originalTemplate, std::floor(samplingX), std::floor(samplingY));
                }
                else if (samplingX <= 1 && samplingY <= 1)
                {
                    downsampleImage(m_resampledTemplate, m_originalTemplate, std::floor(1.0/samplingX), std::floor(1.0/samplingY));
                }
                else
                {
                    m_resampledTemplate.clear();
                    wmLog(eWarning, "Unsupported sampling factor %f, %f \n", samplingX, samplingY);
                }

                if (!m_oSparseKernel.empty())
                {
                    m_oSparseKernel = {m_resampledTemplate};
                    if (m_oSparseKernel.empty())
                    {
                        //use a dummy image image with one element, otherwise the next time sparseKernel it's ignored because it's empty
                        BImage dummyImage(geo2d::Size(1,1));
                        dummyImage[0][0] = 1;
                        m_oSparseKernel = {dummyImage};
                    }
                    assert(!m_oSparseKernel.empty());
                }

                m_lastUsedSamplingX = samplingX;
                m_lastUsedSamplingY = samplingY;
            }
        }

        const BImage & CrossCorrelation::TemplateFromFileInfo::getTemplate(double samplingX, double samplingY)
        {
            updateTemplates(samplingX, samplingY);
            return m_resampledTemplate;
        }

        const SparseKernel & CrossCorrelation::TemplateFromFileInfo::getSparseKernel(double samplingX, double samplingY)
        {
            updateTemplates(samplingX, samplingY);
            return m_oSparseKernel;
        }

		void CrossCorrelation::setParameter()
		{
			TransformFilter::setParameter();

			//read previous parameter values, in order to recompute the kernel only when something relevant has changed
			auto prevSource = m_oSource;
			auto prevTemplateWidth = m_oTemplateWidth;
			auto prevTemplateHeight = m_oTemplateHeight;
			auto prevSigma = m_oSigma;
			auto prevIntensityUpperLeftCorner = m_oIntensityUpperLeftCorner;
			auto prevIntensityLowerRightCorner = m_oIntensityLowerRightCorner;
			auto prevSquareLength = m_oSquareLength;
			auto prevMethod = m_oMethod;

			//update parameters from WM
			m_oSource = static_cast<eKernelSource>(parameters_.getParameter("Source").convert<int>());
			m_oMethod = static_cast<eCorrelationMethod>(parameters_.getParameter("Method").convert<int>());
			m_oTemplateNumber = parameters_.getParameter("TemplateNumber").convert<unsigned int>();
			m_oTemplateWidth = parameters_.getParameter("TemplateWidth").convert<unsigned int>();
			m_oTemplateHeight = parameters_.getParameter("TemplateHeight").convert<unsigned int>();
			m_oSigma = parameters_.getParameter( "Sigma" ).convert<double>();
			m_oIntensityUpperLeftCorner = parameters_.getParameter( "IntensityUpperLeftCorner" ).convert<unsigned int>();
			m_oIntensityLowerRightCorner = parameters_.getParameter( "IntensityLowerRightCorner" ).convert<unsigned int>();
			m_oSquareLength = parameters_.getParameter( "SquareLength" ).convert<unsigned int>();
			//backcompatibility
			m_oKernelParameter1 = parameters_.getParameter("KernelParameter1").convert<double>();
			m_oKernelParameter2 = parameters_.getParameter("KernelParameter2").convert<double>();
			m_oFillOutputBorder = parameters_.getParameter("FillOutputBorder").convert<bool>();

			m_oRangeInputMinPerc = parameters_.getParameter("RangeInputMinPerc").convert<double>();
			m_oRangeInputMaxPerc = parameters_.getParameter("RangeInputMaxPerc").convert<double>();
			//validity check
			if ( m_oRangeInputMaxPerc <= m_oRangeInputMinPerc )
			{
				wmLog(eError, "Wrong input range, restoring to default 0-100");
				m_oRangeInputMinPerc = 0;
				m_oRangeInputMaxPerc = 100;
			}

			//backcompatibility (in v3.5  kernelParameter1 and  kernelParameter2 where used)
			if ( (m_oKernelParameter1 != -1) && (m_oKernelParameter2 != -1) )
			{
				switch ( m_oSource )
				{
					case eKernelSource::Box:
						wmLog( eWarning, "Compatibility mode for CrossCorrelation Parameters" );
						m_oTemplateWidth = static_cast<unsigned int>(m_oKernelParameter1);
						m_oTemplateHeight = static_cast<unsigned int>(m_oKernelParameter2);
						break;
					case eKernelSource::Gaussian:
						wmLog( eWarning, "Compatibility mode for CrossCorrelation Parameters" );
						m_oSigma = m_oKernelParameter1;
						m_oTemplateWidth = int(m_oKernelParameter2);
						m_oTemplateHeight = int(m_oKernelParameter2);
					default:
						break;
				}
			}

			//pre-compute kernel if necessary
			//attention to default parameters, they haven't changed but m_oKernel has not been computed yet (not valid)
			bool bSourceHasChanged = !m_oBKernel.isValid() || prevSource != m_oSource;

            bool imageFromDiskUpdated = m_oTemplateFromFileInfo.update(m_oSource, m_oTemplateNumber);//compute only if filename has changed (assuming that file on disk doesn't change)

			switch (m_oSource){
			case eKernelSource::Disabled:
				wmLog(eError, "Method %d not supported\n", m_oSource);
				break;
            case eKernelSource::FileFromConfig:
            case eKernelSource::FileFromSystemGraphs:
            case eKernelSource::ImageToResampleFromConfig:
            case eKernelSource::ImageToResampleFromSystemGraphs:
                if (imageFromDiskUpdated || prevMethod != m_oMethod)
                {
                    bool normalize = (m_oMethod == eCorrelationMethod::NormedCrossCorrelation);
                    auto oTemplateImage = m_oTemplateFromFileInfo.getTemplate(1.0,1.0);
                    updateKernel(oTemplateImage, normalize);
                }
				break;
            case eKernelSource::SparseKernelFromBmp:
                if (imageFromDiskUpdated || prevMethod != m_oMethod)
                {
                    bool normalize = (m_oMethod == eCorrelationMethod::NormedCrossCorrelation);
                    if (!normalize)
                    {
                        wmLog(eWarning, "Only normalized crosscorrelation impleented \n");
                    }
                    auto oTemplateImage = m_oTemplateFromFileInfo.getTemplate(1.0,1.0);
                    updateKernel(oTemplateImage, normalize);//TODO update only the paint variables
                }
				break;


			case eKernelSource::Box:
				if (bSourceHasChanged || (prevTemplateWidth != m_oTemplateWidth) || (prevTemplateHeight != m_oTemplateHeight))
				{
					DImage horizontalKernelComponent;
					DImage verticalKernelComponent;
					double horizontalKernelComponentSum, verticalKernelComponentSum;
					calcBoxTemplate( m_oTemplateWidth, 1, horizontalKernelComponent, horizontalKernelComponentSum );
					calcBoxTemplate( 1, m_oTemplateHeight, verticalKernelComponent, verticalKernelComponentSum );
					updateKernel( horizontalKernelComponent, verticalKernelComponent, horizontalKernelComponentSum*verticalKernelComponentSum );
				}
				break;
			case eKernelSource::Gaussian:
				if ( m_oTemplateWidth != m_oTemplateHeight )
				{
					wmLog( eWarning, "Implemented only symmetric gaussian filter, height will be bypassed" );
				}
				if (bSourceHasChanged || (prevTemplateWidth != m_oTemplateWidth) || (prevSigma != m_oSigma))
				{
					double sigma = m_oSigma;
					int radius = int(std::floor(m_oTemplateWidth / 2));
					DImage horizontalKernelComponent;
					DImage verticalKernelComponent;
					double kernelSum;
					calcGaussianTemplate( radius, sigma, horizontalKernelComponent, verticalKernelComponent, kernelSum );
					updateKernel( horizontalKernelComponent, verticalKernelComponent, kernelSum );
				}
				break;
			case eKernelSource::Checkered :
				if (bSourceHasChanged ||
					(prevIntensityUpperLeftCorner != m_oIntensityUpperLeftCorner) || (prevIntensityLowerRightCorner != m_oIntensityLowerRightCorner) || (prevSquareLength != m_oSquareLength) ||
					(prevTemplateWidth != m_oTemplateWidth) || (prevTemplateHeight != m_oTemplateHeight))
				{
					int oTileSize = int( m_oSquareLength );
					int oHorizontalRepetitions = int(std::max( 1.0, std::ceil( m_oTemplateWidth / (2 * oTileSize) ) ));
					int oVerticalRepetitions = int(std::max( 1.0, std::ceil( m_oTemplateHeight / (2 * oTileSize) ) ));
					double oLowIntensity = m_oIntensityUpperLeftCorner;
					double oHighIntensity = m_oIntensityLowerRightCorner;
					double kernelSum;
					DImage horizontalKernelComponent;
					DImage verticalKernelComponent;
					calcCheckeredTemplate(oTileSize, oTileSize, oHorizontalRepetitions, oVerticalRepetitions, oLowIntensity, oHighIntensity,
						horizontalKernelComponent, verticalKernelComponent, kernelSum);
					updateKernel(horizontalKernelComponent, verticalKernelComponent, kernelSum);
				}
				break;

			default:
				wmLog(eError, "Method %d not implemented\n", m_oSource);
				updateKernel( DImage( Size( 0, 0 )), 0.0 );
				break;
			}
		}


		bool CrossCorrelation::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
		{
			m_pPipeInImageFrame = dynamic_cast<fliplib::SynchronePipe<ImageFrame> *>(&p_rPipe);
			// herewith we set the proceed as callback (return is always true)
			return BaseFilter::subscribe(p_rPipe, p_oGroup);
		} // subscribe

		void CrossCorrelation::updateKernel(const BImage & pTemplateImage, bool pNormalize)
		{
            if ( pTemplateImage.isValid() )
            {
                //normalize kernel
                double k = pNormalize ? calcSumIntensitiesOfRegion<byte>(pTemplateImage, Point(0, 0), pTemplateImage.size()) : 1.0;
                if ( k == 0 )
                {
                    wmLog(eWarning, "Kernel sum =0, cannot be normalized");
                    k = 1.0;
                }
                double normFactor = 1 / k;
                m_oKernel1.resize(pTemplateImage.size());
                rescalePixelIntensities<byte, double>(pTemplateImage, m_oKernel1, normFactor, 0);
            }
            else
            {
                m_oKernel1.clear();
            }
			updateKernelVariables(false);

		}

		void CrossCorrelation::updateKernel(const DImage & pTemplateImage, double pNormalizationFactor)
		{
			if (pNormalizationFactor == 0)
			{
				wmLog(eWarning, "Kernel sum = 0, cannot be normalized");
			}
			else
			{
                m_oKernel1.resize(pTemplateImage.size());
                if ( pTemplateImage.isValid() )
                {
                    double k = 1 / pNormalizationFactor;
                    rescalePixelIntensities<double, double>(pTemplateImage, m_oKernel1, k, 0);
                }
			}
			updateKernelVariables(false);
		}

		void CrossCorrelation::updateKernel(const DImage & pTemplateHorizontalComponent, const DImage & pTemplateVerticalComponent,
			double pNormalizationFactor)
		{

			if (pNormalizationFactor == 0)
			{
				wmLog(eWarning, "Kernel sum =0, cannot be normalized");
			}
			else
			{

				double k = 1 / pNormalizationFactor;
				m_oKernel1.resize(pTemplateHorizontalComponent.size());
				m_oKernel2.resize(pTemplateVerticalComponent.size());
                if ( pTemplateHorizontalComponent.isValid() && pTemplateVerticalComponent.isValid() )
                {
                    rescalePixelIntensities<double, double>(pTemplateHorizontalComponent, m_oKernel1, k, 0);
                    rescalePixelIntensities<double, double>(pTemplateVerticalComponent, m_oKernel2, 1, 0); //copies every pixel, is it necessary?
                }

			}
			updateKernelVariables(true);
		}

		void CrossCorrelation::updateKernelVariables( const bool isSeparableKernel )
		{
			if ( !m_oKernel1.isValid() )
			{
				m_oBKernel.clear();
				m_oKernelRootSquaredSum = -1;
				return;
			}

			double kernelSum = 0;
			double kernelSquaredSum = 0;
			double maxValue = 0; //so far we are only using kernels with positive elements
			DImage actualKernel;
			int P = m_oKernel1.width();
			int Q = m_oKernel2.height();


			bool useKernelComponents = isSeparableKernel;

			if ( isSeparableKernel )
			{
				//if I have the components of the kernel, I need in to compute the actual one
				multiplyHorizontalAndVerticalMatrix(m_oKernel1, m_oKernel2, actualKernel);
				//For a kernel of size (P,Q), the advantage is PQ/(P+Q) e.g http://blogs.mathworks.com/steve/2006/10/04/separable-convolution/
				if ( (P*Q < (P + Q)) )
				{
					//it's not worth to use a separable kernel, I replace the member variables and forget that I had the two components
					useKernelComponents = false;
					m_oKernel1 = actualKernel;
					m_oKernel2.clear();
				}
			}

			if ( !useKernelComponents )
			{
				actualKernel = m_oKernel1;
				//make sure m_oKernel2 is not valid, because that's what indicates whether we are using a separable kernel or not
				m_oKernel2.clear();
			}

			if (!actualKernel.isValid())
			{
				m_oBKernel.clear();
				m_oKernelRootSquaredSum = -1;
				return;
			}

			for (auto && pixelValue : actualKernel)
			{
				kernelSum += pixelValue;
				kernelSquaredSum += (pixelValue * pixelValue);
				maxValue = pixelValue > maxValue ? pixelValue : maxValue;
			};


			if ((kernelSum < 0.9) || (kernelSum > 1.1))
			{
				wmLog(eWarning, "Kernel is not normalized: size %d %d sum=%f", actualKernel.width(), actualKernel.height(), kernelSum);
			}

			if (maxValue == 0)
			{
				wmLog(eWarning, "maxValue 0");
				maxValue = 1; //avoid division by zero
			}

			m_oKernelRootSquaredSum = sqrt(kernelSquaredSum);

            //rescale to byte
			m_oBKernel.resize(actualKernel.size());
            double oFactor = 255/maxValue;
            actualKernel.transformTo(m_oBKernel,
                                     [&oFactor](const double & val)
                                     {
                                         double result = oFactor * val;
                                         result = result < 255 ? result : 255;
                                         return result > 0 ? static_cast<byte>(result) : 0;
                                    }
                         );
		}



		void CrossCorrelation::proceed(const void* sender, fliplib::PipeEventArgs& e)
		{
			poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor

			// Empfangenes Frame auslesen
			const ImageFrame & rFrame = m_pPipeInImageFrame->read(m_oCounter);
			const BImage & rImage = rFrame.data();
			ResultType	curAnalysisResult = rFrame.analysisResult();

			//compute the correlation

			//decide parameters according to method
			double defaultRange, defaultMin;
			bool oNormalized;
			switch ( m_oMethod )
			{
				case eCorrelationMethod::CrossCorrelation:
					oNormalized = false;
					defaultMin = 0;
					defaultRange = 255;
					break;
				case eCorrelationMethod::NormedCrossCorrelation:
					oNormalized = true;
					defaultMin = 0;
					defaultRange = 1;
					break;
				default:
					wmLog(eError, "unknown method %d", m_oMethod);
					oNormalized=false;
					defaultRange=0;
					defaultMin = 0;
					assert(true);
			}

			if (m_oTemplateFromFileInfo.needsToResampleTemplate(rFrame.context().SamplingX_, rFrame.context().SamplingY_))
			{
				updateKernel(m_oTemplateFromFileInfo.getTemplate(rFrame.context().SamplingX_, rFrame.context().SamplingY_), oNormalized);
			}
			const auto & rSparseKernel = m_oTemplateFromFileInfo.getSparseKernel(rFrame.context().SamplingX_, rFrame.context().SamplingY_);

			double expectedMax = defaultMin + m_oRangeInputMaxPerc / 100 * defaultRange;
			double expectedMin = defaultMin + m_oRangeInputMinPerc / 100 * defaultRange;
			assert(expectedMin < expectedMax);

			unsigned int numberOfBins = 5;
			bool computeStats = m_oVerbosity >= eMedium && !m_skipPaint;
			byte outputMin = 0;
			byte outputMax = 255;
			DescriptiveStats stats(expectedMin, expectedMax, numberOfBins);

            if ((m_oSource == SparseKernelFromBmp ) && !(rSparseKernel.empty()) && rImage.isValid() )
            {
                DImage oResultImage;
                switch (m_oMethod)
                {
                    case eCorrelationMethod::CrossCorrelation:
                        oResultImage = rSparseKernel.crossCorrelation(rImage, true);
                        break;
                    case eCorrelationMethod::NormedCrossCorrelation:
                        oResultImage = rSparseKernel.normalizedCrossCorrelation(rImage, true);
                        break;
                    default:
                        wmLog(eError, "unknown method %d", m_oMethod);
                        break;
                }

                if (!m_oFillOutputBorder)
                {
                    wmLog(eWarning, "FillOutputBorder=false not implemented for sparse kernel\n"); //TODO
                }

                m_oImageOut.resizeFill(rImage.size(), 0);

                double minMatch = oResultImage[0][0];
                double maxMatch = minMatch;
                oResultImage.for_each([&minMatch, &maxMatch](const double & pixel)
                    {
                        minMatch = std::min(minMatch, pixel);
                        maxMatch = std::max(maxMatch, pixel);
                    }
                );

                byte outputRange = 255;

                for (int y = 0, yMax = rImage.height(); y < yMax; y++)
                {
                    for (int x = 0, xMax = rImage.width(); x < xMax; x++)
                    {
                        auto val = oResultImage[y][x];
                        auto & val_out = m_oImageOut[y][x];
                        if ( val < expectedMax )
                        {
                            if ( val > expectedMin )
                            {    //expectedMin <= val < expectedMax
                                val_out = static_cast<byte>(outputRange*((val - expectedMin) / defaultRange) + 0);

                            }
                            else
                            {	 //val <= expectedMin
                                val_out = 0;
                            }
                        }
                        else
                        {	//val >= expectedMax
                            val_out = 255;
                        }
                        if (computeStats)
                        {
                            stats.addIntensityValue(val);
                        }//end computeStats
                    }
                }


            }
            else
            {
                if (!m_oBKernel.isValid() || !rImage.isValid() ||
                    (m_oBKernel.width() > rImage.width()) ||
                    (m_oBKernel.height() > rImage.height()) )
                {
                    m_oImageOut.clear();
                    //since the output is an empty image, no overlay will be painted
                    wmLog(eDebug, "CrossCorrelation: image and template size not compatible ([%d %d],[%d %d]) \n",
                        rImage.width(), rImage.height(), m_oBKernel.width(), m_oBKernel.height()
                        );
                    ImageFrame oNewFrame( rFrame.context(), m_oImageOut, rFrame.analysisResult() );
                    ImageFrame oTemplateFrame( rFrame.context(), m_oBKernel, rFrame.analysisResult() );
                    preSignalAction();
                    m_oPipeImageFrame.signal( oNewFrame ); m_oPipeTemplateFrame.signal( oTemplateFrame );	// invoke linked filter(s)
                    return;
                }


                if (oNormalized)
                {
                    computeResultImage<byte,double,byte,true>( m_oImageOut, stats,
                            rImage,
                            m_oKernel1, m_oKernel2,
                            m_oBKernel.size(), m_oKernelRootSquaredSum,
                            m_oFillOutputBorder,
                            computeStats,
                            outputMin, outputMax,
                            m_oIntegralImage);
                }
                else
                {
                    computeResultImage<byte,double,byte,false>( m_oImageOut, stats,
                            rImage,
                            m_oKernel1, m_oKernel2,
                            m_oBKernel.size(), m_oKernelRootSquaredSum,
                            m_oFillOutputBorder,
                            computeStats,
                            outputMin, outputMax,
                            m_oIntegralImage);

                }
            }

			m_oResultInfo.clear();
			if ( computeStats )
			{
				stats.printResultHistogram(m_oResultInfo, stats.getCount());
			}

			Point outputOffSet = Point((rImage.width() - m_oImageOut.width()) / 2, (rImage.height() - m_oImageOut.height()) / 2);

			m_oSpTrafo = rFrame.context().trafo()->apply(LinearTrafo(outputOffSet));

			ImageContext oCurContext(rFrame.context(), m_oSpTrafo);
			ImageFrame oNewFrame(oCurContext, m_oImageOut, curAnalysisResult);
			ImageFrame oTemplateFrame(oCurContext, m_oBKernel, curAnalysisResult);
			preSignalAction();
			m_oPipeImageFrame.signal(oNewFrame);
			m_oPipeTemplateFrame.signal(oTemplateFrame);
		}


		void CrossCorrelation::paint()
		{

			if ((m_oVerbosity < eLow) || m_oSpTrafo.isNull() || !m_oImageOut.isValid())
			{
				return;
			}

			if ( m_skipPaint )
			{
				return;
			}

			const Trafo & rTrafo(*m_oSpTrafo);
			OverlayCanvas & rCanvas(canvas<OverlayCanvas>(m_oCounter) );
			OverlayLayer & rLayerImage(rCanvas.getLayerImage());
			OverlayLayer & rLayerLine(rCanvas.getLayerLine());
			OverlayLayer & rLayerText(rCanvas.getLayerText());

			//template and result image contour
			int tx = m_oBKernel.width();
			int ty = m_oBKernel.height();
			Rect templateROI(m_oFillOutputBorder ? rTrafo(Point(-tx/2, -ty/2)): rTrafo(Point(-tx, -ty)), m_oBKernel.size());
			rLayerLine.add<OverlayRectangle>(templateROI, Color::Orange());

			Rect actualROI(rTrafo(Point(0, 0)), m_oImageOut.size());
			rLayerLine.add<OverlayRectangle>(actualROI, Color::Orange());
			if (m_oVerbosity >= eHigh)
			{
				//image overlay of result
				const auto		oTitle = OverlayText("Cross-correlation image", Font(), Rect(150, 18), Color::Black());
				rLayerImage.add<OverlayImage>(actualROI.offset(), m_oImageOut, oTitle);
			}


			if (m_oVerbosity > eHigh)
			{
				//image overlay of template

				const auto	oTitle = OverlayText("Template", Font(), Rect(150, 18), Color::Black());
				rLayerImage.add<OverlayImage>(templateROI.offset(), m_oBKernel, oTitle);

				int textpos = 10;
				int textHeight = 15;
				for ( auto && msg : m_oResultInfo )
				{
					rLayerText.add<OverlayText>(msg.c_str(), Font(12), Rect(10, textpos, 300, textHeight), Color::Magenta());
					textpos += textHeight;
				}

			}
		} // paint

	} // namespace filter
} // namespace precitec

