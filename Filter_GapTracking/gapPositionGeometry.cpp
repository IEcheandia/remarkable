/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		KIH, Simon Hilsenbeck (HS)
* 	@date		2012
* 	@brief
*/

#include "gapPositionGeometry.h"

#include <sstream>
#include <cmath>

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




LinePatternPointDescription8T::LinePatternPointDescription8T()
{
	t=0;
	h1=0;
	h2=0;
}





#define TREATSLOT_INT(A)     parameters_.add(#A, "int", A);
#define TREATSLOT_DOUBLE(A) parameters_.add(#A, "double", A);

const std::string GapPositionGeometry::m_oFilterName 		= std::string("GapPositionGeometry");
const std::string GapPositionGeometry::PIPENAME_POSX	= std::string("PositionX");
const std::string GapPositionGeometry::PIPENAME_POSY	= std::string("PositionY");

const std::string GapPositionGeometry::FILTERBESCHREIBUNG = std::string("GapPositionGeometry GapPosition aus PatternMatching V1\n");

GapPositionGeometry::GapPositionGeometry() :
TransformFilter			( GapPositionGeometry::m_oFilterName, Poco::UUID{"72d53d0a-05f4-4c15-b1c7-77097981bd18"} ),
	pipeInLineY_		( nullptr ),
	pipeOutPosX_		( new SynchronePipe< GeoDoublearray >( this, GapPositionGeometry::PIPENAME_POSX ) ),
	pipeOutPosY_		( new SynchronePipe< GeoDoublearray >( this, GapPositionGeometry::PIPENAME_POSY ) ),
	firstValidIndex		(0),
	lastValidIndex		(0)
{
	pipeInLineY_=nullptr;
	lastXpos=-1234;
	lastYpos=-1234;

	PaintNPointsMax=2000;
	Paint_i_Point=0;
	PaintbufferX=0;
	PaintbufferY=0;
	PaintbufferX=new int [PaintNPointsMax];
	PaintbufferY=new int [PaintNPointsMax];
	lpd.reserve(100);
	trackstart=0;


	// Parameter
	TREATSLOT_INT(trackstart);
	TREATSLOT_DOUBLE(constantXOffset);
	TREATSLOT_DOUBLE(constantYOffset);
	TREATSLOT_INT(npoints);
	TREATSLOT_INT(t1);
	TREATSLOT_INT(ha1);
	TREATSLOT_INT(hb1);
	TREATSLOT_INT(t2);
	TREATSLOT_INT(ha2);
	TREATSLOT_INT(hb2);
	TREATSLOT_INT(t3);
	TREATSLOT_INT(ha3);
	TREATSLOT_INT(hb3);
	TREATSLOT_INT(t4);
	TREATSLOT_INT(ha4);
	TREATSLOT_INT(hb4);
	TREATSLOT_INT(t5);
	TREATSLOT_INT(ha5);
	TREATSLOT_INT(hb5);
	TREATSLOT_INT(t6);
	TREATSLOT_INT(ha6);
	TREATSLOT_INT(hb6);

    setInPipeConnectors({{Poco::UUID("21fc8d1f-c7f5-4745-a3df-be3fe8c1f940"), pipeInLineY_, "Line", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("5d9dd68c-3282-4af5-b93d-7fe430dd12e9"), pipeOutPosX_, "PositionX", 0, ""},
    {Poco::UUID("f0319359-417a-46d0-a79f-4cb2267bfc5d"), pipeOutPosY_, "PositionY", 0, ""}});
    setVariantID(Poco::UUID("8B4DAE63-68EC-4750-AB59-6A37E0E8B3FD"));
}

#undef TREATSLOT_INT
#undef TREATSLOT_DOUBLE


GapPositionGeometry::~GapPositionGeometry()
{
	delete pipeOutPosX_;
	delete pipeOutPosY_;

	if(PaintbufferX!=0) delete [] PaintbufferX;
	if(PaintbufferY!=0) delete [] PaintbufferY;


}




#define TREATSLOT_INT(A)     A = parameters_.getParameter(#A).convert<int> ();
#define TREATSLOT_DOUBLE(A)  A = parameters_.getParameter(#A).convert<double> ();



//tt.t=t0; tt.h1=ha0; tt.h2=hb0; lpd.push_back(tt); if(t2>9999 || ha2>9999 || hb2>9999) return;
#define TREATSLOT_LPD(A)    tt.t=t##A; tt.h1=ha##A; tt.h2=hb##A; lpd.push_back(tt); if(t##A>9999 || ha##A>9999 || hb##A>9999) return;




void GapPositionGeometry::setParameter()
{
	TransformFilter::setParameter();

	LinePatternPointDescription8T 	tt;
	lpd.clear();

	// Parameter
	TREATSLOT_INT(trackstart);
	TREATSLOT_DOUBLE(constantXOffset);
	TREATSLOT_DOUBLE(constantYOffset);
	TREATSLOT_INT(npoints);
	TREATSLOT_INT(t1);
	TREATSLOT_INT(ha1);
	TREATSLOT_INT(hb1);
	TREATSLOT_INT(t2);
	TREATSLOT_INT(ha2);
	TREATSLOT_INT(hb2);
	TREATSLOT_INT(t3);
	TREATSLOT_INT(ha3);
	TREATSLOT_INT(hb3);
	TREATSLOT_INT(t4);
	TREATSLOT_INT(ha4);
	TREATSLOT_INT(hb4);
	TREATSLOT_INT(t5);
	TREATSLOT_INT(ha5);
	TREATSLOT_INT(hb5);
	TREATSLOT_INT(t6);
	TREATSLOT_INT(ha6);
	TREATSLOT_INT(hb6);



	if(m_oVerbosity >= eMedium)
	{
		wmLog(precitec::eDebug, "Filter '%s': %s.\n", m_oFilterName.c_str(), FILTERBESCHREIBUNG.c_str());
	}
	tt.t=0; tt.h1=0; tt.h2=0; lpd.push_back(tt); //Dummy Ausgangspunkt 0
	TREATSLOT_LPD(1)
		TREATSLOT_LPD(2)
		TREATSLOT_LPD(3)
		TREATSLOT_LPD(4)
		TREATSLOT_LPD(5)
		TREATSLOT_LPD(6)

		if(npoints>int(lpd.size()))    npoints=lpd.size();

}

#undef TREATSLOT_INT
#undef TREATSLOT_DOUBLE





void GapPositionGeometry::arm (const fliplib::ArmStateBase& state)
{
	int ArmState = state.getStateID();

	if(m_oVerbosity >= eMedium) wmLog(eDebug, "GapPositionGeometry armstate=%d\n",ArmState);

	if(ArmState == 0 )
	{
		//Nahtanfang
		lastXpos=-1234;
		lastYpos=-1234;
	}

} // arm



void GapPositionGeometry::paint() {
	if(m_oVerbosity < eLow || m_oSpTrafo.isNull()){
		return;
	} // if

	const Trafo		&rTrafo			( *m_oSpTrafo );
	OverlayCanvas	&rCanvas		( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerPosition	( rCanvas.getLayerPosition());
	OverlayLayer	&rLayerContour	( rCanvas.getLayerContour());

	rLayerPosition.add( new	OverlayCross(rTrafo(m_oGapPosition), Color::Green())); // paint position in green

	if(m_oVerbosity <= eMedium) {
		return;
	} // if


	for (int i = 0;  i < Paint_i_Point; ++i) {
		const int x = PaintbufferX[i];
		const int y = PaintbufferY[i];
		rLayerContour.add( new OverlayPoint(rTrafo(Point(x,y)), Color::Orange()) );
	} // for
} // paint



bool GapPositionGeometry::subscribe(BasePipe& p_rPipe, int p_oGroup) {
	pipeInLineY_ = dynamic_cast< SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);
	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe


void GapPositionGeometry::proceed(const void* sender, PipeEventArgs& e)
{
	poco_assert_dbg(pipeInLineY_ != nullptr); // to be asserted by graph editor

	// Paint Stuff
	Paint_i_Point=0;



	int i (0);
	double x (0),y (0),rank (0);

	if(m_oVerbosity >= eMedium)
	{
		wmLog(eDebug, "constantXOffset=%g\n",constantXOffset);
		wmLog(eDebug, "constantYOffset=%g\n",constantYOffset);

		for(i=0;i<npoints;++i)  wmLog(eDebug, "%d  t=%d   h1=%d  h2=%d\n",i,lpd[i].t,lpd[i].h1,lpd[i].h2);

	}

	// Empfangenes Frame auslesen

	const GeoVecDoublearray & geolineY = (pipeInLineY_->read(m_oCounter));
	const VecDoublearray &  lline = geolineY.ref();

	m_oSpTrafo	= geolineY.context().trafo();

	if ( inputIsInvalid( geolineY ) ) {
		const GeoPoint			point		(geolineY.context(), m_oGapPosition, AnalysisOK, rank );
		const GeoDoublearray	oGeoPosXOut	( point.context(), Doublearray(1, point.ref().x, eRankMin), point.analysisResult(), interface::NotPresent );
		const GeoDoublearray	oGeoPosYOut	( point.context(), Doublearray(1, point.ref().y, eRankMin), point.analysisResult(), interface::NotPresent );

		preSignalAction();
		pipeOutPosX_->signal(oGeoPosXOut);
		pipeOutPosY_->signal(oGeoPosYOut);

		return; // RETURN
	} // if

	firstValidIndex	= getFirstValidIndex(lline.front());
	lastValidIndex	= getLastValidIndex(lline.front());

	DetermineGapPosition(lline, x,y,rank);

	if(m_oVerbosity >= eMedium)
	{
		wmLog(eDebug, "x=%g  y=%g  rank=%g\n",x,y,rank);
	}


	if(x<0 || y<0) rank = 0.0; //Koordinaten negativ: rank null setzen




	if (rank > 0.0)
	{
		// x nur korrigieren wenn rank OK
		x += constantXOffset;
		y += constantYOffset;
	}

	m_oGapPosition = Point(static_cast<int>(x),static_cast<int>(y));


	// Resultat eintragen:


	const GeoPoint			point		(geolineY.context(), m_oGapPosition, AnalysisOK, rank );
	const int				oRank		( doubleToIntRank(point.rank()) );
	const GeoDoublearray	oGeoPosXOut	( point.context(), Doublearray(1, point.ref().x, oRank), point.analysisResult(), point.rank() );
	const GeoDoublearray	oGeoPosYOut	( point.context(), Doublearray(1, point.ref().y, oRank), point.analysisResult(), point.rank() );

	preSignalAction();
	pipeOutPosX_->signal(oGeoPosXOut);
	pipeOutPosY_->signal(oGeoPosYOut);

	if(m_oVerbosity >= eMedium)
	{
		wmLog(eDebug, "GapPositionGeometry::proceed  -> oGapPosition: %i, %i Rank: %g", m_oGapPosition.x, m_oGapPosition.y, rank);
	}
}


bool GapPositionGeometry::checkposition(const VecDoublearray & lline, int index,int i1)
{
	const std::vector<double> & lineY = lline.front().getData(); // work only with 1st line

	int x1,x2;
	int y1,y2;
	int hmin,hmax;

	x1=index;
	y1=int( lineY[x1] );
	if(y1<0) return false;

	x2=x1+lpd[i1].t;//  t -> x

	if(x2<firstValidIndex) return false;

	if(x2>lastValidIndex)  return false;

	y2=int( lineY[x2] );

	if(y2<0) return false;

	int ht;

	ht=y2 - y1; // y -> h

	hmin = std::min(lpd[i1].h1 , lpd[i1].h2);
	if(ht<hmin) return false;

	hmax = std::max(lpd[i1].h1 , lpd[i1].h2);
	if(ht>hmax) return false;

	return true;
}



void GapPositionGeometry::DetermineGapPosition(const VecDoublearray & lline, double & ergx,double & ergy,double & ergrank)
{
	const std::vector<double> & lineY = lline.front().getData(); // work only with 1st line
	if (checkIndices(lineY, firstValidIndex, lastValidIndex) == false) {
		return;
	} // if

	ergrank=0.0;

	double ergxLeft = 0;
	double ergyLeft = 0;
	double ergxRight = 0;
	double ergyRight = 0;
	bool firstPointFound = false;;
	int index = 0;
	int i1 = 0;
	int h1 = 0;
	if(m_oVerbosity >= eMedium)
	{
		wmLog(eDebug, "firstValidIndex=%d   lastValidIndex=%d \n",firstValidIndex,lastValidIndex);
	}

	firstPointFound=false;
	for(index=firstValidIndex;index<lastValidIndex;++index)
	{
		h1=int( lineY[index] );
		if(h1<0) continue;

		// checke position i1 -> position i2
		for(i1=1;i1<=npoints;++i1)
		{

			if(!checkposition(lline, index,i1)) goto nextindex;

		}

		ergrank=1.0;
		if(!firstPointFound)
		{
			// set left (==first) result
			firstPointFound=true;
			ergxLeft=index;
			ergyLeft=h1;
		}
		// set right (== later last) result

		ergxRight=index;
		ergyRight=h1;

		//ovpoint(index,h1,0xFF00FF);

		// Paint Stuff
		if(Paint_i_Point<PaintNPointsMax)
		{
			PaintbufferX[Paint_i_Point]=(int)ergxRight;
			PaintbufferY[Paint_i_Point]=(int)ergyRight;
			Paint_i_Point++;

		}


nextindex: ;
	}

	if(trackstart==0)
	{
		ergx=ergxRight;
		ergy=ergyRight;
	}
	else
	{
		ergx=ergxLeft;
		ergy=ergyLeft;
	}


	return;
}





	}}

