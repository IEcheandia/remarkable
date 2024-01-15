/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIH, Simon Hilsenbeck (HS), Christian Duchow (Duw)
 * 	@date		2015
 * 	@brief		
 */

#ifndef COMPUTE_ANGLE_H_
#define COMPUTE_ANGLE_H_

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

class FILTER_API ComputeAngle: public fliplib::TransformFilter {
public:
	ComputeAngle();
	virtual ~ComputeAngle();

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

	//Zeugs zum Zeichnen
	interface::SmpTrafo						m_oSpTrafo;				///< roi translation
	int m_oPos1X;
	int m_oPos2X;
	int m_oPos1Y;
	int m_oPos2Y;

	//output
	fliplib::SynchronePipe< GeoDoublearray >* pipeOutAngle_; //<- Out
	
	// Parameter
	int m_oFitintervalSize;

	static const std::string m_oFilterName;
	static const std::string m_oPipenameOut;
	static const std::string m_oFitintervalSizeName;

	ImageContext			m_oImageContext;

	geo2d::TPoint<double>	m_oHwRoi;
	filter::LaserLine m_oTypeOfLaserLine;

};

bool doLineFit(const std::vector<int>& p_rInRankLine, const std::vector<double>& p_rInDataLine, double p_x0, double p_x1, double& p_ry0, double& p_ry1);

}
}

#endif /*Mismatch_H_*/
