/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Stefan Birmanns (SB), HS
 *  @date		2011
 *  @brief		Computes a simple gradient / forward difference of a signal.
 */

// clib includes
#include <limits.h>
// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include <filter/algoArray.h>		///< algorithmic interface for class TArray
#include "module/moduleLogger.h"
// local includes
#include "lineForwardDiff.h"
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {


const std::string LineForwardDiff::m_oFilterName 		= std::string("LineForwardDiff");
const std::string LineForwardDiff::PIPENAME_OUT1	= std::string("Line");



LineForwardDiff::LineForwardDiff() :
	TransformFilter( LineForwardDiff::m_oFilterName, Poco::UUID{"B41896F7-3BC3-4237-8AAF-27EC85A50440"} ),
	m_pPipeLineIn(NULL),
	m_pPipeLineOut( NULL ),
	m_oLineOut( 1, Doublearray( 1, 0 ) )
{
	m_pPipeLineOut = new SynchronePipe< GeoVecDoublearray >( this, LineForwardDiff::PIPENAME_OUT1 );

    setInPipeConnectors({{Poco::UUID("68E03B24-D13D-430A-A66A-CA771B5E08EE"), m_pPipeLineIn, "Line", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("B2E0BCEE-5048-45EF-93CF-5B6895C5D033"), m_pPipeLineOut, PIPENAME_OUT1, 0, ""}});
    setVariantID(Poco::UUID("005AA500-0554-4FD0-80BF-71F0480010FA"));
}



LineForwardDiff::~LineForwardDiff()
{
	delete m_pPipeLineOut;
}



void LineForwardDiff::setParameter()
{
	TransformFilter::setParameter();
}

bool LineForwardDiff::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)

{
	m_pPipeLineIn  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
}



void LineForwardDiff::paint() {
	if(m_oVerbosity < eLow || inputIsInvalid(m_oLineOut) || m_oSpTrafo.isNull()){
		return;
	} // if

	const Trafo		&rTrafo			( *m_oSpTrafo );
	OverlayCanvas	&rCanvas		( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerContour	( rCanvas.getLayerContour());

	const auto	oGradient		( m_oLineOut.front().getData() );
    bool connectedPointList = m_oVerbosity >= eHigh;
    rLayerContour.add<OverlayPointList>(rTrafo(Point(0,0)), oGradient, Color::Orange(), connectedPointList);

} // paint



void LineForwardDiff::proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeLineIn != nullptr); // to be asserted by graph editor
	// Read-out laserline
	const GeoVecDoublearray& rLineIn = m_pPipeLineIn->read(m_oCounter);
	m_oSpTrafo	= rLineIn.context().trafo();
	// And extract the byte-array
	const VecDoublearray& rArrayIn = rLineIn.ref();

	// input validity check

	if ( inputIsInvalid(rLineIn) ) {
		const GeoVecDoublearray oGeoVecDoublearrayOut(rLineIn.context(), m_oLineOut, rLineIn.analysisResult(), interface::NotPresent); // bad rank
		preSignalAction();  m_pPipeLineOut->signal( oGeoVecDoublearrayOut ); // invoke linked filter(s)

		return; // RETURN
	}

	// Now do the actual image processing
	forwardDiff( rArrayIn, m_oLineOut );

	// Create a new byte array, put the global context into the resulting profile and copy the rank over
	const auto oAnalysisResult	= rLineIn.analysisResult() == AnalysisOK ? AnalysisOK : rLineIn.analysisResult(); // replace 2nd AnalysisOK by your result type
	const GeoVecDoublearray &rGeoProfile = GeoVecDoublearray( rLineIn.context(), m_oLineOut, oAnalysisResult, rLineIn.rank() );
	preSignalAction();  m_pPipeLineOut->signal( rGeoProfile );
}



void LineForwardDiff::forwardDiff( const geo2d::VecDoublearray &p_rLineIn, geo2d::VecDoublearray &p_rLineOut )
{
	const unsigned int	oNbLines	= p_rLineIn.size();
	p_rLineOut.resize(oNbLines); // if the size of the output signal is not equal to the input line size, resize
	for (unsigned int lineN = 0; lineN < oNbLines; ++lineN) { // loop over N lines
		// get the direct references to the stl vectors
		auto& rLineOut_Data = p_rLineOut[lineN].getData();
		auto& rLineOut_Rank = p_rLineOut[lineN].getRank();
		const auto& rLineIn_Data = p_rLineIn[lineN].getData();
		const auto& rLineIn_Rank = p_rLineIn[lineN].getRank();

		// if the size of the output signal is not equal to the input line size, resize
		if (rLineOut_Data.size() != rLineIn_Data.size())
		{
			rLineOut_Data.resize( rLineIn_Data.size());
			rLineOut_Rank.resize( rLineIn_Data.size());
		}

		// forward difference
		for( unsigned int x = 1; x < rLineIn_Data.size() - 1; x++ )
		{
			rLineOut_Data[x] = (rLineIn_Data[x-1] - rLineIn_Data[x+1]);

			// calculate the minimum of the rank of x-1, x and x+1
			if (rLineIn_Rank[x-1] < rLineIn_Rank[x+1])
				rLineOut_Rank[x] = rLineIn_Rank[x-1];
			else
				rLineOut_Rank[x] = rLineIn_Rank[x+1];
			if (rLineIn_Rank[x] < rLineOut_Rank[x])
				rLineOut_Rank[x] = rLineIn_Rank[x];
		}

		// boundaries
		if( rLineIn_Data.size() > 1 )
		{
			rLineOut_Data[0] = rLineOut_Data[1];
			rLineOut_Rank[0] = rLineOut_Rank[1];
			rLineOut_Data[ rLineIn_Data.size()-1 ] = rLineOut_Data[ rLineIn_Data.size()-2 ];
			rLineOut_Rank[ rLineIn_Rank.size()-1 ] = rLineOut_Rank[ rLineIn_Rank.size()-2 ];
		}
	} // for
}



} // namespace precitec
} // namespace filter
