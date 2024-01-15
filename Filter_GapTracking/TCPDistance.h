/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		KIH, Simon Hilsenbeck (HS)
* 	@date		2012
* 	@brief		
*/


#ifndef TCPDistance_H_
#define TCPDistance_H_

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"

#include "geo/geo.h"
#include "math/calibrationData.h"
#include "math/3D/projectiveMathStructures.h"

namespace precitec {
namespace filter {

class FILTER_API TCPDistance: public fliplib::TransformFilter {
public:
	static const std::string m_oFilterName;
	static const std::string m_oPipeXName;
	static const std::string m_oPipeYName;

	TCPDistance();

	void setParameter();
	void paint();
	
	void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEvent);
	virtual bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	
private:
	typedef fliplib::SynchronePipe<interface::GeoDoublearray>	scalar_pipe_t;


	//input pipes
	scalar_pipe_t*						m_pPipeInPosX;
	scalar_pipe_t*						m_pPipeInPosY;

	//output pipes
	scalar_pipe_t						m_oPipeOutCoordX;
	scalar_pipe_t						m_oPipeOutCoordY; 

	interface::SmpTrafo					m_oSpTrafo;				///< roi translation

	geo2d::TPoint<double>				m_oPositionIn;
	geo2d::TPoint<double>				m_oHwRoi;

    geo2d::TPoint<double>				m_oTcpPosition;
	geo2d::Doublearray					m_oXOut;
	geo2d::Doublearray					m_oYOut;

	geo2d::Size m_oSensorSize;
    MeasurementType m_oTypeOfMeasurement;
    bool m_oComputeDistanceFromScannerCenter;
};


} // namespace filter
} // namespace precitec

#endif /*TCPDistance_H_*/
