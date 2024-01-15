/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIH, ?
 * 	@date		2012
 * 	@brief		altlast
 */

#ifndef SOUVISSOURCEEXPORTEDTYPES_H_
#define SOUVISSOURCEEXPORTEDTYPES_H_

namespace precitec {

struct SSF_SF_ImageInfoT
{
public:

	int iImageNumber;  //1..iLastImageNumber
	int iFirstImageNumber;
	int iLastImageNumber;
};

struct SSF_SF_InputStruct
{
public:
	const unsigned char *img;
	int pitch;
	int npixx;
	int npixy;

	const unsigned char *roistart;
	int roix0;
	int roiy0;
	int roidx;
	int roidy;

	SSF_SF_ImageInfoT SSF_SF_ImageInfo;

};

struct SSF_SF_InputStruct_updatechecked : public SSF_SF_InputStruct
{
	bool isUpdated;
};

} // namespace precitec


#endif /* SOUVISSOURCEEXPORTEDTYPES_H_ */
