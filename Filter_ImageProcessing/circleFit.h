/**
*  @copyright:		Precitec Vision GmbH & Co. KG
*  @author			OS
*  @date			01/2015
*  @file			
*  @brief			Performs a circle fit
*/

#ifndef CIRCLEFIT_INCLUDED_H
#define CIRCLEFIT_INCLUDED_H

// local includes
#include "fliplib/Fliplib.h"			
#include "fliplib/TransformFilter.h"	
#include "fliplib/SynchronePipe.h"		

#include "system/types.h"				
#include "common/frame.h"				
#include "filter/parameterEnums.h"
#include "image/image.h"				
#include "circleFitImpl.h"

// std lib
#include <string>


namespace precitec {
	namespace filter {

/**
 * @brief The CircleFit class
 * Diese Klasse setzt die Berechnung des optimalen Kreises analog zu
 * http://www.dtcenter.org/met/users/docs/write_ups/circle_fit.pdf
 * um.
*/
class FILTER_API CircleFit  : public fliplib::TransformFilter {

public:

	static const std::string m_oFilterName;		///< Filter name.
	static const std::string m_oPipeOutNameX;	///< Out-pipe name.
	static const std::string m_oPipeOutNameY;	///< Out-pipe name.
	static const std::string m_oPipeOutNameR;	///< Out-pipe name.


	/**
	 * @brief CircleFit Constructor.
	 */
	CircleFit();

	/**
	 * @brief Declaration of function doCircleFit
	*/
	void doCircleFit(
			interface::GeoVecAnnotatedDPointarray         dataPointLists,
			geo2d::Doublearray        &	dataOutX,
			geo2d::Doublearray        &	dataOutY,
			geo2d::Doublearray        &	dataOutR,
			int                       p_oPartStart,
			int                       p_oPartEnd,
			int						  p_oMinRadius
            );

	/**
	 * @brief Set filter parameters.
	 */
	void setParameter();

private:

	/**
	 * @brief In-pipe registration.
	 * @param p_rPipe Reference to pipe that is getting connected to the filter.
	 * @param p_oGroup Group number (0 - proceed is called whenever a pipe has a data element, 1 - proceed is only called when all pipes have a data element).
	 */
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);

	void proceed( const void* p_pSender, fliplib::PipeEventArgs& p_rEvent );

	/**
	* @brief Paints ovlerlay.
	*/
	void paint();

	typedef fliplib::SynchronePipe<interface::ImageFrame>		image_pipe_t;

	const fliplib::SynchronePipe< interface::GeoVecAnnotatedDPointarray >*	m_pPipeInData;			///< Data in-pipe.
	fliplib::SynchronePipe< interface::GeoDoublearray >				m_oPipeOutCircleX;			///< Data out-pipe.
	fliplib::SynchronePipe< interface::GeoDoublearray >				m_oPipeOutCircleY;			///< Data out-pipe.
	fliplib::SynchronePipe< interface::GeoDoublearray >				m_oPipeOutCircleR;			///< Data out-pipe.

	int							m_oMode;				///< number of iterations
	int							m_oMinRadius;			///min size of found radius, in pixel
	int							m_oMaxRadius;
	int							m_oPartStart;			///percentual part start left
	int							m_oPartEnd;				///percentual part end right


	interface::SmpTrafo			m_oSpTrafo;				///< roi translation
	image::BImage				m_oResImageOut;			///< result image

	int m_resultX, m_resultY, m_resultR;

	bool m_isValid;
}; 



} // namespace filter
} // namespace precitec


#endif /*CIRCLEFIT_INCLUDED_H*/



