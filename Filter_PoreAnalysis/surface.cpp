/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		HS
 * 	@date		2013
 * 	@brief		Filter which computes the surface of a pore.
 */

// WM includes
#include "surface.h"
#include "direction.h"

#include "fliplib/PipeEventArgs.h"
#include "fliplib/Parameter.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "image/image.h"
#include "module/moduleLogger.h"
#include "filter/algoStl.h"
#include "filter/algoImage.h"
#include <fliplib/TypeToDataTypeImpl.h>


using namespace fliplib;
namespace precitec {
	using namespace geo2d;
	using namespace interface;
	using namespace image;
namespace filter {


const std::string Surface::m_oFilterName 			( "Surface" );			///< Filter name.
const std::string Surface::m_oPipeOutVarianceName 	( "SurfaceVarianceOut" );	///< Pipe: Scalar out-pipe.


Surface::Surface()
	:
	TransformFilter			( m_oFilterName, Poco::UUID{"27563326-A431-4727-8EA8-A891470E955C"} ),
	m_pPipeInImageFrame		( nullptr ),
	m_pPipeInBlob			( nullptr ),
	m_oPipeOutVariance		( this, m_oPipeOutVarianceName ),
	m_oBoundingBoxScale		( 0.4 ),
	m_oAlgoTexture			(AlgoTextureType::eVariance)
{
	parameters_.add("BoundingBoxScale",		Parameter::TYPE_double,		m_oBoundingBoxScale);
	parameters_.add("AlgoTexture",			Parameter::TYPE_int,		static_cast<int>(m_oAlgoTexture));

    setInPipeConnectors({{Poco::UUID("0CBB6003-2C01-4707-A650-B42F9E2B691B"), m_pPipeInImageFrame, "SurfaceImageIn", 1, ""},
    {Poco::UUID("576EE1A0-6753-498d-9F84-674422585AC9"), m_pPipeInBlob, "SurfaceBlobsIn", 1, ""}});
    setOutPipeConnectors({{Poco::UUID("AD989EDA-FAFA-4db3-B987-9B6C3769787C"), &m_oPipeOutVariance, m_oPipeOutVarianceName, 0, ""}});
    setVariantID(Poco::UUID("AD989EDA-FAFA-4db3-B987-9B6C3769787C"));
} // Surface



void Surface::setParameter() {
	using namespace std::placeholders;

	TransformFilter::setParameter();
	m_oBoundingBoxScale	= parameters_.getParameter("BoundingBoxScale");
	m_oAlgoTexture		= static_cast<AlgoTextureType>(parameters_.getParameter("AlgoTexture").convert<int>());

	switch (m_oAlgoTexture) {
	case AlgoTextureType::eVariance:
		m_oAlgorithm = calcVariance;
		break;
	case AlgoTextureType::eMinMaxDistance:
		m_oAlgorithm = calcMinMaxDistance;
		break;
	case AlgoTextureType::eGradientX:
		m_oAlgorithm = calcGradientSumX;
		break;
	case AlgoTextureType::eGradientY:
		m_oAlgorithm = calcGradientSumY;
		break;
	case AlgoTextureType::eMeanIntensity:
		m_oAlgorithm = calcMeanIntensity;
		break;
	default: {
				 m_oAlgorithm = calcVariance;

		std::ostringstream oMsg;
		oMsg << "No case for switch argument: " << int(m_oAlgoTexture);
		wmLog(eDebug, oMsg.str().c_str());
		}
		break;
	} // switch

} // setParameter.



void Surface::paint() {
	if(m_oVerbosity < eMedium){
		return;
	} // if

	if (m_oSpTrafo.isNull())
    {
        return;
    }


	const GeoBlobarray&			rGeoBlobsIn				( m_pPipeInBlob->read(m_oCounter) );
	const Trafo					&rTrafo					( *m_oSpTrafo );
	OverlayCanvas				&rCanvas				( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer				&rLayerLine				( rCanvas.getLayerLine());
	OverlayLayer				&rLayerText				( rCanvas.getLayerText());
	const auto&					rVarianceVector			( m_oVarianceOut.getData() );

	auto						oBlobIt					( std::begin(rGeoBlobsIn.ref().getData()) );
	auto						oVarianceIt				( std::begin(rVarianceVector) );
	auto						oVarianceRankIt			( std::begin(m_oVarianceOut.getRank()) );
	auto						oScaledRectIt			( std::begin(m_oScaledRects) );

	while(oVarianceIt != std::end(rVarianceVector) && oScaledRectIt != std::end(m_oScaledRects)) {
		if (*oVarianceRankIt == eRankMin) {

			++oBlobIt;
			++oVarianceIt;
			++oVarianceRankIt;
			++oScaledRectIt;
			continue;
		}
		std::ostringstream	oMsg;
		oMsg << "Surface: " << std::setprecision(2) << std::fixed << *oVarianceIt;
		rLayerText.add(new OverlayText(oMsg.str(), Font(14), rTrafo(Rect(oBlobIt->xmax, oBlobIt->ymin + 5*15, 200, 20)), Color::Yellow()));  // +5*15 because 6th
		rLayerLine.add(new OverlayRectangle(rTrafo(*oScaledRectIt), Color::White()));

		++oBlobIt;
		++oVarianceIt;
		++oVarianceRankIt;
		++oScaledRectIt;
	} // while
} // paint



bool Surface::subscribe(BasePipe& p_rPipe, int p_oGroup) {
	if ( p_rPipe.type() == typeid(ImageFrame) ) {
		m_pPipeInImageFrame = dynamic_cast<image_pipe_t*>(&p_rPipe);
	} // if
	else {
		m_pPipeInBlob		= dynamic_cast<blob_pipe_t*>(&p_rPipe);
	} // else

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void Surface::proceedGroup(const void* p_pSender, PipeGroupEventArgs&) {
	poco_assert_dbg(m_pPipeInBlob != nullptr && m_pPipeInImageFrame != nullptr);

	const ImageFrame&			rFrame				( m_pPipeInImageFrame->read(m_oCounter) );
	const GeoBlobarray&			rGeoBlobsIn			( m_pPipeInBlob->read(m_oCounter) );

	m_oSpTrafo	= rFrame.context().trafo();

	if ( rFrame.data().isValid() == false ) {
		GeoDoublearray				oGeoVariancesOut	( rGeoBlobsIn.context(), m_oVarianceOut, rGeoBlobsIn.analysisResult(), NotPresent ); // bad rank
		preSignalAction(); m_oPipeOutVariance.signal(oGeoVariancesOut);

		return; // RETURN
	} // if

	calcTexture(rFrame.data(), rGeoBlobsIn.ref(), m_oBoundingBoxScale);

	GeoDoublearray				oGeoVariancesOut	( rGeoBlobsIn.context(), m_oVarianceOut, rGeoBlobsIn.analysisResult(), rGeoBlobsIn.rank() ); // full geo rank, detailed rank in array
	preSignalAction(); m_oPipeOutVariance.signal(oGeoVariancesOut);
} // proceed



void Surface::calcTexture(const BImage& p_rImageIn, const Blobarray& p_rBlobsIn, double p_oBoundingBoxScale) {
	m_oVarianceOut.assign(p_rBlobsIn.size(), 0, eRankMin);
	m_oScaledRects.assign(p_rBlobsIn.size(), Rect());

	const Range			oValidImgRangeX		( 0, p_rImageIn.size().width );
	const Range			oValidImgRangeY		( 0,  p_rImageIn.size().height );
	const auto&			rBlobVector			( p_rBlobsIn.getData() );

	auto				oBlobIt				( std::begin(rBlobVector) );
	auto				oBlobRankIt			( std::begin(p_rBlobsIn.getRank()) );
	auto				oVarianceIt			( std::begin(m_oVarianceOut.getData()) );
	auto				oVarianceRankIt		( std::begin(m_oVarianceOut.getRank()) );
	auto				oScaledRectIt		( std::begin(m_oScaledRects) );

	while (oBlobIt != std::end(rBlobVector)) {
		if (*oBlobRankIt == eRankMin) {

			++oBlobIt;
			++oBlobRankIt;
			++oVarianceIt;
			++oVarianceRankIt;
			++oScaledRectIt;
			continue;
		} // if

		const Point			oBoundingBoxStart	( oBlobIt->xmin, oBlobIt->ymin );
		const Point			oBoundingBoxEnd		( oBlobIt->xmax, oBlobIt->ymax );
		const Rect			oBoundingBox		( oBoundingBoxStart, oBoundingBoxEnd );

		const LinearTrafo	oOffset				( int((1 - m_oBoundingBoxScale) * oBoundingBox.width() / 2), int((1 - m_oBoundingBoxScale) * oBoundingBox.height() / 2) );
		const unsigned int	oNewWidth			( roundToT<unsigned int>(oBoundingBox.width() * m_oBoundingBoxScale) );
		const unsigned int	oNewHeight			( roundToT<unsigned int>(oBoundingBox.height() * m_oBoundingBoxScale) );
		const unsigned int	oNewWidthClipped	( oNewWidth > 0 ? oNewWidth : 1 );
		const unsigned int	oNewHeightClipped	( oNewHeight > 0 ? oNewHeight : 1 );
		const Rect			oScaled			    ( oBoundingBoxStart.x, oBoundingBoxStart.y, oNewWidthClipped , oNewHeightClipped  );
		const Rect			oScaledAndOffset	( oOffset(oScaled) );
		const Rect			oScaledOffCorr		( intersect(oScaledAndOffset, Rect(oValidImgRangeX, oValidImgRangeY)) );

		// calculate texture feature like sample mean (de: stichprobenvarianz)

		const auto			oArea	= 	oScaledAndOffset.sizeWithBorder().area();
		if (oArea <= 1) { // 'area - 1' will be divisor
			if (oArea == 1) {
				*oVarianceIt		= 0;
				*oVarianceRankIt	= 1; // handle degenerated case for very small images. Prevent zero rank to obtain a result still valid in PoreClassificator.
			} // if

			++oBlobIt;
			++oBlobRankIt;
			++oVarianceIt;
			++oVarianceRankIt;
			++oScaledRectIt;
			continue;
		} // if
		const BImage		oPatch				( p_rImageIn, oScaledOffCorr, true );

		*oScaledRectIt		= oScaledOffCorr;
		*oVarianceIt		= m_oAlgorithm(oPatch);
		*oVarianceRankIt	= eRankMax;

		++oBlobIt;
		++oBlobRankIt;
		++oVarianceIt;
		++oVarianceRankIt;
		++oScaledRectIt;
	} // while

} // calcTexture



} // namespace filter
} // namespace precitec
