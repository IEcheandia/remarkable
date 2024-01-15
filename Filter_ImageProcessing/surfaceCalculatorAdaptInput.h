/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		LB
* 	@date		2019
* 	@brief		Filter 'SurfaceCalculatorAdaptInput'. Equal to SurfaceCalculator, with an additional input to enlarge the input ROI
*/


#ifndef SURFACECALCULATORADAPTINPUT_H_
#define SURFACECALCULATORADAPTINPUT_H_

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
    namespace image
    {
        //forward decalaration
        class OverlayCanvas;
    }
	namespace filter {

		/**
		* @brief Divides an image into tiles. The tiles may overlap. For each tile a texture feature is calculated.
		*/
		class FILTER_API SurfaceCalculatorAdaptInput : public fliplib::TransformFilter
		{
		public:

			/**
			* @brief CTor.
			*/
			SurfaceCalculatorAdaptInput();

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
			void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEvent);

		private:
			typedef fliplib::SynchronePipe<interface::ImageFrame>		image_pipe_t;
			interface::SmpTrafo updateNewROI(const geo2d::Size minOutSize, 
				const interface::SmpTrafo& roiInTrafo, const geo2d::Size& roiInSize,
				const interface::SmpTrafo& sourceImageTrafo, const image::BImage& sourceImage);
			
			const image_pipe_t*										m_pPipeInImageFrame;		///< in-pipe
			const image_pipe_t*										m_pPipeInExtendedROIFrame;		///< in-pipe
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

			image::BImage m_oNewROI;

		}; // class TileFeature

	} // namespace filter
} // namespace precitec

#endif /* SURFACECALCULATORADAPTINPUT_H_ */
