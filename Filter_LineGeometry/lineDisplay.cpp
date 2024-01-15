/**
 *	@file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Stefan Birmanns (SB), HS
 *  @date		2011
 *  @brief		Simple display filter for laser-line objects.
 */

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "overlay/layerType.h"
#include <common/geoContext.h>
#include <filter/parameterEnums.h>		///< rank
#include <filter/algoArray.h>			///< algorithmic interface for class TArray
#include "system/typeTraits.h"
#include "module/moduleLogger.h"
// local includes
#include "lineDisplay.h"
#include "lineExtract.h"

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string LineDisplay::m_oFilterName 		= std::string("LineDisplay");
const std::string LineDisplay::PIPENAME_OUT1	= std::string("Line");



LineDisplay::LineDisplay() :
	TransformFilter( LineDisplay::m_oFilterName, Poco::UUID{"24F20394-52EA-43CE-BD96-3E8163757EDF"} ),
	m_pPipeLineIn(NULL),
	m_pPipeLineOut( NULL ),
	m_oStart( -1 ),
	m_oEnd( -1 ),
	m_oHeight( -1 ),
	m_oFrame( false ),
	m_oRed( 0 ),
	m_oGreen( 255 ),
	m_oBlue( 0 ),
	m_oCross( 0 ),
	m_oPaintLayer( 2 ),
	m_oLineOut( 1, Doublearray( 1, 0 ) )
{
	m_pPipeLineOut = new SynchronePipe< GeoVecDoublearray >( this, LineDisplay::PIPENAME_OUT1 );

	// Set default values of the parameters of the filter
	parameters_.add("Start",		Parameter::TYPE_int, m_oStart);
	parameters_.add("End",     		Parameter::TYPE_int, m_oEnd);
	parameters_.add("Height",	 	Parameter::TYPE_int, m_oHeight);
	parameters_.add("Frame",	 	Parameter::TYPE_bool, m_oFrame);
	parameters_.add("Cross",		Parameter::TYPE_bool, m_oCross);
	parameters_.add("Red", 			Parameter::TYPE_int, m_oRed);
	parameters_.add("Blue",			Parameter::TYPE_int, m_oBlue);
	parameters_.add("Green",		Parameter::TYPE_int, m_oGreen);
	parameters_.add("PaintLayer",   Parameter::TYPE_UInt32, m_oPaintLayer);

    setInPipeConnectors({{Poco::UUID("68211672-F992-4A5D-9E8C-49E2DFA86154"), m_pPipeLineIn, "Line", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("72C6C14E-C19B-4E84-B50F-FE011B38FD54"), m_pPipeLineOut, "Line", 0, ""}});
    setVariantID(Poco::UUID("A6187196-5F41-4295-A64D-424D333FC56F"));
} // LineDisplay



LineDisplay::~LineDisplay()
{
	delete m_pPipeLineOut;

} // ~LineDisplay



void LineDisplay::setParameter()
{
	TransformFilter::setParameter();
	m_oStart  		= parameters_.getParameter("Start").convert<int>();
	m_oEnd    		= parameters_.getParameter("End").convert<int>();
	m_oHeight 		= parameters_.getParameter("Height").convert<int>();
	m_oFrame  		= parameters_.getParameter("Frame").convert<bool>();
	m_oRed 			= parameters_.getParameter("Red").convert<int>();
	m_oGreen 		= parameters_.getParameter("Green").convert<int>();
	m_oBlue 		= parameters_.getParameter("Blue").convert<int>();
	m_oCross 		= parameters_.getParameter("Cross").convert<bool>();

	// sanity checks
	if (m_oEnd > 0 && m_oStart > m_oEnd)
		std::swap( m_oStart, m_oEnd );
	poco_assert_dbg(m_oPaintLayer <= eNbLayers);

} // setParameter



void LineDisplay::paint()
{
	if(m_oVerbosity < eLow || inputIsInvalid(m_oLineOut) || m_oSpTrafo.isNull()){
		return;
	} // if

	// Do we have an in-pipe?
	if (m_pPipeLineIn == NULL)
		return;

	// Get the canvas and layer
	const Trafo		&rTrafo			( *m_oSpTrafo );
	OverlayCanvas	&rCanvas		( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerLine		( rCanvas.getLayerLine());
	OverlayLayer	&rLayerContour	( rCanvas.getLayerContour());

	// Get output line and rank
	auto &rLineOut_Data = m_oLineOut.front()/*draw first only*/.getData();
	auto &rLineOut_Rank = m_oLineOut.front()/*draw first only*/.getRank();

	// Is there actually some data?
	if (rLineOut_Data.size() == 0)
		return;

	// Plot a frame around the signal
	if ( m_oFrame )
	{
		geo2d::Rect oFrame;
		oFrame.x().start() = 0;
		oFrame.y().start() = 0;
		oFrame.x().end() = rLineOut_Data.size();
		if (m_oHeight > 0)
			oFrame.y().end() = m_oHeight;
		else
			oFrame.y().end() = int( *max_element( rLineOut_Data.begin(), rLineOut_Data.end() ) );
		rLayerLine.add( new OverlayRectangle( rTrafo(oFrame), Color::Blue() ) );
	}

	static const Color	oDarkGreen(0, 150, 0);
	// Plot the signal
	Color oColor( m_oRed, m_oGreen, m_oBlue );
	for (unsigned int i = 0; i < rLineOut_Data.size(); i++)
	{
		Point oPoint( i, int( rLineOut_Data[i] ));

		if (m_oCross)
		{
			if (rLineOut_Rank[i] != eRankMin)
				rLayerContour.add( new OverlayCross( (*m_oSpTrafo)(oPoint), oColor ) );
			else
				rLayerContour.add( new OverlayCross( (*m_oSpTrafo)(oPoint), oDarkGreen ) );

		} else {

			if (rLineOut_Rank[i] != eRankMin)
				rLayerContour.add( new OverlayPoint( (*m_oSpTrafo)(oPoint), oColor ) );
			else
				rLayerContour.add( new OverlayPoint( (*m_oSpTrafo)(oPoint), oDarkGreen ) );
		}
	}


} // paint



bool LineDisplay::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeLineIn  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void LineDisplay::proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeLineIn != nullptr); // to be asserted by graph editor


	// Do we have an in-pipe?
	if (m_pPipeLineIn == NULL)
    {
        preSignalAction();
		return;
    }

	// Read-out laserline
	const GeoVecDoublearray& rLineIn = m_pPipeLineIn->read(m_oCounter);
	m_oSpTrafo = rLineIn.context().trafo();
	// And extract byte-array
	const VecDoublearray& rArrayIn = rLineIn.ref();

	// some more sanity checks before we extract the data
	int oStart = m_oStart;
	int oEnd   = m_oEnd;

	if (rArrayIn.empty() || rArrayIn.front()/*draw first only*/.getData().empty()) {
		wmLog(eDebug, "Input data not ok\n");
        preSignalAction();
		return;
	}

	// let's check what we should display
	if (m_oStart < 0)
		oStart = 0;
	if (m_oEnd < 0 || m_oEnd > (int)(rArrayIn.front()/*draw first only*/.getData().size()))
		oEnd = rArrayIn.front()/*draw first only*/.getData().size();

	// Extract the line
	LineExtract::extract( rArrayIn, oStart, oEnd, m_oLineOut );

	// Find extrema to scale output
	if (m_oHeight > 0)
	{
		auto& rLineOut_Data = m_oLineOut.front()/*draw first only*/.getData();
		auto& rLineOut_Rank = m_oLineOut.front()/*draw first only*/.getRank();
		double oMin = std::numeric_limits<double>::max();
		double oMax = std::numeric_limits<double>::lowest();
		for( unsigned int x = 0; x < rLineOut_Data.size(); x++ )
		{
			if (rLineOut_Rank[x] > 0)
			{
				if (rLineOut_Data[x] > oMax)
					oMax = rLineOut_Data[x];
				if (rLineOut_Data[x] < oMin)
					oMin = rLineOut_Data[x];
			}
		}
		float fLength = (float)(oMax - oMin);
		float fHeight = (float)(m_oHeight - 1); // height == highest y pos + 1
		for (unsigned int i = 0; i < rLineOut_Data.size(); ++i)
			if (rLineOut_Rank[i] > 0 && fLength > 0.0f)
				rLineOut_Data[i] = m_oHeight - (int)(fHeight * ((float)( rLineOut_Data[i] - oMin) / fLength));
			else
				rLineOut_Data[i] = (int)(fHeight * 0.5f);
	}

	// now adjust the context to reflect the new offset
	geo2d::Point oOffset( oStart, 0 );

	// Trafo ...
	LinearTrafo oSubTrafo(oOffset);
	// ... and generate a new context with the new trafo and the old meta information
	SmpImageContext pNewContext = new ImageContext(rLineIn.context(), oSubTrafo(m_oSpTrafo));
	// Now create a new byte array, put the global context into the resulting profile and copy the rank over
	const GeoVecDoublearray& rGeoProfile = GeoVecDoublearray(*pNewContext, m_oLineOut, AnalysisOK, rLineIn.rank());
	preSignalAction();  m_pPipeLineOut->signal( rGeoProfile );

} // proceed

} // namespace precitec
} // namespace filter
