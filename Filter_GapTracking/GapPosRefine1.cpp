/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIH, Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief		Gapposition verbessern durch Parabelfit
 */


#include "GapPosRefine1.h"

#include <cmath>
#include <sstream>
#include "Poco/SharedPtr.h"
#include "fliplib/Parameter.h"
#include "fliplib/SynchronePipe.h"

#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "filter/algoArray.h"
#include "module/moduleLogger.h"

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


const std::string GapPositionRefine::m_oFilterName 	= std::string("GapPositionRefine");
const std::string GapPositionRefine::PIPENAME_POSX	= std::string("PositionX");
const std::string GapPositionRefine::PIPENAME_POSY	= std::string("PositionY");

const std::string GapPositionRefine::FILTERBESCHREIBUNG = std::string("GapPositionRefine Gapposition aus Linienabbruch\n");



#define TREATSLOT_INT(A)     parameters_.add(#A, "int", A);
#define TREATSLOT_DOUBLE(A) parameters_.add(#A, "double", A);

GapPositionRefine::GapPositionRefine() :
TransformFilter( GapPositionRefine::m_oFilterName, Poco::UUID{"adff7ee0-f374-4200-89f6-b939525ad39f"} ),
	pipeInLineY_		(nullptr),
	pipeInPosX_			(nullptr),
	pipeInPosY_			(nullptr),
	pipeOutPosX_		( new SynchronePipe< GeoDoublearray >( this, GapPositionRefine::PIPENAME_POSX ) ),
	pipeOutPosY_		( new SynchronePipe< GeoDoublearray >( this, GapPositionRefine::PIPENAME_POSY ) ),
	firstValidIndex		(0),
	lastValidIndex		(0)
{
	lastXpos=-1234;
	lastYpos=-1234;

	PaintNPointsMax=2000;
	Paint_i_Point=0;
	PaintbufferX=0;
	PaintbufferY=0;
	PaintbufferX=new int [PaintNPointsMax];
	PaintbufferY=new int [PaintNPointsMax];


	// Parameter
	TREATSLOT_INT(fitradius);
	TREATSLOT_INT(PosCorrectionMax);
	TREATSLOT_DOUBLE(constantXOffset);
	TREATSLOT_DOUBLE(constantYOffset);

    setInPipeConnectors({{Poco::UUID("882d853d-a1c9-4b22-be47-d2274f0b0d22"), pipeInLineY_, "Laserline", 1, "line"},
    {Poco::UUID("ccdc2c34-3c9c-44ad-b39e-4f778e426738"), pipeInPosX_, "PositionX", 1, "position_x"},
    {Poco::UUID("4a2d282f-0ef2-4212-b566-8ffe330e8244"), pipeInPosY_, "PositionY", 1, "position_y"}});
    setOutPipeConnectors({{Poco::UUID("6F7D9C6F-CC69-4e9a-A1F4-F36909DBAAC2"), pipeOutPosX_, "PositionX", 0, ""},
    {Poco::UUID("634FB61C-39E4-4afb-92F9-8182A57D4838"), pipeOutPosY_, "PositionY", 0, ""}});
    setVariantID(Poco::UUID("4E41F5A0-6E4F-49F0-8898-1428A69E14C5"));
}

#undef TREATSLOT_INT
#undef TREATSLOT_DOUBLE


GapPositionRefine::~GapPositionRefine()
{
	wmLog(eDebug, "GapPositionRefine DEST Start\n");
	Paint_i_Point=0;
	delete pipeOutPosX_;
	delete pipeOutPosY_;
	if(PaintbufferX!=0) delete [] PaintbufferX;
	if(PaintbufferY!=0) delete [] PaintbufferY;
	wmLog(eDebug, "GapPositionRefine DEST End\n");

}



#define TREATSLOT_INT(A)     A = parameters_.getParameter(#A).convert<int> ();
#define TREATSLOT_DOUBLE(A)  A = parameters_.getParameter(#A).convert<double> ();

void GapPositionRefine::setParameter()
{
	TransformFilter::setParameter();


	// Parameter
	TREATSLOT_INT(fitradius);
	TREATSLOT_INT(PosCorrectionMax);
	TREATSLOT_DOUBLE(constantXOffset);
	TREATSLOT_DOUBLE(constantYOffset);


	if(m_oVerbosity >= eMedium)
	{
		wmLog(precitec::eDebug, "Filter '%s': %s.\n", m_oFilterName.c_str(), FILTERBESCHREIBUNG.c_str());
	}
}

#undef TREATSLOT_INT
#undef TREATSLOT_DOUBLE


void GapPositionRefine::arm (const fliplib::ArmStateBase& state)
{
	int ArmState = state.getStateID();

	if(m_oVerbosity >= eMedium) wmLog(eDebug, "GapPositionRefine armstate=%d\n",ArmState);

	if(ArmState == 0 )
	{
		//Nahtanfang
		lastXpos=-1234;
		lastYpos=-1234;
	}

} // arm



void GapPositionRefine::paint() {
	if(m_oVerbosity < eLow || m_oSpTrafo.isNull()){
		return;
	} // if

	const Trafo		&rTrafo			( *m_oSpTrafo );
	OverlayCanvas	&rCanvas		( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerPosition	( rCanvas.getLayerPosition());
	OverlayLayer	&rLayerContour	( rCanvas.getLayerContour());

	rLayerPosition.add( new	OverlayCross(rTrafo(m_oGapPosition), Color::Yellow())); // paint position

	if(m_oVerbosity <= eMedium) {
		return;
	} // if


	for (int i = 0;  i < Paint_i_Point; ++i) {
		const int x = PaintbufferX[i];
		const int y = PaintbufferY[i];
		rLayerContour.add( new OverlayPoint(rTrafo(Point(x,y)), Color::Orange()) );
	} // for
} // paint



bool GapPositionRefine::subscribe(BasePipe& p_rPipe, int p_oGroup)
{

	if ( p_rPipe.type() == typeid(GeoVecDoublearray) )
		pipeInLineY_ = dynamic_cast< SynchronePipe <GeoVecDoublearray> * >(&p_rPipe);
	else if ( p_rPipe.type() == typeid(GeoDoublearray) ) {
		if (p_rPipe.tag() == "position_x")
			pipeInPosX_  = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray > * >(&p_rPipe);
		else if (p_rPipe.tag() == "position_y")
			pipeInPosY_  = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray > * >(&p_rPipe);
	} // if

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
}


void GapPositionRefine::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e)
{
	poco_assert_dbg(pipeInLineY_ != nullptr); // to be asserted by graph editor
	poco_assert_dbg(pipeInPosX_ != nullptr); // to be asserted by graph editor
	poco_assert_dbg(pipeInPosY_ != nullptr); // to be asserted by graph editor

	double xin (0),yin (0),rankin (0);
	double xout (0),yout (0),rankout (0);


	// Paint Stuff
	Paint_i_Point=0;

	const GeoVecDoublearray & geolineY = (pipeInLineY_->read(m_oCounter));
	const VecDoublearray &  lline = geolineY.ref();

	// Empfangenes Frame auslesen

	const GeoDoublearray	&rGeoPosXIn	(pipeInPosX_->read(m_oCounter));
	const GeoDoublearray	&rGeoPosYIn	(pipeInPosY_->read(m_oCounter));
	const Doublearray		&rPosXIn	( rGeoPosXIn.ref() );
	const Doublearray		&rPosYIn	( rGeoPosYIn.ref() );
	m_oSpTrafo	= rGeoPosXIn.context().trafo();

	if ( inputIsInvalid(geolineY) || inputIsInvalid(rGeoPosXIn) || inputIsInvalid(rGeoPosYIn)) {
		const double			oRank		( interface::NotPresent );
		const GeoDoublearray	oGeoPosXOut	( rGeoPosXIn.context(), Doublearray(1, 0.0, eRankMin), rGeoPosXIn.analysisResult(), oRank );
		const GeoDoublearray	oGeoPosYOut	( rGeoPosYIn.context(), Doublearray(1, 0.0, eRankMin), rGeoPosYIn.analysisResult(), oRank );

		preSignalAction();
		pipeOutPosX_->signal(oGeoPosXOut);
		pipeOutPosY_->signal(oGeoPosYOut);

		return; // RETURN
	}

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

	firstValidIndex	= getFirstValidIndex(lline.front());
	lastValidIndex	= getLastValidIndex(lline.front());

	const auto oAnalysisOk = rGeoPosXIn.analysisResult() == AnalysisOK ? rGeoPosYIn.analysisResult() :  rGeoPosXIn.analysisResult();
	const double	oRankMean		( (rGeoPosXIn.rank() + rGeoPosYIn.rank()) / 2. );
	GeoPoint oGeoPoint	=	GeoPoint(rGeoPosXIn.context(), Point(roundToT<int>(rPosXIn.getData().front()), roundToT<int>(rPosYIn.getData().front())), oAnalysisOk, oRankMean);


	pinGeoPoint = &(oGeoPoint);

	pinPos_ = &pinGeoPoint->ref();
	rankin=pinGeoPoint->rank();

	xin= pinPos_->x;
	yin= pinPos_->y;

	if(xin<0 || yin<0 || rankin<1e-7)
	{
		if(m_oVerbosity >= eMedium)
		{
			wmLog(eDebug, "input Pos invalid\n");
		}
		rankout=0.0;
		xout=yout=0;
		goto exitWithBadRank2;
	}
	else
	{

		if(m_oVerbosity >= eMedium)
		{
			wmLog(eDebug, "input Pos ROI coordinates x=%d y=%d\n",(int) xin,(int) yin);
		}
	}

	int startX;
	startX=(int)(0.5+xin);
	DetermineGapPosition(lline, startX , xout,yout,rankout);

	if(m_oVerbosity >= eMedium)
	{
		wmLog(eDebug, "xout=%g  yout=%g  rankout=%g\n",xout,yout,rankout);
	}


	if(xout<0 || yout<0) rankout = 0.0; //Koordinaten negativ: rank null setzen

	if (rankout > 0.0)
	{
		// x nur korrigieren wenn rank OK (fit war OK)
		xout += constantXOffset;
		yout += constantYOffset;
	}
	else
	{
		// fit war nicht OK
		xout = xin+constantXOffset;
		yout = yin+constantYOffset;
		rankout=rankin*0.5; //nicht toll aber OK

	}

exitWithBadRank2:


	// Resultat eintragen:

	m_oGapPosition = Point(static_cast<int>(xout),static_cast<int>(yout));
	GeoPoint point(geolineY.context(), m_oGapPosition, AnalysisOK, rankout );

	const int				oRank		( doubleToIntRank(point.rank()) );
	const GeoDoublearray	oGeoPosXOut	( point.context(), Doublearray(1, point.ref().x, oRank), point.analysisResult(), point.rank() );
	const GeoDoublearray	oGeoPosYOut	( point.context(), Doublearray(1, point.ref().y, oRank), point.analysisResult(), point.rank() );

	preSignalAction();
	pipeOutPosX_->signal(oGeoPosXOut);
	pipeOutPosY_->signal(oGeoPosYOut);

	if(m_oVerbosity >= eMedium)
	{
		wmLog(eDebug, "GapPositionRefine::proceedGroup  -> Position: %i, %i Rank: %g",  m_oGapPosition.x, m_oGapPosition.y, rankout);
		wmLog(eDebug, "GapPositionRefine::proceedGroup  xko= %g", xout);
	}
} // proceedGroup


/*
void GapPositionRefine::DetermineGapPosition(double & x,double & y,double & rank)
{

int i;
const LaserlineResultT &LaserlineTrackerresult = *pLaserlineTrackerresult;

i=LaserlineTrackerresult.lastValidIndex;
x=i;
y=LaserlineTrackerresult.Y[i];
rank=1.0;
return;

}
*/

void GapPositionRefine::DetermineGapPosition(const VecDoublearray & lline, int startX , double & xout,double & yout,double & rankout)
{
	const std::vector<double> & lineY = lline.front().getData(); // work only with 1st line
	if (checkIndices(lineY, firstValidIndex, lastValidIndex) == false) {
		return;
	} // if

	int i,imin,imax;

	int x,y,y0,y1;
	double s40,s30,s20,s10,s00,s21,s11,s01;
	double xq,a,b,c,xd,yd;

	rankout=0.0;
	xout=startX;
	yout=lineY[startX];

	imin=startX-fitradius;
	imax=startX+fitradius;

	if(imin < firstValidIndex ) imin=firstValidIndex ;
	if(imin > lastValidIndex ) imin=lastValidIndex ;

	if(imax < firstValidIndex ) imax=firstValidIndex ;
	if(imax > lastValidIndex ) imax=lastValidIndex ;

	int n;
	s40=s30=s20=s10=s00=s21=s11=s01=0.0;

	x=-fitradius;
	y=0;
	n=0;
	y0=int( lineY[imin] );
	for(i=imin;i<=imax;++i,++x)
	{
		y=int( lineY[i] );
		if(y<0) continue;

		y1=y-y0;
		xq=x*x;
		s40+=xq*xq;
		s30+=xq*x;
		s20+=xq;
		s10+=x;
		n++;
		s21+=xq*y1;
		s11+=x*y1;
		s01+=y1;
	}//end for i
	s00=double(n);

	double nenn;


	nenn= (2*s20*s30*s10-s10*s10*s40-s20*s20*s20-s00*s30*s30+s00*s40*s20);
	if(fabs(nenn)>1e-7)
	{
		c = (s20*s30*s11+s10*s30*s21-s10*s40*s11-s20*s20*s21-s01*s30*s30+s01*s40*s20)/nenn;
		b =-(s40*s10*s01-s40*s11*s00+s20*s20*s11-s20*s30*s01-s20*s10*s21+s30*s21*s00)/nenn;
		a = (s30*s10*s01-s30*s11*s00+s20*s10*s11-s20*s20*s01-s21*s10*s10+s21*s00*s20)/nenn;

		if(m_oVerbosity >= eMedium)
		{
			//m_oImageContextm_oImageContextStart Plot Result*PPPPPPPPPPPPPPPPPPPPPPPPPPPPPP
			x=-fitradius;
			for(i=imin;i<=imax;++i,++x)
			{
				y=(int)(a*x*x+b*x+c);
				// Paint Stuff
				if(Paint_i_Point<PaintNPointsMax)
				{
					PaintbufferX[Paint_i_Point]=(int)(x+startX);
					PaintbufferY[Paint_i_Point]=(int)(y+y0);
					Paint_i_Point++;
				}

			}
		} //end if(m_oVerbosity >= eMedium)

		if(fabs(a)>1e-7)
		{
			xd=(-b/(2.0*a));
			x=(int)xd;
			if(x>-PosCorrectionMax && x<PosCorrectionMax)
			{
				yd=(a*x*x+b*x+c);
				y=(int)yd;
				xout=xd+startX;
				yout=yd+y0;
				rankout=1.0;
			}

			//m_oImageContextm_oImageContextEnd Plot Result*PPPPPPPPPPPPPPPPPPPPPPPPPPPPPP

		} // end  if(fabs(a)>1e-7)
	}// end if(fabs(nenn)>1e-7)

}




	}}

