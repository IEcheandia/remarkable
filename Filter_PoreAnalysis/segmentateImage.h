/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIH, HS
 * 	@date		2012
 * 	@brief		Labeling algorithm.
 */


#ifndef SEGMENTATEIMAGE_H_
#define SEGMENTATEIMAGE_H_

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "souvisSourceExportedTypes.h"
#include "geo/blob.h"


//#define DEBUG  /##/
#define ERR_TOO_MANY_ENTRIES_IN_SPOTLINE 1
#define ERR_TOO_MANY_ENTRIES_IN_SPOTBUF 2
#define ERR_TOO_MANY_OUTPUTSPOTS 3

#define _ATR_preprocessing_N_spots_per_line 64
//Maximum number of spots to be open
#define _ATR_preprocessing_N_spots 64
//void printflags(sc_uint<64> w);


namespace precitec {
namespace filter {



//*********SPOTLINE******************************************************************************************
class spotline
	{
	public:
	unsigned short xmin; //9Bit
	unsigned short xmax; //9Bit
	unsigned short myspotnumber; //6 Bit
	spotline();
    inline bool operator == (const spotline & b) const;
    inline spotline& operator = (const spotline & b);
	inline friend std::ostream& operator << ( std::ostream& os,  spotline const & v );

};


unsigned char default_segmentateimage_binfunction(SSF_SF_InputStruct & img,unsigned long i);
extern unsigned char (* segmentateimage_binfunction) (SSF_SF_InputStruct & img,unsigned long i);

unsigned short segmentateimage(SSF_SF_InputStruct & img,geo2d::DataBlobDetectionT & DataBlobDetection, unsigned int p_oMinBlobSize);

void printflags(unsigned long long w);
void changeroot(unsigned short oldroot, unsigned short newroot);
void updatenode(unsigned short rootspotnumber,unsigned short childspotnumber);
void addpixelto(unsigned short spotnumber ,unsigned short wpixwert,unsigned short x,unsigned short y);
int getnewspot(unsigned short & newspotnumber);
void outputspot(int & nspots,geo2d::Blob *outspot,int nspotsmax, unsigned int p_oMinBlobSize);
void releasefreespots(void);
void mPowerup_init(void);

} // namespace filter
} // namespace precitec


#endif /* SEGMENTATEIMAGE_H_ */
