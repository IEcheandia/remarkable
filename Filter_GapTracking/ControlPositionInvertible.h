/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		JS
* 	@date		2013
* 	@brief		
*/


#ifndef ControlPositionInvertible_H_
#define ControlPositionInvertible_H_

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"

#include "geo/geo.h"
#include "common/geoContext.h"
#include "math/calibrationData.h"

namespace precitec {
namespace filter {

	class FILTER_API ControlPositionInvertible : public fliplib::TransformFilter {
public:

	// Diese Inhalte stehen so in der DB
	static const std::string m_oFilterName;

	// Namen der Ausgaenge
	static const std::string m_oPipeXOutName;
	static const std::string m_oPipeYOutName;


	ControlPositionInvertible();

	void setParameter();
	void paint();
	
	void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEvent);
	virtual bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	
private:
	typedef fliplib::SynchronePipe<interface::GeoDoublearray>	scalar_pipe_t;

	//input pipes
	const scalar_pipe_t*	m_pPipeInPosX1;
	const scalar_pipe_t*	m_pPipeInPosY1;
	const scalar_pipe_t*	m_pPipeInPosX2;
	const scalar_pipe_t*	m_pPipeInPosY2;
	const scalar_pipe_t*	m_pPipeInInvRefSide;

	//output pipes
	scalar_pipe_t			m_oPipeOutCoordX;
	scalar_pipe_t			m_oPipeOutCoordY; 

	double					m_oImgPosX1;
	double					m_oImgPosX2;
	double					m_oImgPosY1;
	double					m_oImgPosY2;

	geo2d::TPoint<double>	m_oSensorPos;		//	Position auf dem Sensor (kompletter Chip i.d.R. 1024x1024 pixel
	geo2d::TPoint<double>	m_oHWRoiPos;		//	Position auf dem Hw Roi

	int                     m_oPositionsGewichtung;
	

	interface::SmpTrafo		m_oSpTrafo1;		///< roi translation
	interface::SmpTrafo		m_oSpTrafo2;
	geo2d::TPoint<double>	m_oPositionIn1;
	geo2d::TPoint<double>	m_oPositionIn2;
	geo2d::TPoint<double>	m_oHwRoi;
	geo2d::TPoint<double>	m_oTcpPosition;
	double					m_oInvRefSide;
	geo2d::Doublearray		m_oXOut;
	geo2d::Doublearray		m_oYOut;

	
};


} // namespace filter
} // namespace precitec

#endif /*ControlPositionInvertible_H_*/
