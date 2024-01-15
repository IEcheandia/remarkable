/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIH, HS
 * 	@date		2012
 * 	@brief		Data structure which represents properties of a single blob (Blob).
 */



#include "geo/blob.h"
#include <cstdio>


namespace precitec {
namespace geo2d {

// constructor
Blob::Blob () 
	:
	sx		( 0 ),
	sy		( 0 ),	// 32 Bit
	si		( 0 ),	// 25 Bit
	npix	( 0 ),	// 18 Bit	
	xmin	( 0 ),	// 9 Bit
	xmax	( 0 ),	// 9 Bit
	ymin	( 0 ),	// 9 Bit
	ymax	( 0 ),	// 9 Bit
	startx	( 0 ),	// 9 Bit
	starty	( 0 )	// 9 Bit
{}




inline bool Blob::operator == (const Blob& p_rOther) const {
	if (p_rOther.sx != sx)		return false;
	if (p_rOther.sx != sx)		return false;
	if (p_rOther.sy != sy)		return false;
	if (p_rOther.si != si)		return false;
	if (p_rOther.xmin != xmin)	return false;
	if (p_rOther.xmax != xmax)	return false;
	if (p_rOther.ymin != ymin)	return false;
	if (p_rOther.ymax != ymax)	return false;
	if (p_rOther.npix != npix)	return false;
	if (p_rOther.startx != startx)	return false;
	if (p_rOther.starty != starty)	return false;
	return true;
} // operator ==



std::ostream& operator << (std::ostream& os,  Blob const & v) {
	os <<" xmin=" << v.xmin;
	os <<" xmax=" << v.xmax;
	os <<" ymin=" << v.ymin;
	os <<" ymax=" << v.ymax;
	os <<"  npix=" << v.npix;
	os <<" si=" << v.si;
	os <<" sx=" << v.sx;
	os <<" sy=" << v.sy;
	//os <<" root=" << v.root;
	return os;
} // operator <<



DataBlobDetectionT::DataBlobDetectionT()
{
	nspots=0;
	outspot=NULL;
	noutspotsmax=0;
}


DataBlobDetectionT::~DataBlobDetectionT()
{
	//if (outspot!=NULL)
	//{
	//	std::cerr << "WARNING: DataBlobDetectionT was not freed.\n";
	//	free();
	//}
}



int DataBlobDetectionT::alloc(int n)
{
	nspots=0;


	if (n<=noutspotsmax) return 0;


	if (outspot!=NULL)
	{
		delete []  outspot;
	}

	outspot=new Blob[n];
	if (outspot!=NULL)
	{
		noutspotsmax=n;
		return 0;
	}
	else
	{
		outspot=NULL;
		noutspotsmax=0;
		return 1;
	}

}



void DataBlobDetectionT::free()
{

	if (outspot!=nullptr)
	{
		delete []  outspot;
		outspot = nullptr;
	}
	nspots=0;
	noutspotsmax=0;

}



} // namespace geo2d
} // namespace precitec
