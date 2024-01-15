/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		HS
 * 	@date		2013
 * 	@brief		Filter which computes the gradient of a pore.
 */

#ifndef POREGRADIENT_INCLUDED_20130304_H_
#define POREGRADIENT_INCLUDED_20130304_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include "common/frame.h"
#include "geo/geo.h"
#include "geo/array.h"

namespace precitec {
namespace filter {

/**
 * @brief Filter which computes the gradient of a pore.
 */
class FILTER_API PoreGradient : public fliplib::TransformFilter
{
public:

	/**
	 * @brief CTor.
	 */
	PoreGradient();

	/**
	 * @brief Set filter parameters.
	 */
	void setParameter();

	/**
	 * @brief Paint overlay output.
	 */
	void paint();


	// Declare constants

	static const std::string		m_oFilterName;						///< Filter name.
	static const std::string		m_oPipeOutPoreGradientName;				///< Pipe name: out-pipe.
	static const std::string		m_oParamNbNeighboors;				///< Parameter name.
	static const std::string		m_oParamOuterNeighboorDistance;		///< Parameter name.

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

	/**
	 * @brief Computes the gradient of a blob, i.e. the contrast between surrounding intensity and inner blob intensity.
	 * @param p_rImageIn				Grey image to be processed.
	 * @param p_rBlobsIn				Blob list to be processed.
	 * @param p_rPoreGradientOut			Calculated gradient for each blob.
	 * @param p_oNbNeighboors			Number of pixels perpendicular to the contour, which are used for gradient calculation.
	 * @param p_oOuterNeighboorDistance	Distance, before outer neighboor pixels are used for gradient.
	 */
	void calcPoreGradient(
		const image::BImage&		p_rImageIn, 
		const geo2d::Blobarray&		p_rBlobsIn, 
		geo2d::Doublearray&			p_rPoreGradientOut,
		unsigned int				p_oNbNeighboors,
		unsigned int				p_oOuterNeighboorDistance);


	typedef fliplib::SynchronePipe<interface::ImageFrame>		image_pipe_t;
	typedef fliplib::SynchronePipe<interface::GeoBlobarray>		blob_pipe_t;
	typedef fliplib::SynchronePipe<interface::GeoDoublearray>	scalar_pipe_t;

	const image_pipe_t*			m_pPipeInImageFrame;		///< in pipe
	const blob_pipe_t*			m_pPipeInBlob;				///< in-pipe.
	scalar_pipe_t 				m_oPipeOutPoreGradient;			///< out-pipe.

	interface::SmpTrafo			m_oSpTrafo;					///< roi translation

	unsigned int 				m_oNbNeighboors;			///< parameter
	unsigned int 				m_oOuterNeighboorDistance;	///< parameter

	geo2d::Doublearray			m_oPoreGradientOut;
}; // class PoreGradient

} // namespace filter
} // namespace precitec

#endif /* POREGRADIENT_INCLUDED_20130304_H_ */
