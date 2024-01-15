/*!
*  @copyright		Precitec Vision GmbH & Co. KG
*  @author			Simon Hilsenbeck (HS)
*  @date			2011
*  @file
*  @brief			Fliplib filter 'Maximum' in component 'Filter_SeamSearch'. Calculates right and left seam position.
*/


#include "maximum.h"

#include "seamSearch.h"								///< input check, rank calculation

#include "system/types.h"							///< typedefs
#include "common/defines.h"							///< debug assert integrity assumptions
#include "module/moduleLogger.h"

#include "overlay/overlayPrimitive.h"				///< paint overlay
#include "image/image.h"							///< BImage

#include "filter/algoArray.h"						///< Intarray algo
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace image;
	using namespace interface;
	using namespace geo2d;
namespace filter {


const std::string Maximum::m_oFilterName 	= std::string("Maximum");
const std::string Maximum::m_oPipeOutName1	= std::string("ContourLeft");
const std::string Maximum::m_oPipeOutName2	= std::string("ContourRight");


Maximum::Maximum() :
TransformFilter				( Maximum::m_oFilterName, Poco::UUID{"7D8D1473-AEE3-476e-9F18-BCE626F56AD4"} ),
	m_pPipeInGradientLeft		( nullptr ),
	m_pPipeInGradientRight		( nullptr ),
	m_pPipeInMaxFLenght			( nullptr ),
	m_pPipeInImgSize			( nullptr ),
	m_oPipeOutContourLeft		( this, m_oPipeOutName1 ),
	m_oPipeOutContourRight		( this, m_oPipeOutName2 ),
	m_oContourLeftOut			( 1 ), // fixed 1 here ok
	m_oContourRightOut			( 1 ), // fixed 1 here ok
	m_oMaxFilterLenght			( 20 )
{
	// Defaultwerte der Parameter setzen
    setInPipeConnectors({{Poco::UUID("8cfa27b2-9294-4d3f-8e68-765a2eb92bec"), m_pPipeInGradientLeft, "Line", 1, "gradient_left"},
    {Poco::UUID("98c81f22-6648-4268-9697-6756fdfb2792"), m_pPipeInGradientRight, "Line", 1, "gradient_right"},
    {Poco::UUID("64821794-a74e-4e78-810a-f09458f89bc3"), m_pPipeInMaxFLenght, "MaxFilterLength", 1, "max_filter_length"},
    {Poco::UUID("607061fb-bc61-42ae-af63-23b1f5db313e"), m_pPipeInImgSize, "ImageSize", 1, "image_size"}});
    setOutPipeConnectors({{Poco::UUID("f3c6657f-f91b-4fe6-a8ba-68466392e3da"), &m_oPipeOutContourLeft, m_oPipeOutName1, 0, ""},
    {Poco::UUID("AEBAE7C6-6169-4d38-92E5-0F6460BA3317"), &m_oPipeOutContourRight, m_oPipeOutName2, 0, ""}});
    setVariantID(Poco::UUID("2a8c9e04-445b-4a15-8dc8-e09f0709a178"));
} // Maximum



void Maximum::setParameter() {
	TransformFilter::setParameter();
} // setParameter


void Maximum::paint() {
	if(m_oVerbosity <= eNone) {
		return;
	} // if

	// paint contour points

	const GeoVecDoublearray &rGradientLeftIn	= m_pPipeInGradientLeft->read(m_oCounter);
	const unsigned int	oNProfiles			= rGradientLeftIn.ref().size();

	interface::SmpTrafo oSmpTrafo( rGradientLeftIn.context().trafo() );
    if (oSmpTrafo.isNull())
    {
        return;
    }
	OverlayCanvas& rCanvas = canvas<OverlayCanvas>(m_oCounter);
	OverlayLayer&				rLayerPosition			( rCanvas.getLayerPosition());
	OverlayLayer&				rLayerText				( rCanvas.getLayerText());

	for (unsigned int sliceN = 0; sliceN < oNProfiles; ++sliceN) { // loop over N profiles
		const Point oPositionLeft( int(m_oContourLeftOut.getData()[sliceN]), 100 );
		std::stringstream oTmpL; oTmpL << m_oContourLeftOut.getRank()[sliceN];  // draw first contour only
		rLayerPosition.add( new  OverlayCross((*oSmpTrafo)(oPositionLeft), Color::Yellow() ) );
		rLayerText.add( new OverlayText(oTmpL.str(), image::Font(), (*oSmpTrafo)(Rect(oPositionLeft.x, oPositionLeft.y, 30, 20)), Color::Yellow() ) );

		const Point oPositionRight( int(m_oContourRightOut.getData()[sliceN]), 100 );
		std::stringstream oTmpR; oTmpR << m_oContourRightOut.getRank()[sliceN]; // draw first contour only
		rLayerPosition.add( new  OverlayCross((*oSmpTrafo)(oPositionRight), Color::Magenta() ) );
		rLayerText.add( new OverlayText(oTmpR.str(), image::Font(), (*oSmpTrafo)(Rect(oPositionRight.x, oPositionRight.y, 30, 20)), Color::Magenta() ) );
	}
} // paint


bool Maximum::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {
	if (p_rPipe.tag() == "gradient_left") {
		m_pPipeInGradientLeft  = dynamic_cast< line_pipe_t* >(&p_rPipe);
	}
	else if (p_rPipe.tag() == "gradient_right") {
		m_pPipeInGradientRight  = dynamic_cast< line_pipe_t* >(&p_rPipe);
	}
	else if (p_rPipe.tag() == "max_filter_length") {
		m_pPipeInMaxFLenght = dynamic_cast< scalar_pipe_t* >(&p_rPipe);
	}
	else if (p_rPipe.tag() == "image_size") {
		m_pPipeInImgSize		= dynamic_cast< scalar_pipe_t* >(&p_rPipe);
	}

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe


void Maximum::proceedGroup(const void* sender, PipeGroupEventArgs& e) {

	poco_assert_dbg(m_pPipeInGradientLeft != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInGradientRight != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInMaxFLenght != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInImgSize != nullptr); // to be asserted by graph editor


	// get data from frame

	const GeoVecDoublearray rGradientLeftIn		= m_pPipeInGradientLeft->read(m_oCounter);
	const GeoVecDoublearray rGradientRightIn	= m_pPipeInGradientRight->read(m_oCounter);
	const GeoDoublearray	rGeoImgSizeIn		= m_pPipeInImgSize->read(m_oCounter);
	const auto&				rImgSizeIn			= rGeoImgSizeIn.ref().getData();

	// input validity check

	if ( inputIsInvalid( rGradientLeftIn) || inputIsInvalid(rGradientRightIn ) || inputIsInvalid(rGeoImgSizeIn) || rGradientLeftIn.ref().size() != rGradientRightIn.ref().size() ) {
		const GeoDoublearray oGeoSeamPosLeftOut		( rGradientLeftIn.context(), m_oContourLeftOut, rGradientLeftIn.analysisResult(), 0.0 ); // bad rank
		const GeoDoublearray oGeoSeamPosRightOut	( rGradientRightIn.context(), m_oContourRightOut, rGradientRightIn.analysisResult(), 0.0 ); // bad rank

		preSignalAction();
		m_oPipeOutContourLeft.	signal( oGeoSeamPosLeftOut );
		m_oPipeOutContourRight.	signal( oGeoSeamPosRightOut );

		return; // RETURN
	}

	m_oImageSize.width		= int( rImgSizeIn[0] );
	m_oImageSize.height		= int( rImgSizeIn[1] );
	m_oMaxFilterLenght							= int( m_pPipeInMaxFLenght->read(m_oCounter).ref().getData().front() );
	poco_assert_dbg( ! rGradientLeftIn.ref().empty() );

	reinitialize( rGradientLeftIn.ref(), rGradientRightIn.ref() ); // (re)initialization of output structure

	calcMaximum( rGradientLeftIn.ref(), rGradientRightIn.ref(), m_oMaxFilterLenght, m_oImageSize.height, m_oContourLeftOut, m_oContourRightOut); // signal processing

	enforceIntegrity (m_oContourLeftOut, m_oContourRightOut, m_oImageSize.width); // validate integrity

	const double oNewRankLeft	= (rGradientLeftIn.rank()	+ 1.0) / 2.; // full rank
	const double oNewRankRight	= (rGradientRightIn.rank()	+ 1.0) / 2.; // full rank
	const GeoDoublearray oGeoSeamPosLeftOut		( rGradientLeftIn.context(), m_oContourLeftOut, rGradientLeftIn.analysisResult(), oNewRankLeft );
	const GeoDoublearray oGeoSeamPosRightOut	( rGradientRightIn.context(), m_oContourRightOut, rGradientRightIn.analysisResult(), oNewRankRight );

	preSignalAction();
	m_oPipeOutContourLeft.	signal( oGeoSeamPosLeftOut );
	m_oPipeOutContourRight.	signal( oGeoSeamPosRightOut );
} // proceed



void Maximum::reinitialize(
		const VecDoublearray		&p_rGradientLeftIn,
		const VecDoublearray		&p_rGradientRightIn
	) {
	m_oContourLeftOut.assign( p_rGradientLeftIn.size(), 0, eRankMin ); // (re)initialize output based on input dimension
	m_oContourRightOut.assign( p_rGradientRightIn.size(), 0, eRankMin ); // (re)initialize output based on input dimension
} // reinitialize



// actual signal processing
/*static*/
void Maximum::calcMaximum(
	const VecDoublearray		&p_rGradientLeftIn,
	const VecDoublearray		&p_rGradientRightIn,
	int						p_oMaxFilterLenght,
	int						p_oImageHeight,
	Doublearray				&p_rContourLeftOut,
	Doublearray				&p_rContourRightOut
	)
{

	// Parameter assertion. Should be pre-checked by UI / MMI / GUI.

	poco_assert_dbg( p_rGradientLeftIn.size()	==  p_rGradientRightIn.size());
	poco_assert_dbg( ! p_rGradientLeftIn.empty() );
	poco_assert_dbg( p_oMaxFilterLenght			>   0);

	const unsigned int oNProfiles				= p_rGradientLeftIn.size();
	const unsigned int oProfileSize				= p_rGradientLeftIn.front().getData().size();
	const unsigned int oMaxFilterLengthMinusOne	= p_oMaxFilterLenght - 1;

	poco_assert_dbg( p_oMaxFilterLenght <= static_cast<int>(oProfileSize) ); // Parameter assertion. Should be pre-checked by UI / MMI / GUI.

	// get references to data

	auto&	rContourLeftOutData		= p_rContourLeftOut.getData();
	auto&	rContourRightOutData	= p_rContourRightOut.getData();

	for (unsigned int profileN = 0; profileN < oNProfiles; ++profileN) { // loop over N profiles

		// find max in gradientLeft - search from Left

		// range without boundary values
		auto oItStart	= p_rGradientLeftIn[profileN].getData().begin()	+ oMaxFilterLengthMinusOne;
		auto oItEnd		= p_rGradientLeftIn[profileN].getData().end()	- oMaxFilterLengthMinusOne;

		const auto	oItMaxLeft		= std::max_element(oItStart, oItEnd);
		const unsigned int	oMaxIndexLeft	= std::distance(oItStart, oItMaxLeft) + oMaxFilterLengthMinusOne;	// get index


		// find max in gradientRight - search from Right

		// range without boundary values
		auto			oRItStart		= p_rGradientRightIn[profileN].getData().rbegin()	+ oMaxFilterLengthMinusOne;
		auto			oRItEnd			= p_rGradientRightIn[profileN].getData().rend()	- oMaxFilterLengthMinusOne;

		const auto				oRItMaxRight	= std::max_element(oRItStart, oRItEnd);
		const unsigned int		oMaxIndexRight	= std::distance(oRItMaxRight, oRItEnd) + oMaxFilterLengthMinusOne;	// get index

		// lower index is on the left
		const unsigned int	oSeamPosLeft		= oMaxIndexLeft;
		// higher index is on the right
		const unsigned int	oSeamPosRight		= oMaxIndexRight;

		// set result

		rContourLeftOutData[profileN]	= oSeamPosLeft;
		rContourRightOutData[profileN]	= oSeamPosRight;

	} // for profileN

	// calculate rank
	calcRank(p_rContourLeftOut, p_rContourRightOut, oProfileSize, p_oMaxFilterLenght);


} // calcMaximum




	} // namespace filter
} // namespace precitec
