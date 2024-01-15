/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIH, Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief
 */


#include "Mismatch1.h"

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


const std::string Mismatch::m_oFilterName 		= std::string("Mismatch");
const std::string Mismatch::PIPENAME_MISMATCH	= std::string("Mismatch");

const std::string Mismatch::FILTERBESCHREIBUNG = std::string("Mismatch Gapposition aus Linienabbruch\n");



#define TREATSLOT_INT(A)     parameters_.add(#A, "int", A);
#define TREATSLOT_DOUBLE(A) parameters_.add(#A, "double", A);

Mismatch::Mismatch() :
TransformFilter( Mismatch::m_oFilterName, Poco::UUID{"2c28173f-e7d7-4778-810e-ad51d28b82c6"} ),
	pipeInLineY_		( nullptr ),
	pipeInPosX_			( nullptr ),
	pipeInPosY_			( nullptr ),
	pipeOutMismatch_	(  new SynchronePipe< GeoDoublearray >( this, Mismatch::PIPENAME_MISMATCH )  ),
	firstValidIndex		(0),
	lastValidIndex(0),
	m_oTypeOfLaserLine(LaserLine::FrontLaserLine)

{
	pipeInPosX_=nullptr;
	pipeInPosY_=nullptr;
	lastXpos=-1234;
	lastYpos=-1234;


	// Parameter
	TREATSLOT_INT(FitintervalSize);
	TREATSLOT_INT(LeftFitintervalDistance);
	TREATSLOT_INT(RightFitintervalDistance);
	TREATSLOT_INT(FitintervalSubsamplingIncrement);
	TREATSLOT_INT(InvertMismatchSign);
	parameters_.add("TypeOfLaserLine", Parameter::TYPE_int, int(m_oTypeOfLaserLine));

    setInPipeConnectors({{Poco::UUID("20661027-ac1f-4ece-bc7e-05c90c3a5fae"), pipeInLineY_, "Laserline", 1, "line"},
    {Poco::UUID("de0a11f8-8993-47b2-9909-15f29483b505"), pipeInPosX_, "PositionX", 1, "position_x"},
    {Poco::UUID("1F8B8F15-17E6-4120-AC77-61C282DBD3FC"), pipeInPosY_, "PositionY", 1, "position_y"}});

    setOutPipeConnectors({{Poco::UUID("b204b537-94ca-477e-a0dd-1a77595ae5c5"), pipeOutMismatch_, "Mismatch", 0, ""}});
    setVariantID(Poco::UUID("abe673c9-eb8d-47ed-9de4-b6a17406a966"));
}

#undef TREATSLOT_INT
#undef TREATSLOT_DOUBLE


Mismatch::~Mismatch()
{
	delete pipeOutMismatch_;
}



#define TREATSLOT_INT(A)     A = parameters_.getParameter(#A).convert<int> ();
#define TREATSLOT_DOUBLE(A)  A = parameters_.getParameter(#A).convert<double> ();

void Mismatch::setParameter()
{
	TransformFilter::setParameter();


	// Parameter
	TREATSLOT_INT(FitintervalSize);
	TREATSLOT_INT(LeftFitintervalDistance);
	TREATSLOT_INT(RightFitintervalDistance);
	TREATSLOT_INT(FitintervalSubsamplingIncrement);
	TREATSLOT_INT(InvertMismatchSign);
	m_oTypeOfLaserLine = castToLaserLine(parameters_.getParameter("TypeOfLaserLine").convert<int>());




	if(m_oVerbosity >= eMedium)
	{
		wmLog(precitec::eDebug, "Filter '%s': %s.\n", m_oFilterName.c_str(), FILTERBESCHREIBUNG.c_str());
	}



}

#undef TREATSLOT_INT
#undef TREATSLOT_DOUBLE


void Mismatch::arm (const fliplib::ArmStateBase& state)
{
	int ArmState = state.getStateID();

	if(m_oVerbosity >= eMedium) wmLog(eDebug, "Mismatch armstate=%d\n",ArmState);

	if(ArmState == 0 )
	{
		//Nahtanfang
		lastXpos=-1234;
		lastYpos=-1234;
	}

} // arm



void Mismatch::paint() {
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

	rLayer.add( new	OverlayCross(rTrafo(oPoint1), Color::Yellow())); // paint position in Yellow
	rLayer.add( new	OverlayCross(rTrafo(oPoint2), Color::Yellow())); // paint position in Yellow
	rLayer.add( new	OverlayCross(rTrafo(oPoint3), Color::Yellow())); // paint position in Yellow
	rLayer.add( new	OverlayCross(rTrafo(oPoint4), Color::Yellow())); // paint position in Yellow

} // paint



bool Mismatch::subscribe(BasePipe& pipe, int group)
{
	if ( pipe.type() == typeid(GeoVecDoublearray) )
		pipeInLineY_ = dynamic_cast< SynchronePipe <GeoVecDoublearray> * >(&pipe);
	else if ( pipe.type() == typeid(GeoDoublearray) ) {
		if (pipe.tag() == "position_x")
			pipeInPosX_  = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray > * >(&pipe);
		else if (pipe.tag() == "position_y")
			pipeInPosY_  = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray > * >(&pipe);
	} // if

	return BaseFilter::subscribe( pipe, group );
}


void Mismatch::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e)
{
	poco_assert_dbg(pipeInLineY_ != nullptr); // to be asserted by graph editor
	poco_assert_dbg(pipeInPosX_ != nullptr); // to be asserted by graph editor
	poco_assert_dbg(pipeInPosY_ != nullptr); // to be asserted by graph editor

	double xin(0),yin(0),rankin(0);
	double Mismatchout,rankout(0);

	const GeoVecDoublearray & geolineY = (pipeInLineY_->read(m_oCounter));
	const VecDoublearray &  lline = geolineY.ref();

	const GeoDoublearray	&rGeoPosXIn	(pipeInPosX_->read(m_oCounter));
	const GeoDoublearray	&rGeoPosYIn	(pipeInPosY_->read(m_oCounter));
	const Doublearray		&rPosXIn	( rGeoPosXIn.ref() );
	const Doublearray		&rPosYIn	( rGeoPosYIn.ref() );
	m_oSpTrafo	= rGeoPosXIn.context().trafo();

	if ( inputIsInvalid(rGeoPosXIn) || inputIsInvalid(rGeoPosYIn)) {
		const double			oRank		( interface::NotPresent );
		const GeoDoublearray	oGeoMismatchOut	( rGeoPosXIn.context(), Doublearray(1, 0.0, eRankMin), rGeoPosXIn.analysisResult(), oRank );
		preSignalAction(); pipeOutMismatch_->signal(oGeoMismatchOut);

		return; // RETURN
	}

	m_oImageContext=rGeoPosXIn.context(); //context speichern

	m_oHwRoi.x = m_oImageContext.HW_ROI_x0;
	m_oHwRoi.y = m_oImageContext.HW_ROI_y0;


	if (rPosXIn.size() != 1) { // result is always one point
		wmLog(eDebug, "Filter '%s': Received %u X values. X values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rPosXIn.size());
	}
	if (rPosYIn.size() != 1) { // result is always one point
		wmLog(eDebug, "Filter '%s': Received %u X values. Y values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rPosYIn.size());
	}
	if (rGeoPosXIn.context() != rGeoPosYIn.context()) { // contexts expected to be equal
		std::ostringstream oMsg;
		oMsg << m_oFilterName << ": Different contexts for x and y value: '" << rGeoPosXIn.context() << "', '" << rGeoPosYIn.context() << "'\n";
		wmLog(eWarning, oMsg.str());
	}
	if (rGeoPosXIn.context() != m_oImageContext) { // contexts expected to be equal
		std::ostringstream oMsg;
		oMsg << m_oFilterName << ": Different contexts for x value and laser line: '" << rGeoPosXIn.context() << "', '" << m_oImageContext << "'\n";
		wmLog(eWarning, oMsg.str());
	}

	firstValidIndex	= getFirstValidIndex(lline.front());
	lastValidIndex	= getLastValidIndex(lline.front());

	const auto oAnalysisOk = rGeoPosXIn.analysisResult() == AnalysisOK ? rGeoPosYIn.analysisResult() :  rGeoPosXIn.analysisResult();
	const double	oRank		( (rGeoPosXIn.rank() + rGeoPosYIn.rank()) / 2. );
	GeoPoint oGeoPoint	= GeoPoint(rGeoPosXIn.context(), Point(roundToT<int>(rPosXIn.getData().front()), roundToT<int>(rPosYIn.getData().front())), oAnalysisOk,  oRank );

	pinGeoPoint = &(oGeoPoint);

	pinPos_ = &pinGeoPoint->ref();
	rankin=pinGeoPoint->rank();

	xin= pinPos_->x;
	yin= pinPos_->y;

	if(xin<0 || yin<0 || rankin<1e-7)
	{
		if(m_oVerbosity >= eMedium)
		{
			wmLog(eDebug, "input Pos (%e, %e) invalid\n", xin, yin);
		}
		rankout=0.0;
		Mismatchout=0;
	}
	else
	{

		if(m_oVerbosity >= eMedium)
		{
			wmLog(eDebug, "input Pos ROI coordinates x=%d y=%d\n",(int) xin,(int) yin);
		}
	}

	int startX;
	startX=static_cast<int>(0.5+xin);

	//Kalibrationsdaten holen
	auto &rCalib(system::CalibDataSingleton::getCalibrationCoords(math::SensorId::eSensorId0));

	// Funktion liefert Pixelkoordinaten im SW ROI --ToDo
	double x1 = 0.0;
	double y1 = 0.0;
	double x2 = 0.0;
	double y2 = 0.0;
	CalculateMismatch(lline, startX , rankout,x1,y1,x2,y2);

	const TPoint<double>   oPixPos1(x1, y1);
	const TPoint<double>   oPixPos2(x2, y2);


	//Achtung jeder Punkt im SW  ROI braucht die Korrektur Sw ROI und HW ROI
	const TPoint<double>	oGlobalPos			( m_oSpTrafo->dx(), m_oSpTrafo->dy() );	    //	Offset ROI Koordinaten -> Bildkoordinaten
	const TPoint<double>	oSensorPos1			( oPixPos1 + oGlobalPos + m_oHwRoi );	//	Offset Bildkoordinaten -> Sensorkoordinaten
	const TPoint<double>	oSensorPos2			( oPixPos2 + oGlobalPos + m_oHwRoi );	//	Offset Bildkoordinaten -> Sensorkoordinaten

	//Jetzt 3D Positionen
	const math::Vec3D oSensorCoord1 = rCalib.to3D(static_cast<int>(oSensorPos1.x), static_cast<int>(oSensorPos1.y), m_oTypeOfLaserLine);
	const math::Vec3D oSensorCoord2 = rCalib.to3D(static_cast<int>(oSensorPos2.x), static_cast<int>(oSensorPos2.y), m_oTypeOfLaserLine);

	//Differenzen mit Vorzeichen
	Mismatchout = oSensorCoord1[2] - oSensorCoord2[2];



	if(m_oVerbosity >= eMedium)
	{
		wmLog(eDebug, "Mismatchout=%g  rankout=%g\n",Mismatchout,rankout);
	}

	const int				oRankOut			(doubleToIntRank(rankout));
	const double			oGeoRankOut			= std::min(rGeoPosXIn.rank(), rGeoPosYIn.rank());
	const GeoDoublearray	oGeoDoublearrayOut	(m_oImageContext, Doublearray(1, Mismatchout, oRankOut), rGeoPosXIn.analysisResult(), oGeoRankOut );

	if(m_oVerbosity >= eMedium)
	{
		wmLog(eDebug, "Mismatch::proceed  -> Mismatchout: %g, Rank: %g", Mismatchout,  rankout);
	}

	// Resultat eintragen:
	preSignalAction(); pipeOutMismatch_->signal(oGeoDoublearrayOut);
}




// xin in ROI Koordinaten


int Mismatch::CalculateMismatch(const VecDoublearray & lline, int xIn , double & rankout, double& x1,double&y1,double&x2,double&y2)
{
	//double x1,y1,x2,y2;
	int ret=0;

	ret = CalculateMismatchPoints(lline, xIn ,x1,y1,x2,y2,rankout);
	//Mismatchout=y1-y2; //Pixel



	//double x1out,y1out,z1out,x2out,y2out,z2out;
	////Berechnung Roi Koordinaten -> Image Koordinaten -> Sensor Koordinaten innerhalb von TriagulationCalculation
	//ret = TriagulationCalculation(x1,y1,x1out,y1out,z1out);
	//ret = TriagulationCalculation(x2,y2,x2out,y2out,z2out);

	//Mismatchout=z1out-z2out; //mm Camera Coordinate System

	//if(InvertMismatchSign!=0) Mismatchout=-Mismatchout;

	return ret;
}



// alle x,y roi Koordinaten (Software ROI nicht beruecksichtigt, Hardware Roi nicht beruecksichtigt)
int Mismatch::CalculateMismatchPoints(const VecDoublearray & lline, int x0 ,double & x1,double & y1,double & x2,double & y2,double & rankout)
{
	const std::vector<double> & lineY = lline.front().getData(); // work only with 1st line

	int x1min,x1max,x2min,x2max;
	double m1,m2,t1,t2;
	rankout=0.0;

	x1max=x0-LeftFitintervalDistance;
	if(x1max<0) return -1;
	if(x1max<firstValidIndex) return -1;
	if(x1max>lastValidIndex) return -1;

	x1min=x1max-FitintervalSize;
	if(x1min<0) return -1;
	if(x1min<firstValidIndex) return -1;
	if(x1min>lastValidIndex) return -1;

	m_oPos1X=x1min;
	m_oPos1Y=int( lineY[m_oPos1X] );
	m_oPos2X=x1max;
	m_oPos2Y=int( lineY[m_oPos2X] );
	linefit(lline, x1min,x1max,m1,t1);


	x2min=x0+RightFitintervalDistance;
	if(x2min<0) return -1;
	if(x2min<firstValidIndex) return -1;
	if(x2min>lastValidIndex) return -1;

	x2max=x2min+FitintervalSize;
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
	//ovcross(x0,y1,20,0xff00ff);
	//ovcross(x0,y2,20,0x00ffff);

	rankout=1.0;

	return 0;
}





int  Mismatch::linefit(const VecDoublearray & lline,  int imin,int imax,double & m,double & tout)
{
	const std::vector<double> & lineY = lline.front().getData(); // work only with 1st line

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
	for(i=imin;i<=imax;i+=FitintervalSubsamplingIncrement)
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






	}}

