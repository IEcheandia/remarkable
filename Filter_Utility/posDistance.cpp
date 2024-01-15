/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2014
 * 	@brief		This filter computes the 3d distance of two 2d points.
 */

// todo: compute rank correctly from all input data elements
// todo: compute analysisResult from all input data elements
// todo: this filter does not work correctly yet, it only approximates the distance for a simple coax setup.

#include "posDistance.h"
#include "util/calibDataSingleton.h"
#include <math/3D/projectiveMathStructures.h>
#include <filter/algoArray.h>
#include <overlay/overlayCanvas.h>
#include <overlay/overlayPrimitive.h>
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
using namespace geo2d;
using namespace interface;
using namespace image;
namespace filter {

const std::string PosDistance::m_oFilterName	("PosDistance");
const std::string PosDistance::m_oPipeOutXName	("DistanceX");
const std::string PosDistance::m_oPipeOutZName	("DistanceZ");


PosDistance::PosDistance() :
		TransformFilter	( PosDistance::m_oFilterName, Poco::UUID{"F661AC15-970C-4C9C-9899-402773CDC92A"} ),
		m_pPipeInPosAX	( nullptr ),
		m_pPipeInPosAY	( nullptr ),
		m_pPipeInPosBX	( nullptr ),
		m_pPipeInPosBY	( nullptr ),
		m_oPipeOutDistX	( this, PosDistance::m_oPipeOutXName ),
		m_oPipeOutDistZ(this, PosDistance::m_oPipeOutZName),
		m_oTypeOfLaserLine(LaserLine::FrontLaserLine)
{
	parameters_.add("TypeOfLaserLine", fliplib::Parameter::TYPE_int, int(m_oTypeOfLaserLine));

    setInPipeConnectors({{Poco::UUID("BB66513F-2796-4EE8-A28E-8EEE42C56DA0"), m_pPipeInPosAX, "PositionAX", 1, "pos_a_x"},
    {Poco::UUID("180A4CD2-E261-4DF6-B644-DB4F9AF44EE5"), m_pPipeInPosAY, "PositionAY", 1, "pos_a_y"},
    {Poco::UUID("C3E94E68-8EBC-4A99-BF7D-1FAAD521B18F"), m_pPipeInPosBX, "PositionBX", 1, "pos_b_x"},
    {Poco::UUID("736D9847-D478-46BC-8811-2C1978536154"), m_pPipeInPosBY, "PositionBY", 1, "pos_b_y"}});
    setOutPipeConnectors({{Poco::UUID("59997DEF-6AB2-4847-B9B6-8E28932B7C75"), &m_oPipeOutDistX, m_oPipeOutXName, 0, "dist_x"},
    {Poco::UUID("A8F1696D-A9F8-47F3-BA0D-4C31C746DE80"), &m_oPipeOutDistZ, m_oPipeOutZName, 0, "dist_z"}});
    setVariantID(Poco::UUID("F4C21540-083B-4F46-97D6-9923D166D9CC"));
} // CTor.



/*virtual*/ PosDistance::~PosDistance()
{

} // DTor.



void PosDistance::setParameter()
{
	TransformFilter::setParameter();
	m_oTypeOfLaserLine = castToLaserLine(parameters_.getParameter("TypeOfLaserLine").convert<int>());

} // SetParameter



bool PosDistance::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.tag() == "pos_a_x" )
		m_pPipeInPosAX = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	if ( p_rPipe.tag() == "pos_a_y" )
		m_pPipeInPosAY = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	if ( p_rPipe.tag() == "pos_b_x" )
		m_pPipeInPosBX = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	if ( p_rPipe.tag() == "pos_b_y" )
		m_pPipeInPosBY = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void PosDistance::paint()
{
	if( m_oVerbosity < eLow || m_pTrafoA.isNull()|| m_pTrafoB.isNull() )
	{
		return;
	} // if

	const Trafo			&rTrafoA		( *m_pTrafoA );
	const Trafo			&rTrafoB		( *m_pTrafoB );
	OverlayCanvas		&rCanvas		( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer		&rLayerLine		( rCanvas.getLayerLine() );
	OverlayLayer		&rLayerPosition	( rCanvas.getLayerPosition() );

	// draw cross at point A and B
	const Point			oPointA			( m_oPointA.x, m_oPointA.y );
	rLayerPosition.add( new OverlayCross( rTrafoA(oPointA), Color::Green()) );
	const Point			oPointB			( m_oPointB.x, m_oPointB.y );
	rLayerPosition.add( new OverlayCross( rTrafoB(oPointB), Color::Green()) );

	if(m_oVerbosity < eMedium){
		return;
	} // if

	// connect both points
	rLayerLine.add( new OverlayLine(rTrafoA(oPointA), rTrafoB(oPointB), Color::Green()) );

} // paint



void PosDistance::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEvent)
{
	poco_assert_dbg( m_pPipeInPosAX != nullptr );
	poco_assert_dbg( m_pPipeInPosAY != nullptr );
	poco_assert_dbg( m_pPipeInPosBX != nullptr );
	poco_assert_dbg( m_pPipeInPosBY != nullptr );

	// read first position from in pipe
	const GeoDoublearray	&rGeoPosAXIn	( m_pPipeInPosAX->read(m_oCounter) );
	const GeoDoublearray	&rGeoPosAYIn	( m_pPipeInPosAY->read(m_oCounter) );
	const Doublearray		&rPosAXIn		( rGeoPosAXIn.ref() );
	const Doublearray		&rPosAYIn		( rGeoPosAYIn.ref() );
	const ImageContext		&rContextX		( rGeoPosAXIn.context() );
	const ImageContext		&rContextY		( rGeoPosAYIn.context() );

	// read second position from in pipe
	const GeoDoublearray	&rGeoPosBXIn	( m_pPipeInPosBX->read(m_oCounter) );
	const GeoDoublearray	&rGeoPosBYIn	( m_pPipeInPosBY->read(m_oCounter) );
	const Doublearray		&rPosBXIn		( rGeoPosBXIn.ref() );
	const Doublearray		&rPosBYIn		( rGeoPosBYIn.ref() );

    if (rPosAXIn.size() == 0 || rPosAYIn.size() == 0 || rPosBXIn.size() == 0 || rPosBYIn.size() == 0)
    {
        int oRank = eRankMin;

        // signal result
        geo2d::Doublearray oXOut; oXOut.assign( 1, 0, oRank );
        geo2d::Doublearray oZOut; oZOut.assign( 1, 0, oRank );

        const GeoDoublearray oGeoOutX( rContextX, oXOut, rGeoPosAXIn.analysisResult(), rGeoPosAXIn.rank() );
        const GeoDoublearray oGeoOutZ( rContextY, oZOut, rGeoPosAYIn.analysisResult(), rGeoPosAYIn.rank() );

        m_pTrafoA = nullptr;
        m_pTrafoB = nullptr;
        preSignalAction();
        m_oPipeOutDistX.signal( oGeoOutX );
        m_oPipeOutDistZ.signal( oGeoOutZ );
        return;

    }

	m_oPointA.x	= (int)(rPosAXIn.getData().front());
	m_oPointA.y	= (int)(rPosAYIn.getData().front());
	m_oPointB.x	= (int)(rPosBXIn.getData().front());
	m_oPointB.y	= (int)(rPosBYIn.getData().front());

	// get hw roi from context
	m_oHwRoi.x	= rContextX.HW_ROI_x0;
	m_oHwRoi.y	= rContextX.HW_ROI_y0;

	// sanity checks
	if (rPosAXIn.size() != 1 || rPosBXIn.size() != 1 || rPosAYIn.size() != 1 || rPosBYIn.size() != 1) {
		wmLog(eDebug, "Filter '%s': Received multiple incoming data items, expected only one per pipe. Will process first element, rest will be discarded.\n", m_oFilterName.c_str() );
	} // if
	if (rGeoPosAXIn.context() != rGeoPosAYIn.context() || rGeoPosBXIn.context() != rGeoPosBYIn.context()) {
		wmLog(eWarning, "Filter '%s': Incoming data has different contexts?\n", m_oFilterName.c_str() );
	} // if

	// calculate sensor coordinates
	m_pTrafoA = rGeoPosAXIn.context().trafo();
	Point oPointA( doubleToInt(rPosAXIn).getData().front(), doubleToInt(rPosAYIn).getData().front() );
	oPointA.x = oPointA.x + m_pTrafoA->dx() + m_oHwRoi.x;
	oPointA.y = oPointA.y + m_pTrafoA->dy() + m_oHwRoi.y;

	m_pTrafoB = rGeoPosBXIn.context().trafo();
	Point oPointB( doubleToInt(rPosBXIn).getData().front(), doubleToInt(rPosBYIn).getData().front() );
	oPointB.x = oPointB.x + m_pTrafoB->dx() + m_oHwRoi.x;
	oPointB.y = oPointB.y + m_pTrafoB->dy() + m_oHwRoi.y;

	auto &rCalib( system::CalibDataSingleton::getCalibrationCoords(math::SensorId::eSensorId0) );

	const math::Vec3D oCoordA = rCalib.to3D(oPointA.x, oPointA.y, m_oTypeOfLaserLine);
	const math::Vec3D oCoordB = rCalib.to3D(oPointB.x, oPointB.y, m_oTypeOfLaserLine);

	// compute distance
	double oDeltaMmOutX( std::abs(oCoordB[0] - oCoordA[0]) );
	double oDeltaMmOutZ( std::abs(oCoordB[2] - oCoordA[2]) );

	int oRank = eRankMax;

	// signal result
	geo2d::Doublearray oXOut; oXOut.assign( 1, oDeltaMmOutX, oRank );
	geo2d::Doublearray oZOut; oZOut.assign( 1, oDeltaMmOutZ, oRank );

	const GeoDoublearray oGeoOutX( rContextX, oXOut, rGeoPosAXIn.analysisResult(), rGeoPosAXIn.rank() );
	const GeoDoublearray oGeoOutZ( rContextY, oZOut, rGeoPosAYIn.analysisResult(), rGeoPosAYIn.rank() );

	preSignalAction();
	m_oPipeOutDistX.signal( oGeoOutX );
	m_oPipeOutDistZ.signal( oGeoOutZ );

} // proceedGroup


} // namespace filter
} // namespace precitec
