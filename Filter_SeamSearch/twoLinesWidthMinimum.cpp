/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		2016
 * 	@brief 		This filter gets a minimum and maximum seam width and looks for a minimum in the 2 laserline width curves
 */

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include <filter/algoArray.h>
#include "module/moduleLogger.h"
// local includes
#include "twoLinesWidthMinimum.h"
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string TwoLinesWidthMinimum::m_oFilterName 		= std::string("TwoLinesWidthMinimum");
const std::string TwoLinesWidthMinimum::PIPENAME_SEAMPOS_OUT	= std::string("SeamPositionOut");
const std::string TwoLinesWidthMinimum::PIPENAME_SEAMLEFT_OUT	= std::string("SeamLeftOut");
const std::string TwoLinesWidthMinimum::PIPENAME_SEAMRIGHT_OUT	= std::string("SeamRightOut");


TwoLinesWidthMinimum::TwoLinesWidthMinimum() :
	TransformFilter( TwoLinesWidthMinimum::m_oFilterName, Poco::UUID{"BB87837B-FEA9-4A17-91D5-532F463ACA99"} ),
	m_pPipeInFirstLaserLine( NULL ),
	m_pPipeInSecondLaserLine( NULL ),
	m_pPipeInSeamWidthMin( NULL ),
	m_pPipeInSeamWidthMax( NULL )
{
	m_pPipeOutSeamPos = new SynchronePipe< interface::GeoDoublearray > ( this, TwoLinesWidthMinimum::PIPENAME_SEAMPOS_OUT );
	m_pPipeOutSeamLeft = new SynchronePipe< interface::GeoDoublearray > ( this, TwoLinesWidthMinimum::PIPENAME_SEAMLEFT_OUT );
	m_pPipeOutSeamRight = new SynchronePipe< interface::GeoDoublearray > ( this, TwoLinesWidthMinimum::PIPENAME_SEAMRIGHT_OUT );

	// Set default values of the parameters of the filter
	parameters_.add("Mode",    Parameter::TYPE_int, m_oMode);
	parameters_.add("Resolution",    Parameter::TYPE_int, m_oResolution);

    setInPipeConnectors({{Poco::UUID("97512344-15F3-4C52-B152-EDBA4CEA7957"), m_pPipeInFirstLaserLine, "LineWidth1In", 1, "LineWidth1"},
    {Poco::UUID("B824FCD8-6572-4FCC-AA4B-EDE68E6E51CF"), m_pPipeInSecondLaserLine, "LineWidth2In", 1, "LineWidth2"},
    {Poco::UUID("CEEE1F82-F4EA-49A6-923D-F6B18B8DBE3E"), m_pPipeInSeamWidthMin, "SeamWidthMinIn", 1, "SeamWidthMin"},
    {Poco::UUID("E41A0316-CF13-4AC6-AC03-419CADDDD349"), m_pPipeInSeamWidthMax, "SeamWidthMaxIn", 1, "SeamWidthMax"}});
    setOutPipeConnectors({{Poco::UUID("5AEC02BA-A996-4A5F-8C2B-FE6517CDB7E7"), m_pPipeOutSeamPos, "PIPENAME_SEAMPOS_OUT", 0, ""},
    {Poco::UUID("910775CB-B054-4DA2-9A9E-77885FFED4FF"), m_pPipeOutSeamLeft, PIPENAME_SEAMLEFT_OUT, 0, ""},
    {Poco::UUID("6EEE82AC-D3F9-4E2D-9817-F4552BBBB603"), m_pPipeOutSeamRight, PIPENAME_SEAMRIGHT_OUT, 0, ""}});
    setVariantID(Poco::UUID("7C3C415A-5F53-4777-9325-ED9CF8AB70F6"));
} // LineProfile

TwoLinesWidthMinimum::~TwoLinesWidthMinimum()
{
	delete m_pPipeOutSeamPos;
	delete m_pPipeOutSeamLeft;
	delete m_pPipeOutSeamRight;
} // ~LineProfile

void TwoLinesWidthMinimum::setParameter()
{
	TransformFilter::setParameter();
	m_oMode = parameters_.getParameter("Mode").convert<int>();
	m_oResolution = parameters_.getParameter("Resolution").convert<int>();
} // setParameter

bool TwoLinesWidthMinimum::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.tag() == "LineWidth1" )
		m_pPipeInFirstLaserLine  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);
	if ( p_rPipe.tag() == "LineWidth2" )
		m_pPipeInSecondLaserLine  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);
	if ( p_rPipe.tag() == "SeamWidthMin" )
		m_pPipeInSeamWidthMin  = dynamic_cast< SynchronePipe < GeoDoublearray > * >(&p_rPipe);
	if ( p_rPipe.tag() == "SeamWidthMax" )
		m_pPipeInSeamWidthMax  = dynamic_cast< SynchronePipe < GeoDoublearray > * >(&p_rPipe);


	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe

void TwoLinesWidthMinimum::paint()
{
	if (!m_isPaintPossible) return; // raus, wenn es nix zu malen gibt!

	if( m_oVerbosity < eLow || m_oSpFirstTrafo.isNull() || m_oSpSecondTrafo.isNull() )
	{
		m_isPaintPossible = false;
		return;
	}

	if ( (_overlappSize <= 0) || (_twoLinesContainer.getMaxValue()<=0) )
	{
		m_isPaintPossible = false;
		return;
	}

	try
	{

		const Trafo		&rFirstTrafo(*m_oSpFirstTrafo);
		const Trafo		&rSecondTrafo(*m_oSpSecondTrafo);
		OverlayCanvas	&rCanvas(canvas<OverlayCanvas>(m_oCounter));
		OverlayLayer	&rLayerContour(rCanvas.getLayerContour());

		Point startPoint1 = rFirstTrafo(Point(0, 0));
		Point startPoint2 = rSecondTrafo(Point(0, 0));

		//if ((m_resultSeamLeft1 != 0) && (m_resultSeamRight1 != 0) && (m_resultSeamPos1 != 0))
		//{
		//	rLayerContour.add(new OverlayCross(rFirstTrafo(Point(m_resultSeamLeft1, int(40))), Color::Red()));
		//	rLayerContour.add(new OverlayCross(rFirstTrafo(Point(m_resultSeamPos1, int(40))), Color::Orange()));
		//	rLayerContour.add(new OverlayCross(rFirstTrafo(Point(m_resultSeamRight1, int(40))), Color::Green()));
		//}

		//if ((m_resultSeamLeft2 != 0) && (m_resultSeamRight2 != 0) && (m_resultSeamPos2 != 0))
		//{
		//	rLayerContour.add(new OverlayCross(rSecondTrafo(Point(m_resultSeamLeft2, int(40))), Color::Red()));
		//	rLayerContour.add(new OverlayCross(rSecondTrafo(Point(m_resultSeamPos2, int(40))), Color::Orange()));
		//	rLayerContour.add(new OverlayCross(rSecondTrafo(Point(m_resultSeamRight2, int(40))), Color::Green()));
		//}

		precitec::interface::SmpTrafo smpTrafo = (_isTrafo1Crucial) ? m_oSpFirstTrafo : m_oSpSecondTrafo;
		const Trafo		&crucialTrafo(*smpTrafo);

		int diffX1 = (_isTrafo1Crucial) ? 0 : std::abs(startPoint1.x - startPoint2.x);
		int diffX2 = (_isTrafo1Crucial) ? std::abs(startPoint1.x - startPoint2.x) : 0;

		Point p1 = crucialTrafo(Point(0, 0));
		//Point p2 = crucialTrafo(p1);

		int py = (startPoint1.y+startPoint2.y) / 2;

		Point seamPointLeft = crucialTrafo(Point(m_resultSeamLeftTotal, 0));
		Point seamPointRight = crucialTrafo(Point(m_resultSeamRightTotal, 0));
		rLayerContour.add<OverlayCross>(seamPointLeft.x, py, 20, Color::Yellow());
		rLayerContour.add<OverlayCross>(seamPointLeft.x-1, py, 20, Color::Yellow());
		rLayerContour.add<OverlayCross>(seamPointRight.x, py, 20, Color::Yellow());
		rLayerContour.add<OverlayCross>(seamPointRight.x+1, py, 20, Color::Yellow());

		rLayerContour.add<OverlayCross>(p1.x, py, 5, Color::Red());
		rLayerContour.add<OverlayCross>(p1.x + _overlappSize, py, 5, Color::Red());

		for (int i=0; i<_overlappSize; i++)
		{
			int max = _twoLinesContainer.getMaxValue();
			SingleTwoLinesPoint singleTwoLinesPoint = _twoLinesContainer.getSingleTwoLinesPoint(i);

			// aus LineWidth
			int yval;
			const int yo = 20; // Malbereich in y
			const int yu = 100;

			yval = (int)(0.5 + yu - (singleTwoLinesPoint.line1Data / max) * (yu-yo));
			rLayerContour.add<OverlayPoint>(rFirstTrafo(Point(i+diffX1, yval )), Color::Red());
			yval = (int)(0.5 + yu - (singleTwoLinesPoint.line2Data / max) * (yu-yo));
			rLayerContour.add<OverlayPoint>(rSecondTrafo(Point(i+diffX2, yval )), Color::Red());

			yval = (int)(0.5 + yu - (singleTwoLinesPoint.lineSum / max) * (yu-yo));
			Point p1 = rFirstTrafo(Point(i+diffX1, yval ));
			Point p2 = rSecondTrafo(Point(i+diffX2, yval ));
			Point p  = Point( (p1.x+p2.x)/2, (p1.y+p2.y)/2 );
			rLayerContour.add<OverlayPoint>(p, Color::Red());
		}

		/*
		for (int i = 0; i < _pos1.size(), i < _pos2.size(), i < _posSum.size(); i++)
		{
			Point seamPoint1 = crucialTrafo(Point(_pos1[i], 0));
			Point seamPoint2 = crucialTrafo(Point(_pos2[i], 0));
			Point seamSumPoint = crucialTrafo(Point(_posSum[i], 0));

			rLayerContour.add(new OverlayCross(seamPoint1.x, py - 5, 10, Color::Red()));
			rLayerContour.add(new OverlayCross(seamSumPoint.x, py, 10, Color::Red()));
			rLayerContour.add(new OverlayCross(seamPoint2.x, py + 5, 10, Color::Red()));
		}
		*/


		//} // for

		// Hier gefundene Naht markieren
		//rLayerContour.add(new OverlayLine( rTrafo(Point(m_resultMinimum - m_oWindowWidth/2, 40)), rTrafo(Point(m_resultMinimum + m_oWindowWidth/2, 40)), Color::Cyan()));
		m_isPaintPossible = false;
	}
	catch(...)
	{
		m_isPaintPossible = false;
		return;
	}
} // paint

void TwoLinesWidthMinimum::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
{
	m_isPaintPossible = false;

	poco_assert_dbg(m_pPipeInFirstLaserLine != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInSecondLaserLine != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInSeamWidthMin != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInSeamWidthMax != nullptr); // to be asserted by graph editor

	m_resultSeamLeft1 = m_resultSeamRight1 = m_resultSeamPos1 = 0;
	m_resultSeamLeft2 = m_resultSeamRight2 = m_resultSeamPos2 = 0;

	const GeoDoublearray& rSeamWidthMinIn = m_pPipeInSeamWidthMin->read(m_oCounter);
	const GeoDoublearray& rSeamWidthMaxIn = m_pPipeInSeamWidthMax->read(m_oCounter);
	const GeoVecDoublearray& rFirstLaserLineIn = m_pPipeInFirstLaserLine->read(m_oCounter);
	const GeoVecDoublearray& rSecondLaserLineIn = m_pPipeInSecondLaserLine->read(m_oCounter);

	m_oSpFirstTrafo = rFirstLaserLineIn.context().trafo();
	m_oSpSecondTrafo = rSecondLaserLineIn.context().trafo();

	Trafo firstTrafo(*m_oSpFirstTrafo);
	Trafo secondTrafo(*m_oSpSecondTrafo);

	//Point p1 = firstTrafo(Point(0, 0));
	//Point p2 = secondTrafo(p1);

	bool firstValid = !inputIsInvalid(rFirstLaserLineIn);
	bool secondValid = !inputIsInvalid(rSecondLaserLineIn);

	geo2d::Doublearray oOutSeamPos;
	geo2d::Doublearray oOutSeamLeft;
	geo2d::Doublearray oOutSeamRight;

	const auto oAnalysisResult = rFirstLaserLineIn.analysisResult() == AnalysisOK ? rSecondLaserLineIn.analysisResult() : rFirstLaserLineIn.analysisResult(); // replace 2nd AnalysisOK by your result type

	if (!firstValid && !secondValid)
	{
		oOutSeamPos.getData().push_back(0);
		oOutSeamPos.getRank().push_back(0);
		oOutSeamLeft.getData().push_back(0);
		oOutSeamLeft.getRank().push_back(0);
		oOutSeamRight.getData().push_back(0);
		oOutSeamRight.getRank().push_back(0);

		const GeoDoublearray &rSeamPos = GeoDoublearray( rFirstLaserLineIn.context(), oOutSeamPos, rFirstLaserLineIn.analysisResult(), interface::NotPresent );
		const GeoDoublearray &rSeamLeft = GeoDoublearray( rFirstLaserLineIn.context(), oOutSeamLeft, rFirstLaserLineIn.analysisResult(), interface::NotPresent );
		const GeoDoublearray &rSeamRight = GeoDoublearray( rFirstLaserLineIn.context(), oOutSeamRight, rFirstLaserLineIn.analysisResult(), interface::NotPresent );
		preSignalAction();
		m_pPipeOutSeamPos->signal( rSeamPos );
		m_pPipeOutSeamLeft->signal( rSeamLeft );
		m_pPipeOutSeamRight->signal( rSeamRight );
		return; // RETURN
	}

	const VecDoublearray& rFirstLaserarray = firstValid ? rFirstLaserLineIn.ref() : rSecondLaserLineIn.ref();
	const VecDoublearray& rSecondLaserarray = secondValid ? rSecondLaserLineIn.ref() : rFirstLaserLineIn.ref();

	// Now do the actual image processing
	//calcLineWidthMinimum( rFirstLaserarray, rSeamWidthMinIn.ref(), oOutSeamPos, oOutSeamLeft, oOutSeamRight, m_resultSeamPos1, m_resultSeamLeft1, m_resultSeamRight1);

	//calcLineWidthMinimum( rSecondLaserarray, rSeamWidthMinIn.ref(), oOutSeamPos, oOutSeamLeft, oOutSeamRight, m_resultSeamPos2, m_resultSeamLeft2, m_resultSeamRight2);

	bool bSuccess = calculateSeamPos(rFirstLaserarray, rSecondLaserarray, rSeamWidthMinIn.ref(), rSeamWidthMaxIn.ref(), oOutSeamPos, oOutSeamLeft, oOutSeamRight );

	if (bSuccess)
	{
		m_isPaintPossible = true;
	}
	else
	{
		oOutSeamPos.getData().push_back(0);
		oOutSeamPos.getRank().push_back(0);
		oOutSeamLeft.getData().push_back(0);
		oOutSeamLeft.getRank().push_back(0);
		oOutSeamRight.getData().push_back(0);
		oOutSeamRight.getRank().push_back(0);
	}

	// Create a new byte array, and put the global context into the resulting profile

	const GeoDoublearray &rGeoSeamPos = GeoDoublearray(_isTrafo1Crucial ? rFirstLaserLineIn.context() : rSecondLaserLineIn.context(), oOutSeamPos, oAnalysisResult, bSuccess ? interface::Perfect : interface::NotPresent);
	const GeoDoublearray &rGeoSeamLeft = GeoDoublearray(_isTrafo1Crucial ? rFirstLaserLineIn.context() : rSecondLaserLineIn.context(), oOutSeamLeft, oAnalysisResult, bSuccess ? interface::Perfect : interface::NotPresent);
	const GeoDoublearray &rGeoSeamRight = GeoDoublearray(_isTrafo1Crucial ? rFirstLaserLineIn.context() : rSecondLaserLineIn.context(), oOutSeamRight, oAnalysisResult, bSuccess ? interface::Perfect : interface::NotPresent);
	preSignalAction();
	m_pPipeOutSeamPos->signal( rGeoSeamPos );
	m_pPipeOutSeamLeft->signal( rGeoSeamLeft );
	m_pPipeOutSeamRight->signal( rGeoSeamRight );

} // proceedGroup

bool TwoLinesWidthMinimum::calculateSeamPos(const geo2d::VecDoublearray & firstLaserLineIn, const geo2d::VecDoublearray & secondLaserLineIn,
						const geo2d::Doublearray & minimumSeamWidth, const geo2d::Doublearray & maximumSeamWidth,
						geo2d::Doublearray & seamPosOut, geo2d::Doublearray & seamLeftOut, geo2d::Doublearray & seamRightOut)
{
	// Ecken oben links der ROIs holen
	const Trafo &rFirstTrafo(*m_oSpFirstTrafo);
	const Trafo &rSecondTrafo(*m_oSpSecondTrafo);
	_twoLinesContainer.reset();

	Point firstStartPoint = rFirstTrafo(Point(0,0));
	Point secondStartPoint = rSecondTrafo(Point(0,0));
	int startX1 = firstStartPoint.x;
	int startX2 = secondStartPoint.x;

	_isTrafo1Crucial = (startX1 >= startX2);

	if ( (firstLaserLineIn.size() < 1) || (secondLaserLineIn.size() < 1) )
	{  // Fehlerfall! Laselinien nicht vorhanden. Output Null setzen (passiert ausserhalb), raus
		return false;
	}

	try
	{
		const auto& firstLaserLineIn_Data = firstLaserLineIn[0].getData();
		const auto& firstLaserLineIn_Rank = firstLaserLineIn[0].getRank();
		const auto& secondLaserLineIn_Data = secondLaserLineIn[0].getData();
		const auto& secondLaserLineIn_Rank = secondLaserLineIn[0].getRank();

		int size1 = firstLaserLineIn_Data.size();
		int size2 = secondLaserLineIn_Data.size();
		//int diffSize = std::abs(size1 - size2);

		int endX1 = startX1 + size1 - 1;
		int endX2 = startX2 + size2 - 1;

		int rankSize1 = firstLaserLineIn_Rank.size();
		int rankSize2 = secondLaserLineIn_Rank.size();

		if ((size1 != rankSize1) || (size2 != rankSize2))
		{ // Groesse der Daten und des Rank stimmen irgendwo nicht ueberein. Fehlerfall! Output Null setzen, raus
			return false;
		}

		int minStart = (startX1 < startX2) ? startX1 : startX2;
		int maxEnd = (endX1 > endX2) ? endX1 : endX2;
		// Der Bereich minStart bis maxEnd bildet jetzt das kleinste Intervall in dem beide ROIs enthalten sind.

		int counter1 = 0;
		int counter2 = 0;

		for (int counter = minStart; counter <= maxEnd; counter++)
		{
			bool isIn1 = ((counter >= startX1) && (counter <= endX1));
			bool isIn2 = ((counter >= startX2) && (counter <= endX2));

			if ((isIn1) && (isIn2)) // Ueberlapp-Fall, Stelle gefunden, die beide ROIs abdecken
			{
				_twoLinesContainer.addSingleTwoLinesPoint(SingleTwoLinesPoint(firstLaserLineIn_Data[counter1], firstLaserLineIn_Rank[counter1], secondLaserLineIn_Data[counter2], secondLaserLineIn_Rank[counter2]));
				// ToDo: Funktionalitaet im Container
			}

			if (isIn1) counter1++;
			if (isIn2) counter2++;
			if (counter1 >= size1) counter1 = size1 - 1;
			if (counter2 >= size2) counter2 = size2 - 1;
		}

		// Groesse von twoLinesContainer abfragen, raus, falls zu klein

		int curSeamWidthStart = (int)(0.5 + minimumSeamWidth.getData()[0]);
		int curSeamWidthEnd = (int)(0.5 + maximumSeamWidth.getData()[0]);

		int pos1, pos2, posSum;
		double width1, width2, widthSum;

		SeamFindResultContainer resultContainer;

		// hier nur die erste Haefte zur Breitenberechnung auf der Naht durchgehen
		for (int seamWidth = curSeamWidthStart; seamWidth <= (curSeamWidthEnd + curSeamWidthStart) / 2; seamWidth++)
		{
			// Resolution beachten
			int mod = seamWidth - curSeamWidthStart;
			if ((seamWidth != curSeamWidthStart) && (seamWidth != (curSeamWidthEnd + curSeamWidthStart) / 2)) // letzten auf jeden Fall mitmachen
			{
				if (mod % m_oResolution != 0) continue;
			}

			_twoLinesContainer.calcMinimum(seamWidth, pos1, pos2, posSum, width1, width2, widthSum);
			int pos1left = pos1 - seamWidth / 2;
			int pos1right = pos1 + seamWidth / 2;

			//_pos1.push_back(pos1left);
			//_pos1.push_back(pos1right);
			resultContainer.addResult(SeamFindResult(pos1left, pos1right, seamWidth, width1, Roi1));

			int pos2left = pos2 - seamWidth / 2;
			int pos2right = pos2 + seamWidth / 2;

			//_pos2.push_back(pos2left);
			//_pos2.push_back(pos2right);
			resultContainer.addResult(SeamFindResult(pos2left, pos2right, seamWidth, width2, Roi2));

			int posSumleft = posSum - seamWidth / 2;
			int posSumright = posSum + seamWidth / 2;

			//_posSum.push_back(posSumleft);
			//_posSum.push_back(posSumright);
			resultContainer.addResult(SeamFindResult(posSumleft, posSumright, seamWidth, widthSum, SumLine));

		}

		//double meanSeamWidth = resultContainer.getAverageSeamWidth();
		double meanWidth = resultContainer.getAverageLineWidth();
		//double meanLeft = resultContainer.getAverageLeft();
		//double meanRight = resultContainer.getAverageRight();
		//double meanNotOnSeam = _twoLinesContainer.getAverageWidthWithoutArea(meanLeft-50, meanRight+50);
		double medianLeft = resultContainer.getMedianLeft();
		double medianRight = resultContainer.getMedianRight();

		int leftSide = (int)(0.5 + medianLeft);
		int rightSide = (int)(0.5 + medianRight);
		double widthOutside = _twoLinesContainer.getAverageWidthWithoutArea(leftSide - 100, rightSide + 100);
		double maxWidth = (meanWidth + widthOutside) / 2;
		int startWidth = rightSide - leftSide;

		for (int it = startWidth; it < curSeamWidthEnd; it++)
		{
			double widthLeft = _twoLinesContainer.getAverageInArea(leftSide - 3, leftSide + 1);
			double widthLeftLeft = _twoLinesContainer.getAverageInArea(leftSide - 6, leftSide - 2);
			double widthRight = _twoLinesContainer.getAverageInArea(rightSide - 1, rightSide + 3);
			double widthRightRight = _twoLinesContainer.getAverageInArea(rightSide + 2, rightSide + 6);

			if (widthLeft < widthRight) //links kleinere Linienbreite
			{
				if (widthLeft<maxWidth) // nach link ist kleiner als mittlere Breite auf der Naht
				{
					if (leftSide - 3 > 0)
					{
						leftSide--;
						continue;
					}
				}
				else
				{

				}
			}

			if (widthRight < maxWidth) // nach rechts ist kleiner als mittlere Breite auf der Naht
			{
				if (rightSide + 3 < _twoLinesContainer.getSize())
				{
					rightSide++;
					continue;
				}
			}

			if (widthLeftLeft < widthRightRight) //links kleinere Linienbreite
			{
				if (widthLeftLeft<maxWidth) // nach link ist kleiner als mittlere Breite auf der Naht
				{
					if (leftSide - 6 > 0)
					{
						leftSide--;
						continue;
					}
				}
			}

			if (widthRightRight < maxWidth) // nach rechts ist kleiner als mittlere Breite auf der Naht
			{
				if (rightSide + 6 < _twoLinesContainer.getSize())
				{
					rightSide++;
					continue;
				}
			}
		}

		_overlappSize = _twoLinesContainer.getSize();

		m_resultSeamLeftTotal = leftSide;
		m_resultSeamRightTotal = rightSide;
		m_resultSeamPosTotal = (rightSide + leftSide) / 2;

		seamPosOut.getData().push_back((rightSide + leftSide) / 2);
		seamPosOut.getRank().push_back(255);
		seamLeftOut.getData().push_back(leftSide);
		seamLeftOut.getRank().push_back(255);
		seamRightOut.getData().push_back(rightSide);
		seamRightOut.getRank().push_back(255);

		return true;
	}
	catch (...)
	{
		return false;
	}
}


void TwoLinesWidthMinimum::calcLineWidthMinimum( const geo2d::VecDoublearray &p_rLaserLineIn, const Doublearray & p_rSeamWidth,
		Doublearray &p_rSeamPosOut, Doublearray &p_rSeamLeftOut, Doublearray &p_rSeamRightOut, int & drawPos, int & drawPosLeft, int & drawPosRight )
{
	const unsigned int	oNbLines	= p_rLaserLineIn.size();

	try
	{
		for (unsigned int lineN = 0; lineN < oNbLines; ++lineN)
		{ // loop over N lines
			int index = lineN > p_rSeamWidth.size() - 1 ? p_rSeamWidth.size() - 1 : lineN;
			int curSeamWidth = (int)(0.5+p_rSeamWidth.getData()[index]);

			const std::vector<double, std::allocator<double>>& rLaserLineIn_Data = p_rLaserLineIn[lineN].getData();
			//const std::vector<int, std::allocator<int>>& rLaserLineIn_Rank = p_rLaserLineIn[lineN].getRank();

			// if the size of the profile is not equal to the laser line size, resize the profile

			int minX = 0;
			int minSum = 1000000;

			for (int x = 1 + curSeamWidth / 2; x < (int)rLaserLineIn_Data.size() - curSeamWidth / 2 -1; x++)
			{
				int curWindowSum = 0;
				for (int index = x - curSeamWidth / 2; index < x + curSeamWidth / 2; index++) curWindowSum += (int)rLaserLineIn_Data[index];
				if (curWindowSum < minSum)
				{
					minX = x;
					minSum = curWindowSum;
				}
			}
			if (minSum != 1000000) // Min gefunden
			{
				drawPosLeft = minX - curSeamWidth / 2;
				drawPos = minX;
				drawPosRight = minX + curSeamWidth / 2;
				//p_rSeamPosOut.getData().push_back(minX);
				//p_rSeamPosOut.getRank().push_back(255);
				//p_rSeamLeftOut.getData().push_back(minX - curSeamWidth / 2);
				//p_rSeamLeftOut.getRank().push_back(255);
				//p_rSeamRightOut.getData().push_back(minX + curSeamWidth / 2);
				//p_rSeamRightOut.getRank().push_back(255);
			}
			else // kein Min gefunden
			{
				drawPosLeft = 0;
				drawPos = 0;
				drawPosRight = 0;
				//p_rSeamPosOut.getData().push_back(0);
				//p_rSeamPosOut.getRank().push_back(0);
				//p_rSeamLeftOut.getData().push_back(0);
				//p_rSeamLeftOut.getRank().push_back(0);
				//p_rSeamRightOut.getData().push_back(0);
				//p_rSeamRightOut.getRank().push_back(0);
			}

		} // for
	}
	catch(...)
	{
		drawPosLeft = 0;
		drawPos = 0;
		drawPosRight = 0;
		//p_rSeamPosOut.getData().push_back(0);
		//p_rSeamPosOut.getRank().push_back(0);
		//p_rSeamLeftOut.getData().push_back(0);
		//p_rSeamLeftOut.getRank().push_back(0);
		//p_rSeamRightOut.getData().push_back(0);
		//p_rSeamRightOut.getRank().push_back(0);
	}
} // extractLineProfile

/////////////////////////////////////////////////////////////
// SingleTwoLinesPoint
/////////////////////////////////////////////////////////////

SingleTwoLinesPoint::SingleTwoLinesPoint()
{
	line1Data   = 0.0;
	line1Rank   = 0;
	line2Data   = 0.0;
	line2Rank   = 0;
	lineSum     = 0.0;
	lineSumRank = 0;
}

SingleTwoLinesPoint::SingleTwoLinesPoint(double dataLine1, int rankLine1, double dataLine2, int rankLine2)
{
	line1Data = dataLine1;
	line1Rank = rankLine1;
	line2Data = dataLine2;
	line2Rank = rankLine2;

	bool is1Bad = (dataLine1 < 0) || (rankLine1 <= 0);
	bool is2Bad = (dataLine2 < 0) || (rankLine2 <= 0);

	if (is1Bad && is2Bad) // beide sind gestoert
	{
		lineSum = 0;
		lineSumRank = 0;
	}
	else
	{
		if (is1Bad)  //nur 1 gestoert
		{
			lineSum = line2Data;
			lineSumRank = line2Rank;
		}
		else
		{
			if (is2Bad) // nur 2 gestoert
			{
				lineSum = line1Data;
				lineSumRank = line1Rank;
			}
			else
			{
				lineSum = (line1Data + line2Data) / 2;
				lineSumRank = (line1Rank < line2Rank) ? line1Rank : line2Rank;
			}
		}
	}
}

/////////////////////////////////////////////////////////////
// TwoLinesContainer
/////////////////////////////////////////////////////////////

TwoLinesContainer::TwoLinesContainer()
{
	reset();
}

void TwoLinesContainer::reset()
{
	_container.clear();
	_maxVal = 0;
}

void TwoLinesContainer::addSingleTwoLinesPoint(SingleTwoLinesPoint singleTwoLinesPoint)
{
	_container.push_back(singleTwoLinesPoint);

	if (singleTwoLinesPoint.line1Data > _maxVal) _maxVal = (int)singleTwoLinesPoint.line1Data;
	if (singleTwoLinesPoint.line2Data > _maxVal) _maxVal = (int)singleTwoLinesPoint.line2Data;
	if (singleTwoLinesPoint.lineSum   > _maxVal) _maxVal = (int)singleTwoLinesPoint.lineSum;
}

SingleTwoLinesPoint TwoLinesContainer::getSingleTwoLinesPoint(int pos)
{
	if (pos>=(int)_container.size()) return SingleTwoLinesPoint();
	return _container[pos];
}

int TwoLinesContainer::getSize()
{
	return _container.size();
}

int TwoLinesContainer::getMaxValue()
{
	return _maxVal;
}

void TwoLinesContainer::resetSums()
{
	_hasSum1 = false;
	_curFirst1 = 0;
	_curLast1 = 0;
	_curSum1 = 0;
	_hasSum2 = false;
	_curFirst2 = 0;
	_curLast2 = 0;
	_curSum2 = 0;
	_hasSumSum = false;
	_curFirstSum = 0;
	_curLastSum = 0;
	_curSumSum = 0;
}

int TwoLinesContainer::calcSum1(int first, int last)
{
	if (_hasSum1) // hat bereits Daten fuer Laserliniensumme => checken, von wo bis wo diese ging, entsprechend rauswerfen, dazunehmen
	{
		if (first < _curFirst1) //kommen welche dazu
		{
			for (int i = _curFirst1 - 1; i >= first; i--)
				_curSum1 += (int)getSingleTwoLinesPoint(i).line1Data;
		}
		else if (first > _curFirst1) // gehen welche raus
		{
			for (int i = _curFirst1 + 1; i <= first; i++)
				_curSum1 -= (int)getSingleTwoLinesPoint(i).line1Data;
		}
		// im 3. Fall (first == _curFirst1) muss vorne nix gemacht werden

		if (last < _curLast1) // gehen welche raus
		{
			for (int i = _curLast1 - 1; i >= last; i--)
				_curSum1 -= (int)getSingleTwoLinesPoint(i).line1Data;
		}
		else if (last > _curLast1)  // kommen welche dazu
		{
			for (int i = _curLast1 + 1; i <= last; i++)
				_curSum1 += (int)getSingleTwoLinesPoint(i).line1Data;
		}
		// im 3. Fall (last == _curLast1) muss hinten nix gemacht werden

		// neue Grenzen speichern
		_curFirst1 = first;
		_curLast1 = last;
	}
	else // hat keine Daten fuer diese Laserlinie => komplett neu berechnen, von wo bis wo merken
	{
		_hasSum1 = true;
		_curSum1 = 0;
		_curFirst1 = first;
		_curLast1 = last;
		for (int i = first; i <= last; i++) _curSum1 += (int)getSingleTwoLinesPoint(i).line1Data;
	}

	return _curSum1;
}

int TwoLinesContainer::calcSum2(int first, int last)
{
	if (_hasSum2) // hat bereits Daten fuer Laserliniensumme => checken, von wo bis wo diese ging, entsprechend rauswerfen, dazunehmen
	{
		if (first < _curFirst2) //kommen welche dazu
		{
			for (int i = _curFirst2 - 1; i >= first; i--)
				_curSum2 += (int)getSingleTwoLinesPoint(i).line2Data;
		}
		else if (first > _curFirst2) // gehen welche raus
		{
			for (int i = _curFirst1 + 1; i <= first; i++)
				_curSum2 -= (int)getSingleTwoLinesPoint(i).line2Data;
		}
		// im 3. Fall (first == _curFirst2) muss vorne nix gemacht werden

		if (last < _curLast2) // gehen welche raus
		{
			for (int i = _curLast2 - 1; i >= last; i--)
				_curSum2 -= (int)getSingleTwoLinesPoint(i).line2Data;
		}
		else if (last > _curLast2)  // kommen welche dazu
		{
			for (int i = _curLast2 + 1; i <= last; i++)
				_curSum2 += (int)getSingleTwoLinesPoint(i).line2Data;
		}
		// im 3. Fall (last == _curLast1) muss hinten nix gemacht werden

		// neue Grenzen speichern
		_curFirst2 = first;
		_curLast2 = last;
	}
	else // hat keine Daten fuer diese Laserlinie => komplett neu berechnen, von wo bis wo merken
	{
		_hasSum2 = true;
		_curSum2 = 0;
		_curFirst2 = first;
		_curLast2 = last;
		for (int i = first; i <= last; i++) _curSum2 += (int)getSingleTwoLinesPoint(i).line2Data;
	}

	return _curSum2;
}

int TwoLinesContainer::calcSumSum(int first, int last)
{
	if (_hasSumSum) // hat bereits Daten fuer Laserliniensumme => checken, von wo bis wo diese ging, entsprechend rauswerfen, dazunehmen
	{
		if (first < _curFirstSum) //kommen welche dazu
		{
			for (int i = _curFirstSum - 1; i >= first; i--)
				_curSumSum += (int)getSingleTwoLinesPoint(i).lineSum;
		}
		else if (first > _curFirstSum) // gehen welche raus
		{
			for (int i = _curFirstSum + 1; i <= first; i++)
				_curSumSum -= (int)getSingleTwoLinesPoint(i).lineSum;
		}
		// im 3. Fall (first == _curFirstSum) muss vorne nix gemacht werden

		if (last < _curLastSum) // gehen welche raus
		{
			for (int i = _curLastSum - 1; i >= last; i--)
				_curSumSum -= (int)getSingleTwoLinesPoint(i).lineSum;
		}
		else if (last > _curLastSum)  // kommen welche dazu
		{
			for (int i = _curLastSum + 1; i <= last; i++)
				_curSumSum += (int)getSingleTwoLinesPoint(i).lineSum;
		}
		// im 3. Fall (last == _curLast1) muss hinten nix gemacht werden

		// neue Grenzen speichern
		_curFirstSum = first;
		_curLastSum = last;
	}
	else // hat keine Daten fuer diese Laserlinie => komplett neu berechnen, von wo bis wo merken
	{
		_hasSumSum = true;
		_curSumSum = 0;
		_curFirstSum = first;
		_curLastSum = last;
		for (int i = first; i <= last; i++) _curSumSum += (int)getSingleTwoLinesPoint(i).lineSum;
	}

	return _curSumSum;
}

void TwoLinesContainer::calcMinimum(int seamWidth, int & pos1, int & pos2, int & posSum, double & width1, double & width2, double & widthSum)
{
	int min1 = 10000000; int min2 = 10000000; int minSum = 10000000;
	resetSums();

	for (int x = 1 + seamWidth / 2; x < getSize() - seamWidth / 2 -1; x++)
	{
		int curWindow1 = 0;	int curWindow2 = 0; int curWindowSum = 0;

		curWindow1 = calcSum1(x - seamWidth / 2, x + seamWidth / 2 - 1);
		curWindow2 = calcSum2(x - seamWidth / 2, x + seamWidth / 2 - 1);
		curWindowSum = calcSumSum(x - seamWidth / 2, x + seamWidth / 2 - 1);

		/*for (int index = x - seamWidth / 2; index <= x + seamWidth / 2 -1; index++)
		{
			SingleTwoLinesPoint singleTwoLinesPoint = getSingleTwoLinesPoint(index);
			//curWindow1 += (int)singleTwoLinesPoint.line1Data;
			curWindow2 += (int)singleTwoLinesPoint.line2Data;
			curWindowSum += (int)singleTwoLinesPoint.lineSum;
		}*/

		if (curWindow1 < min1)
		{
			pos1 = x;
			min1 = curWindow1;
		}

		if (curWindow2 < min2)
		{
			pos2 = x;
			min2 = curWindow2;
		}

		if (curWindowSum < minSum)
		{
			posSum = x;
			minSum = curWindowSum;
		}
	}

	double doubleSeamWidth = seamWidth;
	width1 = min1 / doubleSeamWidth;
	width2 = min2 / doubleSeamWidth;
	widthSum = minSum / doubleSeamWidth;
}

double TwoLinesContainer::getAverageInArea(int left, int right)
{
	double sum = 0.0;
	int counter = 0;

	for (int i=0; i<(int)_container.size(); i++)
	{
		if ( (i<left) || (i>right) ) continue;
		sum += getSingleTwoLinesPoint(i).lineSum;

		counter++;
	}
	if (counter <= 0) return 0.0;
	return sum / counter;
}

double TwoLinesContainer::getAverageWidthWithoutArea(int left, int right)
{
	double sum = 0.0;
	int counter = 0;

	for (int i=0; i<(int)_container.size(); i++)
	{
		if ( (i>=left) && (i<=right) ) continue;
		sum += getSingleTwoLinesPoint(i).lineSum;

		counter++;
	}
	if (counter <= 0) return 0.0;
	return sum / counter;
}

SeamFindResult::SeamFindResult()
{
	_leftPos = 0;
	_rightPos = 0;
	_seamWidth = 0;
	_averageLineWidth = 0.0;
	_typeOfResult = SumLine;
}

SeamFindResult::SeamFindResult(int leftPos, int rightPos, int seamWidth, double averageLineWidth, ResultType typeOfResult)
{
	_leftPos = leftPos;
	_rightPos = rightPos;
	_seamWidth = seamWidth;
	_averageLineWidth = averageLineWidth;
	_typeOfResult = typeOfResult;
}

SeamFindResultContainer::SeamFindResultContainer()
{
	reset();
}

void SeamFindResultContainer::reset()
{
	_container.clear();
	_leftSum = _rightSum = _widthSum = _averageSum = 0.0;

}

void SeamFindResultContainer::addResult(SeamFindResult result)
{
	_container.push_back(result);
	_leftSum += result._leftPos;
	_rightSum += result._rightPos;
	_widthSum += result._seamWidth;
	_averageSum += result._averageLineWidth;
}

SeamFindResult SeamFindResultContainer::getSingleSeamFindResult(int pos)
{
	if (pos >= (int)_container.size()) return SeamFindResult();
	return _container[pos];
}

int SeamFindResultContainer::getSize()
{
	return _container.size();
}

double SeamFindResultContainer::getAverageLeft()
{
	int size = getSize();
	if (size == 0) return 0;
	return _leftSum / size;
}

double SeamFindResultContainer::getMedianLeft()
{
	StatisticCalculator calculator;
	for (int i=0; i<getSize(); i++)
	{
		SeamFindResult singleResult = getSingleSeamFindResult(i);
		calculator.addValue(singleResult._leftPos);
	}
	return calculator.getMedian();
}

double SeamFindResultContainer::getAverageRight()
{
	int size = getSize();
	if (size == 0) return 0;
	return _rightSum / size;
}

double SeamFindResultContainer::getMedianRight()
{
	StatisticCalculator calculator;
	for (int i=0; i<getSize(); i++)
	{
		SeamFindResult singleResult = getSingleSeamFindResult(i);
		calculator.addValue(singleResult._rightPos);
	}
	return calculator.getMedian();
}

double SeamFindResultContainer::getAverageSeamWidth()
{
	int size = getSize();
	if (size == 0) return 0;
	return _widthSum / size;
}

double SeamFindResultContainer::getAverageLineWidth()
{
	int size = getSize();
	if (size == 0) return 0;
	return _averageSum / size;
}

double SeamFindResultContainer::getAverageLineWidthForSeamWidth(int minSeamWidth, int maxSeamWidth)
{
	double sum = 0.0;
	int counter = 0;

	for (int i=0; i<(int)_container.size(); i++)
	{
		SeamFindResult singleSeamFindResult = getSingleSeamFindResult(i);

		if ( (singleSeamFindResult._seamWidth < minSeamWidth) || (singleSeamFindResult._seamWidth > maxSeamWidth) ) continue;

		sum += singleSeamFindResult._averageLineWidth;
		counter++;
	}
	if (counter <= 0) return 0.0;
	return sum / counter;
}

StatisticCalculator::StatisticCalculator()
{
	reset();
}

void StatisticCalculator::reset()
{
	_data.clear();
}

void StatisticCalculator::addValue(double value)
{
	_data.push_back(value);
}

double StatisticCalculator::getMedian()
{
	int size = _data.size();
	if (size<=0) return 0;
	if (size==1) return _data[0];

	sortIt();
	if (size%2==0) // Anzahl gerade
	{
		return (_data[size/2] + _data[size/2 -1]) / 2.0;
	}
	else // Anzahl ungerade
	{
		return _data[size/2];
	}
}

double StatisticCalculator::getMean()
{
	int size = _data.size();
	if (size<=0) return 0;
	if (size==1) return _data[0];

	double sum = 0;
	for (int i=0; i<size; i++) sum += _data[i];
	return sum / size;
}

void StatisticCalculator::sortIt()
{ // Bubble Sort
	int size = _data.size();
	if (size <= 1) return;
	for (int i=0; i<size-1; i++)
	{
		for (int j=0; j<size-i-1; j++)
		{
			if (_data[j]>_data[j+1]) exchange(_data[j], _data[j+1]);
		}
	}
}

void StatisticCalculator::exchange(double & d1, double & d2)
{
	double d = d1;
	d1 = d2;
	d2 = d;
}



} // namespace precitec
} // namespace filter
