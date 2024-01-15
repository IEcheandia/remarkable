/***
*	@file
*	@copyright		Precitec Vision GmbH & Co. KG
*	@author			Simon Hilsenbeck (HS)
*	@date			2010
*	@brief			NICHT OPTIMIERTE 2-d Medianberechnung. Randbereich wird ausgelassen. Erzeugt neues Bild selber Groesse.
*/

#include "median.h"

#include "filter/algoStl.h"			///< stl algo
#include "module/moduleLogger.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

#include <array>					///< histogram
#include <limits>					///< byte max value
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {


const std::string	Median::m_oFilterName 	= std::string("Median");
const std::string	Median::PIPENAME	= std::string("ImageFrame");
const unsigned int	Median::MAXGRAYVAL	= std::numeric_limits<byte>::max();
const unsigned int	Median::NGRAYVAL	= MAXGRAYVAL + 1;


/**
 * NICHT OPTIMIERTE 2-d Medianberechnung via lokalem Histogramm
 */

Median::Median() :
	TransformFilter		( Median::m_oFilterName, Poco::UUID{"44351F97-3C3A-4d5b-901E-FD7138DFE996"} ),
	m_pPipeInImageFrame	( nullptr ),
	m_oPipeImageFrame	( this, Median::PIPENAME ),
	m_oFilterRadius		( 1 ), // means filter lenght 3
	m_oHistogram		( NGRAYVAL )
{
	// Defaultwerte der Parameter setzen
	parameters_.add("FilterRadius", fliplib::Parameter::TYPE_UInt32, m_oFilterRadius);

    setInPipeConnectors({{Poco::UUID("7F045631-B637-4FF0-A833-7C3C15004AE2"), m_pPipeInImageFrame, "ImageFrame", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("D21D7431-BEE1-4A94-982F-FFC6AEDF38C2"), &m_oPipeImageFrame, "ImageFrame", 0, ""}});
    setVariantID(Poco::UUID("63AA0E96-E1A7-44b0-950E-CB99299D2846"));
}



void Median::setParameter() {
	TransformFilter::setParameter();
	m_oFilterRadius = parameters_.getParameter("FilterRadius").convert<unsigned int>();
}


bool Median::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeInImageFrame  = dynamic_cast< fliplib::SynchronePipe < ImageFrame > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
}



void Median::proceed(const void* sender, PipeEventArgs& e) {
	poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor

	// Empfangenes Frame auslesen
	const ImageFrame	&rFrame			= m_pPipeInImageFrame->read(m_oCounter);
	const BImage		&rImage			= rFrame.data();

	if (rImage.isValid() == false) {
		ImageFrame oNewFrame( rFrame.context(), BImage(), rFrame.analysisResult());
		preSignalAction(); m_oPipeImageFrame.signal( oNewFrame );

		return; // RETURN
	}

	const int			oImgWidth		= rImage.size().width;
	const int			oImgHeight		= rImage.size().height;
	const int			oFilterLength	= (m_oFilterRadius * 2) + 1;
	const int			oHalfArea		= (oFilterLength * oFilterLength) / 2;
    auto&               rMedianImageOut = m_oMedianImageOut[m_oCounter % g_oNbPar];

	m_oSpTrafo	= rFrame.context().trafo();

	if ( (oFilterLength < rImage.size().width && oFilterLength < rImage.size().height) == false) {
		wmLog(eWarning, "Filter size (%uX%u) must be smaller than image size (%uX%u).", oFilterLength, oFilterLength, rImage.size().width, rImage.size().height);
        preSignalAction();
		return;
	} // if

    rMedianImageOut.resize( Size2D(oImgWidth, oImgHeight) );
	const int	oFilterRadius	= m_oFilterRadius;

	// copy border values
	for (int y = 0; y < oFilterRadius; ++y) {
		std::copy(rImage[y], rImage[y] + oImgWidth, rMedianImageOut[y]);
	} // for

	// for each pixel in image
	for (int y = oFilterRadius; y < oImgHeight - oFilterRadius; ++y) { // exclude boundaries
		byte		*pLineOut		= rMedianImageOut[y];

		// copy border values
		for (int x = 0; x < oFilterRadius; ++x) {
			pLineOut[x]	=	rImage[y][x];
		} // for

		for (int x = oFilterRadius; x < oImgWidth - oFilterRadius; ++x) { // exclude boundaries
			// clear hist
			std::fill(m_oHistogram.begin(), m_oHistogram.end(), 0);
			// for each pixel in filter
			for(int yf = -oFilterRadius; yf < oFilterRadius+1; ++yf) {
				for(int xf = -oFilterRadius; xf < oFilterRadius+1; ++xf) {
					++ m_oHistogram[ rImage[y+yf][x+xf] ];
				} // for
			} // for

			pLineOut[x] = calcMedianHist<unsigned int>(m_oHistogram, oHalfArea);
		} // for

		// copy border values
		for (int x = oImgWidth - oFilterRadius; x < oImgWidth; ++x) {
			pLineOut[x]	=	rImage[y][x];
		} // for
	} // for

	// copy border values
	for (int y = oImgHeight - oFilterRadius; y < oImgHeight; ++y) {
		std::copy(rImage[y], rImage[y] + oImgWidth, rMedianImageOut[y]);
	} // for

	// neues Bild mit altem Kontext
	const auto oAnalysisResult	= rFrame.analysisResult() == AnalysisOK ? AnalysisOK : rFrame.analysisResult(); // replace 2nd AnalysisOK by your result type
	ImageFrame oNewFrame( rFrame.context(), rMedianImageOut, oAnalysisResult );

	preSignalAction(); m_oPipeImageFrame.signal( oNewFrame );

} // proceed



void Median::paint() {
	if ((m_oVerbosity < eMedium) || m_oSpTrafo.isNull()) {
		return;
	}

	const Trafo					&rTrafo(*m_oSpTrafo);
	OverlayCanvas				&rCanvas				( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer				&rLayerImage			( rCanvas.getLayerImage());

	const auto		oPosition	=	rTrafo(Point(0, 0));
	const auto		oTitle		=	OverlayText("Median image", Font(), Rect(150, 18), Color::Black());

	rLayerImage.add(new OverlayImage(oPosition, m_oMedianImageOut[m_oCounter % g_oNbPar], oTitle));
} // paint

} // namespace filter
} // namespace precitec


