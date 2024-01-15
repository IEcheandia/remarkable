/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Stefan Birmanns (SB), HS
 *  @date		2011
 *  @brief		Computes maximum / minimum of a laserline.
 */

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include "module/moduleLogger.h"
#include <filter/armStates.h>

#include "filter/algoArray.h"	///< algorithmic interface for class TArray

// local includes
#include "lineExtremum.h"

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string LineExtremum::m_oFilterName 	= std::string("LineExtremum");
const std::string LineExtremum::PIPENAME_OUT1	= std::string("PositionX");
const std::string LineExtremum::PIPENAME_OUT2	= std::string("PositionY");


LineExtremum::LineExtremum() :
	TransformFilter( LineExtremum::m_oFilterName, Poco::UUID{"8F43A348-1EB7-4B80-B786-36819E1C155C"} ),
	m_pPipeLineIn		( nullptr ),
	m_oPipePositionXOut	( this, LineExtremum::PIPENAME_OUT1 ),
	m_oPipePositionYOut	( this, LineExtremum::PIPENAME_OUT2 ),
	m_oExtremumType		( eMaximum ),
	m_oDirection		( eFromLeft ),
	m_oOut				( 1 )
{
	// Set default values of the parameters of the filter
	parameters_.add( "ExtremumType",	Parameter::TYPE_int, static_cast<int>(m_oExtremumType) );
	parameters_.add( "SearchDir",		Parameter::TYPE_int, static_cast<int>(m_oDirection) );

    setInPipeConnectors({{Poco::UUID("84BC8BFC-6D17-4003-8463-4134AEAC6523"), m_pPipeLineIn, "Line", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("4CDE2A1A-6A46-4EE0-B1CF-B734F8B4E50B"), &m_oPipePositionXOut, PIPENAME_OUT1, 0, ""},
    {Poco::UUID("ECBCAC5D-DE96-4fa5-B929-D1D5EA39E658"), &m_oPipePositionYOut, PIPENAME_OUT2, 0, ""}});
    setVariantID(Poco::UUID("74A097B1-542B-4913-A5D0-681CA6D2B3E0"));
} // LineExtremum



void LineExtremum::setParameter()
{
	TransformFilter::setParameter();
	m_oExtremumType = static_cast<ExtremumType>( parameters_.getParameter("ExtremumType").convert<int>() );
	m_oDirection	= static_cast<SearchDirType>( parameters_.getParameter("SearchDir").convert<int>() );
} // setParameter



void LineExtremum::paint() {
	if(m_oVerbosity < eLow || inputIsInvalid(m_oOut) || m_oSpTrafo.isNull()){
		return;
	} // if

	const Trafo		&rTrafo			( *m_oSpTrafo );
	OverlayCanvas	&rCanvas		( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerPosition	( rCanvas.getLayerPosition());

	for (unsigned int oExtremum = 0; oExtremum < m_oOut.size(); ++oExtremum) { // loop over N extrema
		rLayerPosition.add( new	OverlayCross(rTrafo(m_oOut.getData()[oExtremum]), Color::Cyan())); // paint all positions
	}
} // paint



bool LineExtremum::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeLineIn  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void LineExtremum::proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeLineIn != nullptr); // to be asserted by graph editor
	// Read-out laserline
	const GeoVecDoublearray& rLineIn = m_pPipeLineIn->read(m_oCounter);
	m_oSpTrafo	= rLineIn.context().trafo();
	// And extract the byte-array
	const VecDoublearray& rArrayIn = rLineIn.ref();
	/// reset output based on input size
	m_oOut.assign(rArrayIn.size(), Point(0, 0), eRankMin);

	// input validity check

	if ( inputIsInvalid(rLineIn) ) {
		const GeoDoublearray oGeoDoublearrayXOut(rLineIn.context(), getCoordinate(m_oOut, eX), rLineIn.analysisResult(), interface::NotPresent); // bad rank
		const GeoDoublearray oGeoDoublearrayYOut(rLineIn.context(), getCoordinate(m_oOut, eY), rLineIn.analysisResult(), interface::NotPresent); // bad rank
		preSignalAction();
		m_oPipePositionXOut.signal( oGeoDoublearrayXOut ); // invoke linked filter(s)
		m_oPipePositionYOut.signal( oGeoDoublearrayYOut ); // invoke linked filter(s)

		return; // RETURN
	}

	// Now do the actual image processing
	findExtremum( rArrayIn, m_oExtremumType, m_oOut );

	// Create a new point, put the global context into the resulting profile and copy the rank over
	const auto oAnalysisResult	= rLineIn.analysisResult() == AnalysisOK ? AnalysisOK : rLineIn.analysisResult(); // replace 2nd AnalysisOK by your result type
	const GeoDoublearray oGeoDoublearrayXOut(rLineIn.context(), getCoordinate(m_oOut, eX), oAnalysisResult, interface::Limit); // full rank here, detailed rank in array
	const GeoDoublearray oGeoDoublearrayYOut(rLineIn.context(), getCoordinate(m_oOut, eY), oAnalysisResult, interface::Limit); // full rank here, detailed rank in array
	preSignalAction();
	m_oPipePositionXOut.signal( oGeoDoublearrayXOut ); // invoke linked filter(s)
	m_oPipePositionYOut.signal( oGeoDoublearrayYOut ); // invoke linked filter(s)

} // proceedGroup


void LineExtremum::findExtremum( const geo2d::VecDoublearray &p_rLineIn, ExtremumType p_oExtremumType, geo2d::Pointarray &p_rPositionOut)
{
	const unsigned int	oNbLines	= p_rLineIn.size();
	p_rPositionOut.assign(oNbLines, Point(0, 0)); // if the size of the output signal is not equal to the input line size, resize
	for (unsigned int lineN = 0; lineN < oNbLines; ++lineN) { // loop over N lines
		const Doublearray	&rLineIn			= p_rLineIn[lineN];
		Point				&rPositionOut		= p_rPositionOut.getData()[lineN];
		int					&rPositionOutRank	= p_rPositionOut.getRank()[lineN];

        //switch inside the loop, but typically this loop is very short
        switch(p_oExtremumType)
        {
            case eMaximum:
            {
                std::tie(rPositionOut.x, rPositionOutRank)= calcExtremum<double,eMaximum>(rLineIn, m_oDirection);
                break;
            }
            case eMinimum:
            {
                std::tie(rPositionOut.x, rPositionOutRank)= calcExtremum<double,eMinimum>(rLineIn, m_oDirection);
                break;
            }
            case eZeroCrossing:
            default:
                wmLog( eDebug, "Invalid ExtremumType '%i'\n", p_oExtremumType);
                assert(false && "not supported in this filter");
                break;
        }

        rPositionOut.y		= int( rLineIn.getData()[rPositionOut.x] );
	} // for
} // findExtremum

/*virtual*/ void
LineExtremum::arm(const fliplib::ArmStateBase& state) {
	if (state.getStateID() == eSeamStart)
	{
		m_oSpTrafo = nullptr;
	}
} // arm

} // namespace precitec
} // namespace filter
