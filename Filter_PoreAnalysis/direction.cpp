/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		HS
 * 	@date		2013
 * 	@brief		Enum which represents the moore neihboorhood (8 directions) and helper functions which work with positions and directions.
 */

// WM includes
#include "direction.h"
// Poco includes
#include "Poco/Bugcheck.h"
// Stdlib includes
#include <sstream>


namespace precitec {
	using namespace geo2d;
	using namespace Poco;
namespace filter {



Point getNeighborFrom(Point p_oPosition, Dir p_oDirection) {
	switch (p_oDirection) {
		case  N:  return Point( p_oPosition.x,		p_oPosition.y - 1	);
		case  NE: return Point( p_oPosition.x + 1,	p_oPosition.y - 1	);
		case  E:  return Point( p_oPosition.x + 1,	p_oPosition.y		);
		case  SE: return Point( p_oPosition.x + 1,	p_oPosition.y + 1	);
		case  S:  return Point( p_oPosition.x,		p_oPosition.y + 1	);
		case  SW: return Point( p_oPosition.x - 1,	p_oPosition.y + 1 );
		case  W:  return Point( p_oPosition.x - 1,	p_oPosition.y		);
		case  NW: return Point( p_oPosition.x - 1,	p_oPosition.y - 1	);
		default: {
			std::ostringstream oMsg;
			oMsg << "No case for switch argument: " << p_oDirection;
			poco_bugcheck_msg(oMsg.str().c_str());
			return Point();
		}
	} // switch
} // getNeighborFrom



Dir getDir(Point p_oBasePoint, Point p_oReferencePoint) {
	int dx = p_oReferencePoint.x - p_oBasePoint.x;
	int dy = p_oReferencePoint.y - p_oBasePoint.y;
	if ( dx == -1 ) {
		if ( dy == -1 ) return NW;
		if ( dy ==  0 ) return  W;
		if ( dy ==  1 ) return SW;
	}
	if ( dx == 0  ) {
		if ( dy == -1 ) return  N;
		if ( dy ==  1 ) return  S;
	}
	if ( dx == 1  ) {
		if ( dy == -1 ) return NE;
		if ( dy ==  0 ) return  E;
		if ( dy ==  1 ) return SE;
	}

	poco_assert_dbg(false);
	return N;  // dummy
} // getDir



void get90GradDirection(Dir p_oDirection, Dir &p_rDirection90In, Dir &p_rDirection90Out) {
	switch (p_oDirection) {
		// clockwise:

		case  N:  {p_rDirection90In = E;  p_rDirection90Out = W;  break;};
		case  NE: {p_rDirection90In = SE; p_rDirection90Out = NW; break;};
		case  E:  {p_rDirection90In = S;  p_rDirection90Out = N;  break;};
		case  SE: {p_rDirection90In = SW; p_rDirection90Out = NE; break;};
		case  S:  {p_rDirection90In = W;  p_rDirection90Out = E;  break;};
		case  SW: {p_rDirection90In = NW; p_rDirection90Out = SE; break;};
		case  W:  {p_rDirection90In = N;  p_rDirection90Out = S;  break;};
		case  NW: {p_rDirection90In = NE; p_rDirection90Out = SW; break;};

		// counter-clockwise:

		//case  N:  {p_rDirection90In = W;  p_rDirection90Out = E;  break;};
		//case  NE: {p_rDirection90In = NW; p_rDirection90Out = SE; break;};
		//case  E:  {p_rDirection90In = N;  p_rDirection90Out = S;  break;};
		//case  SE: {p_rDirection90In = NE; p_rDirection90Out = SW; break;};
		//case  S:  {p_rDirection90In = E;  p_rDirection90Out = W;  break;};
		//case  SW: {p_rDirection90In = SE; p_rDirection90Out = NW; break;};
		//case  W:  {p_rDirection90In = S;  p_rDirection90Out = N;  break;};
		//case  NW: {p_rDirection90In = SW; p_rDirection90Out = NE; break;};
		//default:  { 
		//	std::ostringstream oMsg;
		//	oMsg << "No case for switch argument: " << p_oDirection;
		//	poco_bugcheck_msg(oMsg.str().c_str()); 
		//}
	} // switch
} // get90GradDirection



} // namespace filter
} // namespace precitec
