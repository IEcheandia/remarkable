/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		  Andreas Beschorner (BA)
 *  @date		    2012
 *  @brief	    Computes local Dispersions from gradient and averaged/ smoothed gradient
 */

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include "module/moduleLogger.h"

#include "filter/algoArray.h"	///< algorithmic interface for class TArray

// local includes
#include "dispersion.h"
#include "2D/avgAndRegression.h"

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string Dispersion::m_oFilterName = std::string("Dispersion");
const std::string Dispersion::PIPENAME_AboveAvgLeft  = std::string("AboveAvgLeft");
const std::string Dispersion::PIPENAME_BelowAvgLeft  = std::string("BelowAvgLeft");
const std::string Dispersion::PIPENAME_AboveAvgRight  = std::string("AboveAvgRight");
const std::string Dispersion::PIPENAME_BelowAvgRight  = std::string("BelowAvgRight");
const std::string Dispersion::PIPENAME_Trend  = std::string("Trend");

Dispersion::Dispersion() : TransformFilter( Dispersion::m_oFilterName, Poco::UUID{"2028B7DB-5CBD-4D9A-8C3E-0797B1CF004F"} ),
	m_pPipeInLine(NULL), m_oPipeOutUpperLeft( this, Dispersion::PIPENAME_AboveAvgLeft), m_oPipeOutLowerLeft( this, Dispersion::PIPENAME_BelowAvgLeft),
	m_oPipeOutUpperRight( this, Dispersion::PIPENAME_AboveAvgRight), m_oPipeOutLowerRight( this, Dispersion::PIPENAME_BelowAvgRight),
	m_oPipeOutTrend( this, Dispersion::PIPENAME_Trend),
	m_oWidth(10), m_oShift(1)
{
	m_oKLine.resize(1);
	m_oUpperLeft.resize(1);
	m_oLowerLeft.resize(1);
	m_oUpperRight.resize(1);
	m_oLowerRight.resize(1);
	m_oTrend.resize(1);
	parameters_.add("Width", Parameter::TYPE_UInt32, m_oWidth);
	parameters_.add("Shift", Parameter::TYPE_UInt32, m_oShift);

    setInPipeConnectors({{Poco::UUID("F88FD50B-4D28-444D-BB69-23275119B3B4"), m_pPipeInLine, "Line", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("201AC776-F8A4-4B05-BC5A-9A39DFD322C5"), &m_oPipeOutUpperLeft, PIPENAME_AboveAvgLeft, 0, ""},
    {Poco::UUID("02AF8AFC-9A3F-4E10-9380-BB96FBEA2B45"), &m_oPipeOutLowerLeft, PIPENAME_BelowAvgLeft, 0, ""},
    {Poco::UUID("28F76E7D-0D3F-4431-9832-61518A392F07"), &m_oPipeOutUpperRight, PIPENAME_AboveAvgRight, 0, ""},
    {Poco::UUID("2B1195F1-20BF-4A2C-816A-61773EC9264D"), &m_oPipeOutLowerRight, PIPENAME_BelowAvgRight, 0, ""},
    {Poco::UUID("CAE41337-C639-4BFF-955B-D9BA96D5458D"), &m_oPipeOutTrend, PIPENAME_Trend, 0, ""}});
    setVariantID(Poco::UUID("06275726-682D-433C-8216-D11D35CB6B16"));
}

Dispersion::~Dispersion()
{
}

bool Dispersion::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeInLine = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray >* >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe

void Dispersion::setParameter()
{
	TransformFilter::setParameter();
	m_oWidth = parameters_.getParameter("Width").convert<unsigned int>();
	m_oShift = parameters_.getParameter("Shift").convert<unsigned int>();
	poco_assert_dbg((m_oWidth >= 3) && (m_oShift > 0));
}

// ----------------------------------------------------------------------------

// compute real valued gradient
void Dispersion::computeGradient( const VecDoublearray &p_rLine, std::vector<double> &p_rGrad, std::vector<double> &p_rGradRank )
{
	unsigned int i;
	p_rGrad[0] = 0.0;
	p_rGradRank[0] = 1.0*p_rLine[0].getRank()[0];
	for (i=1; i < p_rLine[0].getData().size()-1; ++i)
	{
		p_rGrad[i] = 0.5*(p_rLine[0].getData()[i-1] - p_rLine[0].getData()[i+1]); // direction of gradient inverse, as pixel values increase from top to bottom!
		p_rGradRank[i] = std::min(p_rLine[0].getRank()[i-1], p_rLine[0].getRank()[i+1]);
	}
	p_rGrad[i] = 0.0;
	p_rGradRank[i] = 1.0*p_rLine[0].getRank()[i];
}

void Dispersion::computeDispersion( const VecDoublearray &p_rLine )
{
	std::vector<double> oData, oRank, oGrad, oGradRank;

	oGrad.resize(p_rLine[0].getData().size());
	oGradRank.resize(p_rLine[0].getData().size());
	oData.resize(2*m_oWidth+1);
	oRank.resize(2*m_oWidth+1);

	computeGradient( p_rLine, oGrad, oGradRank );

	unsigned int i=0;
	double oAvg;

	unsigned int oCntUpperLeft, oCntUpperRight, oCntLowerLeft, oCntLowerRight;
	unsigned int oIdx;

	double oTmpVal, oAvgSum;
	double oRankSum;
	for (unsigned j=0; j < m_oWidth; ++j)
	{
		m_oTrend[0].getData()[j] = 0.0;
		m_oTrend[0].getRank()[j] = 0;
		m_oTrend[0].getData()[p_rLine[0].getData().size() - 1 - j] = 0.0;
		m_oTrend[0].getRank()[p_rLine[0].getData().size() - 1 - j] = 0;
	}

	for (unsigned j=m_oWidth; j < p_rLine[0].getData().size()-m_oWidth; ++j)
	//for (unsigned j=0; j < p_rLine[0].getData().size()-2*m_oWidth-1; ++j)
	{
		// todo: optimize when used. nur noch ersten wert abziehen, letzten dazuaddieren, avg neu berechnen.
		// copy linesegment and compute gradients
		oIdx = 0; // reset position and rank for segment
		oRankSum = 0;
		for (i=j-m_oWidth; i <= j+m_oWidth; ++i)
		//for (i=j; i <= j+(2*m_oWidth); ++i)
		{
			oData[oIdx] = oGrad[i];
			oRankSum += oGradRank[i];
			oRank[oIdx] = oGradRank[i] / (1.0*eRankMax); // normalize
			++oIdx;
		}
		//oData[0]=0; oData[m_oWidth]=0;

		// compute average
		math::arithmeticAvgVec( oAvg, oData, oAvgSum );

		m_oTrend[0].getData()[j] = 4*oAvg;
		m_oTrend[0].getRank()[j] = (int)(oRankSum / (2*m_oWidth+1));

		//  end compute gradient and its trend
		// ------------------------------------

		// reset dispersion values and rank counters
		oCntUpperLeft = 0; oCntUpperRight = 0;
		oCntLowerLeft = 0; oCntLowerRight = 0;

		i=0;
		// wenn funktioniert, optimieren!
		// compute Dispersions
		while (i < m_oWidth)
		{
			// left half
			oTmpVal = oData[i];

			if (oTmpVal > oAvg)
			{
				m_oUpperLeft[0].getData()[j] += (oTmpVal-oAvg);
				m_oUpperLeft[0].getRank()[j] += (int)(oRank[i]);
				++oCntUpperLeft;
			} else if (oTmpVal < oAvg)
			{
				m_oLowerLeft[0].getData()[j] += (oAvg -oTmpVal);
				m_oLowerLeft[0].getRank()[j] += (int)(oRank[i]);
				++oCntLowerLeft;
			}

			// right half
			oTmpVal = oData[m_oWidth+i];

			if (oTmpVal > oAvg)
			{
				m_oUpperRight[0].getData()[j] += (oTmpVal-oAvg);
				m_oUpperRight[0].getRank()[j] += (int)(oRank[m_oWidth+i]);
				++oCntUpperRight;
			} else if (oTmpVal < oAvg)
			{
				m_oLowerRight[0].getData()[j] += (oAvg -oTmpVal);
				m_oLowerRight[0].getRank()[j] += (int)(oRank[m_oWidth+i]);
				++oCntLowerRight;
			}

			++i;
		}

		if (oCntUpperLeft > 0)
		{
			m_oUpperLeft[0].getRank()[j] /= oCntUpperLeft;
		} else
		{
			m_oUpperLeft[0].getRank()[j] = 255;
		}
		if (oCntLowerLeft > 0)
		{
			m_oLowerLeft[0].getRank()[j] /= oCntLowerLeft;
		} else
		{
			m_oLowerLeft[0].getRank()[j] = 255;
		}
		if (oCntUpperRight > 0)
		{
			m_oUpperRight[0].getRank()[j] /= oCntUpperRight;
		} else
		{
			m_oUpperRight[0].getRank()[j] = 255;
		}
		if (oCntLowerRight > 0)
		{
			m_oLowerRight[0].getRank()[j] /= oCntLowerRight;
		} else
		{
			m_oLowerRight[0].getRank()[j] = 255;
		}
	}
}

/// Verbosity here references the value to display!
void Dispersion::paint() {
	if( m_oVerbosity < eLow || m_oSpTrafo.isNull() )
	{
		return;
	} // if

	const Trafo		&rTrafo				( *m_oSpTrafo );
	OverlayCanvas	&rCanvas			( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerContour		( rCanvas.getLayerContour());

	for (unsigned int i = 0; i < m_oUpperLeft[0].getData().size(); ++i)
	{
		/*
		rLayer.add( new OverlayPoint(rTrafo(Point(i, 180+(int)(m_oUpperLeft[0].getData()[i]+0.5))), Color::Blue() ) );
		rLayer.add( new OverlayPoint(rTrafo(Point(i, 192+(int)(m_oLowerLeft[0].getData()[i]+0.5))), Color::Red() ) );
		rLayer.add( new OverlayPoint(rTrafo(Point(i, 204+(int)(m_oUpperRight[0].getData()[i]+0.5))), Color::Blue() ) );
		rLayer.add( new OverlayPoint(rTrafo(Point(i, 216+(int)(m_oLowerRight[0].getData()[i]+0.5))), Color::Red() ) );
		*/
	//	if ((m_oTrend[0].getData()[i]+0.5) > 0)
		if (m_oTrend[0].getRank()[i] > 0)
		{
			if ((m_oTrend[0].getData()[i]) > 0)
			{
				rLayerContour.add( new OverlayPoint(rTrafo(Point(i, 140+2*(int)(m_oTrend[0].getData()[i]+0.5))), Color::Green() ) );
			} else if ((m_oTrend[0].getData()[i]) < 0)
			{
				rLayerContour.add( new OverlayPoint(rTrafo(Point(i, 140+2*(int)(m_oTrend[0].getData()[i]+0.5))), Color::Red() ) );
			} else
			{
				rLayerContour.add( new OverlayPoint(rTrafo(Point(i, 140+2*(int)(m_oTrend[0].getData()[i]+0.5))), Color::Orange() ) );
			}
		}
	}
	return;
} // paint


void Dispersion::proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeInLine != nullptr); // to be asserted by graph editor

	// Get Line and trafo
	const GeoVecDoublearray &rLine( m_pPipeInLine->read(m_oCounter) );
	m_oSpTrafo	= rLine.context().trafo();
	unsigned int oLength = rLine.ref()[0].getData().size();

	m_oUpperLeft[0].assign(oLength, 0.0, 0);
	m_oLowerLeft[0].assign(oLength, 0.0, 0);
	m_oUpperRight[0].assign(oLength, 0.0, 0);
	m_oLowerRight[0].assign(oLength, 0.0, 0);
	m_oTrend[0].assign(oLength, 0.0, 0);

	if ( inputIsInvalid( rLine ) || ((m_oWidth << 2) >= oLength) || ((m_oShift << 2) >= oLength) || (m_oShift >= m_oWidth) )
	{
		if ( (m_oWidth << 2) >= oLength )
		{
			wmLogTr(eError, "QnxMsg.Filter.dispWidth", "Filter kCurvation: Parameter shiftK too large w.r.t. length of line");
		}
		if ( (m_oShift << 2) >= oLength)
		{
			wmLogTr(eError, "QnxMsg.Filter.dispShift", "Filter kCurvation: Parameter shiftK too large w.r.t. length of line");
		}
		if (m_oShift >= m_oWidth)
		{
			wmLogTr(eError, "QnxMsg.Filter.dispWidthShift", "Filter kCurvation: Parameter shift must be smaller than parameter width");
		}
		const GeoVecDoublearray kUpperLeft(rLine.context(), m_oUpperLeft, rLine.analysisResult(), interface::NotPresent);
		const GeoVecDoublearray kLowerLeft(rLine.context(), m_oLowerLeft, rLine.analysisResult(), interface::NotPresent);
		const GeoVecDoublearray kUpperRight(rLine.context(), m_oUpperRight, rLine.analysisResult(), interface::NotPresent);
		const GeoVecDoublearray kLowerRight(rLine.context(), m_oLowerRight, rLine.analysisResult(), interface::NotPresent);
		const GeoVecDoublearray kTrend(rLine.context(), m_oTrend, rLine.analysisResult(), interface::NotPresent);
		preSignalAction();
		m_oPipeOutUpperLeft.signal( kUpperLeft ); m_oPipeOutLowerLeft.signal( kLowerLeft );
		m_oPipeOutUpperRight.signal( kUpperRight ); m_oPipeOutLowerRight.signal( kLowerRight );
		m_oPipeOutTrend.signal( kTrend );
	} else
	{
		m_oSpTrafo	= rLine.context().trafo();
		computeDispersion( rLine.ref() );
		const auto oAnalysisResult	= rLine.analysisResult() == AnalysisOK ? AnalysisOK : rLine.analysisResult(); // replace 2nd AnalysisOK by your result type
		const GeoVecDoublearray kUpperLeft(rLine.context(), m_oUpperLeft, oAnalysisResult, eRankMax);
		const GeoVecDoublearray kLowerLeft(rLine.context(), m_oLowerLeft, oAnalysisResult, eRankMax);
		const GeoVecDoublearray kUpperRight(rLine.context(), m_oUpperRight, oAnalysisResult, eRankMax);
		const GeoVecDoublearray kLowerRight(rLine.context(), m_oLowerRight, oAnalysisResult, eRankMax);
		const GeoVecDoublearray kTrend(rLine.context(), m_oTrend, oAnalysisResult, eRankMax);
		preSignalAction();
		m_oPipeOutUpperLeft.signal( kUpperLeft ); m_oPipeOutLowerLeft.signal( kLowerLeft );
		m_oPipeOutUpperRight.signal( kUpperRight ); m_oPipeOutLowerRight.signal( kLowerRight );
		m_oPipeOutTrend.signal( kTrend );
	}
}


} // namespace precitec
} // namespace filter
