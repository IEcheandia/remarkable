/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		AB
 * 	@date		2011
 * 	@brief		SigChange structure
 */


#ifndef STRUCTURESFORFILTERS_H_
#define STRUCTURESFORFILTERS_H_

#include "Analyzer_Interface.h"
#include "parameterEnums.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"


namespace precitec {
	namespace filter {

	/// signature change struct type/ pos
	struct SigChange
	{
	public:
		SigChange()
		{
			oPos = -1;
			oType = 0;
		}
		SigChange(int pos, int type)
		{
			oPos = pos;
			oType = type;
		}
		int oPos;           // position
		int oType;    // type
	};

	ANALYZER_INTERFACE_API TRunOrientation valueToOrientation(const int);
	ANALYZER_INTERFACE_API TRunOrientation valueToOrientation(const double);

	} // namespace filter
} // namespace precitec


#endif // ARMSTATES_20110905_H_
