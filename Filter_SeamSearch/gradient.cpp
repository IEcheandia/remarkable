/*!
 *  @file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2011
 *  @brief			Fliplib filter 'Gradient' in component 'Filter_SeamSearch'. Gradient calculation on grey level profile.
 */


#include "gradient.h"

#include <algorithm>								///< max, all_of
#include <functional>								///< max, all_of

#include <system/types.h>							///< typedefs
#include <system/platform.h>						///< global and platform specific defines
#include <system/tools.h>							///< debug assert integrity assumptions
#include "module/moduleLogger.h"

#include "overlay/overlayPrimitive.h"				///< paint overlay

#include "filter/algoArray.h"						///< Intarray algo
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace image;
	using namespace interface;
	using namespace geo2d;
namespace filter {


const std::string Gradient::m_oFilterName 	= std::string("Gradient");
const std::string Gradient::m_oPipeOutName1	= std::string("GradientLeft");
const std::string Gradient::m_oPipeOutName2	= std::string("GradientRight");
const std::string Gradient::m_oPipeOutName3	= std::string("MaxFilterLength");


Gradient::Gradient() :
	TransformFilter			( Gradient::m_oFilterName, Poco::UUID{"4588362d-5fdc-43d6-91bc-46c5a366c6cc"} ),
	m_pPipeInProfileLpOffSeam		( nullptr ),
	m_pPipeInProfileLpOnSeam		( nullptr ),
	m_oPipeOutGradLeft		( this, m_oPipeOutName1 ),
	m_oPipeOutGradRight		( this, m_oPipeOutName2 ),
	m_oPipeOutMaxFLength	( this, m_oPipeOutName3 ),
	m_oFilterRadiusOffSeam	(5),
	m_oFilterRadiusOnSeam	(5),
	m_oGradientType			(eAbsolute)
{
	// Defaultwerte der Parameter setzen
	parameters_.add("FilterRadiusOffSeam",	Parameter::TYPE_int, m_oFilterRadiusOffSeam);
	parameters_.add("FilterRadiusOnSeam",	Parameter::TYPE_int, m_oFilterRadiusOnSeam);
	parameters_.add("GradientType",			Parameter::TYPE_int,	static_cast<int>(m_oGradientType));

    setInPipeConnectors({{Poco::UUID("19b01b85-962c-444f-b617-a8f34d5a9c53"), m_pPipeInProfileLpOffSeam, "Line", 1, "profile_off_seam"},
    {Poco::UUID("9002ff30-b00f-45e7-9a8a-0212f2033460"), m_pPipeInProfileLpOnSeam, "Line", 1, "profile_on_seam"}});
    setOutPipeConnectors({{Poco::UUID("950881d2-9c8c-4837-b206-80a695f7e0ab"), &m_oPipeOutGradLeft, "GradientLeft", 0, "gradient_left"},
    {Poco::UUID("ab861f37-3e74-4aa2-9d97-e74abf769324"), &m_oPipeOutGradRight, "GradientRight", 0, "gradient_right"},
    {Poco::UUID("1C216888-9D45-4215-A38E-5E60B3F8CD18"), &m_oPipeOutMaxFLength, "MaxFilterLength", 0, ""}});
    setVariantID(Poco::UUID("5b8cbbbe-56cd-4498-8403-f46df14913a1"));
} // Gradient



void Gradient::setParameter() {
	TransformFilter::setParameter();
	m_oFilterRadiusOffSeam	= parameters_.getParameter("FilterRadiusOffSeam").convert<int>();
	m_oFilterRadiusOnSeam	= parameters_.getParameter("FilterRadiusOnSeam").convert<int>();
	m_oGradientType			= static_cast<GradientType>(parameters_.getParameter("GradientType").convert<int>());

	poco_assert_dbg(m_oFilterRadiusOffSeam >= 1);	// Parameter assertion. Should be pre-checked by UI / MMI / GUI.
	poco_assert_dbg(m_oFilterRadiusOnSeam >= 1);	// Parameter assertion. Should be pre-checked by UI / MMI / GUI.
	poco_assert_dbg(m_oGradientType >= eGradientTypeMin && m_oGradientType <= eGradientTypeMax);	// Parameter assertion. Should be pre-checked by UI / MMI / GUI.

} // setParameter


void Gradient::paint() {
	if(m_oVerbosity <= eNone || m_oGradientLeftOut.empty()) {
		return;
	} // if

	//if(m_pPipeInProfileLpOffSeam == m_pPipeInProfileLpOnSeam) {
	//	wmLog(eWarning, "Input pipes in filter 'Gradient' should stem from different filters.\n"); // possible if group events come from the same sender
	//} // if

	const GeoVecDoublearray	&rGeoProfileLpOffIn	= m_pPipeInProfileLpOffSeam->read(m_oCounter); // just for size
	//const GeoVecDoublearray	&rGeoProfileLpOnIn	= m_pPipeInProfileLpOnSeam->read(m_oCounter); // just for size
	const unsigned int		oProfileSize		= rGeoProfileLpOffIn.ref().front().size();

	interface::SmpTrafo oSmpTrafo( rGeoProfileLpOffIn.context().trafo() );
    if (oSmpTrafo.isNull())
    {
        return;
    }
	OverlayCanvas&	rCanvas = canvas<OverlayCanvas>(m_oCounter);
	OverlayLayer&				rLayerContour			( rCanvas.getLayerContour());
	OverlayLayer&				rLayerText				( rCanvas.getLayerText());

	// paint low passed profiles and gradients of last slice

    const int oOffsetY = 1;
    for (unsigned int x = 0; x < oProfileSize - 1; ++x)
    {
        rLayerContour.add<OverlayLine>((*oSmpTrafo)(Point(x, oOffsetY + 255 + int(m_oGradientLeftOut.back().getData()[x]))),
                                       (*oSmpTrafo)(Point(x + 1, oOffsetY + 255 + int(m_oGradientLeftOut.back().getData()[x + 1]))), Color::Orange());
        rLayerContour.add<OverlayLine>((*oSmpTrafo)(Point(x, oOffsetY + 1 + 255 + int(m_oGradientRightOut.back().getData()[x]))),
                                       (*oSmpTrafo)(Point(x + 1, oOffsetY + 1 + 255 + int(m_oGradientRightOut.back().getData()[x + 1]))), Color::Cyan());
    }

    rLayerText.add<OverlayText>("Gradient Left",  image::Font(), Rect(10, 50, 200, 20), Color::Orange());
    rLayerText.add<OverlayText>("Gradient Right", image::Font(), Rect(10, 70, 200, 20), Color::Cyan());
} // paint



bool Gradient::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {
	if (p_rPipe.tag() == "profile_off_seam") {
		m_pPipeInProfileLpOffSeam = dynamic_cast< SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);
	}
	else if (p_rPipe.tag() == "profile_on_seam") {
		m_pPipeInProfileLpOnSeam = dynamic_cast< SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);
	}
	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void  Gradient::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg) {

	poco_assert_dbg(m_pPipeInProfileLpOffSeam != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInProfileLpOnSeam != nullptr); // to be asserted by graph editor


	// get input data
	const GeoVecDoublearray	&rGeoProfileLpOffSeamIn	= m_pPipeInProfileLpOffSeam->read(m_oCounter);
	const GeoVecDoublearray	&rGeoProfileLpOnSeamIn	= m_pPipeInProfileLpOnSeam->read(m_oCounter);
	poco_assert_dbg( ! rGeoProfileLpOffSeamIn.ref().empty() );
	poco_assert_dbg( ! rGeoProfileLpOnSeamIn.ref().empty() );
	poco_assert_dbg( rGeoProfileLpOffSeamIn.ref().size() == rGeoProfileLpOnSeamIn.ref().size());

	// (re)initialization of output structure
	reinitialize(rGeoProfileLpOffSeamIn.ref()); // both output gradients depend on both input profiles. Therefore, input profiles are expected to be of same size.

	// input validity check
	const unsigned int oProfileSize	    = std::min( rGeoProfileLpOffSeamIn.ref()[0].getData().size(), rGeoProfileLpOnSeamIn.ref()[0].getData().size() );
	const unsigned int oMaxFilterLength = std::max(m_oFilterRadiusOffSeam * 2 + 1, m_oFilterRadiusOnSeam * 2 + 1);
	const auto oAnalysisOk = rGeoProfileLpOffSeamIn.analysisResult() == AnalysisOK ? rGeoProfileLpOnSeamIn.analysisResult() :  rGeoProfileLpOffSeamIn.analysisResult();

	if ( inputIsInvalid(rGeoProfileLpOffSeamIn) || inputIsInvalid(rGeoProfileLpOnSeamIn) || rGeoProfileLpOffSeamIn.ref().size() != rGeoProfileLpOnSeamIn.ref().size() || oMaxFilterLength >= oProfileSize ) {
		const GeoVecDoublearray oGeoGradLeftOut	(rGeoProfileLpOffSeamIn.context(), m_oGradientLeftOut, oAnalysisOk, 0.0); // bad rank
		const GeoVecDoublearray oGeoGradRightOut	(rGeoProfileLpOffSeamIn.context(), m_oGradientRightOut, oAnalysisOk, 0.0); // bad rank
		const GeoDoublearray	oGeoMaxFLengthOut	( rGeoProfileLpOffSeamIn.context(), Doublearray(1, 0, eRankMin), oAnalysisOk, 0.0 );

		preSignalAction();
		m_oPipeOutGradLeft.signal	( oGeoGradLeftOut ); // invoke linked filter(s)
		m_oPipeOutGradRight.signal	( oGeoGradRightOut ); // invoke linked filter(s)
		m_oPipeOutMaxFLength.signal	( oGeoMaxFLengthOut );  // invoke linked filter(s) WORKAROUND

		return; // RETURN
	}

	calcGradient(
		rGeoProfileLpOffSeamIn.ref(),
		rGeoProfileLpOnSeamIn.ref(),
		m_oFilterRadiusOffSeam,
		m_oFilterRadiusOnSeam,
		m_oGradientType,
		m_oGradientLeftOut,
		m_oGradientRightOut
	); // signal processing

	const double				oNewRank			( (rGeoProfileLpOffSeamIn.rank() + rGeoProfileLpOnSeamIn.rank() + 1.0) / 3. ); // full rank
	const double				oMaxFLength			( std::max (m_oFilterRadiusOnSeam, m_oFilterRadiusOffSeam) );
	const GeoVecDoublearray		oGeoGradLeftOut		( rGeoProfileLpOffSeamIn.context(), m_oGradientLeftOut, oAnalysisOk, oNewRank );		// context of input pipes expected to be equal
	const GeoVecDoublearray		oGeoGradRightOut	( rGeoProfileLpOffSeamIn.context(), m_oGradientRightOut, oAnalysisOk, oNewRank );	// context of input pipes expected to be equal
	const GeoDoublearray		oGeoMaxFLengthOut	( rGeoProfileLpOffSeamIn.context(), Doublearray(1, oMaxFLength, eRankMax), oAnalysisOk, oNewRank );

	preSignalAction();
	m_oPipeOutGradLeft.signal	( oGeoGradLeftOut );	// invoke linked filter(s)
	m_oPipeOutGradRight.signal	( oGeoGradRightOut );	// invoke linked filter(s)
	m_oPipeOutMaxFLength.signal	( oGeoMaxFLengthOut );  // invoke linked filter(s) WORKAROUND

} // proceed



void Gradient::reinitialize(const VecDoublearray &p_rProfileIn) {
	filter::resetFromInput(p_rProfileIn, m_oGradientRightOut);	// (re)initialize output based on input dimension
	filter::resetFromInput(p_rProfileIn, m_oGradientLeftOut);	// (re)initialize output based on input dimension
} // reinitialize



// actual signal processing
/*static*/
void Gradient::calcGradient(
	const VecDoublearray &p_rProfileLpOffSeamIn,
	const VecDoublearray &p_rProfileLpOnSeamIn,
	unsigned int	p_oFilterRadiusOffSeam,
	unsigned int	p_oFilterRadiusOnSeam,
	GradientType	p_oGradientType,
	VecDoublearray		&p_rGradientLeftOut,
	VecDoublearray		&p_rGradientRightOut
	)
{
	poco_assert_dbg( p_rProfileLpOffSeamIn.size() == p_rProfileLpOnSeamIn.size() );
	const unsigned int	oFilterLengthOffSeam	= p_oFilterRadiusOffSeam * 2 + 1;
	const unsigned int	oFilterLengthOnSeam		= p_oFilterRadiusOnSeam * 2 + 1;
	const unsigned int	oMaxFilterLength		= std::max(oFilterLengthOffSeam, oFilterLengthOnSeam);
	const unsigned int	oNProfiles				= p_rProfileLpOffSeamIn.size();

	poco_assert_dbg(oNProfiles					!= 0); // there must be at least one profile line

	const unsigned int	oProfileLpOffSeamSize		= p_rProfileLpOffSeamIn[0].getData().size();
	const unsigned int	oProfileLpOnSeamSize		= p_rProfileLpOnSeamIn[0].getData().size();

	poco_assert_dbg( oProfileLpOffSeamSize == oProfileLpOnSeamSize );
	const unsigned int	oProfileSize		= std::min(oProfileLpOffSeamSize, oProfileLpOnSeamSize); // equal to size of second profile - see asssertion above

	poco_assert_dbg(oMaxFilterLength * 2 <= oProfileLpOffSeamSize); // Parameter assertion. Should be pre-checked by UI / MMI / GUI.
	poco_assert_dbg(oMaxFilterLength * 2 <= oProfileLpOnSeamSize); // Parameter assertion. Should be pre-checked by UI / MMI / GUI.

	// assert that all profile lines are of equal length
	for (unsigned int profileN = 0; profileN < oNProfiles; ++profileN) { // loop over N profiles
		poco_assert_dbg( oProfileLpOffSeamSize == p_rProfileLpOffSeamIn[profileN].getData().size() );
		poco_assert_dbg( oProfileLpOnSeamSize == p_rProfileLpOnSeamIn[profileN].getData().size() );
	}

	const auto oIsBadRankPredicate		=  [] (int p_rRank) { return p_rRank == eRankMin; };

	for (unsigned int profileN = 0; profileN < oNProfiles; ++profileN) { // loop over N profiles
		auto&	rProfileOffSeamData			= p_rProfileLpOffSeamIn[profileN].getData();
		auto&	rProfileOffSeamRank			= p_rProfileLpOffSeamIn[profileN].getRank();
		auto&	rProfileOnSeamData			= p_rProfileLpOnSeamIn[profileN].getData();
		auto&	rProfileOnSeamRank			= p_rProfileLpOnSeamIn[profileN].getRank();

		const bool	oAllBadRank1	= std::all_of(rProfileOnSeamRank.begin(), rProfileOnSeamRank.end(), oIsBadRankPredicate );
		const bool	oAllBadRank2	= std::all_of(rProfileOffSeamRank.begin(), rProfileOffSeamRank.end(), oIsBadRankPredicate );

		// validate preconditions

		if (oAllBadRank1 == true) {
			std::cerr << "Input array1 in profile #" << profileN << " has overall bad rank. '" << m_oFilterName << "' skipped." << std::endl;
			return;
		}
		if (oAllBadRank2 == true) {
			std::cerr << "Input array2 in profile #" << profileN << " has overall bad rank. '" << m_oFilterName << "' skipped." << std::endl;
			return;
		}

		auto&	rGradientLeftData		= p_rGradientLeftOut[profileN].getData();
		auto&	rGradientLeftRank		= p_rGradientLeftOut[profileN].getRank();
		auto&	rGradientRightData		= p_rGradientRightOut[profileN].getData();
		auto&	rGradientRightRank		= p_rGradientRightOut[profileN].getRank();

		// resize buffers if necessary

		// only check for rGradientLeftData because sizes ought to be consistent
		if (rGradientLeftData.size() != oProfileSize) {
			rGradientLeftData.resize(oProfileSize);
			rGradientLeftRank.resize(oProfileSize);
			rGradientRightData.resize(oProfileSize);
			rGradientRightRank.resize(oProfileSize);
		}

		// gradient calculation

		const unsigned int	oStart						= oMaxFilterLength;
		const unsigned int	oEnd						= oProfileSize - oMaxFilterLength;
		const unsigned int  HALF_FLENGTH_ON_PLUS_ONE	= oFilterLengthOnSeam / 2 + 1;
		const unsigned int  HALF_FLENGTH_OFF_PLUS_ONE	= oFilterLengthOffSeam / 2 + 1;

		// loop over low pass filtered profiles without boundary values
		for (unsigned int x = oStart; x < oEnd; ++x) {
			// ist zb filterlaenge 3, und zentrisch gefilter (zB: mean[pos] = x[pos-1]+x[pos]+x[pos+1] / 3)
			// muss man vom aktuellen werte filterlaenge halbe + 1 also hier (3+1)/2 = 2 positionen von der momentanen position nach
			// vorne oder hinten gehen um den mittelwert VOR bzw NACH der aktuellen position zu erhalten
			const unsigned int	LEFT_OFF	= x - HALF_FLENGTH_OFF_PLUS_ONE;
			const unsigned int	LEFT_ON		= x + HALF_FLENGTH_ON_PLUS_ONE;
			const unsigned int	RIGHT_ON	= x - HALF_FLENGTH_ON_PLUS_ONE;
			const unsigned int	RIGHT_OFF	= x + HALF_FLENGTH_OFF_PLUS_ONE;

			const double	oLeftMeanOffSeam	= rProfileOffSeamData	[LEFT_OFF];
			const double	oLeftMeanOnSeam		= rProfileOnSeamData	[LEFT_ON];
			const double	oRightMeanOnSeam	= rProfileOnSeamData	[RIGHT_ON];
			const double	oRightMeanOffSeam	= rProfileOffSeamData	[RIGHT_OFF];

			const double	oGradientLeft	= oLeftMeanOffSeam	- oLeftMeanOnSeam;
			const double	oGradientRight	= oRightMeanOffSeam - oRightMeanOnSeam;

			// dependung on the brightness or drakness of the seam the gradient is computed in such a way, that a maximal positive value is obtained for left resp. right seam  position

			switch (p_oGradientType) {
			case eAbsolute :
				rGradientLeftData[x]	= std::abs( oGradientLeft );	// take absolute value
				rGradientRightData[x]	= std::abs( oGradientRight );	// take absolute value
				break;
			case eDarkSeam :
				rGradientLeftData[x]	= + ( oGradientLeft );	// dark seam left side: bright minus dark values -> sign ok
				rGradientRightData[x]	= + ( oGradientRight );	// dark seam right side: dark minus bright values -> turn sign
				break;
			case eBrightSeam :
				rGradientLeftData[x]	= - ( oGradientLeft );	// bright seam left side: dark minus bright values -> turn sign
				rGradientRightData[x]	= - ( oGradientRight );	// bright seam right side: bright minus dark values -> sign ok
				break;
			// error state
			default :
				std::ostringstream oMsg;
				oMsg << "No case for switch argument: " << p_oGradientType;
				poco_bugcheck_msg(oMsg.str().c_str());
			}
			// set rank

			rGradientLeftRank[x]	= (rProfileOffSeamRank[LEFT_OFF]	+ rProfileOnSeamRank[RIGHT_ON]) / 2; // take mean of the two ranks
			rGradientRightRank[x]	= (rProfileOnSeamRank[LEFT_ON]		+ rProfileOnSeamRank[RIGHT_OFF]) / 2; // take mean of the two ranks

			// boundary rank values already got bad rank from initialisation

		} // for


	} // for profileN < oNProfiles
} // calcGradient


} // namespace filter
} // namespace precitec
