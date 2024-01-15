/*!
 *  @copyright:		Precitec Vision GmbH & Co. KG
 *  @author			JS
 *  @date			2016
 *  @file
 *  @brief			Fliplib filter 'LinearLut' in component 'Filter_ImageProcessing'. process a frame
 *                  through a linear lut with min and max value.
 */

#include "linearLut.h"

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

const std::string LinearLut::m_oFilterName 	 = std::string("LinearLut");
const std::string LinearLut::m_oPipeOutName = std::string("ImageFrame");


LinearLut::LinearLut() :
	TransformFilter( LinearLut::m_oFilterName, Poco::UUID{"BB875831-6CDE-41FC-BA66-EDA9DDF5008A"} ),
	m_pPipeInImageFrame	(NULL),
	m_oPipeOutImgFrame	( this, m_oPipeOutName ),
	m_pFrameIn			(NULL)
{
	// Defaultwerte der Parameter setzen
	parameters_.add("Min", fliplib::Parameter::TYPE_int, m_oMin);
	parameters_.add("Max", fliplib::Parameter::TYPE_int, m_oMax);

    setInPipeConnectors({{Poco::UUID("27747B6A-CBAD-44C6-8CAB-AFE60A6B11FC"), m_pPipeInImageFrame, "ImageFrame", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("80596AD3-6BFA-4C7E-8ABC-759EDDE1B747"), &m_oPipeOutImgFrame, "ImageFrame", 0, ""}});
    setVariantID(Poco::UUID("810680EB-91A1-4922-9360-DD3BCC190CB3"));
} // LinearLut



/*virtual*/ void
LinearLut::setParameter() {
	TransformFilter::setParameter();
	m_oMin	= parameters_.getParameter("Min").convert<int>();
	m_oMax	= parameters_.getParameter("Max").convert<int>();

} // setParameter



/*virtual*/ void
LinearLut::paint() {
	if ((m_oVerbosity < eMedium) || m_oSpTrafo.isNull() ) {
		return;
	}

	const Trafo					&rTrafo					( *m_oSpTrafo );
	OverlayCanvas				&rCanvas				( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer				&rLayerImage			( rCanvas.getLayerImage());

	const auto		oPosition	=	rTrafo(Point(0, 0));
	const auto		oTitle		=	OverlayText("Linear Lut image", Font(), Rect(150, 18), Color::Black());

	rLayerImage.add(new OverlayImage(oPosition, m_oLinearLutImageOut, oTitle));
} // paint



/*virtual*/ bool
LinearLut::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {
	m_pPipeInImageFrame  = dynamic_cast< fliplib::SynchronePipe < ImageFrame > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



/*virtual*/ void
LinearLut::proceed(const void* sender, fliplib::PipeEventArgs& e) {
	poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor

	// get data from frame

	m_pFrameIn				= &( m_pPipeInImageFrame->read(m_oCounter) );
	const BImage &rImageIn	= m_pFrameIn->data();
	const Size2d oSizeImgIn	= rImageIn.size();
	const auto oAnalysisResult	= m_pFrameIn->analysisResult() == AnalysisOK ? AnalysisOK : m_pFrameIn->analysisResult(); // replace 2nd AnalysisOK by your result type

	m_oSpTrafo	= m_pFrameIn->context().trafo();

	// input validity check

	if (rImageIn.isValid() == false) {
		ImageFrame oNewFrame( m_pFrameIn->context(), BImage(), oAnalysisResult );	// signal null image
		preSignalAction(); m_oPipeOutImgFrame.signal( oNewFrame );						// invoke linked filter(s)

		return; // RETURN
	}

	m_oLinearLutImageOut.resize(oSizeImgIn);

	calcLinearLut( rImageIn, m_oMin,m_oMax, m_oLinearLutImageOut ); // image processing

	ImageFrame oNewFrame( m_pFrameIn->context(), m_oLinearLutImageOut, oAnalysisResult ); // put image into frame
	preSignalAction(); m_oPipeOutImgFrame.signal( oNewFrame );			// invoke linked filter(s)
} // proceed



/*static*/ void
LinearLut::calcLinearLut( // actual signal processing
	const BImage		&p_rImageIn,
	const int			&p_min,
	const int           &p_max,
	BImage				&p_rImageOut
)
{
	const int oImgWidth		= p_rImageIn.size().width;
	const int oImgHeight	= p_rImageIn.size().height;


	//calc min max in the frame
	int min = 255;
	int max = 0;
	for (int y = 0; y < oImgHeight; ++y)
	{

		const byte	*pLineInCur = p_rImageIn[y];
		//byte		*pLineOut = p_rImageOut[y];

		for (int x = 1; x < oImgWidth - 1; ++x)
		{
			if (pLineInCur[x] < min)
				min = pLineInCur[x];
			if (pLineInCur[x] > max)
				max = pLineInCur[x];
		} // for

	} // for


	double aa = 0;
	if (max - min >0)
	{
		int maxNeu = 0;
		int minNeu = 255;
		for (int y = 0; y < oImgHeight; ++y)
		{
			const byte	*pLineInCur		= p_rImageIn[y];
			byte		*pLineOut		= p_rImageOut[y];

			for (int x = 0; x < oImgWidth; ++x)
			{
				aa = static_cast<double>(p_max)*  (static_cast<double>(pLineInCur[x] - min)) / (static_cast<double>(max - min));
				pLineOut[x] = static_cast<byte>(p_min + static_cast<int>(aa));

				if (pLineOut[x]>maxNeu)
					maxNeu = pLineOut[x];
				if (pLineOut[x]<minNeu)
					minNeu = pLineOut[x];

			} // for
		} // for

		//wmLog(eInfo,"LUT max,min: %d,%d \n",maxNeu,minNeu);

	}
	else
	{
		//logmeldung
		wmLog(eInfo, "max - min der linear lut   ist <= 0 !\n");
	}

} // calcLinearLut



} // namespace filter
} // namespace precitec
