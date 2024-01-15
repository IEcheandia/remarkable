/**
 *  @copyright:		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2013
 *  @file
 *  @brief			Performs morphology operations on a binary image.
 */

// local includes
#include "morphology.h"

#include "image/image.h"				///< BImage
#include "overlay/overlayPrimitive.h"	///< overlay
#include "module/moduleLogger.h"
#include "filter/morphologyImpl.h"
#include <fliplib/TypeToDataTypeImpl.h>

// system includes
#include <cmath>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {


const std::string Morphology::m_oFilterName 		( std::string("Morphology") );
const std::string Morphology::m_oPipeOut1Name		( std::string("MorphologyImageFrameOut") );


Morphology::Morphology() :
	TransformFilter( Morphology::m_oFilterName, Poco::UUID{"82C1EE70-FD56-471f-966C-7B200037052D"} ),
	m_pPipeInImageFrame	( nullptr ),
	m_oPipeOutImgFrame	( this, m_oPipeOut1Name ),
	m_oMorphOp			( eOpening ),
	m_oNbIterations		( 2 )
{
	// Defaultwerte der Parameter setzen
	parameters_.add("MorphOp",			Parameter::TYPE_int,		static_cast<int>(m_oMorphOp));
	parameters_.add("NbIterations",		Parameter::TYPE_UInt32,		static_cast<unsigned int>(m_oNbIterations));

    setInPipeConnectors({{Poco::UUID("D3C37DE7-CEAA-4827-ABB1-DE93C3BEB428"), m_pPipeInImageFrame, "MorphologyImageFrameIn", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("6D0DDB8F-A41D-4095-B215-4AB122EC6487"), &m_oPipeOutImgFrame, m_oPipeOut1Name, 0, ""}});
    setVariantID(Poco::UUID("96BB6C68-4ECD-42a8-8B64-7E89BFDD2D61"));
} // Morphology



/*virtual*/ void Morphology::setParameter() {
	TransformFilter::setParameter();
	m_oMorphOp				= static_cast<MorphOpType>(parameters_.getParameter("MorphOp").convert<int>());
	m_oNbIterations			= parameters_.getParameter("NbIterations");
} // setParameter



/*virtual*/ bool Morphology::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {
	m_pPipeInImageFrame		= dynamic_cast<image_pipe_t*>(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



/*virtual*/ void Morphology::proceed(const void* sender, fliplib::PipeEventArgs& e) {
	poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor
	// get data from frame

	const ImageFrame&		rFrameIn			( m_pPipeInImageFrame->read(m_oCounter) );
	const BImage&			rImageIn			( rFrameIn.data() );
	const Size2d			oSizeImgIn			( rImageIn.size() );

	m_oSpTrafo = rFrameIn.context().trafo();

	// input validity check

	if ( rImageIn.isValid() == false ) {
		ImageFrame				oNewFrame			( rFrameIn.context(), BImage(), rFrameIn.analysisResult() );	// signal null image
		preSignalAction(); m_oPipeOutImgFrame.signal( oNewFrame );						// invoke linked filter(s)

		return; // RETURN
	} // if

    auto& rBinImageOut = m_oBinImageOut[m_oCounter % g_oNbPar];
    rBinImageOut.resize(oSizeImgIn);
    calcMorphology(rImageIn, rBinImageOut, m_oNbIterations); // image processing

	const ImageFrame		oFrameOut(rFrameIn.context(), rBinImageOut, rFrameIn.analysisResult()); // put image into frame
	preSignalAction(); m_oPipeOutImgFrame.signal(oFrameOut);			// invoke linked filter(s)
} // proceed



/*virtual*/ void Morphology::paint() {
	if ((m_oVerbosity < eMedium) || m_oSpTrafo.isNull()) {
		return;
	}

	const Trafo					&rTrafo(*m_oSpTrafo);
	OverlayCanvas				&rCanvas(canvas<OverlayCanvas>(m_oCounter));
	OverlayLayer				&rLayerImage(rCanvas.getLayerImage());

	const auto		oPosition = rTrafo(Point(0, 0));
	const auto		oTitle = OverlayText("Morphology image", Font(), Rect(150, 18), Color::Black());

	rLayerImage.add(new OverlayImage(oPosition, m_oBinImageOut[m_oCounter % g_oNbPar], oTitle));
} // paint



#define USE_FAST_MORPHOLOGY (1) // open32 close32 at 5 iterations ~50% faster (160X300)

void Morphology::calcMorphology(const BImage& p_rImageIn, BImage& p_rImageOut, byte	p_oNbIterations) {
#if USE_FAST_MORPHOLOGY == 1
	if (p_rImageIn.size().width >= 32 && p_rImageIn.size().height >= 32) { // packed computation not possible on small images (at least one int must be filled)
		const Size	oSizePacked(int(std::ceil(double(p_rImageIn.size().width) / 32)), p_rImageIn.size().height);
		U32Image	oImgInPacked(oSizePacked);
		U32Image	oImgOutPacked(oSizePacked);

		bin2ToBin32(p_rImageIn, oImgInPacked); // pack image

		switch (m_oMorphOp) {
			case eOpening:
				opening32(oImgInPacked, oImgOutPacked, p_oNbIterations);
				break;
			case eClosing:
				closing32(oImgInPacked, oImgOutPacked, p_oNbIterations);
				break;
			case eOpeningClosing: {
					U32Image oImgTmp(oImgInPacked.size());

					opening32(oImgInPacked, oImgTmp, p_oNbIterations);
					closing32(oImgTmp, oImgOutPacked, p_oNbIterations);
				} // case
				break;
			default:
				std::ostringstream oMsg;
				oMsg << "No case for switch argument: " << m_oMorphOp;
				wmLog(eWarning, oMsg.str().c_str());
		} // switch

		bin32ToBin2(oImgOutPacked, p_rImageOut); // unpack image

		//printPixelTImage(p_rImageIn, toString(__LINE__).c_str(), 1, 1); // debug
		//printBitTImage(oImgInPacked, p_rImageIn.size().width, toString(__LINE__).c_str(), 1, 1); // debug
		//printBitTImage(oImgOutPacked, p_rImageIn.size().width, toString(__LINE__).c_str(), 1, 1); // debug
		//printPixelTImage(p_rImageOut, toString(__LINE__).c_str(), 2, 2); // debug
	} // if
	else {
		switch (m_oMorphOp) {
			case eOpening:
				opening(p_rImageIn, p_rImageOut, p_oNbIterations);
				break;
			case eClosing:
				closing(p_rImageIn, p_rImageOut, p_oNbIterations);
				break;
			case eOpeningClosing: {
					BImage oImgTmp(p_rImageIn.size());
					opening(p_rImageIn, oImgTmp, p_oNbIterations);
					closing(oImgTmp, p_rImageOut, p_oNbIterations);
				} // case
				break;
			default:
				std::ostringstream oMsg;
				oMsg << "No case for switch argument: " << m_oMorphOp;
				wmLog(eWarning, oMsg.str().c_str());
		} // else
	} // switch


#else
	switch (m_oMorphOp) {
		case eOpening :
			opening(p_rImageIn, p_rImageOut, p_oNbIterations);
		break;
		case eClosing :
			closing(p_rImageIn, p_rImageOut, p_oNbIterations);
		break;
		case eOpeningClosing : {
			BImage oImgTmp	 (p_rImageIn.size() );
			opening(p_rImageIn, oImgTmp, p_oNbIterations);
			closing(oImgTmp, p_rImageOut, p_oNbIterations);
		}
		break;
		default :
			std::ostringstream oMsg;
			oMsg << "No case for switch argument: " << m_oMorphOp;
			wmLog(eWarning, oMsg.str().c_str());
	} // switch

#endif
} // calcMorphology



} // namespace filter
} // namespace precitec
