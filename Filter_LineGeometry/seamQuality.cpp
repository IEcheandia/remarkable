// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>

#include "filter/algoArray.h"	///< algorithmic interface for class TArray
#include "filter/algoImage.h"   ///< for applying trafo to Vec3D for 2d to 3d conversion
#include <filter/structures.h>
#include <filter/armStates.h>

#include <fliplib/TypeToDataTypeImpl.h>

// local includes
#include "seamQuality.h"

#include "util/calibDataSingleton.h"
#include "2D/avgAndRegression.h"

using namespace fliplib;

namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;

namespace filter {

const std::string SeamQuality::m_oFilterName      = std::string("SeamQuality");
const std::string SeamQuality::PIPENAME_LENGTH = std::string("Length");	 	 //Ausgangspipename
const std::string SeamQuality::PIPENAME_DEVIATION = std::string("Deviation");    //Ausgangspipename
const std::string SeamQuality::PIPENAME_TRACKED_LENGTH = std::string("TrackedLength");	 	 //Ausgangspipename
const std::string SeamQuality::PIPENAME_GAP_DETECTED = std::string("GapDetected");    //Ausgangspipename

std::pair<double, int> length(	const geo2d::Doublearray &p_rLaserLine,
								const int &xStart,
								const int &xEnd,
								const int &maxJump,
								const int &maxDistanc,
								geo2d::TPoint<double>  &JumpPos,
								SearchDirType p_oDirection);


const double m_epsilon  = 0.000001;
const double m_bigSlope = 1000000.0;

SeamQuality::SeamQuality() : TransformFilter(SeamQuality::m_oFilterName, Poco::UUID{"07AA08CD-9DFF-4BCA-B712-F2FE08CEEF05"}),
m_pPipeInLaserline(nullptr),
m_pPipeInXLeft(nullptr),
m_pPipeInXRight(nullptr),
m_oPipeOutLength(this, SeamQuality::PIPENAME_LENGTH),
m_oPipeOutDeviation(this, SeamQuality::PIPENAME_DEVIATION),
m_oPipeOutTrackedLength(this, SeamQuality::PIPENAME_TRACKED_LENGTH),
m_oPipeOutGapDetected(this, SeamQuality::PIPENAME_GAP_DETECTED)

{

		parameters_.add("MaxJump", Parameter::TYPE_Int32, m_oMaxJump);
		parameters_.add("MaxDistance", Parameter::TYPE_Int32, m_oMaxDistance);
		parameters_.add("MaxDeviation", Parameter::TYPE_double, m_oMaxDeviation);
		parameters_.add("FitMethod", Parameter::TYPE_Int32, m_oFitMethod);
		parameters_.add("SearchDir", Parameter::TYPE_int, static_cast<int>(m_oDirection));
		parameters_.add("GapThreshold", Parameter::TYPE_double, m_oGapThreshold);

        setInPipeConnectors({{Poco::UUID("BDFB41EB-E0E1-4053-81DB-B4BB82CFD35A"),m_pPipeInLaserline, "Line", 1, ""},
        {Poco::UUID("19867781-C162-4E93-80F7-087EA7A915A7"), m_pPipeInXLeft, "xLeft", 1, "xLeft"},
        {Poco::UUID("EE2FD34B-446F-4698-87AF-446E891510FC"), m_pPipeInXRight, "xRight", 1, "xRight"}});
        setOutPipeConnectors({{Poco::UUID("290DD1FE-C868-409D-9CFC-088180477BE1"), &m_oPipeOutLength, "Length", 0, ""},
        {Poco::UUID("B8E6D32A-14D8-454C-A9F5-4A6505A116D1"), &m_oPipeOutDeviation, "Deviation", 0, ""},
        {Poco::UUID("33840088-8C75-4F87-A7E7-B6AF0043C9D7"), &m_oPipeOutTrackedLength, "TrackedLength", 0, ""},
        {Poco::UUID("4B3B4AB1-09E2-4ACF-A0B0-5F5736ACFA86"), &m_oPipeOutGapDetected, "GapDetected", 0, ""}});
        setVariantID(Poco::UUID("93FF28D6-C378-4E21-9C0C-778F4C004234"));
}

void SeamQuality::setParameter()
{
	TransformFilter::setParameter();
	m_oMaxJump 		= parameters_.getParameter("MaxJump").convert<int> ();
	m_oMaxDistance 	= parameters_.getParameter("MaxDistance").convert<int> ();
	m_oMaxDeviation	= parameters_.getParameter("MaxDeviation").convert<double> ();
	m_oFitMethod    = parameters_.getParameter("FitMethod").convert<int> ();
	m_oDirection	= static_cast<SearchDirType>( parameters_.getParameter("SearchDir").convert<int>() );
	m_oGapThreshold = parameters_.getParameter("GapThreshold").convert<double>();

}


void SeamQuality::paint()
{

	if( m_oVerbosity < eLow || m_oSpTrafo.isNull() )
	{
		return;
	}

	if (fabs(m_oJumpPos.x) < 0.1 && fabs(m_oJumpPos.y) < 0.1)
	{
		return;
	}

	//const Trafo		&rTrafo( *m_oSpTrafo );
	//double dx = m_oSpTrafo->dx();
	//double dy = m_oSpTrafo->dy();

	OverlayCanvas	&rCanvas			( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer		&rLayerPosition(rCanvas.getLayerPosition());
	const Point		DrawPositionIn(roundToT<int>(m_oJumpPos.x + m_oSpTrafo->dx()), roundToT<int>(m_oJumpPos.y + m_oSpTrafo->dy() ));
	//std::cout<<"paint: "<<DrawPositionIn<<std::endl;
	rLayerPosition.add(new OverlayCross(DrawPositionIn, Color::Red())); // draw cross at in-position


} // paint



//Die Namen der Eingangspipes muessen so wie sie hier stehen
// auch im XML file stehen:
// z.B.: <in group = "1" pipe = "Scalar" sender = "Parameter1">xLeft< / in>
// Es sei denn " " ist erlaubt: (p_rPipe.tag() == ""
bool SeamQuality::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{


	if (p_rPipe.tag() == "xLeft")																//Name der Eingangspipe
	{
		m_pPipeInXLeft  = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray >* >(&p_rPipe);
	} else if (p_rPipe.tag() == "xRight")														// Name der Einganspipe
	{
		m_pPipeInXRight  = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray >* >(&p_rPipe);
	} else if ( (p_rPipe.tag() == "Line") || (p_rPipe.tag() == "") )							// Name der Eingangspipe
	{
		m_pPipeInLaserline = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray >* >(&p_rPipe);
	}
	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void SeamQuality::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
{

	poco_check_ptr(m_pPipeInXLeft      != nullptr); // to be asserted by graph editor
	poco_check_ptr(m_pPipeInXRight     != nullptr); // to be asserted by graph editor
	poco_check_ptr(m_pPipeInLaserline  != nullptr); // to be asserted by graph editor


	// In einen GeoDoubleArray lesen
	const GeoDoublearray&		rGeoPosX1In			( m_pPipeInXLeft->read(m_oCounter) );
	const GeoDoublearray&		rGeoPosX2In			( m_pPipeInXRight->read(m_oCounter) );
	const GeoVecDoublearray&    rLineIn 	        = m_pPipeInLaserline->read(m_oCounter);
	// And extract the byte-array
	const VecDoublearray& rArrayIn = rLineIn.ref();

	// Aus dem GeoDouble in einen Double Array lesen
	const Doublearray&			rPosX1In			( rGeoPosX1In.ref() );
	const Doublearray&			rPosX2In			( rGeoPosX2In.ref() );


    // Kontext auslesen
	const ImageContext&			rContextX1			( rGeoPosX1In.context() );


	// HW ROI Werte holen
	m_oHWRoiPos.x	= rContextX1.HW_ROI_x0;
	m_oHWRoiPos.y	= rContextX1.HW_ROI_y0;

	// Sw ROI bzw. Translation rausholen
	// m_oSpTrafo	= rGeoPosX1In.context().trafo();	// speichere trafo (SW-ROI) um spaeter korrekt zeichnen zu koennen
	m_oSpTrafo = rLineIn.context().trafo();


	//Eingangswerte pruefen
	if (rPosX1In.size() != 1) { // result is always one point
		wmLog(eDebug, "Filter '%s': Received %u X values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rPosX1In.size());
	}
	if (rPosX2In.size() != 1) { // result is always one point
		wmLog(eDebug, "Filter '%s': Received %u X values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rPosX2In.size());
	}



	if(inputIsInvalid(rGeoPosX1In) ||  inputIsInvalid(rGeoPosX2In))
	{
		if(m_oVerbosity >= eMedium) {
			wmLog(eInfo, "SeamQuality: Input Values invalid.\n");
		} // if
	}

	m_oLengthOut.assign(1, 0, eRankMin);
	m_oDeviationOut.assign(1,0.0,eRankMin);
	m_oTrackedLengthOut.assign(1, 0.0, eRankMin);
	m_oGapDetectedOut.assign(1, 0, eRankMin);

	m_oJumpPos.x = 0;
	m_oJumpPos.y = 0;


	//CTOR mit ImageContext const& context, const ValueT& p_rValue, const ResultType p_oRes, double r=NotPresent
	// schlechte Ausgabewerte bauen

	// input validity check
	if( inputIsInvalid(rLineIn) )
	{

		const GeoDoublearray		oGeoOutLength(rContextX1, m_oLengthOut, rGeoPosX1In.analysisResult(), NotPresent); // bad geo rank
		const GeoDoublearray		oGeoOutDeviation(rContextX1, m_oDeviationOut, rGeoPosX1In.analysisResult(), NotPresent); // bad geo rank
		const GeoDoublearray		oGeoOutTrackedLength(rContextX1, m_oTrackedLengthOut, rGeoPosX1In.analysisResult(), NotPresent); // bad geo rank
		const GeoDoublearray		oGeoOutGapDetected(rContextX1, m_oGapDetectedOut, rGeoPosX1In.analysisResult(), NotPresent); // bad geo rank
		preSignalAction();

		//ausgabe im Fehlerfall
		m_oPipeOutLength.signal(oGeoOutLength); // invoke linked filter(s)
		m_oPipeOutDeviation.signal(oGeoOutDeviation);
		m_oPipeOutTrackedLength.signal(oGeoOutTrackedLength); // invoke linked filter(s)
		m_oPipeOutGapDetected.signal(oGeoOutGapDetected);
		return; // RETURN
	}


	//Verbeitung
	SmpTrafo oSmpTrafoLeft = rGeoPosX1In.context().trafo();
	SmpTrafo oSmpTrafoRight = rGeoPosX2In.context().trafo();
	double oDiffLeft = static_cast<double>(oSmpTrafoLeft->dx() - m_oSpTrafo->dx());  // m_oSpTrafo entspricht der Transformation der Laserlinie
	double oDiffRight = static_cast<double>(oSmpTrafoRight->dx() - m_oSpTrafo->dx());

	//Eingangswerte aus den Arrays holen
	m_oPositionInX1	= rPosX1In.getData().front() + oDiffLeft;
	m_oPositionInX2	= rPosX2In.getData().front() + oDiffRight;

	//Ausgabe:
	//std::cout<<"seamQuality- xLeft, xRight: "<<m_oPositionInX1<<" "<<m_oPositionInX2<<std::endl;

	// Now do the actual image processing
	calcLength(rArrayIn, m_oDirection, m_oLengthOut, m_oTrackedLengthOut, m_oGapDetectedOut);
	//calcDeviation();


	//const unsigned int	oNumber = rArrayIn.size();

	//if (m_oFitMethod > 0)
	//{
	//	// Berechnen der max. Abweichung der tatsaechlichen Raupe von
	//	//einer angelegten Parabel
	//}



	// senden

	const GeoDoublearray	oGeoOutLength(rLineIn.context(), m_oLengthOut, rLineIn.analysisResult(), rLineIn.rank());
	const GeoDoublearray	oGeoOutDeviation(rLineIn.context(), m_oDeviationOut, rLineIn.analysisResult(), rLineIn.rank());
	const GeoDoublearray	oGeoOutTrackedLength(rLineIn.context(), m_oTrackedLengthOut, rLineIn.analysisResult(), rLineIn.rank());
	const GeoDoublearray	oGeoOutGapDetected(rLineIn.context(), m_oGapDetectedOut, rLineIn.analysisResult(), rLineIn.rank());

	preSignalAction();

	m_oPipeOutLength.signal(oGeoOutLength);
	m_oPipeOutDeviation.signal(oGeoOutDeviation);
	m_oPipeOutTrackedLength.signal(oGeoOutTrackedLength);
	m_oPipeOutGapDetected.signal(oGeoOutGapDetected);




} // proceedGroup


void SeamQuality::calcLength(const geo2d::VecDoublearray &p_rLineIn,
							SearchDirType p_oSearchType,
							geo2d::Doublearray &p_rLengthOut,
							geo2d::Doublearray &p_rTrackedLengthOut,
							geo2d::Doublearray &p_rGapDetectedOut)
{
	const unsigned int	oNbLines = p_rLineIn.size();

	p_rLengthOut.assign(oNbLines, 0.0); // if the size of the output signal is not equal to the input line size, resize
	p_rTrackedLengthOut.assign(oNbLines, 0.0); // if the size of the output signal is not equal to the input line size, resize
	p_rGapDetectedOut.assign(oNbLines, 0.0); // if the size of the output signal is not equal to the input line size, resize
	for (unsigned int lineN = 0; lineN < oNbLines; ++lineN)
	{ // loop over N lines
		const Doublearray	&rLineIn = p_rLineIn[lineN];

		double				&rLengthOut = p_rLengthOut.getData()[lineN];
		int					&rLengthOutRank = p_rLengthOut.getRank()[lineN];

		int                 xStart = static_cast<int>(m_oPositionInX1);
		int                 xEnd = static_cast<int>(m_oPositionInX2);

		if (xEnd < xStart)//swap
		{
			int d  = xStart;
			xStart = xEnd;
			xEnd = d;
		}

		const std::vector<double> &value = rLineIn.getData();
		const std::vector<int> 	  &rank = rLineIn.getRank();
		int lineLength = value.size();
		if (lineLength == 0) lineLength = 1;

		// zunaechst fuer neue Zaehlart beschneiden
		if (xStart < 0) xStart = 0;
		if (xEnd > lineLength - 1) xEnd = lineLength - 1;
		int numPoints = xEnd - xStart + 1;
		if (numPoints <= 0) numPoints = 1;

		int counter = 0;

		for (int i = xStart; i <= xEnd; i++)
		{
			if (rank[i] > 0) counter++;
		}

		double validTrackingPoints;
		if (counter == numPoints)
		{
			validTrackingPoints = 100.0;
		}
		else
		{
			validTrackingPoints = (100.00 * counter) / (double)numPoints;
		}

		p_rTrackedLengthOut.getData()[lineN] = validTrackingPoints;
		p_rTrackedLengthOut.getRank()[lineN] = 255;

		int gapDetected = validTrackingPoints >= m_oGapThreshold ? 0 : 1;

		p_rGapDetectedOut.getData()[lineN] = gapDetected;
		p_rGapDetectedOut.getRank()[lineN] = 255;

		//if ( (xStart<2) || (xEnd > dd-2 ) )
		//{
		//	rLengthOut = 0.0;
		//	rLengthOutRank = 0;
		//	return;
		//}

		// Abfrage geaendert, so dass mehr Eingangswerte moeglich sind
		if (xStart < 2) xStart = 2;
		if (xEnd > lineLength - 2) xEnd = lineLength - 2;



		//std::cout<<"seamQuality -- start,stop: "<<xStart<<" "<<xEnd<<std::endl;
		//find length of line rLineIn - length liefert ein pair mit wert und rank
		auto result = length(rLineIn, xStart, xEnd, m_oMaxJump, m_oMaxDistance,m_oJumpPos, m_oDirection);


		rLengthOut = std::get<eData>(result);
		rLengthOutRank = std::get<eRank>(result);

		//std::cout<<"seamQuality -- rLengthOut "<<rLengthOut<<std::endl;


	} // for
} // calcLength


std::pair<double, int> length(const geo2d::Doublearray &rLineIn,
								const int &xStart,
								const int &xEnd,
								const int &maxJump,
								const int &maxDistance,
								geo2d::TPoint<double> &JumpPos,
								SearchDirType p_oDirection)
{
	int length = 0;
	double dLength = 0.0;
	bool firstpoint = false;
	//double dummy = 0.0;
	//double a = 0.0;
	//double b = 0.0;
	int ctr = 0;

	// wie kommt man an den ersten wert der rLineIn ??
	const std::vector<double> &value = rLineIn.getData();
	const std::vector<int> 	  &rank = rLineIn.getRank();

	// rank der Laserlinie an der stelle 0
	std::make_pair(0.0, rank[0]);

	//laenge rechnen :
	//von inks
	if (p_oDirection == 0)
	{
		for (int x = xStart; x< xEnd-1; ++x)
		{
			if (!firstpoint && rank[x] > 0)
				firstpoint = true; //first valid point:
			if (firstpoint && rank[x]>0 && rank[x-1]> 0)
			{
				if (     std::abs(value[x] - value[x - 1]) < maxJump)
					length++;
				else{
					JumpPos.x = x;
					JumpPos.y = value[x];
					x = xEnd;
				}

			}
			else if (firstpoint && rank[x] == 0)
			{
				// Weiterlaufen bis zum naechsten gueltigen rank Wert:
				ctr = 0;
				for (int i = x; i < xEnd - 1; ++i)
				{
					if (rank[i] == 0)
					{
						ctr++;
						if (ctr > maxDistance)
						{

							JumpPos.x = x;
							JumpPos.y = value[x - 1]; // links sollte der letzte Wert noch gueltigen rank haben...
							i = xEnd; //raus aus der loop
							x = xEnd;
						}
					}
					else if (std::abs(value[i + 1] - value[i]) > maxJump)
					{
						JumpPos.x = x;
						JumpPos.y = value[x - 1]; // von links sollte der letzte Wert noch gueltigen rank haben...
						x = xEnd;
						i = xEnd;
					}
					else
					{
						x = i;
						i = xEnd;
					}

				}//for i

			}//else if

		}
	}
	else
	{
		for (int x = xEnd; x > xStart; --x)
		{
			if (!firstpoint && rank[x] > 0)
				firstpoint = true; //first valid point:
			if (firstpoint && rank[x] > 0 && rank[x+1] > 0)
			{
				if( std::abs(value[x] - value[x+1]) < maxJump)
					length++;
				else{
					JumpPos.x = x;
					JumpPos.y = value[x];
					x = 0;
				}
			}
			else if (firstpoint && rank[x] == 0) //Laserlinie hat kein gueltigen Werte mehr
			{
				// Weiterlaufen bis zum naechsten gueltigen rank Wert:
				ctr = 0;
				for (int i = x; i > xStart; --i)
				{
					if (rank[i] == 0)
					{
						ctr++;
						if (ctr > maxDistance)
						{

							JumpPos.x = x;            //Stoerung hat bei x begonnen
							JumpPos.y = value[x + 1]; // von links sollte der letzte Wert noch gueltigen rank haben...
							x = xStart;
							i = xStart; //raus aus der loop
						}
					}
					else if (std::abs(value[i + 1] - value[i]) > maxJump)
					{
						JumpPos.x = x;
						JumpPos.y = value[x + 1]; // von links sollte der letzte Wert noch gueltigen rank haben...
						x = xStart;
						i = xStart;
					}
					else
					{
						x = i;
						i = 1;
					}

				}//for i


			}

		}
	}

	/*if (length > 0)
	{
		dLength = ((double)length / (double)value.size()) * 100.0;
	}*/

	//std::cout<<"seam Quality -- length "<<length<<std::endl;
	dLength = static_cast<double>(length);
	return std::make_pair(dLength, rank[length]);

}

/*virtual*/ void
SeamQuality::arm(const fliplib::ArmStateBase& state) {
	if (state.getStateID() == eSeamStart)
	{
		m_oSpTrafo = nullptr;
	}
} // arm

} // namespace filter
} // namespace precitec
