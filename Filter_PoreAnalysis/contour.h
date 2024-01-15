/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		HS
 * 	@date		2013
 * 	@brief		Filter which computes the contour of a pore.
 */

#ifndef CONTOUR_INCLUDED_20130220_H_
#define CONTOUR_INCLUDED_20130220_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include "common/frame.h"
#include "geo/geo.h"
#include "geo/array.h"
#include "calcContour.h"

namespace precitec {
namespace filter {

/**
 * @brief Filter which computes the contour of a pore.
 */
class FILTER_API Contour : public fliplib::TransformFilter
{
public:

	/**
	 * @brief CTor.
	 */
	Contour();

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
	static const std::string m_oPipeOutBlobName;			///< Pipe name: out-pipe.
	static const std::string m_oPipeOutPointXName;			///< Pipe name: out-pipe.
	static const std::string m_oPipeOutPointYName;			///< Pipe name: out-pipe.
	static const std::string m_oPipeOutPointsName;			///< Pipe name: out-pipe.


protected:

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
	void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs&);

private:
	typedef fliplib::SynchronePipe<interface::ImageFrame>		image_pipe_t;
	typedef fliplib::SynchronePipe<interface::GeoBlobarray>		blob_pipe_t;

	const image_pipe_t*			m_pPipeInImageFrame;	///< in pipe
	const blob_pipe_t*			m_pPipeInBlob;			///< in-pipe.
	blob_pipe_t 				m_oPipeOutBlob;			///< out-pipe.
	fliplib::SynchronePipe< interface::GeoDoublearray >				m_oPipeOutPointX;			///< Data out-pipe.
	fliplib::SynchronePipe< interface::GeoDoublearray >				m_oPipeOutPointY;			///< Data out-pipe.
	fliplib::SynchronePipe< interface::GeoVecAnnotatedDPointarray >			m_oPipeOutPoints;			///< Data out-pipe.

	interface::SmpTrafo			m_oSpTrafo;				///< roi translation
	geo2d::Blobarray			m_oBlobsOut;
}; // class Contour

} // namespace filter
} // namespace precitec

#endif /* CONTOUR_INCLUDED_20130220_H_ */
