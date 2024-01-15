#pragma once

#include "direction.h"
#include "image/image.h"
#include "geo/geo.h"
#include "module/moduleLogger.h"

namespace precitec
{
namespace filter
{
/**
  * @brief Provides the next contour point and its direction. Returns false if no neighboor was found (isolated pixel).
  * @param p_rImageIn       Binary image.
  * @param p_rDir           Current direction. Will be overwritten with next direction.
  * @param p_rCurrentPos    Current position. Will be overwritten with next position.
  */
bool getNextContourPointAndDirection(const image::BImage& p_rImageIn, Dir &p_rDir, geo2d::Point &p_rCurrentPos);

/**
  * @brief Computes the contour of a blob.
  * @param p_rImageIn Binary image to be processed.
  * @param p_rBlobsOut Blob list to be processed. The extracted contour is added to each blob.
  */
void calcContour (const image::BImage& p_rImageIn, geo2d::Blobarray& p_rBlobsOut, geo2d::Doublearray & oValX, geo2d::Doublearray & oValY, std::vector<geo2d::AnnotatedDPointarray> & oValPointLists,
    const std::string& filterName
);
}
}
