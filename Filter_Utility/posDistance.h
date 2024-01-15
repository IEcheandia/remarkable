/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2014
 * 	@brief		This filter computes the distance of two pixel coordinates.
 */

// WM includes
#include <fliplib/Fliplib.h>
#include <fliplib/TransformFilter.h>
#include <fliplib/SynchronePipe.h>
#include <geo/geo.h>
#include <math/3D/projectiveMathStructures.h>


#ifndef POSDISTANCE_H_
#define POSDISTANCE_H_

namespace precitec {
namespace filter   {

/**
 * @brief	This filter computes the 3d distance of two 2d points.
 */
class FILTER_API PosDistance : public fliplib::TransformFilter
{
public:

	/**
	 * CTor.
	 */
	PosDistance();
	/**
	 * @brief DTor.
	 */
	virtual ~PosDistance();

	// Declare constants
	static const std::string m_oFilterName;			///< Filter name.
	static const std::string m_oPipeOutXName;		///< Out-pipe, distance between points, x coord.
	static const std::string m_oPipeOutZName;		///< Out-pipe, distance between points, z coord.

	/**
	 * @brief Set filter parameters.
	 */
	void setParameter();

	void paint();

protected:

	/**
	 * @brief In-pipe registration.
	 * @param p_rPipe Reference to pipe that is getting connected to the filter.
	 * @param p_oGroup Group number (0 - proceed is called whenever a pipe has a data element, 1 - proceed is only called when all pipes have a data element).
	 */
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);

	/**
	 * @brief Processing routine.
	 * @param p_pSender
	 * @param p_rEvent
	 */
	void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEvent);

protected:
	typedef fliplib::SynchronePipe<interface::GeoDoublearray> scalar_pipe_t;

	const scalar_pipe_t*	m_pPipeInPosAX;			///< X-coordinate of first point - in-pipe.
	const scalar_pipe_t*	m_pPipeInPosAY;			///< Y-coordinate of first point - in-pipe.
	const scalar_pipe_t*	m_pPipeInPosBX;			///< X-coordinate of second point - in-pipe.
	const scalar_pipe_t*	m_pPipeInPosBY;			///< Y-coordinate of second point - in-pipe.

	scalar_pipe_t			m_oPipeOutDistX;		///< Distance between points, x component.
	scalar_pipe_t			m_oPipeOutDistZ;		///< Distance between points, z component.

	geo2d::Point 			m_oPointA;				///< First point.
	interface::SmpTrafo		m_pTrafoA;				///< ROI trafo, first point.
	geo2d::Point 			m_oPointB;				///< Second point.
	interface::SmpTrafo		m_pTrafoB;				///< ROI trafo, second point.

	geo2d::Point			m_oHwRoi;				///< Coordinate of hw roi.
	filter::LaserLine m_oTypeOfLaserLine;


}; // class PosDisplay

} // namespace filter
} // namespace precitec

#endif /* DISTANCE_H_ */
