/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		  Andreas Beschorner (BA)
 *  @date		    2012/2013
 *  @brief	    Computes locally averaged/ smoothed gradient
 */

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include "module/moduleLogger.h"

#include "filter/algoArray.h"	///< algorithmic interface for class TArray

// local includes
#include "gradientTrend.h"
#include "2D/avgAndRegression.h"

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string GradientTrend::m_oFilterName = std::string("GradientTrend");
const std::string GradientTrend::PIPENAME_Trend  = std::string("GradientTrend");
const std::string GradientTrend::PIPENAME_Sigs  = std::string("SignatureChanges");

GradientTrend::GradientTrend() : TransformFilter( GradientTrend::m_oFilterName, Poco::UUID{"28A69BE8-86C9-43C3-8E45-F7D1FDF93B9F"} ),
	m_pPipeInLine(NULL), m_oPipeOutTrend( this, GradientTrend::PIPENAME_Trend), m_oPipeOutSigs( this, GradientTrend::PIPENAME_Sigs ),
	m_oWidth(10), m_oShift(1), m_oMinG(0.0), m_oMaxG(0.0)
{
	m_oTrend.resize(1);
	m_oSigChangeVector.resize(1);
	parameters_.add("TrendWinLength", Parameter::TYPE_UInt32, m_oWidth);
	parameters_.add("Shift", Parameter::TYPE_UInt32, m_oShift);

    setInPipeConnectors({{Poco::UUID("B7FE2739-9796-4E1B-A6D2-D3C287B27643"), m_pPipeInLine, "Line", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("D199E85E-4920-40B3-9D61-BFE47679802C"), &m_oPipeOutTrend, PIPENAME_Trend, 0, ""},
    {Poco::UUID("BDC2D1AA-F7ED-4D3A-BE15-B2C145473536"), &m_oPipeOutSigs, PIPENAME_Sigs, 0, ""}});
    setVariantID(Poco::UUID("AE9D52E7-1017-4AB9-92AF-932C3B512463"));
}

GradientTrend::~GradientTrend()
{
}

bool GradientTrend::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeInLine = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray >* >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe

void GradientTrend::setParameter()
{
	TransformFilter::setParameter();
	m_oWidth = parameters_.getParameter("TrendWinLength").convert<unsigned int>();
	m_oShift = parameters_.getParameter("Shift").convert<unsigned int>();

	if (m_oShift < 0)
	{
		m_oShift = 0;
	}
	if (m_oWidth < 3)
	{
		m_oWidth = 3;
	}
	poco_assert_dbg((m_oWidth >= 3) && (m_oShift > 0));
	if (m_oShift >= m_oWidth)
	{
		wmLogTr(eError, "QnxMsg.Filter.dispWidthShift", "Filter GradientTrend: Parameter shift must be smaller than TrendWinLength. Fixed by filter.");
		m_oShift = m_oWidth - 1;
	}
}

// ----------------------------------------------------------------------------

// compute real valued gradient
void GradientTrend::computeGradient( const VecDoublearray &p_rLine, std::vector<double> &p_rGrad, std::vector<int> &p_rGradRank )
{
	unsigned int i;

	for (i=1; i < p_rLine[0].getData().size()-1; ++i)
	{
		if ( (p_rLine[0].getRank()[i] > eRankMin) && (p_rLine[0].getRank()[i-1] > eRankMin) && (p_rLine[0].getRank()[i+1] > eRankMin) )
		{
			p_rGrad[i] = 0.5*(p_rLine[0].getData()[i-1] - p_rLine[0].getData()[i+1]); // direction of gradient inverse, as pixel values increase from top to bottom!
			p_rGradRank[i] = std::min(p_rLine[0].getRank()[i-1], p_rLine[0].getRank()[i+1]);
		} else
		{
			p_rGrad[i] = 0.0; p_rGradRank[i] = eRankMin;
		}
		if (i > 1)
		{
			if (p_rGrad[i] < m_oMinG)
			{
				m_oMinG = p_rGrad[i];
			}
			else if (p_rGrad[i] > m_oMaxG)
			{
				m_oMaxG = p_rGrad[i];
			}
		}
		else
		{
			m_oMinG = p_rGrad[1];  m_oMaxG = p_rGrad[1];
		}
	}
	p_rGrad[0] = p_rGrad[1]; // for continuity
	p_rGradRank[0] = p_rGradRank[1];//1.0*p_rLine[0].getRank()[0];
	p_rGrad[i] = p_rGrad[i-1];
	p_rGradRank[i] = p_rGradRank[i-1];//1.0*p_rLine[0].getRank()[i];
}

void GradientTrend::processSignatureChange(const int m_oPos, const double m_oCurVal, const double m_oPrevVal)
{
	int oSigDiff = math::sgn(m_oPrevVal) - math::sgn(m_oCurVal);
	if (oSigDiff != 0)
	{
		SigChange oChange(m_oPos, oSigDiff);
		m_oSignatureChange.push_back(oChange);
	}
}

bool GradientTrend::isValidLine(const VecDoublearray p_rLine, int &p_rStartPos, int &p_rEndPos)
{
	auto oData = p_rLine[0].getData();
	auto oRank = p_rLine[0].getRank();
	const unsigned int oLength = oData.size();

	if ( inputIsInvalid( p_rLine ) || ((m_oWidth << 2) >= oLength) || ((m_oShift << 2) >= oLength) || (m_oShift >= m_oWidth) )
	{
		return false;
	}

	while (oRank[p_rStartPos] == eRankMin)
	{
		++p_rStartPos;
	}
	while (oRank[p_rEndPos] == eRankMin)
	{
		--p_rEndPos;
	}
	if ((int)(m_oWidth) >= (p_rEndPos - p_rStartPos) )
	{
		return false;
	}

	return true;
}

bool GradientTrend::computeGradientTrend( const VecDoublearray &p_rLine, const unsigned int p_oStartPos, const unsigned int p_oEndPos )
{
	std::vector<double> oData, oRank, oGrad;
	std::vector<int> oGradRank;

	oGrad.assign(p_rLine[0].getData().size(), 0.0); oGradRank.assign(p_rLine[0].getData().size(), 0); // gradient
	oData.assign(2*m_oWidth+1, 0.0); oRank.assign(2*m_oWidth+1, 0.0); // segment for smoothing window

	computeGradient( p_rLine, oGrad, oGradRank );

	unsigned int j=0;
	double oAvg=0.0, oAvgSum=0.0;           // result values for average calculation function
	double oOverallSum=0.0;                 // sum over values over complete line for mean substraction afterwards
	double oMarginSum=0.0, oMarginRank=0.0; // sum of margin pixel values and rank values

	// allow left and right connected segments of bad rank and along with that compute "real" start and end of line


	// treat margins: compute average over length of interval from border to current point
	unsigned int oOffset = p_oStartPos+m_oWidth; unsigned int oInset = p_oEndPos-m_oWidth;
	if ( (p_oStartPos+(m_oWidth << 1) >= oInset)  || (oOffset >= oInset) )
	{
		return false;  // laser line not long enough
	}

	// for the first m_oWidth line pixels we compute the ordinary average over the number of pixels up to the pixel in question
	j = p_oStartPos;

	while ( j < oOffset )
	{
		const double jinverse = 1.0/(j-p_oStartPos+1);

		oMarginSum += oGrad[j]; oMarginRank += oGradRank[j];
		oData[j-p_oStartPos] = oGrad[j]; oRank[j-p_oStartPos] = oGradRank[j];

		if (oGradRank[j] > eRankMin)
		{
			m_oTrend[0].getRank()[j] = oGradRank[j];
		} else
		{
			m_oTrend[0].getRank()[j] = eRankMin;
		}
		m_oTrend[0].getData()[j] = oMarginSum*jinverse; // compute avg.
		m_oTrend[0].getData()[j] *= ( math::sgn(m_oTrend[0].getData()[j])*m_oTrend[0].getData()[j] ); // square result but keep sign
		oOverallSum += m_oTrend[0].getData()[j];             // and add result to total average
		++j;
	}

	// left margin, remainder
	while ( j <= (p_oStartPos+(m_oWidth << 1)) )
	{
		if (oGradRank[j] > eRankMin) // good rank
		{
			// fill initial data and rank arrays
			oData[j-p_oStartPos] = oGrad[j]; oRank[j-p_oStartPos] = oGradRank[j];
		} else // bad rank
		{
			oData[j-p_oStartPos] = 0.0; oRank[j-p_oStartPos] = eRankMin;
		}
		++j;
	}

	// continue. oData and oRank are now filled.
	oOffset += m_oWidth;  // is now (oStartPos + 2*m_oWidth), which is not really an offset but is used rather often as an index
	unsigned int oIdx = m_oWidth;
	for (j=p_oStartPos+m_oWidth; j <= p_oEndPos-m_oWidth; ++j)
	{
		if (oGradRank[j] > eRankMin)
		{
			// only change one array entry using a circular index
			const unsigned int oChangeIdx = ( oIdx % (2 * m_oWidth + 1) );

			// compute average
			if (j > oOffset)
			{
				oData[oChangeIdx] = oGrad[j];
				math::arithmeticAvgVec(oAvg, oData, oAvgSum);
			}
			else // oData and oRank are already filled for m_oWidth <= j <= (2*m_oWidth), so just compute averages restricted to length j-oStartPos here
			{
				math::arithmeticAvgVec(oAvg, oData, oAvgSum, (oIdx + 1));
			}
			m_oTrend[0].getData()[j] = oAvg;//math::sgn(oAvg)*oAvg*oAvg;
			m_oTrend[0].getRank()[j] = oGradRank[j];
			oOverallSum += m_oTrend[0].getData()[j];
			++oIdx;
		} else
        {
	        m_oTrend[0].getData()[j] = 0.0;
            m_oTrend[0].getRank()[j] = eRankMin;
        }
	}

	// right margin
	const unsigned int oLoopEnd = std::min((int)(p_rLine[0].getData().size() - 1), (int)p_oEndPos);
	while (j < oLoopEnd)
	{
		const unsigned int oChangeIdx = (j % (2 * m_oWidth + 1));
		oData[oChangeIdx] = oGrad[j];
		math::arithmeticAvgVec(oAvg, oData, oAvgSum);
		m_oTrend[0].getData()[j] = math::sgn(oAvg)*oAvg*oAvg;
		m_oTrend[0].getRank()[j] = oGradRank[j];
		oOverallSum += m_oTrend[0].getData()[j];
		++j;
	}

	// compute mean and...
	oOverallSum /= p_rLine[0].getData().size();

	// ... substract to guarantee signature changes at turning points! This gives way better performance for images with strong overall slope!
	m_oTrend[0].getData()[0] -= oOverallSum;
	m_oMinG = m_oTrend[0].getData()[0]; m_oMaxG = m_oTrend[0].getData()[0];
	for (j = 1; j < m_oTrend[0].getData().size(); ++j)
	{
		auto &oVal(m_oTrend[0].getData()[j]);
		oVal -= oOverallSum;
		if (oVal < m_oMinG)
		{
			m_oMinG = oVal;
		}
		if (oVal >= m_oMaxG)
		{
			m_oMaxG = oVal;
		}
		processSignatureChange(j, m_oTrend[0].getData()[j], m_oTrend[0].getData()[j - 1]);
	}

	auto &oTrendData(m_oTrend[0].getData());
	rescale(oTrendData, -1, 1); // normalize Trend to [-1, 1] asymmetrically around 0

	return true;
}

/// Verbosity here references the value to display!
void GradientTrend::paint() {
	if (m_oVerbosity < eLow || m_oSpTrafo.isNull())
	{
		return;
	} // if

	const Trafo		&rTrafo(*m_oSpTrafo);
	OverlayCanvas	&rCanvas(canvas<OverlayCanvas>(m_oCounter));
	OverlayLayer	&rLayerContour(rCanvas.getLayerContour());

	const double oScale = 12;

	for (unsigned int i = 0; i < m_oTrend[0].getData().size(); ++i)
	{
		if (m_oTrend[0].getRank()[i] > 0)
		{
			if (m_oTrend[0].getData()[i] > 0)
			{
				rLayerContour.add(new OverlayPoint(rTrafo(Point(i, 14 + (int)(oScale*m_oTrend[0].getData()[i]))), Color::Green()));
			}
			else if (m_oTrend[0].getData()[i] < 0)
			{
				rLayerContour.add(new OverlayPoint(rTrafo(Point(i, 14 + (int)(oScale*m_oTrend[0].getData()[i]))), Color::Red()));
			}
			else
			{
				rLayerContour.add(new OverlayPoint(rTrafo(Point(i, 14 + (int)(oScale*m_oTrend[0].getData()[i]))), Color::Yellow()));
			}
		}
	}
	return;
} // paint

// assymetric rescaling: scale negative and positive parts independently to keep the zero neighbourhood around zero... Otherwise one might get a bias
void GradientTrend::rescale(std::vector<double> &p_rGrad, const double p_oMin, const double p_oMax)
{
	// rescale gradient trend to interval [p_oMin, p_oMax]
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
	if ((std::abs(m_oMinG) < math::eps) || (std::abs(m_oMaxG) < math::eps) )
	{
		return;
	}
	double oRelMin = std::abs(m_oMinG)/(std::abs(m_oMinG)+std::abs(m_oMaxG));
	double oRelMax = std::abs(m_oMaxG)/(std::abs(m_oMinG)+std::abs(m_oMaxG));

	for (double &oVal : p_rGrad) // scale to new interval, bias free
	{
		if (oVal <= 0)
		{
			oVal = (oVal*p_oMin)*oRelMin/m_oMinG;
		}
		else
		{
			oVal = (oVal*p_oMax)*oRelMax/m_oMaxG;
		}
	}
}


void GradientTrend::proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeInLine != nullptr); // to be asserted by graph editor

	const GeoVecDoublearray &rLine( m_pPipeInLine->read(m_oCounter) );
	int oStartPos = 0; int oEndPos = rLine.ref()[0].getData().size() - 1; // for skipping margins with bad rank

	if (isValidLine(rLine.ref(), oStartPos, oEndPos) )
	{
		// Get Line and trafo
		m_oSpTrafo	= rLine.context().trafo();
		const unsigned int oLength = rLine.ref()[0].getData().size();

		m_oTrend[0].assign(oLength, 0.0, 0);
		m_oSignatureChange.clear();
		computeGradientTrend( rLine.ref(), oStartPos, oEndPos );

		m_oSigChangeVector[0].getData().resize(m_oSignatureChange.size() * 2, 0.0);
		m_oSigChangeVector[0].getRank().resize(m_oSignatureChange.size() * 2, 0);
		for (unsigned int i=0; i < m_oSignatureChange.size(); ++i)
		{
			(m_oSigChangeVector[0].getData())[2*i] = m_oSignatureChange[i].oPos;
			(m_oSigChangeVector[0].getData())[(2*i)+1] = m_oSignatureChange[i].oType;
			(m_oSigChangeVector[0].getRank())[2*i] = eRankMax;
			(m_oSigChangeVector[0].getRank())[(2*i)+1] = eRankMax;
		}
		const GeoVecDoublearray kTrend(rLine.context(), m_oTrend, AnalysisOK, 1.0);
		const GeoVecDoublearray sigs(rLine.context(), m_oSigChangeVector, AnalysisOK, 1.0);
		preSignalAction();
		m_oPipeOutTrend.signal( kTrend );
		m_oPipeOutSigs.signal(sigs);
	} else
	{
		const GeoVecDoublearray kTrend(rLine.context(), m_oTrend, AnalysisOK, interface::NotPresent);
		const GeoVecDoublearray sigs(rLine.context(), m_oSigChangeVector, AnalysisOK, interface::NotPresent);
		preSignalAction();
		m_oPipeOutSigs.signal( sigs );
		m_oPipeOutTrend.signal( kTrend );
	}
}

} // namespace precitec
} // namespace filter
