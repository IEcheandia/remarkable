/*!
 *  @copyright:		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2013
 *  @file
 *  @brief			Binarizes an image depending on threshold.
 */

// local includes
#include "binarize.h"

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


const std::string Binarize::m_oFilterName 		( std::string("Binarize") );
const std::string Binarize::m_oPipeOut1Name		( std::string("ImageFrame") );


Binarize::Binarize() :
	TransformFilter( Binarize::m_oFilterName, Poco::UUID{"9DCAFD08-C439-46d7-8849-AEF73E82ED53"} ),
	m_pPipeInImageFrame			( nullptr ),
	m_oPipeOutImgFrame			( this, m_oPipeOut1Name ),
	m_oDistToMeanIntensity		( 128 ),
	m_oComparisonType			( eLess ),
	m_oBinarizeType				( BinarizeType::eGlobal )
{
	// Defaultwerte der Parameter setzen
	parameters_.add( "Threshold",		Parameter::TYPE_UInt32,	static_cast<unsigned int>(m_oDistToMeanIntensity) ); // "DistToMeanIntensity" rename not yet in db key
	parameters_.add("ComparisonType",	Parameter::TYPE_int,	static_cast<int>(m_oComparisonType));
	parameters_.add("BinarizeType",		Parameter::TYPE_int,	static_cast<int>(m_oBinarizeType));

    setInPipeConnectors({{Poco::UUID("2F41D8F0-3952-4ecb-8697-2BB6932E36EC"), m_pPipeInImageFrame, "ImageFrame", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("46803E1C-5D78-48c1-B2A6-ED71AC06DBC6"), &m_oPipeOutImgFrame, "ImageFrame", 0, ""}});
    setVariantID(Poco::UUID("A4227109-F030-4f9c-8AF5-CB0D354CD2DA"));
} // Binarize



/*virtual*/ void Binarize::setParameter() {
	TransformFilter::setParameter();
	m_oDistToMeanIntensity	= parameters_.getParameter("Threshold");
	m_oComparisonType		= static_cast<ComparisonType>(parameters_.getParameter("ComparisonType").convert<int>());
	m_oBinarizeType			= static_cast<BinarizeType>(parameters_.getParameter("BinarizeType").convert<int>());

	poco_assert_dbg(m_oComparisonType	>= eComparisonTypeMin);				// Parameter assertion. Should be pre-checked by UI / MMI / GUI.
	poco_assert_dbg(m_oComparisonType	<= eComparisonTypeMax);				// Parameter assertion. Should be pre-checked by UI / MMI / GUI.
	poco_assert_dbg(m_oBinarizeType		>= BinarizeType::eBinarizeTypeMin);	// Parameter assertion. Should be pre-checked by UI / MMI / GUI.
	poco_assert_dbg(m_oBinarizeType		<= BinarizeType::eBinarizeTypeMax);	// Parameter assertion. Should be pre-checked by UI / MMI / GUI.
} // setParameter



/*virtual*/ bool Binarize::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {
	m_pPipeInImageFrame		= dynamic_cast<image_pipe_t*>(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



/*virtual*/ void Binarize::proceed(const void* sender, fliplib::PipeEventArgs& e) {
	poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor
	// get data from frame

	const ImageFrame&		rFrameIn			( m_pPipeInImageFrame->read(m_oCounter) );
	const BImage&			rImageIn			( rFrameIn.data() );
	const Size2d			oSizeImgIn			( rImageIn.size() );

	m_oSpTrafo	= rFrameIn.context().trafo();

	// input validity check

	if ( rImageIn.isValid() == false ) {
		ImageFrame				oNewFrame			( rFrameIn.context(), BImage(), rFrameIn.analysisResult() );	// signal null image
		preSignalAction(); m_oPipeOutImgFrame.signal( oNewFrame );						// invoke linked filter(s)

		return; // RETURN
	} // if

 	// if the threshold is set to 0, the binarization makes no sense and is therefore disabled - the input image is passed through directly ...
	if ( m_oDistToMeanIntensity == 0 ) {
        auto& rBinImageOut = m_oBinImageOut[m_oCounter % g_oNbPar];
        rBinImageOut.resize(oSizeImgIn);
        auto oWidth = oSizeImgIn.width; auto oHeight = oSizeImgIn.height;
        for (int i=0; i < oHeight; ++i) {
            memcpy( rBinImageOut[i], rImageIn[i], oWidth );
        }
		preSignalAction(); m_oPipeOutImgFrame.signal( rFrameIn );						// invoke linked filter(s)
		return; // RETURN
	} // if

    auto& rBinImageOut = m_oBinImageOut[m_oCounter % g_oNbPar];
    rBinImageOut.resize(oSizeImgIn);

	// image processing

	switch (m_oBinarizeType) {
	case BinarizeType::eGlobal: // global and dynamically
		calcBinarizeDynamic(rImageIn, m_oComparisonType, m_oDistToMeanIntensity, rBinImageOut);
		break;
	case BinarizeType::eLocal: // local and dynamically
		calcBinarizeLocal(rImageIn, m_oComparisonType, m_oDistToMeanIntensity, rBinImageOut);
		break;
	case BinarizeType::eStatic: // global and statically
		calcBinarizeStatic(rImageIn, m_oComparisonType, m_oDistToMeanIntensity, rBinImageOut);
		break;
	default:
		calcBinarizeDynamic(rImageIn, m_oComparisonType, m_oDistToMeanIntensity, rBinImageOut);
		break;
	} // switch

	const ImageFrame		oFrameOut			( rFrameIn.context(), rBinImageOut, rFrameIn.analysisResult() ); // put image into frame
	preSignalAction(); m_oPipeOutImgFrame.signal(oFrameOut);			// invoke linked filter(s)
} // proceed



/*virtual*/ void Binarize::paint() {
	if ((m_oVerbosity < eMedium) || m_oSpTrafo.isNull()) {
		return;
	}

	const Trafo					&rTrafo					( *m_oSpTrafo );
	OverlayCanvas				&rCanvas				( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer				&rLayerImage			( rCanvas.getLayerImage());

	const auto		oPosition	=	rTrafo(Point(0, 0));
	const auto		oTitle		=	OverlayText("Binarized image", Font(), Rect(150, 18), Color::Black());

	rLayerImage.add<OverlayImage>(oPosition, m_oBinImageOut[m_oCounter % g_oNbPar], oTitle);
} // paint




} // namespace filter
} // namespace precitec
