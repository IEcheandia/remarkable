/**
 *  @copyright:		Precitec Vision GmbH & Co. KG
 *  @author			JS
 *  @date			2014
 *  @file
 *  @brief			Performs edge detection operations
 */

// local includes
#include "edgeDetection.h"

#include "image/image.h"				///< BImage
#include "overlay/overlayPrimitive.h"	///< overlay
#include "module/moduleLogger.h"

#include "edgeDetectionImpl.h"

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {


const std::string EdgeDetection::m_oFilterName 		( std::string("EdgeDetection") );
const std::string EdgeDetection::m_oPipeOut1Name	( std::string("EdgeDetectionImageFrameOut") );

//void sobel(const BImage& p_rSource, BImage& p_rDestin,int pMode);
//int cannyEdge(const BImage& p_rSource, BImage& p_rDestin, int pMode);

EdgeDetection::EdgeDetection() :
	TransformFilter( EdgeDetection::m_oFilterName, Poco::UUID{"7076849D-1B06-4085-BD5C-3B99A9212445"} ),
	m_pPipeInImageFrame	( nullptr ),
	m_oPipeOutImgFrame	( this, m_oPipeOut1Name ),
	m_oEdgeOp			( eSobel ),  //sobel = 0
	m_oMode      		( 0 )
{
	// Defaultwerte der Parameter setzen
	parameters_.add("EdgeOp",			Parameter::TYPE_int,		static_cast<int>(m_oEdgeOp));
	parameters_.add("Mode",				Parameter::TYPE_int,		static_cast<int>(m_oMode));

    setInPipeConnectors({{Poco::UUID("33ED4FA0-E83A-47E7-ABAD-38FFC7C8DDC3"), m_pPipeInImageFrame, "ImageFrameIn", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("07CE5634-EAF5-48AF-B147-929B1E44E9FA"), &m_oPipeOutImgFrame, "EdgeDetectionImageFrameOut", 0, ""}});
    setVariantID(Poco::UUID("5F1C8B81-852B-408D-A13E-F1C29B64F530"));
} // Morphology


/*virtual*/ void EdgeDetection::setParameter() {
	TransformFilter::setParameter();
	m_oEdgeOp				= static_cast<EdgeOpType>(parameters_.getParameter("EdgeOp").convert<int>());
	m_oMode     			= parameters_.getParameter("Mode");
} // setParameter



/*virtual*/ bool EdgeDetection::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {
	m_pPipeInImageFrame		= dynamic_cast<image_pipe_t*>(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



/*virtual*/ void EdgeDetection::proceed(const void* sender, fliplib::PipeEventArgs& e) {
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

    auto& rResImageOut = m_oResImageOut[m_oCounter % g_oNbPar];
	rResImageOut.resize(rImageIn.size());
	calcEdges(rImageIn, rResImageOut, m_oMode); // image processing

	const ImageFrame		oFrameOut(rFrameIn.context(), rResImageOut, rFrameIn.analysisResult()); // put image into frame
	preSignalAction(); m_oPipeOutImgFrame.signal(oFrameOut);			// invoke linked filter(s)
} // proceed



/*virtual*/ void EdgeDetection::paint() {
	if ((m_oVerbosity <= eNone) || m_oSpTrafo.isNull()) {
		return;
	}

	const Trafo					&rTrafo(*m_oSpTrafo);
	OverlayCanvas				&rCanvas(canvas<OverlayCanvas>(m_oCounter));
	OverlayLayer				&rLayerImage(rCanvas.getLayerImage());

	const auto		oPosition = rTrafo(Point(0, 0));
	const auto		oTitle = OverlayText("Edge image", Font(), Rect(150, 18), Color::Black());

	rLayerImage.add(new OverlayImage(oPosition, m_oResImageOut[m_oCounter % g_oNbPar], oTitle));
} // paint





void EdgeDetection::calcEdges(const BImage& p_rImageIn, BImage& p_rImageOut, int p_oMode) {



		switch (m_oEdgeOp) {

		case eRoberts:
				roberts(p_rImageIn, p_rImageOut, p_oMode);
				break;

		case eSobel:
				sobel(p_rImageIn, p_rImageOut,p_oMode);
				break;

			case eKirsch:
				kirsch(p_rImageIn, p_rImageOut,p_oMode);
				break;

			case eCanny:
				cannyEdgeCv(p_rImageIn, p_rImageOut, p_oMode);
				break;

			default:
				std::ostringstream oMsg;
				oMsg << "No case for switch argument: " << m_oEdgeOp;
				wmLog(eWarning, oMsg.str().c_str());
		} // switch



} // calcEdges



} // namespace filter
} // namespace precitec
