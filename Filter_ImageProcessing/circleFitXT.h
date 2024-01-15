/**
*  @copyright:		Precitec Vision GmbH & Co. KG
*  @author			LB
*  @date			11/2020
*  @file
*  @brief			Performs a circle fit
*/

#ifndef CIRCLEFITWITHVALIDINPUT_H
#define CIRCLEFITWITHVALIDINPUT_H

// local includes
#include "fliplib/Fliplib.h"			
#include "fliplib/TransformFilter.h"	
#include "fliplib/SynchronePipe.h"		

#include "system/types.h"				
#include "common/frame.h"				
#include "image/image.h"				
#include "circleFitDefinitions.h"

// std lib
#include <string>


namespace precitec {
	namespace filter {


class FILTER_API CircleFitXT  : public fliplib::TransformFilter {

public:

	/**
	 * @brief CircleFitXT Constructor.
	 */
	CircleFitXT();

	/**
	 * @brief Set filter parameters.
	 */
	void setParameter();

private:

    enum class Algorithm
    {
      LeastSquares, Hough
    };

	/**
	 * @brief In-pipe registration.
	 * @param p_rPipe Reference to pipe that is getting connected to the filter.
	 * @param p_oGroup Group number (0 - proceed is called whenever a pipe has a data element, 1 - proceed is only called when all pipes have a data element).
	 */
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);

	void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

	/**
	* @brief Paints ovlerlay.
	*/
	void paint();

    
	typedef fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray> pointlist_pipe_t;
    typedef fliplib::SynchronePipe<interface::GeoDoublearray> scalar_pipe_t;

    ///< Data in-pipes
	const pointlist_pipe_t* m_pPipeInPointList;
    const scalar_pipe_t * m_pPipeInRadiusMin;
    const scalar_pipe_t * m_pPipeInRadiusMax;
    const scalar_pipe_t * m_pPipeInValidXStart;
    const scalar_pipe_t * m_pPipeInValidYStart;
    const scalar_pipe_t * m_pPipeInValidXEnd;
    const scalar_pipe_t * m_pPipeInValidYEnd;

    ///< Data out-pipes
	scalar_pipe_t m_oPipeOutCircleX;
	scalar_pipe_t m_oPipeOutCircleY;
	scalar_pipe_t m_oPipeOutCircleR;
	scalar_pipe_t m_oPipeOutScore;

	Algorithm m_oAlgorithm;
    double m_oHoughRadiusStep;
    CircleHoughParameters::ScoreType m_oHoughScoreType;
    int m_oHoughScoreThreshold;
    int m_oHoughCandidates;

	interface::SmpTrafo			m_oSpTrafo;
    double m_oPaintSamplingX = 1.0;
    double m_oPaintSamplingY = 1.0;
    bool m_paintAdvanced;
    double m_paintX;
    double m_paintY;
    double m_paintR;
    geo2d::Point m_paintValidCenterStart;
    geo2d::Point m_paintValidCenterEnd;
    std::vector<geo2d::DPoint> m_paintInPoints;

}; 



} // namespace filter
} // namespace precitec


#endif /*CIRCLEFIT_INCLUDED_H*/



