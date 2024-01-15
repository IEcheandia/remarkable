// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include "module/moduleLogger.h"
#include "geo/array.h"
#include "geo/geo.h"

#include "filter/algoArray.h"	// algorithmic interface for class TArray

// local includes
#include "findBeadEnds.h"
#include "2D/avgAndRegression.h" // for sgn

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string FindBeadEnds::m_oFilterName = std::string("FindBeadEnds");
const std::string FindBeadEnds::PIPENAME_OUTLeftX = std::string("MarkerLeftX");
const std::string FindBeadEnds::PIPENAME_OUTLeftY = std::string("MarkerLeftY");
const std::string FindBeadEnds::PIPENAME_OUTRightX = std::string("MarkerRightX");
const std::string FindBeadEnds::PIPENAME_OUTRightY = std::string("MarkerRightY");
const std::string FindBeadEnds::PIPENAME_OUTOrientation = std::string("Orientation");

const int FindBeadEnds::m_oRankThreshold = (int)(eRankMax*0.95);

FindBeadEnds::FindBeadEnds() :
	TransformFilter( FindBeadEnds::m_oFilterName, Poco::UUID{"1C7F4872-6914-4C0C-BFDA-F437E8D61229"} ),
	m_pPipeLineIn(nullptr), m_pPipeLineInGradTrend(nullptr),
	m_pPipeInRoiLeft(nullptr), m_pPipeInRoiRight(nullptr),
	m_oPipePositionOutLeftX( this, FindBeadEnds::PIPENAME_OUTLeftX ), m_oPipePositionOutLeftY( this, FindBeadEnds::PIPENAME_OUTLeftY ),
	m_oPipePositionOutRightX( this, FindBeadEnds::PIPENAME_OUTRightX ), m_oPipePositionOutRightY( this, FindBeadEnds::PIPENAME_OUTRightY ),
	m_oPipeOutOrientation( this, FindBeadEnds::PIPENAME_OUTOrientation ),
	m_oOrientation(eOrientationInvalid),
	m_oRoiLeft( 20, 50, 150, 50 ), m_oRoiRight( 300, 50, 150, 50 ),
	m_oEndLeft(0.84), m_oEndRight(0.84),
	// outlier and notches
	m_oOutlierLeft(0.0), m_oOutlierRight(0.0),
	m_oStartpos(-1), m_oEndpos(-1), m_oSlope(0.0), m_oIntercept(0.0), m_oPaint(true), m_oAnalysisResult(interface::AnalysisOK)
{
	// Set default values of the parameters of the filter
	m_oOutLeftX.getData().resize(1); m_oOutLeftX.getRank().resize(1);
	m_oOutLeftY.getData().resize(1); m_oOutLeftY.getRank().resize(1);
	m_oOutRightX.getData().resize(1); m_oOutRightX.getRank().resize(1);
	m_oOutRightY.getData().resize(1); m_oOutRightY.getRank().resize(1);
	m_oOrientationValue.getData().resize(1); m_oOrientationValue.getRank().resize(1);

	// left end parameter
	parameters_.add("EndLeft", Parameter::TYPE_double, static_cast<double>(m_oEndLeft));
	// left roi notches and outlier
	parameters_.add("OutlierLeft", Parameter::TYPE_double, static_cast<double>( m_oOutlierLeft ) ); // % of outliers = pixel not in left ROI
	parameters_.add("OutlierLeftDir", Parameter::TYPE_int, static_cast<double>( m_oDirOutlierLeft ) ); // % direction (none, above, below, both)
	parameters_.add("NotchesLeft", Parameter::TYPE_double, static_cast<double>( m_oNotchesLeft ) ); // % of notches = pixel not in left ROI
	parameters_.add("NotchesLeftDir", Parameter::TYPE_int, static_cast<double>( m_oDirNotchesLeft ) ); // % direction (none, above, below, both)

	// right end parameter
	parameters_.add("EndRight", Parameter::TYPE_double, static_cast<double>(m_oEndRight));
	// right roi notches and outlier
	parameters_.add("OutlierRight", Parameter::TYPE_double, static_cast<double>( m_oOutlierRight ) ); // % of outliers = pixel not in right ROI
	parameters_.add("OutlierRightDir", Parameter::TYPE_int, static_cast<double>( m_oDirOutlierRight ) ); // % direction (none, above, below, both)
	parameters_.add("NotchesRight", Parameter::TYPE_double, static_cast<double>( m_oNotchesRight ) ); // % of notches = pixel not in right ROI
	parameters_.add("NotchesRightDir", Parameter::TYPE_int, static_cast<double>( m_oDirNotchesRight ) ); // % direction (none, above, below, both)

	setInPipeConnectors({{Poco::UUID("93C4F310-5E15-4D3F-839D-611A5316E0E9"), m_pPipeLineIn, "LaserLine", 1, "LaserLine"},
    {Poco::UUID("9208DDF9-DF5E-48D3-82BF-0BB676730ED1"), m_pPipeLineInGradTrend, "GradientTrend", 1, "GradientTrend"},
    {Poco::UUID("EC57727C-0BA3-4CB5-AC47-CA0994883FD9"), m_pPipeInRoiLeft, "RoiLeft", 1, "RoiLeft"},
    {Poco::UUID("909842D7-C9DC-4C07-A033-90CF80E659C6"), m_pPipeInRoiRight, "RoiRight", 1, "RoiRight"}});
    setOutPipeConnectors({{Poco::UUID("38BABD4C-D252-41F8-AE9D-F6EE06BE6DBC"), &m_oPipePositionOutLeftX, PIPENAME_OUTLeftX, 0, ""},
    {Poco::UUID("D72185A9-EEFA-4930-90DE-E3858A909658"), &m_oPipePositionOutLeftY, PIPENAME_OUTLeftY, 0, ""},
    {Poco::UUID("AC8E05D4-934C-4BEB-9092-A9CD636B28DB"), &m_oPipePositionOutRightX, PIPENAME_OUTRightX, 0, ""},
    {Poco::UUID("D0ED019B-ECEC-44CC-A5D5-8EE903420BAA"), &m_oPipePositionOutRightY, PIPENAME_OUTRightY, 0, ""},
    {Poco::UUID("BC476314-3461-49A6-A2A0-A227EF673246"), &m_oPipeOutOrientation, PIPENAME_OUTOrientation, 0, ""}});
    setVariantID(Poco::UUID("6075EB26-1FBB-4B92-A538-DC2C10EE0087"));
}
const double FindBeadEnds::m_oMarginSize = 0.1;

void FindBeadEnds::setParameter()
{
	TransformFilter::setParameter();

	m_oEndLeft = 1.0 - parameters_.getParameter("EndLeft").convert<double>();
	m_oOutlierLeft = parameters_.getParameter("OutlierLeft").convert<double>();
	m_oDirOutlierLeft = parameters_.getParameter("OutlierLeftDir").convert<int>();
	m_oNotchesLeft = parameters_.getParameter("NotchesLeft").convert<double>();
	m_oDirNotchesLeft = parameters_.getParameter("NotchesLeftDir").convert<int>();

	m_oEndRight = 1.0 - parameters_.getParameter("EndRight").convert<double>();
	m_oOutlierRight = parameters_.getParameter("OutlierRight").convert<double>();
	m_oDirOutlierRight = parameters_.getParameter("OutlierRightDir").convert<int>();
	m_oNotchesRight = parameters_.getParameter("NotchesRight").convert<double>();
	m_oDirNotchesRight = parameters_.getParameter("NotchesRightDir").convert<int>();
}

void FindBeadEnds::paint() {

	if (m_oSpTrafo.isNull())
	{
		return;
	}

	const Trafo		&rTrafo				( *m_oSpTrafo );
	OverlayCanvas	&rCanvas			( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerLine	( rCanvas.getLayerLine());

	Color oColLeft, oColRight;

	if (m_oVerbosity >= eLow)
	{
		rLayerLine.add(new OverlayLine(m_oRoiLeft.x().start(), m_oRoiLeft.y().start()+(m_oRoiLeft.height()/2), m_oRoiLeft.width()+m_oRoiLeft.x().start(), m_oRoiLeft.y().start()+(m_oRoiLeft.height()/2), Color::Green()) );
		rLayerLine.add(new OverlayLine(m_oRoiRight.x().start(), (m_oRoiRight.y().start()+(m_oRoiLeft.height()/2)), m_oRoiRight.width()+m_oRoiRight.x().start(), (m_oRoiRight.y().start()+(m_oRoiLeft.height()/2)), Color::Green()) );
	}

	if (m_oPaint && (m_oVerbosity >= eMedium) )
	{
		for (size_t i = 0; i < m_oLine.size(); ++i)
		{
			rLayerLine.add(new OverlayPoint(rTrafo(m_oLine[i]), Color::Green()));
		}
	}
}

bool FindBeadEnds::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
  if (p_rPipe.tag() == "LaserLine")
	{
		m_pPipeLineIn = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);
	} else if (p_rPipe.tag() == "GradientTrend")
	{
		m_pPipeLineInGradTrend = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);
	} else if (p_rPipe.tag() == "RoiLeft")
	{
		m_pPipeInRoiLeft = dynamic_cast< fliplib::SynchronePipe< interface::ImageFrame >* >(&p_rPipe);
	} else if (p_rPipe.tag() == "RoiRight")
	{
		m_pPipeInRoiRight = dynamic_cast< fliplib::SynchronePipe< interface::ImageFrame >* >(&p_rPipe);
	}
	return BaseFilter::subscribe( p_rPipe, p_oGroup );
}


// ----------------------------------------------------------

bool FindBeadEnds::inROIX(const int p_oX, const geo2d::Rect &p_rRoi)
{
	return ( ( (p_oX+m_oSpTrafo->dx()) >= p_rRoi.x().start() ) && ( (p_oX+m_oSpTrafo->dx()) < (p_rRoi.x().start() + p_rRoi.width()) ) );
}


bool FindBeadEnds::inROIY(const int p_oY, const geo2d::Rect &p_rRoi)
{
	return ( ( (p_oY+m_oSpTrafo->dy()) >= p_rRoi.y().start() ) && ( (p_oY+m_oSpTrafo->dy()) < (p_rRoi.y().start() + p_rRoi.height()) ) );
}

//todo: rank test!!
auto FindBeadEnds::findEndpoint(int &p_rX, double &p_rY,
	const int p_oMin, const int p_oMax, const bool p_oRightToLeft,
	const geo2d::VecDoublearray &p_rLine, const geo2d::VecDoublearray &p_rGradTrend) -> stateOfAnalysis
{
	if (m_oOrientation == eOrientationInvalid)
	{
		m_oAnalysisResult = interface::AnalysisErrNoBeadOrGap;
		return eBeadNoBeadFound;
	}

	// Assumes m_oOrientation is precomputed
	auto oLine = p_rLine[0].getData();
	auto oData = p_rGradTrend[0].getData(); // was GradTrend
	auto oRank = p_rGradTrend[0].getRank();

	// initializations
	int oLeft = p_oMin; int oRight = p_oMax;

	int oDirection = 1; // right end of potential gap/bead
	double oEndMarker(m_oEndRight);
	if (p_oRightToLeft) // this means we are searching for the left end of a potential gap/bead
	{
		oDirection *= -1;
		std::swap(oLeft, oRight);
		oEndMarker = m_oEndLeft;
	}

	//std::cout << "from " << oLeft;
	p_rX = oLeft; p_rY = oData[oLeft];

	std::function<bool (double, double)> oCmpData;
	if ( ( (m_oOrientation == eOrientationConvex) && p_oRightToLeft) || ( (m_oOrientation == eOrientationConcave) && !p_oRightToLeft) )
	{
		oCmpData = std::less_equal<double>();
	} else
	{
		oCmpData = std::greater_equal<double>();
	}


	/* Find extremum subject to parameters m_oEndLeft, m_oEndRight */
	for (int i = oLeft+1; i != oRight; i += oDirection)
	{
		if (  (std::abs(oData[i]) > oEndMarker) && oCmpData(oData[i], p_rY) )
		{
			p_rX = i; p_rY = oData[i]; // does the maximum value continue?
		}
	}

	p_rY = oLine[p_rX];
	//std::cout << "x " << p_rX << "\n";
	/* min != max found?
	* The idea of shifting the endpositions is the following: We traverse the line from left to right.
		The left position falls onto the first of a potential sequence of extremal position and thus on the start of the event.
		In the same manner, the right position will be shifted to the last extremal position. In both cases, the constraints determined
		by the parameters m_oEndLeft, m_oEndRight are regarded.
		By the means of an example, ignoring the constraints:
		Find start and end of 'a's in the following string
			"oo_Aaaaaa_bcbdeebdebdb_aaaaA_ooxo"
		Capital 'A's depict the final positions.
   */

	bool oInRoi(true);
	if (p_oRightToLeft)  // left end of bead/gap
	{
		while ((p_rX > (oRight + 1)) && (std::abs(math::sgn(oData[p_rX]) - math::sgn(oData[p_rX - 1])) < math::eps) && (std::abs(oData[p_rX - 1]) > m_oEndLeft)) // testing for oRight as oLeft and oRight were swapped before...
		{
			--p_rX; // shift right marker pos
		}
		p_rY = oLine[p_rX];

		if (!inROIY(int(p_rY), m_oRoiLeft))
		{
			oInRoi =false;
		}


	} else
	{
		while ( (p_rX < (oRight-1)) && ( std::abs( math::sgn(oData[p_rX]) - math::sgn(oData[p_rX+1]) )  < math::eps) && ( std::abs(oData[p_rX+1]) > m_oEndRight) )
		{
			++p_rX;  // shift left pos
		}
		p_rY = oLine[p_rX];


		if (!inROIY(int(p_rY), m_oRoiRight))
		{
			oInRoi =false;
		}

	}
	if (!oInRoi)
	{
		m_oAnalysisResult = interface::AnalysisErrNoBeadOrGap;
		return eBeadAnalysisError;
	}
	return eBeadOK;
}

bool FindBeadEnds::isOutlier(const int p_oTestDirection, const unsigned int p_oNumOutliersAbove, const unsigned int p_oNumOutliersBelow,
	const double p_oArea, const double p_oPercent) const
{
	// Directions enum: None=0, Above = 1, Below = 2, Both = 3
	if ( (p_oTestDirection % 2) == 1 )
	{
		if ( static_cast<double>(p_oNumOutliersAbove * p_oArea) >= p_oPercent  )
		{
			return true;
		}
	} else if (p_oTestDirection >= 2)
	{
		if ( static_cast<double>(p_oNumOutliersBelow * p_oArea) >= p_oPercent  )
		{
			return true;
		}
	}
	return false;
}

// computes y-value averages of line in roi and returns valid boundaries for
auto FindBeadEnds::getBaseLinePos(int &p_rX, double &p_rY, int &p_rMin, int &p_rMax, const bool p_oLeftRoi,
		const geo2d::VecDoublearray &p_rLine) -> stateRoiArea
{
	auto oData = p_rLine[0].getData();
	auto oRoiY( m_oRoiLeft.y().start() );
	auto oRoiHeight( m_oRoiLeft.height() );

	int oStartPos = m_oStartpos;// + m_oSpTrafo->dx();
	int oEndPos = m_oEndpos;// + m_oSpTrafo->dx();      // to compare main ROI with limiting ROIs we need an absolute translation
	int oOffsetY = m_oSpTrafo->dy();

	//std::cout << "Trafo-X: " << m_oSpTrafo->dx() << "; X: " << m_oRoiLeft.x().start() << std::endl;

	// boundary check
	if (p_oLeftRoi)
	{
		p_rMin = std::max(oStartPos, m_oRoiLeft.x().start());
		p_rMax = std::max(oStartPos, m_oRoiLeft.x().end());
		if (oStartPos <= m_oRoiLeft.x().start())
		{
			if ((p_rMax >= m_oRoiLeft.x().start()))
			{
				p_rMax -= m_oSpTrafo->dx();
			}
			if ((p_rMin >= m_oRoiLeft.x().start()))
			{
				p_rMin -= m_oSpTrafo->dx();
			}
		}
	} else
	{
		p_rMin = std::max(oStartPos, m_oRoiRight.x().start());
		p_rMax = std::max(oStartPos, m_oRoiRight.x().end());
		if (oStartPos <= m_oRoiRight.x().start())
		{
			if ((p_rMax >= m_oRoiRight.x().start()))
			{
				p_rMax -= m_oSpTrafo->dx();
			}
			if ((p_rMin >= m_oRoiRight.x().start()))
			{
				p_rMin -= m_oSpTrafo->dx();
			}
		}
		oRoiY = m_oRoiRight.y().start();
		oRoiHeight = m_oRoiRight.height();
	}
	//std::cout << "min before: " << p_rMin << std::endl;

	if (p_rMax <= p_rMin)
	{
		m_oAnalysisResult = interface::AnalysisErrBadROI;
		return eBadRoi;
	}

	p_rMax = std::min(p_rMax, oEndPos);
	//std::cout << "min after: " << p_rMax << std::endl;

	// left ROI: check for outliers and notches

	p_rX = p_rMax;
	p_rY = oData[p_rMin];
	double oMin = oData[p_rMin]; double oMax = oData[p_rMin];

	unsigned int oOutlierAbove(0);
	unsigned int oOutlierBelow(0);

	// compute averages
	for (int i=p_rMin+1; i <= p_rMax;++i)
	{
		if ( (oData[i] + oOffsetY) < oRoiY )
		{
			++oOutlierAbove;
		}
		if ( (oData[i] + oOffsetY) > (oRoiY + oRoiHeight) )
		{
			++oOutlierBelow;
		}

		// also potential alternative: not avg. but highest position on screen to avoid potential flaws when Randkerbe lies below laserline
		if (oData[i] < oMin)
		{
			//p_rYLeft = oData[i]
			oMin = oData[i];
		}
		if (oData[i] > oMax)
		{
			oMax = oData[i];
		}

		p_rY += oData[i];
		/* alternative
		p_rY = oMax;
		*/
	}
	double oWidth = 1.0/(p_rMax - p_rMin);
	p_rY *= oWidth; // comment out when using alternative

	stateRoiArea oRoiState(eRoiOK);
	bool oIsOutlier(false);
	if (p_oLeftRoi)
	{
		oIsOutlier = isOutlier(m_oDirOutlierLeft, oOutlierAbove, oOutlierBelow, oWidth, m_oOutlierLeft);
	}
	else
	{
		oIsOutlier = isOutlier(m_oDirOutlierRight, oOutlierAbove, oOutlierBelow, oWidth, m_oOutlierRight);
	}
	if (oIsOutlier)
	{
		m_oAnalysisResult = interface::AnalysisErrDefectiveSeam;
	}

	/* todo: notches
	if ((m_oOutlierLeft > math::eps) && ((oMax - oMin)  > m_oRoiLeft.height()*m_oOutlierLeft))
	{
		m_oAnalysisResult = interface::AnalysisErrNotchDefect;
		std::cout << "\n*******ACUIOGWEO*FUIA\n";
	}
	*/

	return oRoiState;
}

// input data p_oXLeft, ..., p_oYRight are results from getBaseLinePos(...)
RunOrientation FindBeadEnds::determineOrientation(const geo2d::VecDoublearray &p_rLine,
		const int p_oXLeft, const double p_oYLeft, const int p_oXRight, const double p_oYRight)
{
	// left and right positions are assumed to be correct and ordered from former computations!

	if (p_oXRight <= p_oXLeft)
	{
		return eOrientationInvalid;
	}

	// get laserline
	auto oData = p_rLine[0].getData();

	// get baseline
	m_oSlope = (p_oYRight - p_oYLeft)/(p_oXRight - p_oXLeft);
	m_oIntercept = ((p_oYLeft - m_oSlope*p_oXLeft) + (p_oYRight - m_oSlope*p_oXRight))*0.5;
	double oValue = 0.0; // value of baseline

	// find max and min of line in area between left and right ROI
	double oMinY = 0; double oMaxY = 0;
	double oDelta = 0.0;
	for (int i=p_oXLeft; i <= p_oXRight; ++i)
	{
		oValue = m_oSlope*i + m_oIntercept;
		oDelta = (oData[i] - oValue);
		if (oDelta <=0)  // laserline above or on baseline
		{
			oMinY = std::min(oMinY, oDelta);
		} else
		{
			oMaxY = std::max(oMaxY, oDelta);
		}
	}

	// convex or concave?
	if (std::abs(oMinY) >= std::abs(oMaxY))
	{
		return eOrientationConvex;
	}
	return eOrientationConcave;
}



// is there a run?
auto FindBeadEnds::verifyRun(int &p_rPosMin, double &p_rValMin, int &p_rPosMax, double &p_rValMax,
	const geo2d::VecDoublearray &p_rLine, const geo2d::VecDoublearray &p_rGradTrendLine) -> stateOfAnalysis
{
	int oXLeftFrom = -1; int oXRightFrom = -1; int oXLeftTo = -1; int oXRightTo = -1; // left and right ROI boundaries
	int oXLeft = -1; int oXRight = -1; // pos oft left and right position of baseline for bead/gap
	double oYLeft = 0.0; double oYRight = 0.0;

	// search ROIs for rough boundaries and check for notches and outliers (sparks, spatter)
	stateRoiArea oRoiLeft = getBaseLinePos(oXLeft, oYLeft, oXLeftFrom, oXLeftTo, true, p_rLine);
	stateRoiArea oRoiRight(eRoiOK);
	if (oRoiLeft == eRoiOK)
	{
		oRoiRight = getBaseLinePos(oXRight, oYRight, oXRightFrom, oXRightTo, false, p_rLine);
	} else
	{
		return eBeadNoBeadFound;
	}
	if ( oRoiRight != eRoiOK )
	{
		// analysisResult has already been altered in function getBaseLinePos
		return eBeadNoBeadFound;
	}
	if (m_oAnalysisResult != interface::AnalysisOK)
	{
		return eBeadAnalysisError;  // todo: trotzdem Nahtraupe analysieren, wenn vorhanden?
	}
	// try to detect orientation
	m_oOrientation = determineOrientation(p_rLine, oXLeft, oYLeft, oXRight, oYRight);
	if (m_oOrientation == eOrientationInvalid)
	{
		m_oAnalysisResult = interface::AnalysisErrNoBeadOrGap;
		return eBeadNoBeadFound;
	}

	// find Endpoints
	p_rPosMin = -1; p_rValMin = 0;
	auto oStateLeft = findEndpoint( p_rPosMin, p_rValMin, oXLeftFrom, oXLeftTo, true,
		p_rLine, p_rGradTrendLine);

	p_rPosMax = -1; p_rValMax = 0;
	auto oStateRight = findEndpoint( p_rPosMax, p_rValMax, oXRightFrom, oXRightTo, false,
		p_rLine, p_rGradTrendLine);

	if ( (oStateLeft == eBeadOK) && (oStateRight == eBeadOK))
	{
		return eBeadOK;
	}

	m_oAnalysisResult = interface::AnalysisErrNoBeadOrGap;
	return eBeadNoBeadFound;
}

// laserline needs to be continuous! No gaps or missing pixel!
bool FindBeadEnds::isValidLine(const geo2d::VecDoublearray &p_rLine)
{
	if ( (p_rLine[0].size() < 1) || (p_rLine[0].getRank().size() < 1) || (p_rLine[0].getData().size() < 1) )
	{
		return false;
	}
	auto oRank = p_rLine[0].getRank();
	int oSize = oRank.size();

	int oStartpos = 0; int oEndpos = oRank.size()-1;

	// allow margins of rank eRankMin...
	while ( (oStartpos < oEndpos) && (oRank[oStartpos] < m_oRankThreshold) )
	{
		++oStartpos;
	}
	if (oStartpos >= oEndpos-1)
	{
		return false;
	}

	while ( (oEndpos > oStartpos) && (oRank[oEndpos] < m_oRankThreshold) )
	{
		--oEndpos;
	}
	if (oEndpos <= oStartpos+1)
	{
		return false;
	}

	// we allow 5% missing points at each margins (following discussion BA & JS on 2013/04/25)
	if ( ((1.0*oStartpos/oSize) > m_oMarginSize) || ( ((1.0*oSize - oEndpos)/oSize) > m_oMarginSize) )
	{
		return false;
	}

	// ...but no inner point
	for (int i=oStartpos; i <= oEndpos; ++i)
	{
		if (oRank[i] < m_oRankThreshold)
		{
			return false;
		} else
		{
			oRank[i] = eRankMax;
		}
	}
	if (m_oEndpos < 0)
	{
		m_oEndpos = oEndpos; m_oStartpos = oStartpos;
	}
	else
	{
		m_oStartpos = std::max(m_oStartpos, oStartpos); m_oEndpos = std::min(m_oEndpos, oEndpos);

	}

	if ( (m_oStartpos < 0) || (m_oEndpos <= m_oStartpos) )
	{
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------

void FindBeadEnds::signalSend(const ImageContext &p_rImgContext, ResultType p_oAnalysisResult, const int p_oIO)
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
			oGeoDoublearrayOutLeftX = GeoDoublearray(p_rImgContext, m_oOutLeftX, oAnalysisResult, 1); // bad rank
			oGeoDoublearrayOutLeftY = GeoDoublearray(p_rImgContext, m_oOutLeftY, oAnalysisResult, 1); // bad rank
			oGeoDoublearrayOutRightX = GeoDoublearray(p_rImgContext, m_oOutRightX, oAnalysisResult, 1); // bad rank
			oGeoDoublearrayOutRightY = GeoDoublearray(p_rImgContext, m_oOutRightY, oAnalysisResult, 1); // bad rank
			oGeoDoublearrayOutOrientation = GeoDoublearray(p_rImgContext, m_oOrientationValue, oAnalysisResult, 1); // rank ok, no bead found
		} else
		{
            //std::cout << "****FindBeadEnds::signalSend() AnalysisError -> interface::AnalysisErrBadLaserline (1201)" << std::endl;
            wmLog( eDebug,"****FindBeadEnds::signalSend() AnalysisError -> interface::AnalysisErrBadLaserline (1201)\n");
			oGeoDoublearrayOutLeftX = GeoDoublearray(p_rImgContext, m_oOutLeftX, p_oAnalysisResult, 1); // bad rank
			oGeoDoublearrayOutLeftY = GeoDoublearray(p_rImgContext, m_oOutLeftY, p_oAnalysisResult, 1); // bad rank
			oGeoDoublearrayOutRightX = GeoDoublearray(p_rImgContext, m_oOutRightX, p_oAnalysisResult, 1); // bad rank
			oGeoDoublearrayOutRightY = GeoDoublearray(p_rImgContext, m_oOutRightY, p_oAnalysisResult, 1); // bad rank
			oGeoDoublearrayOutOrientation = GeoDoublearray(p_rImgContext, m_oOrientationValue, p_oAnalysisResult, 1); // bad rank, laserline problem
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

void FindBeadEnds::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeLineIn != nullptr);
	poco_assert_dbg(m_pPipeLineInGradTrend != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInRoiLeft != nullptr);

	m_oAnalysisResult = interface::AnalysisOK;
	m_oOrientation = eOrientationInvalid;

	// Read-out laserline and trafo
	const auto &rLine = m_pPipeLineIn->read(m_oCounter);
	const auto &rRoiLeft = m_pPipeInRoiLeft->read(m_oCounter);
	const auto &rRoiRight = m_pPipeInRoiRight->read(m_oCounter);
	const GeoVecDoublearray& rLineInGradTrend = m_pPipeLineInGradTrend->read(m_oCounter);

	m_oSpTrafo	= rLine.context().trafo();
	const interface::SmpTrafo &oSpDataRoiLeft(rRoiLeft.context().trafo());
	const interface::SmpTrafo &oSpDataRoiRight(rRoiRight.context().trafo());
	m_oRoiOffsetX = oSpDataRoiLeft->dx() - m_oSpTrafo->dx(); // left and right ROI should lie completely within main ROI. Left one is tested.
	m_oRoiOffsetY = oSpDataRoiLeft->dy() - m_oSpTrafo->dy(); // left and right ROI should lie completely within main ROI. Left one is tested.

	//Nachtraegliche Abfragen zur Vermeidung eines crash aufgrund Zugriffe auf nicht definierte Werte in der Linie **********
	//Start/Stop der Laserlinie
	// laserline offset:
	int lineOffsetX= m_oSpTrafo->dx();

	// left ROI data
	const BImage&			rImageInLeft(rRoiLeft.data());
	const Size2d			oSizeImgInLeft(rImageInLeft.size());
	auto roiLeftX = rRoiLeft.context().trafo()->dx();
	auto roiLeftDX = oSizeImgInLeft.width;

	// right ROI data
	const BImage&			rImageInRight(rRoiRight.data());
	const Size2d			oSizeImgInRight(rImageInRight.size());
	auto roiRightX = rRoiRight.context().trafo()->dx();
	auto roiRightDX = oSizeImgInRight.width;

	const VecDoublearray& rArray = rLine.ref();

	auto rankData = rArray[0].getRank();
	auto data     = rArray[0].getData();
	int lineLength = data.size();

	bool oROIsOK(true);

	if( (roiLeftX<= lineOffsetX) || ((roiLeftX + roiLeftDX) >= lineOffsetX+lineLength  ) )
		oROIsOK = false;

	if ( (roiRightX <= lineOffsetX) || ((roiRightX + roiRightDX) >= lineOffsetX + lineLength))
		oROIsOK = false;

	//*********************************************************************************************************************

	if ( (m_oRoiOffsetX < 0 ) || (oSpDataRoiRight->dx() < oSpDataRoiLeft->dx() ) )
	{
		oROIsOK = false;
	} else
	if ( (m_oRoiOffsetY < 0) || ( (oSpDataRoiRight->dy() - m_oSpTrafo->dy()) < 0) )
	{
		oROIsOK = false;
	}

	m_oRoiLeft = Rect( oSpDataRoiLeft->dx(), oSpDataRoiLeft->dy(),
			rRoiLeft.data().width(), rRoiLeft.data().height() );
	m_oRoiRight = Rect( oSpDataRoiRight->dx(), oSpDataRoiRight->dy(),
			rRoiRight.data().width(), rRoiRight.data().height() );

	m_oOutLeftX.getData()[0]=-1.0; m_oOutLeftX.getRank()[0]=1;
	m_oOutLeftY.getData()[0]=0.0; m_oOutLeftY.getRank()[0]=1;
	m_oOutRightX.getData()[0]=-1.0; m_oOutRightX.getRank()[0]=1;
	m_oOutRightY.getData()[0]=0.0; m_oOutRightY.getRank()[0]=1;
	m_oOrientationValue.getData()[0] = eOrientationInvalid; m_oOrientationValue.getRank()[0]=1;

	if ( !isValidLine(rLine.ref()) || !isValidLine(rLineInGradTrend.ref()) ||
		!(*rLine.context().trafo().get() == *rLineInGradTrend.context().trafo().get()) || !oROIsOK )
	{
		if ( !(*rLine.context().trafo().get() == *rLineInGradTrend.context().trafo().get()) )
		{
			wmLog(eInfo, "QnxMsg.Filter.boundsTrafos", "Filter FindBeadEnds: Different trafos for incoming pipes");
		}
		m_oOutLeftX.getRank()[0]=0; m_oOutLeftY.getRank()[0]=0;
		m_oOutRightX.getRank()[0]=0; m_oOutRightY.getRank()[0]=0;
		m_oOrientationValue.getRank()[0]=0;
		m_oPaint = false;
		signalSend(rLineInGradTrend.context(), rLineInGradTrend.analysisResult(), -1);
	}
	else
	{
        std::pair<precitec::interface::ResultType, int> signalSendData;
		auto oAnalysisResult = rLineInGradTrend.analysisResult() == AnalysisOK ? AnalysisOK : rLineInGradTrend.analysisResult();
		if (oAnalysisResult != AnalysisOK)
		{
			m_oOrientation = eOrientationInvalid;
			signalSendData = std::make_pair(oAnalysisResult, 0);
		}

		int oStartX = -1; double oStartY = 0;
		int oEndX = -1; double oEndY = 0;
		stateOfAnalysis oFound = verifyRun(oStartX, oStartY, oEndX, oEndY, rLine.ref(), rLineInGradTrend.ref());

		//std::cout << "startx: " << oStartX << std::endl;
		if (oFound == eBeadOK)
		{
			m_oLine.clear();
			for (int i = oStartX; i <= oEndX; ++i)
			{
				double oValue = m_oSlope*i + m_oIntercept;
				m_oLine.push_back(geo2d::Point(i, (int)oValue));
			}

			m_oOrientationValue.getData()[0] = m_oOrientation; m_oOrientationValue.getRank()[0]=eRankMax;
			m_oOutLeftX.getData()[0] = oStartX; m_oOutLeftY.getData()[0] = oStartY;
			m_oOutRightX.getData()[0] = oEndX; m_oOutRightY.getData()[0] = oEndY;
			m_oOutLeftX.getRank()[0] = eRankMax; m_oOutLeftY.getRank()[0] = eRankMax;
			m_oOutRightX.getRank()[0] = eRankMax; m_oOutRightY.getRank()[0] = eRankMax;

			m_oPaint = true;

			signalSendData = std::make_pair(rLineInGradTrend.analysisResult(), 1);

		}
		else
		{
			m_oOrientation = eOrientationInvalid;
			signalSendData = std::make_pair(m_oAnalysisResult, -1);
		}
        signalSend(rLineInGradTrend.context(), signalSendData.first, signalSendData.second);
	}
} // proceed


} // namespace precitec
} // namespace filter
