/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief		Data structure which represents surface data calculated on a given roi
*/

#ifndef SURFACEINFO_H_
#define SURFACEINFO_H_


#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "InterfacesManifest.h"

namespace precitec
{
	namespace geo2d
	{

		//class SingleTile;
		//class __declspec(dllimport) VecWrapper : public std::vector<SingleTile> {};   // C4251

		class INTERFACES_API SingleTile
		{
		public:
			SingleTile();
			SingleTile(int startX, int startY, int width, int height, int number);

			void setMeanValue(double meanVal);
			void setRelIntensity(double relIntensityVal);
			void setVariation(double variationValue);
			void setMinMaxDistance(double minMaxDistanceValue);
			void setSurface(double surfaceValue);
			void setSurfaceX(double surfaceXValue);
			void setSurfaceY(double surfaceYValue);
			void setTexture(double textureValue);
			void setStructure(double structureValue);

			void setDefect();

			void setMeanDefect();
			void setRelIntDefect();
			void setVariationDefect();
			void setMinMaxDistanceDefect();
			void setSurfaceDefect();
			void setSurfaceXDefect();
			void setSurfaceYDefect();
			void setTextureDefect();
			void setStructureDefect();

		//private:
			int m_number;

			int m_startX;
			int m_startY;
			int m_width;
			int m_height;

			bool m_isDefect;
			int m_blobNumber;

			bool m_isMeanValid;
			double m_MeanValue;
			bool m_isMeanDefect;

			bool m_isRelIntensityValid;
			double m_relIntensity;
			bool m_isRelIntDefect;

			bool m_isVariationValid;
			double m_Variation;
			bool m_isVariationDefect;

			bool m_isMinMaxDistanceValid;
			double m_MinMaxDistance;
			bool m_isMinMaxDistanceDefect;

			bool m_isSurfaceValid;
			double m_Surface;
			bool m_isSurfaceDefect;

			bool m_isSurfaceXValid;
			double m_SurfaceX;
			bool m_isSurfaceXDefect;

			bool m_isSurfaceYValid;
			double m_SurfaceY;
			bool m_isSurfaceYDefect;

			bool m_isTextureValid;
			double m_Texture;
			bool m_isTextureDefect;

			bool m_isStructureValid;
			double m_Structure;
			bool m_isStructureDefect;


		};

		///////////////////
		// TileContainer //
		///////////////////

		class INTERFACES_API TileContainer
		{
		public:
			TileContainer();
			TileContainer(int sizeX, int sizeY,
				int tileWidth, int tileHeight,
				int tileOffsetX, int tileOffsetY,
				int tileJumpX, int tileJumpY,
				bool TwoRows = false);
			void putSingleTile(int x, int y, SingleTile tile);
			const SingleTile &getSingleTile(int x, int y) const;
			bool empty() const;

			int m_sizeX;
			int m_sizeY;

			std::vector<SingleTile> m_Tiles;
		};

		/////////////////
		// SurfaceInfo //
		/////////////////

		class INTERFACES_API SurfaceInfo
		{

		public:
			SurfaceInfo();

			TileContainer getTileContainer();
			void setTileContainer(TileContainer container);
			void reset();

            bool usesMean;
			bool usesRelBrightness;
			bool usesVariation;
			bool usesMinMaxDistance;
			bool usesSurface;
			bool usesSurfaceX;
			bool usesSurfaceY;
			bool usesTexture;
			bool usesStructure;


		private:

			TileContainer m_TileContainer;
		};



	} // namespace geo2d
} // namespace precitec


#endif /* SURFACEINFO_H_ */
