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

#include "resultQualas.h"


using namespace fliplib;
namespace precitec {
	using namespace geo2d;
	using namespace interface;
	using namespace image;
namespace filter {


const std::string ResultQualas::m_oFilterName 							( std::string("ResultQualas") );						///< Filter name.
const std::string ResultQualas::m_oPipeOutPenetrationIndexName 			( std::string("ResultQualasPenetrationIndexOut") );		///< Pipe: Scalar out-pipe.
const std::string ResultQualas::m_oPipeOutSeamPositionName 				( std::string("ResultQualasSeamPositionOut") );			///< Pipe: Scalar out-pipe.
const std::string ResultQualas::m_oPipeOutSeamAsymmetryName 			( std::string("ResultQualasSeamAsymmetryOut") );		///< Pipe: Scalar out-pipe.
const std::string ResultQualas::m_oPipeOutAdditionalMaterialXName 		( std::string("ResultQualasAdditionalMaterialXOut") );	///< Pipe: Scalar out-pipe.
const std::string ResultQualas::m_oPipeOutAdditionalMaterialYName 		( std::string("ResultQualasAdditionalMaterialYOut") );	///< Pipe: Scalar out-pipe.


ResultQualas::ResultQualas() :
	TransformFilter			( m_oFilterName, Poco::UUID{"D8011573-9A2B-4B10-B821-75CDB284743E"} ),
	m_pPipeInBlobSmall		( nullptr ),
	m_pPipeInBlobLarge		( nullptr ),
	m_pPipeInCircleMiddleX  ( nullptr ),
	m_pPipeInCircleMiddleY  ( nullptr ),
	m_pPipeInCircleRadius   ( nullptr ),
	m_oPipeOutPenetrationIndex		( this, m_oPipeOutPenetrationIndexName ),
	m_oPipeOutSeamPosition			( this, m_oPipeOutSeamPositionName ),
	m_oPipeOutSeamAsymmetry			( this, m_oPipeOutSeamAsymmetryName ),
	m_oPipeOutAdditionalMaterialX	( this, m_oPipeOutAdditionalMaterialXName ),
	m_oPipeOutAdditionalMaterialY	( this, m_oPipeOutAdditionalMaterialYName )
{
		m_additionalMaterialY = 0;
		m_isValid = true;

        setInPipeConnectors({{Poco::UUID("56AAB0AC-9341-4376-B066-79D050EA3949"), m_pPipeInBlobSmall, "BlobsSmallIn", 1, "PoreSmall"},
        {Poco::UUID("EB6443B9-2E63-40F6-A214-24FFB29688B8"), m_pPipeInBlobLarge, "BlobsLargeIn", 1, "PoreLarge"},
        {Poco::UUID("338A8D1F-A06D-4007-A29E-E0A1E3154296"), m_pPipeInCircleMiddleX, "CircleXIn", 1, "CircleCenterX"},
        {Poco::UUID("D7D8D700-C091-46C5-B513-B16F0D0AFB83"), m_pPipeInCircleMiddleY, "CircleYIn", 1, "CircleCenterY"},
        {Poco::UUID("21A1A4BE-E53C-4698-999C-215F8ECB9BCE"), m_pPipeInCircleRadius, "CircleRadiusIn", 1, "CircleRadius"}});
        setOutPipeConnectors({{Poco::UUID("13C3BE0D-7601-4135-9DE0-C0D693487234"), &m_oPipeOutPenetrationIndex, m_oPipeOutPenetrationIndexName, 0, ""},
        {Poco::UUID("320E9E6F-EF49-4F3B-B463-B0D6658AFF3A"), &m_oPipeOutSeamPosition, m_oPipeOutSeamPositionName, 0, ""},
        {Poco::UUID("330AFB4A-1AD1-4865-86F3-2D1ED0F79BFA"), &m_oPipeOutSeamAsymmetry, m_oPipeOutSeamAsymmetryName, 0, ""},
        {Poco::UUID("5A8F429A-4A4F-4ABA-9C23-092FBCCB0981"), &m_oPipeOutAdditionalMaterialX, m_oPipeOutAdditionalMaterialXName, 0, ""},
        {Poco::UUID("F208B3A0-515A-4BF0-A79E-E2F97F11357D"), &m_oPipeOutAdditionalMaterialY, m_oPipeOutAdditionalMaterialYName, 0, ""}});
        setVariantID(Poco::UUID("E7540F22-1143-4B36-85E7-8F152531933D"));
}

void ResultQualas::setParameter()
{
	TransformFilter::setParameter();
}

void ResultQualas::paint()
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
		//OverlayLayer				&rLayerImage(rCanvas.getLayerImage());
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
			rLayerPosition.add(new  OverlayRectangle(rTrafo(rec1), Color::Green()));

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

bool ResultQualas::subscribe(BasePipe& p_rPipe, int p_oGroup)
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

void ResultQualas::proceedGroup(const void* p_pSender, PipeGroupEventArgs&)
{
	m_isValid = true;

	geo2d::Doublearray oOutPenetrationIndex (1);
	geo2d::Doublearray oOutSeamPosition (1);
	geo2d::Doublearray oOutSeamAsymmetry (1);
	geo2d::Doublearray oOutAdditionalMaterialX (1);
	geo2d::Doublearray oOutAdditionalMaterialY (1);

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

		oOutAdditionalMaterialX.getData()[0] = m_additionalMaterialX;
		oOutAdditionalMaterialX.getRank()[0] = 255;

		oOutAdditionalMaterialY.getData()[0] = m_additionalMaterialY;
		oOutAdditionalMaterialY.getRank()[0] = 255;

		// send the data out ...

		double rank = (m_isValid) ? 1.0 : 0.0;

		const interface::GeoDoublearray oGeoDoubleOutPenetrationIndex(m_rGeoDoubleArrayInX.context(), oOutPenetrationIndex, m_rGeoBlobsSmallIn.analysisResult(), rank);
		const interface::GeoDoublearray oGeoDoubleOutSeamPosition(m_rGeoDoubleArrayInX.context(), oOutSeamPosition, m_rGeoBlobsSmallIn.analysisResult(), rank);
		const interface::GeoDoublearray oGeoDoubleOutSeamAsymmetry(m_rGeoDoubleArrayInX.context(), oOutSeamAsymmetry, m_rGeoBlobsSmallIn.analysisResult(), rank);
		const interface::GeoDoublearray oGeoDoubleOutAdditionalMaterialX(m_rGeoDoubleArrayInX.context(), oOutAdditionalMaterialX, m_rGeoBlobsSmallIn.analysisResult(), rank);
		const interface::GeoDoublearray oGeoDoubleOutAdditionalMaterialY(m_rGeoDoubleArrayInX.context(), oOutAdditionalMaterialY, m_rGeoBlobsSmallIn.analysisResult(), rank);

        preSignalAction();
        m_oPipeOutPenetrationIndex.signal(oGeoDoubleOutPenetrationIndex);
        m_oPipeOutSeamPosition.signal(oGeoDoubleOutSeamPosition);
        m_oPipeOutSeamAsymmetry.signal(oGeoDoubleOutSeamAsymmetry);
        m_oPipeOutAdditionalMaterialX.signal(oGeoDoubleOutAdditionalMaterialX);
        m_oPipeOutAdditionalMaterialY.signal(oGeoDoubleOutAdditionalMaterialY);
	}
	catch(...)
	{
		oOutPenetrationIndex.getData()[0] = 0;
		oOutPenetrationIndex.getRank()[0] = 0;
		oOutSeamPosition.getData()[0] = 0;
		oOutSeamPosition.getRank()[0] = 0;
		oOutSeamAsymmetry.getData()[0] = 0;
		oOutSeamAsymmetry.getRank()[0] = 0;
		oOutAdditionalMaterialX.getData()[0] = 0;
		oOutAdditionalMaterialX.getRank()[0] = 0;
		oOutAdditionalMaterialY.getData()[0] = 0;
		oOutAdditionalMaterialY.getRank()[0] = 0;

		const interface::GeoDoublearray oGeoDoubleOutPenetrationIndex(m_rGeoBlobsSmallIn.context(), oOutPenetrationIndex, m_rGeoBlobsSmallIn.analysisResult(), interface::NotPresent);
        const interface::GeoDoublearray oGeoDoubleOutSeamPosition(m_rGeoBlobsSmallIn.context(), oOutSeamPosition, m_rGeoBlobsSmallIn.analysisResult(), interface::NotPresent);
		const interface::GeoDoublearray oGeoDoubleOutSeamAsymmetry(m_rGeoBlobsSmallIn.context(), oOutSeamAsymmetry, m_rGeoBlobsSmallIn.analysisResult(), interface::NotPresent);
        const interface::GeoDoublearray oGeoDoubleOutAdditionalMaterialX(m_rGeoBlobsSmallIn.context(), oOutAdditionalMaterialX, m_rGeoBlobsSmallIn.analysisResult(), interface::NotPresent);
        const interface::GeoDoublearray oGeoDoubleOutAdditionalMaterialY(m_rGeoBlobsSmallIn.context(), oOutAdditionalMaterialY, m_rGeoBlobsSmallIn.analysisResult(), interface::NotPresent);

        preSignalAction();
		m_oPipeOutPenetrationIndex.signal(oGeoDoubleOutPenetrationIndex);
		m_oPipeOutSeamPosition.signal(oGeoDoubleOutSeamPosition);
		m_oPipeOutSeamAsymmetry.signal(oGeoDoubleOutSeamAsymmetry);
		m_oPipeOutAdditionalMaterialX.signal(oGeoDoubleOutAdditionalMaterialX);
		m_oPipeOutAdditionalMaterialY.signal(oGeoDoubleOutAdditionalMaterialY);
	}
} // proceed


////////// Berechnungsfunktionen


double ResultQualas::calcWeldingBathArea()
{
	if (!m_isValid) return 0.0;
	m_weldingBathArea = m_rGeoBlobsLargeIn.ref().getData()[0].npix;
	return m_weldingBathArea;
}

double ResultQualas::calcWeldingBathLength()
{
	if (!m_isValid) return 0.0;

	int left  = m_rGeoBlobsLargeIn.ref().getData()[0].xmin;
	int right = m_rGeoBlobsSmallIn.ref().getData()[0].xmax;

	m_weldingBathLength = right - left;
	return m_weldingBathLength;
}

double ResultQualas::calcWeldingBathCenterOfMassX()
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

double ResultQualas::calcWeldingBathCenterOfMassY()
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

double ResultQualas::calcKeyholeCenterOfMassX()
{ // aus CircleFit
	if (!m_isValid) return 0.0;

	m_keyholeCenterOfMassX = m_rGeoDoubleArrayInX.ref().getData()[0];
	return m_keyholeCenterOfMassX;
}

double ResultQualas::calcKeyholeCenterOfMassY()
{ // aus CircleFit
	if (!m_isValid) return 0.0;

	m_keyholeCenterOfMassY = m_rGeoDoubleArrayInY.ref().getData()[0];
	return m_keyholeCenterOfMassY;
}

double ResultQualas::calcSeamWidth()
{ // aus Porenanalyse
	if (!m_isValid) return 0.0;

	m_seamWidth = m_rGeoBlobsLargeIn.ref().getData()[0].ymax - m_rGeoBlobsLargeIn.ref().getData()[0].ymin;
	return m_seamWidth;
}

double ResultQualas::calcSeamDepth()
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

void ResultQualas::calcAdditionalMaterialY()
{
	if (!m_isValid)
	{
		m_additionalMaterialX = 0.0;
	    m_additionalMaterialY = 0.0;

		return;
	}

	int numberOfContourPoints = m_rGeoBlobsSmallIn.ref().getData()[0].m_oContour.size();

	if (numberOfContourPoints <= 1)
	{
		m_additionalMaterialX = 0.0;
	    m_additionalMaterialY = 0.0;
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

	m_additionalMaterialX = (maxX + m_rGeoDoubleArrayInX.ref().getData()[0] + m_rGeoDoubleArrayInR.ref().getData()[0]) / 2;
	m_additionalMaterialY = maxY;

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

	if (!foundTop || foundBottom)
	{
		m_additionalMaterialMinY = m_additionalMaterialMaxY;
	}
	if (foundTop || !foundBottom)
	{
		m_additionalMaterialMaxY = m_additionalMaterialMinY;
	}
}

double ResultQualas::calcSeamLayer()
{
	if (!m_isValid) return 0.0;
	if (m_additionalMaterialY == 0) return 0.0;

	m_seamLayer = m_keyholeCenterOfMassY - m_additionalMaterialY;
	return m_seamLayer;
}

double ResultQualas::calcSeamAsymmetry()
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
