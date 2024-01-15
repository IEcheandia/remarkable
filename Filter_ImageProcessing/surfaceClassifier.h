/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2017
* 	@brief		Filter 'SurfaceClassifier'. Gets a SurfaceInfo structure and checks the given limits
*/

#ifndef SURFACECLASSIFIER_H_
#define SURFACECLASSIFIER_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"

#include "common/frame.h"
#include "common/geoContext.h"
#include "filter/parameterEnums.h"
#include <geo/geo.h>

// std lib includes
#include <functional>

#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"


namespace precitec {
	using namespace image;
	namespace filter {


		struct SurfacePoint
		{
			SurfacePoint();
			SurfacePoint(int x, int y, Color color);
			int x;
			int y;
			Color color;
		};

		struct SurfaceLine
		{
			SurfaceLine();
			SurfaceLine(int x1, int y1, int x2, int y2, Color color);
			int x1;
			int y1;
			int x2;
			int y2;
			Color color;
		};

		struct SurfaceRectangle
		{
			SurfaceRectangle();
			SurfaceRectangle(int x, int y, int width, int height, Color color);
			int x;
			int y;
			int width;
			int height;
			Color color;
		};

		class SurfaceOverlay
		{
		public:
			SurfaceOverlay();

			void reset();

			std::vector<SurfacePoint> getPointContainer();
			std::vector<SurfaceLine> getLineContainer();
			std::vector<SurfaceRectangle> getRectangleContainer();

			void addPoint(int x, int y, Color color);
			void addLine(int x1, int y1, int x2, int y2, Color color);
			void addRectangle(int x, int y, int width, int height, Color color);

		private:
			std::vector<SurfacePoint> _pointContainer;
			std::vector<SurfaceLine> _lineContainer;
			std::vector<SurfaceRectangle> _rectangleContainer;
		};

		class SurfaceInfoLine
		{
		public:
			SurfaceInfoLine();
			SurfaceInfoLine(int number, int value, Color color);
			std::string getLine() const;

			int _number, _value;
			Color _color;
		};


		/**
		* @brief Divides an image into tiles. The tiles may overlap. For each tile a texture feature is calculated.
		*/
		class FILTER_API SurfaceClassifier : public fliplib::TransformFilter
		{
		public:

			/**
			* @brief CTor.
			*/
			SurfaceClassifier();

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
			static const std::string m_oPipeOutSizeConnectedName;	///< Pipe name: out-pipe.
			static const std::string m_oPipeOutWidthNioName;	///< Pipe name: out-pipe.
			static const std::string m_oPipeOutHeightNioName;	///< Pipe name: out-pipe.


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

			const fliplib::SynchronePipe< interface::GeoSurfaceInfoarray > * m_pPipeInSurfaceInfo;		///< in-pipe
			fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutSizeConnected;				///< out-pipe.
			fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutWidthNio;				///< out-pipe.
			fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutHeightNio;				///< out-pipe.

			geo2d::SurfaceInfo m_oInInfo;
			interface::SmpTrafo m_oSpTrafo;

			int					m_oDisplay;
			bool				m_oUse4Neighborhood;

			double				m_oMinMean;
			double				m_oMaxMean;
			double				m_oMinRelInt;
			double				m_oMaxRelInt;
			double				m_oMinVariation;
			double				m_oMaxVariation;
			double				m_oMinMinMaxDistance;
			double				m_oMaxMinMaxDistance;
			double				m_oMinSurface;
			double				m_oMaxSurface;
			double				m_oMinSurfaceX;
			double				m_oMaxSurfaceX;
			double				m_oMinSurfaceY;
			double				m_oMaxSurfaceY;
			double				m_oMinTexture;
			double				m_oMaxTexture;
			double				m_oMinStructure;
			double				m_oMaxStructure;

			precitec::geo2d::TileContainer m_tileContainer;

			//Funktionen zur Analyse des TileContainers
			void tagBlobs(precitec::geo2d::TileContainer & tileContainer);
			void combineEqualBlobs(precitec::geo2d::TileContainer & tileContainer, int firstNumber, int secondNumber);
			int getMaxBlobNumber(precitec::geo2d::TileContainer & tileContainer);
			int findBiggestBlob(precitec::geo2d::TileContainer & tileContainer);
			void deleteBlobNumbersExceptGiven(precitec::geo2d::TileContainer & tileContainer, int givenNumber);
			int getTotalBlobSize(precitec::geo2d::TileContainer & tileContainer, int & blobWidth, int & blobHeight);

			bool m_hasPainting;

			std::vector<std::vector<SurfaceInfoLine>> _allInfoLines;

			SurfaceOverlay _overlay;
			std::string m_oOverlayString;


		}; // class TileFeature

	} // namespace filter
} // namespace precitec

#endif /* SURFACECLASSIFIER_H_ */
