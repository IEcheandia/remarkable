/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		2015
 * 	@brief 		This filter determines the top and lower boundaries of the laserline
 */

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include <filter/algoArray.h>
#include "module/moduleLogger.h"
// local includes
#include "boundaryTracking.h"

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string BoundaryTracking::m_oFilterName 		= std::string("BoundaryTracking");
const std::string BoundaryTracking::PIPENAME_OUT_TOPLINE	= std::string("TopLineOut");
const std::string BoundaryTracking::PIPENAME_OUT_BOTTOMLINE	= std::string("BottomLineOut");
const std::string BoundaryTracking::PIPENAME_OUT_CENTERLINE	= std::string("CenterLineOut");


BoundaryTracking::BoundaryTracking() :
	TransformFilter( BoundaryTracking::m_oFilterName, Poco::UUID{"6DB60993-93B0-4B6B-A48D-7D8BCF823584"} ),
	m_pPipeInImageFrame( NULL ),
	m_pPipeInLaserLine( NULL ),
	m_pPipeOutTopLine( NULL ),
	m_pPipeOutBottomLine( NULL ),
	m_pPipeOutCenterLine( NULL ),
	m_oMode( 20 ),
	m_oThreshold( 255 )
{
	m_pPipeOutTopLine = new SynchronePipe< GeoVecDoublearray >( this, BoundaryTracking::PIPENAME_OUT_TOPLINE );
	m_pPipeOutBottomLine = new SynchronePipe< GeoVecDoublearray >( this, BoundaryTracking::PIPENAME_OUT_BOTTOMLINE );
	m_pPipeOutCenterLine = new SynchronePipe< GeoVecDoublearray >( this, BoundaryTracking::PIPENAME_OUT_CENTERLINE );

	// Set default values of the parameters of the filter
	parameters_.add("Mode", Parameter::TYPE_int, m_oMode);
	parameters_.add("Threshold",    Parameter::TYPE_int, m_oThreshold);

    setInPipeConnectors({{Poco::UUID("46CAA106-66F2-4C8B-95DA-CDC204509228"), m_pPipeInLaserLine, "LaserLineIn", 1, "LaserLine"},
    {Poco::UUID("2B660848-690C-49DF-8ACF-107E55DBA0A9"), m_pPipeInImageFrame, "ImageFrameIn", 1, "ImageFrame"}});
    setOutPipeConnectors({{Poco::UUID("AC1A21A1-459C-41BF-B3DB-E04C4ADE6239"), m_pPipeOutTopLine, PIPENAME_OUT_TOPLINE, 0, ""},
    {Poco::UUID("B99DB54E-3335-42A9-AFCC-7A9801CB12D0"), m_pPipeOutBottomLine, PIPENAME_OUT_BOTTOMLINE, 0, ""},
    {Poco::UUID("1D67E8D0-90A5-4FCF-84CC-2748CC744275"), m_pPipeOutCenterLine, PIPENAME_OUT_CENTERLINE, 0, ""}});
    setVariantID(Poco::UUID("15FEB251-6B8F-449E-A0F2-273736E3576D"));
}

BoundaryTracking::~BoundaryTracking()
{
	delete m_pPipeOutTopLine;
	delete m_pPipeOutBottomLine;
	delete m_pPipeOutCenterLine;
} // ~LineProfile

void BoundaryTracking::setParameter()
{
	TransformFilter::setParameter();
	m_oMode = parameters_.getParameter("Mode").convert<int>();
	m_oThreshold = parameters_.getParameter("Threshold").convert<int>();
} // setParameter

bool BoundaryTracking::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.type() == typeid(GeoVecDoublearray) )
		m_pPipeInLaserLine  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);
	if ( p_rPipe.type() == typeid(ImageFrame) )
		m_pPipeInImageFrame  = dynamic_cast< SynchronePipe < ImageFrame > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe

void BoundaryTracking::paint()
{
	if(m_oVerbosity < eLow || m_oSpTrafo.isNull())
	{
		return;
	} // if

	const Trafo		&rTrafo			( *m_oSpTrafo );
	OverlayCanvas	&rCanvas		( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerContour	( rCanvas.getLayerContour());

	if (!m_paintAvailable) return;

	int size = m_paintPosTopVec.size();
	size = (size<(int)m_paintPosBottomVec.size()) ? size : (int)m_paintPosBottomVec.size();
	size = (size<(int)m_paintPosCenterVec.size()) ? size : (int)m_paintPosCenterVec.size();
	size = (size<(int)m_paintRankVec.size())      ? size : (int)m_paintRankVec.size();

	for (int x = 0; x < size; x++)
	{
		int oTopLineY = int(m_paintPosTopVec[x]);
		int oBottomLineY = int(m_paintPosBottomVec[x]);
		int oCenterLineY = int(m_paintPosCenterVec[x]);
		int rank = m_paintRankVec[x];

		if (rank<= 0) continue;

		rLayerContour.add( new OverlayPoint(rTrafo(Point(x, oTopLineY )), Color::Blue()) );
		rLayerContour.add( new OverlayPoint(rTrafo(Point(x, oBottomLineY )), Color::Red()) );
		rLayerContour.add( new OverlayPoint(rTrafo(Point(x, oCenterLineY )), Color::Cyan()) );
	}
} // paint

void BoundaryTracking::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeInLaserLine != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor

	m_paintAvailable = false;

	// Read out image frame from pipe
	const ImageFrame& rFrameIn = m_pPipeInImageFrame->read(m_oCounter);
	m_oSpTrafo	= rFrameIn.context().trafo();
	// Extract actual image and size
	const BImage &rImageIn = rFrameIn.data();
	// Read-out laserline
	const GeoVecDoublearray& rLaserLineIn = m_pPipeInLaserLine->read(m_oCounter);
	m_oSpTrafo	= rLaserLineIn.context().trafo();
	// And extract byte-array
	const VecDoublearray& rLaserarray = rLaserLineIn.ref();
	// input validity check

	if ( inputIsInvalid(rLaserLineIn) )
	{
		const GeoVecDoublearray &rGeoTopLine = GeoVecDoublearray( rFrameIn.context(), m_oTopLineOut, rFrameIn.analysisResult(), interface::NotPresent );
		const GeoVecDoublearray &rGeoBottomLine = GeoVecDoublearray( rFrameIn.context(), m_oBottomLineOut, rFrameIn.analysisResult(), interface::NotPresent );
		const GeoVecDoublearray &rGeoCenterLine = GeoVecDoublearray( rFrameIn.context(), m_oCenterLineOut, rFrameIn.analysisResult(), interface::NotPresent );
		preSignalAction();
		m_pPipeOutTopLine->signal(rGeoTopLine);
		m_pPipeOutBottomLine->signal(rGeoBottomLine);
		m_pPipeOutCenterLine->signal(rGeoCenterLine);

		return;
	}

	// Now do the actual image processing
	calcLines( rImageIn, rLaserarray, m_oMode, m_oThreshold, m_oTopLineOut, m_oBottomLineOut, m_oCenterLineOut);

	// Create a new byte array, and put the global context into the resulting profile
	const auto oAnalysisResult	= rFrameIn.analysisResult() == AnalysisOK ? AnalysisOK : rFrameIn.analysisResult(); // replace 2nd AnalysisOK by your result type
	const GeoVecDoublearray &rGeoTopLine = GeoVecDoublearray(rFrameIn.context(), m_oTopLineOut, oAnalysisResult, filter::eRankMax );
	const GeoVecDoublearray &rGeoBottomLine = GeoVecDoublearray(rFrameIn.context(), m_oBottomLineOut, oAnalysisResult, filter::eRankMax );
	const GeoVecDoublearray &rGeoCenterLine = GeoVecDoublearray(rFrameIn.context(), m_oCenterLineOut, oAnalysisResult, filter::eRankMax );
	preSignalAction();
	m_pPipeOutTopLine->signal(rGeoTopLine);
	m_pPipeOutBottomLine->signal(rGeoBottomLine);
	m_pPipeOutCenterLine->signal(rGeoCenterLine);

} // proceedGroup

void BoundaryTracking::calcLines( const image::BImage &p_rImageIn, const geo2d::VecDoublearray &p_rLaserLineIn, int p_oMode, int p_oThreshold,
	geo2d::VecDoublearray & p_rTopLineOut, geo2d::VecDoublearray & p_rBottomLineOut, geo2d::VecDoublearray & p_rCenterLineOut)
{
	const unsigned int	oNbLines	= p_rLaserLineIn.size();

	p_rTopLineOut.resize(oNbLines); // if the size of the output signal is not equal to the input line size, resize
	p_rBottomLineOut.resize(oNbLines); // if the size of the output signal is not equal to the input line size, resize
	p_rCenterLineOut.resize(oNbLines); // if the size of the output signal is not equal to the input line size, resize

	for (unsigned int lineN = 0; lineN < oNbLines; ++lineN)
	{ // loop over N lines
		// get the references to the stl vectors
		auto& rTopLineOut_Data = p_rTopLineOut[lineN].getData();
		auto& rTopLineOut_Rank = p_rTopLineOut[lineN].getRank();

		auto& rBottomLineOut_Data = p_rBottomLineOut[lineN].getData();
		auto& rBottomLineOut_Rank = p_rBottomLineOut[lineN].getRank();

		auto& rCenterLineOut_Data = p_rCenterLineOut[lineN].getData();
		auto& rCenterLineOut_Rank = p_rCenterLineOut[lineN].getRank();

		const auto& rLaserLineIn_Data = p_rLaserLineIn[lineN].getData();
		const auto& rLaserLineIn_Rank = p_rLaserLineIn[lineN].getRank();

		// if the size of the TopLine is not equal to the laser line size, resize the profile
		if ( rTopLineOut_Data.size() != rLaserLineIn_Data.size() )
		{
			rTopLineOut_Data.resize( rLaserLineIn_Data.size() );
			rTopLineOut_Rank.resize( rLaserLineIn_Rank.size() );
		}
		std::copy( rLaserLineIn_Rank.begin(), rLaserLineIn_Rank.end(), rTopLineOut_Rank.begin() );

		// if the size of the BottomLine is not equal to the laser line size, resize the profile
		if ( rBottomLineOut_Data.size() != rLaserLineIn_Data.size() )
		{
			rBottomLineOut_Data.resize( rLaserLineIn_Data.size() );
			rBottomLineOut_Rank.resize( rLaserLineIn_Rank.size() );
		}
		std::copy( rLaserLineIn_Rank.begin(), rLaserLineIn_Rank.end(), rBottomLineOut_Rank.begin() );

		// if the size of the CenterLine is not equal to the laser line size, resize the profile
		if ( rCenterLineOut_Data.size() != rLaserLineIn_Data.size() )
		{
			rCenterLineOut_Data.resize( rLaserLineIn_Data.size() );
			rCenterLineOut_Rank.resize( rLaserLineIn_Rank.size() );
		}
		std::copy( rLaserLineIn_Rank.begin(), rLaserLineIn_Rank.end(), rCenterLineOut_Rank.begin() );

		if (lineN == 0)
		{
			m_paintRankVec.clear();
			m_paintPosTopVec.clear();
			m_paintPosBottomVec.clear();
			m_paintPosCenterVec.clear();
		}

		for (unsigned int x=0; x<rLaserLineIn_Data.size(); x++)
		{
			m_paintAvailable = true;

			int oLineY = int(rLaserLineIn_Data[x]);
			int rank = rLaserLineIn_Rank[x];
			if (lineN == 0) m_paintRankVec.push_back(rank);

			rTopLineOut_Data[x] = oLineY;
			rBottomLineOut_Data[x] = oLineY;
			rCenterLineOut_Data[x] = oLineY;

			if (rank <= 0) continue;

			// nach oben laufen
			for (int y=oLineY; y>=0; y--)
			{
				rTopLineOut_Data[x] = y;

				if (p_rImageIn[y][x] < p_oThreshold) break;	// wird Laserlinien-Schwellwert nicht erreicht abbrechen, ansonsten weiter.
			}
			if (lineN == 0) m_paintPosTopVec.push_back((int)(0.5+rTopLineOut_Data[x]));

			// nach unten laufen
			for (int y=oLineY; y<p_rImageIn.height(); y++)
			{
				rBottomLineOut_Data[x] = y;

				if (p_rImageIn[y][x] < p_oThreshold) break;	// wird Laserlinien-Schwellwert nicht erreicht abbrechen, ansonsten weiter.
			}
			if (lineN == 0) m_paintPosBottomVec.push_back((int)(0.5+rBottomLineOut_Data[x]));

			rCenterLineOut_Data[x]  = (rTopLineOut_Data[x]  + rBottomLineOut_Data[x]) / 2;
			if (lineN == 0) m_paintPosCenterVec.push_back((int)(0.5+rCenterLineOut_Data[x]));
		}

	} // for

} // extractLineProfile



} // namespace precitec
} // namespace filter
