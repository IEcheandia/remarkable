/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Holger Kirschner (KiH), Stefan Birmanns (SB), HS
 *  @date		2011
 *  @brief		Moves a GeoPoint.
 */

// clib includes
#include <sstream>
// WM includes
#include "geo/point.h"
#include "overlay/overlayPrimitive.h"
#include "filter/algoArray.h"
#include "displacement1.h"
#include "module/moduleLogger.h"

#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {

using namespace interface;
using namespace image;
using namespace geo2d;

namespace filter {

using Poco::SharedPtr;
using fliplib::SynchronePipe;
using fliplib::BaseFilterInterface;
using fliplib::BasePipe;
using fliplib::TransformFilter;
using fliplib::PipeEventArgs;
using fliplib::PipeGroupEventArgs;
using fliplib::Parameter;

const std::string displacement::m_oFilterName 		= std::string("displacement");
const std::string displacement::PIPENAME_POSX			= std::string("PositionX");
const std::string displacement::PIPENAME_POSY			= std::string("PositionY");
const std::string displacement::FILTERBESCHREIBUNG 	= std::string("displacement nur x Position \n");

displacement::displacement() : TransformFilter( displacement::m_oFilterName, Poco::UUID{"B1E6A9DF-E626-4B0C-9321-BB2F68ED121B"} ),
	m_pPipeArrayX	( nullptr ),
	m_pPipeArrayY	( nullptr ),
	m_pPipeOutPosX	( new SynchronePipe< GeoDoublearray >( this, displacement::PIPENAME_POSX ) ),
	m_pPipeOutPosY	( new SynchronePipe< GeoDoublearray >( this, displacement::PIPENAME_POSY ) ),
	m_oTargetX		(200.0),
	m_oTargetY		(0.0),
	m_oXOut			( 1, 0.0 ),
	m_oYOut			( 1, 0.0 )
{
	parameters_.add("targetX", "double", m_oTargetX);
	parameters_.add("targetY", "double", m_oTargetY);

    setInPipeConnectors({{Poco::UUID("118F4019-40DB-4312-8074-9F20E5822D08"), m_pPipeArrayX , "PositionX", 1, "position_x"},
    {Poco::UUID("2B634F02-5B86-496c-849F-8246CB515F70"), m_pPipeArrayY , "PositionY", 1, "position_y"}});

    setOutPipeConnectors({{Poco::UUID("436FD743-4015-435B-88D0-7E41E5C8DC24"), m_pPipeOutPosX, "PositionX", 0, ""},
    {Poco::UUID("05F817CD-89DA-4fba-A5B8-4BCAE72E9522"), m_pPipeOutPosY, "PositionY", 0, ""}});
    setVariantID(Poco::UUID("17381E71-D6F7-4A09-BB0C-EE02E56E941B"));
}



displacement::~displacement()
{
	delete m_pPipeOutPosX;
	delete m_pPipeOutPosY;
}



void displacement::setParameter()
{
	TransformFilter::setParameter();
	m_oTargetX		= parameters_.getParameter("targetX").convert<double>();
	m_oTargetY		= parameters_.getParameter("targetY").convert<double>();

	if( m_oVerbosity >= eMedium )
	{
		wmLog(precitec::eDebug, "Filter '%s': %s.\n", m_oFilterName.c_str(), FILTERBESCHREIBUNG.c_str());
	}
}



void displacement::paint() {
	if(m_oVerbosity < eLow || inputIsInvalid(m_oXOut) ||  inputIsInvalid(m_oXOut) || m_oSpTrafo.isNull()){
		return;
	} // if

	const Trafo		&rTrafo			( *m_oSpTrafo );
	OverlayCanvas	&rCanvas		( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerPosition	( rCanvas.getLayerPosition());

	const Point		oPointOut	(roundToT<int>( m_oXOut.getData().front() ), roundToT<int>(  m_oYOut.getData().front()) );
	rLayerPosition.add( new OverlayPoint(rTrafo(oPointOut), Color::Red()) );
} // paint



bool displacement::subscribe(BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.type() == typeid(GeoDoublearray) ) {
		if (p_rPipe.tag() == "position_x")
			m_pPipeArrayX  = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray > * >(&p_rPipe);
		else if (p_rPipe.tag() == "position_y")
			m_pPipeArrayY  = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray > * >(&p_rPipe);
	} // if

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
}



void displacement::proceedGroup(const void* p_pSender, PipeGroupEventArgs& p_rArgs )
{
	poco_assert_dbg(m_pPipeArrayX != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeArrayY != nullptr); // to be asserted by graph editor


	if( m_oVerbosity >= eMedium )
	{
		std::printf("targetX=%g\n", m_oTargetX);
		std::printf("targetY=%g\n", m_oTargetY);
	}

	// Depending on input pipe, convert everything to a single GeoPoint.

	const GeoDoublearray		&rGeoPointX		= m_pPipeArrayX->read(m_oCounter);
	const GeoDoublearray		&rGeoPointY		= m_pPipeArrayY->read(m_oCounter);

	const std::vector<double>	&rPointDataX	= rGeoPointX.ref().getData();
	const std::vector<double>	&rPointDataY	= rGeoPointY.ref().getData();

	const std::vector<int>		&rPointRankX	= rGeoPointX.ref().getRank();

	m_oSpTrafo	= rGeoPointX.context().trafo();

	if (rPointDataX.size() != 1) { // result is always one point
		wmLog(eDebug, "Filter '%s': Received %u X values. X values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rPointDataX.size());
	}
	if (rPointDataY.size() != 1) { // result is always one point
		wmLog(eDebug, "Filter '%s': Received %u X values. Y values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rPointDataY.size());
	}
	if (rGeoPointX.context() != rGeoPointY.context()) { // contexts expected to be equal
		std::ostringstream oMsg;
		oMsg << m_oFilterName << ": Different contexts for x and y value: '" << rGeoPointX.context() << "', '" << rGeoPointY.context() << "'\n";
		wmLog(eWarning, oMsg.str());
	}

	Point oPoint( roundToT<int>(rPointDataX.front()), roundToT<int>(rPointDataY.front()) );
	if ( rGeoPointX.isValid() == false || rGeoPointY.isValid() == false ) {
		wmLog(eDebug, "displacement input not valid: x=%d y=%d rankx=%g ranky=%g\n", oPoint.x, oPoint.y, rGeoPointX.rank(), rGeoPointY.rank() );
	} // if

	// print input data

	if( m_oVerbosity >= eMedium ) {
		wmLog(eDebug, "displacement input: x=%d y=%d rankx=%g ranky=%g\n", oPoint.x, oPoint.y, rGeoPointX.rank(), rGeoPointY.rank() );
	} // if

	// Now move the point

	oPoint.x  -= roundToT<int>(m_oTargetX);
	oPoint.x  -= roundToT<int>(0.0);

	// print output data

	if( m_oVerbosity >= eMedium ) {
		wmLog(eDebug, "displacement output: xout=%d yout=%d rankout=%g\n", oPoint.x, oPoint.y, rGeoPointX.rank(), rGeoPointY.rank() );
	} // if

	m_oXOut[0]	=	std::make_pair(oPoint.x, rPointRankX.front());
	m_oYOut[0]	=	std::make_pair(oPoint.y, eRankMin); // y always zero
	const GeoDoublearray	oGeoDoublearrayX(rGeoPointX.context(), m_oXOut, rGeoPointX.analysisResult(), rGeoPointX.rank());
	const GeoDoublearray	oGeoDoublearrayY(rGeoPointY.context(), m_oYOut, rGeoPointY.analysisResult(), rGeoPointY.rank());

	preSignalAction();
	m_pPipeOutPosX->signal( oGeoDoublearrayX );
	m_pPipeOutPosY->signal( oGeoDoublearrayY );
}



} // namespace filter
} // namespace precitec

