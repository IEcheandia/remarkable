// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>

#include "filter/algoArray.h"	///< algorithmic interface for class TArray
#include <filter/structures.h>

// local includes
#include "getRunData.h"

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string GetRunData::m_oFilterName    = std::string("GetRunData");
const std::string GetRunData::PIPENAME_HEIGHT  = std::string("Height");
const std::string GetRunData::PIPENAME_LENGTH  = std::string("Length");
const std::string GetRunData::PIPENAME_SURF    = std::string("Surface");

GetRunData::GetRunData() : TransformFilter( GetRunData::m_oFilterName, Poco::UUID{"C1B69526-575E-45F0-B6B6-282C6745F4D6"} ),
	m_pPipeInLaserline( nullptr ), m_pPipeInOrientation( nullptr ),
	m_pPipeInXLeft( nullptr ), m_pPipeInXRight( nullptr ),
	m_pPipeInSlope( nullptr ), m_pPipeInIntercept( nullptr ),
	m_oPipeOutHeight( this, GetRunData::PIPENAME_HEIGHT ), m_oPipeOutLength( this, GetRunData::PIPENAME_LENGTH ), m_oPipeOutSurface( this, GetRunData::PIPENAME_SURF ),
	m_oOrientation(eOrientationInvalid), m_oXLeft(-1.0), m_oXRight(-1.0), m_oYLeft(-1.0), m_oYRight(-1.0),
	m_oSlope(0.0), m_oIntercept(0.0),
	m_oXPos(-1.0), m_oYPos(-1.0), m_oHeight(-1.0), m_oLength(0.0), m_oSurface(0.0)
{
    setInPipeConnectors({{Poco::UUID("29FD8C18-2B15-4F83-A257-F8C161854975"), m_pPipeInLaserline, "Line", 1, "line"},
    {Poco::UUID("5466CBE1-071C-46AA-8E19-9ACD99C071B0"), m_pPipeInXLeft, "MarkerLeftX", 1, "xleft"},
    {Poco::UUID("B8A59E2E-5EA5-4063-8FCF-8544DC0781A0"), m_pPipeInXRight, "MarkerRightX", 1, "xright"},
    {Poco::UUID("657629B4-B258-4447-A7A1-0413481C1EDC"), m_pPipeInSlope, "Slope", 1, "slope"},
    {Poco::UUID("3E658D8B-313F-44DA-B35F-31BF3E33123E"), m_pPipeInIntercept, "Intercept", 1, "intercept"},
    {Poco::UUID("E5C72319-92E3-489B-AEF3-03F672C57FDE"), m_pPipeInOrientation, "Orientation", 1, "orientation"}});
    setOutPipeConnectors({{Poco::UUID("F8AE74C4-A0D0-4576-901E-58033D3B10F9"), &m_oPipeOutHeight, PIPENAME_HEIGHT, 0, ""},
    {Poco::UUID("68F31FF4-3088-4958-9FFE-B5366E15B9ED"), &m_oPipeOutLength, PIPENAME_LENGTH, 0, ""},
    {Poco::UUID("9902E5E8-AD3B-4036-9331-29CC8014A1C1"), &m_oPipeOutSurface, PIPENAME_SURF, 0, ""}});
    setVariantID(Poco::UUID("6724DB64-04E2-449C-BCD7-C5D6634DCF7F"));
}

void GetRunData::setParameter()
{
	TransformFilter::setParameter();
}

void GetRunData::paint() {
	if(m_oVerbosity < eLow || m_oSpTrafo.isNull() )
	{
		return;
	}
	if ( (m_oXPos < 0) || (m_oYPos < 0) || (m_oXProj < 0) || (m_oYProj < 0) )
	{
		return;
	}
	if ( (m_oHeight <= 0.0) || (m_oSurface <= 0.0) || (m_oLength <= 0.0) )
	{
		return;
	}

	const Trafo		&rTrafo				( *m_oSpTrafo );
	OverlayCanvas	&rCanvas			( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerPosition		( rCanvas.getLayerPosition());
	OverlayLayer	&rLayerLine			( rCanvas.getLayerLine());

	geo2d::Point pEx((int)m_oXPos, (int)m_oYPos);
	geo2d::Point pProj((int)m_oXProj, (int)m_oYProj);

	rLayerPosition.add( new OverlayCross(rTrafo( pEx ), Color::Green()) );
	rLayerPosition.add( new OverlayCross(rTrafo( pProj ), Color::Yellow()) );
	rLayerLine.add( new OverlayLine(rTrafo( pEx ), rTrafo(pProj), Color::Red()) );
} // paint

bool GetRunData::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if (p_rPipe.tag() == "xleft")
	{
		m_pPipeInXLeft  = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray >* >(&p_rPipe);
	} else if (p_rPipe.tag() == "xright")
	{
		m_pPipeInXRight  = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray >* >(&p_rPipe);
	} else if (p_rPipe.tag() == "slope")
	{
		m_pPipeInSlope  = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray >* >(&p_rPipe);
	} else if (p_rPipe.tag() == "intercept")
	{
		m_pPipeInIntercept  = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray >* >(&p_rPipe);
	} else if (p_rPipe.tag() == "orientation")
	{
		m_pPipeInOrientation = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray >* >(&p_rPipe);
	} else if ( (p_rPipe.tag() == "line") || (p_rPipe.tag() == "") )
	{
		m_pPipeInLaserline = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray >* >(&p_rPipe);
	}
	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe


// -----------------------------------------------------------


bool GetRunData::findExtremum(const geo2d::VecDoublearray &p_rLine, const RunOrientation p_oOrientation)
{
	double oHeight = 0.0;
	double oOldHeight = std::numeric_limits<double>::lowest();

	auto oData = p_rLine[0].getData(); m_oXPos = -1;

	// project onto line segment connecting markers XLeft and XRight
	double yDir = m_oYRight - m_oYLeft;
	double xDir = m_oXRight - m_oXLeft;
	double oDenom = (xDir*xDir)+(yDir*yDir); // fix denom, inner product of directional vector with itself
	double oNom = 0.0, oXProj, oYProj;

	int oIdx = -1;
	unsigned int oXPos=0;
	for (int i = (int)m_oXLeft; i <= (int)m_oXRight; ++i) // traverse laserline points and project onto line segment
	{
		oXPos = i; m_oYPos = oData[i]; // get laserline point
		oNom = (oXPos - 0)*xDir + (m_oYPos - m_oIntercept)*yDir; // nominator of equation
		oXProj = (0 + (oNom/oDenom)*xDir); // x coord of projection; should equal oXPos if marker line segment is purely horizontal (slope 0.0)
		oYProj = (m_oIntercept + (oNom/oDenom)*yDir); // y coord of projection onto line segment
		oHeight = sqrt( (oXPos - oXProj)*(oXPos - oXProj) + (m_oYPos - oYProj)*(m_oYPos - oYProj) ); // compute height of projection
		m_oSurface += oHeight;
		if (oHeight > oOldHeight)
		{
			oOldHeight = oHeight;
			oIdx = oXPos;
			m_oXProj = oXProj; m_oYProj = oYProj; m_oHeight = oHeight;
		}
	}
	m_oLength = sqrt(oDenom); // get length of line segment (euclidean distance!!!)

	if (oIdx > -1)
	{
		m_oXPos = oIdx; m_oYPos = oData[oIdx];
		return true;
	} else
	{
		m_oXPos = -1.0; m_oYPos = 0.0; m_oSurface = 0.0; m_oHeight = -1.0; // error
		return false;
	}
}

void GetRunData::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeInLaserline != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInOrientation != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInXLeft != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInXRight != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInSlope != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInIntercept != nullptr); // to be asserted by graph editor

	// Get Coords
	const GeoVecDoublearray &rLine( m_pPipeInLaserline->read(m_oCounter) );
	const GeoDoublearray &rOrientation( m_pPipeInOrientation->read(m_oCounter) );
	const GeoDoublearray &rXLeft( m_pPipeInXLeft->read(m_oCounter) );
	const GeoDoublearray &rXRight( m_pPipeInXRight->read(m_oCounter) );
	const GeoDoublearray &rSlope( m_pPipeInSlope->read(m_oCounter) );
	const GeoDoublearray &rIntercept( m_pPipeInIntercept->read(m_oCounter) );

	const ImageContext &rContext(rXLeft.context());

	if ( (rXLeft.rank() == 0) || (rXRight.rank() == 0) || (rLine.rank() == 0) || (rOrientation.rank() == 0) ||
		(rSlope.rank() == 0) || (rIntercept.rank() == 0) )
	{
		m_oXPos = -1; // suppress paint

        preSignalAction();
		m_oPipeOutHeight.signal( GeoDoublearray(rContext, Doublearray(1, -1.0, 0), AnalysisOK, 0) );
		m_oPipeOutLength.signal( GeoDoublearray(rContext, Doublearray(1, 0.0, 0), AnalysisOK, 0) );
		m_oPipeOutSurface.signal( GeoDoublearray(rContext, Doublearray(1, 0.0, 0), AnalysisOK, 0) );
	} else
	{
		m_oSurface = 0.0; m_oHeight = -1.0; m_oLength = 0.0;

		auto oData = rLine.ref()[0].getData();
		m_oOrientation = valueToOrientation( std::get<eData>(rOrientation.ref()[0]) );
		m_oXLeft = std::get<eData>(rXLeft.ref()[0]); m_oXRight = std::get<eData>(rXRight.ref()[0]);
		m_oSlope = std::get<eData>(rSlope.ref()[0]); m_oIntercept = std::get<eData>(rIntercept.ref()[0]);

		if ( (m_oXLeft < 0) || (m_oXRight < 0) || (m_oXLeft >= m_oXRight) ) // if necessary, swap left and right points
		{
			if ( (m_oXLeft == m_oXRight) || (m_oXLeft < 0) || (m_oXRight < 0) )
			{
				m_oXPos = -1;

				preSignalAction();
				m_oPipeOutHeight.signal( GeoDoublearray(rContext, Doublearray(1, -1.0, 0), AnalysisOK, 0) );
				m_oPipeOutLength.signal( GeoDoublearray(rContext, Doublearray(1, 0.0, 0), AnalysisOK, 0) );
				m_oPipeOutSurface.signal( GeoDoublearray(rContext, Doublearray(1, 0.0, 0), AnalysisOK, 0) );
				return;
			}
			m_oYLeft = oData[(int)m_oXLeft]*1.0; m_oYRight = oData[(int)m_oXRight]*1.0;
			double oTmp = m_oXLeft; m_oXLeft = m_oXRight; m_oXRight = oTmp;
			oTmp = m_oYLeft; m_oYLeft = m_oYRight; m_oYRight = oTmp;
		}

		m_oSpTrafo	= rXLeft.context().trafo();
		if ( findExtremum(rLine.ref(), m_oOrientation) )
		{
			// do we also need to output the extremal point??? I daresay we don't...

            const auto oHeight    =   m_oHeight;    // dont access members after preSignalAction()
            const auto oLength    =   m_oLength;
            const auto oSurface   =   m_oSurface;

			preSignalAction();
			m_oPipeOutHeight.signal( GeoDoublearray(rContext, Doublearray(1, oHeight, eRankMax), AnalysisOK, 1.0) );
			m_oPipeOutLength.signal( GeoDoublearray(rContext, Doublearray(1, oLength, eRankMax), AnalysisOK, 1.0) );
			m_oPipeOutSurface.signal( GeoDoublearray(rContext, Doublearray(1, oSurface, eRankMax), AnalysisOK, 1.0) );
		} else
		{ // error
			m_oXPos = -1;
			preSignalAction();
			m_oPipeOutHeight.signal( GeoDoublearray(rContext, Doublearray(1, -1.0, 0), AnalysisOK, 0) );
			m_oPipeOutLength.signal( GeoDoublearray(rContext, Doublearray(1, 0.0, 0), AnalysisOK, 0) );
			m_oPipeOutSurface.signal( GeoDoublearray(rContext, Doublearray(1, 0.0, 0), AnalysisOK, 0) );
		}
	}

}


} // namespace precitec
} // namespace filter
