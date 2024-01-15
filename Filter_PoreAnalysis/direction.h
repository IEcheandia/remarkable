/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		HS
 * 	@date		2013
 * 	@brief		Enum which represents the moore neihboorhood (8 directions) and helper functions which work with positions and directions.
 */

#ifndef DIRECTION_INCLUDED_20130307_H
#define DIRECTION_INCLUDED_20130307_H


// WM includes
#include "geo/point.h"
// Stdlib includes
#include <string>


namespace precitec {
namespace filter {

/**
 * @brief Enum that represents the Moore neighbourhood (8 neighbourhood) directions of a pixel using the 8 compass points. 
 */
enum Dir {N, NE, E, SE, S, SW, W, NW};
static const std::string g_oDirString[8] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"}; // debug

/**
 * @brief This operator provides clockwise rotation of a direction. 
 */
inline Dir operator ++(Dir &p_rDir) { return p_rDir = Dir(( int(p_rDir) + 1 ) & 7); }

/**
 * @brief This operator provides counterclockwise rotation of a direction. 
 */
inline Dir operator --(Dir &p_rDir) { return p_rDir = Dir(( int(p_rDir) - 1 + 8 ) & 7); }

/**
  * @brief Returns the neighboor position for a certain direction.
  * @param p_oPosition		Current position.
  * @param p_oDirection		Direction.
  */
geo2d::Point getNeighborFrom(geo2d::Point p_oPosition, Dir p_oDirection);

/**
  * @brief Returns the direction between two neighboored positions.
  * @param p_oBasePoint			First position.
  * @param p_oReferencePoint	Second position.
  */
Dir getDir(geo2d::Point p_oBasePoint, geo2d::Point p_oReferencePoint);

/**
  * @brief Given a direction, the two perpendicular directions are calculated. 
  * @details	If the direction describes a contour direction (counter-clockwise), the two output directions point to the inner and outer direction relative to the contour.
  * @param p_oDirection			Given direction.
  * @param p_rDirection90In		First perpendicular direction.
  * @param p_rDirection90Out	Second perpendicular direction.
  */
void get90GradDirection(Dir p_oDirection, Dir &p_rDirection90In, Dir &p_rDirection90Out);


} // namespace filter
} // namespace precitec

#endif /* DIRECTION_INCLUDED_20130307_H */
