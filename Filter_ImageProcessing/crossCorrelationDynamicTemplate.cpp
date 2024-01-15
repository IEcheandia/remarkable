/***
*	@file
*	@copyright		Precitec Vision GmbH & Co. KG
*	@author			LB
*	@date			2018
*	@brief			Filter to compute the cross-correlation of an image with a pattern, both coming from input pipes
*/

#include "crossCorrelationDynamicTemplate.h"
#include "crossCorrelationImpl.h"
#include "system/tools.h"
#include "module/moduleLogger.h"
#include "overlay/overlayPrimitive.h"
#include "common/bitmap.h"

#include <fliplib/TypeToDataTypeImpl.h>
#include <filter/algoPoint.h>

namespace precitec {
    using namespace interface;
    using namespace image;
    using namespace geo2d;
    namespace filter {
        using namespace crosscorrelation;

        const std::string	CrossCorrelationDynamicTemplate::m_oFilterName = std::string("CrossCorrelationDynamicTemplate");
        const std::string	CrossCorrelationDynamicTemplate::m_oPipeOutImage = std::string("ImageFrame");


        CrossCorrelationDynamicTemplate::CrossCorrelationDynamicTemplate() :
            TransformFilter(CrossCorrelationDynamicTemplate::m_oFilterName, Poco::UUID{"b403c37b-8250-4a2c-a4f2-d7fdb9295ed3"}),
            m_pPipeInImageFrame(nullptr),
            m_oPipeInTemplateFrame(nullptr),
            m_oPipeImageFrame(this, CrossCorrelationDynamicTemplate::m_oPipeOutImage),
            m_oMethod(eCorrelationMethod::CrossCorrelation),
            m_oFillOutputBorder(false),
            m_oRangeInputMinPerc(0.0),
            m_oRangeInputMaxPerc(100.0)
        {
            parameters_.add("Method", fliplib::Parameter::TYPE_int, m_oMethod);

            parameters_.add("FillOutputBorder", fliplib::Parameter::TYPE_bool, m_oFillOutputBorder);
            parameters_.add("RangeInputMinPerc", fliplib::Parameter::TYPE_double, m_oRangeInputMinPerc);
            parameters_.add("RangeInputMaxPerc", fliplib::Parameter::TYPE_double, m_oRangeInputMaxPerc);

            setInPipeConnectors({{Poco::UUID("52d96e2e-1d70-4443-9c83-232401f58e12"), m_pPipeInImageFrame, "image", 1, "image"},
            {Poco::UUID("0ff3e6f1-f296-4f14-be5f-98c22c75f60c"), m_oPipeInTemplateFrame, "template", 1, "template"}});
            setOutPipeConnectors({{Poco::UUID("e19c9894-f5c5-41c9-8a5e-4f7e51c078df"), &m_oPipeImageFrame, "ImageFrame", 0, ""}});
            setVariantID(Poco::UUID("4c0401b1-f666-4fee-8ad8-e6b4991eda9e"));
        }


        void CrossCorrelationDynamicTemplate::setParameter()
        {
            TransformFilter::setParameter();

            //update parameters from WM

            m_oMethod = static_cast<eCorrelationMethod>(parameters_.getParameter("Method").convert<int>());
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
        }


        bool CrossCorrelationDynamicTemplate::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
        {
            if ( p_rPipe.tag() == "image" )
            {
                m_pPipeInImageFrame = dynamic_cast<fliplib::SynchronePipe<ImageFrame> *>(&p_rPipe);
            }
            if ( p_rPipe.tag() == "template" )
            {
                m_oPipeInTemplateFrame = dynamic_cast<fliplib::SynchronePipe<ImageFrame> *>(&p_rPipe);
            }
            return BaseFilter::subscribe(p_rPipe, p_oGroup);
        } // subscribe

        void CrossCorrelationDynamicTemplate::updateKernel(const BImage & pTemplateImage, bool pNormalize)
        {
            if ( !pTemplateImage.isValid() )
            {
                //update m_oKernel1, the rest will be handled by updateKernelVariables()
                m_oKernel1.clear();
            }
            else
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
            updateKernelVariables();
        }



        void CrossCorrelationDynamicTemplate::updateKernelVariables( )
        {
            //make sure m_oKernel2 is not valid, because in this implementation separable kernel is not being used
            m_oKernel2.clear();

            if ( !m_oKernel1.isValid() )
            {
                m_oKernelSize = geo2d::Size(0, 0);
                m_oKernelRootSquaredSum = -1;
                return;
            }

            double kernelSum = 0;
            double kernelSquaredSum = 0;
            double maxValue = 0; //so far we are only using kernels with positive elements
            DImage actualKernel;

            actualKernel = m_oKernel1;
            m_oKernelSize = actualKernel.size();

            if (!actualKernel.isValid())
            {
                m_oKernelRootSquaredSum = -1;
                return;
            }
            assert(actualKernel.end() - actualKernel.begin() == actualKernel.size().area());
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
        }


        void CrossCorrelationDynamicTemplate::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e)
        {
            poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor
            poco_assert_dbg(m_oPipeInTemplateFrame != nullptr); // to be asserted by graph editor


            // Empfangenes Frame auslesen
            const ImageFrame & rFrame = m_pPipeInImageFrame->read(m_oCounter);
            const BImage & rImage = rFrame.data();
            ResultType	curAnalysisResult = rFrame.analysisResult();

            const BImage & rTemplateFrame = m_oPipeInTemplateFrame->read(m_oCounter).data();

            updateKernel(rTemplateFrame, true);

            if ( (m_oKernelSize.area() == 0)
                || !rImage.isValid()
                || (m_oKernelSize.width > rImage.width())
                || (m_oKernelSize.height > rImage.height())
                )
            {
                m_oImageOut.clear();
                //since the output is an empty image, no overlay will be painted
                wmLog(eDebug, "CrossCorrelationDynamicTemplate: image and template size not compatible ([%d %d],[%d %d]) \n",
                    rImage.width(), rImage.height(), m_oKernelSize.width, m_oKernelSize.height
                    );
                ImageFrame oNewFrame( rFrame.context(), m_oImageOut, rFrame.analysisResult() );
                preSignalAction();
                m_oPipeImageFrame.signal( oNewFrame );
                return;
            }

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

            double expectedMax = defaultMin + m_oRangeInputMaxPerc / 100 * defaultRange;
            double expectedMin = defaultMin + m_oRangeInputMinPerc / 100 * defaultRange;
            assert(expectedMin < expectedMax);

            int numberOfBins = 5;
            bool computeStats = m_oVerbosity >= eMedium;
            byte outputMin = 0;
            byte outputMax = 255;
            DescriptiveStats stats(expectedMin, expectedMax, numberOfBins);

            if (oNormalized)
            {
                computeResultImage<byte,double,byte, true>( m_oImageOut, stats,
                        rImage,
                        m_oKernel1, m_oKernel2,
                        m_oKernelSize, m_oKernelRootSquaredSum,
                        m_oFillOutputBorder,
                        computeStats,
                        outputMin, outputMax,
                        m_oIntegralImage);
            }
            else
            {
                computeResultImage<byte,double,byte, false>( m_oImageOut,  stats,
                        rImage,
                        m_oKernel1, m_oKernel2,
                        m_oKernelSize, m_oKernelRootSquaredSum,
                        m_oFillOutputBorder,
                        computeStats,
                        outputMin, outputMax,
                        m_oIntegralImage);
            }

            m_oResultInfo.clear();
            if ( computeStats )
            {
                stats.printResultHistogram(m_oResultInfo, stats.getCount());
                for ( auto && line : m_oResultInfo )
                {
                    wmLog(eInfo, "%s ",  line.c_str());
                }

            }

            Point outputOffSet = Point((rImage.width() - m_oImageOut.width()) / 2, (rImage.height() - m_oImageOut.height()) / 2);

            m_oSpTrafo = rFrame.context().trafo()->apply(LinearTrafo(outputOffSet));

            ImageContext oCurContext(rFrame.context(), m_oSpTrafo);
            ImageFrame oNewFrame(oCurContext, m_oImageOut, curAnalysisResult);
            preSignalAction();
            m_oPipeImageFrame.signal(oNewFrame);
        }

        void CrossCorrelationDynamicTemplate::paint()
        {

            if ((m_oVerbosity < eLow) || m_oSpTrafo.isNull() || !m_oImageOut.isValid())
            {
                return;
            }


            const Trafo & rTrafo(*m_oSpTrafo);
            OverlayCanvas & rCanvas(canvas<OverlayCanvas>(m_oCounter) );
            OverlayLayer & rLayerImage(rCanvas.getLayerImage());
            OverlayLayer & rLayerLine(rCanvas.getLayerLine());

            //template and result image contour
            int tx = m_oKernelSize.width;
            int ty = m_oKernelSize.height;

            Rect templateROI(rTrafo(Point(-tx, -ty)), Size(tx, ty));
            rLayerLine.add<OverlayRectangle>(templateROI, Color::Orange());

            Rect actualROI(rTrafo(Point(0, 0)), m_oImageOut.size());
            rLayerLine.add<OverlayRectangle>(actualROI, Color::Orange());
            if (m_oVerbosity >= eHigh)
            {
                //image overlay of result
                const auto  oPosition = rTrafo(Point(0, 0));
                const auto  oTitle = OverlayText("Cross-correlation image", Font(), Rect(150, 18), Color::Black());
                rLayerImage.add<OverlayImage>(oPosition, m_oImageOut, oTitle);
            }

        } // paint

    } // namespace filter
} // namespace precitec

