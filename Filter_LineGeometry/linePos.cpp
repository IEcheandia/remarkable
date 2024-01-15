/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Stefan Birmanns (SB), HS
 * 	@date		2011
 * 	@brief 		Extracts an element of a laser-line and returns it as a position.
 */

#include "linePos.h"

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include <filter/algoArray.h>
#include <filter/algoPoint.h>
#include "module/moduleLogger.h"
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string LinePos::m_oFilterName 		= std::string("LinePos");
const std::string LinePos::PIPENAME_OUT1	= std::string("PositionX");
const std::string LinePos::PIPENAME_OUT2	= std::string("PositionY");

LinePos::LinePos() :
	TransformFilter( LinePos::m_oFilterName, Poco::UUID{"30BC4691-B38D-4F04-BC51-71552A9582B6"} ),
	m_pPipeLineIn	(NULL),
	m_pPipePosXIn	(NULL),
	m_pPipePosYIn	(NULL),
	m_pPipePosXOut	(NULL),
	m_pPipePosYOut	(NULL),
    m_oStartPos     (0),
    m_oStartDelta   (10),
    m_useBadRankPoint(false),
	m_oOut			(1)
{
	m_pPipePosXOut = new SynchronePipe< GeoDoublearray >( this, LinePos::PIPENAME_OUT1 );
	m_pPipePosYOut = new SynchronePipe< GeoDoublearray >( this, LinePos::PIPENAME_OUT2 );

    // Set default values for the parameters of the filter
    parameters_.add("StartPos", Parameter::TYPE_int, m_oStartPos);
    parameters_.add("StartDeltaX", Parameter::TYPE_int, m_oStartDelta);
    parameters_.add("UseBadRankPoint", Parameter::TYPE_bool, m_useBadRankPoint);

    setInPipeConnectors({{Poco::UUID("798707E2-6742-4E77-852A-5FC9F5617DE7"), m_pPipeLineIn, "Line", 1, ""},
    {Poco::UUID("5C93C148-0D40-4DF7-A88D-DF56C3F49637"), m_pPipePosXIn, "PositionX", 1, "position_x"},
    {Poco::UUID("EA0BF412-C032-47f1-81C1-207AEFBFBCE1"), m_pPipePosYIn, "PositionY", 1, "position_y"}});
    setOutPipeConnectors({{Poco::UUID("E2EB9BA5-C8EB-4DA8-9D31-4A8FBF5C4B93"), m_pPipePosXOut, PIPENAME_OUT1, 0, ""},
    {Poco::UUID("61FA9524-D897-4298-8306-D5EFF1290A12"), m_pPipePosYOut, PIPENAME_OUT2, 0, ""}});
    setVariantID(Poco::UUID("C3B5B9CB-79EB-417B-B3FA-8628DD802134"));
} // LinePos



LinePos::~LinePos()
{
	delete m_pPipePosXOut;
	delete m_pPipePosYOut;

} // ~LinePos



void LinePos::setParameter()
{
	TransformFilter::setParameter();
    m_oStartPos   = parameters_.getParameter("StartPos").convert<int>();
    m_oStartDelta = parameters_.getParameter("StartDeltaX").convert<int>();
    m_useBadRankPoint = parameters_.getParameter("UseBadRankPoint").convert<bool>();
} // setParameter



void LinePos::paint()
{
	if (m_oVerbosity < eLow || m_oOut.empty() || m_oSpTrafo.isNull())
	{
		return;
	} // if

	// Read-out signal
	const GeoVecDoublearray& rLineIn = m_pPipeLineIn->read(m_oCounter);
	// Get context
	SmpTrafo oSTrafo( rLineIn.context().trafo() );
	// Get the canvas and layer
	const Trafo		&rTrafo			( *m_oSpTrafo );
	OverlayCanvas	&rCanvas		( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerPosition	( rCanvas.getLayerPosition());

	// Draw a cross at the position
	Point oPoint(static_cast<int>(m_oOut.getData()[0].x), static_cast<int>(m_oOut.getData()[0].y)); /*draw first only*/
	rLayerPosition.add<OverlayCross>(rTrafo(oPoint), Color::Red());

} // paint



bool LinePos::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.type() == typeid(GeoVecDoublearray) )
		m_pPipeLineIn  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);
	else if ( p_rPipe.type() == typeid(GeoDoublearray) ) {
		if (p_rPipe.tag() == "position_x")
			m_pPipePosXIn  = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray > * >(&p_rPipe);
		else if (p_rPipe.tag() == "position_y")
			m_pPipePosYIn  = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray > * >(&p_rPipe);
	} // if

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void LinePos::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeLineIn != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipePosXIn != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipePosYIn != nullptr); // to be asserted by graph editor


	// Read-out laserline
	const GeoVecDoublearray&	rGeoLineIn 			= m_pPipeLineIn->read(m_oCounter);
	const GeoDoublearray	&rGeoPosXIn 			= m_pPipePosXIn->read(m_oCounter);
	const GeoDoublearray	&rGeoPosYIn 			= m_pPipePosYIn->read(m_oCounter);
	m_oSpTrafo	= rGeoLineIn.context().trafo();

	const VecDoublearray&		rIArrayIn 			= rGeoLineIn.ref();
	const Doublearray&		rPosXArrayIn 		= rGeoPosXIn.ref();
	const Doublearray&		rPosYArrayIn 		= rGeoPosYIn.ref();
	// input validity check

    auto emptyInput = [&]()
    {
        return rIArrayIn.empty() || rPosXArrayIn.empty() || rPosYArrayIn.empty();
    };
    auto badGeoRankInput = [&]()
    {
        return (rGeoLineIn.rank() == NotPresent || rGeoPosXIn.rank() == NotPresent || rGeoPosYIn.rank() == NotPresent);
    };

    //invalid input is too strict, we evaluate it only if UseBadRankPoint is false
    auto invalidInput = [&]()
    {
        return (inputIsInvalid(rGeoLineIn) || inputIsInvalid(rGeoPosXIn) || inputIsInvalid(rGeoPosYIn));
    };

    if (emptyInput() || m_useBadRankPoint ? badGeoRankInput() : invalidInput())
    {
        m_oSpTrafo.reset(); //disable paint
		const GeoDoublearray &rGeoDoublearrayX = GeoDoublearray( rGeoLineIn.context(), getCoordinate(m_oOut, eX), rGeoLineIn.analysisResult(), interface::NotPresent );
		const GeoDoublearray &rGeoDoublearrayY = GeoDoublearray( rGeoLineIn.context(), getCoordinate(m_oOut, eY), rGeoLineIn.analysisResult(), interface::NotPresent );
		preSignalAction();
		m_pPipePosXOut->signal( rGeoDoublearrayX );
		m_pPipePosYOut->signal( rGeoDoublearrayY );

		return; // RETURN
	}

	const unsigned int	oNbLines	= rIArrayIn.size();
	m_oOut.assign(oNbLines); // if the size of the output signal is not equal to the input line size, resize
	for (unsigned int lineN = 0; lineN < oNbLines; ++lineN) { // loop over N lines

		const auto&		rLineIn_Data 	= rIArrayIn[lineN].getData();
		const auto&		rLineIn_Rank 	= rIArrayIn[lineN].getRank();
        const Point& rPointInData = Point(roundToT<int>(rPosXArrayIn.getData()[lineN]),
                                          roundToT<int>(rPosYArrayIn.getData()[lineN]));
        auto pointInRank = rPosXArrayIn.getRank()[lineN]; //the rank of posY is not important, it's used only in case of bad input

		DPoint&						rPointOutData	= m_oOut.getData()[lineN];
		int&						rPointOutRank	= m_oOut.getRank()[lineN];

		// calculate the laserline global x coordinate
		int oLineOffset  = rGeoLineIn.context().trafo()->dx();
		int oPointOffset = rGeoPosXIn.context().trafo()->dx(); // trafo for x in and y in equal
		oPointOffset = oPointOffset + rPointInData.x - oLineOffset;

        // Calculate 'Start position' in internal ROI
        const int oStartPosROI = m_oStartPos - oLineOffset;

        // If it's the first image check for 'Start position' values
        if (    (m_oStartPos > 0)
             && (rGeoLineIn.context().imageNumber() == 0)
           )
        {
            // It's the first image and the given 'Start position' is to be checked
            // Actual x position not more than 'StartDeltaX' away from 'Start position'?
            //   => yes:  use actual x position
            //   => no:   use the given 'Start position' as x position
            if ( abs(oPointOffset - oStartPosROI) > m_oStartDelta )
            {
                oPointOffset = oStartPosROI;
            }
        }

		if (oPointOffset < (int)(rLineIn_Data.size()) && oPointOffset >= 0)
		{
			rPointOutData.x = oPointOffset;
			rPointOutData.y = rLineIn_Data[ oPointOffset ] ;

            rPointOutRank = std::min(rLineIn_Rank[oPointOffset], pointInRank);
/*
			// check rank and if point invalid, give warning
			if (rLineIn_Rank[ oPointOffset ] <= 0)
			{
				std::cout << "LinePos: Warning, point on line not valid!" << std::endl;
				std::cout << "LinePos: Position-X: " << rPointOutData.x << std::endl;
				std::cout << "LinePos: Position-Y: " << rPointOutData.y << std::endl;
			}
*/
		} else {

			rPointOutData.x = rPointInData.x;
			rPointOutData.y = rPointInData.y;
		}
	} // for

	// Create a new point and put the global context into the resulting profile
	const auto oAnalysisResult	= rGeoLineIn.analysisResult() == AnalysisOK ? AnalysisOK : rGeoLineIn.analysisResult(); // replace 2nd AnalysisOK by your result type
	const GeoDoublearray &rGeoDoublearrayX = GeoDoublearray( rGeoLineIn.context(), getCoordinate(m_oOut, eX), oAnalysisResult, interface::Limit );
	const GeoDoublearray &rGeoDoublearrayY = GeoDoublearray( rGeoLineIn.context(), getCoordinate(m_oOut, eY), oAnalysisResult, interface::Limit );
	preSignalAction();
	m_pPipePosXOut->signal( rGeoDoublearrayX );
	m_pPipePosYOut->signal( rGeoDoublearrayY );

} // proceedGroup



} // namespace precitec
} // namespace filter
