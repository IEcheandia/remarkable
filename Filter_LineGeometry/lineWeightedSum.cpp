/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Stefan Birmanns (SB), HS
 * 	@date		2011
 * 	@brief 		This filter computes the weighted sum of two signals / laser lines.
 */

// clib includes
#include <limits.h>
// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include <filter/algoArray.h>
#include "module/moduleLogger.h"
// local includes
#include "lineWeightedSum.h"

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string LineWeightedSum::m_oFilterName 		= std::string("LineWeightedSum");
const std::string LineWeightedSum::PIPENAME_OUT1	= std::string("Line");



LineWeightedSum::LineWeightedSum() :
	TransformFilter( LineWeightedSum::m_oFilterName, Poco::UUID{"A48971E2-250E-4A31-A97F-7CA11B790DB6"} ),
	m_pPipeLineInA( NULL ),
	m_pPipeLineInB( NULL ),
	m_pPipeLineOut( NULL ),
	m_oWeightA( 1.0f ),
	m_oWeightB( 1.0f ),
	m_oLineOut( 1, Doublearray( 1, 0 ) )
{
	m_pPipeLineOut = new SynchronePipe< GeoVecDoublearray >( this, LineWeightedSum::PIPENAME_OUT1 );

	// Set default values of the parameters of the filter
	parameters_.add("WeightA",    Parameter::TYPE_float, m_oWeightA);
	parameters_.add("WeightB",    Parameter::TYPE_float, m_oWeightB);

    setInPipeConnectors({{Poco::UUID("F37A663B-779C-4CD6-8747-A3AC95AE5810"), m_pPipeLineInA, "Line", 1, "lineA"},
    {Poco::UUID("3EB0CDB3-AFBC-428D-9B72-C1C138554D4C"), m_pPipeLineInB, "Line", 1, "lineB"}});
    setOutPipeConnectors({{Poco::UUID("A81EF4E8-98DF-4C09-969E-778280B2878A"), m_pPipeLineOut, "Line", 0, ""}});
    setVariantID(Poco::UUID("BC6F187F-C1F1-428F-B3ED-2D91592A5571"));
} // LineWeightedSum



LineWeightedSum::~LineWeightedSum()
{
	delete m_pPipeLineOut;

} // ~LineWeightedSum



void LineWeightedSum::setParameter()
{
	TransformFilter::setParameter();
	m_oWeightA    = parameters_.getParameter("WeightA").convert<float>();
	m_oWeightB    = parameters_.getParameter("WeightB").convert<float>();

} // setParameter



bool LineWeightedSum::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{

	if (p_rPipe.tag() == "lineA")
		m_pPipeLineInA  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);
	if (p_rPipe.tag() == "lineB")
		m_pPipeLineInB  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void LineWeightedSum::paint() {
	if(m_oVerbosity < eLow || inputIsInvalid(m_oLineOut) || m_oSpTrafo.isNull()){
		return;
	} // if

	const Trafo		&rTrafo			( *m_oSpTrafo );
	OverlayCanvas	&rCanvas		( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerContour	( rCanvas.getLayerContour());

	const auto & rWeightedSum	( m_oLineOut.front().getData() );
    rLayerContour.add<OverlayPointList>(Point{rTrafo.dx(), rTrafo.dy()}, rWeightedSum, Color::Orange());

} // paint



void LineWeightedSum::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeLineInA != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeLineInB != nullptr); // to be asserted by graph editor

	// Read-out laserline A
	const GeoVecDoublearray& rLineInA = m_pPipeLineInA->read(m_oCounter);
	// Read-out laserline B
	const GeoVecDoublearray& rLineInB = m_pPipeLineInB->read(m_oCounter);
	m_oSpTrafo	= rLineInA.context().trafo();

	// input validity check

	if ( inputIsInvalid( rLineInA ) || inputIsInvalid ( rLineInB ) || rLineInA.ref().size() != rLineInB.ref().size() ) {
		const GeoVecDoublearray &rGeoProfile = GeoVecDoublearray( rLineInA.context(), m_oLineOut, rLineInA.analysisResult(), interface::NotPresent );
		preSignalAction(); m_pPipeLineOut->signal( rGeoProfile );

		return; // RETURN
	}

	// Now do the actual image processing
	weightedSum( m_oWeightA, rLineInA.ref(), m_oWeightB, rLineInB.ref(), m_oLineOut );

	// Create a new byte array, and put the global context into the resulting profile
	const auto oAnalysisResult	= rLineInA.analysisResult() == AnalysisOK ? AnalysisOK : rLineInA.analysisResult(); // replace 2nd AnalysisOK by your result type
	const GeoVecDoublearray &rGeoProfile = GeoVecDoublearray( rLineInA.context(), m_oLineOut, oAnalysisResult, (m_oWeightA * rLineInA.rank()) + (m_oWeightB * rLineInB.rank()) );

	preSignalAction(); m_pPipeLineOut->signal( rGeoProfile );

} // proceedGroup



void LineWeightedSum::weightedSum(const float p_oWeightA, const geo2d::VecDoublearray &p_rLineInA, const float p_oWeightB,
	const geo2d::VecDoublearray &p_rLineInB, geo2d::VecDoublearray &p_rLineOut)
{
	if(p_rLineInA.size() != p_rLineInB.size()) {
		wmLog(eWarning, "Input line sizes inconsisent.\n");
	} // if
	const unsigned int	oNbLines	= p_rLineInA.size();
	p_rLineOut.resize(oNbLines); // if the size of the output signal is not equal to the input line size, resize
	for (unsigned int lineN = 0; lineN < oNbLines; ++lineN) { // loop over N lines
		// get the references to the stl vectors
		auto& rLineOut_Data = p_rLineOut[lineN].getData();
		auto& rLineOut_Rank = p_rLineOut[lineN].getRank();
		const auto& rLineInA_Data = p_rLineInA[lineN].getData();
		const auto& rLineInA_Rank = p_rLineInA[lineN].getRank();
		const auto& rLineInB_Data = p_rLineInB[lineN].getData();
		const auto& rLineInB_Rank = p_rLineInB[lineN].getRank();

		// determine the correct size
		unsigned int oSize = rLineInA_Data.size();
		if (rLineInB_Data.size() < oSize)
			oSize = rLineInB_Data.size();

		// if the size of the profile is not equal to the input size, resize the profile
		if ( rLineOut_Data.size() != oSize )
		{
			rLineOut_Data.resize( oSize );
			rLineOut_Rank.resize( oSize );
		}

		for (unsigned int i=0; i<oSize; i++)
		{
			rLineOut_Data[i] = (int)(p_oWeightA * (float)(rLineInA_Data[i])) + (int)(p_oWeightB * (float)(rLineInB_Data[i]));

			if (rLineInA_Rank[i] < rLineInB_Rank[i])
				rLineOut_Rank[i] = rLineInA_Rank[i];
			else
				rLineOut_Rank[i] = rLineInB_Rank[i];
		}
	} // for
} // weightedSum



} // namespace precitec
} // namespace filter
