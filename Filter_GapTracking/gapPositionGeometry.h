/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIH, Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief		
 */

#ifndef GAP_POSITION_GEOMETRY_20120711_INCLUDED
#define GAP_POSITION_GEOMETRY_20120711_INCLUDED

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"
#include <vector>
#include "geo/geo.h"
#include "common/frame.h"


namespace precitec {
	using namespace image;
	using namespace interface;
namespace filter {


	


	class LinePatternPointDescription8T
	{
	public:
		LinePatternPointDescription8T();
		int     t;
		int    h1;
		int    h2;
	};




#define TREATSLOT_INT(A) int A;
#define TREATSLOT_DOUBLE(A) double A;



class FILTER_API GapPositionGeometry: public fliplib::TransformFilter {
public:
	GapPositionGeometry();
	virtual ~GapPositionGeometry();

	static const std::string m_oFilterName;
	static const std::string FILTERBESCHREIBUNG;
	static const std::string PIPENAME_POSX;
	static const std::string PIPENAME_POSY;

	void setParameter();
	void paint();

	std::vector <LinePatternPointDescription8T> lpd;

protected:
	void arm (const fliplib::ArmStateBase& state);
	void proceed(const void* sender, fliplib::PipeEventArgs& e);
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	double lastXpos;
	double lastYpos;

	// Paint Stuff 
	interface::SmpTrafo						m_oSpTrafo;				///< roi translation
	int PaintNPointsMax;
		int Paint_i_Point;

	int *PaintbufferX;
	int *PaintbufferY;

	

private:
	

	//input pipes	
	const fliplib::SynchronePipe<  GeoVecDoublearray >* pipeInLineY_;  //laserlinie

	//  Inputs

	void DetermineGapPosition(const geo2d::VecDoublearray & lline, double & x,double & y,double & rank);
	bool checkposition(const geo2d::VecDoublearray & lline, int index,int i1);

	//output
	fliplib::SynchronePipe< GeoDoublearray >* pipeOutPosX_; //<- Out x
	fliplib::SynchronePipe< GeoDoublearray >* pipeOutPosY_; //<- Out y

	
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


	geo2d::Point m_oGapPosition;
	int firstValidIndex;
	int lastValidIndex;


};



#undef TREATSLOT_INT
#undef TREATSLOT_DOUBLE



}
}

#endif /*GAP_POSITION_GEOMETRY_20120711_INCLUDED*/
