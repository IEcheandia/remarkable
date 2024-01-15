/*!
*  @copyright:		Precitec Vision GmbH & Co. KG
*  @author			Simon Hilsenbeck (HS)
*  @date			2013
*  @file			
*  @brief			Binarizes an image depending on threshold.
*/

#ifndef BINARIZEDYNAMIC_H
#define BINARIZEDYNAMIC_H

// local includes
#include "fliplib/Fliplib.h"			// export macro
#include "fliplib/TransformFilter.h"	// base class
#include "fliplib/SynchronePipe.h"		// in- / output

#include "system/types.h"				// byte
#include "common/frame.h"				// ImageFrame
#include "filter/parameterEnums.h"		// ComparisonType

// std lib
#include <string>


namespace precitec
{
namespace filter
{

/**
* @brief	Binarize filter. Binarizes an image depending on threshold.
* @details	Binarizes an 8-bit grey image depending on threshold to zero or 255. Not inplace.
*/
class FILTER_API BinarizeDynamic  : public fliplib::TransformFilter {

public:

	static const std::string m_oFilterName;		///< Filter name.
	static const std::string m_oPipeOut1Name;	///< Out-pipe name.

	/**
	 * @brief CTor.
	 */
	BinarizeDynamic();

private:


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
	void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

	/**
	 * @brief Paints ovlerlay.
	 */
	/*virtual*/ void paint();

    typedef fliplib::SynchronePipe<interface::ImageFrame>		image_pipe_t;
    typedef fliplib::SynchronePipe<interface::GeoDoublearray>  threshold_pipe_t;


	const image_pipe_t*			m_pPipeInImageFrame;	///< in pipe
	const threshold_pipe_t*			m_pPipeInThreshold;

	image_pipe_t				m_oPipeOutImgFrame;		///< out pipe

	ComparisonType 				m_oComparisonType;		///< parameter - comparison type
	BinarizeType 				m_oBinarizeType;		///< parameter - binarize type

	interface::SmpTrafo			m_oSpTrafo;				///< roi translation
    std::array<image::BImage, g_oNbParMax>	m_oBinImageOut;			///< binarized image
}; // class Binarize

} // namespace filter
} // namespace precitec


#endif /*BINARIZE_20110816_INCLUDED_H*/



