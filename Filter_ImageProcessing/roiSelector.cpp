/*!
 *  @copyright:		Precitec Vision GmbH & Co. KG
 *  @author			KIR, WoR, HS
 *  @date			2010-2011
 *  @file
 *  @brief			Fliplib filter 'RoiSelector' in component 'Filter_ImageProcessing'. Clips image to roi dimension.
 */

#include "roiSelector.h"

#include "image/image.h"				///< BImage
#include "overlay/overlayPrimitive.h"	///< Overlay
#include "module/moduleLogger.h"
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string ROISelector::m_oFilterName = std::string("ROISelector");
const std::string ROISelector::PIPENAME1	= std::string("ImageFrame");


ROISelector::ROISelector() :
	TransformFilter			( ROISelector::m_oFilterName, Poco::UUID{"D41DBCE0-3CC0-41ad-9B8F-A237EB2E4BCB"} ),
	m_pPipeInImageFrame		( nullptr ),
	m_oPipeOutImageFrame	( this, ROISelector::PIPENAME1 ),
	m_oRoi					( 50, 50, 200, 200 ),
	m_oColor				( Color::Green() )
{
	// Defaultwerte der Parameter setzen

	parameters_.add("X",		Parameter::TYPE_UInt32,	static_cast<unsigned int>( m_oRoi.x().start()) );
	parameters_.add("Y",		Parameter::TYPE_UInt32,	static_cast<unsigned int>( m_oRoi.y().start()) );
	parameters_.add("Width",	Parameter::TYPE_UInt32, static_cast<unsigned int>( m_oRoi.width()) );
	parameters_.add("Height",	Parameter::TYPE_UInt32, static_cast<unsigned int>( m_oRoi.height()) );
	parameters_.add("rgbRed",	Parameter::TYPE_UInt32, static_cast<unsigned int>( m_oColor.red) );
	parameters_.add("rgbGreen", Parameter::TYPE_UInt32, static_cast<unsigned int>( m_oColor.green) );
	parameters_.add("rgbBlue",	Parameter::TYPE_UInt32, static_cast<unsigned int>( m_oColor.blue) );
	parameters_.add("rgbAlpha", Parameter::TYPE_UInt32, static_cast<unsigned int>( m_oColor.alpha) );

    setInPipeConnectors({{Poco::UUID("84F7477C-C9B2-424E-8FB6-95DEE814D250"), m_pPipeInImageFrame, "ImageFrame", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("0D223950-D676-4FD0-B6AC-FC98B1A6C953"), &m_oPipeOutImageFrame, PIPENAME1, 0, ""}});
    setVariantID({Poco::UUID("97A468CF-9909-49b5-AF7B-0F52A9A69751"), Poco::UUID("F8F671B1-05D4-4789-BA67-2AB3B7677739"), Poco::UUID("DAE69563-E2DD-45FE-9279-F436363D2993")}); //Three IDs
}



void ROISelector::setParameter() {
	TransformFilter::setParameter();
	m_oRoi.x().start() = parameters_.getParameter("X").convert<int>();
	m_oRoi.y().start() = parameters_.getParameter("Y").convert<int>();
	m_oRoi.x().end() = m_oRoi.x().start() + parameters_.getParameter("Width").convert<int>();
	m_oRoi.y().end() = m_oRoi.y().start() + parameters_.getParameter("Height").convert<int>();

	m_oRoi.x().start()	= std::abs(m_oRoi.x().start());
	m_oRoi.y().start()	= std::abs(m_oRoi.y().start());
	m_oRoi.x().end()	= std::abs(m_oRoi.x().end());
	m_oRoi.y().end()	= std::abs(m_oRoi.y().end());

	m_oColor.red	= parameters_.getParameter("rgbRed").convert<byte>();
	m_oColor.green	= parameters_.getParameter("rgbGreen").convert<byte>();
	m_oColor.blue	= parameters_.getParameter("rgbBlue").convert<byte>();
	m_oColor.alpha	= parameters_.getParameter("rgbAlpha").convert<byte>();
} // setParameter



/*virtual*/ void
ROISelector::arm (const fliplib::ArmStateBase& state) {
	//std::cout << "\nFilter '" << m_oFilterName << "' armed at armstate " << state.getStateID() << std::endl;
} // arm



void ROISelector::paint() {
	if (m_oVerbosity < eLow || m_oSpTrafo.isNull()){
		return;
	}

	const Trafo		&rTrafo			( *m_oSpTrafo );
	OverlayCanvas	&rCanvas		( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerLine		( rCanvas.getLayerLine());
	// Zeichne Rechteck fuer ROI

	rLayerLine.add<OverlayRectangle>(rTrafo(m_oRoi), m_oColor);
}



using fliplib::SynchronePipe;

bool ROISelector::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {
	m_pPipeInImageFrame  = dynamic_cast< SynchronePipe < ImageFrame > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



using geo2d::Rect;
using geo2d::Size;

/// die eigentliche Filterfunktion
SmpBImage ROISelector::selectRoi(BImage const& p_rImgIn, Rect &p_rRoi) {
		p_rRoi = intersect(Rect( Range( 0, p_rImgIn.size().width ),  Range( 0, p_rImgIn.size().height ) ), p_rRoi);

		if (p_rRoi.isEmpty()) {
			wmLogTr(eWarning, "QnxMsg.Filter.RoiReset", "ROI origin (%i, %i) lies out of image size (%i X %i). ROI origin reset to (0, 0).\n",
				p_rRoi.x().start(), p_rRoi.y().start(), p_rImgIn.size().width,  p_rImgIn.size().height);
			p_rRoi.x().start()	= 0;
			p_rRoi.y().start()	= 0;
		}

		return new BImage(p_rImgIn, p_rRoi); // altes Bild mit neuem ROI - shallow copy
}



using fliplib::PipeEventArgs;

void ROISelector::proceed(const void* sender, PipeEventArgs& e) {
	poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor

	// hole const referenz aus pipe
	const ImageFrame &rFrame = m_pPipeInImageFrame->read(m_oCounter);
	m_oSpTrafo	= rFrame.context().trafo();

	if (rFrame.data().isValid() )
	{
		// die eigentiche Filterfunktion
		//SmpBImage
		oSmpNewImage = selectRoi(rFrame.data(), m_oRoi);

		// Frame-/Contextverwaltung

		// Trafo von globalImage zu subImage
		LinearTrafo subTrafo(offset(m_oRoi));
		// das ROI hat einen neuen Kontext = alter Kontext bis auf die Trafo
		ImageContext oNewContext( rFrame.context(), subTrafo( rFrame.context().trafo() ) );
		const auto oAnalysisResult	= rFrame.analysisResult() == AnalysisOK ? AnalysisOK : rFrame.analysisResult(); // replace 2nd AnalysisOK by your result type
		// Neues ImageFrame versenden (Imageframe wird kopiert -> es bleiben keine Referenzen auf lokale Variable)
        const auto oFrameOut    =   ImageFrame(oNewContext, *oSmpNewImage, oAnalysisResult);
		preSignalAction(); m_oPipeOutImageFrame.signal( oFrameOut );
	} else
	{
        const auto oFrameOut    =   ImageFrame(rFrame.context(), BImage(), rFrame.analysisResult());
		preSignalAction(); m_oPipeOutImageFrame.signal( oFrameOut ); // return zero image
	}

} // proceed



} // namespace filter
} // namespace precitec

