/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS/CB
 * 	@date		2021
 * 	@brief 		This filter calculates the seam pos out of the laserline, the image and to lines for lap joint seams (Audi)
 */

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include <filter/algoArray.h>
#include "module/moduleLogger.h"
#include <fliplib/TypeToDataTypeImpl.h>
// local includes
#include "lapJointXT.h"

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string LapJointXT::m_oFilterName 		= std::string("LapJointXT");
const std::string LapJointXT::PIPENAME_SEAMPOS_OUT	= std::string("SeamPositionOut");
const std::string LapJointXT::PIPENAME_SEAMLEFT_OUT	= std::string("SeamLeft");
const std::string LapJointXT::PIPENAME_SEAMRIGHT_OUT	= std::string("SeamRight");
const std::string LapJointXT::PIPENAME_AREALEFT_OUT	= std::string("AreaLeft");
const std::string LapJointXT::PIPENAME_AREARIGHT_OUT	= std::string("AreaRight");

LapJointXT::LapJointXT() :
	TransformFilter( LapJointXT::m_oFilterName, Poco::UUID{"A750890A-9DB8-448A-989A-0362B4AB40B6"} ),
	m_pPipeInImageFrame( NULL ),
	m_pPipeInLaserLine( NULL ),
	m_pPipeInLeftLineSlope( NULL ),
	m_pPipeInLeftLineYIntercept( NULL ),
	m_pPipeInRightLineSlope( NULL ),
	m_pPipeInRightLineYIntercept( NULL ),
	m_pPipeInLeftMaxLineDiff ( NULL ),
	m_pPipeInRightMaxLineDiff ( NULL ),

	m_oSeamAreaStart( 0 ),
	m_oSeamAreaEnd( 100 ),
	m_oThresholdLeft( 10 ),
	m_oThresholdRight( 10 ),
	m_oNumberOfPixLeft(10),
	m_oNumberOfPixRight(10),

	m_oLineThreshold(255),
	m_oMaxLineDiff_left(1),
    m_oMaxLineDiff_right(1)
{
	m_pPipeOutSeamPos = new SynchronePipe< interface::GeoDoublearray > ( this, LapJointXT::PIPENAME_SEAMPOS_OUT );
	m_pPipeOutSeamLeft = new SynchronePipe< interface::GeoDoublearray > ( this, LapJointXT::PIPENAME_SEAMLEFT_OUT );
	m_pPipeOutSeamRight = new SynchronePipe< interface::GeoDoublearray > ( this, LapJointXT::PIPENAME_SEAMRIGHT_OUT );
	m_pPipeOutAreaLeft = new SynchronePipe< interface::GeoDoublearray > ( this, LapJointXT::PIPENAME_AREALEFT_OUT );
	m_pPipeOutAreaRight = new SynchronePipe< interface::GeoDoublearray > ( this, LapJointXT::PIPENAME_AREARIGHT_OUT );

	// Set default values of the parameters of the filter
	parameters_.add("Mode",				Parameter::TYPE_int, m_oMode);
	parameters_.add("SeamAreaStart",    Parameter::TYPE_int, m_oSeamAreaStart);
	parameters_.add("SeamAreaEnd",		Parameter::TYPE_int, m_oSeamAreaEnd);
	parameters_.add("ThresholdLeft",    Parameter::TYPE_int, m_oThresholdLeft);
	parameters_.add("ThresholdRight",	Parameter::TYPE_int, m_oThresholdRight);
	parameters_.add("NumberOfPixLeft", Parameter::TYPE_int, m_oNumberOfPixLeft);
	parameters_.add("NumberOfPixRight", Parameter::TYPE_int, m_oNumberOfPixRight);

	parameters_.add("LineThreshold", Parameter::TYPE_int, m_oLineThreshold);
	//parameters_.add("MaxLineDiff", Parameter::TYPE_int, m_oMaxLineDiff);

	m_lastSeamLeft = m_lastSeamRight = -1;

    setInPipeConnectors({{Poco::UUID("BF66A464-90C0-4D7A-BE1A-614F31535A16"), m_pPipeInImageFrame, "ImageFrameIn", 1, "ImageFrame"},
    {Poco::UUID("1EFE5482-2B7F-4185-8FA1-169B9B31938A"), m_pPipeInLaserLine, "LaserLine", 1, "LaserLine"},
    {Poco::UUID("A2A70387-4BEF-4683-9E5B-9AE1B221F16D"), m_pPipeInLeftLineSlope, "LeftSlopeIn", 1, "LeftSlope"},
    {Poco::UUID("9DD3E6FB-E52C-4F80-A224-7DC8A334A99B"), m_pPipeInLeftLineYIntercept, "LeftYInterceptIn", 1, "LeftYIntercept"},
    {Poco::UUID("8C0D4788-029C-44C7-92C6-1D69E757F762"), m_pPipeInRightLineSlope, "RightSlopeIn", 1, "RightSlope"},
    {Poco::UUID("528B32A2-CD7F-4635-A5FB-9FBFA3EB342D"), m_pPipeInRightLineYIntercept, "RightYInterceptIn", 1, "RightYIntercept"},
    {Poco::UUID("CB8C2174-4F03-4334-BF56-53705B42AC7C"), m_pPipeInLeftMaxLineDiff, "LeftMaxLineDiffIn", 1, "LeftMaxLineDiff"},
    {Poco::UUID("3DF997AE-7E4A-49B5-82F2-CCA4D2F14A25"), m_pPipeInRightMaxLineDiff, "RightMaxLineDiffIn", 1, "RightMaxLineDiff"}});

    setOutPipeConnectors({{Poco::UUID("15910A79-DB84-42B2-822F-2D6C6B2A02E3"), m_pPipeOutSeamPos, PIPENAME_SEAMPOS_OUT, 0, ""},
    {Poco::UUID("759DF280-8ACA-4E9A-B3C9-F73F7D15C168"), m_pPipeOutSeamLeft, PIPENAME_SEAMLEFT_OUT, 0, ""},
    {Poco::UUID("53EC7532-F079-4107-B487-22F995D32FF1"), m_pPipeOutSeamRight, PIPENAME_SEAMRIGHT_OUT, 0, ""},
    {Poco::UUID("836B8F82-9D9B-4036-816D-B603225D038E"), m_pPipeOutAreaLeft, PIPENAME_AREALEFT_OUT, 0, ""},
    {Poco::UUID("779BD7AD-4DE3-41F4-BD5E-1DF3EE156A38"), m_pPipeOutAreaRight, PIPENAME_AREARIGHT_OUT, 0, ""}});
    setVariantID(Poco::UUID("34F6AD7D-E877-40FF-9CFD-7A1C4DF3D69E"));
} // LineProfile

LapJointXT::~LapJointXT()
{
	delete m_pPipeOutSeamPos;
	delete m_pPipeOutSeamLeft;
	delete m_pPipeOutSeamRight;
	delete m_pPipeOutAreaLeft;
	delete m_pPipeOutAreaRight;
} // ~LineProfile

void LapJointXT::setParameter()
{
	TransformFilter::setParameter();
	m_oMode				= parameters_.getParameter("Mode").convert<int>();
	m_oSeamAreaStart    = parameters_.getParameter("SeamAreaStart").convert<int>();
	m_oSeamAreaEnd		= parameters_.getParameter("SeamAreaEnd").convert<int>();
	m_oThresholdLeft	= parameters_.getParameter("ThresholdLeft").convert<int>();
	m_oThresholdRight	= parameters_.getParameter("ThresholdRight").convert<int>();
	m_oNumberOfPixLeft  = parameters_.getParameter("NumberOfPixLeft").convert<int>();
	m_oNumberOfPixRight = parameters_.getParameter("NumberOfPixRight").convert<int>();

	m_oLineThreshold    = parameters_.getParameter("LineThreshold").convert<int>();
	//m_oMaxLineDiff      = parameters_.getParameter("MaxLineDiff").convert<int>();

} // setParameter

bool LapJointXT::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.type() == typeid(GeoVecDoublearray) )
		m_pPipeInLaserLine  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);

	if ( p_rPipe.tag() == "ImageFrame" )
		m_pPipeInImageFrame  = dynamic_cast< SynchronePipe < ImageFrame > * >(&p_rPipe);
	if ( p_rPipe.tag() == "LeftSlope" )
		m_pPipeInLeftLineSlope = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	if ( p_rPipe.tag() == "LeftYIntercept" )
		m_pPipeInLeftLineYIntercept = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	if ( p_rPipe.tag() == "RightSlope" )
		m_pPipeInRightLineSlope = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	if ( p_rPipe.tag() == "RightYIntercept" )
		m_pPipeInRightLineYIntercept = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	if ( p_rPipe.tag() == "LeftMaxLineDiff" )
		m_pPipeInLeftMaxLineDiff = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	if ( p_rPipe.tag() == "RightMaxLineDiff" )
		m_pPipeInRightMaxLineDiff = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
    
	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe

void LapJointXT::paint()
{
	if(m_oVerbosity < eLow  || m_oSpTrafo.isNull())
	{
		return;
	} // if

	//int ydiff = m_overlayMax - m_overlayMin;
	//if (ydiff <= 0) return;

	//const int yo = 20; // Malbereich in y
	//const int yu = 100;

	try
	{
		int yval, yval1=0, yval2=0; //, xStart, xEnd;

		const Trafo		&rTrafo(*m_oSpTrafo);
		OverlayCanvas	&rCanvas(canvas<OverlayCanvas>(m_oCounter));
		OverlayLayer	&rLayerContour(rCanvas.getLayerContour());

		yval = 15;
		rLayerContour.add(new OverlayLine(rTrafo(Point(m_seamStartX, yval)), rTrafo(Point(m_seamEndX, yval)), Color::Blue()));
		rLayerContour.add(new OverlayLine(rTrafo(Point(m_seamStartX, yval-3)), rTrafo(Point(m_seamStartX, yval+3)), Color::Blue()));
		rLayerContour.add(new OverlayLine(rTrafo(Point(m_seamEndX, yval-3)), rTrafo(Point(m_seamEndX, yval+3)), Color::Blue()));

		if (m_doLeft)
		{
			yval1 = (int)(0.5 + m_resultSeamLeft * m_leftLineSlope + m_leftLineYIntercept); // Y-Wert aus Geradeberechnung
			rLayerContour.add(new OverlayCross(rTrafo(Point(m_resultSeamLeft, yval1)), Color::Red()));
			//if (m_resultSeamLeft != m_correctedSeamLeft)
			//{
			//	rLayerContour.add(new OverlayCross(rTrafo(Point(m_correctedSeamLeft, int(40))), Color::Red()));
			//	rLayerContour.add(new OverlayLine(rTrafo(Point(m_resultSeamLeft, int(40))), rTrafo(Point(m_correctedSeamLeft, int(40))), Color::Red()));
			//}
		}

		if (m_doRight)
		{
			yval2 = (int)(0.5 + m_resultSeamRight * m_rightLineSlope + m_rightLineYIntercept); // Y-Wert aus Geradeberechnung
			rLayerContour.add(new OverlayCross(rTrafo(Point(m_resultSeamRight, yval2)), Color::Green()));
			//if (m_resultSeamRight != m_correctedSeamRight)
			//{
			//	rLayerContour.add(new OverlayCross(rTrafo(Point(m_correctedSeamRight, int(40))), Color::Green()));
			//	rLayerContour.add(new OverlayLine(rTrafo(Point(m_resultSeamRight, int(40))), rTrafo(Point(m_correctedSeamRight, int(40))), Color::Green()));
			//}
		}

		yval = (yval1 + yval2) / 2;
		if (m_doLeft && m_doRight) rLayerContour.add(new OverlayCross(rTrafo(Point(m_resultSeamPos, yval)), Color::Orange()));

		//if (m_paintA != 0 || m_paintB != 0) // Parabel zum Zeichnen
		//{
		//	for (int i=m_resultSeamPos-100; i<m_resultSeamPos+100; i++)
		//	{
		//		yval = (int)(0.5 + m_paintA*i*i + m_paintB*i + m_paintC);
		//		rLayerContour.add(new OverlayPoint(rTrafo(Point(i, yval)), Color::Yellow()));
		//	}
		//}

		//for (int i=0; i<m_numberOfDebugPoints; i++) rLayerContour.add(new OverlayPoint(rTrafo(Point(i*10+10, int(10))), Color::Yellow()));


	}
	catch(...)
	{
		return;
	}
} // paint


void LapJointXT::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeInLaserLine != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor

	//m_numberOfDebugPoints = 0;

	setModeVariables();

	geo2d::Doublearray oOutSeamPos;
	geo2d::Doublearray oOutSeamLeft;
	geo2d::Doublearray oOutSeamRight;
	geo2d::Doublearray oOutAreaLeft;
	geo2d::Doublearray oOutAreaRight;

	geo2d::Doublearray oOutA;
	geo2d::Doublearray oOutB;
	geo2d::Doublearray oOutC;

	//m_overlayMin = 1000000;
	//m_overlayMax = -1000000;

	// Read out image frame from pipe
	const ImageFrame& rFrameIn = m_pPipeInImageFrame->read(m_oCounter);

	try
	{
		m_oSpTrafo = rFrameIn.context().trafo();
		// Extract actual image and size
		const BImage &rImageIn = rFrameIn.data();

		// Read-out laserline
		const GeoVecDoublearray& rLaserLineIn = m_pPipeInLaserLine->read(m_oCounter);
		m_oSpTrafo = rLaserLineIn.context().trafo();
		// And extract byte-array
		const VecDoublearray& rLaserarray = rLaserLineIn.ref();
		// input validity check

		const GeoDoublearray& leftLineSlopeIn = m_pPipeInLeftLineSlope->read(m_oCounter);
		const Doublearray leftLineSlope = leftLineSlopeIn.ref();
		m_leftLineSlope = leftLineSlope.getData()[0];

		const GeoDoublearray& leftLineYInterceptIn = m_pPipeInLeftLineYIntercept->read(m_oCounter);
		const Doublearray leftLineYIntercept = leftLineYInterceptIn.ref();
		m_leftLineYIntercept = leftLineYIntercept.getData()[0];

		const GeoDoublearray& rightLineSlopeIn = m_pPipeInRightLineSlope->read(m_oCounter);
		const Doublearray rightLineSlope = rightLineSlopeIn.ref();
		m_rightLineSlope = rightLineSlope.getData()[0];

		const GeoDoublearray& rightLineYInterceptIn = m_pPipeInRightLineYIntercept->read(m_oCounter);
		const Doublearray rightLineYIntercept = rightLineYInterceptIn.ref();
		m_rightLineYIntercept = rightLineYIntercept.getData()[0];
        
		const GeoDoublearray& LeftMaxLineDiffIn = m_pPipeInLeftMaxLineDiff->read(m_oCounter);
		const Doublearray LeftMaxLineDiff = LeftMaxLineDiffIn.ref();
		m_oMaxLineDiff_left = LeftMaxLineDiff.getData()[0];
		
		const GeoDoublearray& RightMaxLineDiffIn = m_pPipeInRightMaxLineDiff->read(m_oCounter);
		const Doublearray RightMaxLineDiff = RightMaxLineDiffIn.ref();
		m_oMaxLineDiff_right = RightMaxLineDiff.getData()[0];

		if (inputIsInvalid(rLaserLineIn))
		{
			oOutSeamPos.getData().push_back(0);
			oOutSeamPos.getRank().push_back(0);
			oOutSeamLeft.getData().push_back(0);
			oOutSeamLeft.getRank().push_back(0);
			oOutSeamRight.getData().push_back(0);
			oOutSeamRight.getRank().push_back(0);
			oOutAreaLeft.getData().push_back(0);
			oOutAreaLeft.getRank().push_back(0);
			oOutAreaRight.getData().push_back(0);
			oOutAreaRight.getRank().push_back(0);

			const GeoDoublearray &rSeamPos = GeoDoublearray(rFrameIn.context(), oOutSeamPos, rLaserLineIn.analysisResult(), interface::NotPresent);
			const GeoDoublearray &rSeamLeft = GeoDoublearray(rFrameIn.context(), oOutSeamLeft, rLaserLineIn.analysisResult(), interface::NotPresent);
			const GeoDoublearray &rSeamRight = GeoDoublearray(rFrameIn.context(), oOutSeamRight, rLaserLineIn.analysisResult(), interface::NotPresent);
			const GeoDoublearray &rAreaLeft = GeoDoublearray(rFrameIn.context(), oOutAreaLeft, rLaserLineIn.analysisResult(), interface::NotPresent);
			const GeoDoublearray &rAreaRight = GeoDoublearray(rFrameIn.context(), oOutAreaRight, rLaserLineIn.analysisResult(), interface::NotPresent);
			preSignalAction();
			m_pPipeOutSeamPos->signal(rSeamPos);
			m_pPipeOutSeamLeft->signal(rSeamLeft);
			m_pPipeOutSeamRight->signal(rSeamRight);
			m_pPipeOutAreaLeft->signal(rAreaLeft);
			m_pPipeOutAreaRight->signal(rAreaRight);

			return; // RETURN
		}

		// Now do the actual image processing
		calcSeam(rImageIn, rLaserarray, oOutSeamPos, oOutSeamLeft, oOutSeamRight, oOutAreaLeft, oOutAreaRight);

		// Now calculate the seam parabel
		//calcParabel(rImageIn, rLaserarray, oOutSeamLeft, oOutSeamRight, oOutA, oOutB, oOutC);

		m_correctedSeamLeft = m_resultSeamLeft;
		m_correctedSeamRight = m_resultSeamRight;

		if (false) // es gibt einen vorherigen Wert
		{ // => Parabel-Zeug in ParabelFit ausgelagert
			int lastWidth = m_lastSeamRight - m_lastSeamLeft;
			int curWidth = m_resultSeamRight - m_resultSeamLeft;

			if ( lastWidth - curWidth > 5) // Nahtbreite ploetzlich um mind. 10 Pix schmaler
			{
				if (m_resultSeamLeft - m_lastSeamLeft > 3) // linker Rand um mind. 3 nach rechts gewandert
				{
					int count=0;
					for (int i=m_resultSeamLeft; i>=m_lastSeamLeft; i--)
					{
						m_correctedSeamLeft = i;

						int parabelY = (int)(m_paintA*i*i + m_paintB*i + m_paintC + 0.5);

						if (parabelY >= rImageIn.height()) //errechneter Punkt ausserhalb des Bildes
						{
							count++;
							continue;
						}

						if (parabelY < 0)  //errechneter Punkt ausserhalb des Bildes
						{
							count++;
							continue;
						}

						if (rImageIn[parabelY][i] < 50)
						{
							count++;
						}

						if (count>=3) break;
					}
				}

				if (m_lastSeamRight - m_resultSeamRight > 3) // rechter Rand um mind. 3 nach links gewandert
				{
					int count=0;
					for (int i=m_resultSeamRight; i<=m_lastSeamRight; i++)
					{
						m_correctedSeamRight = i;
						int parabelY = (int)(m_paintA*i*i + m_paintB*i + m_paintC + 0.5);

						//m_numberOfDebugPoints++;

						if (parabelY >= rImageIn.height()) //errechneter Punkt ausserhalb des Bildes
						{
							count++;
							continue;
						}

						//m_numberOfDebugPoints++;

						if (parabelY < 0)  //errechneter Punkt ausserhalb des Bildes
						{
							count++;
							continue;
						}

						//m_numberOfDebugPoints++;

						if (rImageIn[parabelY][i] < 50)
						{
							count++;
						}

						if (count>=3) break;
					}
				}
			}

		}

		m_lastSeamLeft = m_correctedSeamLeft;
		m_lastSeamRight = m_correctedSeamRight;

		// Create a new byte array, and put the global context into the resulting profile
		const auto oAnalysisResult = rFrameIn.analysisResult() == AnalysisOK ? AnalysisOK : rFrameIn.analysisResult(); // replace 2nd AnalysisOK by your result type
		const GeoDoublearray &rGeoSeamPos = GeoDoublearray(rLaserLineIn.context(), oOutSeamPos, oAnalysisResult, filter::eRankMax);
		const GeoDoublearray &rGeoSeamLeft = GeoDoublearray(rLaserLineIn.context(), oOutSeamLeft, oAnalysisResult, filter::eRankMax);
		const GeoDoublearray &rGeoSeamRight = GeoDoublearray(rLaserLineIn.context(), oOutSeamRight, oAnalysisResult, filter::eRankMax);

		const GeoDoublearray &rGeoAreaLeft = GeoDoublearray(rLaserLineIn.context(), oOutAreaLeft, oAnalysisResult, filter::eRankMax);
		const GeoDoublearray &rGeoAreaRight = GeoDoublearray(rLaserLineIn.context(), oOutAreaRight, oAnalysisResult, filter::eRankMax);

		preSignalAction();
		m_pPipeOutSeamPos->signal(rGeoSeamPos);
		m_pPipeOutSeamLeft->signal(rGeoSeamLeft);
		m_pPipeOutSeamRight->signal(rGeoSeamRight);
		m_pPipeOutAreaLeft->signal(rGeoAreaLeft);
		m_pPipeOutAreaRight->signal(rGeoAreaRight);
	}
	catch (...)
	{
		oOutSeamPos.getData().push_back(0);
		oOutSeamPos.getRank().push_back(0);
		oOutSeamLeft.getData().push_back(0);
		oOutSeamLeft.getRank().push_back(0);
		oOutSeamRight.getData().push_back(0);
		oOutSeamRight.getRank().push_back(0);
		oOutAreaLeft.getData().push_back(0);
		oOutAreaLeft.getRank().push_back(0);
		oOutAreaRight.getData().push_back(0);
		oOutAreaRight.getRank().push_back(0);

		const GeoDoublearray &rSeamPos = GeoDoublearray(rFrameIn.context(), oOutSeamPos, rFrameIn.analysisResult(), interface::NotPresent);
		const GeoDoublearray &rSeamLeft = GeoDoublearray(rFrameIn.context(), oOutSeamLeft, rFrameIn.analysisResult(), interface::NotPresent);
		const GeoDoublearray &rSeamRight = GeoDoublearray(rFrameIn.context(), oOutSeamRight, rFrameIn.analysisResult(), interface::NotPresent);
		const GeoDoublearray &rAreaLeft = GeoDoublearray(rFrameIn.context(), oOutAreaLeft, rFrameIn.analysisResult(), interface::NotPresent);
		const GeoDoublearray &rAreaRight = GeoDoublearray(rFrameIn.context(), oOutAreaRight, rFrameIn.analysisResult(), interface::NotPresent);
		preSignalAction();
		m_pPipeOutSeamPos->signal(rSeamPos);
		m_pPipeOutSeamLeft->signal(rSeamLeft);
		m_pPipeOutSeamRight->signal(rSeamRight);
		m_pPipeOutAreaLeft->signal(rAreaLeft);
		m_pPipeOutAreaRight->signal(rAreaRight);

		return; // RETURN
	}
} // proceedGroup

void LapJointXT::setModeVariables()
{
	switch (m_oMode)
	{
	case 0:
		m_doLeft = true;
		m_doRight = true;
		m_isNoSeamDetection = false;
		m_isTracking = false;
		break;
	case 1:
		m_doLeft = true;
		m_doRight = false;
		m_isNoSeamDetection = false;
		m_isTracking = false;
		break;
	case 2:
		m_doLeft = false;
		m_doRight = true;
		m_isNoSeamDetection = false;
		m_isTracking = false;
		break;
	case 11:
		m_doLeft = true;
		m_doRight = true;
		m_isNoSeamDetection = true;
		m_isTracking = false;
		break;
	case 20:
		m_doLeft = true;
		m_doRight = true;
		m_isNoSeamDetection = false;
		m_isTracking = true;
		m_AbortType = 0;
		break;
	case 21:
		m_doLeft = true;
		m_doRight = true;
		m_isNoSeamDetection = false;
		m_isTracking = true;
		m_AbortType = 1;
		break;
	case 22:
		m_doLeft = true;
		m_doRight = true;
		m_isNoSeamDetection = false;
		m_isTracking = true;
		m_AbortType = 2;
		break;
   	case 30:
		m_doLeft = true;
		m_doRight = true;
		m_isNoSeamDetection = false;
		m_isTracking = true;
		m_AbortType = 0;
		m_isTrackingWithGap = true;
        break;
	case 31:
		m_doLeft = true;
		m_doRight = true;
		m_isNoSeamDetection = false;
		m_isTracking = true;
		m_AbortType = 1;
		m_isTrackingWithGap = true;
		break;
	case 32:
		m_doLeft = true;
		m_doRight = true;
		m_isNoSeamDetection = false;
		m_isTracking = true;
		m_AbortType = 2;
		m_isTrackingWithGap = true;
		break;
	}

}

void LapJointXT::calcSeam( const image::BImage &p_rImageIn, const geo2d::VecDoublearray &p_rLaserLineIn,
	geo2d::Doublearray &seamPos, geo2d::Doublearray &seamLeft, geo2d::Doublearray &seamRight, geo2d::Doublearray &areaLeft, geo2d::Doublearray &areaRight )
{
	const unsigned int	oNbLines	= p_rLaserLineIn.size();
	try
	{
		int imageWidth = p_rImageIn.width();
		int imageHeight = p_rImageIn.height();

		for (unsigned int lineN = 0; lineN < oNbLines; ++lineN)
		{ // loop over N lines

			const auto& rLaserLineIn_Data = p_rLaserLineIn[lineN].getData();
			const auto& rLaserLineIn_Rank = p_rLaserLineIn[lineN].getRank();

			m_seamStartX = (int)(0.5 + rLaserLineIn_Data.size() * m_oSeamAreaStart / 100.0);
			m_seamEndX = (int)(0.5 + rLaserLineIn_Data.size() * m_oSeamAreaEnd / 100.0);

			int middle = (m_seamStartX + m_seamEndX) / 2;
			double middleYleftLine = middle * m_leftLineSlope + m_leftLineYIntercept;
			double middleYrightLine = middle * m_rightLineSlope + m_rightLineYIntercept;
			bool isLeftHigher = (middleYrightLine > middleYleftLine);

			m_resultSeamLeft = 0;
			m_resultSeamPos = 0;
			m_resultSeamRight = 0;

			double curDiff, calcY;
			double diffSumLeft = 0, diffSumRight = 0;
			int lineY, rank;
			int greyVal;

			bool leftFound = false;
			bool rightFound = false;

			int counterLeft = 0;
			int counterRight = 0;
			int lastLineY = 0;

			bool hasOneFound = false;

			int lastSign = 0, curSign = 0;
			int leavePointLeft = 0;
			int leavePointRight = 0;

			m_resultSeamLeft = m_seamStartX;
			if (m_doLeft)
			{
				for (int x = m_seamStartX; x < m_seamEndX; x++) // von links nach rechts
				{
					lineY = (int)rLaserLineIn_Data[x]; // Y-Wert aus Tracking
					rank = (int)rLaserLineIn_Rank[x];

					if (!m_isTrackingWithGap)
					{
						if ((rank <= 0) && !hasOneFound) continue; // Rank ist Mist, ist aber noch nix da...
						if ((lineY <= 0) && !hasOneFound) continue;

						if ((rank <= 0) || (lineY <= 0)) // Rank ist Mist, aber vorheriger Wert ist da
						{
							lineY = lastLineY;
                            
                            if (m_isTracking) break;
						}
						else // Rank ist gut
						{
							hasOneFound = true;
							lastLineY = lineY;
						}
					}

					if (diffSumLeft == 0)
					{
						leavePointLeft = x; // merken, wo Totalsumme noch 0
					}

					calcY = x * m_leftLineSlope + m_leftLineYIntercept; // Y-Wert aus Geradeberechnung
					int calcYInt = (int)(0.5 + calcY);

					if (!m_isTracking)
					{
						// Bildpixel holen, Aufloesung beachten, koennte voellig daneben liegen
						if ((x >= 0) && (x < imageWidth) && (calcYInt >= 0) && (calcYInt < imageHeight))
							greyVal = p_rImageIn[calcYInt][x];
						else
							greyVal = 0;
					}

					curDiff = lineY - calcY; // Differenz aus tatsaechlichem Tracking und Gerade

					if (m_isTracking)
					{
						if (m_AbortType == 2) curDiff = -curDiff;
                        
                    if (lineY < 0 || m_AbortType == 0)
							curDiff = std::abs(curDiff);
					}
					else
					{
						if (!isLeftHigher) curDiff = -curDiff;
					}

					diffSumLeft += curDiff; // aktuelle Differenz auf TotalDiff hinzunehmen => kein Betrag mehr!

					if (lastSign == 0) // Gab noch kein voriges
					{
						lastSign = (curDiff > 0) ? +1 : -1;
					}

					// TotalDiff nullen bei:
					// 1. VZ-Wechsel
					// 2. zu dicht beieinander

					curSign = (curDiff > 0) ? +1 : -1;
					if ((curSign != lastSign) && !leftFound) // VZ-Wechsel
					{
						diffSumLeft = 0;
						lastSign = curSign;
					}

					if (m_isTracking)
					{
						if ((curDiff <= m_oMaxLineDiff_left) && !leftFound) // Abstand zu gering
						{
							diffSumLeft = 0;
						}
						else
						{
							m_resultSeamLeft = leavePointLeft;
							leftFound = true;
						}

						if (leftFound)
						{ // bereits gefunden.
							break;
						}
					}
					else
					{
						if ((std::abs(curDiff) <= m_oMaxLineDiff_left) && !leftFound) // Abstand zu gering
						{
							diffSumLeft = 0;
						}


						if (m_isNoSeamDetection)
						{
							if (greyVal >= m_oLineThreshold) // aktueller Pixel liegt noch auf der Linie (hat zumindest den Grauwert)
							{
								diffSumLeft = 0;
							}
						}

						if (leftFound)
						{ // bereits gefunden. Rechts davon Flaeche berechnen
							counterLeft++;
							if (counterLeft >= m_oNumberOfPixLeft) // Nu ist die Summe rechts vom gefundenen Punkt ueber n Pixel
								break;
						}
						else
						{  // noch nicht gefunden
							if (std::abs(diffSumLeft) > m_oThresholdLeft)
							{
								m_resultSeamLeft = leavePointLeft;
								leftFound = true;
								//diffSumLeft = 0; // neu anfangen => berechnet dann rechts vom gefundenen Punkt
							}
						}
					}
				}
			}

			hasOneFound = false;

			lastSign = 0, curSign = 0;
			m_resultSeamRight = m_seamEndX;
			if (m_doRight)
			{
				for (int x = m_seamEndX - 1; x > m_seamStartX; x--)  // von rechts nach links
				{
					lineY = (int)rLaserLineIn_Data[x];
					rank = (int)rLaserLineIn_Rank[x];

					if (!m_isTrackingWithGap)
					{
						if ((rank <= 0) && !hasOneFound) continue; // Rank ist Mist, ist aber noch nix da...
						if ((lineY <= 0) && !hasOneFound) continue; // => dann also weiter

						// ab hier Rank ok oder zumindest davor war schon einer

						if ((rank <= 0) || (lineY <= 0)) // Rank ist Mist, aber vorheriger Wert ist da
						{
							lineY = lastLineY;
                        
                            if (m_isTracking) break;
						}
						else // Rank ist gut => nehmen, merken
						{
							hasOneFound = true;
							lastLineY = lineY;
						}
					}

					if (diffSumRight == 0)
					{
						leavePointRight = x; // merken, wo Totalsumme noch 0
					}

					calcY = x * m_rightLineSlope + m_rightLineYIntercept;
					int calcYInt = (int)(0.5 + calcY);

					if (!m_isTracking)
					{
						// Bildpixel holen, Aufloesung beachten, koennte voellig daneben liegen
						if ((x >= 0) && (x < imageWidth) && (calcYInt >= 0) && (calcYInt < imageHeight))
							greyVal = p_rImageIn[calcYInt][x];
						else
							greyVal = 0;
					}

					curDiff = calcY - lineY;

					if (m_isTracking)
					{
						if (m_AbortType == 1) curDiff = -curDiff;

						if (lineY < 0 || m_AbortType == 0)
							curDiff = std::abs(curDiff);
					}
					else
					{
						if (!isLeftHigher) curDiff = -curDiff;
					}

					diffSumRight += curDiff; // => kein Betrag mehr

					if (lastSign == 0) // Gab noch kein voriges
					{
						lastSign = (curDiff > 0) ? +1 : -1;
					}

					// TotalDiff nullen bei:
					// 1. VZ-Wechsel
					// 2. zu dicht beieinander

					curSign = (curDiff > 0) ? +1 : -1;
					if ((curSign != lastSign) && !rightFound) // VZ-Wechsel
					{
						diffSumRight = 0;
						lastSign = curSign;
					}

					if (m_isTracking)
					{
						if ((curDiff <= m_oMaxLineDiff_right) && !rightFound) // Abstand zu gering
						{
							diffSumRight = 0;
						}
						else
						{
							m_resultSeamRight = leavePointRight;
							rightFound = true;
						}

						if (rightFound)
						{ // bereits gefunden.
							break;
						}
					}
					else
					{
						if ((std::abs(curDiff) <= m_oMaxLineDiff_right) && !rightFound) // Abstand zu gering
						{
							diffSumRight = 0;
						}

						if (m_isNoSeamDetection)
						{
							if (greyVal >= m_oLineThreshold) // aktueller Pixel liegt noch auf der Linie (hat zumindest den Grauwert)
							{
								diffSumRight = 0;
							}
						}

						if (rightFound)
						{ // bereits gefunden. Nur weitergehen, um links davon Flaeche zu berechnen
							counterRight++;
							if (counterRight >= m_oNumberOfPixRight) // Nu ist die Summe links vom gefundenen Punkt ueber n Pixel
								break;
						}
						else
						{  // noch nicht gefunden
							if (std::abs(diffSumRight) > m_oThresholdRight)
							{
								m_resultSeamRight = leavePointRight;
								rightFound = true;
								//diffSumRight = 0; // neu anfangen => berechnet dann links vom gefundenen Punkt
							}
						}
					}
				}
			}

			m_resultSeamPos = (m_resultSeamLeft + m_resultSeamRight) / 2;
			if (m_doLeft && !m_doRight)
			{
				m_resultSeamPos = m_resultSeamLeft;
				m_resultSeamRight = m_resultSeamLeft;
			}
			if (!m_doLeft && m_doRight)
			{
				m_resultSeamPos = m_resultSeamRight;
				m_resultSeamLeft = m_resultSeamRight;
			}

			seamLeft.getData().push_back(m_resultSeamLeft);
			seamRight.getData().push_back(m_resultSeamRight);
			seamPos.getData().push_back(m_resultSeamPos);
			areaLeft.getData().push_back(diffSumLeft);
			areaRight.getData().push_back(diffSumRight);

			int iSetRank = 255, iSetRankSeamPos = 255;
			if (!leftFound)
			{
				iSetRank = 0;
				iSetRankSeamPos = 127;
			}
			seamLeft.getRank().push_back(iSetRank);
			areaLeft.getRank().push_back(iSetRank);
			iSetRank = 255;
			if (!rightFound)
			{
				iSetRank = 0;
				iSetRankSeamPos = 127;
			}
			seamRight.getRank().push_back(iSetRank);
			areaRight.getRank().push_back(iSetRank);
			if (!leftFound && !rightFound)
			{
				iSetRankSeamPos = 0;
			}
			seamPos.getRank().push_back(iSetRankSeamPos);

		} // for
	}
	catch(...)
	{
		seamPos.getData().push_back(0);
		seamPos.getRank().push_back(0);
		seamLeft.getData().push_back(0);
		seamLeft.getRank().push_back(0);
		seamRight.getData().push_back(0);
		seamRight.getRank().push_back(0);
		areaLeft.getData().push_back(0);
		areaLeft.getRank().push_back(0);
		areaRight.getData().push_back(0);
		areaRight.getRank().push_back(0);
	}
} // calcSeam


} // namespace precitec
} // namespace filter
