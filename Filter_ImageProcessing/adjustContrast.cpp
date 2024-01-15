/*!
 *  @copyright:		Precitec Vision GmbH & Co. KG
 *  @author			LB
 *  @date			2020
 *  @file
 *  @brief			Apply LUT to an image depending on input thresholds.
 */

// local includes
#include "adjustContrast.h"

#include "image/image.h"				///< BImage
#include "overlay/overlayPrimitive.h"	///< overlay
#include "filter/algoImage.h"
#include "module/moduleLogger.h"

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {


const std::string AdjustContrast::m_oFilterName 		( std::string("AdjustContrast") );
const std::string AdjustContrast::m_oPipeOut1Name		( std::string("ImageFrame") );


AdjustContrast::AdjustContrast() :
	TransformFilter(AdjustContrast::m_oFilterName, Poco::UUID{"b334b34a-0742-4f65-9482-994c56d4d4bf"}),
	m_pPipeInImageFrame			( nullptr ),
	m_pPipeInThreshold1          ( nullptr ),
	m_pPipeInThreshold2          ( nullptr ),
	m_oPipeOutImgFrame			( this, m_oPipeOut1Name ),
	m_oOperation (Operation::ApplyLUT)
{
    parameters_.add("Operation",	Parameter::TYPE_int,	static_cast<int>(m_oOperation));

    setInPipeConnectors(
        {{Poco::UUID("dda251b1-8c5b-4b14-b580-4c951d5ea275"), m_pPipeInImageFrame, "Image", 1, "image"},
        {Poco::UUID("534d87d6-7d9e-4145-b52c-5e7f67db34a0"), m_pPipeInThreshold1, "Threshold1", 1, "threshold1"},
        {Poco::UUID("00eeabf5-8c0b-4bdc-9e20-643fc8f650de"), m_pPipeInThreshold2, "Threshold2", 1, "threshold2"}}
    );
    setOutPipeConnectors({{Poco::UUID("6ad38735-0f17-40b7-8080-a40d49728f06"), &m_oPipeOutImgFrame, "ImageFrame", 0, ""}});
    setVariantID(Poco::UUID("8bb6d4b4-c4e0-40fa-86bc-f36fc006f76c"));
}


/*virtual*/ void AdjustContrast::setParameter()
{
	TransformFilter::setParameter();
    m_oOperation = static_cast<Operation>(parameters_.getParameter("Operation").convert<int>());
} // setParameter



/*virtual*/ bool AdjustContrast::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{

    const auto &inPipes = inPipeConnectors();
    if (p_rPipe.tag() == inPipes[0].tag())
    {
        m_pPipeInImageFrame = dynamic_cast<image_pipe_t*>(&p_rPipe);
    }
    else if (p_rPipe.tag() == inPipes[1].tag())
    {
        m_pPipeInThreshold1 = dynamic_cast<scalar_pipe_t*> (&p_rPipe);
    }
    else if (p_rPipe.tag() == inPipes[2].tag())
    {
        m_pPipeInThreshold2 = dynamic_cast<scalar_pipe_t*> (&p_rPipe);
    }
    else
    {
        wmLog(eWarning, "Unknown pipe %d for AdjustContrast \n", p_rPipe.tag().c_str());
        return false;
    }
    return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



/*virtual*/ void AdjustContrast::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e)
{
	poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInThreshold1 != nullptr);
    poco_assert_dbg(m_pPipeInThreshold2 != nullptr);

	// get data from frame

	const ImageFrame& rFrameIn ( m_pPipeInImageFrame->read(m_oCounter) );


	const BImage& rImageIn ( rFrameIn.data() );
	const Size2d oSizeImgIn ( rImageIn.size() );
    const auto & rGeoThreshold1In(m_pPipeInThreshold1->read(m_oCounter).ref());
    const auto & rGeoThreshold2In(m_pPipeInThreshold2->read(m_oCounter).ref());
	// input validity check

	if ( rImageIn.isValid() == false || rGeoThreshold1In.size() == 0  || rGeoThreshold2In.size() == 0)
    {
		ImageFrame oNewFrame ( rFrameIn.context(), BImage(), rFrameIn.analysisResult() ); // signal null image
		preSignalAction();
        m_oPipeOutImgFrame.signal( oNewFrame );
		return;
	}

	if (rGeoThreshold1In.size() > 1 || rGeoThreshold2In.size() > 1)
    {
        wmLog(eDebug, "Filter '%s': Received %u %u input values. Can only process first element, rest will be discarded.\n",
              m_oFilterName.c_str(), rGeoThreshold1In.size(), rGeoThreshold2In.size());
    }

	//ignore ranks

	//input is double, but the threshold in the image is byte
	const auto threshold1Value = static_cast<byte>(std::max(std::min( int(std::round(rGeoThreshold1In.getData()[0])), 255), 0));
    const auto threshold2Value = static_cast<byte>(std::max(std::min( int(std::round(rGeoThreshold2In.getData()[0])), 255), 0));


	m_oSpTrafo	= rFrameIn.context().trafo();

    auto& rBinImageOut = m_oBinImageOut[m_oCounter % g_oNbPar];
    rBinImageOut.resize(oSizeImgIn);

    auto thresholdMin = std::min(threshold1Value, threshold2Value);
    auto thresholdMax = std::max(threshold1Value, threshold2Value);

    switch (m_oOperation)
    {
        case Operation::ApplyLUT:
        case Operation::ApplyLUTAndMaskOutsideThreshold:
            {
                int thresholdDelta = threshold2Value - threshold1Value; // signed so that LUT can be inverted
                switch (thresholdDelta)
                {
                    case 0:
                        rBinImageOut.fill(0);
                        break;
                    case 255:
                        //outputLUT equal to input
                        assert(threshold2Value == 255 && threshold1Value == 0);
                        rBinImageOut = rImageIn;
                        break;
                    default:
                        {
                            auto rescaleIntensity =  [&threshold1Value, &thresholdDelta] (byte pixel) {return 255 * (pixel - threshold1Value)/thresholdDelta;};
                            auto output1 = rescaleIntensity(thresholdMin);
                            auto output2 = (m_oOperation == Operation::ApplyLUTAndMaskOutsideThreshold) ? output1 : rescaleIntensity(thresholdMax);
                            rImageIn.transformTo(rBinImageOut, [&] (byte pixel)
                            { return pixel < thresholdMin ? output1 : (pixel > thresholdMax) ? output2 : rescaleIntensity(pixel);});
                        }
                        break;
                }
            }
            break;
        case Operation::BinarizeBetweenThresholds:
            rImageIn.transformTo(rBinImageOut, [&thresholdMin, &thresholdMax] (byte pixel)
                { return pixel < thresholdMin ? 0 : (pixel > thresholdMax) ? 0 : 255;});
            break;
        case Operation::BinarizeOutsideThresholds:
            rImageIn.transformTo(rBinImageOut, [&thresholdMin, &thresholdMax] (byte pixel)
                { return pixel < thresholdMin ? 255 : (pixel > thresholdMax) ? 255 : 0;});
            break;
        case Operation::Saturate:
            rImageIn.transformTo(rBinImageOut, [&thresholdMin, &thresholdMax] (byte pixel)
                { return pixel < thresholdMin ? thresholdMin : (pixel > thresholdMax) ? thresholdMax : pixel;});
            break;
    }

	const ImageFrame		oFrameOut			( rFrameIn.context(), rBinImageOut, rFrameIn.analysisResult() ); // put image into frame
	preSignalAction(); m_oPipeOutImgFrame.signal(oFrameOut);			// invoke linked filter(s)
} // proceed



/*virtual*/ void AdjustContrast::paint()
{
	if ((m_oVerbosity < eMedium) || m_oSpTrafo.isNull()) {
		return;
	}

	const Trafo					&rTrafo					( *m_oSpTrafo );
	OverlayCanvas				&rCanvas				( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer				&rLayerImage			( rCanvas.getLayerImage());

	const auto		oPosition	=	rTrafo(Point(0, 0));
	const auto		oTitle		=	OverlayText("Adjust Contrast image", Font(), Rect(150, 18), Color::Black());

	rLayerImage.add<OverlayImage>(oPosition, m_oBinImageOut[m_oCounter % g_oNbPar], oTitle);
} // paint



} // namespace filter
} // namespace precitec
