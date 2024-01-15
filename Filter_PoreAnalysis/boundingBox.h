/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		HS
 * 	@date		2013
 * 	@brief		Filter that computes the x and y dimension (bounding box) for each pore in a pore list.
 */

#ifndef BOUNDINGBOX_INCLUDED_20130218_H_
#define BOUNDINGBOX_INCLUDED_20130218_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include "geo/geo.h"
#include "geo/array.h"
#include "math/3D/projectiveMathStructures.h"

namespace precitec {
namespace filter {

/**
 * @brief Filter that computes the x and y dimension (bounding box) for each pore in a pore list.
 */
class FILTER_API BoundingBox : public fliplib::TransformFilter
{
public:

	/**
	 * @brief CTor.
	 */
	BoundingBox();

private:

	/**
	 * @brief Set filter parameters.
	 */
	void setParameter();

	/**
	 * @brief Paint overlay output.
	 */
	void paint();


	// Declare constants

	static const std::string m_oFilterName;					///< Filter name.
	static const std::string m_oPipeOutDilationXName;		///< Pipe name: out-pipe.
	static const std::string m_oPipeOutDilationYName;		///< Pipe name: out-pipe.


	/**
	 * @brief In-pipe registration.
	 * @param p_rPipe Reference to pipe that is getting connected to the filter.
	 * @param p_oGroup Group number (0 - proceed is called whenever a pipe has a data element, 1 - proceed is only called when all pipes have a data element).
	 */
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);

	/**
	 * @brief Processing routine.
	 * @param p_pSender pointer to
	 * @param p_rEvent
	 */
	void proceed( const void* p_pSender, fliplib::PipeEventArgs& );


	typedef fliplib::SynchronePipe<interface::GeoBlobarray>		blob_pipe_t;
	typedef fliplib::SynchronePipe<interface::GeoDoublearray>	scalar_pipe_t;

	const blob_pipe_t*			m_pPipeInBlob;			///< in-pipe.
	scalar_pipe_t 				m_oPipeOutDilationX;	///< out-pipe.
	scalar_pipe_t 				m_oPipeOutDilationY;    ///< out-pipe.

	interface::SmpTrafo			m_oSpTrafo;				///< roi translation

	geo2d::Doublearray			m_oArrayDilationX;		///< x dilation of pore
	geo2d::Doublearray			m_oArrayDilationY;		///< y dilation of pore

}; // class BoundingBox

} // namespace filter
} // namespace precitec

#endif /* BOUNDINGBOX_INCLUDED_20130218_H_ */
