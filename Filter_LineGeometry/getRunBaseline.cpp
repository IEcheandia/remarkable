/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		  Andreas Beschorner (BA)
 *  @date		    2012
 *  @brief	    Gets baseline.
 */

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include "module/moduleLogger.h"

#include "filter/algoArray.h"	///< algorithmic interface for class TArray

// local includes
#include "getRunBaseline.h"
#include "mathCommon.h"

#include <fliplib/TypeToDataTypeImpl.h>
#include <math/2D/LineEquation.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string GetRunBaseline::m_oFilterName 	      = std::string("GetRunBaseline");
const std::string GetRunBaseline::PIPENAME_SLOPE	      = std::string("Slope");
const std::string GetRunBaseline::PIPENAME_INTERCEPT	  = std::string("Intercept");
const std::string GetRunBaseline::PIPENAME_XSTART	      = std::string("StartPosX");
const std::string GetRunBaseline::PIPENAME_LINESEGMENT  = std::string("Linesegment");
const std::string GetRunBaseline::PIPENAME_LINE_OUT = std::string("LineEquation");

GetRunBaseline::GetRunBaseline() : TransformFilter( GetRunBaseline::m_oFilterName, Poco::UUID{"01B076B9-33F1-4EEA-85C1-981C7CA11C2E"} ),
	m_pPipeInXLeft(NULL), m_pPipeInYLeft(NULL),
	m_pPipeInXRight(NULL), m_pPipeInYRight(NULL),
	m_oPipeOutSlope( this, GetRunBaseline::PIPENAME_SLOPE ),
	m_oPipeOutIntercept( this, GetRunBaseline::PIPENAME_INTERCEPT ),
	m_oPipeOutXStart ( this, GetRunBaseline::PIPENAME_XSTART ),
	m_oPipeOutLinesegment( this, GetRunBaseline::PIPENAME_LINESEGMENT),
	m_oPipeOutLineEquation(this, GetRunBaseline::PIPENAME_LINE_OUT),
	m_oSlope(0.0), m_oIntercept(0.0), m_oXStart(-1.0), m_oPaint(true)
{
	m_oLinesegment.resize(1);

    setInPipeConnectors({{Poco::UUID("2880CE38-32CD-4E47-987D-4AE017C69848"), m_pPipeInXLeft, "PositionX", 1, "xleft"},
    {Poco::UUID("386DD58E-FF70-4FE6-930F-2DC270805C00"), m_pPipeInYLeft, "PositionY", 1, "yleft"},
    {Poco::UUID("2E783E4D-1C24-4925-9135-9A8903BE28B5"), m_pPipeInXRight, "PositionX", 1, "xright"},
    {Poco::UUID("6264F103-5667-48B2-85AA-3A69C31FFB7B"), m_pPipeInYRight, "PositionY", 1, "yright"}});
    setOutPipeConnectors({{Poco::UUID("4658537A-B413-4645-90EF-4F82BB88A581"), &m_oPipeOutSlope, PIPENAME_SLOPE, 0, ""},
    {Poco::UUID("57DD39BE-3994-40C3-A422-EBC017D4C790"), &m_oPipeOutIntercept, PIPENAME_INTERCEPT, 0, ""},
    {Poco::UUID("87D42031-04E1-4CB7-8C8F-8B3B6C2B8B75"), &m_oPipeOutXStart, PIPENAME_XSTART, 0, ""},
    {Poco::UUID("14C3EF09-FA20-41FA-A7E9-C1FDA10ADD08"), &m_oPipeOutLinesegment, PIPENAME_LINESEGMENT, 0, ""},
    {Poco::UUID("79FBD2BF-EC1D-4C6C-AD61-0500C7E333B1"), &m_oPipeOutLineEquation, PIPENAME_LINE_OUT, 0, ""}
    });
    setVariantID(Poco::UUID("7993567C-79D9-495E-AE01-7B4450683413"));
}

void GetRunBaseline::setParameter()
{
	TransformFilter::setParameter();
}

void GetRunBaseline::paint() {
	if( !m_oPaint || m_oVerbosity < eLow || m_oSpTrafo.isNull() || 	(m_oLinesegment[0].getData().size() <= 1) )
	{
		return;
	}

	const Trafo		&rTrafo				( *m_oSpTrafo );
	OverlayCanvas	&rCanvas			( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerPosition		( rCanvas.getLayerPosition());
	OverlayLayer	&rLayerContour		( rCanvas.getLayerContour());

	auto segment = m_oLinesegment[0];
	unsigned int i=0;
	for (; i < segment.getData().size(); ++i)
	{
		geo2d::Point p(i+(int)m_oXStart, int( segment.getData()[i] ));
		rLayerContour.add( new	OverlayPoint(rTrafo(p), Color::Yellow() )); // paint first position
	}
	rLayerPosition.add( new OverlayCross(rTrafo( Point((int)m_oXStart, int( segment.getData()[0] )) ), Color::Red()) );
	rLayerPosition.add( new OverlayCross(rTrafo( Point((int)((i-1)+m_oXStart), int( segment.getData()[i-1] )) ), Color::Red()) );
} // paint

bool GetRunBaseline::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if (p_rPipe.tag() == "xleft")
	{
		m_pPipeInXLeft  = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray >* >(&p_rPipe);
	} else if (p_rPipe.tag() == "yleft")
	{
		m_pPipeInYLeft = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray >* >(&p_rPipe);
	} else if (p_rPipe.tag() == "xright")
	{
		m_pPipeInXRight  = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray >* >(&p_rPipe);
	} else if (p_rPipe.tag() == "yright")
	{
		m_pPipeInYRight  = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray >* >(&p_rPipe);
	}

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe


// -----------------------------------------------------------


bool GetRunBaseline::computeLinesegment(double p_oXLeft, double p_oYLeft, double p_oXRight, double p_oYRight)
{
	if ( (p_oXRight - p_oXLeft) <  math::eps)
	{
		return false;
	}
	m_oSlope = (p_oYRight - p_oYLeft)/(p_oXRight - p_oXLeft);
    m_oIntercept = (p_oYLeft - m_oSlope * p_oXLeft);
    assert(math::isClose(m_oIntercept, (p_oYRight - m_oSlope * p_oXRight)));
	m_oLinesegment[0].getData().resize((int)(std::abs(p_oXRight-p_oXLeft+0.5))+1);
	m_oLinesegment[0].getRank().resize((int)(std::abs(p_oXRight-p_oXLeft+0.5))+1);

	int oIdx = 0; m_oXStart = p_oXLeft;
	for (int x=(int)p_oXLeft; x <= (int)p_oXRight; ++x)
	{
		((m_oLinesegment[0]).getData())[oIdx] = (int)((m_oSlope*x+m_oIntercept)+0.5);
		((m_oLinesegment[0]).getRank())[oIdx] = eRankMax;
		++oIdx;
	}
	return true;
}

void GetRunBaseline::signalSend(const interface::ImageContext &p_rContext, const bool p_oIO)
{
	if (p_oIO)
	{
        const auto oSlope         =   m_oSlope;  // do not access members after preSignalAction()
        const auto oIntercept     =   m_oIntercept;
        const auto oXStart        =   m_oXStart;
        const auto oLinesegment   =   m_oLinesegment;
        const auto lineEquation = math::LineEquation(oSlope, oIntercept);
        auto [a,b,c] = lineEquation.getCoefficients(true);
        auto xCenter = oXStart + 0.5*oLinesegment.size() ;
        const auto lineModel = geo2d::LineModel (xCenter, lineEquation.getY(xCenter), a, b, c);

		preSignalAction();
		m_oPipeOutSlope.signal( GeoDoublearray(p_rContext, Doublearray(1, oSlope, eRankMax), AnalysisOK, 1.0) );
		m_oPipeOutIntercept.signal( GeoDoublearray(p_rContext, Doublearray(1, oIntercept, eRankMax), AnalysisOK, 1.0) );
		m_oPipeOutXStart.signal( GeoDoublearray(p_rContext, Doublearray(1, oXStart, eRankMax), AnalysisOK, 1.0) );
		m_oPipeOutLinesegment.signal( interface::GeoVecDoublearray( p_rContext, oLinesegment, AnalysisOK, 1.0) );
        m_oPipeOutLineEquation.signal(GeoLineModelarray(p_rContext, geo2d::LineModelarray{1, lineModel, filter::eRankMax}, AnalysisOK, 1.0));
	} else
	{
        m_oLinesegment[0].getData().clear(); m_oLinesegment[0].getData().clear();  // do not access members after preSignalAction()
        const auto oLinesegment   =   m_oLinesegment;

		preSignalAction();
		m_oPipeOutSlope.signal( GeoDoublearray(p_rContext, Doublearray(1, 0.0, 0), AnalysisOK, 0) );
		m_oPipeOutIntercept.signal( GeoDoublearray(p_rContext, Doublearray(1, 0.0, 0), AnalysisOK, 0) );
		m_oPipeOutXStart.signal( GeoDoublearray(p_rContext, Doublearray(1, -1.0, 0), AnalysisOK, 0) );
		m_oPipeOutLinesegment.signal( interface::GeoVecDoublearray( p_rContext, oLinesegment, AnalysisOK, 0) );
        m_oPipeOutLineEquation.signal(GeoLineModelarray(p_rContext, geo2d::LineModelarray{1, {}, 0}, AnalysisOK, 0));

	}
}

void GetRunBaseline::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeInXLeft != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInYLeft != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInXRight != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInYRight != nullptr); // to be asserted by graph editor

	// Get Coords
	const GeoDoublearray &rXLeft( m_pPipeInXLeft->read(m_oCounter) );
	const GeoDoublearray &rYLeft( m_pPipeInYLeft->read(m_oCounter) );
	const GeoDoublearray &rXRight( m_pPipeInXRight->read(m_oCounter) );
	const GeoDoublearray &rYRight( m_pPipeInYRight->read(m_oCounter) );

	m_oPaint = true;

	const ImageContext &rContext(rXLeft.context());
    bool resultParameterForSignalSend = false;

	if ( (rXLeft.rank() == 0) || (rYLeft.rank() == 0) || (rXRight.rank() == 0) || (rYRight.rank() == 0) )
	{
		m_oPaint = false;
	} else
	{
		double oXLeft=std::get<eData>(rXLeft.ref()[0]); double oYLeft = std::get<eData>(rYLeft.ref()[0]);
		double oXRight=std::get<eData>(rXRight.ref()[0]); double oYRight = std::get<eData>(rYRight.ref()[0]);

		if (oXLeft >= oXRight) // if necessary, swap left and right points
		{
			if (oXLeft == oXRight)
			{
				m_oPaint = false;
			}
			double oTmp = oXLeft; oXLeft = oXRight; oXRight = oTmp;
			oTmp = oYLeft; oYLeft = oYRight; oYRight = oTmp;
		}

		m_oSpTrafo	= rXLeft.context().trafo();
		if ( computeLinesegment(oXLeft, oYLeft, oXRight, oYRight) )
		{
            resultParameterForSignalSend = true;
		} else
		{
			m_oPaint = false;
		}
	}
    signalSend(rContext, resultParameterForSignalSend);

}


} // namespace precitec
} // namespace filter
