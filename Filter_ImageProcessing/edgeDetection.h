/**
*  @copyright:		Precitec Vision GmbH & Co. KG
*  @author			JS
*  @date			10/2014
*  @file			
*  @brief			Performs edge detection operations
*/

#ifndef EDGEDETECTION_INCLUDED_H
#define EDGEDETECTION_INCLUDED_H

// local includes
#include "fliplib/Fliplib.h"			// export macro
#include "fliplib/TransformFilter.h"	// base class
#include "fliplib/SynchronePipe.h"		// in- / output

#include "system/types.h"				// byte
#include "common/frame.h"				// ImageFrame
#include "filter/parameterEnums.h"

// std lib
#include <string>


namespace precitec {
	namespace filter {





/**
* @brief	Morphology filter. Morphologys an image depending on threshold.
* @details	Morphologys an 8-bit grey image depending on threshold to zero or 255. Not inplace.
*/
class FILTER_API EdgeDetection  : public fliplib::TransformFilter {

public:

	static const std::string m_oFilterName;		///< Filter name.
	static const std::string m_oPipeOut1Name;	///< Out-pipe name.

	/**
	 * @brief CTor.
	 */
	EdgeDetection();

	/**
	 * @brief						Morphologys an image depending on threshold.
	 * @param p_rImageIn			Input image to be read.
	 * @param p_rImageOut			Output binary image to be calculated. Contains only 0 and 255.
	 * @param p_oNbIterations		8 bit threshold for image segmentation
	*/
	void calcEdges(
			const image::BImage&					p_rImageIn,
			image::BImage&							p_rImageOut,
			int									    p_oMode);

	/**
	 * @brief Set filter parameters.
	 */
	/*virtual*/ void setParameter();

private:

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

	/**
	* @brief Paints ovlerlay.
	*/
	/*virtual*/ void paint();

	typedef fliplib::SynchronePipe<interface::ImageFrame>		image_pipe_t;

	const image_pipe_t*			m_pPipeInImageFrame;	///< in pipe
	image_pipe_t				m_oPipeOutImgFrame;		///< out pipe

	EdgeOpType					m_oEdgeOp;				///< type of edge detection
	int							m_oMode;				///< number of iterations

	interface::SmpTrafo			m_oSpTrafo;				///< roi translation
	std::array<image::BImage, g_oNbParMax>				m_oResImageOut;			///< result image
}; // class Morphology



	} // namespace filter
} // namespace precitec


#endif /*EDGEDETECTION_INCLUDED_H*/



