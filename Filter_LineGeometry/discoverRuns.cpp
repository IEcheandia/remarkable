// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include "module/moduleLogger.h"
#include "geo/array.h"
#include "geo/geo.h"

#include "filter/algoArray.h"	// algorithmic interface for class TArray

// local includes
#include "discoverRuns.h"
#include "2D/avgAndRegression.h" // for sgn

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string DiscoverRuns::m_oFilterName = std::string("DiscoverRuns");
const std::string DiscoverRuns::PIPENAME_OUTLeftX = std::string("MarkerLeftX");
const std::string DiscoverRuns::PIPENAME_OUTLeftY = std::string("MarkerLeftY");
const std::string DiscoverRuns::PIPENAME_OUTRightX = std::string("MarkerRightX");
const std::string DiscoverRuns::PIPENAME_OUTRightY = std::string("MarkerRightY");
const std::string DiscoverRuns::PIPENAME_OUTOrientation = std::string("Orientation");

const int DiscoverRuns::m_oRankThreshold = (int)(eRankMax*0.95);

DiscoverRuns::DiscoverRuns() :
	TransformFilter( DiscoverRuns::m_oFilterName, Poco::UUID{"C2A881ED-404D-4AD1-B9FC-5800E1943F8F"} ),
	m_pPipeLineInKCurve(nullptr), m_pPipeLineInGradTrend(nullptr),  m_pPipeInSignatureChanges( nullptr ),
	m_oPipePositionOutLeftX( this, DiscoverRuns::PIPENAME_OUTLeftX ), m_oPipePositionOutLeftY( this, DiscoverRuns::PIPENAME_OUTLeftY ),
	m_oPipePositionOutRightX( this, DiscoverRuns::PIPENAME_OUTRightX ), m_oPipePositionOutRightY( this, DiscoverRuns::PIPENAME_OUTRightY ),
	m_oPipeOutOrientation( this, DiscoverRuns::PIPENAME_OUTOrientation ),
	m_oSearchWinSize(60), m_oTurningPointEps(8),
	m_oMinX(-1), m_oMaxX(-1), m_oMinY(-1.0), m_oMaxY(-1.0), m_oOrientation(eOrientationInvalid),
	m_oStartpos(-1), m_oEndpos(-1), m_oPaint(true)
{
	// Set default values of the parameters of the filter
	m_oOutLeftX.getData().resize(1); m_oOutLeftX.getRank().resize(1);
	m_oOutLeftY.getData().resize(1); m_oOutLeftY.getRank().resize(1);
	m_oOutRightX.getData().resize(1); m_oOutRightX.getRank().resize(1);
	m_oOutRightY.getData().resize(1); m_oOutRightY.getRank().resize(1);
	m_oOrientationValue.getData().resize(1); m_oOrientationValue.getRank().resize(1);

	// all parameters in [um]!
	parameters_.add( "SearchWinSize", Parameter::TYPE_UInt32, static_cast<unsigned int>(m_oSearchWinSize) );
	parameters_.add( "TurningPointEps", Parameter::TYPE_UInt32, static_cast<unsigned int>(m_oTurningPointEps) ); // [2..20]

    setInPipeConnectors({{Poco::UUID("062500B2-A83F-4ED1-901C-491F99A3735A"), m_pPipeLineInKCurve, "KCurveTrend", 1, "KCurveTrend"},
    {Poco::UUID("2047D5CA-148E-4B2E-AD01-2A380CEF950A"), m_pPipeLineInGradTrend, "GradientTrend", 1, "GradientTrend"},
    {Poco::UUID("19DF98FE-2832-4AC7-8962-B3EED4FFC0B9"), m_pPipeInSignatureChanges, "SignatureChanges", 1, "SignatureChanges"}});
    setOutPipeConnectors({{Poco::UUID("36444BAA-3A5F-4C4D-AD6D-5A87D27F88DC"), &m_oPipePositionOutLeftX, PIPENAME_OUTLeftX, 0, ""},
    {Poco::UUID("9FB8BE2C-7A0A-4AA0-B9EE-AF5A441AA99F"), &m_oPipePositionOutLeftY, PIPENAME_OUTLeftY, 0, ""},
    {Poco::UUID("A944ECFC-7ECA-432B-8493-20D4DA820165"), &m_oPipePositionOutRightX, PIPENAME_OUTRightX, 0, ""},
    {Poco::UUID("FF80EDAE-F9F6-4BD4-8523-3C3BC885CF12"), &m_oPipePositionOutRightY, PIPENAME_OUTRightY, 0, ""},
    {Poco::UUID("A1A07718-E236-4690-B017-861A0C03BFC3"), &m_oPipeOutOrientation, PIPENAME_OUTOrientation, 0, ""}});
    setVariantID(Poco::UUID("11CC04D9-D5B5-42A6-B581-BF98E701F07D"));
}
const double DiscoverRuns::m_oMarginSize = 0.1;

void DiscoverRuns::setParameter()
{
	TransformFilter::setParameter();
	m_oSearchWinSize = static_cast<unsigned int>( parameters_.getParameter("SearchWinSize").convert<unsigned int>() );
	m_oTurningPointEps = static_cast<unsigned int>( parameters_.getParameter("TurningPointEps").convert<unsigned int>() );
	if (m_oTurningPointEps < 2)
	{
		m_oTurningPointEps = 2;
	} else if (m_oTurningPointEps > 20)
	{
		m_oTurningPointEps = 20;
	}
}

void DiscoverRuns::paint() {
	if( !m_oPaint || m_oVerbosity < eLow  || inputIsInvalid(m_oOutLeftX) || inputIsInvalid(m_oOutLeftY) || inputIsInvalid(m_oOutRightX) || inputIsInvalid(m_oOutRightY) || m_oSpTrafo.isNull())
	{
		return;
	}

	if ( (m_oOrientationValue.getData()[0] == (int)eOrientationInvalid) || (m_oOutLeftX.getRank()[0] < 1 ) || (m_oOutRightX.getData()[0] < 1) )
	{
		return;
	}

	const Trafo		&rTrafo				( *m_oSpTrafo );
	OverlayCanvas	&rCanvas			( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerPosition		( rCanvas.getLayerPosition());

	Color oColLeft, oColRight;

	if (m_oOrientation == eOrientationConvex)
	{
		oColLeft = Color::White(); oColRight = Color::Yellow();
	} else
	{
		oColLeft = Color::Yellow(); oColRight = Color::White();
	}
	rLayerPosition.add( new	OverlayCross(rTrafo( Point((int)m_oOutLeftX.getData().front(), 150+(int)(m_oOutLeftY.getData().front()+0.5) ) ), oColLeft ) ); // paint first position
	rLayerPosition.add( new	OverlayCross(rTrafo( Point((int)m_oOutRightX.getData().front(), 150+(int)(m_oOutRightY.getData().front()+0.5) ) ), oColRight ) );
}

bool DiscoverRuns::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
  if (p_rPipe.tag() == "KCurveTrend")
	{
		m_pPipeLineInKCurve = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);
	} else if (p_rPipe.tag() == "GradientTrend")
	{
		m_pPipeLineInGradTrend = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);
	} else if (p_rPipe.tag() == "SignatureChanges")
	{
		m_pPipeInSignatureChanges = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);
	}
	return BaseFilter::subscribe( p_rPipe, p_oGroup );
}


// ----------------------------------------------------------

bool DiscoverRuns::isGradTrendDirection(const geo2d::VecDoublearray &p_rGradTrend, const unsigned int p_oPos, const bool p_oDirDown)
{
	std::function<bool (double, double)> oCompFirst;
	std::function<bool (double, double)> oCompSecond;
	if (p_oDirDown)
	{
		oCompFirst = std::greater<double>();
		oCompSecond = std::less<double>();
	} else
	{
		oCompFirst = std::less<double>();
		oCompSecond = std::greater<double>();
	}
	double leps = 0.00000001;

	auto oData = p_rGradTrend[0].getData();
	if ((p_oPos <= 1) || (p_oPos >= oData.size()-1) )
	{
		return false;
	}

	// we allow oCnt pixel to any direction before the sought one.
	int oCnt = 20; int oIdx = p_oPos-1;
	double foundStep = (oData[oIdx] - oData[p_oPos]);
	while ( (oIdx > 1) && (oCnt > 1) && (std::abs(foundStep) < leps) )
	{
		--oIdx;
		--oCnt;
		foundStep = (oData[oIdx] - oData[p_oPos]);
	}
	if (oCompFirst(foundStep, leps)) {
		return false;
	}

	oCnt = 20; oIdx = p_oPos+1;
	foundStep = (oData[oIdx] - oData[p_oPos]);
	while ( (oIdx < (int)(oData.size())-1) && (oCnt > 1) && (std::abs(foundStep) < leps) )
	{
		++oIdx;
		--oCnt;
		foundStep = (oData[oIdx] - oData[p_oPos]);
	}
	if (oCompSecond(foundStep, -leps)) {
		return false;
	}

	return true;
}

bool DiscoverRuns::isGradTrendDownwards(const geo2d::VecDoublearray &p_rGradTrend, const unsigned int p_oPos)
{
	return isGradTrendDirection(p_rGradTrend, p_oPos, true);
}

bool DiscoverRuns::isGradTrendUpwards(const geo2d::VecDoublearray &p_rGradTrend, const unsigned int p_oPos)
{
	return isGradTrendDirection(p_rGradTrend, p_oPos, false);
}

// eps-neighbourhood entered?
bool DiscoverRuns::isNearSigChangeVal(const double p_oVal, bool &r_oEntered)
{
	double oMin = m_oValAtSigChange - m_oValEpsOK; double oMax = m_oValAtSigChange + m_oValEpsOK;

	//return true;
	r_oEntered = ( (oMin <= p_oVal) && (p_oVal <= oMax) );
	return r_oEntered;
}


//todo: rank test!!
auto DiscoverRuns::findEndpoints(int &p_rPosMin, double &p_rValMin, int &p_rPosMax, double &p_rValMax,
	const geo2d::VecDoublearray &p_rKCurveLine, const geo2d::VecDoublearray &p_rGradTrend) -> stateOfAnalysis
{
	auto oData = p_rKCurveLine[0].getData();
	auto oRank = p_rKCurveLine[0].getRank();
	auto oRankTr = p_rGradTrend[0].getRank();
	int minStart, maxStart, minEnd, maxEnd;
	int minStep, maxStep;

	int delta = ((int)std::abs((m_oMinX - m_oMaxX)) >> 2); // search for bead or gap end/start within within length/4 pixel left and right

	int tmpPosMin = -1, tmpPosMax = -1;
	const int oSearchWinSize = (int)m_oSearchWinSize;
	const int oSize = (int)oData.size();

	std::function<bool (double, double)> oCompTo;
	std::function<bool (const geo2d::VecDoublearray&, int)> oCheckTrend;
	if (m_oOrientation == eOrientationConvex)
	{
		oCompTo = std::less_equal<double>();
		oCheckTrend = std::bind(&DiscoverRuns::isGradTrendDownwards, this, std::placeholders::_1, std::placeholders::_2);
		p_rPosMin = -1; p_rValMin = -0.00000001;
		p_rPosMax = -1; p_rValMax = -0.00000001;
		// convex: min is right from max, thus endpoint is right from min, startpoint left from max
		minStep = 1; maxStep = -1;
		minEnd = std::min(oSize, m_oMinX + oSearchWinSize); maxEnd = std::max(0, m_oMaxX - oSearchWinSize);
		minStart = std::min(oSize, m_oMinX - delta); maxStart = std::max(0, m_oMaxX + delta);
	} else
	{
		oCompTo = std::greater_equal<double>();
		oCheckTrend = std::bind(&DiscoverRuns::isGradTrendUpwards, this, std::placeholders::_1, std::placeholders::_2);
		p_rPosMin = -1; p_rValMin = 0.00000001;
		p_rPosMax = -1; p_rValMax = 0.00000001;
		// concav: min is left from max, thus endpoint is right from max, startpoint left from min
		minStep = -1; maxStep = 1;
		minEnd = std::max(0, m_oMinX - oSearchWinSize); maxEnd = std::min(oSize, m_oMaxX + oSearchWinSize);
		minStart = std::max(0, m_oMinX + delta); maxStart = std::min((int)oSize, m_oMaxX - delta);
	}

	bool oEntered = false;
	int i=minStart; int oLastCompPosMin = -1;

	while ( i != minEnd )
	{
		if ( oCompTo(oData[i], p_rValMin) )
		{

			if (minStep > 0)
			{
				oLastCompPosMin = i;
			}
			if ( oCheckTrend(p_rGradTrend, i) )
			{
				//	if (!oEntered)
				if ( isNearSigChangeVal( (p_rGradTrend[0].getData())[i], oEntered ) )
				{
					p_rPosMin = i;
				} else
				{
					tmpPosMin = i;
				}
			}
		}
		i += minStep;
	}

	oEntered = false;
	i = maxStart; int oFirstCompPosMax = -1;
	while ( i != maxEnd )
	{
		if ( oCompTo(oData[i], p_rValMax) )
		{
			if ( (oFirstCompPosMax < 0) && (maxStep > 0) )
			{
				oFirstCompPosMax = i;
			}
			if ( oCheckTrend(p_rGradTrend, i) ) // correct direction?
			{
				if ( isNearSigChangeVal( (p_rGradTrend[0].getData())[i], oEntered ) ) // are we near the turning point?
				{
					p_rPosMax = i;
				} else
				{
					tmpPosMax = i;
				}
			}
		}
		i += maxStep;
	}

	stateOfAnalysis oState = eRunOK;
	// fix endpoints
	if (p_rPosMin >= 0)
	{
		p_rValMin = oData[p_rPosMin]; p_rValMin = oData[p_rPosMin];
	} else if (tmpPosMin >= 0)
	{
		p_rPosMin = tmpPosMin; p_rValMin = oData[tmpPosMin];
	} else if (oLastCompPosMin >= 0)
	{
		p_rPosMin = oLastCompPosMin; p_rValMin = oData[oLastCompPosMin];
	} else
	{
		oState = eRunNoRunFound;
	}

	if (p_rPosMax > 0)
	{
		p_rValMax = oData[p_rPosMax]; p_rValMax = oData[p_rPosMax];
	} else if (tmpPosMax >= 0)
	{
		p_rPosMax = tmpPosMax; p_rValMax = oData[tmpPosMax];
	} else if (oFirstCompPosMax >= 0)
	{
		p_rPosMax = oFirstCompPosMax; p_rValMax = oData[oFirstCompPosMax];
	} else
	{
		oState = eRunNoRunFound;
	}
	return oState;
}

auto DiscoverRuns::determineOrientation(const geo2d::VecDoublearray &p_rGradTrendLine, const geo2d::VecDoublearray &p_rKCurveTrendLine) -> stateOfAnalysis
{
	// Sets member variable m_oOrientation to eOrientationConcav or eOrientationConvex on success,  eOrientationInvalid on error
	auto oData = p_rGradTrendLine[0].getData();
	auto oRank = p_rGradTrendLine[0].getRank();
	auto oCurveData = p_rKCurveTrendLine[0].getData();

	m_oOrientation = eOrientationInvalid;

	int oMaxEq=-1, oMinEq=-1;
	m_oMaxX = -1; m_oMinX = -1;
	m_oMinY = std::numeric_limits<double>::max();
	m_oMaxY = -(m_oMinY-1); // work around as limits<double>::min() returns the smallest positive number, not the largest negative one

	/* seek max and min of gradientTrend. We not only seek max and min, but also the position to which they extend,
	AND we do both in one loop. Hence we do not use std::max/min_element and std::distance */
	for (int i=m_oStartpos; i <= m_oEndpos; ++i)
	{
		if (oData[i] >= m_oMaxY)
		{
			if (oData[i] == m_oMaxY)
			{
				oMaxEq = i;
			} else
			{
				m_oMaxX = i; m_oMaxY = oData[i]; // does the maximum value continue?
			}
		}
		if (oData[i] <= m_oMinY)
		{
			if (oData[i] == m_oMinY)
			{
				oMinEq = i;;
			} else
			{
				m_oMinX = i; m_oMinY = oData[i]; // does the mininum value continue?
			}
		}
	}
	/* min != max found?
	* The idea of shifting the markers is the following: We traverse the line from left to right. Here,
		the left marker remains on the first extremal position and thus on the start of the event.
		In the same mammer, the right marker being also on the first extremal position might not be the absolute end of
		a potential chain of equal extremal values.
		By the means of an example:
		Find start and end of 'a's in the following string
			"oo_aaaaaa_bcbdeebdebdb_aaaaa_ooxo"
		shows that we have to use positions of first occurance of an "a" at the left, the last at the right end respectively.
   */
	if ( (m_oMaxX >= 0) && (m_oMinX >= 0) && (std::abs(m_oMinX - m_oMaxX) > 5) ) // no bead or gap smaller than 5 pixel...
	{
		// determin orientation
		if (m_oMaxX < m_oMinX)
		{
			m_oOrientation = eOrientationConvex;
			while ( (m_oMinX  < oMinEq) && (math::sgn(oCurveData[m_oMinX]) == math::sgn(oCurveData[m_oMinX+1]) ) )
			{
				++m_oMinX;// = oMinEq;// m_oMinY = oValMinEq; // shift right marker pos
			}
			m_oMinY = oData[m_oMinX];
		} else
		{
			m_oOrientation = eOrientationConcave;
			while ( (oMaxEq > m_oMaxX) && (math::sgn(oCurveData[m_oMaxX]) == math::sgn(oCurveData[m_oMaxX+1]) ) )
			{
				++m_oMaxX;// = oMaxEq; //m_oMaxY = oValMaxEq; // shift left pos
			}
			m_oMaxY = oData[m_oMaxX];
		}
		return eRunOK;
	}
	return eRunInvalidOrientation;
}

int DiscoverRuns::getSigChangeIndex(const geo2d::VecDoublearray& p_rSigChanges)
{
	SigChange oSigChange;

	auto oData = p_rSigChanges[0].getData();
	if ( (oData.size() <= 0) || (oData.size() % 2) )
	{
		return -1; // insufficient data!
	}

	// start from middle of potential seam run
	int oMiddle = (int)((m_oMinX + m_oMaxX)*0.5);
	int oIdx = -1; double oDelta = std::numeric_limits<double>::max();

	// iterate through all signature changes:
	for (unsigned int i=0; i < oData.size()-1; i += 2)
	{
		if ( (std::abs(oData[i+1]) >= 1) && (std::abs(oData[i]-oMiddle) < oDelta) )
		{
			oIdx = (int)oData[i]; oDelta = std::abs(oData[i]-oMiddle);
		}
	}
	return oIdx;
}

// is there a run?
auto DiscoverRuns::verifyRun( int &p_rPosMin, double &p_rValMin, int &p_rPosMax, double &p_rValMax,
	const geo2d::VecDoublearray &p_rGradTrendLine, const geo2d::VecDoublearray &p_rKCurveTrendLine, const geo2d::VecDoublearray &p_rSigChanges) -> stateOfAnalysis
{
	stateOfAnalysis oState = determineOrientation(p_rGradTrendLine, p_rKCurveTrendLine);
	if ( oState == eRunOK )
	{
		m_oValAtSigChange = 0.0; m_oValEpsOK= -1.0;
		int oPosSigChange = getSigChangeIndex(p_rSigChanges);
		if (oPosSigChange > -1)
		{
			m_oValAtSigChange = p_rGradTrendLine[0].getData()[oPosSigChange];
			m_oValEpsOK = std::numeric_limits<double>::lowest();
			for (int j=std::max(0, (int)(oPosSigChange-m_oTurningPointEps)); j < std::min((int)(p_rGradTrendLine[0].getData().size()), (int)(oPosSigChange+m_oTurningPointEps)); ++j)
			{
				if ( std::abs(p_rGradTrendLine[0].getData()[j]-m_oValAtSigChange) > m_oValEpsOK)
				{
					m_oValEpsOK = std::abs(p_rGradTrendLine[0].getData()[j]-m_oValAtSigChange);
				}
			}
			if ( (m_oOrientation != eOrientationInvalid) && (m_oValEpsOK >= 0.0) )
			{
				auto oFound = findEndpoints(p_rPosMin, p_rValMin, p_rPosMax, p_rValMax, p_rKCurveTrendLine, p_rGradTrendLine);
				if ( (p_rPosMax >= (int)p_rGradTrendLine[0].size()) || (p_rPosMax >= (int)p_rKCurveTrendLine[0].size()) || (p_rPosMin <= 1) )
				{
					oFound = eRunNoRunFound;
				}
				return oFound;
			}
			return eRunNoRunFound;
		} else
		{
			return eRunNoRunFound;
		}
	}
	return oState;
}

bool DiscoverRuns::isValidLine(const geo2d::VecDoublearray &p_rLine)
{
	if ( (p_rLine[0].size() < 1) || (p_rLine[0].getRank().size() < 1) || (p_rLine[0].getData().size() < 1) )
	{
		return false;
	}
	auto oRank = p_rLine[0].getRank();
	int oSize = oRank.size();
	m_oStartpos = 0; m_oEndpos = oRank.size()-1;

	// allow margins of rank eRankMin...
	while ( (m_oStartpos < m_oEndpos) && (oRank[m_oStartpos] < m_oRankThreshold) )
	{
		++m_oStartpos;
	}
	if (m_oStartpos >= m_oEndpos-1)
	{
		return false;
	}

	while ( (m_oEndpos > m_oStartpos) && (oRank[m_oEndpos] < m_oRankThreshold) )
	{
		--m_oEndpos;
	}
	if (m_oEndpos <= m_oStartpos+1)
	{
		return false;
	}

	// we allow 5% missing points at each margins (following discussion BA & JS on 2013/04/25)
	if ( ((1.0*m_oStartpos/oSize) > m_oMarginSize) || ( ((1.0*oSize - m_oEndpos)/oSize) > m_oMarginSize) )
	{
		return false;
	}

	// ...but no inner point
	for (int i=m_oStartpos; i <= m_oEndpos; ++i)
	{
		if (oRank[i] < m_oRankThreshold)
		{
			return false;
		} else
		{
			oRank[i] = eRankMax;
		}
	}
	return true;
}

// -------------------------------------------------------------------------

void DiscoverRuns::signalSend(const ImageContext &p_rImgContext, ResultType p_oAnalysisResult, const int p_oIO)
{
	GeoDoublearray oGeoDoublearrayOutLeftX, oGeoDoublearrayOutLeftY, oGeoDoublearrayOutRightX,oGeoDoublearrayOutRightY, oGeoDoublearrayOutOrientation;

	if (p_oIO > 0)
	{
		oGeoDoublearrayOutLeftX = GeoDoublearray(p_rImgContext, m_oOutLeftX, p_oAnalysisResult, 1.0);
		oGeoDoublearrayOutLeftY = GeoDoublearray(p_rImgContext, m_oOutLeftY, p_oAnalysisResult, 1.0);
		oGeoDoublearrayOutRightX = GeoDoublearray(p_rImgContext, m_oOutRightX, p_oAnalysisResult, 1.0);
		oGeoDoublearrayOutRightY = GeoDoublearray(p_rImgContext, m_oOutRightY, p_oAnalysisResult, 1.0);
		oGeoDoublearrayOutOrientation = GeoDoublearray(p_rImgContext, m_oOrientationValue, p_oAnalysisResult, 1.0);
	} else
	{
		if (p_oIO == 0)
		{
			const auto oAnalysisResult	= interface::AnalysisOK;
			oGeoDoublearrayOutLeftX = GeoDoublearray(p_rImgContext, m_oOutLeftX, oAnalysisResult, interface::NotPresent); // bad rank
			oGeoDoublearrayOutLeftY = GeoDoublearray(p_rImgContext, m_oOutLeftY, oAnalysisResult, interface::NotPresent); // bad rank
			oGeoDoublearrayOutRightX = GeoDoublearray(p_rImgContext, m_oOutRightX, oAnalysisResult, interface::NotPresent); // bad rank
			oGeoDoublearrayOutRightY = GeoDoublearray(p_rImgContext, m_oOutRightY, oAnalysisResult, interface::NotPresent); // bad rank
			oGeoDoublearrayOutOrientation = GeoDoublearray(p_rImgContext, m_oOrientationValue, oAnalysisResult, interface::NotPresent); // rank ok, no bead found
		} else
		{
            //std::cout << "***DiscoverRuns::signalSend() AnalysisError -> interface::AnalysisErrBadLaserline (1201)" << std::endl;
            wmLog( eDebug,"***DiscoverRuns::signalSend() AnalysisError -> interface::AnalysisErrBadLaserline (1201)\n");
			oGeoDoublearrayOutLeftX = GeoDoublearray(p_rImgContext, m_oOutLeftX, p_oAnalysisResult, interface::NotPresent); // bad rank
			oGeoDoublearrayOutLeftY = GeoDoublearray(p_rImgContext, m_oOutLeftY, p_oAnalysisResult, interface::NotPresent); // bad rank
			oGeoDoublearrayOutRightX = GeoDoublearray(p_rImgContext, m_oOutRightX, p_oAnalysisResult, interface::NotPresent); // bad rank
			oGeoDoublearrayOutRightY = GeoDoublearray(p_rImgContext, m_oOutRightY, p_oAnalysisResult, interface::NotPresent); // bad rank
			oGeoDoublearrayOutOrientation = GeoDoublearray(p_rImgContext, m_oOrientationValue, p_oAnalysisResult, interface::NotPresent); // bad rank, laserline problem
		}
	}
	preSignalAction();
	m_oPipePositionOutLeftX.signal( oGeoDoublearrayOutLeftX );
	m_oPipePositionOutLeftY.signal( oGeoDoublearrayOutLeftY );
	m_oPipePositionOutRightX.signal( oGeoDoublearrayOutRightX );
	m_oPipePositionOutRightY.signal( oGeoDoublearrayOutRightY );
	m_oPipeOutOrientation.signal( oGeoDoublearrayOutOrientation );
}

// -------------------------------------------------------------------------

void DiscoverRuns::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeLineInKCurve != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeLineInGradTrend != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInSignatureChanges != nullptr);

	m_oOrientation = eOrientationInvalid;

	// Read-out laserline and trafo
	const GeoVecDoublearray& rLineInKCurve = m_pPipeLineInKCurve->read(m_oCounter);
	const GeoVecDoublearray& rLineInGradTrend = m_pPipeLineInGradTrend->read(m_oCounter);
	const GeoVecDoublearray& rInSigChanges = m_pPipeInSignatureChanges->read(m_oCounter);

	m_oSpTrafo	= rLineInKCurve.context().trafo();

	m_oOutLeftX.getData()[0]=-1.0; m_oOutLeftX.getRank()[0]=1;
	m_oOutLeftY.getData()[0]=0.0; m_oOutLeftY.getRank()[0]=1;
	m_oOutRightX.getData()[0]=-1.0; m_oOutRightX.getRank()[0]=1;
	m_oOutRightY.getData()[0]=0.0; m_oOutRightY.getRank()[0]=1;
	m_oOrientationValue.getData()[0] = eOrientationInvalid; m_oOrientationValue.getRank()[0]=1;

	if ( !isValidLine(rLineInKCurve.ref()) || !isValidLine(rLineInGradTrend.ref()) ||
		!(*rLineInKCurve.context().trafo().get() == *rLineInGradTrend.context().trafo().get()) )
	{
		if ( !(*rLineInKCurve.context().trafo().get() == *rLineInGradTrend.context().trafo().get()) )
		{
			wmLog(eInfo, "QnxMsg.Filter.boundsTrafos", "Filter DiscoverRuns: Different trafos for incoming pipes");
		}
		m_oOutLeftX.getRank()[0]=0; m_oOutLeftY.getRank()[0]=0;
		m_oOutRightX.getRank()[0]=0; m_oOutRightY.getRank()[0]=0;
		m_oOrientationValue.getRank()[0]=0;
		m_oPaint = false;
		signalSend(rLineInKCurve.context(), rLineInKCurve.analysisResult(), -1);
	} else
	{
		int oPosMin=-1, oPosMax=-1;
		double oValMin=0.0, oValMax=0.0;

        std::pair<precitec::interface::ResultType, int> signalSendData;

		auto oAnalysisResult = rLineInKCurve.analysisResult() == AnalysisOK ? AnalysisOK : rLineInKCurve.analysisResult();
		if (oAnalysisResult == AnalysisOK)
		{
			oAnalysisResult = rLineInGradTrend.analysisResult() == AnalysisOK ? AnalysisOK : rLineInGradTrend.analysisResult();
		}
		if (oAnalysisResult != AnalysisOK)
		{
			m_oOrientation = eOrientationInvalid;
			signalSendData = std::make_pair(oAnalysisResult, 0);
		}

		stateOfAnalysis oFound = verifyRun(oPosMin, oValMin, oPosMax, oValMax, rLineInGradTrend.ref(), rLineInKCurve.ref(), rInSigChanges.ref());

		if (oFound == eRunOK)
		{
			m_oOrientationValue.getData()[0] = m_oOrientation; m_oOrientationValue.getRank()[0]=eRankMax;
			if (m_oOrientation == eOrientationConvex)
			{
				m_oOutLeftX.getData()[0] = oPosMin; m_oOutLeftY.getData()[0] = oValMin;
				m_oOutRightX.getData()[0] = oPosMax; m_oOutRightY.getData()[0] = oValMax;
			} else
			{
				m_oOutLeftX.getData()[0] = oPosMax; m_oOutLeftY.getData()[0] = oValMax;
				m_oOutRightX.getData()[0] = oPosMin; m_oOutRightY.getData()[0] = oValMin;
			}
			m_oOutLeftX.getRank()[0] = eRankMax; m_oOutLeftY.getRank()[0] = eRankMax;
			m_oOutRightX.getRank()[0] = eRankMax; m_oOutRightY.getRank()[0] = eRankMax;

			signalSendData = std::make_pair(rLineInKCurve.analysisResult(), 1);

		} else
		{
			m_oOrientation = eOrientationInvalid;
			signalSendData = std::make_pair(AnalysisErrNoBeadOrGap, -1);
		}
		signalSend(rLineInKCurve.context(), signalSendData.first, signalSendData.second);
	}
} // proceed


} // namespace precitec
} // namespace filter
