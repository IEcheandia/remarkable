/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		HS
 * 	@date		2013
 * 	@brief		Filter which computes the gradient of a pore.
 */

// WM includes
#include "poreGradient.h"
#include "direction.h"

#include "fliplib/PipeEventArgs.h"
#include "fliplib/Parameter.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "image/image.h"
#include "module/moduleLogger.h"
#include "filter/algoStl.h"
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace geo2d;
	using namespace interface;
	using namespace image;
namespace filter {


const std::string PoreGradient::m_oFilterName 						( "PoreGradient" );					///< Filter name.
const std::string PoreGradient::m_oPipeOutPoreGradientName 			( "GradientContrastOut" );		///< Pipe: Scalar out-pipe.
const std::string PoreGradient::m_oParamNbNeighboors 				( "NbNeighboors" );				///< Parameter name.
const std::string PoreGradient::m_oParamOuterNeighboorDistance 		( "OuterNeighboorDistance" );	///< Parameter name.


PoreGradient::PoreGradient()
	:
	TransformFilter				( m_oFilterName, Poco::UUID{"BE472B9B-91E3-46f8-BFD5-C19CEBD5009C"} ),
	m_pPipeInImageFrame			( nullptr ),
	m_pPipeInBlob				( nullptr ),
	m_oPipeOutPoreGradient		( this, m_oPipeOutPoreGradientName ),
	m_oNbNeighboors				( 3 ),
	m_oOuterNeighboorDistance	( 2 )

{
	parameters_.add( m_oParamNbNeighboors,				Parameter::TYPE_UInt32,	m_oNbNeighboors );
	parameters_.add( m_oParamOuterNeighboorDistance,	Parameter::TYPE_UInt32,	m_oOuterNeighboorDistance );

    setInPipeConnectors({{Poco::UUID("915C9661-BFA1-40D6-B252-AF3025BEEE8F"), m_pPipeInImageFrame, "GradientImageIn", 1, ""},
    {Poco::UUID("27DD60E8-331B-4A23-95E5-56453B55B787"), m_pPipeInBlob, "GradientBlobsIn", 1, ""}});
    setOutPipeConnectors({{Poco::UUID("20B22C32-12CE-41B5-8CCF-4508283013BA"), &m_oPipeOutPoreGradient, m_oPipeOutPoreGradientName, 0, ""}});
    setVariantID(Poco::UUID("019795CD-8FFD-4dda-88E1-F2627A460222"));
} // PoreGradient



void PoreGradient::setParameter() {
	TransformFilter::setParameter();
	m_oNbNeighboors 			= parameters_.getParameter(m_oParamNbNeighboors).convert<unsigned int>();
	m_oOuterNeighboorDistance 	= parameters_.getParameter(m_oParamOuterNeighboorDistance).convert<unsigned int>();
} // setParameter.



void PoreGradient::paint() {

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
	OverlayLayer				&rLayerText				( rCanvas.getLayerText());
	const auto&					rPoreGradientVector		( m_oPoreGradientOut.getData() );

	auto						oBlobIt					( std::begin(rGeoBlobsIn.ref().getData()) );
	auto						oPoreGradientRankIt		( std::begin(m_oPoreGradientOut.getRank()) );

	for(auto oPoreGradientIt = std::begin(rPoreGradientVector); oPoreGradientIt != std::end(rPoreGradientVector); ++oBlobIt, ++oPoreGradientIt, ++oPoreGradientRankIt) {
		if (*oPoreGradientRankIt == eRankMin) {
			continue;
		}
		std::ostringstream	oMsg;
		oMsg << "Contrast:" << g_oLangKeyUnitNone << ":" << std::setprecision(2) << std::fixed << *oPoreGradientIt;
		rLayerText.add(new OverlayText(oMsg.str(), Font(14), rTrafo(Rect(oBlobIt->xmax, oBlobIt->ymin + 4*15, 200, 20)), Color::Yellow()));  // +4*15 because 5th

	} // for
} // paint



bool PoreGradient::subscribe(BasePipe& p_rPipe, int p_oGroup) {
	if ( p_rPipe.type() == typeid(ImageFrame) ) {
		m_pPipeInImageFrame = dynamic_cast<image_pipe_t*>(&p_rPipe);
	} // if
	else {
		m_pPipeInBlob		= dynamic_cast<blob_pipe_t*>(&p_rPipe);
	} // else

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void PoreGradient::proceedGroup(const void* p_pSender, PipeGroupEventArgs&) {
	poco_assert_dbg(m_pPipeInBlob != nullptr && m_pPipeInImageFrame != nullptr);

	const ImageFrame&			rFrame				( m_pPipeInImageFrame->read(m_oCounter) );
	const GeoBlobarray&			rGeoBlobsIn			( m_pPipeInBlob->read(m_oCounter) );

	m_oSpTrafo	= rFrame.context().trafo();

	if ( rFrame.data().isValid() == false ) {
		GeoDoublearray				oGeoPoreGradientsOut	( rGeoBlobsIn.context(), m_oPoreGradientOut, rGeoBlobsIn.analysisResult(), NotPresent ); // bad rank
		preSignalAction(); m_oPipeOutPoreGradient.signal(oGeoPoreGradientsOut);

		return; // RETURN
	} // if

	calcPoreGradient(rFrame.data(), rGeoBlobsIn.ref(), m_oPoreGradientOut, m_oNbNeighboors, m_oOuterNeighboorDistance);

	GeoDoublearray				oGeoPoreGradientsOut	( rGeoBlobsIn.context(), m_oPoreGradientOut, rGeoBlobsIn.analysisResult(), rGeoBlobsIn.rank() ); // full geo rank, detailed rank in array
	preSignalAction(); m_oPipeOutPoreGradient.signal(oGeoPoreGradientsOut);

} // proceed



/************************************************************************
* Description:  schmeisst alle Konturen raus,                           *
*               die einen zu geringen PoreGradienten besitzen               *
*               (Analysefunktion)                                       *
*                                                                       *
* Parameter:    ci:           eine Kontur                               *
*               v_sourceGrau:   Graubild                                  *
*               iPoreGradient:    untere Grenze f. d. PoreGradienten            *
*               rect:         Umgebung der Pore                         *
************************************************************************/

/*
	PoreGradient: Differenz der Grauwerte der Pixel
			ausserhalb der Kontur "+"
			minus
			innerhalb der Kontur "-"
	Zu den aeusseren Pixeln wird ein Abstand von iAbstandAussen "|" gelassen.

								+
								+ --> iAnzahlNachbarn (hier 3)
								+
								| --> iAbstandAussen (hier 2)
	ausserhalb der Pore         |
	Kontur            --------------------
	innerhalb der Pore          -
								- --> iAnzahlNachbarn (hier 3)
								-

*/

void PoreGradient::calcPoreGradient(
	const BImage&				p_rImageIn,
	const Blobarray&			p_rBlobsIn,
	Doublearray&				p_rPoreGradientOut,
	unsigned int				p_oNbNeighboors,
	unsigned int				p_oOuterNeighboorDistance)
{
	const unsigned int oNbBlobsIn	( p_rBlobsIn.size() );

	p_rPoreGradientOut.assign(oNbBlobsIn, 0, eRankMin);
	if (oNbBlobsIn == 0) {
		return;
	} // if

	//const Trafo		&rTrafo							( *m_oSpTrafo ); // debug
	//OverlayCanvas&	rOverlayCanvas					( canvas<OverlayCanvas>(m_oCounter) ); // debug
	//OverlayLayer		&rLayer							( rOverlayCanvas.getLayer(eLayerMin + 6) ); // debug

	const Range			oValidImgRangeX					( 0, p_rImageIn.size().width - 1 );
	const Range			oValidImgRangeY					( 0,  p_rImageIn.size().height - 1 );
	const auto&			rBlobVector						( p_rBlobsIn.getData() );


	auto				oBlobRankIt						( std::begin(p_rBlobsIn.getRank()) );
	auto				oPoreGradientIt						( std::begin(p_rPoreGradientOut.getData()) );
	auto				oPoreGradientRankIt					( std::begin(p_rPoreGradientOut.getRank()) );
	for(auto oBlobIt = std::begin(rBlobVector); oBlobIt != std::end(rBlobVector); ++oBlobIt, ++oPoreGradientIt, ++oPoreGradientRankIt) {
		const Point			oBoundingBoxStart				( oBlobIt->xmin, oBlobIt->ymin );
		const Point			oBoundingBoxEnd					( oBlobIt->xmax, oBlobIt->ymax );
		const Rect			oBoundingBox					( oBoundingBoxStart, oBoundingBoxEnd );

		if (*oBlobRankIt == eRankMin || oBlobIt->m_oContour.empty() == true) {
			//wmLog(eDebug, "Filter '%s': Empty pore discarded. Size of bounding box: (%iX%i). Bad rank set.\n", m_oFilterName.c_str(),  oBoundingBox.width(), oBoundingBox.height() );
			continue;
		} // if


		Point			oContourPosition				( oBlobIt->startx, oBlobIt->starty );
		Point			oContourPositionIn;
		Point			oContourPositionOut;
		Dir				oDirection						( N );
		Dir				oDirection90In					( N );
		Dir				oDirection90Out					( N );
		unsigned int	oInnerIntensitySum				( 0 );
		unsigned int	oOuterIntensitySum				( 0 );
		unsigned int	oNbInnerPosOnBorder				( 0 );
		unsigned int	oNbOuterPosOnBorder				( 0 );
		const auto		oItContourEnd					( oBlobIt->m_oContour.cend() );

		for (auto oContourIt = oBlobIt->m_oContour.begin(); oContourIt != oItContourEnd - 1; ++oContourIt) { // not empty assertion above
			oDirection			= getDir(*oContourIt, *(oContourIt + 1));
			get90GradDirection(oDirection, oDirection90In, oDirection90Out);
			//std::cout << "Dir: " << g_oDirString[oDirection] << "\tIn: " << g_oDirString[oDirection90In] << "\tOut: " << g_oDirString[oDirection90Out] << "\n";
			oContourPositionIn		=  *oContourIt;
			oContourPositionOut		=  *oContourIt;

			// innere Nachbarn

			Point			oContourPositionInNext;
			for (unsigned int i = 0; i < p_oNbNeighboors; i++) {
				oContourPositionInNext = getNeighborFrom(oContourPositionIn, oDirection90In);
				if (oValidImgRangeX.contains(oContourPositionInNext.x) && oValidImgRangeY.contains(oContourPositionInNext.y)) {
					oContourPositionIn	= oContourPositionInNext;
					oInnerIntensitySum	+= p_rImageIn[oContourPositionIn.y][oContourPositionIn.x];
					//rLayer.add( new  OverlayPoint(rTrafo(oContourPositionIn), Color::Blue() ) ); // debug
				} // if
				else
					++oNbInnerPosOnBorder;
			} // for

			// aeussere Nachbarn

			Point			oContourPositionOutNext;
			for (unsigned int i = 0; i < (p_oOuterNeighboorDistance + p_oNbNeighboors); i++) {
				oContourPositionOutNext = getNeighborFrom(oContourPositionOut, oDirection90Out);
				if (oValidImgRangeX.contains(oContourPositionOutNext.x) && oValidImgRangeY.contains(oContourPositionOutNext.y)) {
					oContourPositionOut = oContourPositionOutNext;
					if (i >= p_oOuterNeighboorDistance) {
						oOuterIntensitySum += p_rImageIn[oContourPositionOut.y][oContourPositionOut.x];
						//rLayer.add( new  OverlayPoint(rTrafo(oContourPositionOut), Color::White() ) ); // debug
					} // if
				} // if
				else if (i >= p_oOuterNeighboorDistance) {
					++oNbOuterPosOnBorder;
				} // else
			}  // for
		} // for

		const unsigned int	oNbContourPoints				( oBlobIt->m_oContour.size() );
		const unsigned int	oNbValidInnerIntensities		( oNbContourPoints * p_oNbNeighboors - oNbInnerPosOnBorder );
		if (oNbValidInnerIntensities != 0) {
			oInnerIntensitySum /= oNbValidInnerIntensities;
		} // if
		else {
			oInnerIntensitySum = 0;
		} // else

		const unsigned int	oNbValidOuterIntensities		( oNbContourPoints * p_oNbNeighboors - oNbOuterPosOnBorder );
		if (oNbValidOuterIntensities != 0) {
			oOuterIntensitySum /= oNbValidOuterIntensities;
		} // if
		else {
			oOuterIntensitySum = 0;
		} // else

		const int	oDiffIntensity							( oOuterIntensitySum - oInnerIntensitySum);
		const unsigned int	oPoreGradientIntensity				( oDiffIntensity >= 0 ? oDiffIntensity : 0 );

		*oPoreGradientIt		= oPoreGradientIntensity;
		*oPoreGradientRankIt	= eRankMax;
	} // for
} // calcPoreGradient



} // namespace filter
} // namespace precitec
