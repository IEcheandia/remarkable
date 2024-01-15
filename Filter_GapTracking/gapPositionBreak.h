/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIH, Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief		
 */

#ifndef GAP_POSITION_BREAK_20120711_INCLUDED
#define GAP_POSITION_BREAK_20120711_INCLUDED

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


class FILTER_API GapPositionBreak: public fliplib::TransformFilter {
public:
	GapPositionBreak();
	virtual ~GapPositionBreak();

	static const std::string m_oFilterName;
	static const std::string FILTERBESCHREIBUNG;
	static const std::string PIPENAME_POS1;
	static const std::string PIPENAME_POS2;

	void setParameter();
	void paint();

protected:
	void arm (const fliplib::ArmStateBase& state);
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	void proceed(const void* sender, fliplib::PipeEventArgs& e);
	double m_lastXpos;
	double m_lastYpos;

private:
	void resizeOutArrays(unsigned int size);
	void updateOutArrays(unsigned int lineN, double x, double y, int rank);
	
	const fliplib::SynchronePipe<  GeoVecDoublearray >* pipeInLineY_;  //neue Laserlinienstruktur  y(x)	

	//output
	fliplib::SynchronePipe< GeoDoublearray >* pipeOutPosX_; //<- Out
	fliplib::SynchronePipe< GeoDoublearray >* pipeOutPosY_; //<- Out

	interface::SmpTrafo						m_oSpTrafo;				///< roi translation
	int trackstart_;
	double constantXOffset_;

	int useTimeSeriesCorrection_;
	double maxjumpX_;
	double maxjumpY_;

	geo2d::Doublearray m_oGapPositionsX;
	geo2d::Doublearray m_oGapPositionsY;
	double m_oGeoOutRank;
	

	int m_lastUpdatedArrayIndex; //only for debug

	void DetermineGapPosition(const geo2d::Doublearray & lineY, double & x,double & y,double & rank) const;
};

}
}

#endif /*GAP_POSITION_BREAK_20120711_INCLUDED*/
