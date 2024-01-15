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
#include "lineExtractDynamic.h"
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string LineExtractDynamic::m_oFilterName 		= std::string("LineExtractDynamic");
const std::string LineExtractDynamic::PIPENAME_OUT1	= std::string("Line");



LineExtractDynamic::LineExtractDynamic() :
	TransformFilter( LineExtractDynamic::m_oFilterName, Poco::UUID{"13B625E5-0F66-45F7-91C3-7391B9366D72"} ),
	m_pPipeLineIn(nullptr),
	m_pPipeLineOut( nullptr ),
	m_pPipeStart(nullptr),
	m_pPipeEnd(nullptr ),
	m_oLineOut( 1, Doublearray( 1, 0 ) ),
	m_oColor(Color::Orange())
{
	m_pPipeLineOut = new SynchronePipe< GeoVecDoublearray >( this, LineExtractDynamic::PIPENAME_OUT1 );
	//parameters default values
	parameters_.add("rgbRed", Parameter::TYPE_UInt32, static_cast<unsigned int>(m_oColor.red));
	parameters_.add("rgbGreen", Parameter::TYPE_UInt32, static_cast<unsigned int>(m_oColor.green));
	parameters_.add("rgbBlue", Parameter::TYPE_UInt32, static_cast<unsigned int>(m_oColor.blue));
	parameters_.add("rgbAlpha", Parameter::TYPE_UInt32, static_cast<unsigned int>(m_oColor.alpha));

    setInPipeConnectors({{Poco::UUID("6B7B91E2-7D0C-4121-9493-83C48F54F052"), m_pPipeLineIn, "Line", 1, "Line"},
    {Poco::UUID("F1E4D5D1-D8C3-4908-AB89-C3AADB9C7F57"), m_pPipeStart, "Start", 1, "Start"},
    {Poco::UUID("0BB8344D-AA30-4263-B6DC-69A31D6BA37E"), m_pPipeEnd, "End", 1, "End"}});
    setOutPipeConnectors({{Poco::UUID("69F2E33B-9AA7-41A1-9AA8-3EE8DC4A571C"), m_pPipeLineOut, "Line", 0, ""}});
    setVariantID(Poco::UUID("AC93081D-F89B-48C5-8FC9-9310FE2AF0CA"));
} // LineExtractDynamic



LineExtractDynamic::~LineExtractDynamic()
{
	delete m_pPipeLineOut;

} // ~LineExtractDynamic



void LineExtractDynamic::setParameter()
{
		// here BaseFilter sets the common parameters (currently verbosity)
		TransformFilter::setParameter();

		m_oColor.red = parameters_.getParameter("rgbRed").convert<byte>();
		m_oColor.green = parameters_.getParameter("rgbGreen").convert<byte>();
		m_oColor.blue = parameters_.getParameter("rgbBlue").convert<byte>();
		m_oColor.alpha = parameters_.getParameter("rgbAlpha").convert<byte>();
	} // setParameter



bool LineExtractDynamic::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if (p_rPipe.tag() == "Line") {
		m_pPipeLineIn = dynamic_cast<fliplib::SynchronePipe < GeoVecDoublearray > *>(&p_rPipe);
	}
	if (p_rPipe.tag() == "Start") {
		m_pPipeStart = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray> * >(&p_rPipe);
	}
	if (p_rPipe.tag() == "End") {
		m_pPipeEnd = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray> * >(&p_rPipe);
	}

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void LineExtractDynamic::paint() {
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
			rLayerContour.add( new OverlayPoint(rTrafo(Point(i, int( oLineY[i] ))), m_oColor) );
		}
	} // for
} // paint



void LineExtractDynamic::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg)
{
	poco_assert_dbg(m_pPipeLineIn != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeStart != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeEnd != nullptr); // to be asserted by graph editor

	m_oPaint = false; // needs to be at the very top to make sure paint will not be called when errors occur!

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



    const auto & rStartIn = m_pPipeStart->read(m_oCounter);
    const auto & rEndIn = m_pPipeEnd->read(m_oCounter);
    if (rStartIn.context() != rEndIn.context()) // contexts expected to be equal
    {
        std::ostringstream oMsg;
        oMsg << m_oFilterName << ": Different contexts for start and end value: '" << rStartIn.context() << "', '" << rEndIn.context() << "'\n";
        wmLog(eWarning, oMsg.str());
    }
    if (rLineIn.context() != rStartIn.context()) // contexts expected to be equal
    {
        std::ostringstream oMsg;
        oMsg << m_oFilterName << ": Different contexts for line and start value: '" << rLineIn.context() << "', '" << rEndIn.context() << "'\n";
        wmLog(eWarning, oMsg.str());
    }
    if (rLineIn.context() != rEndIn.context()) // contexts expected to be equal
    {
        std::ostringstream oMsg;
        oMsg << m_oFilterName << ": Different contexts for line and end value: '" << rLineIn.context() << "', '" << rEndIn.context() << "'\n";
        wmLog(eWarning, oMsg.str());
    }

	int m_oStart = int( rStartIn.ref().getData()[0] );
	int m_oEnd = int(rEndIn.ref().getData()[0]);

	if (m_oStart > m_oEnd)
		std::swap( m_oStart, m_oEnd );

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



void LineExtractDynamic::extract( const VecDoublearray &p_rLineIn, int p_oStart, int p_oEnd, VecDoublearray &p_rLineOut )
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
		p_oStart = std::max(0, p_oStart );
		p_oEnd = std::min( (int)(rLineIn_Data.size()-1), p_oEnd );
		if (p_oStart >= p_oEnd || p_oStart > ( (int)(rLineIn_Data.size())- 1)) {
			//input line is too short, output should be empty
			rLineOut_Data.clear();
			rLineOut_Rank.clear();
			continue;
		}

		// if the size of the output signal is not equal to the input line size, resize
		if (rLineOut_Data.size() != (unsigned int)( p_oEnd - p_oStart ))
		{
			rLineOut_Data.resize( p_oEnd - p_oStart );
			rLineOut_Rank.resize( p_oEnd - p_oStart );
		}

		// copy
		copy(rLineIn_Data.begin()+ p_oStart, rLineIn_Data.begin()+ p_oEnd, rLineOut_Data.begin());
		copy(rLineIn_Rank.begin()+ p_oStart, rLineIn_Rank.begin()+ p_oEnd, rLineOut_Rank.begin());
	} // for
} // extract



} // namespace precitec
} // namespace filter
