/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		JS
 *  @date		2014
 *  @brief		Computes n-th local maximum / minimum of a line.
 */

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include "module/moduleLogger.h"
#include <filter/armStates.h>

#include "filter/algoArray.h"	///< algorithmic interface for class TArray

// local includes
#include "lineExtremumNumber.h"

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string LineExtremumNumber::m_oFilterName 	= std::string("LineExtremumNumber");
const std::string LineExtremumNumber::PIPENAME_OUT1  	= std::string("PositionX");
const std::string LineExtremumNumber::PIPENAME_OUT2	    = std::string("PositionY");
const std::string LineExtremumNumber::m_oParameterComputeAverageName = std::string("ComputeAverage");

LineExtremumNumber::LineExtremumNumber() :
	TransformFilter(LineExtremumNumber::m_oFilterName, Poco::UUID{"B08F3F81-5E28-475D-B586-6E3A2C8C5748"}),
	m_pPipeLineIn(nullptr),
	m_oPipePositionXOut	(this, LineExtremumNumber::PIPENAME_OUT1),
	m_oPipePositionYOut	(this, LineExtremumNumber::PIPENAME_OUT2),
	m_oExtremumType(eMaximum),
	m_oDirection		( eFromLeft ),
	m_oExtremumNumber	( 1 ),
	m_oExtremumThreshold (0.0),
	m_oExtremumDistance  (1),
	m_oExtremumDifference(0.),
	m_oComputeAverage(true),       // zu gewuenschtem Standard-Wert initialisieren
	m_oLinesIn(1),
    m_oOut( 1 )

{
	// Set default values of the parameters of the filter
	parameters_.add( "ExtremumType",	Parameter::TYPE_int, static_cast<int>(m_oExtremumType) );
	parameters_.add( "SearchDir",		Parameter::TYPE_int, static_cast<int>(m_oDirection) );
	parameters_.add( "ExtremumNumber",	Parameter::TYPE_int, static_cast<int>(m_oExtremumNumber) );
	parameters_.add( "ExtremumThreshold",	Parameter::TYPE_double, static_cast<double>(m_oExtremumThreshold) );
	parameters_.add("ExtremumDistance", Parameter::TYPE_int, static_cast<int>(m_oExtremumDistance));
	parameters_.add("ExtremumDifference", Parameter::TYPE_double, static_cast<double>(m_oExtremumDifference));
	parameters_.add(m_oParameterComputeAverageName, Parameter::TYPE_bool, m_oComputeAverage);

    setInPipeConnectors({{Poco::UUID("A9807233-32BE-4D3F-BBA7-E73FD77C589B"), m_pPipeLineIn, "Line", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("B2F98184-B4CD-4B0C-AD9F-C26C12753464"), &m_oPipePositionXOut, PIPENAME_OUT1, 0, ""},
    {Poco::UUID("07261AE7-EC49-4797-B7A8-8ACAB49C1090"), &m_oPipePositionYOut, PIPENAME_OUT2, 0, ""}});
    setVariantID(Poco::UUID("9133DC61-2BD2-4945-85E7-1BBEA4F12F9B"));
} // LineExtremum



void LineExtremumNumber::setParameter()
{
	TransformFilter::setParameter();
	m_oExtremumType			= static_cast<ExtremumType>( parameters_.getParameter("ExtremumType").convert<int>() );
	m_oDirection			= static_cast<SearchDirType>( parameters_.getParameter("SearchDir").convert<int>() );
	m_oExtremumNumber		= static_cast<int>( parameters_.getParameter("ExtremumNumber").convert<int>() );
	m_oExtremumThreshold	= static_cast<double>( parameters_.getParameter("ExtremumThreshold").convert<double>() );
	m_oExtremumDistance     = static_cast<int>(parameters_.getParameter("ExtremumDistance").convert<int>());
	m_oExtremumDifference   = static_cast<double>(parameters_.getParameter("ExtremumDifference").convert<double>());
	m_oComputeAverage = static_cast<bool>(parameters_.getParameter(m_oParameterComputeAverageName).convert<bool>());
} // setParameter



void LineExtremumNumber::paint()
{
	if(m_oVerbosity < eLow || inputIsInvalid(m_oOut) || m_oSpTrafo.isNull()){
		return;
	} // if

	const Trafo		&rTrafo			( *m_oSpTrafo );
	OverlayCanvas	&rCanvas		( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerPosition	( rCanvas.getLayerPosition());
	OverlayLayer	&rLayerContour  ( rCanvas.getLayerContour());
	OverlayLayer	&rLayerText		( rCanvas.getLayerText() );

	if ((m_overlayMax - m_overlayMin) <= 0) return;
	const int yo = -50; // Malbereich in y
	const int yu = 50;
	auto y_rescale = [this, &yo, &yu]( const double& y ) {
		//change sign and rescale the  coordinate between yu (for y = min) and yo (for y= max)
		int ydiff = this->m_overlayMax - this->m_overlayMin;
		return  yu - (int)std::ceil((y - this->m_overlayMin) / ydiff * (yu - yo));
	};

	if (m_oVerbosity >= eHigh)
	{

		const unsigned int	oNbLines = m_oLinesIn.size();
		const auto&	rProfile(m_oLinesIn[0].getData());
		for (unsigned int lineN = 0; lineN < oNbLines; ++lineN)
		{ // loop over N lines
			const Doublearray	&rLineIn = m_oLinesIn[lineN];
			const auto&	rProfile(rLineIn.getData());

            std::vector<int> yRescaled;
            yRescaled.reserve(rProfile.size());
			for (unsigned int i = 0; i != rProfile.size(); ++i)
			{
				yRescaled.push_back(y_rescale(rProfile[i]));

			} // for
			rLayerContour.add<OverlayPointList>(Point{rTrafo.dx(), rTrafo.dy()}, std::move(yRescaled), Color::Yellow());

		} //for
		//annotate y axis
		for (int val : { m_overlayMax, m_overlayMax / 2, m_overlayMin / 2, m_overlayMin, int(m_oOut.getData().front().y) })
		{
			std::stringstream oMsg;
			oMsg.precision(2);
			oMsg << std::fixed << val;
			rLayerText.add<OverlayText>(oMsg.str(), Font(10), rTrafo(Rect(rProfile.size(), y_rescale(val), 200, 20)), Color::Yellow());
			}
};

	if (m_oVerbosity > eHigh)
	{
		// Schwelle
		const auto&	rProfile(m_oLinesIn[0].getData());
		int yThreshold = y_rescale(m_iThreshold);
		rLayerContour.add(new OverlayLine(rTrafo(Point(0, yThreshold)), rTrafo(Point(rProfile.size(), yThreshold)), Color::Blue()));

		{//Momentics can't use to_string
		std::stringstream oMsg;
		oMsg.precision(2);
		oMsg << std::fixed << m_iThreshold;
		rLayerText.add<OverlayText>(oMsg.str(), Font(10), rTrafo(Rect(0, yThreshold, 200, 20)), Color::Blue());
		}

		double distanceOnThreshold = (m_oExtremumType == eMaximum) ? m_iThreshold + m_oExtremumDifference : m_iThreshold - m_oExtremumDifference;
		int ydistanceOnThreshold = y_rescale(distanceOnThreshold);
		rLayerContour.add<OverlayLine>(rTrafo(Point(0, ydistanceOnThreshold)), rTrafo(Point(rProfile.size(), ydistanceOnThreshold)), Color::Orange());

	}

	geo2d::Point q(0, 0);
	if (m_oVerbosity >= eMedium)
	{

		for (unsigned int ix = 0; ix < m_oResPoints.size(); ++ix)
		{
			q.x = static_cast<int>(m_oResPoints[ix].x);
			q.y = y_rescale(m_oResPoints[ix].y);

			rLayerPosition.add<OverlayCross>(rTrafo(q), Color::Cyan());
			if (m_oVerbosity > eHigh)
			{
				std::ostringstream oMsg;
				oMsg << ix + 1 << "/" << m_oResPoints.size() << ") [" << int(m_oResPoints[ix].x) << " ," << m_oResPoints[ix].y << "] ";
				wmLog(eDebug, oMsg.str());
				std::cout << oMsg.str() << std::endl;
			}

		}
	}

	if ((m_oVerbosity >=eMedium ) && (m_oResPoints.size() == 0))
	{
		rLayerText.add<OverlayText>("No extremum found", Font(14), rTrafo(Rect(0, 0, 200, 20)), Color::Cyan());
	}


	if (m_oVerbosity > eLow)
	{
		q.x = int(m_oOut.getData().front().x);
		q.y = y_rescale(m_oOut.getData().front().y);

		rLayerPosition.add<OverlayCross>(rTrafo(q), Color::Red()); // paint first position
	}

} // paint



bool LineExtremumNumber::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeLineIn  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void LineExtremumNumber::proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeLineIn != nullptr); // to be asserted by graph editor
	// Read-out laserline
	const GeoVecDoublearray& rLineIn = m_pPipeLineIn->read(m_oCounter);
	m_oSpTrafo	= rLineIn.context().trafo();
	// And extract the byte-array
	m_oLinesIn = rLineIn.ref();
	const VecDoublearray& rArrayIn = rLineIn.ref();
	/// reset output based on input size
	m_oOut.assign(rArrayIn.size(), DPoint(0, 0), eRankMin); //testdouble

	m_overlayMin = 1000000;
	m_overlayMax = -1000000;


	// input validity check

	if ( inputIsInvalid(rLineIn) ) {
		const GeoDoublearray oGeoDoublearrayXOut(rLineIn.context(), getCoordinate(m_oOut, eX), rLineIn.analysisResult(), interface::NotPresent); // bad rank
		const GeoDoublearray oGeoDoublearrayYOut(rLineIn.context(), getCoordinate(m_oOut, eY), rLineIn.analysisResult(), interface::NotPresent); // bad rank
		preSignalAction();
		m_oPipePositionXOut.signal( oGeoDoublearrayXOut ); // invoke linked filter(s)
		m_oPipePositionYOut.signal( oGeoDoublearrayYOut ); // invoke linked filter(s)

		return; // RETURN
	}

	// Now do the actual image processing
	findExtremumNumber(rArrayIn, m_oExtremumType, m_oExtremumNumber, m_oExtremumThreshold,
		    m_oExtremumDistance, m_oExtremumDifference, m_oOut);

	// Create a new point, put the global context into the resulting profile and copy the rank over
	const auto oAnalysisResult	= rLineIn.analysisResult() == AnalysisOK ? AnalysisOK : rLineIn.analysisResult(); // replace 2nd AnalysisOK by your result type
	const GeoDoublearray oGeoDoublearrayXOut(rLineIn.context(), getCoordinate(m_oOut, eX), oAnalysisResult, interface::Limit); // full rank here, detailed rank in array
	const GeoDoublearray oGeoDoublearrayYOut(rLineIn.context(), getCoordinate(m_oOut, eY), oAnalysisResult, interface::Limit); // full rank here, detailed rank in array
	preSignalAction();
	m_oPipePositionXOut.signal( oGeoDoublearrayXOut ); // invoke linked filter(s)
	m_oPipePositionYOut.signal( oGeoDoublearrayYOut ); // invoke linked filter(s)

} // proceedGroup


void LineExtremumNumber::findExtremumNumber( const geo2d::VecDoublearray &p_rLineIn, ExtremumType p_oExtremumType,
	int p_oExtremumNumber, double p_oExtremumThreshold, int p_oExtremumDistance, double p_oExtremumDifference, geo2d::DPointarray &p_rPositionOut)
{
	const unsigned int	oNbLines	= p_rLineIn.size();
	p_rPositionOut.assign(oNbLines, DPoint(0, 0)); // if the size of the output signal is not equal to the input line size, resize
	for (unsigned int lineN = 0; lineN < oNbLines; ++lineN) { // loop over N lines
		const Doublearray	&rLineIn = p_rLineIn[lineN];
		DPoint				&rPositionOut = p_rPositionOut.getData()[lineN];
		int					&rPositionOutRank = p_rPositionOut.getRank()[lineN];
        std::pair<double, int> oResult;

		if ( p_oExtremumType == eZeroCrossing )
		{
			oResult = searchZeroCrossing(rLineIn, p_oExtremumNumber, p_oExtremumThreshold, p_oExtremumDifference, m_oDirection);
	    }
		else
		{
		    oResult = searchLocalMax(rLineIn, p_oExtremumType, p_oExtremumNumber, p_oExtremumThreshold, p_oExtremumDistance, p_oExtremumDifference, m_oDirection);
        }
		//int xOut = std::get<eData>(oExtremum);
		//int rankOut = std::get<eRank>(oExtremum);
		//int yOut = int(rLineIn.getData()[xOut]);

		rPositionOut.x      = std::get<eData>( oResult );    //oExtremum.x;   //std::get<eData>(oExtremum);
		rPositionOutRank    = std::get<eRank>( oResult );
		rPositionOut.y      = static_cast<double>(rLineIn.getData()[rPositionOut.x]);

		/*for (unsigned int m = 0; m < m_oResPoints.size(); ++m)
					std::cout<<"RESULT LOCAL MAX: "<<rPositionOut.y
					         <<" xPosition: " << m_oResPoints[m].x
					         <<" yPosition: " << m_oResPoints[m].y << std::endl;*/

	} // for
} // findExtremum



std::pair<double, int> LineExtremumNumber::searchZeroCrossing( const Doublearray &rLineIn,
	const int    p_oExtremumNumber,
	const double p_oExtremumThreshold,
	const double    p_oExtremumDifference,
	SearchDirType p_oDirection )
{

	geo2d::DPoint p( 0, 0 );

	m_iThreshold = p_oExtremumThreshold;

	int   ctr = 0;

	const std::vector<double> &value = rLineIn.getData();
	const std::vector<int> &rank = rLineIn.getRank();


	//vector m_oResPoints zuruecksetzen
	m_oResPoints.clear();

	if ( p_oExtremumNumber <= 0 )
	{
		return std::make_pair( 0, rank[0] );
	}


	// Startpunkt der Linie erster Punkt mit gueltigem rank ?

	//modify iteration conditions and values such as it's always a search from left to right

	int start = (p_oDirection == eFromLeft) ? 0 : (value.size() - 1);
	int end = (p_oDirection == eFromLeft) ? (value.size()) : -1;
	int direction = (p_oDirection == eFromLeft) ? 1 : -1;
	int margin = 1; //15 when the mean is used to compute the threshold
	double threshold = m_iThreshold;
	int actualStart( start + direction*margin );

	// position of y value: 0: on threshold, -1 : below threshold, 1: above threshold
	int cur_pos = 0; // position respect threshold +- extremumDifference : 1=above, -1=below
	int prev_pos = 0;
	bool firstRun = true;

	for ( int x = actualStart; x != end; x = x + direction )
	{
		//assert((x >= 0) && (x < value.size()));

		if ( rank[x] > eRankMin )
		{

			if ( value[x] > m_overlayMax )
			{
				m_overlayMax = int( value[x] );
			}
			if ( value[x] < m_overlayMin )
			{
				m_overlayMin = int( value[x] );
			}

			prev_pos = cur_pos;
			double y = value[x];
			double diff = y - threshold;

			if ( diff > p_oExtremumDifference )
			{
				cur_pos = 1;
			}
			else if ( diff < -p_oExtremumDifference )
			{
				cur_pos = -1;
			}
			else
			{
				cur_pos = 0;
			}

			if ( !(cur_pos == prev_pos) && !firstRun )
			{
				p.x = static_cast<double>(x);
				p.y = static_cast<double>(y);
				m_oResPoints.push_back( p );

				ctr++;
				if ( ctr == p_oExtremumNumber )
				{
					return	std::make_pair( x, rank[x] );
				}

			}
			firstRun = false;

		}//if rank



	}// for x= start

	return	std::make_pair( 0, rank[0] );
}





// typedef TArray< double >	doublearray;
// TArray definiert in array.h
// Daten steh in m_odata
// rank in m_oRank
// Point TArray<Point>
//geo2d::Point

std::pair<double,int> LineExtremumNumber::searchLocalMax(const Doublearray &rLineIn,
	ExtremumType p_oExtremumType,
	const int    p_oExtremumNumber,
	const double p_oExtremumThreshold,
	const int    p_ExtremumDistance,
	const double p_oExtremumDifference,
	SearchDirType p_oDirection)
{


	geo2d::DPoint p(0, 0); //testdouble

	//finde n tes Extremum in einer Kurve oberhalb einer schwelle
	// ueber x
	// von links nach rechts, maximum

	//double maxValue = 0.0;
	//double localMax = 0.0;
	//int    xMaxPos = 0;
	int    found = 0;
	m_iThreshold = p_oExtremumThreshold;

	int   xPos = 0; // xPos[1024];
	int   ctr = 0;
	int   ctrMax = 0;
	unsigned int    x = 0;

	//double lastValue = 0.0;
	unsigned int diff = 1;        // muss mindestens 1 sein
	double     correct = 0.0;      //faktor

	// wie kommt man an den ersten wert der rLineIn ??
	const std::vector<double> &value = rLineIn.getData();
	const std::vector<int> &rank = rLineIn.getRank();



	diff = p_ExtremumDistance;
	correct = p_oExtremumDifference;

	//vector m_oResPoints zuruecksetzen
	m_oResPoints.clear();

	if (p_oExtremumNumber <= 0)
	{
		return std::make_pair(p.x, rank[p.x]);
	}

	// Startpunkt der Linie erster Punkt mit gueltigem rank ?

	//mittelwert aus den ersten 15 Werten rechnen:
	int    m = 0;
	double sum = 0.0;
	unsigned int   rand = 15;

	if (diff > rand)
		rand = diff;

	//die ersten 16 Punkte zur Mittelwertberechnung
	if( value.size() < rand+2 )
		return	std::make_pair(p.x, rank[p.x]);
	if (value.size() < diff + 2)
		return	std::make_pair(p.x, rank[p.x]);

	unsigned int startx = 0;
	unsigned int endx   = 0;


	//von inks
	if (p_oDirection == 0)
	{

		//first valid point:
		for (x = 1; x< value.size() - rand; ++x)
		{
			if (rank[x]>0)
			{
				startx = x;
				endx = startx + rand + 1;
				x = value.size();
			}
		}

		for (x = startx; x< endx; ++x)
		{
			if (rank[x]>0)
			{
				sum += value[x];
				m++;
			}
		}
		if (m_oComputeAverage)
		{
			if (m > 5)
			{
				sum /= (double)m;
				if (p_oExtremumType == eMinimum)
					m_iThreshold = sum - m_iThreshold;
				else
					m_iThreshold = sum + m_iThreshold;
			}
			else // nicht genuegend Werte zum mitteln
			{
				return	std::make_pair(p.x, rank[p.x]);
			}
		}
		else
		{
			// NEU in GUI kann negativer Grenzwert eingetragen werden. Deshalb nicht mehr notwendig!!
			//if (p_oExtremumType == eMinimum)
			//{
				// Wenn wir nach einem Minimum suchen und den Mittelwert nicht auf die Grenze anwenden wollen,
				// dann muss der Grenzwert trotzdem negativ sein... jedenfalls ist das die Annahme.
				// Falls jemand nach einem positiven Minimum suchen will, dann ist dieser Fall nicht abgedeckt!
				// Das GUI ist so gemacht, dass man einen negativen Grenzwert nicht eingeben kann, und aus
				// Kompatibilitaetsgruenden wollen wir das GUI nicht aendern!
				//threshold = -threshold;
			//}

			// Rand auf Filterdiffernez setzen
			rand = diff;
		}
	}//p_oDirection
	else // von rechts
	{

		//first valid point right side
		//first valid point:
		for (x = value.size()-1; x > rand; --x) //Achtung hoechster Wert von rank[] ist value.size -1 +
		{
			if (rank[x]>0)
			{
				startx = x;
				endx = startx - rand + 1;
				x = rand; //loop end
			}
		}

		for (x = startx; x>endx; --x)
		{
			if (rank[x]>0)
			{
				sum += value[x];
				m++;
			}
		}
		if (m_oComputeAverage)
		{
			if (m > 5)
			{
				sum /= (double)m;
				if (p_oExtremumType == eMinimum)
					m_iThreshold = sum - m_iThreshold;
				else
					m_iThreshold = sum + m_iThreshold;
			}
			else // nicht genuegend Werte zum mitteln
			{
				return	std::make_pair(p.x, rank[p.x]);
			}
		}
		else
		{
			// NEU in GUI kann negativer Grenzwert eingetragen werden. Deshalb nicht mehr notwendig!!
			//if (p_oExtremumType == eMinimum)
			//{
			// Wenn wir nach einem Minimum suchen und den Mittelwert nicht auf die Grenze anwenden wollen,
			// dann muss der Grenzwert trotzdem negativ sein... jedenfalls ist das die Annahme.
			// Falls jemand nach einem positiven Minimum suchen will, dann ist dieser Fall nicht abgedeckt!
			// Das GUI ist so gemacht, dass man einen negativen Grenzwert nicht eingeben kann, und aus
			// Kompatibilitaetsgruenden wollen wir das GUI nicht aendern!
			//threshold = -threshold;
			//}

			// Rand auf Filterdiffernez setzen
			rand = diff;
		}
	}//else


	for (x = rand; x< value.size()-rand ; ++x)
	{
		if (rank[x]>0)
		{
				if (p_oExtremumType==1)
				{

					if ((value[x] > m_iThreshold) && (value[x] > (value[x - diff] + correct)))  //schwelle
					{
						xPos = x;
						found = 1;
					}
					if (found)
					{
						if (value[x] < (value[x - diff] - correct))
						{
							// Punkt uebernehmen indem index erhoeht wird
							xPos = (x + xPos) / 2;
							p.x = static_cast<double>(xPos); // xPos[ctr];
							p.y = static_cast<double>(value[xPos]);  //testdouble
							m_oResPoints.push_back(p);

							ctr++;
							found = 0;
						}
					}
				}//if p_oEXtremumType
				else
				{
					if ((value[x] < m_iThreshold) && (value[x] < (value[x - diff] - correct)))  //schwelle
					{
						xPos = x;
						found = 1;
					}
					if (found)
					{
						if (value[x] > (value[x - diff] + correct))
						{
							// Punkt uebernehmen indem index erhoeht wird
							xPos = (x + xPos) / 2;
							p.x = static_cast<double>(xPos); // xPos[ctr];
							p.y = static_cast<double>(value[xPos]);  //testdouble
							m_oResPoints.push_back(p);

							ctr++;
							found = 0;
						}
					}
				}//else if p_oExtremumType

		}//if rank

		if (value[x] > m_overlayMax)
		{
			m_overlayMax = int(value[x]);
		}
		if (value[x] < m_overlayMin)
		{
			m_overlayMin = int(value[x]);
		}

	}// for x= start


	ctrMax = ctr;

	//nix gefunden:
	if (ctr - 1 < 0)
	{
		p.x = 0.0;  //testdouble
		p.y = 0.0;
		return	std::make_pair(p.x, rank[p.x]);
	}

	int position = 0;
	//von links kommend;
	if (p_oDirection==0)
	{
		if (p_oExtremumNumber >= 1)
			position = p_oExtremumNumber - 1;
		// position muss existieren:
		if (position > ctrMax - 1)
			position = ctrMax - 1;

		p.x = static_cast<double>( m_oResPoints[position].x);     //xPos[position];
		p.y = static_cast<double>(value[p.x]);                    //testdouble
	}
	else // von rechts kommend :
	{
		position = ctrMax - p_oExtremumNumber;
		if (position < 0)
			position = 0; // p_oExtremumNumber - 1;
		//Result
		p.x = static_cast<double>(m_oResPoints[position].x);                           //    xPos[position];      //testdouble
		p.y = static_cast<double>(value[p.x]);

	}

	return	std::make_pair(p.x,rank[p.x]);

} // searchLocalMAx


/*virtual*/ void
LineExtremumNumber::arm(const fliplib::ArmStateBase& state) {
	if (state.getStateID() == eSeamStart)
	{
		m_oSpTrafo = nullptr;
	}
} // arm

} // namespace precitec
} // namespace filter
