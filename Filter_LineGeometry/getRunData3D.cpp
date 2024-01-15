// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>

#include "filter/algoArray.h"	///< algorithmic interface for class TArray
#include <filter/structures.h>

// local includes
#include "getRunData3D.h"
#include "util/calibDataSingleton.h"
#include "2D/avgAndRegression.h"

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string GetRunData3D::m_oFilterName    = std::string("GetRunData3D");
const std::string GetRunData3D::PIPENAME_HEIGHT  = std::string("Height");
const std::string GetRunData3D::PIPENAME_LENGTH  = std::string("Length");
const std::string GetRunData3D::PIPENAME_SURF    = std::string("Surface");
const std::string GetRunData3D::PIPENAME_MID     = std::string("SeamCenter");

GetRunData3D::GetRunData3D() : TransformFilter( GetRunData3D::m_oFilterName, Poco::UUID{"66B86D76-DE17-47FA-90CA-EA2D9538BE43"} ),
	m_pPipeInXLeft( nullptr ), m_pPipeInXRight( nullptr ),
	m_pPipeInLaserline( nullptr ), m_pPipeInOrientation( nullptr ),

	m_oPipeOutHeight( this, GetRunData3D::PIPENAME_HEIGHT ), m_oPipeOutLength( this, GetRunData3D::PIPENAME_LENGTH ), m_oPipeOutSurface( this, GetRunData3D::PIPENAME_SURF ),
	m_oPipeOutRunMid( this, GetRunData3D::PIPENAME_MID ),

	m_oXLeft(-1), m_oXRight(-1), m_oOrientation(eOrientationInvalid),
	m_oHeight(0.0), m_oLength(0.0), m_oSurface(0.0),
	m_oPaint(true),
	m_oTypeOfLaserLine(LaserLine::FrontLaserLine)
{
	m_oMidArray.resize(1);
	//calib.setCalibrationMatrix(m_oCalibCam, true);
	//m_oCalibCamInv.invert(m_oCalibCam);
	parameters_.add("TypeOfLaserLine", Parameter::TYPE_int, int(m_oTypeOfLaserLine));

    setInPipeConnectors({{Poco::UUID("B4A99278-1425-4565-9EA5-0E3AB1373277"), m_pPipeInXLeft, "MarkerLeftX", 1, "xleft"},
    {Poco::UUID("3DE9AD71-A45D-42EE-B500-E1E068568278"), m_pPipeInXRight, "MarkerRightX", 1, "xright"},{Poco::UUID("C6E6CF36-E317-4E1B-9A5B-2CBE1D171B6A"), m_pPipeInLaserline, "Line", 1, "line"},{Poco::UUID("948E4DF8-9426-4CFC-8F6C-06716D441F94"), m_pPipeInOrientation, "Orientation", 1, "orientation"}});

    setOutPipeConnectors({{Poco::UUID("D632C02D-F640-4BD6-BD8B-812E65956AC5"), &m_oPipeOutLength, PIPENAME_LENGTH, 0, ""}, {Poco::UUID("5471A067-A20D-492D-9E6C-B4420D9F97E2"), &m_oPipeOutHeight, PIPENAME_HEIGHT, 0, ""},
    {Poco::UUID("C00EED2B-921B-4E27-BAC5-DFF52CCC69E0"), &m_oPipeOutSurface, PIPENAME_SURF, 0, ""},
    {Poco::UUID("45A28C9B-4BC8-4730-A546-1D68052EF164"), &m_oPipeOutRunMid, PIPENAME_MID, 0, ""}});
    setVariantID(Poco::UUID("02D828F3-E274-4390-8AC3-1ABC29CFC052"));

}

void GetRunData3D::setParameter()
{
	TransformFilter::setParameter();
	m_oTypeOfLaserLine = castToLaserLine(parameters_.getParameter("TypeOfLaserLine").convert<int>());

}

void GetRunData3D::paint() {
	if(m_oVerbosity < eLow || m_oSpTrafo.isNull() || (m_oYcoords.size() <= 0) )
	{
		return;
	}
	if ( !m_oPaint || (std::abs(m_oHeight) < math::eps) || (m_oSurface <= 0.0) || (m_oLength <= 0.0) )
	{
		return;
	}

	const Trafo		&rTrafo				( *m_oSpTrafo );
	OverlayCanvas	&rCanvas			( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerLine			( rCanvas.getLayerLine());

	geo2d::Point oFrom, oTo;

	for (unsigned int i=0; i < m_oYcoords.size()-1; ++i)
	{
		if ( (m_oYcoords[i][0] >= 0) && (m_oYcoords[i+1][0] >= 0) )
		{
			geo2d::Point oLineStart((int)m_oYcoords[i][0], (int)m_oYcoords[i][1]);
			geo2d::Point oLineEnd((int)m_oYcoords[i + 1][0], (int)m_oYcoords[i + 1][1]);
			rLayerLine.add(new OverlayLine(rTrafo(oLineStart), rTrafo(oLineEnd), Color::Yellow()));
		}
	}

	oFrom.x = (int)m_oExtremum[0]; oFrom.y = (int)m_oExtremum[1];
	oTo.x = (int)m_oExtremumProj[0]; oTo.y = (int)m_oExtremumProj[1];
	rLayerLine.add( new OverlayLine(rTrafo(oFrom), rTrafo(oTo), Color::Orange()) );
	rLayerLine.add( new OverlayCross(rTrafo( oFrom ), 4, Color::Orange()) );
	rLayerLine.add( new OverlayCross(rTrafo( oTo ), 4, Color::Orange()) );

	//oFrom.x = (int)m_oMid[0]; oFrom.y = (int)m_oMid[1];
	//oTo.x = (int)m_oMidProj[0]; oTo.y = (int)m_oMidProj[1];
	//rLayer.add( new OverlayLine(rTrafo(oFrom), rTrafo(oTo), Color::Yellow() ) );
	//rLayer.add( new OverlayCross(rTrafo( oFrom ), 6, Color::Yellow()) );
	//rLayer.add( new OverlayCross(rTrafo( oTo ), 6, Color::Yellow()) );
	rLayerLine.add( new OverlayCross(rTrafo( geo2d::Point(m_oXLeft, m_oYLeft) ), 6, Color::Green()) );
	rLayerLine.add( new OverlayCross(rTrafo( geo2d::Point(m_oXRight, m_oYRight) ), 6, Color::Green()) );

} // paint

bool GetRunData3D::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if (p_rPipe.tag() == "xleft")
	{
		m_pPipeInXLeft  = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray >* >(&p_rPipe);
	} else if (p_rPipe.tag() == "xright")
	{
		m_pPipeInXRight  = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray >* >(&p_rPipe);
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


bool GetRunData3D::get3DValues(const geo2d::VecDoublearray &p_rLine, const Vec3D &p_oStart, const Vec3D &p_oEnd)
{
	Vec3D oCoordScreen, oProjScreen, oProj3D, oExtremum, oExtremumProj;
	LineSegment oSegmentScreen, oSegment3D;

	if (std::abs(p_oEnd[0] - p_oStart[0]) < math::eps)
	{
		// todo: handle case
		return false;
	}

	auto &oData = p_rLine[0].getData();
	auto &oRank = p_rLine[0].getRank();
	oSegmentScreen.preComputeSegmentData( p_oStart, p_oEnd );

	// for painting, pixel data for object on screen
	double oSlopeScreen = (p_oEnd[1] - p_oStart[1])/(p_oEnd[0] - p_oStart[0]);
	double oYValScreen = 0.0;
	int oLenScreen = int(p_oEnd[0] - p_oStart[0] + 1);
	m_oYcoords.resize(oLenScreen);

	// object coordinate 3D system variables
	assert(m_pCoordTransformer && "m_pCoordTransformer called before it was set in ProceedGroup");

	Vec3D oFrom = m_pCoordTransformer->imageCoordTo3D(p_oStart[0], p_oStart[1]);
	Vec3D oTo = m_pCoordTransformer->imageCoordTo3D(p_oEnd[0], p_oEnd[1]);
	Vec3D oCoord3D;
	oSegment3D.preComputeSegmentData( oFrom, oTo );
	m_oLength = std::abs(oTo[0] - oFrom[0]);
	m_oHeight = 0.0; double oZHeight = 0.0; m_oSurface = 0.0; int oIdx = -1;

	// traverse seam to find highest point
	for (int i=0; i < oLenScreen; ++i)
	{
		const double oYVal(p_oStart[0] + i);  // current x position
		const int oYValInt = static_cast<int>(oYVal);
		if (oRank[oYValInt] > eRankMin)
		{
			oYValScreen = oSlopeScreen*i + p_oStart[1];             // get screen y...
			m_oYcoords[i].set(oYVal, oYValScreen);                 // and x coordinate for segment connecting start and endpoint of bead/gap

			oCoord3D = m_pCoordTransformer->imageCoordTo3D(oYValInt, static_cast<int>(oData[oYValInt]));
			oProj3D = oCoord3D.projOntoSegment(oSegment3D, false);
			double oDist2 = oCoord3D.dist2(oProj3D);                // square of 3D distance from coord to its projection onto the segment joining start end end of seam
			oZHeight = sqrt(oDist2);
			m_oSurface += oZHeight;

			if (oZHeight > std::abs(m_oHeight))
			{
				if (oProj3D[1] < oCoord3D[1]) // mathematical coord system!
				{
					m_oHeight = oZHeight;
				} else
				{
					m_oHeight = -oZHeight;
				}
				oIdx = i;
				oCoordScreen.set(oYVal, oData[oYValInt], 1);
				oProjScreen = oCoordScreen.projOntoSegment(oSegmentScreen, false);
				m_oExtremum.set(oCoordScreen[0], oCoordScreen[1]); m_oExtremumProj.set(oProjScreen[0], oProjScreen[1]);
			}
		} else
		{
			m_oYcoords[i].set(-1, -1);
		}
	}

	if (oIdx >= 0)
	{
		return true;
	} else
	{
		m_oHeight = 0.0;
		m_oSurface = 0.0;
		m_oLength = 0.0;
		return false;
	}
}

bool GetRunData3D::findExtremum(const geo2d::VecDoublearray &p_rLine)
{
	auto &oData = p_rLine[0].getData();
	m_oLength = 0.0;
	m_oHeight = 0.0;
	m_oSurface = 0.0;

	// for paint
	m_oYcoords.resize(0);

	Vec3D oStart(1.0*m_oXLeft, 1.0*oData[m_oXLeft], 1); Vec3D oEnd(1.0*m_oXRight, 1.0*oData[m_oXRight], 1);
	return get3DValues(p_rLine, oStart, oEnd);
}

void GetRunData3D::signalSend(const ImageContext &p_rImgContext, const int p_oIO)
{
 	if (p_oIO > 0)
	{
		m_oMidArray[0].getData().resize(3);
		m_oMidArray[0].getData()[0] = m_oMid[0]; m_oMidArray[0].getData()[1] = m_oMid[1]; m_oMidArray[0].getData()[2] = m_oMid[2];
		m_oMidArray[0].getRank().resize(3);
		m_oMidArray[0].getRank()[0] = eRankMax; m_oMidArray[0].getRank()[1] = eRankMax; m_oMidArray[0].getRank()[2] = eRankMax;

        const auto oGeoHeight       =   GeoDoublearray(p_rImgContext, Doublearray(1, m_oHeight, eRankMax), AnalysisOK, 1.0);
        const auto oGeoLenght       =   GeoDoublearray(p_rImgContext, Doublearray(1, sqrt(m_oLength), eRankMax), AnalysisOK, 1.0);
        const auto oGeoSurface      =   GeoDoublearray(p_rImgContext, Doublearray(1, m_oSurface, eRankMax), AnalysisOK, 1.0);
        const auto oGeoRunMid       =   GeoVecDoublearray(p_rImgContext, m_oMidArray, AnalysisOK, 1.0);

        preSignalAction();
		m_oPipeOutHeight.signal( oGeoHeight );
		m_oPipeOutLength.signal( oGeoLenght );
		m_oPipeOutSurface.signal( oGeoSurface );
		m_oPipeOutRunMid.signal( oGeoRunMid );
	} else
	{
		if (p_oIO == 0)
		{
			m_oMidArray[0].getData().resize(1); m_oMidArray[0].getRank().resize(1);
			m_oMidArray[0].getData()[0] = 0.0; m_oMidArray[0].getRank()[0] = eRankMin;
			// No Bead or Gap is not supposed to throw an error, just to output value 0.0 for height, surface and length
		//	m_oPipeOutHeight.signal( GeoDoublearray(p_rImgContext, Doublearray(1, 0.0, 0), interface::AnalysisErrNoBeadOrGap, 1.0) );
		//	m_oPipeOutLength.signal( GeoDoublearray(p_rImgContext, Doublearray(1, 0.0, 0), interface::AnalysisErrNoBeadOrGap, 1.0) );
		//	m_oPipeOutSurface.signal( GeoDoublearray(p_rImgContext, Doublearray(1, 0.0, 0), interface::AnalysisErrNoBeadOrGap, 1.0) );
		//	m_oPipeOutRunMid.signal(GeoVecDoublearray(p_rImgContext, m_oMidArray, interface::AnalysisErrNoBeadOrGap, 1.0) );

            const auto oGeoRunMid       =   GeoVecDoublearray(p_rImgContext, m_oMidArray, interface::AnalysisOK, 1.0);

            preSignalAction();
			m_oPipeOutHeight.signal( GeoDoublearray(p_rImgContext, Doublearray(1, 0.0, eRankMax), interface::AnalysisOK, 1.0) );
			m_oPipeOutLength.signal( GeoDoublearray(p_rImgContext, Doublearray(1, 0.0, eRankMax), interface::AnalysisOK, 1.0) );
			m_oPipeOutSurface.signal( GeoDoublearray(p_rImgContext, Doublearray(1, 0.0, eRankMax), interface::AnalysisOK, 1.0) );
			m_oPipeOutRunMid.signal( oGeoRunMid );
		} else
		{
            //std::cout << "****GetRunData3D::signalSend() AnalysisError -> interface::AnalysisErrBadLaserline (1201)" << std::endl;
            wmLog( eDebug,"****GetRunData3D::signalSend() AnalysisError -> interface::AnalysisErrBadLaserline (1201)\n");
			m_oMidArray[0].getData().resize(1); m_oMidArray[0].getRank().resize(1);
			m_oMidArray[0].getData()[0] = 0.0; m_oMidArray[0].getRank()[0] = eRankMin;

            const auto oGeoRunMid       =   GeoVecDoublearray(p_rImgContext, m_oMidArray, interface::AnalysisErrBadLaserline, interface::NotPresent);

            preSignalAction();
			m_oPipeOutHeight.signal( GeoDoublearray(p_rImgContext, Doublearray(1, 0.0, 0), interface::AnalysisErrBadLaserline, interface::NotPresent) );
			m_oPipeOutLength.signal( GeoDoublearray(p_rImgContext, Doublearray(1, 0.0, 0), interface::AnalysisErrBadLaserline, interface::NotPresent) );
			m_oPipeOutSurface.signal( GeoDoublearray(p_rImgContext, Doublearray(1, 0.0, 0), interface::AnalysisErrBadLaserline, interface::NotPresent) );
			m_oPipeOutRunMid.signal( oGeoRunMid );
 		}
	}
}

void GetRunData3D::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeInXLeft != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInXRight != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInLaserline != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInOrientation != nullptr); // to be asserted by graph editor

	// Get Coords
	const GeoDoublearray &rXLeft( m_pPipeInXLeft->read(m_oCounter) );
	const GeoDoublearray &rXRight( m_pPipeInXRight->read(m_oCounter) );
	const GeoVecDoublearray &rLine( m_pPipeInLaserline->read(m_oCounter) );
	const GeoDoublearray &rOrientation( m_pPipeInOrientation->read(m_oCounter) );

	const ImageContext &rContext(rXLeft.context());
	m_oSpTrafo = rContext.trafo();
	m_pCoordTransformer = system::CalibDataSingleton::getImageCoordsto3DCoordTransformer(math::SensorId::eSensorId0, rContext, m_oTypeOfLaserLine);

	auto oAnalysisResult = rOrientation.analysisResult() == AnalysisErrNoBeadOrGap ? AnalysisErrNoBeadOrGap : rOrientation.analysisResult();

	if ( (rXLeft.rank() == 0) || (rXRight.rank() == 0) || (rLine.rank() == 0) || (rOrientation.rank() == 0) )
	{
		m_oPaint = false; // suppress paint
		if (oAnalysisResult != AnalysisOK)
		{
			signalSend(rContext, 0);
		} else
		{
			signalSend(rContext, -1);
		}
	} else
	{
		auto &rData( rLine.ref()[0].getData() );
		m_oOrientation = valueToOrientation( std::get<eData>(rOrientation.ref()[0]) );
		if (oAnalysisResult != AnalysisOK)
		{
			m_oPaint = false;
			signalSend(rContext, -1); // whenever the analysis is not OK, send negative area for now...
		} else
		{
			// Get start and end laser line points of run...
			m_oXLeft = (int)(std::get<eData>(rXLeft.ref()[0])); m_oXRight = (int)(std::get<eData>(rXRight.ref()[0]));
            bool send = false;

			 // if necessary, swap left and right points
			if ( (m_oXLeft < 0) || (m_oXRight < 0) || (m_oXLeft >= m_oXRight) )
			{
				if ( (m_oXLeft == m_oXRight) || (m_oXLeft < 0) || (m_oXRight < 0) ) // bead/gap too short, nothing to measure here
				{
					m_oPaint = false; // suppress paint
					send = true;  // mark signal as send
					signalSend(rContext, 0);
				}
				int oTmp = m_oXLeft; m_oXLeft = m_oXRight; m_oXRight = oTmp; // swap
			}

			if (!send)
			{
				m_oYLeft = int( rData[m_oXLeft] ); m_oYRight = int( rData[m_oXRight] ); // for painting

				// Start computation of data
				if ( findExtremum(rLine.ref()) )
				{
					m_oPaint = true;
					signalSend(rContext, 1);
				} else
				{	// error
					m_oPaint = false; // suppress paint
					signalSend(rContext, 0);
				}
			}
		}
	}

}


} // namespace precitec
} // namespace filter
