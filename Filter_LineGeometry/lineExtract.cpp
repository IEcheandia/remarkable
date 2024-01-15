/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Stefan Birmanns (SB), HS
 *  @date		2011
 *  @brief		Extracts a portion from a laserline object.
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
#include "lineExtract.h"

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string LineExtract::m_oFilterName 		= std::string("LineExtract");
const std::string LineExtract::PIPENAME_OUT1	= std::string("Line");



LineExtract::LineExtract() :
	TransformFilter( LineExtract::m_oFilterName, Poco::UUID{"41D6004A-CF5C-4527-AF06-5928EA0CC993"} ),
	m_pPipeLineIn(NULL),
	m_pPipeLineOut( NULL ),
	m_oStart( 0 ),
	m_oEnd( 127 ),
	m_oLineOut( 1, Doublearray( 1, 0 ) )
{
	m_pPipeLineOut = new SynchronePipe< GeoVecDoublearray >( this, LineExtract::PIPENAME_OUT1 );

	// Set default values of the parameters of the filter
	parameters_.add("Start",   Parameter::TYPE_int, m_oStart);
	parameters_.add("End",     Parameter::TYPE_int, m_oEnd);

    setInPipeConnectors({{Poco::UUID("01AC64A0-2A36-441B-A78B-38EFAA4CBBCA"), m_pPipeLineIn, "Line", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("1B89B6D6-2587-49F4-AD6A-F0431FF5AFA9"), m_pPipeLineOut, "Line", 0, ""}});
    setVariantID(Poco::UUID("E9F176E6-0E1C-4483-B281-844EA1C7C6CE"));
} // LineExtract



LineExtract::~LineExtract()
{
	delete m_pPipeLineOut;

} // ~LineExtract



void LineExtract::setParameter()
{
	TransformFilter::setParameter();
	m_oStart = parameters_.getParameter("Start").convert<int>();
	m_oEnd   = parameters_.getParameter("End").convert<int>();

	if (m_oStart < 0)
		m_oStart = 0;
	if (m_oEnd < 0)
		m_oEnd = m_oStart;

	if (m_oStart > m_oEnd)
		std::swap( m_oStart, m_oEnd );

} // setParameter



bool LineExtract::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeLineIn  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void LineExtract::paint() {
	if (m_oVerbosity < eLow || !m_oPaint || m_oSpTrafo.isNull()){
		return;
	} // if

	const Trafo		&rTrafo			( *m_oSpTrafo );
	OverlayCanvas	&rCanvas		( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerContour	( rCanvas.getLayerContour());

	const auto&	oLineY( m_oLineOut.front().getData() );
	const auto& rRank (m_oLineOut.front().getRank());
	for (unsigned int i = 0; i != oLineY.size(); ++i) {
		if (rRank[i] > eRankMin)
		{
			rLayerContour.add( new OverlayPoint(rTrafo(Point(i, int( oLineY[i] ))), Color::Orange()) );
		}
	} // for
} // paint



void LineExtract::proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeLineIn != nullptr); // to be asserted by graph editor
	m_oPaint = false; // needs to be at the very top to make sure paint will not be calles when errors occur!

	// Read-out laserline
	const GeoVecDoublearray& rLineIn = m_pPipeLineIn->read(m_oCounter);
	// And extract byte-array
	const VecDoublearray& rArrayIn = rLineIn.ref();
	// input validity check

	if ( inputIsInvalid(rLineIn) ) {
		const GeoVecDoublearray oGeoVecDoublearrayOut(rLineIn.context(), m_oLineOut, rLineIn.analysisResult(), interface::NotPresent); // bad rank
		preSignalAction();  m_pPipeLineOut->signal( oGeoVecDoublearrayOut ); // invoke linked filter(s)

		return; // RETURN
	}

	// Now do the actual image processing
	extract( rArrayIn, m_oStart, m_oEnd, m_oLineOut );
	m_oPaint = true;

	// now adjust the context to reflect the new offset
	geo2d::Point oOffset( m_oStart, 0 );

	// Trafo ...
	m_oSpTrafo	= LinearTrafo(oOffset)(rLineIn.context().trafo());
	// ... and generate a new context with the new trafo and the old meta information
	SmpImageContext pNewContext = new ImageContext(rLineIn.context(), m_oSpTrafo);
	// Now create a new byte array, put the global context into the resulting profile and copy the rank over
	const auto oAnalysisResult	= rLineIn.analysisResult() == AnalysisOK ? AnalysisOK : rLineIn.analysisResult(); // replace 2nd AnalysisOK by your result type
	const GeoVecDoublearray& rGeoProfile = GeoVecDoublearray(*pNewContext, m_oLineOut, oAnalysisResult, rLineIn.rank());
	preSignalAction();  m_pPipeLineOut->signal( rGeoProfile );

} // proceed



void LineExtract::extract( const VecDoublearray &p_rLineIn, const int p_oStart, const int p_oEnd, VecDoublearray &p_rLineOut )
{
	const unsigned int	oNbLines	= p_rLineIn.size();
	p_rLineOut.resize(oNbLines); // if the size of the output signal is not equal to the input line size, resize
	for (unsigned int lineN = 0; lineN < oNbLines; ++lineN) { // loop over N lines
		// get the direct references to the stl vectors
		auto& rLineOut_Data = p_rLineOut[lineN].getData();
		auto& rLineOut_Rank = p_rLineOut[lineN].getRank();
		const auto& rLineIn_Data = p_rLineIn[lineN].getData();
		const auto& rLineIn_Rank = p_rLineIn[lineN].getRank();

		// sanity check
		unsigned int oEnd = std::min((int)(rLineIn_Data.size() - 1), p_oEnd);
		if (p_oStart > ((int)rLineIn_Data.size() - 1)) {
			//input line is too short, output should be empty
			rLineOut_Data.clear();
			rLineOut_Rank.clear();
			continue;
		}

		// if the size of the output signal is not equal to the input line size, resize
		if (rLineOut_Data.size() != (unsigned int)( oEnd - p_oStart ))
		{
			rLineOut_Data.resize( oEnd - p_oStart );
			rLineOut_Rank.resize( oEnd - p_oStart );
		}

		// copy
		copy(rLineIn_Data.begin()+p_oStart, rLineIn_Data.begin()+oEnd, rLineOut_Data.begin());
		copy(rLineIn_Rank.begin()+p_oStart, rLineIn_Rank.begin()+oEnd, rLineOut_Rank.begin());
	} // for
} // extract



} // namespace precitec
} // namespace filter
