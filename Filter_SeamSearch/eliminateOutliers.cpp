/*!
*  @copyright		Precitec Vision GmbH & Co. KG
*  @author			Simon Hilsenbeck (HS)
*  @date			2011
*  @file
*  @brief			Fliplib filter 'EliminateOutliers' in component 'Filter_SeamSearch'. Eliminates outliers in contour points.
*/


#include "eliminateOutliers.h"

#include "system/types.h"							///< typedefs
#include "common/defines.h"							///< debug assert integrity assumptions
#include "module/moduleLogger.h"

#include "overlay/overlayPrimitive.h"				///< paint overlay

#include "filter/algoArray.h"						///< Intarray algo

#include "seamSearch.h"								///< rank calculation

#include <algorithm>								///< replace
#include <functional>								///< equal
#include <fliplib/TypeToDataTypeImpl.h>


using namespace fliplib;
namespace precitec {
	using namespace image;
	using namespace interface;
	using namespace geo2d;
namespace filter {


const std::string EliminateOutliers::m_oFilterName 		= std::string("EliminateOutliers");
const std::string EliminateOutliers::m_oPipeOutName1	= std::string("ContourLeft");
const std::string EliminateOutliers::m_oPipeOutName2	= std::string("ContourRight");


EliminateOutliers::EliminateOutliers() :
TransformFilter				( EliminateOutliers::m_oFilterName, Poco::UUID{"CE19799E-68B4-48d1-AA6C-343AC21F16E5"} ),
	m_pPipeInContourL		(nullptr),
	m_pPipeInContourR		(nullptr),
	m_pPipeInImgSize		(nullptr),
	m_oPipeOutContourL		( this, m_oPipeOutName1 ),
	m_oPipeOutContourR		( this, m_oPipeOutName2 ),
	m_oVarianceFactor		(1.0),
	m_oNoReplacePos			(false)
{
	// set parameter default values

	parameters_.add("VarianceFactor",	Parameter::TYPE_double, m_oVarianceFactor);
	parameters_.add("NoReplacePosition", Parameter::TYPE_bool, m_oNoReplacePos);

    setInPipeConnectors({{Poco::UUID("F0688C08-DB20-4628-897A-3D7CE16093B6"), m_pPipeInContourL, "ContourLeft", 1, "contour_left"},
    {Poco::UUID("D90D9615-69B6-413b-A8C1-15CADCFD79D1"), m_pPipeInContourR, "ContourRight", 1, "contour_right"},
    {Poco::UUID("6F52EBD7-F384-4f9c-96FE-52C5952BE357"), m_pPipeInImgSize, "ImgSize", 1, ""}});
    setOutPipeConnectors({{Poco::UUID("1662BA26-69B9-4995-BCF7-46C6D25463F9"), &m_oPipeOutContourL, m_oPipeOutName1, 0, ""},
    {Poco::UUID("EED159BD-6C53-4013-8DB9-647CC6411046"), &m_oPipeOutContourR, m_oPipeOutName2, 0, ""}});
    setVariantID(Poco::UUID("959BAF5C-458C-440d-92D7-20CE44FC9BC4"));
} // EliminateOutliers



void EliminateOutliers::setParameter() {
	TransformFilter::setParameter();
	m_oVarianceFactor		= parameters_.getParameter("VarianceFactor").convert<double>();
	m_oNoReplacePos			= parameters_.getParameter("NoReplacePosition").convert<bool>();

	// Parameter assertion. Should be pre-checked by UI / MMI / GUI.
	poco_assert_dbg(m_oVarianceFactor >= 0);

} // setParameter



void EliminateOutliers::paint() {
	if(m_oVerbosity <= eNone) {
		return;
	} // if

	// paint contour points

	const GeoDoublearray		&rGeoContourLIn			( m_pPipeInContourL->read(m_oCounter) );
	const GeoDoublearray		&rGeoContourRIn			( m_pPipeInContourR->read(m_oCounter) );
	const unsigned int			oNProfiles				( rGeoContourLIn.ref().getData().size() ); // draw only first
	const int					oCrossRadius			( 4 );
	const int					oDeltaY					( oNProfiles != 0 ? m_oImageSize.height / oNProfiles : 0 );
	int							oY						( oDeltaY / 2 );

    if (rGeoContourLIn.context().trafo().isNull())
    {
        return;
    }
	const Trafo&			rTrafo						( *rGeoContourLIn.context().trafo() );
	OverlayCanvas&			rCanvas						( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer&			rLayerPosition				( rCanvas.getLayerPosition());
	OverlayLayer&			rLayerLine					( rCanvas.getLayerLine());
	OverlayLayer&			rLayerText					( rCanvas.getLayerText());

	const auto&				rContourLIn					( rGeoContourLIn.ref().getData() );
	const auto&				rContourRIn					( rGeoContourRIn.ref().getData() );
	const auto&				rContourLOut				( m_oContourLOut.getData() );
	const auto&				rContourROut				( m_oContourROut.getData() );
	const auto&				rContourLRankOut			( m_oContourLOut.getRank() );
	const auto&				rContourRRankOut			( m_oContourROut.getRank() );

	for (unsigned int sliceN = 0; sliceN < oNProfiles; ++sliceN) { // loop over N profiles

		const Point			oPositionLOut			( int( rContourLOut[sliceN]), oY );
		const Rect			oTextBoxL				( oPositionLOut.x, oPositionLOut.y, 30, 20 );

		std::stringstream oTmpL; oTmpL << m_oContourLOut.getRank()[sliceN];;
		rLayerPosition.add( new  OverlayCross(rTrafo(oPositionLOut), oCrossRadius, Color::Yellow() ) );
		rLayerText.add( new OverlayText(oTmpL.str(), Font(), rTrafo(oTextBoxL), Color::Yellow() ) );

		if (rContourLRankOut[sliceN] < eRankMax) { // rank not max, maybe was an outlier, draw line to show correction
			const Point oPositionLIn	( int( rContourLIn[sliceN] ), oY );
			rLayerLine.add( new OverlayLine(rTrafo(oPositionLIn), rTrafo(oPositionLOut), Color::Yellow() ) );
		} // if

		const Point			oPositionROut			( int( rContourROut[sliceN]), oY );
		const Rect			oTextBoxR				( oPositionROut.x, oPositionROut.y, 30, 20 );

		std::stringstream oTmpR; oTmpR << m_oContourROut.getRank()[sliceN];;
		rLayerPosition.add( new  OverlayCross(rTrafo(oPositionROut), oCrossRadius, Color::Magenta() ) );
		rLayerText.add( new OverlayText(oTmpR.str(), Font(), rTrafo(oTextBoxR), Color::Magenta() ) );

		if (rContourRRankOut[sliceN] < eRankMax) { // rank not max, maybe was an outlier, draw line to show correction
			const Point oPositionRIn	( int( rContourRIn[sliceN] ), oY );
			rLayerLine.add( new OverlayLine(rTrafo(oPositionRIn), rTrafo(oPositionROut), Color::Magenta() ) );
		} // if

		oY += oDeltaY;
	}
} // paint



bool EliminateOutliers::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {
	if (p_rPipe.tag() == "contour_left")
		m_pPipeInContourL  = dynamic_cast< scalar_pipe_t* >(&p_rPipe);
	else if (p_rPipe.tag() == "contour_right")
		m_pPipeInContourR  = dynamic_cast< scalar_pipe_t* >(&p_rPipe);
	else
		m_pPipeInImgSize = dynamic_cast< scalar_pipe_t * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void EliminateOutliers::proceedGroup(const void* sender, PipeGroupEventArgs& e) {

	poco_assert_dbg(m_pPipeInContourL != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInContourR != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInImgSize != nullptr); // to be asserted by graph editor


	// get data from frame

	const GeoDoublearray	rContourLIn				= m_pPipeInContourL->read(m_oCounter);
	const GeoDoublearray	rContourRIn				= m_pPipeInContourR->read(m_oCounter);
	const auto&				rImgSizeIn				= m_pPipeInImgSize->read(m_oCounter).ref().getData();

	reinitialize(rContourLIn.ref(), rContourRIn.ref()); // (re)initialization of output structure

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

	calcEliminateOutliers(rContourLIn.ref(), rContourRIn.ref(), m_oVarianceFactor, m_oContourLOut, m_oContourROut, m_oNoReplacePos); // signal processing

	//enforceIntegrity (m_oContourLOut, m_oContourROut); // validate integrity
	//std::cout << "m_oContourLOut " << m_oContourLOut << std::endl;
	//std::cout << "rContourRIn " << m_oContourROut << std::endl;

	const double oNewRankL	= (rContourLIn.rank()	+ 1.0) / 2.; // full rank
	const double oNewRankR	= (rContourRIn.rank()	+ 1.0) / 2.; // full rank
	const GeoDoublearray oGeoSeamPosLOut	(rContourLIn.context(), m_oContourLOut, rContourLIn.analysisResult(), oNewRankL);
	const GeoDoublearray oGeoSeamPosROut	(rContourRIn.context(), m_oContourROut, rContourRIn.analysisResult(), oNewRankR);

	preSignalAction();
	m_oPipeOutContourL.signal( oGeoSeamPosLOut );
	m_oPipeOutContourR.signal( oGeoSeamPosROut );
} // proceed



void EliminateOutliers::reinitialize(
		const Doublearray			&p_rContourLIn,
		const Doublearray			&p_rContourRIn
	) {
	m_oContourLOut.assign(p_rContourLIn.size(), 0, eRankMin);
	m_oContourROut.assign(p_rContourRIn.size(), 0, eRankMin);
} // reinitialize



// actual signal processing
/*static*/
void EliminateOutliers::calcEliminateOutliers(
	const Doublearray		&p_rContourLIn,
	const Doublearray		&p_rContourRIn,
	double					p_oVarianceFactor,
	Doublearray				&p_rContourLOut,
	Doublearray				&p_rContourROut,
	bool					p_oNoReplacePos
	)
{

	// calculate sample mean (de: stichprobenvarianz)

	double oSumL		= 0;
	double oSumR		= 0;
	double oSumSqareL	= 0;
	double oSumSqareR	= 0;
    unsigned int CountL = 0;
    unsigned int CountR = 0;


	// equal size for left and right input contour asserted - see above
	poco_assert_dbg( p_rContourLIn.size() == p_rContourRIn.size() );
	const unsigned int	oNbSeamPos	= p_rContourLIn.size();

	for (unsigned int oSeamPosN = 0; oSeamPosN != oNbSeamPos; ++oSeamPosN) {
		const double	oSeamPosLIn	= p_rContourLIn.getData()[oSeamPosN];
		const int		oRankPosLIn	= p_rContourLIn.getRank()[oSeamPosN];

        const double	oSeamPosRIn	= p_rContourRIn.getData()[oSeamPosN];
		const int		oRankPosRIn	= p_rContourRIn.getRank()[oSeamPosN];

        if(oRankPosLIn > eRankMin) {
            oSumL		+= oSeamPosLIn;
            oSumSqareL	+= oSeamPosLIn * oSeamPosLIn;
            ++CountL;
        }
        if(oRankPosRIn > eRankMin) {
            oSumR		+= oSeamPosRIn;
            oSumSqareR	+= oSeamPosRIn * oSeamPosRIn;
            ++CountR;
        }
	}

	const double oMeanL		= oSumL / CountL; // neq zero asserted before
	const double oMeanR		= oSumR / CountR; // neq zero asserted before

	const double oVarianceL	= ( oSumSqareL - oSumL*oMeanL) / CountL;
	const double oVarianceR	= ( oSumSqareR - oSumR*oMeanR) / CountR;

	const double oCustomVarianceL = oVarianceL * p_oVarianceFactor;
	const double oCustomVarianceR = oVarianceR * p_oVarianceFactor;

	const int	OUTLIER		= -1;
	int			oNL			= 0;
	int			oNR			= 0;
    oSumL   = 0;
    oSumR   = 0;


	// if zero variance we do not seek for outliers. all would be outliers.
	if ( !((oCustomVarianceL == 0) && (CountL == oNbSeamPos))
    ) {
		// if squared distance to sample mean is greater than custom variance mark value as outlier
		for (unsigned int oSeamPosN = 0; oSeamPosN != oNbSeamPos; ++oSeamPosN) {
			const double	oSeamPosLIn		= p_rContourLIn.getData()[oSeamPosN];
			const int&		rRankLIn		= p_rContourLIn.getRank()[oSeamPosN];

			double&			rSeamPosLOut	= p_rContourLOut.getData()[oSeamPosN];
			int&			rRankLOut		= p_rContourLOut.getRank()[oSeamPosN];
			if ( (oSeamPosLIn - oMeanL) * (oSeamPosLIn - oMeanL) <= oCustomVarianceL ) {
				rSeamPosLOut	= oSeamPosLIn; // ok
				rRankLOut		= rRankLIn;
				oSumL			+= oSeamPosLIn; // add to sum
				++oNL;
			}
			else {
				if (p_oNoReplacePos)
				{
					rSeamPosLOut = oSeamPosLIn;  // keep pos
				}
				else
				{
					rSeamPosLOut = OUTLIER; // outlier - mark
				}
				if (rRankLIn > eRankMin) {
                    rRankLOut = rRankLIn >> 1; // rank diveded by 2
                }
			}
		} // while
	} // if
	else { // no variance - copy data
		p_rContourLOut	= p_rContourLIn;
	} // else

	// if zero variance we do not seek for outliers. all would be outliers.
	if ( !((oCustomVarianceR == 0) && (CountR == oNbSeamPos)) ) {
		// if squared distance to sample mean is greater than custom variance mark value as outlier
		for (unsigned int oSeamPosN = 0; oSeamPosN != oNbSeamPos; ++oSeamPosN) {
			const double	oSeamPosRIn	= p_rContourRIn.getData()[oSeamPosN];
			const int&		rRankRIn		= p_rContourRIn.getRank()[oSeamPosN];

			double&	rSeamPosROut		= p_rContourROut.getData()[oSeamPosN];
			int&	rRankROut			= p_rContourROut.getRank()[oSeamPosN];
			if ( (oSeamPosRIn - oMeanR) * (oSeamPosRIn - oMeanR) <= oCustomVarianceR ) {
				rSeamPosROut	= oSeamPosRIn; // ok
				rRankROut		= rRankRIn;
				oSumR			+= oSeamPosRIn; // add to sum
				++oNR;
			}
			else {
				if (p_oNoReplacePos)
				{
					rSeamPosROut = oSeamPosRIn;  // keep pos
				}
				else
				{
					rSeamPosROut = OUTLIER; // outlier - mark
				}
				if (rRankRIn > eRankMin) {
                    rRankROut = rRankRIn >> 1; // mean rank diveded by 2
                }
			}
		} // while
	} // if
	else { // no variance - copy data
		p_rContourROut	= p_rContourRIn;
	} // else

	// replace outliers by corrected mean value

	if (!p_oNoReplacePos)
	{
		const double oCorrMeanL = oNL != 0 ? oSumL / oNL : 0; // corrected mean is sum without ouliers divided by new count
		const double oCorrMeanR = oNR != 0 ? oSumR / oNR : 0; // corrected mean is sum without ouliers divided by new count
		for (unsigned int oSeamPosN = 0; oSeamPosN != oNbSeamPos; ++oSeamPosN) {
			double& rSeamPosLOut = p_rContourLOut.getData()[oSeamPosN];
			double& rSeamPosROut = p_rContourROut.getData()[oSeamPosN];
			if (rSeamPosLOut == OUTLIER) {
				rSeamPosLOut = oCorrMeanL;
			}
			if (rSeamPosROut == OUTLIER) {
				rSeamPosROut = oCorrMeanR;
			}
		} // for
	} // if
} // calcEliminateOutliers




	} // namespace filter
} // namespace precitec
