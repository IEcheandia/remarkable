/*!
*  @copyright		Precitec Vision GmbH & Co. KG
*  @author			Simon Hilsenbeck (HS)
*  @date			2011
*  @file
*  @brief			Fliplib filter 'CorrectWidth' in component 'Filter_SeamSearch'. Eliminates seam width outliers in contour points.
*/


#include "correctWidth.h"

#include "system/types.h"							///< typedefs
#include "common/defines.h"							///< debug assert integrity assumptions
#include "module/moduleLogger.h"

#include "overlay/overlayPrimitive.h"				///< paint overlay

#include "filter/algoArray.h"						///< Intarray algo

#include "seamSearch.h"								///< rank calculation
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
	namespace filter {

using fliplib::SynchronePipe;
using fliplib::PipeEventArgs;
using fliplib::PipeGroupEventArgs;
using fliplib::Parameter;

const std::string CorrectWidth::m_oFilterName 	= std::string("CorrectWidth");
const std::string CorrectWidth::m_oPipeOutName1	= std::string("ContourLeft");
const std::string CorrectWidth::m_oPipeOutName2	= std::string("ContourRight");


CorrectWidth::CorrectWidth() :
TransformFilter				( CorrectWidth::m_oFilterName, Poco::UUID{"9659058D-A6AC-44bc-B45F-B16B8BB21ADF"} ),
	m_pPipeInContourL		(nullptr),
	m_pPipeInContourR		(nullptr),
	m_pPipeInImgSize		(nullptr),
	m_oPipeOutContourL		( this, m_oPipeOutName1 ),
	m_oPipeOutContourR		( this, m_oPipeOutName2 ),
	m_oDefaultSeamWidth		(220),
	m_oMaxDiff				(50)
{
	// set parameter default values

	parameters_.add("DefaultSeamWidth",	Parameter::TYPE_int, m_oDefaultSeamWidth);
	parameters_.add("MaxDiff",			Parameter::TYPE_int, m_oMaxDiff);

    setInPipeConnectors({{Poco::UUID("A9C4C2EC-C8F4-461c-A9CE-6FB32B208D31"), m_pPipeInContourL, "ContourLeft", 1, "contour_left"},
    {Poco::UUID("207BB1BA-B287-4de5-BC56-DC6F4CB5D7E7"), m_pPipeInContourR, "ContourRight", 1, "contour_right"},
    {Poco::UUID("40859B6D-D6E9-4243-8663-9D1EBDFB52C3"), m_pPipeInImgSize, "ImgSize", 1, ""}});
    setOutPipeConnectors({{Poco::UUID("13B009D2-0164-4025-BDA0-F7FB60A9E29F"), &m_oPipeOutContourL, "ContourLeft", 0, ""},
    {Poco::UUID("D4641528-2E1E-4ec5-91C5-3029F2548317"), &m_oPipeOutContourR, "ContourRight", 0, ""}});
    setVariantID(Poco::UUID("FB3F846E-BFB7-42f7-A927-66830589BE65"));
} // CorrectWidth



void CorrectWidth::setParameter() {
	TransformFilter::setParameter();
	m_oDefaultSeamWidth		= parameters_.getParameter("DefaultSeamWidth").convert<int>();
	m_oMaxDiff				= parameters_.getParameter("MaxDiff").convert<int>();

	// Parameter assertion. Should be pre-checked by UI / MMI / GUI.
	poco_assert_dbg(m_oDefaultSeamWidth > 0);
	poco_assert_dbg(m_oMaxDiff >= 0);
} // setParameter



void CorrectWidth::paint() {
} // paint



bool CorrectWidth::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {
	if (p_rPipe.tag() == "contour_left")
		m_pPipeInContourL  = dynamic_cast< scalar_pipe_t* >(&p_rPipe);
	else if (p_rPipe.tag() == "contour_right")
		m_pPipeInContourR  = dynamic_cast< scalar_pipe_t* >(&p_rPipe);
	else
		m_pPipeInImgSize = dynamic_cast< scalar_pipe_t * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void CorrectWidth::proceedGroup(const void* sender, PipeGroupEventArgs& e) {

	poco_assert_dbg(m_pPipeInContourL != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInContourR != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInImgSize != nullptr); // to be asserted by graph editor


	// get data from frame

	const GeoDoublearray	rContourLIn				= m_pPipeInContourL->read(m_oCounter);
	const GeoDoublearray	rContourRIn				= m_pPipeInContourR->read(m_oCounter);
	const auto&				rImgSizeIn				= m_pPipeInImgSize->read(m_oCounter).ref().getData();

	// input validity check
	if ( inputIsInvalid(rContourLIn) || inputIsInvalid(rContourRIn) || rContourLIn.ref().size() != rContourRIn.ref().size() ) {
		const GeoDoublearray oGeoSeamPosLOut	(rContourLIn.context(), m_oContourLOut, rContourLIn.analysisResult(), 0.0); // bad rank
		const GeoDoublearray oGeoSeamPosROut	(rContourRIn.context(), m_oContourROut, rContourRIn.analysisResult(), 0.0); // bad rank

		preSignalAction();
		m_oPipeOutContourL.signal( oGeoSeamPosLOut );
		m_oPipeOutContourR.signal( oGeoSeamPosROut );

		return; // RETURN
	}

	m_oImageSize.width	= int( rImgSizeIn[0] );
	m_oImageSize.height	= int( rImgSizeIn[1] );

	poco_assert_dbg(m_oDefaultSeamWidth	< m_oImageSize.width );	// Parameter assertion. Should be pre-checked by UI / MMI / GUI.

	reinitialize( rContourLIn.ref(), rContourRIn.ref() ); // (re)initialization of output structure

	calcCorrectWidth( rContourLIn.ref(), rContourRIn.ref(), m_oDefaultSeamWidth, m_oMaxDiff, m_oContourLOut, m_oContourROut); // signal processing

	enforceIntegrity (m_oContourLOut, m_oContourROut, m_oImageSize.width, m_oDefaultSeamWidth); // validate integrity

	const double oNewRankL	= (rContourLIn.rank()	+ 1.0) / 2.; // full rank
	const double oNewRankR	= (rContourRIn.rank()	+ 1.0) / 2.; // full rank
	const GeoDoublearray oGeoSeamPosLOut	(rContourLIn.context(), m_oContourLOut, rContourLIn.analysisResult(), oNewRankL);
	const GeoDoublearray oGeoSeamPosROut	(rContourRIn.context(), m_oContourROut, rContourRIn.analysisResult(), oNewRankR);

	preSignalAction();
	m_oPipeOutContourL.signal( oGeoSeamPosLOut );
	m_oPipeOutContourR.signal( oGeoSeamPosROut );
} // proceed



void CorrectWidth::reinitialize(
		const Doublearray			&p_rContourLIn,
		const Doublearray			&p_rContourRIn
	) {
	m_oContourLOut.assign(p_rContourLIn.size(), 0, eRankMin);
	m_oContourROut.assign(p_rContourRIn.size(), 0, eRankMin);
} // reinitialize



// actual signal processing
void CorrectWidth::calcCorrectWidth(
	const Doublearray		&p_rContourLIn,
	const Doublearray		&p_rContourRIn,
	int						p_oDefaultSeamWidth,
	int						p_oMaxDiff,
	Doublearray				&p_rContourLOut,
	Doublearray				&p_rContourROut
	)
{
	poco_assert_dbg( p_rContourLIn.size() == p_rContourRIn.size() );
	const unsigned int	oNbSeamPos	= p_rContourLIn.size();

	for (unsigned int oSeamPosN = 0; oSeamPosN != oNbSeamPos; ++oSeamPosN) {
		const		double &rSeamPosLIn	= p_rContourLIn.getData()[oSeamPosN];
		const		double &rSeamPosRIn	= p_rContourRIn.getData()[oSeamPosN];
		const int	&rRankLIn			= p_rContourLIn.getRank()[oSeamPosN];
		const int	 &rRankRIn			= p_rContourRIn.getRank()[oSeamPosN];

		double	&rSeamPosLOut		= p_rContourLOut.getData()[oSeamPosN];
		double	&rSeamPosROut		= p_rContourROut.getData()[oSeamPosN];
		int		&rRankLOut			= p_rContourLOut.getRank()[oSeamPosN];
		int		&rRankROut			= p_rContourROut.getRank()[oSeamPosN];

		const double oDiffToDefault = std::abs(rSeamPosRIn - rSeamPosLIn - p_oDefaultSeamWidth);

		if ( oDiffToDefault < p_oMaxDiff ) {
			//std::cout << "abs: " << std::abs(rSeamPosRIn - rSeamPosLIn - p_oDefaultSeamWidth) << std::endl;
			rSeamPosLOut	= rSeamPosLIn; // ok
			rRankLOut		= rRankLIn;

			rSeamPosROut	= rSeamPosRIn; // ok
			rRankROut		= rRankRIn;

		}
		else {
			if (rRankLIn > rRankRIn) { // if left rank better
				rSeamPosLOut	= rSeamPosLIn; // take in data
				rSeamPosROut	= rSeamPosLIn + p_oDefaultSeamWidth; // extrapolate: left point plus default width
				if (m_oVerbosity >= eHigh) {
					wmLog(eDebug, "%s: Right point extrapolated.", m_oFilterName.c_str());
				} // if

				rRankLOut	= rRankLIn; // take better rank
				rRankROut	= rRankLIn; // take better rank
			}
			else if (rRankRIn > rRankLIn) { // if right rank better
				rSeamPosLOut	= rSeamPosRIn - p_oDefaultSeamWidth; // extrapolate:  right point minus default width
				rSeamPosROut	= rSeamPosRIn; // take in data
				if (m_oVerbosity >= eHigh) {
					wmLog(eDebug, "%s: Left point extrapolated.", m_oFilterName.c_str());
				} // if

				rRankLOut	= rRankLIn; // take better rank
				rRankROut	= rRankLIn; // take better rank
			} // if
			else { // rank equal but bad distance - lower rank
			}
				rSeamPosLOut	= rSeamPosLIn; // ok
				rRankLOut		= rRankLIn / 2;

				rSeamPosROut	= rSeamPosRIn; // ok
				rRankROut		= rRankRIn / 2;

				// reduce rank once more if high difference

				if (oDiffToDefault > p_oDefaultSeamWidth / 2) {
					rRankLOut /= 2;
					rRankROut /= 2;
				}
		}
	} // for

} // calcCorrectWidth




	} // namespace filter
} // namespace precitec
