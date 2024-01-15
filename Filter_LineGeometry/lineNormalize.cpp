/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Stefan Birmanns (SB), HS
 *  @date		2011
 *  @brief		Performs a normalization of a signal between a given maximum and minimum.
 */

// clib includes
#include <limits>
// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include <filter/algoArray.h>	///< algorithmic interface for class TArray
#include "module/moduleLogger.h"
// local includes
#include "lineNormalize.h"
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string LineNormalize::m_oFilterName 	= std::string("LineNormalize");
const std::string LineNormalize::PIPENAME_OUT1	= std::string("Line");



LineNormalize::LineNormalize() :
	TransformFilter( LineNormalize::m_oFilterName, Poco::UUID{"0BB700DA-E5FF-467E-92BE-853F4C58A631"} ),
	m_pPipeLineIn(NULL),
	m_pPipeLineOut( NULL ),
	m_oMinimum( 0 ),
	m_oMaximum( 255 ),
	m_oLineOut( 1, Doublearray( 1, 0 ) )
{
	m_pPipeLineOut = new SynchronePipe< interface::GeoVecDoublearray >( this, LineNormalize::PIPENAME_OUT1 );

	// Set default values of the parameters of the filter
	parameters_.add("Minimum", Parameter::TYPE_int, m_oMinimum);
	parameters_.add("Maximum", Parameter::TYPE_int, m_oMaximum);

    setInPipeConnectors({{Poco::UUID("04E419E9-60CB-46C2-A206-1F4401256929"), m_pPipeLineIn, "Line", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("CE91671E-6E93-4AF5-90C6-630265C78B86"), m_pPipeLineOut, "Line", 0, ""}});
    setVariantID(Poco::UUID("9F88001C-1D9E-4E91-8798-AB18D32ECD9E"));
} // LineNormalize



LineNormalize::~LineNormalize()
{
	delete m_pPipeLineOut;

} // ~LineNormalize



void LineNormalize::setParameter()
{
	TransformFilter::setParameter();
	m_oMinimum = parameters_.getParameter("Minimum").convert<int>();
	m_oMaximum = parameters_.getParameter("Maximum").convert<int>();

} // setParameter



bool LineNormalize::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {
	m_pPipeLineIn  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void LineNormalize::paint() {
	if(m_oVerbosity < eLow || inputIsInvalid(m_oLineOut) || m_oSpTrafo.isNull()){
		return;
	} // if

	const Trafo		&rTrafo			( *m_oSpTrafo );
	OverlayCanvas	&rCanvas		( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerContour	( rCanvas.getLayerContour());

	const auto&	rLaserLineY	( m_oLineOut.front().getData() );
    bool connectedPointList = m_oVerbosity >= eHigh;

    rLayerContour.add<OverlayPointList>(rTrafo(Point(0,0)), rLaserLineY, Color::Orange(), connectedPointList) ;

} // paint



void LineNormalize::proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeLineIn != nullptr); // to be asserted by graph editor
	// Read-out laserline
	const GeoVecDoublearray& rLineIn = m_pPipeLineIn->read(m_oCounter);
	m_oSpTrafo	= rLineIn.context().trafo();
	// And extract byte-array
	const VecDoublearray& rArrayIn = rLineIn.ref();

	resetFromInput(rArrayIn, m_oLineOut);

	// input validity check

	if ( inputIsInvalid(rLineIn) ) {
		const GeoVecDoublearray oGeoVecDoublearrayOut(rLineIn.context(), m_oLineOut, rLineIn.analysisResult(), interface::NotPresent); // bad rank
		preSignalAction(); m_pPipeLineOut->signal( oGeoVecDoublearrayOut ); // invoke linked filter(s)

		return; // RETURN
	}

	// Now do the actual image processing
	normalize( rArrayIn, m_oLineOut, m_oMinimum, m_oMaximum );

	// Create a new byte array, put the global context into the resulting profile and copy the rank over
	const auto oAnalysisResult	= rLineIn.analysisResult() == AnalysisOK ? AnalysisOK : rLineIn.analysisResult(); // replace 2nd AnalysisOK by your result type
	const GeoVecDoublearray& rGeoProfile = GeoVecDoublearray(rLineIn.context(), m_oLineOut, oAnalysisResult, rLineIn.rank());
	preSignalAction(); m_pPipeLineOut->signal( rGeoProfile );

} // proceed


void LineNormalize::normalize( const geo2d::VecDoublearray &p_rLineIn, geo2d::VecDoublearray &p_rLineOut, int p_oMin, int p_oMax )
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
		// copy rank
		std::copy( rLineIn_Rank.begin(), rLineIn_Rank.end(), rLineOut_Rank.begin() );

		// determine min/max
		double oMin = std::numeric_limits<double>::max();
		double oMax = std::numeric_limits<double>::lowest();
		for( unsigned int i = 0; i < rLineIn_Data.size(); i++ )
		{
			if (rLineIn_Data[i] > oMax && rLineIn_Rank[i] > 0)
				oMax = rLineIn_Data[i];
			if (rLineIn_Data[i] < oMin && rLineIn_Rank[i] > 0)
				oMin = rLineIn_Data[i];
		}

		// determine old and new width
		double oSigWidth = oMax - oMin;
		double oNewWidth = (double) (p_oMax - p_oMin);

		// prevent division by zero
		if (oSigWidth == 0.0)
			oSigWidth = 1.0;

		// now normalize the signal
		for( unsigned int i=0; i<rLineIn_Data.size(); i++ )
			rLineOut_Data[i] = p_oMin + ((rLineIn_Data[i] - oMin) * oNewWidth) / oSigWidth;
	} // for
} // normalize

} // namespace precitec
} // namespace filter

