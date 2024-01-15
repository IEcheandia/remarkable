/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIH, Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief
 */

// Diese Header sind auch in hough.cpp verwendet worden. Sehr interessant:
// Diese Zeilen muessen hier als erstes kommen! Das liegt daran, dass <cmath> wohl bereits an anderer Stelle
// inkludiert wird, aber ohne _USE_MATH_DEFINES. Aber weil <cmath> natuerlich Inkludeguards hat,
// hat eine zweite Einbindung weiter unten keinen Effekt - also muss das hier als erstes erfolgen!
#define _USE_MATH_DEFINES						/// pi constant
#include <cmath>								/// trigonometry, pi constant


#include "HeightDifference.h"

#include <cmath>

#include "Poco/SharedPtr.h"
#include "fliplib/Parameter.h"
#include "fliplib/SynchronePipe.h"

#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "filter/algoArray.h"
#include "module/moduleLogger.h"
#include "util/calibDataSingleton.h"

#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec
{
	using namespace interface;
	using namespace image;
	using namespace geo2d;
	namespace filter
	{

using Poco::SharedPtr;
using fliplib::SynchronePipe;
using fliplib::BaseFilterInterface;
using fliplib::BasePipe;
using fliplib::TransformFilter;
using fliplib::PipeEventArgs;
using fliplib::PipeGroupEventArgs;
using fliplib::Parameter;


const std::string HeightDifference::m_oFilterName = std::string("HeightDifference");
const std::string HeightDifference::m_oPipenameOut = std::string("HeightDifference");
const std::string HeightDifference::m_oFitintervalSizeName = std::string("FitintervalSize");
const std::string HeightDifference::m_oLeftFitintervalDistanceName = std::string("LeftFitintervalDistance");
const std::string HeightDifference::m_oRightFitintervalDistanceName = std::string("RightFitintervalDistance");
const std::string HeightDifference::m_oFitintervalSubsamplingIncrementName = std::string("FitintervalSubsamplingIncrement");
const std::string HeightDifference::m_oParamTypeOfLaserLine = std::string("TypeOfLaserLine"); ///< Parameter: Type of LaserLine (e.g. FrontLaserLine, BehindLaserLine)

HeightDifference::HeightDifference() :
TransformFilter(HeightDifference::m_oFilterName, Poco::UUID{"519AC7B3-8BF4-4F15-97D5-9CC1E2B3ABB1"}),
	pipeInLineY_		( nullptr ),
	pipeInPosX_			( nullptr ),
	pipeInPosAngle_(nullptr),
	pipeOutHeightDifference_(new SynchronePipe< GeoDoublearray >(this, HeightDifference::m_oPipenameOut)),
	m_oFitintervalSize(10), // man sollte es sich angewoehnen, die Paramter grundsaetzlich im Konstruktor zu initialisieren.
	m_oLeftFitintervalDistance(10),
	m_oRightFitintervalDistance(10),
	m_oFitintervalSubsamplingIncrement(1),
	firstValidIndex(0),
	lastValidIndex(0),
	m_oTypeOfLaserLine(LaserLine::FrontLaserLine)
{
	// Parameter
	parameters_.add(m_oFitintervalSizeName, Parameter::TYPE_int, static_cast<int>(m_oFitintervalSize));
	parameters_.add(m_oLeftFitintervalDistanceName, Parameter::TYPE_int, static_cast<int>(m_oLeftFitintervalDistance));
	parameters_.add(m_oRightFitintervalDistanceName, Parameter::TYPE_int, static_cast<int>(m_oRightFitintervalDistance));
	parameters_.add(m_oFitintervalSubsamplingIncrementName, Parameter::TYPE_int, static_cast<int>(m_oFitintervalSubsamplingIncrement));

	int oLaserLineTemp = static_cast<int>(m_oTypeOfLaserLine);
	parameters_.add(HeightDifference::m_oParamTypeOfLaserLine, fliplib::Parameter::TYPE_int, oLaserLineTemp);  // Fuege den Parameter mit dem soeben initialisierten Wert hinzu.

    setInPipeConnectors({{Poco::UUID("DC2A229A-9D8E-4A46-9F5F-2A275E3EC96E"), pipeInLineY_, "LaserLine", 1, "line"},
    {Poco::UUID("0F49D9A6-081F-4BF1-91F4-161215E3112D"), pipeInPosX_, "PositionX", 1, "position_x"},
    {Poco::UUID("B7912BEE-2F06-4E87-9C04-72B85C9BB719"), pipeInPosAngle_, "Angle", 1, "angle"}});
    setOutPipeConnectors({{Poco::UUID("41B5CA3E-D1D5-486A-B7BD-12E4B4418260"), pipeOutHeightDifference_, "HeightDifference", 0, ""}});
    setVariantID(Poco::UUID("17C700ED-A6F8-4492-9453-E12B07B9C23E"));
}

HeightDifference::~HeightDifference()
{
	delete pipeOutHeightDifference_;
}

void HeightDifference::setParameter()
{
	TransformFilter::setParameter();

	// Parameter
	m_oFitintervalSize = parameters_.getParameter(m_oFitintervalSizeName).convert<int>();
	m_oLeftFitintervalDistance = parameters_.getParameter(m_oLeftFitintervalDistanceName).convert<int>();
	m_oRightFitintervalDistance = parameters_.getParameter(m_oRightFitintervalDistanceName).convert<int>();
	m_oFitintervalSubsamplingIncrement = parameters_.getParameter(m_oFitintervalSubsamplingIncrementName).convert<int>();

	int oTempLine = parameters_.getParameter(HeightDifference::m_oParamTypeOfLaserLine).convert<int>();
	m_oTypeOfLaserLine = static_cast<LaserLine>(oTempLine);

	if(m_oVerbosity >= eMedium)
	{
		wmLog(precitec::eDebug, "Filter '%s':.\n", m_oFilterName.c_str());
	}
}

void HeightDifference::arm(const fliplib::ArmStateBase& state)
{
	int ArmState = state.getStateID();

	if(m_oVerbosity >= eMedium) wmLog(eDebug, "HeightDifference armstate=%d\n",ArmState);
} // arm

void HeightDifference::paint() {
	if(m_oVerbosity < eLow || m_oSpTrafo.isNull()){
		return;
	} // if

	const Trafo		&rTrafo		( *m_oImageContext.trafo() );
	OverlayCanvas	&rCanvas	( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayer		( rCanvas.getLayerPosition() ); // layer 2: paint over position found before

	const Point oPoint1(m_oPos1X, m_oPos1Y);
	const Point oPoint2(m_oPos2X, m_oPos2Y);
	const Point oPoint3(m_oPos3X, m_oPos3Y);
	const Point oPoint4(m_oPos4X, m_oPos4Y);

	rLayer.add<OverlayCross>(rTrafo(oPoint1), Color::Yellow()); // paint position in Yellow
	rLayer.add<OverlayCross>(rTrafo(oPoint2), Color::Yellow()); // paint position in Yellow
	rLayer.add<OverlayCross>(rTrafo(oPoint3), Color::Yellow()); // paint position in Yellow
	rLayer.add<OverlayCross>(rTrafo(oPoint4), Color::Yellow()); // paint position in Yellow

} // paint

bool HeightDifference::subscribe(BasePipe& pipe, int group)
{
	if ( pipe.type() == typeid(GeoVecDoublearray) )
		pipeInLineY_ = dynamic_cast< SynchronePipe <GeoVecDoublearray> * >(&pipe);
	else if ( pipe.type() == typeid(GeoDoublearray) ) {
		if (pipe.tag() == "position_x")
			pipeInPosX_  = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray > * >(&pipe);
		else if (pipe.tag() == "angle")
			pipeInPosAngle_ = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray > * >(&pipe);
	} // if

	return BaseFilter::subscribe( pipe, group );
}


void HeightDifference::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e)
{
	poco_assert_dbg(pipeInLineY_ != nullptr); // to be asserted by graph editor
	poco_assert_dbg(pipeInPosX_ != nullptr); // to be asserted by graph editor
	poco_assert_dbg(pipeInPosAngle_ != nullptr); // to be asserted by graph editor

	double rankout(0);

	const GeoVecDoublearray & geolineY = (pipeInLineY_->read(m_oCounter));
	const VecDoublearray &  lline = geolineY.ref();

	const GeoDoublearray	&rGeoPosXIn	(pipeInPosX_->read(m_oCounter));
	const GeoDoublearray	&rGeoPosAngleIn(pipeInPosAngle_->read(m_oCounter));
	const Doublearray		&rPosXIn	( rGeoPosXIn.ref() );
	const Doublearray		&rPosAngleIn	( rGeoPosAngleIn.ref() );
	m_oSpTrafo	= geolineY.context().trafo();

	if ( inputIsInvalid(rGeoPosXIn) || inputIsInvalid(rGeoPosAngleIn)
		|| lline.size() == 0 || rPosXIn.size() == 0 || rPosAngleIn.size() == 0
	) {
		const double			oRank		( interface::NotPresent );
		const GeoDoublearray	oGeoMismatchOut	( rGeoPosXIn.context(), Doublearray(1, 0.0, eRankMin), rGeoPosXIn.analysisResult(), oRank );
		preSignalAction();
		pipeOutHeightDifference_->signal(oGeoMismatchOut);

		return; // RETURN
	}

	m_oImageContext=geolineY.context(); //context speichern

	m_oHwRoi.x = m_oImageContext.HW_ROI_x0;
	m_oHwRoi.y = m_oImageContext.HW_ROI_y0;


	const unsigned int	oNbLines	= lline.size();
	const unsigned int	oNbPosX	= rPosXIn.getData().size();
	const unsigned int	oNbPosAngle	= rPosAngleIn.getData().size();
	bool alwaysUseFirstX = oNbPosX != oNbLines;
	bool alwaysUseFirstAngle = oNbPosAngle != oNbLines;
	if (alwaysUseFirstX)
	{
		wmLog(eDebug, "Filter '%s': Received %u X values for %u laser lines. Only process first element, rest will be discarded.\n", m_oFilterName.c_str(), oNbPosX, oNbLines);
	}
	if (alwaysUseFirstAngle)
	{
		wmLog(eDebug, "Filter '%s': Received %u angle values %u laser lines. Only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rPosAngleIn.size(), oNbLines);
	}

	// const auto oAnalysisOk = rGeoPosXIn.analysisResult() == AnalysisOK ? rGeoPosAngleIn.analysisResult() :  rGeoPosXIn.analysisResult();
	const double	oRank		( (rGeoPosXIn.rank() + rGeoPosAngleIn.rank()) / 2. );
	int oDiff = (rGeoPosXIn.context().trafo())->dx() - m_oSpTrafo->dx();   // m_oSpTrafo ist die Transformation der Linie; fuer den Fall, dass die Eingangsdaten vom selben ROI kommen, ist das hier eine NullOperation

	//Kalibrationsdaten holen
	auto &rCalib(system::CalibDataSingleton::getCalibrationCoords(math::SensorId::eSensorId0));
	const TPoint<double>	oGlobalPos			( m_oSpTrafo->dx(), m_oSpTrafo->dy() );	    //	Offset ROI Koordinaten -> Bildkoordinaten


	Doublearray oHeightDifferenceAbsOutArray(oNbLines);

	for (unsigned int lineN = 0; lineN < oNbLines; ++lineN)  // loop over N lines
	{
		const auto& rLineIn = lline[lineN];
		double & rHeightDifferenceAbsOut = oHeightDifferenceAbsOutArray.getData()[lineN];
		int & rRankOut = oHeightDifferenceAbsOutArray.getRank()[lineN];

		firstValidIndex	= getFirstValidIndex(rLineIn);
		lastValidIndex	= getLastValidIndex(rLineIn);
		rankout=0.0;

		const double xin = (alwaysUseFirstX ? rPosXIn.getData().front()  : rPosXIn.getData()[lineN]  )+ oDiff;
		const double anglein = (alwaysUseFirstAngle ? rPosAngleIn.getData().front() : rPosAngleIn.getData()[lineN]);
		const double oAngle = M_PI * anglein / 180.0;

		bool invalidX = (xin<0 || oRank<1e-7);
		if (m_oVerbosity >= eMedium)
		{
			if(invalidX)
			{
				wmLog(eDebug, "input x=%d invalid (lineN=%d)\n", xin, lineN);
			}
			else
			{
				wmLog(eDebug, "input x=%d \n(lineN=%d)",(int) xin, lineN);
			}

		}
		int startX = static_cast<int>(0.5+xin);


		// Funktion liefert Pixelkoordinaten im SW ROI --ToDo
		double x1 = 0.0;
		double y1 = 0.0;
		double x2 = 0.0;
		double y2 = 0.0;
		CalculateMismatch(lline[lineN], startX , rankout,x1,y1,x2,y2);
		if (invalidX)
		{
			rankout = 0.0;
		}

		const TPoint<double>   oPixPos1(x1, y1);
		const TPoint<double>   oPixPos2(x2, y2);

		//Achtung jeder Punkt im SW  ROI braucht die Korrektur Sw ROI und HW ROI
		const TPoint<double>	oSensorPos1			( oPixPos1 + oGlobalPos + m_oHwRoi );	//	Offset Bildkoordinaten -> Sensorkoordinaten
		const TPoint<double>	oSensorPos2			( oPixPos2 + oGlobalPos + m_oHwRoi );	//	Offset Bildkoordinaten -> Sensorkoordinaten

		//Jetzt 3D Positionen
		const math::Vec3D oSensorCoord1 = rCalib.to3D(static_cast<int>(oSensorPos1.x), static_cast<int>(oSensorPos1.y), m_oTypeOfLaserLine);
		const math::Vec3D oSensorCoord2 = rCalib.to3D(static_cast<int>(oSensorPos2.x), static_cast<int>(oSensorPos2.y), m_oTypeOfLaserLine);

		// Berechne die Z-Komponente des um die Y-Achse gedrehten Vektors:
		math::Vec3D oDiffVectorStartsAtRightSegment = oSensorCoord1 - oSensorCoord2;   // Weltkoordinate links minus Weltkoordinate rechts
		math::Vec3D oDiffVectorStartsAtRightSegmentTransformed = math::Calibration3DCoords::FromCalibratedToRotated(oDiffVectorStartsAtRightSegment, oAngle);

		rHeightDifferenceAbsOut = fabs(oDiffVectorStartsAtRightSegmentTransformed[2]);

		rRankOut = (doubleToIntRank(rankout));

		if(m_oVerbosity >= eMedium)
		{
			wmLog(eDebug, "Mismatch::proceed  -> Mismatchout: %g, Rank: %g (line %d)", rHeightDifferenceAbsOut, rankout, lineN);
		}
	}

	const double			oGeoRankOut			= std::min(rGeoPosXIn.rank(), rGeoPosAngleIn.rank());
	const GeoDoublearray	oGeoDoublearrayOut(m_oImageContext, std::move(oHeightDifferenceAbsOutArray), rGeoPosXIn.analysisResult(), oGeoRankOut);

	// Resultat eintragen:
	preSignalAction();
	pipeOutHeightDifference_->signal(oGeoDoublearrayOut);
}

int HeightDifference::CalculateMismatch(const Doublearray & lline, int xIn, double & rankout, double& x1, double&y1, double&x2, double&y2)
{
	int ret=0;

	ret = CalculateMismatchPoints(lline, xIn ,x1,y1,x2,y2,rankout);

	return ret;
}

// alle x,y roi Koordinaten (Software ROI nicht beruecksichtigt, Hardware Roi nicht beruecksichtigt)
int HeightDifference::CalculateMismatchPoints(const Doublearray & lline, int x0, double & x1, double & y1, double & x2, double & y2, double & rankout)
{

	resetPaintVariables();
	const std::vector<double> & lineY = lline.getData();

	int x1min,x1max,x2min,x2max;
	double m1,m2,t1,t2;
	rankout=0.0;

	x1max=x0-m_oLeftFitintervalDistance;
	if(x1max<0) return -1;
	if(x1max<firstValidIndex) return -1;
	if(x1max>lastValidIndex) return -1;

	x1min=x1max-m_oFitintervalSize;
	if(x1min<0) return -1;
	if(x1min<firstValidIndex) return -1;
	if(x1min>lastValidIndex) return -1;

	m_oPos1X=x1min;
	m_oPos1Y=int( lineY[m_oPos1X] );
	m_oPos2X=x1max;
	m_oPos2Y=int( lineY[m_oPos2X] );
	linefit(lline, x1min,x1max,m1,t1);


	x2min=x0+m_oRightFitintervalDistance;
	if(x2min<0) return -1;
	if(x2min<firstValidIndex) return -1;
	if(x2min>lastValidIndex) return -1;

	x2max=x2min+m_oFitintervalSize;
	if(x2max<0) return -1;
	if(x2max<firstValidIndex) return -1;
	if(x2max>lastValidIndex) return -1;


	m_oPos3X=x2min;
	m_oPos3Y=int( lineY[m_oPos3X] );
	m_oPos4X=x2max;
	m_oPos4Y=int( lineY[m_oPos4X] );
	linefit(lline, x2min,x2max,m2,t2);

	y1=x0*m1+t1;
	y2=x0*m2+t2;

	x1=x0;
	x2=x0;

	rankout=1.0;

	return 0;
}

int  HeightDifference::linefit(const Doublearray & lline, int imin, int imax, double & m, double & tout) const
{
	const std::vector<double> & lineY = lline.getData();

	int i;
	int UNUSED xp, UNUSED yp;
	int x,y,y0,y1;
	double s20,s10,s00,s11,s01;
	double t;

	if(imin < firstValidIndex ) imin=firstValidIndex ;
	if(imin > lastValidIndex ) imin=lastValidIndex ;

	if(imax < firstValidIndex ) imax=firstValidIndex ;
	if(imax > lastValidIndex ) imax=lastValidIndex ;

	int n;
	s20=s10=s00=s11=s01=0.0;

	x=0;
	y=0;
	n=0;
	y0=int( lineY[imin] );
	for(i=imin;i<=imax;i+=m_oFitintervalSubsamplingIncrement)
	{
		x=i-imin;
		y=int( lineY[i] );
		if(y<0) continue;

		y1=y-y0;

		s20+=x*x;
		s10+=x;
		n++;
		s11+=x*y1;
		s01+=y1;
	}//end for i
	s00=double(n);

	double nenn;

	nenn= -s10*s10+s00*s20;
	if(fabs(nenn)>1e-7)
	{
		t = (-s10*s11+s01*s20)/nenn;
		m = -(s10*s01-s11*s00)/nenn;
		tout=t+y0-m*imin;

		if(m_oVerbosity >= eMedium)
		{
			wmLog(eDebug, "m=%g   t=%g \n",m,t);
			//m_oImageContextm_oImageContextStart Plot Result*PPPPPPPPPPPPPPPPPPPPPPPPPPPPPP

			//for(i=imin;i<=imax;++i,++x)
			for(i=0;i<=800;++i,++x)
			{
				x=i;
				y=static_cast<int>(m*x+tout);
				xp=i;
				yp=y;
				//ovpoint(xp,yp,markcolor_);
			}

			//m_oImageContextm_oImageContextEnd Plot Result*PPPPPPPPPPPPPPPPPPPPPPPPPPPPPP
		} //end if(globaldebuglevel()>3)
	}// end if(fabs(nenn)>1e-7)
	return 0;
}

void HeightDifference::resetPaintVariables()
{
    //variables for paint are continously overwritten
    //in case of an early return in CalculateMismatchPoints they could be inconsistent
    m_oPos1X = 0;
    m_oPos1Y = 0;
    m_oPos2X = 0;
    m_oPos2Y = 0;
    m_oPos3X = 0;
    m_oPos3Y = 0;
    m_oPos4X = 0;
    m_oPos4Y = 0;
}


}  // namespace filter
}  // namespace precitec

