/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2014
 * 	@brief		This filter visualizes a position using overlay primitives.
 */

#include "posDisplay.h"
// WM includes
#include <filter/algoArray.h>
#include <module/moduleLogger.h>
#include <overlay/overlayPrimitive.h>
#include <overlay/overlayPrimitive.h>
#include <overlay/layerType.h>
#include <fliplib/TypeToDataTypeImpl.h>

// std lib
#include <string>
#define _USE_MATH_DEFINES
#include <math.h>

namespace precitec {
using namespace geo2d;
using namespace interface;
using namespace image;
namespace filter {

const std::string PosDisplay::m_oFilterName 	("PosDisplay");
const std::string PosDisplay::m_oParamStyle		("Style");
const std::string PosDisplay::m_oParamColorRed	("Red");
const std::string PosDisplay::m_oParamColorGreen("Green");
const std::string PosDisplay::m_oParamColorBlue	("Blue");


void PosDisplay::paintCrosshairMedium(OverlayLayer &rLayer, const geo2d::Point & rPoint, const image::Color & rColor, const Trafo &rTrafo)
{
	geo2d::Point oPointA;
	geo2d::Point oPointB;
	std::ostringstream oStr;

	for (unsigned int i = 0; i < 360; i += 5 )
	{
		if ( i % 45 == 0 )
		{
			oPointA.x = (rPoint.x + (int) ((110.0 * cos((360.0 - i) * (M_PI / 180.0))))) - 10;
			oPointA.y = (rPoint.y + (int) ((110.0 * sin((360.0 - i) * (M_PI / 180.0))))) - 10;

			oStr.str(""); oStr << i;
			rLayer.add<OverlayText>(oStr.str(), Font(14), rTrafo(Rect(oPointA.x, oPointA.y, 100, 20)), rColor);

			oPointA.x = (rPoint.x + (int) ((25.0 * cos(i * (M_PI / 180.0)))));
			oPointA.y = (rPoint.y + (int) ((25.0 * sin(i * (M_PI / 180.0)))));
		}
		else
		{
			oPointA.x = (rPoint.x + (int) ((70.0 * cos(i * (M_PI / 180.0)))));
			oPointA.y = (rPoint.y + (int) ((70.0 * sin(i * (M_PI / 180.0)))));
		}
		oPointB.x = (rPoint.x + (int) ((100.0 * cos(i * (M_PI / 180.0)))));
		oPointB.y = (rPoint.y + (int) ((100.0 * sin(i * (M_PI / 180.0)))));

		rLayer.add<OverlayLine>(rTrafo(oPointA), rTrafo(oPointB), rColor);
	}
	rLayer.add<OverlayCross>(rTrafo(rPoint), 15, rColor);
}

void PosDisplay::paintCrosshairLarge(OverlayLayer &rLayer, const geo2d::Point & rPoint, const image::Color & rColor, const Trafo &rTrafo)
{
	geo2d::Point oPointA;
	geo2d::Point oPointB;
	std::ostringstream oStr;

	for ( unsigned int i = 0; i<360; i += 5 )
	{
		if ( (i >= 0 && i < 45) || (i > 315 && i < 360) || (i > 135 && i < 225) )
		{
			if ( i % 45 == 0 )
			{
				oPointA.x = (rPoint.x + (int) ((25.0 * cos(i * (M_PI / 180.0)))));
				oPointA.y = (rPoint.y + (int) ((25.0 * sin(i * (M_PI / 180.0)))));
			}
			else
			{
				oPointA.x = (rPoint.x + (int) ((70.0 * cos(i * (M_PI / 180.0)))));
				oPointA.y = (rPoint.y + (int) ((70.0 * sin(i * (M_PI / 180.0)))));
			}
			oPointB.x = (rPoint.x + (int) ((100.0 * cos(i * (M_PI / 180.0)))));
			oPointB.y = (rPoint.y + (int) ((100.0 * sin(i * (M_PI / 180.0)))));

			rLayer.add<OverlayLine>(rTrafo(oPointA), rTrafo(oPointB), rColor);
		}
	}
	rLayer.add<OverlayCross>(rTrafo(rPoint), 15, rColor);
}


PosDisplay::PosDisplay() :
		TransformFilter			( PosDisplay::m_oFilterName, Poco::UUID{"C5A7C583-06AF-43DD-8045-C404F8FE89C7"} ),
		m_pPipeInDataX			( nullptr ),
		m_pPipeInDataY			( nullptr ),
		m_oStyle				( 0 )
{
	parameters_.add( PosDisplay::m_oParamStyle,			fliplib::Parameter::TYPE_int,  static_cast<unsigned int>( m_oStyle ) );
	parameters_.add( PosDisplay::m_oParamColorRed,		fliplib::Parameter::TYPE_uint, static_cast<unsigned int>( m_oColor.red) );
	parameters_.add( PosDisplay::m_oParamColorGreen,	fliplib::Parameter::TYPE_uint, static_cast<unsigned int>( m_oColor.green) );
	parameters_.add( PosDisplay::m_oParamColorBlue,		fliplib::Parameter::TYPE_uint, static_cast<unsigned int>( m_oColor.blue) );

    setInPipeConnectors({{Poco::UUID("D3758CB1-D1FF-4D34-AF47-872CC6E37214"), m_pPipeInDataX, "PositionX", 1, "pos_x"},
    {Poco::UUID("F44C16D7-41D3-4A8B-A7AD-6BCA6DC97B6E"), m_pPipeInDataY, "PositionY", 1, "pos_y"}});
    setVariantID(Poco::UUID("E4D59539-ECF6-446C-B5F6-192755C58F3E"));
} // CTor.



/*virtual*/ PosDisplay::~PosDisplay()
{

} // DTor.



void PosDisplay::setParameter()
{
	TransformFilter::setParameter();

	m_oStyle 				= parameters_.getParameter( PosDisplay::m_oParamStyle 		).convert<unsigned int>();
	m_oColor.red			= parameters_.getParameter( PosDisplay::m_oParamColorRed 	).convert<byte>();
	m_oColor.green			= parameters_.getParameter( PosDisplay::m_oParamColorGreen 	).convert<byte>();
	m_oColor.blue			= parameters_.getParameter( PosDisplay::m_oParamColorBlue 	).convert<byte>();

} // SetParameter



bool PosDisplay::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.tag() == "pos_x" )
		m_pPipeInDataX = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	if ( p_rPipe.tag() == "pos_y" )
		m_pPipeInDataY = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void PosDisplay::paint()
{
	if (m_oVerbosity == eNone || m_pTrafo.isNull())  // filter should not paint anything on verbosity eNone
	{
		return;
	} // if

	const Trafo		&rTrafo		( *m_pTrafo );
	OverlayCanvas	&rCanvas	( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayer		( rCanvas.getLayerPosition());

	switch( m_oStyle )
	{
	// cross small
	case 0:
		rLayer.add<OverlayCross>( rTrafo( m_oPoint.ref() ), 25, m_oColor );
		break;

	// cross medium
	case 1:
		rLayer.add<OverlayCross>( rTrafo( m_oPoint.ref() ), 75, m_oColor );
		break;

	// cross large
	case 2:
		rLayer.add<OverlayCross>( rTrafo( m_oPoint.ref() ), 350, m_oColor );
		break;

	// crosshair medium
	case 3:
		PosDisplay::paintCrosshairMedium(rLayer, m_oPoint.ref(), m_oColor, rTrafo);
		break;

	// crosshair large
	case 4:
		PosDisplay::paintCrosshairLarge(rLayer, m_oPoint.ref(), m_oColor, rTrafo);
		break;
	}

} // paint



void PosDisplay::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEvent)
{
	poco_assert_dbg( m_pPipeInDataX != nullptr ); // to be asserted by graph editor
	poco_assert_dbg( m_pPipeInDataY != nullptr ); // to be asserted by graph editor

	if( m_oVerbosity == eNone )  // filter should not store any coordinate on verbosity eNone.
	{
        preSignalAction();
		return;
	} // if

	const GeoDoublearray	&rGeoPosXIn	( m_pPipeInDataX->read(m_oCounter) );
	const GeoDoublearray	&rGeoPosYIn	( m_pPipeInDataY->read(m_oCounter) );

	const Doublearray		&rPosXIn	( rGeoPosXIn.ref() );
	const Doublearray		&rPosYIn	( rGeoPosYIn.ref() );

	if (rPosXIn.size() != 1) // result is always one point
	{
		wmLog(eDebug, "Filter '%s': Received %u X values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rPosXIn.size());
	}
	if (rPosYIn.size() != 1) // result is always one point
	{
		wmLog(eDebug, "Filter '%s': Received %u Y values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rPosYIn.size());
	}

    if ( rPosXIn.getData().empty() || rPosYIn.getData().empty() )
    {
        m_pTrafo.assign(nullptr); //disable paint
        preSignalAction();
        return;
    }


	if (rGeoPosXIn.context() != rGeoPosYIn.context()) // contexts expected to be equal
	{ // Kann durchaus vorkommen, fabriziert zu viele Warnungen => verwirrt Kunden => rausgenommen (OS)
		//std::ostringstream oMsg;
		//oMsg << m_oFilterName << ": Different contexts for x and y value: '" << rGeoPosXIn.context() << "', '" << rGeoPosYIn.context() << "'\n";
		//wmLog(eWarning, oMsg.str());
	}

	m_pTrafo = rGeoPosXIn.context().trafo();

	const Point		oPosOut		( doubleToInt(rPosXIn).getData().front(), doubleToInt(rPosYIn).getData().front() );
	const double	oRank		( (rGeoPosXIn.rank() + rGeoPosYIn.rank()) / 2. );

	m_oPoint = GeoPoint( rGeoPosXIn.context(), oPosOut, rGeoPosXIn.analysisResult(), oRank );

    preSignalAction();
} // proceedGroup


} // namespace filter
} // namespace precitec
