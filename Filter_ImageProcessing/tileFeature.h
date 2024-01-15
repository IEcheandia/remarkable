/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		HS
 * 	@date		2013
 * 	@brief		Filter 'TileFeature'. Divides an image into tiles. The tiles may overlap. For each tile a texture feature is calculated.
 */

#ifndef TILEFEATURE_H_20131023_INCLUDED
#define TILEFEATURE_H_20131023_INCLUDED

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"

#include "common/frame.h"
#include "common/geoContext.h"
#include "filter/parameterEnums.h"
// std lib includes
#include <functional>


namespace precitec {
namespace filter {

/**
 * @brief Divides an image into tiles. The tiles may overlap. For each tile a texture feature is calculated.
 */
class FILTER_API TileFeature : public fliplib::TransformFilter
{
public:

	/**
	 * @brief CTor.
	 */
	TileFeature();

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
	static const std::string m_oPipeOutFeatureImageName;	///< Pipe name: out-pipe.


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
	void proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rEvent);

private:
	typedef fliplib::SynchronePipe<interface::ImageFrame>		image_pipe_t;

	/**
	 * @brief Computes a texture feature within a tile.
	 * @param p_rImageIn			Image to be processed.
	 * @param p_rImageOut			Feature image. Smaller.
	 * @param p_oTileSize			Tile sampling distance in x and y direction.
	 * @param p_oJumpingDistance	Size of a tile.
	 * @param p_oAlgorithm			Texture analysis algorithm functor.
	 */
    template <typename AlgoTexture>
	void calcFeatureImg(const image::BImage& p_rImageIn, image::BImage& p_rImageOut, unsigned int p_oTileSize, unsigned int p_oJumpingDistance, AlgoTexture p_oAlgorithm);

	const image_pipe_t*			m_pPipeInImageFrame;		///< in-pipe
	image_pipe_t 				m_oPipeOutFeatureImage;		///< out-pipe.

	interface::ImageFrame		m_oFrameFeatureImageOut;	///< our frame including feature image
	unsigned int				m_oDrawThreshold;			///< treshold, which decides in which color each tile is drawn.

	unsigned int				m_oTileSize;				///< Filter parameter - Size of a tile.
	unsigned int				m_oJumpingDistance;			///< Filter parameter - Tile sampling distance in x and y direction.
	int				m_oAlgoTexture;				///< Filter parameter - Type of texture analysis algorithm
	std::array<image::BImage, g_oNbParMax>				m_oFeatureImageOut;			///< feature image
    interface::Size2D           m_oOffsetFirstTile;         ///< Offset of first tile so the tiles get centered in input image
}; // class TileFeature

} // namespace filter
} // namespace precitec

#endif /* TILEFEATURE_H_20131023_INCLUDED */
