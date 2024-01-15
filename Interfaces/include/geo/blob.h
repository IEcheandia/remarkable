/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIH, HS
 * 	@date		2012
 * 	@brief		Data structure which represents properties of a single blob (Blob).
 */

#ifndef DATABLOBDETECTION_H_
#define DATABLOBDETECTION_H_


#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "InterfacesManifest.h"
#include "geo/point.h"


namespace precitec {
namespace geo2d {

class INTERFACES_API Blob {
public:
    Blob ();

	bool operator == (const Blob& p_rOther) const;

	INTERFACES_API friend std::ostream& operator << ( std::ostream& os,  Blob const & v );


    unsigned long long	sx;			///< 64 Bit	mass center in x-direction, unnormalized?
	unsigned long long	sy;			///< 64 Bit	mass center in y-direction, unnormalized?
	unsigned long		si;			///< 25 Bit	mass center normalizing factor? Value is always equal to npix.
	unsigned long		npix;		///< 18 Bit	number of pixels
	unsigned short		xmin;		///< 9 Bit	minimal position in x-direction
	unsigned short		xmax;		///< 9 Bit	maximal position in x-direction
	unsigned short		ymin;		///< 9 Bit	minimal position in y-direction
	unsigned short		ymax;		///< 9 Bit	maximal position in y-direction
	unsigned short		startx;		///< start point
	unsigned short		starty;		///< start point

	std::vector<Point>	m_oContour;	///< Contour of blob represented as contiguous positions.
};



// deprecated, because redundant to class Blob, but needed within segmentateimage().
class INTERFACES_API DataBlobDetectionT {
public:
	int nspots;
	Blob *outspot;
	int noutspotsmax;

	DataBlobDetectionT();
	~DataBlobDetectionT();

	int alloc(int n);
	void free();
};


} // namespace geo2d
} // namespace precitec


#endif /* DATABLOBDETECTION_H_ */
