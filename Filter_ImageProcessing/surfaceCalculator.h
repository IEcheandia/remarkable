/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2017
* 	@brief		Filter 'SurfaceCalculator'. Divides an image into tiles. The tiles may overlap. For each tile some surface texture features are calculated.
*/

#ifndef SURFACECALCULATOR_H_
#define SURFACECALCULATOR_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"

#include "common/frame.h"
#include "common/geoContext.h"
#include "filter/parameterEnums.h"



namespace precitec {
    namespace image
    {
        //forward decalaration
        class OverlayCanvas;
    }
	namespace filter {

		/**
		* @brief Divides an image into tiles. The tiles may overlap. For each tile a texture feature is calculated.
		*/
		class FILTER_API SurfaceCalculator : public fliplib::TransformFilter
		{
		public:

			/**
			* @brief CTor.
			*/
			SurfaceCalculator();

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
			static const std::string m_oPipeOutSurfaceInfoName;	///< Pipe name: out-pipe.

			static void initializeFilterParameterContainer(geo2d::SurfaceInfo & surfaceInfo, fliplib::ParameterContainer & parameters_);
			static void updateFromParameterContainer(geo2d::SurfaceInfo & surfaceInfo, fliplib::ParameterContainer & parameters_);
            static void paintTiles(image::OverlayCanvas& rOverlayCanvas, 
                const geo2d::TileContainer & r_tileContainer, const interface::Trafo &rTrafo,
                const image::BImage & r_lastImageTmp, const std::string & r_lastTitleImageTmp, bool paintLastImage);
            static void computeTiles(const geo2d::SurfaceInfo & rSurfaceInfo,
                geo2d::TileContainer & tileContainer,
                const image::BImage & rImageIn,
                image::BImage & r_lastImageTmp, std::string & r_lastTitleImageTmp);
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
			
			const image_pipe_t*										m_pPipeInImageFrame;		///< in-pipe
			fliplib::SynchronePipe<interface::GeoSurfaceInfoarray> 	m_oPipeOutSurfaceInfo;		///< out-pipe.

			interface::GeoSurfaceInfoarray		m_oSurfaceInfoOut;	///< our frame including feature image

            geo2d::SurfaceInfo m_oSurfaceInfo;
			int m_oTileWidth;
			int m_oTileJumpX;
			int m_oTileHeight;
			int m_oTileJumpY;
            bool m_oEnsureTile;

			interface::SmpTrafo m_oSpTrafo;

			unsigned int				m_oTileSize;				///< Filter parameter - Size of a tile.
			unsigned int				m_oJumpingDistance;			///< Filter parameter - Tile sampling distance in x and y direction.

			bool m_badInput;
			bool m_hasPainting;

			image::BImage m_lastImageTmp;
			std::string m_lastTitleImageTmp;

		}; // class TileFeature

	} // namespace filter
} // namespace precitec

#endif /* SURFACECALCULATOR_H_ */
