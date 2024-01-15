/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		HS
 * 	@date		2013
 * 	@brief		Filter which computes the surface of a pore.
 */

// WM includes

#include "tileFeature.h"

#include "fliplib/Parameter.h"
#include "overlay/overlayPrimitive.h"
#include "image/image.h"
#include "module/moduleLogger.h"
#include "filter/algoImage.h"
#include "filter/algoStl.h"
#include "system/typeTraits.h"
#include "crossCorrelationImpl.h"
#include <fliplib/TypeToDataTypeImpl.h>

// stdlib includes

#include <algorithm>

namespace {
    using namespace precitec::image;

    template<int nbins>
    std::pair<std::array<int, nbins>, int> calcDirectionality(const BImage& p_rImageIn )
    {
        using namespace precitec::filter::crosscorrelation;
        unsigned int numAngles = 0;
        std::array<int, nbins> angleHistogram{};
        for (int i = 0; i < nbins; i++)
        {
            angleHistogram[i] = 0;
        }

        if (p_rImageIn.width() < 3 || p_rImageIn.height() < 3)
        {
            return {angleHistogram, numAngles};
        }

        DImage oCCorrTemp,kernelH,kernelV;
        auto computeGradient = [&oCCorrTemp, &kernelH, & kernelV, &p_rImageIn](bool horizontalGradient )
        {
            DImage gradient;
            double kernelSum = 0;
            calcSobelKernel( horizontalGradient, kernelH, kernelV, kernelSum);

            calcConvolution(p_rImageIn, kernelH, oCCorrTemp);
            calcConvolution(oCCorrTemp, kernelV, gradient);
            return gradient;
        };

        DImage gradientX = computeGradient(true);
        DImage gradientY = computeGradient(false);

        assert(gradientX.size() == gradientY.size());
        assert(gradientX.isContiguos());
        assert(gradientY.isContiguos());

        //the angles will be computed in the range -pi/2 + pi/2
        constexpr double startingAngle = -M_PI_2;
        constexpr double range_angle_rad = M_PI;

        numAngles = gradientX.size().area();
        auto itX = gradientX.begin();
        auto itY = gradientY.begin();

        for (unsigned int i = 0; i< numAngles; i++, itX++, itY++)
        {
            auto angle_rad = std::atan2(*itY, *itX);// range: -pi + pi
            if (angle_rad > M_PI_2)
            {
                angle_rad -= M_PI;
            }
            else if (angle_rad < - M_PI_2)
            {
                angle_rad += M_PI;
            }
            assert(angle_rad >= startingAngle && angle_rad <= (startingAngle+range_angle_rad));

            auto bin = (angle_rad - startingAngle) * nbins / (range_angle_rad+1e-6);

            assert(bin >= 0 && bin < nbins);
            angleHistogram[bin]++;
        }
        assert(itX == gradientX.end());
        assert(itY == gradientY.end());
        return {angleHistogram, numAngles};
    };

    byte calcFrequencyMainDirection(const BImage& p_rImageIn )
    {
        constexpr int nbins = 10;
        auto result = calcDirectionality<nbins>(p_rImageIn);
        auto & angleHistogram = result.first;
        auto & numAngles = result.second;
        if (numAngles == 0)
        {
            return 0;
        }
        int peakIndex = std::distance(angleHistogram.begin(), std::max_element(angleHistogram.begin(), angleHistogram.end()));
        auto maxCount= angleHistogram[peakIndex];
        return maxCount * 255 / numAngles;
    };

    byte calcMainDirection(const BImage& p_rImageIn )
    {
        constexpr int nbins = 180;
        auto result = calcDirectionality<nbins>(p_rImageIn);
        auto & angleHistogram = result.first;

        int peakIndex = std::distance(angleHistogram.begin(), std::max_element(angleHistogram.begin(), angleHistogram.end()));
        return peakIndex * 255 / nbins;
    };

}

using namespace fliplib;
namespace precitec {
	using namespace geo2d;
	using namespace interface;
	using namespace image;
namespace filter {


const std::string TileFeature::m_oFilterName 				( "TileFeature" );		///< Filter name.
const std::string TileFeature::m_oPipeOutFeatureImageName 	( "FeatureImageOut" );	///< Pipe: Scalar out-pipe.


TileFeature::TileFeature()
	:
	TransformFilter			( m_oFilterName, Poco::UUID{"7442BAA4-E92C-4928-8DB7-9EBBF386C61B"} ),
	m_pPipeInImageFrame		( nullptr ),
	m_oPipeOutFeatureImage	( this, m_oPipeOutFeatureImageName ),
	m_oDrawThreshold		( 10 ),
	m_oTileSize				( 100 ),
	m_oJumpingDistance		( m_oTileSize ),
	m_oAlgoTexture			((int)(AlgoTextureType::eVariance))
{
	parameters_.add("TileSize",			Parameter::TYPE_UInt32,		m_oTileSize);
	parameters_.add("JumpingDistance",	Parameter::TYPE_UInt32,		m_oJumpingDistance);
	parameters_.add("AlgoTexture",		Parameter::TYPE_int,		static_cast<int>(m_oAlgoTexture));
	parameters_.add("DrawThreshold",	Parameter::TYPE_UInt32,		m_oDrawThreshold);

    setInPipeConnectors({{Poco::UUID("C07F4DDC-60C0-4F57-AE54-E8C6DA0E2370"), m_pPipeInImageFrame, "FeatureImageIn", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("58BDDAA5-474E-469D-9AA9-8D02A9CB3BE7"), &m_oPipeOutFeatureImage, m_oPipeOutFeatureImageName, 0, ""}});
    setVariantID(Poco::UUID("8680F91B-52EE-4093-B345-F9FE84AD9CE5"));
} // TileFeature



void TileFeature::setParameter() {
	using namespace std::placeholders;

	TransformFilter::setParameter();
	m_oTileSize			= parameters_.getParameter("TileSize");
	m_oJumpingDistance	= parameters_.getParameter("JumpingDistance");
	m_oAlgoTexture		= parameters_.getParameter("AlgoTexture").convert<int>();
	m_oDrawThreshold	= parameters_.getParameter("DrawThreshold");

} // setParameter.



bool TileFeature::subscribe(BasePipe& p_rPipe, int p_oGroup) {
	m_pPipeInImageFrame = dynamic_cast<const image_pipe_t*>(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void TileFeature::proceed(const void* p_pSender, PipeEventArgs& p_rEvent) {
	poco_assert_dbg(m_pPipeInImageFrame != nullptr);

	const ImageFrame&		rFrameIn			( m_pPipeInImageFrame->read(m_oCounter) );
	const BImage&			rImageIn			( rFrameIn.data() );
	const Size2d			oSizeImgIn			( rImageIn.size() );

	// input validity check

	const unsigned int			oMinImgSize			= static_cast<unsigned int>( std::min(oSizeImgIn.width, oSizeImgIn.height) );
	if ( rImageIn.isValid() == false || m_oJumpingDistance > m_oTileSize || m_oJumpingDistance == 0) {
		m_oFrameFeatureImageOut	= ImageFrame	( rFrameIn.context(), BImage(), rFrameIn.analysisResult() );
		preSignalAction(); m_oPipeOutFeatureImage.signal(m_oFrameFeatureImageOut);						// invoke linked filter(s)

		return; // RETURN
	} // if

    // clip tile size to smaller input dimension if tiles very big

    if (m_oTileSize > oMinImgSize)
    {
        auto    oDiffToImgSize      =   m_oTileSize - oMinImgSize;
        m_oTileSize         =   std::max(1u, m_oTileSize         - oDiffToImgSize);  /*using 1-pixel tiles can still be useful for thresholding operations*/
    }

    // select image and calculate the feature for each tile

    auto& rFeatureImageOut = m_oFeatureImageOut[m_oCounter % g_oNbPar]; // size adjusted within algo

    if (m_oTileSize == 1 && m_oJumpingDistance == 1)
    {
        rFeatureImageOut = rImageIn;
    }
    else
    {

        switch (m_oAlgoTexture)
        {
        case (int) AlgoTextureType::eVariance:
            calcFeatureImg(rImageIn, rFeatureImageOut, m_oTileSize, m_oJumpingDistance, calcVariance);
            break;
        case (int) AlgoTextureType::eMinMaxDistance:
            calcFeatureImg(rImageIn, rFeatureImageOut, m_oTileSize, m_oJumpingDistance, calcMinMaxDistance);
            break;
        case (int) AlgoTextureType::eGradientX:
            calcFeatureImg(rImageIn, rFeatureImageOut, m_oTileSize, m_oJumpingDistance, calcGradientSumX);
            break;
        case (int) AlgoTextureType::eGradientY:
            calcFeatureImg(rImageIn, rFeatureImageOut, m_oTileSize, m_oJumpingDistance, calcGradientSumY);
            break;
        default:
            wmLog(eWarning, "Unkwnown value %d for parameter AlgoTexture \n", m_oAlgoTexture);
            //FALLTHROUGH
        case (int) AlgoTextureType::eMeanIntensity:
            calcFeatureImg(rImageIn, rFeatureImageOut, m_oTileSize, m_oJumpingDistance, calcMeanIntensity);
            break;
        case (int) AlgoTextureType::eMinIntensity:
            calcFeatureImg(rImageIn, rFeatureImageOut, m_oTileSize, m_oJumpingDistance, calcMinIntensity);
            break;
        case (int) AlgoTextureType::eMaxIntensity:
            calcFeatureImg(rImageIn, rFeatureImageOut, m_oTileSize, m_oJumpingDistance, calcMaxIntensity);
            break;
        case 7:
            calcFeatureImg(rImageIn, rFeatureImageOut, m_oTileSize, m_oJumpingDistance, calcFrequencyMainDirection);
            break;
        case 8:
            calcFeatureImg(rImageIn, rFeatureImageOut, m_oTileSize, m_oJumpingDistance, calcMainDirection);
            break;
        } // switch
    }

    // update sampling factors in context
    auto    oNewContext     =   rFrameIn.context();
    oNewContext.SamplingX_  =   static_cast<double>(rFeatureImageOut.width()) / rImageIn.width();
    oNewContext.SamplingY_  =   static_cast<double>(rFeatureImageOut.height()) / rImageIn.height();

	m_oFrameFeatureImageOut	= ImageFrame	( oNewContext, rFeatureImageOut, rFrameIn.analysisResult() ); // put image into frame
	preSignalAction(); m_oPipeOutFeatureImage.signal(m_oFrameFeatureImageOut);			// invoke linked filter(s)
} // proceed


template <typename AlgoTexture>
void TileFeature::calcFeatureImg(const BImage& p_rImageIn, BImage& p_rFeatureImgOut, unsigned int p_oTileSize, unsigned int p_oJumpingDistance, AlgoTexture p_oAlgorithm) {
	using std::numeric_limits;

	const unsigned int		oImgInWidth					( static_cast<unsigned int>( p_rImageIn.size().width ) );
	const unsigned int		oImgInHeight				( static_cast<unsigned int>( p_rImageIn.size().height ) );
	const unsigned int		oDiffTileJump				( p_oTileSize - p_oJumpingDistance );	// p_oJumpingDistance <= p_oTileSize asserted
	const unsigned int		oDiffImgWidthTileJump		( oImgInWidth - oDiffTileJump );		// p_oJumpingDistance <= p_oTileSize asserted
	const unsigned int		oDiffImgHeightTileJump		( oImgInHeight - oDiffTileJump );		// p_oJumpingDistance <= p_oTileSize asserted
	const Size2d			oSizeFeatureImgOut			( oDiffImgWidthTileJump / p_oJumpingDistance, oDiffImgHeightTileJump / p_oJumpingDistance ); // integer div ok

	p_rFeatureImgOut.resize(oSizeFeatureImgOut);	// does nothing if new size equal current size
    if (!p_rFeatureImgOut.isValid())
    {
        return;
    }

    // calculate offsets for centering the tiles within theinput image

    const auto  oOffsetXIn  =   (oDiffImgWidthTileJump  % p_oJumpingDistance) / 2/*int division ok*/;
    const auto  oOffsetYIn  =   (oDiffImgHeightTileJump % p_oJumpingDistance) / 2/*int division ok*/;
    m_oOffsetFirstTile      =   Size2D( oOffsetXIn, oOffsetYIn );

    // calc feature values and min max feature value

	unsigned int	oYIn	= oOffsetYIn;
	for (int oYOut = 0; oYOut < oSizeFeatureImgOut.height; ++oYOut) { // rows
		byte*			pLineOut		= p_rFeatureImgOut[oYOut];
		unsigned int	oXIn			= oOffsetXIn;

		for (int oXOut = 0; oXOut < oSizeFeatureImgOut.width; ++oXOut) { // columns
			const Rect			oTileRoi			( oXIn, oYIn, p_oTileSize, p_oTileSize );
			const BImage		oTileImgIn			( p_rImageIn, oTileRoi , true);

			const double		oFeature			( p_oAlgorithm(oTileImgIn) );

			pLineOut[oXOut]	=  std::min(std::max(int( oFeature ), 0), 255); // prevent clipping
			// we loose double precision - more or less ok if algorithm out range lies within 8 bit, problem: variance

			oXIn += p_oJumpingDistance;
		} // for

		oYIn += p_oJumpingDistance;
	} //
} // calcFeatureImg



void TileFeature::paint() {
	if (m_oVerbosity < eLow || m_oFrameFeatureImageOut.context().trafo().isNull()){
		return;
	} // if

	if(m_oFrameFeatureImageOut.data().isValid() == false){
		return;
	} // if

	const BImage&			rFeatureImgOut(m_oFrameFeatureImageOut.data());
	const Size2d			oSizeFeatureImgOut(rFeatureImgOut.size());
	const Trafo&			rTrafo(*m_oFrameFeatureImageOut.context().trafo());

	OverlayCanvas&			rOverlayCanvas		= canvas<OverlayCanvas>(m_oCounter);
	OverlayLayer&			rLayerLine			= rOverlayCanvas.getLayerLine();
	OverlayLayer&			rLayerText			= rOverlayCanvas.getLayerText();
	OverlayLayer&			rLayerImage			= rOverlayCanvas.getLayerImage();
	OverlayLayer&			rLayerContour		= rOverlayCanvas.getLayerContour();

	const auto		oPositionFeatureImage = rTrafo(Point(0, 0));
	const auto		oTitleFeatureImage = OverlayText("Feature image", Font(), Rect(150, 18), Color::Black());

	rLayerImage.add<OverlayImage>(oPositionFeatureImage, m_oFeatureImageOut[m_oCounter % g_oNbPar], oTitleFeatureImage);

	if (m_oVerbosity < eMedium){
		return;
	} // if


	unsigned int	oYIn = m_oOffsetFirstTile.height;
	for (int oYOut = 0; oYOut < oSizeFeatureImgOut.height; ++oYOut) { // rows
		const byte*			pLineOut = rFeatureImgOut[oYOut];
		unsigned int	oXIn = m_oOffsetFirstTile.width;

		for (int oXOut = 0; oXOut < oSizeFeatureImgOut.width; ++oXOut) { // columns
			const Color			oColor(pLineOut[oXOut] > m_oDrawThreshold ? Color::Green() : Color::Red());

			// depending on jumping distance choose drawing style

			if (m_oJumpingDistance < 11) { // do only paint dots if grid very small (performance).
				const Point			oPosCenter(oXIn + m_oTileSize / 2, oYIn + m_oTileSize / 2);
				rLayerContour.add<OverlayPoint>(rTrafo(oPosCenter), oColor);
			} // if
			else if (m_oJumpingDistance < 20) { // do only paint numbers if grid small (performance).
				const Rect			oTileRoi(oXIn, oYIn, m_oTileSize, m_oTileSize);
				rLayerText.add<OverlayText>(std::to_string(int(pLineOut[oXOut])), Font(8), rTrafo(oTileRoi), oColor);
			} // if
			else {	// paint rectangles and numbers
				const Rect			oTileRoi(oXIn, oYIn, m_oTileSize, m_oTileSize);
				const Font			oFont(m_oTileSize >= 30 ? 14 : 10);
				rLayerLine.add<OverlayRectangle>(rTrafo(oTileRoi), oColor);
				rLayerText.add<OverlayText>(std::to_string(int(pLineOut[oXOut])), oFont, rTrafo(oTileRoi), oColor);
			} // else

			oXIn += m_oJumpingDistance;
		} // for

		oYIn += m_oJumpingDistance;
	} // for

} // paint



} // namespace filter
} // namespace precitec
