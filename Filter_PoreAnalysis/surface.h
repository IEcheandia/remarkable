/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		HS
 * 	@date		2013
 * 	@brief		Filter which computes the surface of a pore.
 */

#ifndef SURFACE_INCLUDED_20130312_H
#define SURFACE_INCLUDED_20130312_H

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include "common/frame.h"
#include "geo/geo.h"
#include "geo/array.h"
#include "filter/parameterEnums.h"
// std lib includes
#include <functional>


namespace precitec {
namespace filter {

/**
 * @brief Filter which computes the surface of a pore.
 */
class FILTER_API Surface : public fliplib::TransformFilter
{
public:

	/**
	 * @brief CTor.
	 */
	Surface();

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
	static const std::string m_oPipeOutVarianceName;		///< Pipe name: out-pipe.


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
	 * @brief Computes a texture feature within a scaled blob's bounding box. The bounding box can be scaled by another parameter.
	 * @param p_rImageIn Binary image to be processed.
	 * @param p_rBlobsOut Blob list to be processed.
	 * @param p_oBoundingBoxScale Number of pixels the bounding box is made bigger or smaller.
	 */
	void calcTexture(const image::BImage& p_rImageIn, const geo2d::Blobarray& p_rBlobsIn, double p_oBoundingBoxScale);


	typedef fliplib::SynchronePipe<interface::ImageFrame>		image_pipe_t;
	typedef fliplib::SynchronePipe<interface::GeoBlobarray>		blob_pipe_t;
	typedef fliplib::SynchronePipe<interface::GeoDoublearray>	scalar_pipe_t;
	typedef std::function<double (const image::BImage&)>		algo_texture_ftor_t;

	const image_pipe_t*			m_pPipeInImageFrame;	///< in pipe
	const blob_pipe_t*			m_pPipeInBlob;			///< in-pipe.
	scalar_pipe_t 				m_oPipeOutVariance;		///< out-pipe.

	interface::SmpTrafo			m_oSpTrafo;				///< roi translation
	geo2d::Doublearray			m_oVarianceOut;

	std::vector<geo2d::Rect>	m_oScaledRects;

	double						m_oBoundingBoxScale;	///< parameter
	AlgoTextureType				m_oAlgoTexture;			///< type of texture analysis algorithm
	algo_texture_ftor_t			m_oAlgorithm;			///< texture analysis algorithm functor

}; // class Surface

} // namespace filter
} // namespace precitec

#endif /* SURFACE_INCLUDED_20130312_H */
