/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		  Andreas Beschorner (BA)
 *  @date		    2012
 *  @brief	    Computes K-Curvation (see: Mustererkennung Proceedings 1991, 13. DAGM-Symposium, p. 168).
 */

#include <cmath> // for atan

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include "module/moduleLogger.h"
#include <filter/armStates.h>

#include "filter/algoArray.h"	///< algorithmic interface for class TArray

// local includes
#include "kCurvation.h"
#include "2D/avgAndRegression.h"

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string KCurvation::m_oFilterName = std::string("KCurvation");
const std::string KCurvation::PIPENAME_KCURVATION  = std::string("Line");
const std::string KCurvation::PIPENAME_Trend  = std::string("KCurveTrend");
const std::string KCurvation::m_oParameterComputeAnglesRotationInvariantName = std::string("ComputeAngle");

KCurvation::KCurvation() : TransformFilter( KCurvation::m_oFilterName, Poco::UUID{"12DD31E7-4B3A-4AB6-80DD-4692DD1F8BAF"} ),
	m_pPipeInLine(nullptr), m_oPipeOutKLine( this, KCurvation::PIPENAME_KCURVATION), m_oPipeOutTrend( this, KCurvation::PIPENAME_Trend ),
	m_oK(3), m_oTrendWindowLength(9), m_oUseRegression(false), m_oComputeAnglesRotationInvariant(false), m_oKInverse(1.0),
	m_oLeftX(nullptr), m_oRightX(nullptr), m_oLeftY(nullptr), m_oRightY(nullptr)
{
	m_oKLine.resize(1);
	m_oTrend.resize(1);
	parameters_.add("ShiftK", Parameter::TYPE_UInt32, m_oK);
	parameters_.add("TrendWinLength", Parameter::TYPE_UInt32, m_oTrendWindowLength);
	parameters_.add("UseRegression", Parameter::TYPE_bool, m_oUseRegression);
	parameters_.add(m_oParameterComputeAnglesRotationInvariantName, Parameter::TYPE_bool, m_oComputeAnglesRotationInvariant);

    setInPipeConnectors({{Poco::UUID("93B79C32-8416-4215-B129-A28B36605482"), m_pPipeInLine, "Line", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("9B970951-75B3-47E1-A3D4-88A22B73F23D"), &m_oPipeOutKLine, PIPENAME_KCURVATION, 0, ""},
    {Poco::UUID("16141887-E7EC-432A-A30A-994185431D8C"), &m_oPipeOutTrend, PIPENAME_Trend, 0, ""}});
    setVariantID(Poco::UUID("C1ECB071-2FFF-42E9-AA85-9F2A0DAB9E95"));
}

KCurvation::~KCurvation()
{
	if (m_oLeftX != nullptr)
	{
		delete [] m_oLeftX;
	}

	if (m_oRightX != nullptr)
	{
		delete [] m_oRightX;
	}

	if (m_oLeftY != nullptr)
	{
		delete [] m_oLeftY;
	}

	if (m_oRightY != nullptr)
	{
		delete [] m_oRightY;
	}
}

KCurvation::KCurvationData KCurvation::computeKCurvation( const Doublearray &p_rLine,
	const unsigned int p_oIdx, const unsigned int p_oMarginShift)
{
	KCurvation::KCurvationData oResult;
	unsigned int oK = m_oK;
	double oKInverse = m_oKInverse;
	if (p_oMarginShift > 0)
	{
		oK = p_oMarginShift;
		oKInverse = 1.0/ static_cast<double>(oK);
	}

	double oSlopeLeft = 0.0; double oSlopeRight = 0.0;
	oResult.m_oRankLeft = 255;
	oResult.m_oRankRight = 255;

	auto & rLineData = p_rLine.getData();
	auto & rLineRank = p_rLine.getRank();
	if (!m_oUseRegression)
	{
		// no regression, compute line segments by simple vector algebra
		int oIdx = std::max(0, (int)(p_oIdx-oK));
		if ( (rLineRank[p_oIdx] > eRankMin) && (rLineRank[oIdx] > eRankMin) )
		{
			oSlopeLeft = ( rLineData[p_oIdx] - rLineData[oIdx] ) * oKInverse;
			oResult.m_oRankLeft = std::min( rLineRank[p_oIdx], rLineRank[oIdx]);
		} else
		{
			oResult.m_oRankLeft = eRankMin;
			oResult.m_oKCurvation = 0.0;
			return oResult;
		}
		oIdx = std::min((int)(p_oIdx+oK), (int)(rLineRank.size()-1));
		if ( (rLineRank[p_oIdx] > eRankMin) && (rLineRank[oIdx] > eRankMin) )
		{
			oSlopeRight = ( rLineData[oIdx] - rLineData[p_oIdx] ) * oKInverse;
			oResult.m_oRankRight = std::min( rLineRank[p_oIdx], rLineRank[oIdx]);
		} else
		{
			oResult.m_oRankRight = eRankMin;
			oResult.m_oKCurvation = 0.0;
			return oResult;
		}
	} else
	{
		// regression; copy line segments using lms linear regression
		// only "good" points will be included
		// this can be optimized...
		int oSizeLeft=0; int oSizeRight=0;
		for (unsigned int i=0; i < oK; ++i)
		{
			int oIdx = std::max(0, (int)(p_oIdx-oK+i));
			if ( rLineRank[oIdx] > eRankMin )
			{
				m_oLeftX[oSizeLeft] = oSizeLeft;
				m_oLeftY[oSizeLeft] = rLineData[oIdx]; // intercept does not matter, thus x[i]=i
				oResult.m_oRankLeft = std::min(oResult.m_oRankLeft, rLineRank[oIdx]);
				++oSizeLeft;
			}
			oIdx = std::min((int)(p_oIdx+i), (int)(rLineRank.size()-1));
			if ( rLineRank[oIdx] > eRankMin )
			{
				m_oRightX[oSizeRight] = oSizeRight;
				m_oRightY[oSizeRight] = rLineData[oIdx];
				oResult.m_oRankRight = std::min(oResult.m_oRankRight, rLineRank[oIdx]);
				++oSizeRight;
			}
		} // end for loop to create point arrays for linear regressions
		double oIntercept, oRegCoeff;

		if ( oSizeLeft > 1 )
		{
			math::linearRegression2D(oSlopeLeft, oIntercept, oRegCoeff, oSizeLeft, m_oLeftX, m_oLeftY);
		} else
		{
			oResult.m_oRankLeft = eRankMin;
			oResult.m_oKCurvation = 0.0;
			return oResult;
		}
		if ( oSizeRight > 1 )
		{
			math::linearRegression2D(oSlopeRight, oIntercept, oRegCoeff, oSizeRight, m_oRightX, m_oRightY);
		} else
		{
			oResult.m_oRankRight = eRankMin;
			oResult.m_oKCurvation = 0.0;
			return oResult;
		}
	} // if (linear regression or two point vector arithmetics)

	oResult.m_oSlopeSum = oSlopeRight - oSlopeLeft;

	if (m_oComputeAnglesRotationInvariant)
	{
		// Darstellung der Kruemmung als Differenz der Winkel zwischen den beiden Schenkeln. Diese Darstellung sorgt dafuer,
		// dass die Ergebniswerte rotationsinvariant gegenueber einer Rotation der Eingangsdaten sind. Das ist nicht
		// gegeben bei der alten Betrachtungsweise, mit der Konsequenz, dass die alte Betrachtungsweise verschiedene
		// Resultate erzeugt, wenn sich die Eingangsdaten nur durch eine Rotation unterscheiden.
		double oAngleRight = atan(oSlopeRight);
		double oAngleLeft = atan(oSlopeLeft);
		double oAngleDiff = oAngleRight - oAngleLeft;
		oResult.m_oKCurvation = oAngleDiff;
		return oResult;
	}
	else{
		oResult.m_oKCurvation = oResult.m_oSlopeSum;
		return oResult;
	}
}

void KCurvation::setParameter()
{
	TransformFilter::setParameter();
	m_oK = static_cast<unsigned int>( parameters_.getParameter("ShiftK").convert<unsigned int>() );
	if (m_oK < 1)
	{
		m_oK = 1;
	} else if (m_oK > 240)
	{
		m_oK = 240;
	}
	m_oTrendWindowLength = static_cast<unsigned int>( parameters_.getParameter("TrendWinLength").convert<unsigned int>() );
	if (m_oTrendWindowLength < 2)
	{
		m_oTrendWindowLength = 2;
	} else if (m_oTrendWindowLength > 200 )
	{
		m_oTrendWindowLength = 200;
	}
	m_oUseRegression = static_cast<bool>( parameters_.getParameter("UseRegression").convert<bool>() );
	poco_assert_dbg((m_oK >= 1) && (m_oK < 240));

	if (m_oUseRegression)
	{
		if (m_oLeftX != nullptr)
			delete [] m_oLeftX;
		m_oLeftX = new double[m_oK+1];

		if (m_oLeftY != nullptr)
			delete [] m_oLeftY;
		m_oLeftY = new double[m_oK+1];

		if (m_oRightX != nullptr)
			delete [] m_oRightX;
		m_oRightX = new double[m_oK+1];

		if (m_oRightY != nullptr)
			delete [] m_oRightY;
		m_oRightY = new double[m_oK+1];
	}

	m_oComputeAnglesRotationInvariant = static_cast<bool>(parameters_.getParameter(m_oParameterComputeAnglesRotationInvariantName).convert<bool>());
}

void KCurvation::paint() {
	if(m_oVerbosity < eLow || m_oSpTrafo.isNull() || (m_oKLine.size() == 0) || (m_oTrend.size() == 0) )
	{
		return;
	}
	const auto & rLastKLine = m_oKLine.back();
	const auto & rLastTrend = m_oTrend.back();
	if (rLastKLine.size() <= 1 || rLastTrend.size() <= 1)
	{
		return;
	}

	const auto & rKLineData = rLastKLine.getData();
	const auto & rKLineRank = rLastKLine.getRank();
	const auto & rTrendData = rLastTrend.getData();
	const auto & rTrendRank = rLastTrend.getRank();

	const Trafo		&rTrafo				( *m_oSpTrafo );
	OverlayCanvas	&rCanvas			( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerPosition		( rCanvas.getLayerPosition());
	OverlayLayer	&rLayerContour		( rCanvas.getLayerContour());

	unsigned int i=0; const double oScale = 12.0;

	geo2d::Point oStart(0, roundToT<int>(120+oScale*(rKLineData[i])));
	rLayerPosition.add<OverlayCross>(rTrafo(oStart), Color::White());
	for (i=0; i < rKLineData.size(); ++i)
	{
		if (rKLineRank[i] > 0 )
		{
			geo2d::Point q( i, roundToT<int>(120+oScale*(rKLineData[i])) );
			if (rKLineData[i] > 0 )
			{
				rLayerContour.add<OverlayPoint>(rTrafo(q), Color::Green());
			} else if (rKLineData[i] < 0 )
			{
				rLayerContour.add<OverlayPoint>(rTrafo(q), Color::Red());
			} else
			{
				rLayerContour.add<OverlayPoint>(rTrafo(q), Color::Orange());
			}
		}
	}
	for (i=0; i < rTrendData.size(); ++i)
	{
		if (rTrendRank[i] > 0 )
		{
			geo2d::Point q( i, roundToT<int>(30+oScale*(rTrendData[i])) );
			if (rTrendData[i] > 0 )
			{
				rLayerContour.add<OverlayPoint>(rTrafo(q), Color::Green());
			} else if (rTrendData[i] < 0 )
			{
				rLayerContour.add<OverlayPoint>(rTrafo(q), Color::Red());
			} else
			{
				rLayerContour.add<OverlayPoint>(rTrafo(q), Color::Orange());
			}
		}
	}
} // paint

bool KCurvation::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeInLine = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray >* >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe

bool KCurvation::isValidLine(const Doublearray & p_rLine, int &p_rStartPos, int &p_rEndPos) const
{
	const auto& line = p_rLine.getData();
	const auto& lineRank = p_rLine.getRank();
	const unsigned int oLength = line.size();

	// bad ranks or line too short?
	if ( inputIsInvalid( p_rLine ) || ( (m_oK << 2) >= oLength ) )
	{
		return false;
	}

	// skip bad rank margins
	while ( (p_rStartPos < p_rEndPos) && (lineRank[p_rStartPos] == eRankMin ) )
	{
		++p_rStartPos;
	}
	while ( ( p_rEndPos > p_rStartPos) && (lineRank[p_rEndPos] == eRankMin ) )
	{
		--p_rEndPos;
	}

	// line needs to be a least as long as max of kcurvation window size. Trend size independent, see function computeTrend
	if ( (int)(m_oK) >= (p_rEndPos - p_rStartPos) )
	{
		return false;
	}

	return true;
}

// assymetric rescaling: scale negative and positive parts independently to keep the zero neighbourhood around zero... Otherwise one might get a bias
void KCurvation::rescale(std::vector<double> &p_rTrend, const double p_oMin, const double p_oMax, const double p_oMinG, const double p_oMaxG)  const
{
	// rescale trend to interval [p_oMin, p_oMax]
	// given minG, maxG and x in [minG, maxG], we compute f(x)=p_oMin + (p_oMax - p_oMin)*(x-minG)/(maxG-minG). Introduces a bias if p_oMin != -p_oMax!
	/*
	double oFactor = (p_oMax - p_oMin) / (m_oMaxG - m_oMinG);
	for (double &oVal : p_rGrad) // scale to new interval
	{
	std::cout << "  " << oVal;
	oVal = p_oMin + (oVal - m_oMinG)*oFactor;
	std::cout << " -> " << oVal << "\n";
	}
	*/


	if ((std::abs(p_oMinG) < math::eps) || (std::abs(p_oMaxG) < math::eps) )
	{
		return;
	}

	double oRelMin = std::abs(p_oMinG)/(std::abs(p_oMinG)+std::abs(p_oMaxG));
	double oRelMax = std::abs(p_oMaxG)/(std::abs(p_oMinG)+std::abs(p_oMaxG));

	for (double &rVal : p_rTrend) // scale to new interval, bias free
	{
		if (rVal <= 0)
		{
			rVal = (rVal*p_oMin)*oRelMin/p_oMinG;
		}
		else
		{
			rVal = (rVal*p_oMax)*oRelMax/p_oMaxG;
		}
	}
}

KCurvation::resultOfKCurvation KCurvation::computeTrend ( geo2d::Doublearray & rOutTrend, const geo2d::Doublearray &p_rKLine, const int p_oStartPos, const int p_oEndPos, const int p_oLength ) const
{

	auto & rOutTrendData = rOutTrend.getData();
	auto & rOutTrendRank = rOutTrend.getRank();

	if ( (int) m_oTrendWindowLength >= (p_oEndPos - p_oStartPos) )
	{
		return eKCurvNoTrend;
	}

	assert((int)rOutTrend.size() == p_oLength);
	std::vector<double> oWindowData(m_oTrendWindowLength);
	std::vector<double> oWindowRank(m_oTrendWindowLength);

	int i=0;
	double oAvg=0.0, oAvgSum=0.0; // result values of average computation
	double oOverallSum=0.0;       // sum over values over complete line for mean substraction afterwards

	// init array and set first element
	for (unsigned int j=p_oStartPos; (j < (p_oStartPos+m_oTrendWindowLength)) && (j < (p_oEndPos - m_oTrendWindowLength)); ++j)
	{
		oWindowData[j-p_oStartPos] = (p_rKLine.getData())[j];
	}
	math::arithmeticAvgVec( oAvg, oWindowData, oAvgSum );
	rOutTrendData[p_oStartPos] = oAvg;
	rOutTrendRank[p_oStartPos] = (p_rKLine.getRank())[p_oStartPos];
	oOverallSum = oAvg;

	// main area (no left margin here as filter is not symmetric but left oriented)
	for (i=p_oStartPos+1; i < (p_oEndPos - (int) m_oTrendWindowLength); ++i)
	{
		const unsigned int oChangeIdx = ( (i-p_oStartPos-1) % m_oTrendWindowLength ); // circular index for quick element replacement instead of new segment computation
		oWindowData[oChangeIdx] = (p_rKLine.getData())[i+m_oTrendWindowLength-1];
		oWindowRank[oChangeIdx] = (p_rKLine.getRank())[i+m_oTrendWindowLength-1];
		math::arithmeticAvgVec( oAvg, oWindowData, oAvgSum );
		rOutTrendData[i] = oAvg;
		rOutTrendRank[i] = (p_rKLine.getRank())[i];
		oOverallSum += oAvg;             // and add result to total average
	}

	while ( i < p_oEndPos )
	{
		math::arithmeticAvgVec( oAvg, oWindowData, oAvgSum);
		rOutTrendData[i] = oAvg;
		rOutTrendRank[i] = (p_rKLine.getRank())[i];
		++i;
		oWindowData.erase(oWindowData.begin(), oWindowData.begin()+1);
		oOverallSum += oAvg;             // and add result to total average
	}

	oOverallSum /= (p_oEndPos - p_oStartPos);
	double oMinG = rOutTrendData[0];
	double oMaxG = rOutTrendData[0];
	for (size_t j = 0; j < rOutTrendData.size(); ++j)
	{
		auto &rVal(rOutTrendData[j]);
		rVal -= oOverallSum;
		if (rVal < oMinG)
		{
			oMinG = rVal;
		}
		if (rVal >= oMaxG)
		{
			oMaxG = rVal;
		}
	}
	//  not usefull for thresholhd chechks. Can be a parameter sometime!!
	//auto &oTrendData(m_oTrend[0].getData());
	//rescale(oTrendData, -1, 1, rMinG, rMaxG); // normalize Trend to [-1, 1] asymmetrically around 0;

	return eKCurvAllOK;
}

void KCurvation::proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeInLine != nullptr); // to be asserted by graph editor

	// Get Line and trafo
	const GeoVecDoublearray &rLine( m_pPipeInLine->read(m_oCounter) );
	m_oSpTrafo	= rLine.context().trafo();

	const unsigned int oNbLines = rLine.ref().size();
	m_oKLine.resize(oNbLines);
	m_oTrend.resize(oNbLines);

	m_oSpTrafo	= rLine.context().trafo();
	m_oKInverse = 1.0/m_oK; // multiplication instead of division...

	double oGeoRankKLine = eRankMin;
	double oGeoRankTrend = eRankMin;

	for (unsigned int lineN = 0; lineN < oNbLines; ++lineN)
	{ // loop over N lines
		const auto & rInputLine = rLine.ref()[lineN];
		const auto oLine = rInputLine.getData();
		const auto oLineRank = rInputLine.getRank();
		const unsigned int oLength = oLine.size();

		auto & rCurrentKLine = m_oKLine[lineN];
		auto & rCurrentTrend = m_oTrend[lineN];
		auto & rKLineData = rCurrentKLine.getData();
		auto & rKLineRank = rCurrentKLine.getRank();
		auto & rTrendData = rCurrentTrend.getData();
		auto & rTrendRank = rCurrentTrend.getRank();

		rKLineData.assign(oLength, 0.0);
		rKLineRank.assign(oLength, 0);
		rTrendData.assign(oLength, 0.0);
		rTrendRank.assign(oLength, 0);

		// ----------------- compute kCurvation --------------------

		double oOverallSum = 0.0; double oCurSlope = 0.0;
		int oNumSlopes = 0;
		resultOfKCurvation oKCurvRes = eKCurvNothing;

		// allow left and right connected segments of bad rank and along with that compute "real" start and end of line
		int oStartPos = 0; int oEndPos = oLine.size() - 1;
		if ( isValidLine(rInputLine, oStartPos, oEndPos) )
		{
			// left margin
			oKCurvRes = eKCurvNoTrend;

			for (unsigned int i=oStartPos+1; i <= oStartPos+m_oK; ++i)
			{
				const unsigned int oOffset = i - oStartPos-1;
				if ( (rInputLine.getRank())[i] > eRankMin)
				{
					auto oKCurvationData = computeKCurvation( rInputLine, i, oOffset+1);
					rKLineData[i] = oKCurvationData.m_oKCurvation;
					oOverallSum += oCurSlope; ++oNumSlopes;
					rKLineRank[i] = std::min(oKCurvationData.m_oRankLeft, oKCurvationData.m_oRankRight); // 0 for oKCurveRank=0
				} else
				{
					rKLineData[i] = 0.0;
					rKLineRank[i] = eRankMin;
				}
			} // for-loop for left margin

			// right margin
			for (unsigned int i=oEndPos-1; i >= oEndPos-m_oK; --i)
			{
				const unsigned int oOffset = oEndPos - i;
				if ( (rInputLine.getRank())[i] > eRankMin)
				{
					auto oKCurvationData = computeKCurvation( rInputLine, i, oOffset);
					rKLineData[i] = oKCurvationData.m_oKCurvation;
					oOverallSum += oCurSlope; ++oNumSlopes;
					rKLineRank[i] = std::min(oKCurvationData.m_oRankLeft, oKCurvationData.m_oRankRight); //(m_oRankLeft + m_oRankRight)/2;
				} else
				{
					rKLineData[i] = 0.0;
					rKLineRank[i]= eRankMin;
				}
			}	 // for loop right margin
			rKLineData[oStartPos] = rKLineData[oStartPos+1];
			rKLineRank[oStartPos] = rKLineRank[oStartPos+1];
			rKLineData[oEndPos] = rKLineData[oEndPos-1];
			rKLineRank[oEndPos] = rKLineRank[oEndPos-1];

			// Main area
			/*
			for (int i = oStartPos+m_oK; (oStartPos+i) <= (oEndPos - m_oK - 1); ++i)
			{
				//const unsigned int oOffset = oStartPos + i;
				const unsigned int oOffset = m_oK + i;
				if ( (rInputLine.getRank())[oOffset] > eRankMin)
				{
					rCurrentKLineData[oOffset] = computeKCurvation( rInputLine, oCurSlope, oOffset);
					oOverallSum += oCurSlope; ++oNumSlopes;
					rCurrentKLineRank[oOffset] = std::min(m_oRankLeft, m_oRankRight); // 0 for bad rank!
				} else
				{
					rCurrentKLineData[oOffset] = 0.0;
					rCurrentKLineRank[oOffset] = eRankMin;
				}
			}
			//oOverallSum /= (oEndPos - oStartPos);
			*/

			for (int i = oStartPos+m_oK; i <= (oEndPos - (int) m_oK); ++i)
			{
				if ( (rInputLine.getRank())[i] > eRankMin)
				{
					auto oKCurvationData =  computeKCurvation( rInputLine, i, 0);
					rKLineData[i] = oKCurvationData.m_oKCurvation;
					oOverallSum += oCurSlope; ++oNumSlopes;
					rKLineRank[i] = std::min(oKCurvationData.m_oRankLeft, oKCurvationData.m_oRankRight); // 0 for bad rank!
				} else
				{
					rKLineData[i] = 0.0;
					rKLineRank[i] = eRankMin;
				}
			}


			if (oNumSlopes > 0)
			{
				oOverallSum /= oNumSlopes;
				for (int i=oStartPos; i <= oEndPos; ++i)
				{
					rTrendData[i] -= oOverallSum;
				}
			}
		} // if isValidLine


		// ---------- smoothing, averaging: compute kCurvation Trend -----------

		if (oKCurvRes != eKCurvNothing)
		{
			oKCurvRes = computeTrend(rCurrentTrend, rCurrentKLine, oStartPos, oEndPos, oLength);
		}

		//set output geo rank to 0 only if all the output elements are null
		switch (oKCurvRes)
		{
			case eKCurvNoTrend: //kline computed, but compute trend could not computed (window bigger than segment)
				{
					oGeoRankKLine = interface::Limit;
					//oGeoRankTrend = interface::NotPresent;
					break;
				}
			case eKCurvNothing: //input line was not valid
				{
					//oGeoRankKLine = interface::NotPresent;
					//oGeoRankTrend = interface::NotPresent;
					break;
				}
			case eKCurvAllOK:
			default:
				{
					oGeoRankKLine = interface::Limit;
					oGeoRankTrend = interface::Limit;
					break;
				}
		} // switch

	} //end for oNbLines

    const GeoVecDoublearray kLineOut(rLine.context(), m_oKLine, AnalysisOK, oGeoRankKLine);
    const GeoVecDoublearray kTrend(rLine.context(), m_oTrend, AnalysisOK, oGeoRankTrend);
    preSignalAction();
    m_oPipeOutKLine.signal( kLineOut );
    m_oPipeOutTrend.signal( kTrend );
}

/*virtual*/ void
KCurvation::arm(const fliplib::ArmStateBase& state) {
	if (state.getStateID() == eSeamStart)
	{
		m_oSpTrafo = nullptr;
	}
} // arm

} // namespace precitec
} // namespace filter
