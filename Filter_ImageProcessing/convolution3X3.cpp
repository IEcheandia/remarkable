/*!
 *  @copyright:		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2011
 *  @file
 *  @brief			Fliplib filter 'Convolution3X3' in component 'Filter_ImageProcessing'. Convolutes an image with a 3X3 filter mask.
 */

#include "convolution3X3.h"

#include <system/platform.h>					///< global and platform specific defines
#include <system/tools.h>						///< poco bugcheck
#include "module/moduleLogger.h"

#include "overlay/overlayPrimitive.h"			///< overlay

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string Convolution3X3::m_oFilterName 	= std::string("Convolution3X3");
const std::string Convolution3X3::m_oPipeOut1Name	= std::string("ImageFrame");


Convolution3X3::Convolution3X3() :
	TransformFilter( Convolution3X3::m_oFilterName, Poco::UUID{"18D1C8F1-9E95-4738-A3E5-B6F20572F88A"} ),
	m_pPipeInImageFrame	(NULL),
	m_oPipeOutImgFrame	( this, m_oPipeOut1Name )
{
	// Defaultwerte der Parameter setzen
	parameters_.add("Coeff1", fliplib::Parameter::TYPE_int, m_oFilterCoeff.m_oCoeff1);
	parameters_.add("Coeff2", fliplib::Parameter::TYPE_int, m_oFilterCoeff.m_oCoeff2);
	parameters_.add("Coeff3", fliplib::Parameter::TYPE_int, m_oFilterCoeff.m_oCoeff3);
	parameters_.add("Coeff4", fliplib::Parameter::TYPE_int, m_oFilterCoeff.m_oCoeff4);
	parameters_.add("Coeff5", fliplib::Parameter::TYPE_int, m_oFilterCoeff.m_oCoeff5);
	parameters_.add("Coeff6", fliplib::Parameter::TYPE_int, m_oFilterCoeff.m_oCoeff6);
	parameters_.add("Coeff7", fliplib::Parameter::TYPE_int, m_oFilterCoeff.m_oCoeff7);
	parameters_.add("Coeff8", fliplib::Parameter::TYPE_int, m_oFilterCoeff.m_oCoeff8);
	parameters_.add("Coeff9", fliplib::Parameter::TYPE_int, m_oFilterCoeff.m_oCoeff9);

    setInPipeConnectors({{Poco::UUID("70C13849-3A64-48b3-83DF-FF40D288940C"), m_pPipeInImageFrame, "ImageFrame", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("F46E2BB8-85E8-4558-8A09-6F9B7A6A61AC"), &m_oPipeOutImgFrame, "ImageFrame", 0, ""}});
    setVariantID(Poco::UUID("689E0E30-B31C-4cd0-9A6E-0DB4559ACE9D"));
} // Convolution3X3



/*virtual*/ void
Convolution3X3::setParameter() {
	TransformFilter::setParameter();
	m_oFilterCoeff.m_oCoeff1	= parameters_.getParameter("Coeff1").convert<int>();
	m_oFilterCoeff.m_oCoeff2	= parameters_.getParameter("Coeff2").convert<int>();
	m_oFilterCoeff.m_oCoeff3	= parameters_.getParameter("Coeff3").convert<int>();
	m_oFilterCoeff.m_oCoeff4	= parameters_.getParameter("Coeff4").convert<int>();
	m_oFilterCoeff.m_oCoeff5	= parameters_.getParameter("Coeff5").convert<int>();
	m_oFilterCoeff.m_oCoeff6	= parameters_.getParameter("Coeff6").convert<int>();
	m_oFilterCoeff.m_oCoeff7	= parameters_.getParameter("Coeff7").convert<int>();
	m_oFilterCoeff.m_oCoeff8	= parameters_.getParameter("Coeff8").convert<int>();
	m_oFilterCoeff.m_oCoeff9	= parameters_.getParameter("Coeff9").convert<int>();

} // setParameter



/*virtual*/ void
Convolution3X3::paint() {
	if ((m_oVerbosity < eMedium) || m_oSpTrafo.isNull() ) {
		return;
	}

	const Trafo					&rTrafo					( *m_oSpTrafo );
	OverlayCanvas				&rCanvas				( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer				&rLayerImage			( rCanvas.getLayerImage());

	const auto		oPosition	=	rTrafo(Point(0, 0));
	const auto		oTitle		=	OverlayText("Convoluted image", Font(), Rect(150, 18), Color::Black());

	rLayerImage.add(new OverlayImage(oPosition, m_oConvolutedImageOut[m_oCounter % g_oNbPar], oTitle));
} // paint



/*virtual*/ bool
Convolution3X3::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {
	m_pPipeInImageFrame  = dynamic_cast< fliplib::SynchronePipe < ImageFrame > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



/*virtual*/ void
Convolution3X3::proceed(const void* sender, fliplib::PipeEventArgs& e) {
	poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor

	// get data from frame

	const auto oFrameIn				=( m_pPipeInImageFrame->read(m_oCounter) );
	const BImage &rImageIn	= oFrameIn.data();
	const Size2d oSizeImgIn	= rImageIn.size();
	const auto oAnalysisResult	= oFrameIn.analysisResult() == AnalysisOK ? AnalysisOK : oFrameIn.analysisResult(); // replace 2nd AnalysisOK by your result type

	m_oSpTrafo	= oFrameIn.context().trafo();

	// input validity check

	if (rImageIn.isValid() == false) {
		ImageFrame oNewFrame( oFrameIn.context(), BImage(), oAnalysisResult );	// signal null image
		preSignalAction(); m_oPipeOutImgFrame.signal( oNewFrame );						// invoke linked filter(s)

		return; // RETURN
	}

    auto& rConvolutedImageOut = m_oConvolutedImageOut[m_oCounter % g_oNbPar];
    rConvolutedImageOut.resize(oSizeImgIn);

	calcConvolution3X3( rImageIn, m_oFilterCoeff, rConvolutedImageOut ); // image processing

	ImageFrame oNewFrame( oFrameIn.context(), rConvolutedImageOut, oAnalysisResult ); // put image into frame
	preSignalAction(); m_oPipeOutImgFrame.signal( oNewFrame );			// invoke linked filter(s)
} // proceed



/*static*/ void
Convolution3X3::calcConvolution3X3( // actual signal processing
	const BImage		&p_rImageIn,
	const FilterCoeff	&p_FilterCoeff,
	BImage				&p_rImageOut
)
{
	const int oImgWidth		= p_rImageIn.size().width;
	const int oImgHeight	= p_rImageIn.size().height;

	const int	C1(p_FilterCoeff.m_oCoeff1), C2(p_FilterCoeff.m_oCoeff2), C3(p_FilterCoeff.m_oCoeff3),
				C4(p_FilterCoeff.m_oCoeff4), C5(p_FilterCoeff.m_oCoeff5), C6(p_FilterCoeff.m_oCoeff6),
				C7(p_FilterCoeff.m_oCoeff7), C8(p_FilterCoeff.m_oCoeff8), C9(p_FilterCoeff.m_oCoeff9);
				// just for shorter names

	std::fill_n(p_rImageOut.upperLeft(), oImgWidth, 0); // fill boundary
	for (int y = 1; y < oImgHeight - 1; ++y) { // exclude boundaries

		const byte	*pLineInPre		= p_rImageIn[y-1];
		const byte	*pLineInCur		= p_rImageIn[y];
		const byte	*pLineInPost	= p_rImageIn[y+1];
		byte		*pLineOut		= p_rImageOut[y];

		pLineOut[0]	= 0; // fill boundary
		for (int x = 1; x < oImgWidth - 1; ++x) { // exclude boundaries
			const int	oRawValue	= std::abs(
				C1 * pLineInPre	[x-1] + C2 * pLineInPre	[x] + C3 * pLineInPre  [x+1] +
				C4 * pLineInCur	[x-1] + C5 * pLineInCur	[x] + C6 * pLineInCur  [x+1] +
				C7 * pLineInPost[x-1] + C8 * pLineInPost[x] + C9 * pLineInPost [x+1]);
			pLineOut[x] = static_cast<byte>( std::min(oRawValue, 255) );	// prevent clipping
		} // for
		pLineOut[oImgWidth - 1]	= 0; // fill boundary
	} // for
	std::fill_n(p_rImageOut.lowerLeft(), oImgWidth, 0); // fill boundary
} // calcConvolution3X3



} // namespace filter
} // namespace precitec
