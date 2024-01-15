/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIH, Simon Hilsenbeck (HS), Christian Duchow (Duw)
 * 	@date		2015
 * 	@brief		
 */

#ifndef HEIGHT_DIFFERENCE_H_
#define HEIGHT_DIFFERENCE_H_

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

class FILTER_API HeightDifference: public fliplib::TransformFilter {
public:
	HeightDifference();
	virtual ~HeightDifference();

	void setParameter();
	void paint();

protected:
	void arm (const fliplib::ArmStateBase& state);
	void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);
	virtual bool subscribe(fliplib::BasePipe& pipe, int group);

private:

	//input pipes	
	const fliplib::SynchronePipe<  GeoVecDoublearray >* pipeInLineY_;  //laserlinie
	const fliplib::SynchronePipe< GeoDoublearray >* pipeInPosX_; //Vorbestimmte Position 
	const fliplib::SynchronePipe< GeoDoublearray >* pipeInPosAngle_; //Vorbestimmter Winkel 

	//Zeugs zum Zeichnen
	interface::SmpTrafo						m_oSpTrafo;				///< roi translation
	int m_oPos1X;
	int m_oPos2X;
	int m_oPos1Y;
	int m_oPos2Y;
	int m_oPos3X;
	int m_oPos4X;
	int m_oPos3Y;
	int m_oPos4Y;

	//output
	fliplib::SynchronePipe< GeoDoublearray >* pipeOutHeightDifference_; //<- Out
	
	// Parameter
	int m_oFitintervalSize;
	int m_oLeftFitintervalDistance;
	int m_oRightFitintervalDistance;
	int m_oFitintervalSubsamplingIncrement;

	static const std::string m_oFilterName;
	static const std::string m_oPipenameOut;
	static const std::string m_oFitintervalSizeName;
	static const std::string m_oLeftFitintervalDistanceName;
	static const std::string m_oRightFitintervalDistanceName;
	static const std::string m_oFitintervalSubsamplingIncrementName;
	static const std::string m_oParamTypeOfLaserLine;		///< Parameter: Taype of LaserLine (e.g. FrontLaserLine, BehindLaserLine)

	ImageContext			m_oImageContext;

	geo2d::TPoint<double>	m_oHwRoi;

	int CalculateMismatch(const geo2d::Doublearray & lline, int x0 , double & rankout,double &x1,double &y1,double &x2,double &y2);
	int CalculateMismatchPoints(const geo2d::Doublearray & lline, int x0 ,double & x1,double & y1,double & x2,double & y2,double & rankout);
	int linefit(const geo2d::Doublearray & lline, int imin,int imax,double & m,double & tout) const; 
	void resetPaintVariables();
	int firstValidIndex;
	int lastValidIndex;	
	filter::LaserLine m_oTypeOfLaserLine;							///< which laser line should be used?
};

}
}

#endif /*Mismatch_H_*/
