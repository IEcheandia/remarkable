/*!
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Andreas Beschorner (AB)
 *  @date		2013
 *  @brief		Low-pass filter for the laser-lines.
 */

// clib includes
#include <limits.h>
// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include <filter/algoArray.h>	///< algorithmic interface for class TArray
#include "module/moduleLogger.h"
// local includes
#include "lineLowPassAndScale.h"
#include "event/results.h"

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string LineLowPassAndScale::m_oFilterName 		= std::string("LineLowPassAndScale");
const std::string LineLowPassAndScale::PIPENAME_OUT1	= std::string("Line");

LineLowPassAndScale::LineLowPassAndScale() :
	TransformFilter( LineLowPassAndScale::m_oFilterName, Poco::UUID{"E16003FE-08EF-4605-8FB7-6490840FA690"} ),
	m_pPipeLineIn(NULL),
	m_pPipeLineOut( NULL ),
	m_oFirWeight(50), m_oScale(8), m_oMulOrDiv(0), m_oInterpolateScaled(true),
	m_oLineOut( 1, Doublearray( 1, 0 ) ), m_oPaint(false)
{
	m_pPipeLineOut = new SynchronePipe< GeoVecDoublearray >( this, LineLowPassAndScale::PIPENAME_OUT1 );

	// Set default values of the parameters of the filter
	parameters_.add("FIRWeight",   Parameter::TYPE_int, m_oFirWeight);
	parameters_.add("Scale", Parameter::TYPE_int, m_oScale);
	parameters_.add("MulOrDiv", Parameter::TYPE_int, m_oMulOrDiv);
	parameters_.add("InterpolateScaled", Parameter::TYPE_bool, m_oInterpolateScaled);

    setInPipeConnectors({{Poco::UUID("948EED45-F111-4090-B01E-FE255B1CFCD1"), m_pPipeLineIn, "Line", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("6AA8FD8E-9CD4-4193-9154-21D402BF2D8C"), m_pPipeLineOut, PIPENAME_OUT1, 0, ""}});
    setVariantID(Poco::UUID("F18A6ED1-B0B0-4312-866D-9E9A728EA27D"));
}

LineLowPassAndScale::~LineLowPassAndScale()
{
	delete m_pPipeLineOut;
}

void LineLowPassAndScale::setParameter()
{
	TransformFilter::setParameter();
	m_oFirWeight   = parameters_.getParameter("FIRWeight").convert<int>();
	m_oScale = parameters_.getParameter("Scale").convert<int>();
	m_oMulOrDiv = parameters_.getParameter("MulOrDiv").convert<int>();
	m_oInterpolateScaled = parameters_.getParameter("InterpolateScaled").convert<bool>();
}

bool LineLowPassAndScale::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeLineIn  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

}

void LineLowPassAndScale::paint() {
	if(m_oVerbosity < eLow || inputIsInvalid(m_oLineOut) || m_oSpTrafo.isNull() ){
		return;
	}

	if (!m_oPaint)
	{
		return;
	}
	const Trafo		&rTrafo			( *m_oSpTrafo );
	OverlayCanvas	&rCanvas		( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerContour	( rCanvas.getLayerContour());

	const auto&	rFilteredLine	( m_oLineOut.front().getData() );
	const auto& rRank (m_oLineOut.front().getRank());
	for (unsigned int i = 0; i != rFilteredLine.size(); ++i) {
		if (rRank[i] > eRankMin)
		{
			rLayerContour.add( new OverlayPoint(rTrafo(Point(i, int( rFilteredLine[i] ))), Color::Orange()) );
		}
	}
}

void LineLowPassAndScale::proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rE)
{
	m_oPaint = false; // needs to be at the very top to make sure paint will not be calles when errors occur!

	poco_assert_dbg(m_pPipeLineIn != nullptr); // to be asserted by graph editor
	// Read-out laserline
	const GeoVecDoublearray& rLineIn = m_pPipeLineIn->read(m_oCounter);
	m_oSpTrafo	= rLineIn.context().trafo();
	// And extract byte-array
	const VecDoublearray& rArrayIn = rLineIn.ref();

	// input validity check
	if ( inputIsInvalid(rLineIn) ) {
		const GeoVecDoublearray oGeoVecDoublearrayOut(rLineIn.context(), m_oLineOut, rLineIn.analysisResult(), interface::NotPresent); // bad rank
		preSignalAction(); m_pPipeLineOut->signal( oGeoVecDoublearrayOut ); // invoke linked filter(s)
		return;
	}

	// Now do the actual image processing
	if (!m_oMulOrDiv )
	{
		lowPass( rArrayIn, m_oScale, m_oFirWeight, m_oLineOut );
	} else
	{
		lowPass( rArrayIn, 1.0/m_oScale, m_oFirWeight, m_oLineOut );
	}

	// Create a new byte array, put the global context into the resulting profile and copy the rank over
	const auto oAnalysisResult	= rLineIn.analysisResult() == AnalysisOK ? AnalysisOK : rLineIn.analysisResult(); // replace 2nd AnalysisOK by your result type
	const GeoVecDoublearray& rGeoProfile = GeoVecDoublearray(rLineIn.context(), m_oLineOut, oAnalysisResult, rLineIn.rank());
	m_oPaint = true;
	preSignalAction(); m_pPipeLineOut->signal( rGeoProfile );
} // proceed

// Implementation of a standard one-tap recursive low pass filter, see documentation details.
void LineLowPassAndScale::lowPass( const geo2d::VecDoublearray &p_rLineIn, double p_oScale, int p_oFirWeight, geo2d::VecDoublearray &p_rLineOut )
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
			rLineOut_Rank.resize( rLineIn_Rank.size());
		}

		int firstValidIndex = -1, lastValidIndex = rLineIn_Data.size()-1; // index of rank

		// init the output line
		std::fill( rLineOut_Data.begin(), rLineOut_Data.end(), 0 );
		// copy the rank over
		std::copy( rLineIn_Rank.begin(), rLineIn_Rank.end(), rLineOut_Rank.begin() );

		// init the first element
		rLineOut_Data[0] = rLineIn_Data[0];

		unsigned int x=0;

		if (rLineIn_Rank[0] == eRankMin)
		{
			rLineOut_Rank[0] = eRankMin;
		}

		double oFirWeight = p_oFirWeight/100.0;

		// forward. This is the filtering part itself as well as the scaling of the output signal.
		for( x = 1; x < rLineIn_Data.size(); x++ )
		{
			if ( rLineIn_Rank[x] != eRankMin && rLineIn_Rank[x-1] != eRankMin )
			{
				if (firstValidIndex >= 0)
				{
					rLineOut_Data[x] = (int)( ( (oFirWeight * rLineOut_Data[x-1]) + ((1-oFirWeight) * rLineIn_Data[x]) ) );
					rLineOut_Data[x-1] = (int)(p_oScale*rLineOut_Data[x-1]);
				} else
				{
					rLineOut_Data[x] = rLineIn_Data[x];
					firstValidIndex = x;
				}
				lastValidIndex = x;
			}
			else
			{
				rLineOut_Rank[0] = eRankMin;
				rLineOut_Data[x] = rLineIn_Data[x];
			}
		}
		if (firstValidIndex > 0)
		{
			rLineOut_Data[firstValidIndex-1] = (int)(p_oScale*rLineOut_Data[firstValidIndex-1]);
		}
		rLineOut_Data[x-1] = (int)(p_oScale*rLineOut_Data[x-1]);

		// backwards. This loop interpolates between the scaled output values!
		if (m_oInterpolateScaled)
		{
			double oScalePlusOne = (m_oScale + 1.0);
			if (m_oMulOrDiv > 0)
			{
				oScalePlusOne = 1+1.0/oScalePlusOne;
			}
			for( int x = lastValidIndex; x >= std::max(firstValidIndex, 1); x-- )
			{
				if ( rLineIn_Rank[x] != eRankMin && rLineIn_Rank[x-1] != eRankMin )
					rLineOut_Data[x-1] = (int)(( (p_oScale * rLineOut_Data[x]) + rLineOut_Data[x-1] ) / oScalePlusOne );
			}
		}
	} // for over lines
} // lowPass


} // namespace precitec
} // namespace filter
