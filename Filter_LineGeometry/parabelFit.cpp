/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		2015
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
#include "parabelFit.h"
#include "math/2D/parabolaEquation.h"

typedef precitec::math::ParabolaEquation SingleParabelFit;


using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string ParabelFit::m_oFilterName 		  = std::string("ParabelFit");
const std::string ParabelFit::PIPENAME_PARABEL_A_OUT  = std::string("ParabelAOut");
const std::string ParabelFit::PIPENAME_PARABEL_B_OUT  = std::string("ParabelBOut");
const std::string ParabelFit::PIPENAME_PARABEL_C_OUT  = std::string("ParabelCOut");
const std::string ParabelFit::PIPENAME_VALUE1_OUT  = std::string("Value1Out");
const std::string ParabelFit::PIPENAME_VALUE2_OUT = std::string("Value2Out");

ParabelFit::ParabelFit() :
	TransformFilter( ParabelFit::m_oFilterName, Poco::UUID{"E7F8974C-594E-41DC-A68B-E3CBAAE8B9C3"} ),
	m_pPipeInImageFrame( NULL ),
	m_pPipeInLaserLine( NULL ),
	m_pPipeInLeftSeam( NULL ),
	m_pPipeInRightSeam( NULL ),

	m_oMode( 0 )
{
	m_pPipeOutParabelA = new SynchronePipe< interface::GeoDoublearray > ( this, ParabelFit::PIPENAME_PARABEL_A_OUT );
	m_pPipeOutParabelB = new SynchronePipe< interface::GeoDoublearray > ( this, ParabelFit::PIPENAME_PARABEL_B_OUT );
	m_pPipeOutParabelC = new SynchronePipe< interface::GeoDoublearray > ( this, ParabelFit::PIPENAME_PARABEL_C_OUT );
	m_pPipeOutValue1 = new SynchronePipe< interface::GeoDoublearray > ( this, ParabelFit::PIPENAME_VALUE1_OUT );
	m_pPipeOutValue2 = new SynchronePipe< interface::GeoDoublearray > ( this, ParabelFit::PIPENAME_VALUE2_OUT );

	// Set default values of the parameters of the filter
	parameters_.add("Mode",				Parameter::TYPE_int, m_oMode);

    setInPipeConnectors({{Poco::UUID("F7C51DDA-3A1A-4BA8-BEDC-1EB2B351DCEF"), m_pPipeInLaserLine, "LaserLineIn", 1, "LaserLine"},
    {Poco::UUID("586DC1B2-74CC-4A32-B99A-FD75935D7383"), m_pPipeInImageFrame, "ImageFrameIn", 1, "ImageFrame"},
    {Poco::UUID("0CF0A4A9-F220-436A-B095-66DF118355A9"), m_pPipeInLeftSeam, "SeamLeftIn", 1, "SeamLeft"},
    {Poco::UUID("CC76FAF0-7C80-4A1C-BE34-014F627A51B9"), m_pPipeInRightSeam, "SeamRight", 1, "SeamRight"}});
    setOutPipeConnectors({{Poco::UUID("93F73FA8-6690-4536-B838-C41FD265EE4D"), m_pPipeOutParabelA, PIPENAME_PARABEL_A_OUT, 0, ""},
    {Poco::UUID("C390F8C4-FE83-45AC-A263-3C3EE2BC5481"), m_pPipeOutParabelB, PIPENAME_PARABEL_B_OUT, 0, ""},
    {Poco::UUID("9BBCA560-AD17-4E61-9F3C-9E2896FC5837"), m_pPipeOutParabelC, PIPENAME_PARABEL_C_OUT, 0, ""},
    {Poco::UUID("AEED0C38-9288-453F-8650-C89F7A49B0C7"), m_pPipeOutValue1, PIPENAME_VALUE1_OUT, 0, ""},
    {Poco::UUID("AFB6B434-A89B-4CBC-A037-8163C9B906A9"), m_pPipeOutValue2, PIPENAME_VALUE2_OUT, 0, ""}});
    setVariantID(Poco::UUID("0F6EEFF3-9311-469B-8D80-A146FDD5691B"));
} // LineProfile

ParabelFit::~ParabelFit()
{
	delete m_pPipeOutParabelA;
	delete m_pPipeOutParabelB;
	delete m_pPipeOutParabelC;
	delete m_pPipeOutValue1;
	delete m_pPipeOutValue2;
}

void ParabelFit::setParameter()
{
	TransformFilter::setParameter();
	m_oMode				= parameters_.getParameter("Mode").convert<int>();

} // setParameter

bool ParabelFit::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.type() == typeid(GeoVecDoublearray) )
		m_pPipeInLaserLine  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);

	if ( p_rPipe.tag() == "ImageFrame" )
		m_pPipeInImageFrame  = dynamic_cast< SynchronePipe < ImageFrame > * >(&p_rPipe);
	if ( p_rPipe.tag() == "SeamLeft" )
		m_pPipeInLeftSeam = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	if ( p_rPipe.tag() == "SeamRight" )
		m_pPipeInRightSeam = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe

void ParabelFit::paint()
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
		//int yval;

		const Trafo		&rTrafo(*m_oSpTrafo);
		OverlayCanvas	&rCanvas(canvas<OverlayCanvas>(m_oCounter));
		OverlayLayer	&rLayerContour(rCanvas.getLayerContour());

		if (m_paintA != 0 || m_paintB != 0) // Parabel zum Zeichnen
		{
			for (int i=m_paintSeamPos-100; i<m_paintSeamPos+100; i++)
			{
				int yval = (int)(0.5 + m_paintA*i*i + m_paintB*i + m_paintC);
				rLayerContour.add(new OverlayPoint(rTrafo(Point(i, yval)), Color::Yellow()));
			}
		}
	}
	catch(...)
	{
		return;
	}
} // paint


void ParabelFit::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeInLaserLine != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor

	//m_numberOfDebugPoints = 0;

	//setModeVariables();

	geo2d::Doublearray oOutParabelA;
	geo2d::Doublearray oOutParabelB;
	geo2d::Doublearray oOutParabelC;
	geo2d::Doublearray oOutValue1;
	geo2d::Doublearray oOutValue2;

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

		const GeoDoublearray& leftSeamIn = m_pPipeInLeftSeam->read(m_oCounter);
		const Doublearray leftSeam = leftSeamIn.ref();

		const GeoDoublearray& rightSeamIn = m_pPipeInRightSeam->read(m_oCounter);
		const Doublearray rightSeam = rightSeamIn.ref();

		// input validity check
		if ( (inputIsInvalid(rLaserLineIn)) || (leftSeam.getData().size()<=0) || (rightSeam.getData().size()<=0) )
		{
			oOutParabelA.getData().push_back(0);
			oOutParabelA.getRank().push_back(0);
			oOutParabelB.getData().push_back(0);
			oOutParabelB.getRank().push_back(0);
			oOutParabelC.getData().push_back(0);
			oOutParabelC.getRank().push_back(0);
			oOutValue1.getData().push_back(0);
			oOutValue1.getRank().push_back(0);
			oOutValue2.getData().push_back(0);
			oOutValue2.getRank().push_back(0);

			const GeoDoublearray &rParabelA = GeoDoublearray(rFrameIn.context(), oOutParabelA, rLaserLineIn.analysisResult(), interface::NotPresent);
			const GeoDoublearray &rParabelB = GeoDoublearray(rFrameIn.context(), oOutParabelB, rLaserLineIn.analysisResult(), interface::NotPresent);
			const GeoDoublearray &rParabelC = GeoDoublearray(rFrameIn.context(), oOutParabelC, rLaserLineIn.analysisResult(), interface::NotPresent);
			const GeoDoublearray &rValue1 = GeoDoublearray(rFrameIn.context(), oOutValue1, rLaserLineIn.analysisResult(), interface::NotPresent);
			const GeoDoublearray &rValue2 = GeoDoublearray(rFrameIn.context(), oOutValue2, rLaserLineIn.analysisResult(), interface::NotPresent);
			preSignalAction();
			m_pPipeOutParabelA->signal(rParabelA);
			m_pPipeOutParabelB->signal(rParabelB);
			m_pPipeOutParabelC->signal(rParabelC);
			m_pPipeOutValue1->signal(rValue1);
			m_pPipeOutValue2->signal(rValue2);
			return;
		}

		// Now calculate the seam parabel
		calcParabel(rImageIn, rLaserarray, leftSeam, rightSeam,	oOutParabelA, oOutParabelB, oOutParabelC, oOutValue1, oOutValue2);

		if (m_lastSeamLeft != -1) // es gibt einen vorherigen Wert
		{
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

		oOutValue1.getData().push_back(m_correctedSeamLeft);
		oOutValue1.getRank().push_back(255);
		oOutValue2.getData().push_back(m_correctedSeamRight);
		oOutValue2.getRank().push_back(255);

		// Create a new byte array, and put the global context into the resulting profile
		const auto oAnalysisResult = rFrameIn.analysisResult() == AnalysisOK ? AnalysisOK : rFrameIn.analysisResult(); // replace 2nd AnalysisOK by your result type

		const GeoDoublearray &rGeoParabelA = GeoDoublearray(rLaserLineIn.context(), oOutParabelA, oAnalysisResult, filter::eRankMax);
		const GeoDoublearray &rGeoParabelB = GeoDoublearray(rLaserLineIn.context(), oOutParabelB, oAnalysisResult, filter::eRankMax);
		const GeoDoublearray &rGeoParabelC = GeoDoublearray(rLaserLineIn.context(), oOutParabelC, oAnalysisResult, filter::eRankMax);

		const GeoDoublearray &rGeoValue1 = GeoDoublearray(rLaserLineIn.context(), oOutValue1, oAnalysisResult, filter::eRankMax); // temp. Hack
		const GeoDoublearray &rGeoValue2 = GeoDoublearray(rLaserLineIn.context(), oOutValue2, oAnalysisResult, filter::eRankMax);

		preSignalAction();
		m_pPipeOutParabelA->signal(rGeoParabelA);
		m_pPipeOutParabelB->signal(rGeoParabelB);
		m_pPipeOutParabelC->signal(rGeoParabelC);
		m_pPipeOutValue1->signal(rGeoValue1);
		m_pPipeOutValue2->signal(rGeoValue2);
	}
	catch (...)
	{
		oOutParabelA.getData().push_back(0);
		oOutParabelA.getRank().push_back(0);
		oOutParabelB.getData().push_back(0);
		oOutParabelB.getRank().push_back(0);
		oOutParabelC.getData().push_back(0);
		oOutParabelC.getRank().push_back(0);
		oOutValue1.getData().push_back(0);
		oOutValue1.getRank().push_back(0);
		oOutValue2.getData().push_back(0);
		oOutValue2.getRank().push_back(0);

		const GeoDoublearray &rParabelA = GeoDoublearray(rFrameIn.context(), oOutParabelA, rFrameIn.analysisResult(), interface::NotPresent);
		const GeoDoublearray &rParabelB = GeoDoublearray(rFrameIn.context(), oOutParabelB, rFrameIn.analysisResult(), interface::NotPresent);
		const GeoDoublearray &rParabelC = GeoDoublearray(rFrameIn.context(), oOutParabelC, rFrameIn.analysisResult(), interface::NotPresent);
		const GeoDoublearray &rValue1 = GeoDoublearray(rFrameIn.context(), oOutValue1, rFrameIn.analysisResult(), interface::NotPresent);
		const GeoDoublearray &rValue2 = GeoDoublearray(rFrameIn.context(), oOutValue2, rFrameIn.analysisResult(), interface::NotPresent);
		preSignalAction();
		m_pPipeOutParabelA->signal(rParabelA);
		m_pPipeOutParabelB->signal(rParabelB);
		m_pPipeOutParabelC->signal(rParabelC);
		m_pPipeOutValue1->signal(rValue1);
		m_pPipeOutValue2->signal(rValue2);

		return;
	}
} // proceedGroup

void ParabelFit::calcParabel(const image::BImage &p_rImageIn, const geo2d::VecDoublearray &p_rLaserLineIn, geo2d::Doublearray seamLeft, geo2d::Doublearray seamRight,
		geo2d::Doublearray & outA, geo2d::Doublearray & outB, geo2d::Doublearray & outC, geo2d::Doublearray & value1, geo2d::Doublearray & value2)
{
	const unsigned int	oNbLines	= p_rLaserLineIn.size();
	try
	{
		for (unsigned int lineN = 0; lineN < oNbLines; ++lineN)
		{ // loop over N lines

			const auto& rLaserLineIn_Data = p_rLaserLineIn[lineN].getData();
			const auto& rLaserLineIn_Rank = p_rLaserLineIn[lineN].getRank();

			if ( (seamLeft.getData().size()>lineN) && (seamRight.getData().size()>lineN) )
			{
				m_seamStartX = (int)(0.5+seamLeft.getData()[lineN]);
				m_seamEndX = (int)(0.5+seamRight.getData()[lineN]);
			}
			else
			{
				m_seamStartX = (int)(0.5+seamLeft.getData()[seamLeft.getData().size()-1]);
				m_seamEndX = (int)(0.5+seamRight.getData()[seamRight.getData().size()-1]);
			}

			if (m_seamEndX - m_seamStartX < 10)
			{
				m_seamEndX += 10;
				m_seamStartX -= 10;
			}

			int lineY, rank;

			SingleParabelFit parabelFit;

			for (int x = m_seamStartX; x < m_seamEndX; x++) // von links nach rechts
			{
				lineY = (int)rLaserLineIn_Data[x]; // Y-Wert aus Tracking
				rank = (int)rLaserLineIn_Rank[x];

				if (rank <= 0) continue;
				if (lineY <= 0) continue;

				parabelFit.AddPoint(x, lineY);
			}

			double a, b, c;
			parabelFit.CalcABC(a, b, c);

			if (lineN == 0)
			{
				m_paintSeamPos = (m_seamStartX + m_seamEndX) / 2;
				m_paintA = a;
				m_paintB = b;
				m_paintC = c;
			}

			outA.getData().push_back(a);
			outA.getRank().push_back(255);
			outB.getData().push_back(b);
			outB.getRank().push_back(255);
			outC.getData().push_back(c);
			outC.getRank().push_back(255);
		} // for
	}
	catch(...)
	{
	}
} // calcParabel




} // namespace precitec
} // namespace filter

