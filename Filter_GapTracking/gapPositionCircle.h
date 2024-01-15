/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		Claudius Batzlen (CB)
* 	@date		2019
* 	@brief		
*/


#ifndef GAPPositionCircle_H_
#define GAPPositionCircle_H_

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"

#include "geo/geo.h"
#include <common/frame.h>
#include "math/calibrationData.h"
#include "math/3D/projectiveMathStructures.h"

namespace precitec {
namespace filter {

	class FILTER_API GapPositionCircle : public fliplib::TransformFilter {
public:
	static const std::string m_oFilterName;
	static const std::string m_oPipeXName;
	static const std::string m_oPipeYName;

	GapPositionCircle();

	void setParameter();
	void paint();
	
	void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEvent);
	virtual bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);

	int calcIntersectionCircleTCPx(geo2d::TPoint<double> m_oCenterRadius, double m_oRadius, int oTcpPositionY, bool oFromLeft, int oRoiWidth);

private:
	typedef fliplib::SynchronePipe<interface::GeoDoublearray>	scalar_pipe_t;

	//input pipes
	const fliplib::SynchronePipe< interface::ImageFrame >*	m_pPipeInImage;
	scalar_pipe_t*											m_pPipeInCenterRadiusX;
	scalar_pipe_t*											m_pPipeInCenterRadiusY;
	scalar_pipe_t*											m_pPipeInRadius;

	//output pipes
	scalar_pipe_t						m_oPipeOutCoordX;
	scalar_pipe_t						m_oPipeOutCoordY; 

	interface::SmpTrafo					m_oSpTrafo;				///< roi translation

	geo2d::TPoint<double>				m_oCenterRadiusIn;
	geo2d::TPoint<double>				m_oPositionOut;
	double								m_oRadiusIn;
	bool     							m_oSearchDirection;			///percentual part start left
	geo2d::TPoint<double>				m_oHwRoi;
	geo2d::TPoint<double>				m_oTcpPositionSensorCoordinates;
	geo2d::Doublearray					m_oXOut;
	geo2d::Doublearray					m_oYOut;

};


} // namespace filter
} // namespace precitec

#endif /*GAPPositionCircle_H_*/
