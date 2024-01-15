/**
*  @copyright:		Precitec Vision GmbH & Co. KG
*  @author			OS
*  @date			01/2015
*  @file			
*  @brief			Performs a circle hough
*/

#ifndef CIRCLEHOUGH_INCLUDED_H
#define CIRCLEHOUGH_INCLUDED_H

// local includes
#include "fliplib/Fliplib.h"			// export macro
#include "fliplib/TransformFilter.h"	// base class
#include "fliplib/SynchronePipe.h"		// in- / output

#include "system/types.h"				// byte
#include "common/frame.h"				// ImageFrame
#include "image/image.h"				///< BImage
#include "circleFitDefinitions.h"

// std lib
#include <string>


namespace precitec {
	namespace filter {



/**
* @brief	Hough filter. Performs circle detection
*/


class FILTER_API CircleHough  : public fliplib::TransformFilter {

public:

	static const std::string m_oFilterName;		///< Filter name.

	/**
	 * @brief CircleHough Constructor.
	 */
	CircleHough();

	/**
	 * @brief						
	 * @param p_rImageIn			Input image to be read.
	 * @param p_rImageOut			Output image to be calculated. 
	*/
	void doCircleHough(const image::BImage& p_rImageIn);

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
	/*virtual*/ void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

	/**
	* @brief Paints ovlerlay.
	*/
	/*virtual*/ void paint();

	typedef fliplib::SynchronePipe<interface::ImageFrame> image_pipe_t;
	typedef fliplib::SynchronePipe< interface::GeoDoublearray > scalar_pipe_t;

	const image_pipe_t*			m_pPipeInImageFrame;	///< in pipe
	const scalar_pipe_t*			m_pPipeInRadiusStart;
    const scalar_pipe_t*			m_pPipeInRadiusEnd;
	scalar_pipe_t				m_oPipeOutX;		///< out pipe
	scalar_pipe_t				m_oPipeOutY;		///< out pipe
	scalar_pipe_t				m_oPipeOutR;		///< out pipe
	scalar_pipe_t				m_oPipeOutScore;		///< out pipe

	double						m_oRadiusStep;				///step width from minimum to maximum radius
	int							m_oNumberMax;
	interface::SmpTrafo			m_oSpTrafo;				///< roi translation
	image::BImage				m_oResImageOut;			///< result image
	int							m_oIntensityThreshold;				///< threshold for image binarization
	CircleHoughParameters::ScoreType	m_oScoreType;
    double						m_oScoreThreshold;
    SearchType m_oSearchOutsideROI; //fit partial circles (search circles with center outside of roi, slower)

    std::vector<hough_circle_t> m_oCircles;
    int m_numValidCircles;

    //variables for paint routine 
    bool m_oPaintInputImage;
    double m_oRadiusStart;
    double m_oRadiusEnd;
}; 




} // namespace filter
} // namespace precitec


#endif /*CIRCLEHOUGH_INCLUDED_H*/



