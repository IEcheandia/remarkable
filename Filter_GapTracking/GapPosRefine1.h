/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIH, Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief		
 */

#ifndef GapPositionRefine_H_
#define GapPositionRefine_H_

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"

#include "geo/geo.h"
#include "common/frame.h"


namespace precitec {
	using namespace image;
	using namespace interface;
namespace filter {

#define TREATSLOT_INT(A) int A;
#define TREATSLOT_DOUBLE(A) double A;

class FILTER_API GapPositionRefine: public fliplib::TransformFilter {
public:
	GapPositionRefine();
	virtual ~GapPositionRefine();

	static const std::string m_oFilterName;
	static const std::string FILTERBESCHREIBUNG;
	static const std::string PIPENAME_POSX;
	static const std::string PIPENAME_POSY;
	static const std::string PIPENAME_XKO;
	static const std::string PIPENAME_YKO;

	void setParameter();
	void paint();

protected:
	void arm (const fliplib::ArmStateBase& state);
	void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);
	virtual bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	double lastXpos;
	double lastYpos;

private:

	//(teils optionale) input pipes	
	const fliplib::SynchronePipe<  GeoVecDoublearray >* pipeInLineY_;  //laserlinie
	const fliplib::SynchronePipe< GeoDoublearray >* pipeInPosX_; //Vorbestimmte Position 
	const fliplib::SynchronePipe< GeoDoublearray >* pipeInPosY_; //Vorbestimmte Position 


	// Paint Stuff 
	interface::SmpTrafo						m_oSpTrafo;				///< roi translation
	int PaintNPointsMax;
	int Paint_i_Point;

	int *PaintbufferX;
	int *PaintbufferY;

		

	//output
	fliplib::SynchronePipe< GeoDoublearray >* pipeOutPosX_; //<- Out
	fliplib::SynchronePipe< GeoDoublearray >* pipeOutPosY_; //<- Out

	
			// Parameter
			TREATSLOT_INT(fitradius);
			TREATSLOT_INT(PosCorrectionMax);
			TREATSLOT_DOUBLE(constantXOffset);
			TREATSLOT_DOUBLE(constantYOffset);
			

	geo2d::Point m_oGapPosition;
	const GeoPoint * pinGeoPoint;
	const geo2d::Point * pinPos_;

	int firstValidIndex;
	int lastValidIndex;	

	//void DetermineGapPosition(double & x,double & y,double & rank);
	void DetermineGapPosition(const geo2d::VecDoublearray & lline, int startX , double & xout,double & yout,double & rankout);

};




#undef TREATSLOT_INT
#undef TREATSLOT_DOUBLE




}
}

#endif /*GapPositionRefine_H_*/
