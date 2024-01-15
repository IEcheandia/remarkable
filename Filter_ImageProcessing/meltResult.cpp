/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		2015
 * 	@brief		Filter which computes the relevant numbers for weldings bath analysis (Qualas).
 */

// WM includes

#include "fliplib/PipeEventArgs.h"
#include "fliplib/Parameter.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "image/image.h"
#include "module/moduleLogger.h"
#include "filter/algoStl.h"
#include <fliplib/TypeToDataTypeImpl.h>

#include "meltResult.h"


using namespace fliplib;
namespace precitec {
	using namespace geo2d;
	using namespace interface;
	using namespace image;
namespace filter {


const std::string MeltResult::m_oFilterName 							( std::string("MeltResult") );							///< Filter name.
const std::string MeltResult::m_oPipeOutPenetrationIndexName 			( std::string("MeltResultPenetrationIndexOut") );		///< Pipe: Scalar out-pipe.
const std::string MeltResult::m_oPipeOutSeamPositionName 				( std::string("MeltResultSeamPositionOut") );			///< Pipe: Scalar out-pipe.
const std::string MeltResult::m_oPipeOutSeamAsymmetryName 				( std::string("MeltResultSeamAsymmetryOut") );			///< Pipe: Scalar out-pipe.
const std::string MeltResult::m_oPipeOutAdditionalMaterialWidthName 	( std::string("MeltResultAdditionalMaterialWidthOut") );	///< Pipe: Scalar out-pipe.
const std::string MeltResult::m_oPipeOutAdditionalMaterialHeightName 	( std::string("MeltResultAdditionalMaterialHeightOut") );	///< Pipe: Scalar out-pipe.


MeltResult::MeltResult() :
	TransformFilter			( m_oFilterName, Poco::UUID{"FD1406FF-04B2-4C11-9F31-524B502890FA"} ),
	m_pPipeInBlobSmall		( nullptr ),
	m_pPipeInBlobLarge		( nullptr ),
	m_pPipeInCircleMiddleX  ( nullptr ),
	m_pPipeInCircleMiddleY  ( nullptr ),
	m_pPipeInCircleRadius   ( nullptr ),
	m_oPipeOutPenetrationIndex		( this, m_oPipeOutPenetrationIndexName ),
	m_oPipeOutSeamPosition			( this, m_oPipeOutSeamPositionName ),
	m_oPipeOutSeamAsymmetry			( this, m_oPipeOutSeamAsymmetryName ),
	m_oPipeOutAdditionalMaterialWidth	( this, m_oPipeOutAdditionalMaterialWidthName ),
	m_oPipeOutAdditionalMaterialHeight	( this, m_oPipeOutAdditionalMaterialHeightName )
{
		m_additionalMaterialY = 0;
		m_isValid = true;

    setInPipeConnectors({{Poco::UUID("8142B9F2-60B6-49F1-82EE-A7A6B561C178"), m_pPipeInBlobSmall, "BlobsSmallIn", 1, "PoreSmall"},
    {Poco::UUID("794A75FE-08A7-4547-A6FF-DC015E76367F"), m_pPipeInBlobLarge, "BlobsLargeIn", 1, "PoreLarge"},
    {Poco::UUID("95F4CB66-15C5-4444-BBD0-08055719FA31"), m_pPipeInCircleMiddleX, "CircleXIn", 1, "CircleCenterX"},
    {Poco::UUID("1FA55DE2-5114-449B-9B7C-6740C817B2D5"), m_pPipeInCircleMiddleY, "CircleYIn", 1, "CircleCenterY"},
    {Poco::UUID("DCBF3037-78DE-4079-A448-99E0C9EC88A1"), m_pPipeInCircleRadius, "CircleRadiusIn", 1, "CircleRadius"}});
    setOutPipeConnectors({{Poco::UUID("F9B42938-C9E6-48BA-A633-557D6FC9324E"), &m_oPipeOutPenetrationIndex, m_oPipeOutPenetrationIndexName, 0, ""},
    {Poco::UUID("F29D55B1-0030-4DAA-8384-F011CE14F3EF"), &m_oPipeOutSeamPosition, m_oPipeOutSeamPositionName, 0, ""},
    {Poco::UUID("640FC499-078B-46AE-973B-CD8B798E27A3"), &m_oPipeOutSeamAsymmetry, m_oPipeOutSeamAsymmetryName, 0, ""},
    {Poco::UUID("0CF9F9B0-AE74-4EEF-9766-5B6D24ED3E60"), &m_oPipeOutAdditionalMaterialWidth, m_oPipeOutAdditionalMaterialWidthName, 0, ""},
    {Poco::UUID("4D9F9460-B726-4A32-B6CA-DBDFEA263AFA"), &m_oPipeOutAdditionalMaterialHeight, m_oPipeOutAdditionalMaterialHeightName, 0, ""}});
    setVariantID(Poco::UUID("C9134361-E414-4988-BC7E-5342233888D4"));
}

void MeltResult::setParameter()
{
	TransformFilter::setParameter();
}

void MeltResult::paint()
{
	if ((m_oVerbosity <= eNone))
	{
		return;
	}

	if (!m_isValid) return; //kein sinnvolles Ergebnis => nix zeichnen

	if (m_oSpTrafo.isNull())
    {
        return;
    }
	try
	{
		const Trafo					&rTrafo(*m_oSpTrafo);
		OverlayCanvas				&rCanvas(canvas<OverlayCanvas>(m_oCounter));
		OverlayLayer				&rLayerContour(rCanvas.getLayerContour());
		OverlayLayer				&rLayerPosition(rCanvas.getLayerPosition());

		if (m_oVerbosity >= eLow)
		{
			const Point		oPoint1((int)m_weldingBathCenterOfMassX, (int)m_weldingBathCenterOfMassY);
			rLayerPosition.add(new  OverlayCross(rTrafo(oPoint1), Color::Red()));

			const Point		oPoint2((int)m_keyholeCenterOfMassX, (int)m_keyholeCenterOfMassY);
			rLayerPosition.add(new  OverlayCross(rTrafo(oPoint2), Color::Orange()));

			const Point		oPoint3((int)m_additionalMaterialX, (int)m_additionalMaterialY);
			rLayerPosition.add(new  OverlayCross(rTrafo(oPoint3), Color::Yellow()));

			// Rechteck vom Zusatzwerkstoff
			int width = (int)(std::abs(m_additionalMaterialMaxX - m_additionalMaterialMinX)+0.5);
			int height = (int)(std::abs(m_additionalMaterialMaxY - m_additionalMaterialMinY)+0.5);
			Rect rec1((int)(m_additionalMaterialMinX+0.5), (int)(m_additionalMaterialMinY+0.5), width, height);
			if (_noseFound) rLayerPosition.add(new  OverlayRectangle(rTrafo(rec1), Color::Green()));

			// Rechteck vom Schmelzbad
			width = (int)(0.5 + m_keyholeCenterOfMassX + m_rGeoDoubleArrayInR.ref().getData()[0] - m_rGeoBlobsLargeIn.ref().getData()[0].xmin);
			height = m_rGeoBlobsLargeIn.ref().getData()[0].ymax - m_rGeoBlobsLargeIn.ref().getData()[0].ymin;
			Rect rec2(m_rGeoBlobsLargeIn.ref().getData()[0].xmin, m_rGeoBlobsLargeIn.ref().getData()[0].ymin, width, height);
			rLayerPosition.add(new  OverlayRectangle(rTrafo(rec2), Color::Blue()));

			// Gefundener Kreis
			for (int angle = 0; angle < 360; angle += 2)
			{
				double angleRad = angle * 3.141592653589793 / 180.0;
				Point point((int)(0.4 + m_rGeoDoubleArrayInX.ref().getData()[0] + m_rGeoDoubleArrayInR.ref().getData()[0] * cos(angleRad)),
					(int)(0.4 + m_rGeoDoubleArrayInY.ref().getData()[0] + m_rGeoDoubleArrayInR.ref().getData()[0] * sin(angleRad)));
				rLayerContour.add(new OverlayPoint(rTrafo(point), Color::Orange()));
			}
		} // if
	}
	catch(...)
	{
		return;
	}
} // paint

bool MeltResult::subscribe(BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.tag() == "PoreSmall" )
		m_pPipeInBlobSmall = dynamic_cast<blob_pipe_t*>(&p_rPipe);
	if ( p_rPipe.tag() == "PoreLarge" )
		m_pPipeInBlobLarge = dynamic_cast<blob_pipe_t*>(&p_rPipe);

	if ( p_rPipe.tag() == "CircleCenterX" )
		m_pPipeInCircleMiddleX = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	if ( p_rPipe.tag() == "CircleCenterY" )
		m_pPipeInCircleMiddleY = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	if ( p_rPipe.tag() == "CircleRadius" )
		m_pPipeInCircleRadius  = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe

void MeltResult::proceedGroup(const void* p_pSender, PipeGroupEventArgs&)
{
	m_isValid = true;
	_noseFound = false;

	geo2d::Doublearray oOutPenetrationIndex (1);
	geo2d::Doublearray oOutSeamPosition (1);
	geo2d::Doublearray oOutSeamAsymmetry (1);
	geo2d::Doublearray oOutAdditionalMaterialWidth (1);
	geo2d::Doublearray oOutAdditionalMaterialHeight (1);

	m_rGeoBlobsSmallIn = m_pPipeInBlobSmall->read(m_oCounter);

	try
	{
		if (m_pPipeInBlobSmall == nullptr) m_isValid = false;
		if (m_pPipeInBlobLarge == nullptr) m_isValid = false;
		if (m_pPipeInCircleMiddleX == nullptr) m_isValid = false;
		if (m_pPipeInCircleMiddleY == nullptr) m_isValid = false;
		if (m_pPipeInCircleRadius == nullptr) m_isValid = false;

		poco_assert_dbg(m_pPipeInBlobSmall != nullptr);
		poco_assert_dbg(m_pPipeInBlobLarge != nullptr);
		poco_assert_dbg(m_pPipeInCircleMiddleX != nullptr);
		poco_assert_dbg(m_pPipeInCircleMiddleY != nullptr);
		poco_assert_dbg(m_pPipeInCircleRadius != nullptr);

		m_rGeoBlobsLargeIn = m_pPipeInBlobLarge->read(m_oCounter);
		m_rGeoDoubleArrayInX = m_pPipeInCircleMiddleX->read(m_oCounter);
		m_rGeoDoubleArrayInY = m_pPipeInCircleMiddleY->read(m_oCounter);
		m_rGeoDoubleArrayInR = m_pPipeInCircleRadius->read(m_oCounter);

		if (m_rGeoBlobsSmallIn.ref().getData().size() <= 0) m_isValid = false;
		if (m_rGeoBlobsLargeIn.ref().getData().size() <= 0) m_isValid = false;
		if (m_rGeoDoubleArrayInX.ref().getData().size() <= 0) m_isValid = false;
		if (m_rGeoDoubleArrayInY.ref().getData().size() <= 0) m_isValid = false;
		if (m_rGeoDoubleArrayInR.ref().getData().size() <= 0) m_isValid = false;

		if (m_rGeoBlobsSmallIn.rank() <= 0) m_isValid = false;
		if (m_rGeoBlobsLargeIn.rank() <= 0) m_isValid = false;
		if (m_rGeoDoubleArrayInX.rank() <= 0) m_isValid = false;
		if (m_rGeoDoubleArrayInY.rank() <= 0) m_isValid = false;
		if (m_rGeoDoubleArrayInR.rank() <= 0) m_isValid = false;

		m_oSpTrafo = m_rGeoBlobsSmallIn.context().trafo();

		geo2d::Doublearray oOutWeldingBathLength(1);
		oOutWeldingBathLength.getData()[0] = calcWeldingBathLength();
		oOutWeldingBathLength.getRank()[0] = 255;

		geo2d::Doublearray oOutWeldingBathArea(1);
		oOutWeldingBathArea.getData()[0] = calcWeldingBathArea();
		oOutWeldingBathArea.getRank()[0] = 255;

		geo2d::Doublearray oOutWeldingBathCenterOfMassX(1);
		oOutWeldingBathCenterOfMassX.getData()[0] = calcWeldingBathCenterOfMassX();
		oOutWeldingBathCenterOfMassX.getRank()[0] = 255;

		geo2d::Doublearray oOutWeldingBathCenterOfMassY(1);
		oOutWeldingBathCenterOfMassY.getData()[0] = calcWeldingBathCenterOfMassY();
		oOutWeldingBathCenterOfMassY.getRank()[0] = 255;

		geo2d::Doublearray oOutKeyholeCenterOfMassX(1);
		oOutKeyholeCenterOfMassX.getData()[0] = calcKeyholeCenterOfMassX();
		oOutKeyholeCenterOfMassX.getRank()[0] = 255;

		geo2d::Doublearray oOutKeyholeCenterOfMassY(1);
		oOutKeyholeCenterOfMassY.getData()[0] = calcKeyholeCenterOfMassY();
		oOutKeyholeCenterOfMassY.getRank()[0] = 255;

		geo2d::Doublearray oOutSeamWidth(1);
		oOutSeamWidth.getData()[0] = calcSeamWidth();
		oOutSeamWidth.getRank()[0] = 255;

		oOutPenetrationIndex.getData()[0] = calcSeamDepth();
		oOutPenetrationIndex.getRank()[0] = 255;

		calcAdditionalMaterialY();

		oOutSeamPosition.getData()[0] = calcSeamLayer();
		oOutSeamPosition.getRank()[0] = 255;

		oOutSeamAsymmetry.getData()[0] = calcSeamAsymmetry();
		oOutSeamAsymmetry.getRank()[0] = 255;

		int width = (int)(std::abs(m_additionalMaterialMaxX - m_additionalMaterialMinX)+0.5);
		int height = (int)(std::abs(m_additionalMaterialMaxY - m_additionalMaterialMinY)+0.5);

		oOutAdditionalMaterialWidth.getData()[0] = (_noseFound) ? width : 0.0; // <--- Diff zum Qualas
		oOutAdditionalMaterialWidth.getRank()[0] = 255;

		oOutAdditionalMaterialHeight.getData()[0] = (_noseFound) ? height : 0.0; // <--- Diff zum Qualas
		oOutAdditionalMaterialHeight.getRank()[0] = 255;

		// send the data out ...

		double rank = (m_isValid) ? 1.0 : 0.0;

		const interface::GeoDoublearray oGeoDoubleOutPenetrationIndex(m_rGeoDoubleArrayInX.context(), oOutPenetrationIndex, m_rGeoBlobsSmallIn.analysisResult(), rank);
		m_oPipeOutPenetrationIndex.signal(oGeoDoubleOutPenetrationIndex);
		const interface::GeoDoublearray oGeoDoubleOutSeamPosition(m_rGeoDoubleArrayInX.context(), oOutSeamPosition, m_rGeoBlobsSmallIn.analysisResult(), rank);
		m_oPipeOutSeamPosition.signal(oGeoDoubleOutSeamPosition);
		const interface::GeoDoublearray oGeoDoubleOutSeamAsymmetry(m_rGeoDoubleArrayInX.context(), oOutSeamAsymmetry, m_rGeoBlobsSmallIn.analysisResult(), rank);
		m_oPipeOutSeamAsymmetry.signal(oGeoDoubleOutSeamAsymmetry);
		const interface::GeoDoublearray oGeoDoubleOutAdditionalMaterialWidth(m_rGeoDoubleArrayInX.context(), oOutAdditionalMaterialWidth, m_rGeoBlobsSmallIn.analysisResult(), rank);
		m_oPipeOutAdditionalMaterialWidth.signal(oGeoDoubleOutAdditionalMaterialWidth);
		const interface::GeoDoublearray oGeoDoubleOutAdditionalMaterialHeight(m_rGeoDoubleArrayInX.context(), oOutAdditionalMaterialHeight, m_rGeoBlobsSmallIn.analysisResult(), rank);
		m_oPipeOutAdditionalMaterialHeight.signal(oGeoDoubleOutAdditionalMaterialHeight);

		preSignalAction();
	}
	catch(...)
	{
		oOutPenetrationIndex.getData()[0] = 0;
		oOutPenetrationIndex.getRank()[0] = 0;
		oOutSeamPosition.getData()[0] = 0;
		oOutSeamPosition.getRank()[0] = 0;
		oOutSeamAsymmetry.getData()[0] = 0;
		oOutSeamAsymmetry.getRank()[0] = 0;
		oOutAdditionalMaterialWidth.getData()[0] = 0;
		oOutAdditionalMaterialWidth.getRank()[0] = 0;
		oOutAdditionalMaterialHeight.getData()[0] = 0;
		oOutAdditionalMaterialHeight.getRank()[0] = 0;
		preSignalAction();
		const interface::GeoDoublearray oGeoDoubleOutPenetrationIndex(m_rGeoBlobsSmallIn.context(), oOutPenetrationIndex, m_rGeoBlobsSmallIn.analysisResult(), interface::NotPresent);
		m_oPipeOutPenetrationIndex.signal(oGeoDoubleOutPenetrationIndex);
		const interface::GeoDoublearray oGeoDoubleOutSeamPosition(m_rGeoBlobsSmallIn.context(), oOutSeamPosition, m_rGeoBlobsSmallIn.analysisResult(), interface::NotPresent);
		m_oPipeOutSeamPosition.signal(oGeoDoubleOutSeamPosition);
		const interface::GeoDoublearray oGeoDoubleOutSeamAsymmetry(m_rGeoBlobsSmallIn.context(), oOutSeamAsymmetry, m_rGeoBlobsSmallIn.analysisResult(), interface::NotPresent);
		m_oPipeOutSeamAsymmetry.signal(oGeoDoubleOutSeamAsymmetry);
		const interface::GeoDoublearray oGeoDoubleOutAdditionalMaterialWidth(m_rGeoBlobsSmallIn.context(), oOutAdditionalMaterialWidth, m_rGeoBlobsSmallIn.analysisResult(), interface::NotPresent);
		m_oPipeOutAdditionalMaterialWidth.signal(oGeoDoubleOutAdditionalMaterialWidth);
		const interface::GeoDoublearray oGeoDoubleOutAdditionalMaterialHeight(m_rGeoBlobsSmallIn.context(), oOutAdditionalMaterialHeight, m_rGeoBlobsSmallIn.analysisResult(), interface::NotPresent);
		m_oPipeOutAdditionalMaterialHeight.signal(oGeoDoubleOutAdditionalMaterialHeight);
	}
} // proceed


////////// Berechnungsfunktionen


double MeltResult::calcWeldingBathArea()
{
	if (!m_isValid) return 0.0;
	m_weldingBathArea = m_rGeoBlobsLargeIn.ref().getData()[0].npix;
	return m_weldingBathArea;
}

double MeltResult::calcWeldingBathLength()
{
	if (!m_isValid) return 0.0;

	int left  = m_rGeoBlobsLargeIn.ref().getData()[0].xmin;
	int right = m_rGeoBlobsSmallIn.ref().getData()[0].xmax;

	m_weldingBathLength = right - left;
	return m_weldingBathLength;
}

double MeltResult::calcWeldingBathCenterOfMassX()
{
	if (!m_isValid) return 0.0;
	if (m_rGeoBlobsLargeIn.ref().getData()[0].si == 0)
	{
		m_isValid = false;
		return 0.0;
	}

	m_weldingBathCenterOfMassX = (double)(m_rGeoBlobsLargeIn.ref().getData()[0].sx / m_rGeoBlobsLargeIn.ref().getData()[0].si);
	return m_weldingBathCenterOfMassX;
}

double MeltResult::calcWeldingBathCenterOfMassY()
{ // aus Porenanalyse
	if (!m_isValid) return 0.0;
	if (m_rGeoBlobsLargeIn.ref().getData()[0].si == 0)
	{
		m_isValid = false;
		return 0.0;
	}

	m_weldingBathCenterOfMassY = (double)(m_rGeoBlobsLargeIn.ref().getData()[0].sy / m_rGeoBlobsLargeIn.ref().getData()[0].si);
	return m_weldingBathCenterOfMassY;
}

double MeltResult::calcKeyholeCenterOfMassX()
{ // aus CircleFit
	if (!m_isValid) return 0.0;

	m_keyholeCenterOfMassX = m_rGeoDoubleArrayInX.ref().getData()[0];
	return m_keyholeCenterOfMassX;
}

double MeltResult::calcKeyholeCenterOfMassY()
{ // aus CircleFit
	if (!m_isValid) return 0.0;

	m_keyholeCenterOfMassY = m_rGeoDoubleArrayInY.ref().getData()[0];
	return m_keyholeCenterOfMassY;
}

double MeltResult::calcSeamWidth()
{ // aus Porenanalyse
	if (!m_isValid) return 0.0;

	m_seamWidth = m_rGeoBlobsLargeIn.ref().getData()[0].ymax - m_rGeoBlobsLargeIn.ref().getData()[0].ymin;
	return m_seamWidth;
}

double MeltResult::calcSeamDepth()
{
	if (!m_isValid) return 0.0;

	double denominator = m_weldingBathCenterOfMassX - m_keyholeCenterOfMassX;
	if (denominator == 0)
	{
		m_isValid = false;
		return 0.0;
	}
	if (denominator < 0) denominator = -denominator;
	m_seamDepth = m_weldingBathArea / denominator;
	return m_seamDepth;
}

void MeltResult::calcAdditionalMaterialY()
{
	m_additionalMaterialX = 0.0;
	m_additionalMaterialY = 0.0;
	m_additionalMaterialMinX = 0.0;
	m_additionalMaterialMaxX = 0.0;
	m_additionalMaterialMinY = 0.0;
	m_additionalMaterialMaxY = 0.0;

	if (!m_isValid) return;

	int numberOfContourPoints = m_rGeoBlobsSmallIn.ref().getData()[0].m_oContour.size();

	if (numberOfContourPoints <= 1)
	{
		m_isValid = false;
		return;
	}

	int maxX=0, maxY=0;
	int curX, curY, sumY=0, countY=0;
	for (int i=0; i<numberOfContourPoints; i++)
	{
		curX = m_rGeoBlobsSmallIn.ref().getData()[0].m_oContour[i].x;
		curY = m_rGeoBlobsSmallIn.ref().getData()[0].m_oContour[i].y;

		if (curX == maxX)
		{
			sumY += curY;
			countY += 1;
		}

		if (curX > maxX)
		{
			maxX = curX;
			sumY = curY;
			countY = 1;
		}

	}

	if (countY > 0)
	{
		maxY = sumY / countY;
	}
	else
	{
		m_isValid = false;
	}

	int circleMaxX = (int)(m_rGeoDoubleArrayInX.ref().getData()[0] + m_rGeoDoubleArrayInR.ref().getData()[0]); // X-Wert vom Mittelpunkt plus Radius

	m_additionalMaterialX = (maxX + circleMaxX) / 2;
	m_additionalMaterialY = maxY;

	if (circleMaxX >= maxX-1) // Dies ist der Fall, wenn der Kreis weiter nach rechts geht als der rechteste Konturpunkt => Nase eingefallen, nicht vorhanden
	{
		m_additionalMaterialMinX = m_additionalMaterialX;
		m_additionalMaterialMaxX = m_additionalMaterialX;
		m_additionalMaterialMinY = m_additionalMaterialY;
		m_additionalMaterialMaxY = m_additionalMaterialY;
		return;
	}

	// Bounding Box vom Zusatzwerkstoff
	m_additionalMaterialMinX = m_rGeoDoubleArrayInX.ref().getData()[0] + m_rGeoDoubleArrayInR.ref().getData()[0];
	m_additionalMaterialMaxX = maxX;
	m_additionalMaterialMinY = m_additionalMaterialY;
	m_additionalMaterialMaxY = m_additionalMaterialY;

	bool foundTop = false;
	bool foundBottom = false;
	bool isTop;
	for (int i = 0; i < numberOfContourPoints; i++)
	{
		curX = m_rGeoBlobsSmallIn.ref().getData()[0].m_oContour[i].x;
		curY = m_rGeoBlobsSmallIn.ref().getData()[0].m_oContour[i].y;

		if ( (int)(m_additionalMaterialX) == curX ) // Konturpunkt gefunden, der x-Koordinate hat
		{
			isTop = (m_additionalMaterialY - curY) > 0; // liegt der gefundene Punkt ueber oder unter dem Zusatzwerkstoffmittelpunkt?

			if (isTop && !foundTop) // ist oberhalb und oberhalb war noch keiner
			{
				m_additionalMaterialMinY = curY;
				foundTop = true;
			}

			if (!isTop && !foundBottom) // ist unterhalb und unterhalb war noch keiner
			{
				m_additionalMaterialMaxY = curY;
				foundBottom = true;
			}
		}
	}

	if (!foundTop && foundBottom)
	{
		m_additionalMaterialMinY = m_additionalMaterialMaxY - 1;
	}
	if (foundTop && !foundBottom)
	{
		m_additionalMaterialMaxY = m_additionalMaterialMinY + 1;
	}
	_noseFound = true;
}

double MeltResult::calcSeamLayer()
{
	if (!m_isValid) return 0.0;
	if (m_additionalMaterialY == 0) return 0.0;

	m_seamLayer = m_keyholeCenterOfMassY - m_additionalMaterialY;
	return m_seamLayer;
}

double MeltResult::calcSeamAsymmetry()
{
	if (m_seamWidth == 0) m_isValid = false;
	if (!m_isValid) return 0.0;
	if (m_additionalMaterialY == 0) return 0.0;

	double weldingBathMiddleY =  (m_rGeoBlobsLargeIn.ref().getData()[0].ymax + m_rGeoBlobsLargeIn.ref().getData()[0].ymin) / 2.0;

	m_seamAsymmetry = (m_additionalMaterialY - weldingBathMiddleY) / m_seamWidth;
	return m_seamAsymmetry;
}

} // namespace filter
} // namespace precitec
