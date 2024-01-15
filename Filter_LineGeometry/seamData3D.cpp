// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>

#include "filter/algoArray.h"	///< algorithmic interface for class TArray
#include "filter/algoImage.h"   ///< for applying trafo to Vec3D for 2d to 3d conversion
#include <filter/structures.h>
#include <fliplib/TypeToDataTypeImpl.h>

// local includes
#include "seamData3D.h"
#include "util/calibDataSingleton.h"
#include "2D/avgAndRegression.h"
#include "line2D.h"

using namespace fliplib;

namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;

namespace filter {

const std::string SeamData3D::m_oFilterName    = std::string("SeamData3D");
const std::string SeamData3D::PIPENAME_HEIGHT  = std::string("Height");
const std::string SeamData3D::PIPENAME_LENGTH  = std::string("Length");
const std::string SeamData3D::PIPENAME_SURF    = std::string("Surface");
const std::string SeamData3D::PIPENAME_ORIENT  = std::string("Orientation");

const std::string SeamData3D::m_oParamTypeOfLaserLine("TypeOfLaserLine"); ///< Parameter: Type of LaserLine (e.g. FrontLaserLine, BehindLaserLine)



SeamData3D::SeamData3D() : TransformFilter( SeamData3D::m_oFilterName, Poco::UUID{"64BADBF5-71F3-4153-AFAA-9B0B1A460466"} ),
	m_pPipeInXLeft( nullptr ), m_pPipeInXRight( nullptr ),
	m_pPipeInLaserline( nullptr ),

	m_oPipeOutHeight( this, SeamData3D::PIPENAME_HEIGHT ), m_oPipeOutLength( this, SeamData3D::PIPENAME_LENGTH ), m_oPipeOutSurface( this, SeamData3D::PIPENAME_SURF ),
	m_oPipeOutOrient( this, SeamData3D::PIPENAME_ORIENT ),

	m_oXLeft(-1), m_oXRight(-1), m_oOrientation(eOrientationInvalid),
	m_oHeight(0.0), m_oLength(0.0), m_oSurface(0.0),
	m_oPaint(true),
	m_oTypeOfLaserLine(LaserLine::FrontLaserLine),
	m_oXPosMax(0),
	m_oYPosMax(0),
	m_oXPosMaxLine(0),
	m_oYPosMaxLine(0)
{
	int oLaserLineTemp = static_cast<int>(m_oTypeOfLaserLine);
	parameters_.add(m_oParamTypeOfLaserLine, fliplib::Parameter::TYPE_int, oLaserLineTemp);  // Fuege den Parameter mit dem soeben initialisierten Wert hinzu.

    setInPipeConnectors({{Poco::UUID("DBAC62A9-028A-4871-8CD4-2F7AD0DEDB12"), m_pPipeInLaserline, "Line", 1, "line"},
    {Poco::UUID("679107E5-A20F-46FF-85BC-55786F1DCFD3"), m_pPipeInXLeft, "MarkerLeftX", 1, "xleft"},
    {Poco::UUID("5B4A0103-E174-4C5E-9B2F-767B4C7A2A19"), m_pPipeInXRight, "MarkerRightX", 1, "xright"}});
    setOutPipeConnectors({{Poco::UUID("C72AB0D0-3166-482C-B848-231AF0B5119A"), &m_oPipeOutLength, PIPENAME_LENGTH, 0, ""}, {Poco::UUID("5C9C11B2-01DE-4D09-9E63-B3DA9568A643"), &m_oPipeOutHeight, PIPENAME_HEIGHT, 0, ""},{Poco::UUID("FCB8060B-5694-4DF2-BED1-F9ACF9CB010A"), &m_oPipeOutSurface, PIPENAME_SURF, 0, ""},
    {Poco::UUID("0C75C5AD-14D5-472C-A903-77E0C1994510"), &m_oPipeOutOrient, PIPENAME_ORIENT, 0, ""}});
    setVariantID(Poco::UUID("8737AE08-461A-48E5-A7C2-AB7DDC87E1FD"));
}

void SeamData3D::setParameter()
{
	TransformFilter::setParameter();

	int oTempLine = parameters_.getParameter(SeamData3D::m_oParamTypeOfLaserLine).convert<int>();
	m_oTypeOfLaserLine = static_cast<LaserLine>(oTempLine);
}

void SeamData3D::paint() {
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

    const auto oYellow = Color::Yellow();
	for (unsigned int i=0; i < m_oYcoords.size()-1; ++i)
	{
		if ( (m_oYcoords[i][0] >= 0) && (m_oYcoords[i+1][0] >= 0) )
		{
			geo2d::Point oLineStart((int)m_oYcoords[i][0], (int)m_oYcoords[i][1]);
			geo2d::Point oLineEnd((int)m_oYcoords[i + 1][0], (int)m_oYcoords[i + 1][1]);
			rLayerLine.add<OverlayLine>(rTrafo(oLineStart), rTrafo(oLineEnd), oYellow);
		}
	}


	rLayerLine.add(new OverlayCross(rTrafo(geo2d::Point(m_oXPosMax, m_oYPosMax)), 6, Color::Orange()));
	rLayerLine.add(new OverlayCross(rTrafo(geo2d::Point(m_oXPosMaxLine, m_oYPosMaxLine)), 6, Color::Orange()));


	oFrom.x = (int)m_oXPosMax;
	oFrom.y = (int)m_oYPosMax;
	oTo.x   = (int)m_oXPosMaxLine;
	oTo.y   = (int)m_oYPosMaxLine;

	rLayerLine.add<OverlayLine>(rTrafo(oFrom), rTrafo(oTo), Color::Orange());

	//rLayerLine.add( new OverlayCross(rTrafo( oFrom ), 4, Color::Orange()) );
	//rLayerLine.add( new OverlayCross(rTrafo( oTo ), 4, Color::Orange()) );

	rLayerLine.add<OverlayCross>(rTrafo( geo2d::Point(m_oXLeft, m_oYLeft) ), 6, Color::Green());
	rLayerLine.add<OverlayCross>(rTrafo( geo2d::Point(m_oXRight, m_oYRight) ), 6, Color::Green());

} // paint

bool SeamData3D::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if (p_rPipe.tag() == "xleft")
	{
		m_pPipeInXLeft  = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray >* >(&p_rPipe);
	} else if (p_rPipe.tag() == "xright")
	{
		m_pPipeInXRight  = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray >* >(&p_rPipe);
	} else if ( (p_rPipe.tag() == "line") || (p_rPipe.tag() == "") )
	{
		m_pPipeInLaserline = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray >* >(&p_rPipe);
	}
	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe


// -----------------------------------------------------------


bool SeamData3D::get3DValues(const geo2d::VecDoublearray &p_rLine, const Vec3D &p_oStart, const Vec3D &p_oEnd, const math::Calibration3DCoords &p_rCalib)
{
	Vec3D oCoordScreen, oProjScreen, oProj3D, oExtremum, oExtremumProj;
	LineSegment oSegmentScreen, oSegment3D;

	if (std::abs(p_oEnd[0] - p_oStart[0]) < math::eps)
	{
		// todo: handle case
		return false;
	}

	/* these 3D coordinates are not used, the actual 3D measurement is only to compute m_oHeight in proceedGroup
	const Trafo	&rTrafo( *m_oSpTrafo );

	Vec3D oStart = filter::applyTrafo(p_oStart, rTrafo);  // we need the correct screen coordinates...
	Vec3D oEnd = filter::applyTrafo(p_oEnd, rTrafo);

	Vec3D oFrom = p_rCalib.to3D(oStart);                // ...for getting the correct 3D coordinates
	Vec3D oTo = p_rCalib.to3D(oEnd);
	//std::cout << "S " << oStart[0] << ", " << oStart[1] << "; F " << oFrom[0] << ", " << oFrom[1] << std::endl;
	*/

	// for painting, pixel data for object on screen
	//oSegmentScreen.preComputeSegmentData( p_oStart, p_oEnd );

	// in pixel: steigung und laenge
	double oSlopeScreen = (p_oEnd[1] - p_oStart[1])/(p_oEnd[0] - p_oStart[0]);
	int oLenScreen		= int(p_oEnd[0] - p_oStart[0] + 1);


	m_oYcoords.resize(oLenScreen);

	// object coordinate 3D system variables
	//oSegment3D.preComputeSegmentData( oFrom, oTo );


	//m_oLength = std::abs(oTo[0] - oFrom[0]);
	m_oLength = 0.0;


	m_oHeight = 0.0;
	//double oZHeight = 0.0;
	m_oSurface = 0.0;
	int oIdx = -1;
	//double oLastHeight(0.0);


	auto &oData = p_rLine[0].getData();
	auto &oRank = p_rLine[0].getRank();
	double oYValScreen( 0.0 );
	//Vec3D oCoord3D, oLastProj3D,oPoint;

	//double diff = 0.0;
	double diff2 = 0.0;
	//double maxdiff = 0.0;
	double maxdiff2 = 0.0;
	//int xPos = 0;
	//int yPos = 0;

	double AreaPos = 0.0;
	double AreaNeg = 0.0;
	double ydiff   = 0.0;

	Line2D seamline(p_oStart[0], p_oStart[1], p_oEnd[0], p_oEnd[1]);

	// traverse seam to find highest point
	for (int i=0; i < oLenScreen; ++i)
	{
		const double oXVal(p_oStart[0] + i);  // current x position -- oYVal
		const int oXValInt = static_cast<int>(oXVal);
		if (oRank[oXValInt] > eRankMin)
		{
			oYValScreen = oSlopeScreen*i + p_oStart[1];            // y Gerade
			m_oYcoords[i].set(oXVal, oYValScreen);                 // Punkte der Verbindungsgeraden

			if(i>1){
				ydiff = oData[oXValInt] - oData[oXValInt-1];
				m_oLength +=   sqrt(1 +  ydiff * ydiff ); // Laenge in Pixeln entlang der Nahtraupe
			}
			//oPoint = filter::applyTrafo(oXValInt, oData[oXValInt], rTrafo); //punkt auf laserlinie + trafo : Achtung HW ROI fehlt

			//pixel koordinaten:
			//std::cout<<"Gerade: "<<oXVal<<" "<<oYValScreen<<" "<<"laserline: "<<oXValInt<<" "<<oData[oXValInt]<<std::endl;

			//distanz in y Richting
			//diff = oYValScreen - oData[oXValInt];
			//if ( std::abs(diff) > maxdiff)
			//{
			//	m_oXPosMax = oXValInt;
			//	m_oYPosMax = oData[oXValInt];

			//	m_oXPosMaxLine = oXValInt;
			//	m_oYPosMaxLine = oYValScreen;
			//	//Achtung neu:
			//	m_oHeight = diff;

			//	maxdiff = std::abs(diff);
			//}


			//distanz senkrecht zur Verbindungsgeraden
			diff2= seamline.calcDistance(oXValInt, oData[oXValInt]);
			if (std::abs(diff2) > maxdiff2)
			{
				oIdx = i;
				m_oXPosMax = oXValInt;
				m_oYPosMax = oData[oXValInt];

				m_oXPosMaxLine = seamline.m_oInterceptX;
				m_oYPosMaxLine = seamline.m_oInterceptY;

				m_oHeight = diff2;
				maxdiff2 = std::abs(diff2);
			}

			//Flaeche in Pixel
			if (oLenScreen > 1)
			{
				if (diff2 > 0)
					AreaPos += diff2;
				if (diff2 < 0)
					AreaNeg += diff2;

			}

			//3D Koordinaten mit laserline 0 oder 1
			//std::cout<<"laserline: "<<m_oTypeOfLaserLine<<std::endl;

			//laserline 3d punkt
			//oCoord3D = p_rGrid.to3D(static_cast<int>( oPoint[0] ), static_cast<int>( oPoint[1] ), 0, m_oTypeOfLaserLine); // todo: sensorID


			//oProj3D = oCoord3D.projOntoSegment(oSegment3D, false); //Projektion auf Verbindungsgerade

			/*
			 * for now we consider z distances only, which vary with the triangulation angle in opposite to the Euklidean
			 * distance, which is invariant. At some point of time, this might be considered as a parameter for the filter... choose y, z, dist...
			 */
			//double oDist2 = oCoord3D.dist2(oProj3D); oZHeight = sqrt(oDist2);  // square of 3D distance from coord to its projection onto the segment joining start end end of seam

			//oZHeight = oCoord3D[2] - oProj3D[2];
			//if (std::abs(oZHeight) > std::abs(m_oHeight))
			//{
			//	m_oHeight = oZHeight;
			//	oIdx = i;

			//	// for painting...
			//	oCoordScreen.set(oXVal, oData[oXValInt], 1);  //oYVal

			//	oProjScreen = oCoordScreen.projOntoSegment(oSegmentScreen, false);
			//	m_oExtremum.set(oCoordScreen[0], oCoordScreen[1]); m_oExtremumProj.set(oProjScreen[0], oProjScreen[1]);
			//}

			// Compute surface (hyperplane) via integration in the real coord system.

			/*if ( oLenScreen > 1)
			{
				if (i >= 1)
				{
					double oDist = std::abs(oLastProj3D[0] - oProj3D[0]);
					m_oSurface += std::abs( 0.5*(oLastHeight + oZHeight)*oDist );
				}
			} else
			{
				m_oSurface += oZHeight;
			}*/


			/*oLastProj3D = oProj3D;
			oLastHeight = oZHeight;*/
		}
		else
		{
			m_oYcoords[i].set(-1, -1);
		}
	}

	//abfrage m_oLenght kann zu gross werden - Hack wg. PLotter
	if(m_oLength > 500.0)
		m_oLength = 500.0;


	if (std::abs(AreaNeg) > AreaPos)
		m_oSurface = AreaNeg;
	else
		m_oSurface = AreaPos;

	if (m_oHeight > 0)
	{
		m_oOrientation = eOrientationConvex;
	} else if (m_oHeight < 0)
	{
		m_oOrientation = eOrientationConcave;
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

bool SeamData3D::findExtremum(const geo2d::VecDoublearray &p_rLine, const math::Calibration3DCoords &p_rCalib)
{
	auto &oData = p_rLine[0].getData();
	m_oLength = 0.0; m_oHeight = 0.0; m_oSurface = 0.0; m_oOrientation = eOrientationInvalid;

	// for paint
	m_oYcoords.resize(0);

	Vec3D oStart(1.0*m_oXLeft, 1.0*oData[m_oXLeft], 1); Vec3D oEnd(1.0*m_oXRight, 1.0*oData[m_oXRight], 1);

	if (!get3DValues(p_rLine, oStart, oEnd, p_rCalib))
		return false;


	return true;

}


// -------------------------------------------------------------

void SeamData3D::signalSendInvalidResult(const ImageContext &p_rImgContext, ResultType p_oAnalysisResult)
{
	//std::cout << "****SeamData3D::signalSend() AnalysisError -> interface::AnalysisErrBadLaserline (1201)" << std::endl;
	wmLog( eDebug,"****SeamData3D::signalSend() AnalysisError -> interface::AnalysisErrBadLaserline (1201)\n");
	preSignalAction();
	m_oPipeOutHeight.signal( GeoDoublearray(p_rImgContext, Doublearray(1, 0.0, 0), p_oAnalysisResult, interface::NotPresent) );
	m_oPipeOutLength.signal( GeoDoublearray(p_rImgContext, Doublearray(1, 0.0, 0), p_oAnalysisResult, interface::NotPresent) );
	m_oPipeOutSurface.signal( GeoDoublearray(p_rImgContext, Doublearray(1, 0.0, 0), p_oAnalysisResult, interface::NotPresent) );
	m_oPipeOutOrient.signal( GeoDoublearray(p_rImgContext, Doublearray(1, 0.0, 0), p_oAnalysisResult, interface::NotPresent) );

}
void SeamData3D::signalSend(const ImageContext &p_rImgContext, const int p_oIO)
{
	const GeoVecDoublearray &rLine( m_pPipeInLaserline->read(m_oCounter) );

	if (p_oIO > 0)
	{
		double oOrientation = static_cast<double>(m_oOrientation);
		clipDouble(m_oHeight, -9999, 9999);
		clipDouble(m_oLength, -9999, 9999);
		clipDouble(m_oSurface, -9999, 9999);
		clipDouble(oOrientation, -9999, 9999);

        const auto oHeight      = m_oHeight;    // do not access members after preSignalAction()
        const auto oLength      = m_oLength;
        const auto oSurface     = m_oSurface;

        preSignalAction();
        m_oPipeOutHeight.signal( GeoDoublearray(p_rImgContext, Doublearray(1, oHeight, eRankMax), rLine.analysisResult(), 1.0) );
		m_oPipeOutLength.signal( GeoDoublearray(p_rImgContext, Doublearray(1, oLength, eRankMax), rLine.analysisResult(), 1.0) );
		m_oPipeOutSurface.signal( GeoDoublearray(p_rImgContext, Doublearray(1, oSurface, eRankMax), rLine.analysisResult(), 1.0) );
		m_oPipeOutOrient.signal( GeoDoublearray(p_rImgContext, Doublearray(1, oOrientation, eRankMax), rLine.analysisResult(), 1.0) );
	} else
	{
		if (p_oIO == 0)
		{
            preSignalAction();
			m_oPipeOutHeight.signal( GeoDoublearray(p_rImgContext, Doublearray(1, 0.0, eRankMax), rLine.analysisResult(), 1.0) );
			m_oPipeOutLength.signal( GeoDoublearray(p_rImgContext, Doublearray(1, 0.0, eRankMax), rLine.analysisResult(), 1.0) );
			m_oPipeOutSurface.signal( GeoDoublearray(p_rImgContext, Doublearray(1, 0.0, eRankMax), rLine.analysisResult(), 1.0) );
			m_oPipeOutOrient.signal( GeoDoublearray(p_rImgContext, Doublearray(1, 0.0, eRankMax), rLine.analysisResult(), 1.0) );
		} else
		{
			signalSendInvalidResult(p_rImgContext, rLine.analysisResult());
		}
	}
}

void SeamData3D::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeInXLeft != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInXRight != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInLaserline != nullptr); // to be asserted by graph editor

	// Get Coords
	const GeoDoublearray &rXLeft( m_pPipeInXLeft->read(m_oCounter) );
	const GeoDoublearray &rXRight( m_pPipeInXRight->read(m_oCounter) );
	const GeoVecDoublearray &rLine( m_pPipeInLaserline->read(m_oCounter) );

	geo2d::TPoint<double>	oPositionLaserInRoi;
	geo2d::TPoint<double>	oPositionLineInRoi;


	const ImageContext &rContext(rXLeft.context());
	interface::SmpTrafo	 oSpTrafo = rXLeft.context().trafo();

	geo2d::TPoint<double>	oRoiPos(oSpTrafo->dx(),oSpTrafo->dy());
	geo2d::TPoint<double>	oHwRoiPos(rContext.HW_ROI_x0, rContext.HW_ROI_x0);

	auto oAnalysisResult = rLine.analysisResult() == AnalysisErrNoBeadOrGap ? AnalysisErrNoBeadOrGap : rLine.analysisResult();

	if ( (rXLeft.rank() == 0) || (rXRight.rank() == 0) || (rLine.rank() == 0)
		|| rXLeft.ref().size() == 0 || rXRight.ref().size() == 0 || rLine.ref().size() == 0
	)
	{
		m_oPaint = false; // suppress paint
		if (oAnalysisResult != AnalysisOK)
		{
			signalSend(rContext, 0);
			return;
		} else
		{
			signalSendInvalidResult(rContext, rLine.analysisResult());
			return;
		}
	}
	if (oAnalysisResult != AnalysisOK)
	{
		m_oPaint = false;
		signalSendInvalidResult(rContext, rLine.analysisResult()); // whenever the analysis is not OK, send negative area for now...
		return;
	}


	auto &rData( rLine.ref()[0].getData() );
	int oDataSize= rData.size();

	// Get start and end laser line points of run... hier knallt es ab und zu get geht aus dem ROI...
	m_oXLeft = (int)(std::get<eData>(rXLeft.ref()[0]));
	m_oXRight = (int)(std::get<eData>(rXRight.ref()[0]));

	// if necessary, swap left and right points
	if (m_oXLeft > m_oXRight)
	{
		int oTmp = m_oXLeft; m_oXLeft = m_oXRight; m_oXRight = oTmp; // swap
	}

	// are the left and right coords meaningful? if not, get out of here ...
	if ( m_oXLeft < 0 || m_oXRight < 0 || m_oXLeft == m_oXRight || m_oXLeft >= oDataSize || m_oXRight >= oDataSize )
	{
		m_oPaint = false; // suppress paint
		signalSend(rContext, 0);
	}
	else
	{
		m_oYLeft = int(rData[m_oXLeft]);
		m_oYRight = int(rData[m_oXRight]); // for painting

		// Start computation of data
		m_oSpTrafo	= rXLeft.context().trafo();
		auto &rCalib( system::CalibDataSingleton::getCalibrationCoords(math::SensorId::eSensorId0) );
		if ( findExtremum(rLine.ref(), rCalib) )
		{
			//3D Umrechnungen
			oPositionLaserInRoi.x = m_oXPosMax;
			oPositionLaserInRoi.y = m_oYPosMax;
			oPositionLineInRoi.x = m_oXPosMaxLine;
			oPositionLineInRoi.y = m_oYPosMaxLine;

			//std::cout<<"Hoehe, Surface  in Pixel: "<<m_oHeight<<", "<<m_oSurface<<std::endl;


			//Umrechnungen: m_oXPosMax,m_oYPosMax,m_oXPosMaxLine,m_oYPosMaxLine,diff;
			const TPoint<double>	oSensorPosLine(oPositionLineInRoi + oRoiPos + oHwRoiPos);	//	Offset Bildkoordinaten -> Sensorkoordinaten
			const TPoint<double>	oSensorPosLaser(oPositionLaserInRoi + oRoiPos + oHwRoiPos);	//	Offset Bildkoordinaten -> Sensorkoordinaten

			//Umrechnung 3D

			const math::Vec3D oSensorCoordLaser = rCalib.to3D(oSensorPosLaser.x, oSensorPosLaser.y, m_oTypeOfLaserLine);
			const math::Vec3D oSensorCoordLine  = rCalib.to3D(oSensorPosLine.x,oSensorPosLine.y, m_oTypeOfLaserLine);

			m_oHeight = oSensorCoordLaser[2] - oSensorCoordLine[2];
			//std::cout<<"Hoehe in abs. Einheiten: "<<m_oHeight<<std::endl;

			m_oPaint = true;
			signalSend(rContext, 1);
		} else
		{	// error
			m_oPaint = false; // suppress pain
			signalSend(rContext, 0);
		}
	} // if (!m_oSend)


}

void SeamData3D::clipDouble(double & d, double lowerLimit, double upperLimit)
{
	if (d > upperLimit) d = upperLimit;
	if (d < lowerLimit) d = lowerLimit;
}



} // namespace precitec
} // namespace filter
