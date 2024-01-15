/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIH, Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief		
 */

#ifndef Mismatch_H_
#define Mismatch_H_

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

class FILTER_API Mismatch: public fliplib::TransformFilter {
public:
	Mismatch();
	virtual ~Mismatch();

	static const std::string m_oFilterName;
	static const std::string FILTERBESCHREIBUNG;
	static const std::string PIPENAME_MISMATCH;

	void setParameter();
	void paint();

protected:
	void arm (const fliplib::ArmStateBase& state);
	void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);
	virtual bool subscribe(fliplib::BasePipe& pipe, int group);
	double lastXpos;
	double lastYpos;

private:

	//input pipes	
	const fliplib::SynchronePipe<  GeoVecDoublearray >* pipeInLineY_;  //laserlinie
	const fliplib::SynchronePipe< GeoDoublearray >* pipeInPosX_; //Vorbestimmte Position 
	const fliplib::SynchronePipe< GeoDoublearray >* pipeInPosY_; //Vorbestimmte Position 

	//Zeugs zum Zeichnen
	interface::SmpTrafo						m_oSpTrafo;				///< roi translation
	int m_oPos1X,m_oPos2X,m_oPos1Y,m_oPos2Y ,m_oPos3X,m_oPos4X,m_oPos3Y,m_oPos4Y;

	//output
	fliplib::SynchronePipe< GeoDoublearray >* pipeOutMismatch_; //<- Out
	
			// Parameter
			TREATSLOT_INT(FitintervalSize);
			TREATSLOT_INT(LeftFitintervalDistance);
			TREATSLOT_INT(RightFitintervalDistance);
			TREATSLOT_INT(FitintervalSubsamplingIncrement);
			TREATSLOT_INT(InvertMismatchSign);
			

	ImageContext			m_oImageContext;

	const GeoPoint			*pinGeoPoint;
	const geo2d::Point		*pinPos_;
	geo2d::TPoint<double>	m_oHwRoi;

	int CalculateMismatch(const geo2d::VecDoublearray & lline, int x0 , double & rankout,double &x1,double &y1,double &x2,double &y2);
	int CalculateMismatchPoints(const geo2d::VecDoublearray & lline, int x0 ,double & x1,double & y1,double & x2,double & y2,double & rankout);
	int linefit(const geo2d::VecDoublearray & lline, int imin,int imax,double & m,double & tout);
	
	int firstValidIndex;
	int lastValidIndex;	

	filter::LaserLine m_oTypeOfLaserLine;


};




#undef TREATSLOT_INT
#undef TREATSLOT_DOUBLE




}
}

#endif /*Mismatch_H_*/
