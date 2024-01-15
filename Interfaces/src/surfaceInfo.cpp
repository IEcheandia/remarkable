/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief		Data structure which represents surface data calculated on a given roi
*/

#include "geo/surfaceInfo.h"
#include <cstdio>
#include <cassert>

namespace precitec
{
	namespace geo2d
	{
		SurfaceInfo::SurfaceInfo()
		{}

		TileContainer SurfaceInfo::getTileContainer()
		{
			return m_TileContainer;
		}

		void SurfaceInfo::setTileContainer(TileContainer container)
		{
			m_TileContainer = container;
		}

		void SurfaceInfo::reset()
		{
			usesMean = false;
			usesRelBrightness = false;
			usesVariation = false;
			usesMinMaxDistance = false;
			usesSurface = false;
			usesSurfaceX = false;
			usesSurfaceY = false;
			usesTexture = false;
			usesStructure = false;
			setTileContainer(TileContainer{});
		}

		////////////////
		// SingleTile //
		////////////////

		SingleTile::SingleTile()
		{
			m_startX = 0;
			m_startY = 0;
			m_width = 0;
			m_height = 0;
			m_number = 0;

			m_isDefect = false;
			m_blobNumber = 0;

			m_isMeanValid = false;
			m_MeanValue = 0;
			m_isMeanDefect = false;

			m_isRelIntensityValid = false;
			m_relIntensity = 0;
			m_isRelIntDefect = false;

			m_isVariationValid = false;
			m_Variation = 0;
			m_isVariationDefect = false;

			m_isMinMaxDistanceValid = false;
			m_MinMaxDistance = 0;
			m_isMinMaxDistanceDefect = false;

			m_isSurfaceValid = false;
			m_Surface = 0;
			m_isSurfaceDefect = false;

			m_isSurfaceXValid = false;
			m_SurfaceX = 0;
			m_isSurfaceXDefect = false;

			m_isSurfaceYValid = false;
			m_SurfaceY = 0;
			m_isSurfaceYDefect = false;

			m_isTextureValid = false;
			m_Texture = 0;
			m_isTextureDefect = false;

			m_isStructureValid = false;
			m_Structure = 0;
			m_isStructureDefect = false;

		}

		SingleTile::SingleTile(int startX, int startY, int width, int height, int number)
		{
			m_startX = startX;
			m_startY = startY;
			m_width = width;
			m_height = height;
			m_number = number;

			m_isDefect = false;
			m_blobNumber = 0;

			m_MeanValue = 0;
			m_isMeanValid = false;			
			m_isMeanDefect = false;

			m_relIntensity = 0;
			m_isRelIntensityValid = false;
			m_isRelIntDefect = false;
		}

		void SingleTile::setMeanValue(double meanVal)
		{
			m_MeanValue = meanVal;
			m_isMeanValid = true;
		}

		void SingleTile::setRelIntensity(double relIntensityVal)
		{
			m_relIntensity = relIntensityVal;
			m_isRelIntensityValid = true;
		}

		void SingleTile::setVariation(double variationValue)
		{
			m_Variation = variationValue;
			m_isVariationValid = true;
		}

		void SingleTile::setMinMaxDistance(double minMaxDistanceValue)
		{
			m_MinMaxDistance = minMaxDistanceValue;
			m_isMinMaxDistanceValid = true;
		}

		void SingleTile::setSurface(double surfaceValue)
		{
			m_Surface = surfaceValue;
			m_isSurfaceValid = true;
		}

		void SingleTile::setSurfaceX(double surfaceXValue)
		{
			m_SurfaceX = surfaceXValue;
			m_isSurfaceXValid = true;
		}

		void SingleTile::setSurfaceY(double surfaceYValue)
		{
			m_SurfaceY = surfaceYValue;
			m_isSurfaceYValid = true;
		}

		void SingleTile::setTexture(double textureValue)
		{
			m_Texture = textureValue;
			m_isTextureValid = true;
		}

		void SingleTile::setStructure(double structureValue)
		{
			m_Structure = structureValue;
			m_isStructureValid = true;
		}

		void SingleTile::setDefect()
		{
			m_isDefect = true;
		}

		void SingleTile::setMeanDefect()
		{
			m_isMeanDefect = true;
		}

		void SingleTile::setRelIntDefect()
		{
			m_isRelIntDefect = true;
		}

		void SingleTile::setVariationDefect()
		{
			m_isVariationDefect = true;
		}

		void SingleTile::setMinMaxDistanceDefect()
		{
			m_isMinMaxDistanceDefect = true;
		}

		void SingleTile::setSurfaceDefect()
		{
			m_isSurfaceDefect = true;
		}

		void SingleTile::setSurfaceXDefect()
		{
			m_isSurfaceXDefect = true;
		}

		void SingleTile::setSurfaceYDefect()
		{
			m_isSurfaceYDefect = true;
		}

		void SingleTile::setTextureDefect()
		{
			m_isTextureDefect = true;
		}

		void SingleTile::setStructureDefect()
		{
			m_isStructureDefect = true;
		}


		///////////////////
		// TileContainer //
		///////////////////

		TileContainer::TileContainer()
		{
			m_sizeX = 0;
			m_sizeY = 0;
			assert(empty());
		}

		TileContainer::TileContainer(int sizeX, int sizeY,
									int tileWidth, int tileHeight,
									int tileOffsetX, int tileOffsetY,
									int tileJumpX, int tileJumpY,
									bool TwoRows)
		{
			m_sizeX = sizeX;
			m_sizeY = sizeY;

			m_Tiles.clear();
			m_Tiles.reserve(sizeX*sizeY);

			if (TwoRows)
			{
				// Special setting for 2 row detection (Voest)
				for (int j = 0; j < sizeY; j++)
				{
					// For regular row
					{
						SingleTile tile;
						tile.m_startX = tileOffsetX;
						tile.m_startY = tileOffsetY + j * tileJumpY;
						tile.m_width = tileWidth;
						tile.m_height = tileHeight;
						tile.m_number = 1 + j * sizeX;  // sizeX = number of rows = 2
						m_Tiles.push_back(tile);
						assert( (int) m_Tiles.size() == tile.m_number);
						assert(getSingleTile(0, j).m_number == tile.m_number);
					}

					// For 'percentage row'
					{
						SingleTile tile;
						// tileJumpX = width of 'percentage row'
						tile.m_startX = tileOffsetX + (tileWidth - tileJumpX) / 2;
						tile.m_startY = tileOffsetY + j*tileJumpY;
						tile.m_width = tileJumpX;
						tile.m_height = tileHeight;
						tile.m_number = 2 + j * sizeX;  // sizeX = number of rows = 2
						m_Tiles.push_back(tile);
						assert( (int) m_Tiles.size() == tile.m_number);
						assert(getSingleTile(1, j).m_number == tile.m_number);
					}
				}
			}
			else
			{
				// Original function
				for (int j = 0; j < sizeY; j++)
				{
					for (int i = 0; i < sizeX; i++)
					{
						SingleTile tile;
						tile.m_startX = tileOffsetX + i*tileJumpX;
						tile.m_startY = tileOffsetY + j*tileJumpY;
						tile.m_width = tileWidth;
						tile.m_height = tileHeight;
						tile.m_number = (i + 1) + j * sizeX;
						m_Tiles.push_back(tile);
						assert( (int) m_Tiles.size() == tile.m_number);
						assert(getSingleTile(i, j).m_number == tile.m_number);
					}
				}
			}
		}

		const SingleTile &TileContainer::getSingleTile(int x, int y) const
		{
			// Check range
			return m_Tiles[y*m_sizeX + x];
		}

		void TileContainer::putSingleTile(int x, int y, SingleTile tile)
		{
			// Check range
			m_Tiles[y*m_sizeX + x] = tile;
		}

		bool TileContainer::empty() const
		{
			assert( (int) m_Tiles.size() == m_sizeX * m_sizeY);
			return m_Tiles.empty();
		}


	} // namespace geo2d
} // namespace precitec
