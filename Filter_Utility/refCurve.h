/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		HS
 * 	@date		2014
 * 	@brief		Provides refernce curve data depending on position.
 */

#ifndef REFCURVE_H_INCLUDED_20140521
#define REFCURVE_H_INCLUDED_20140521

#include "fliplib/Fliplib.h"			// FILTER_API
#include "fliplib/TransformFilter.h"	// base class
#include "fliplib/PipeEventArgs.h"		// event processing
#include "fliplib/SynchronePipe.h"		// in- / output

#include "common/frame.h"				// ImageFrame
#include "common/product1dParameter.h"
#include "image/image.h"				// BImage

namespace precitec {
namespace filter{

/**
 * @brief	This filter provides refernce curve data depending on position.
 */
class FILTER_API RefCurve : public fliplib::TransformFilter  {
public:
	RefCurve();
private:
	typedef fliplib::SynchronePipe<interface::GeoDoublearray>	scalar_pipe_t;
	typedef fliplib::SynchronePipe<interface::ImageFrame>		image_pipe_t;

	/**
	 * @brief Arm the filter. This means here, that the length of the seam is determined and memory is allocated for all the data elements.
	 */
	/*virtual*/ void arm(const fliplib::ArmStateBase& state);

	/**
	 * @brief Set filter parameters.
	 */	
	/*virtual*/ void setParameter();

	/**
	 * @brief In-pipe registration.
	 * @param p_rPipe Reference to pipe that is getting connected to the filter.
	 * @param p_oGroup Group number (0 - proceed is called whenever a pipe has a data element, 1 - proceed is only called when all pipes have a data element).
	 */
	/*virtual*/ bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	
	/**
	 * @brief Processing routine.
	 * @param p_pSender pointer to
	 * @param p_rEvent
	 */
	/*virtual*/ void proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rEventArg);
	
	image_pipe_t*							m_pPipeInImageFrame;	/// in pipe (needed for trigger)
	scalar_pipe_t							m_oPipeOutRefValue;		/// out pipe
	std::string								m_oCurveId;				/// id of refernce curve to be used
	int										m_oPositionOffset;		/// PositionOffset
	double									m_oValueOffset;			/// ValueOffset

	const interface::id_refcurve_map_t*		m_pIdRefCurveMap;		/// stores reference curves
	const interface::Product1dParameter*	m_pProdRefCurve;		/// selected product curve(s) depending on curve id
	const interface::Seam1dParameter*		m_pSeamRefCurve;		/// selected seam curve depending on seam series, seam
	const interface::vec_pos_val_t*			m_pPosValVec;			/// selected pos val vector depending on seam series, seam
	interface::cit_pos_val_t				m_oItCurrent;			/// current pos val element
	interface::cit_pos_val_t				m_oItPrev;				/// previous pos val element
	
	unsigned int							m_oSeamSeries;			/// current seam series - set during arm from external product data
	unsigned int							m_oSeam;				/// current seam series - set during arm from external product data
}; // class RefCurve

} // namespace filter
} // namespace precitec

#endif // REFCURVE_H_INCLUDED_20140521
