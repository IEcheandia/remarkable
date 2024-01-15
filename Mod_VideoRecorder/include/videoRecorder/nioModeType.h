/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief		Recording modes concerning NIOs.
 */

#ifndef NIOMODETYPE_H_20120322_INCLUDED
#define NIOMODETYPE_H_20120322_INCLUDED

namespace precitec {
namespace vdr {

/**
 * @ingroup VideoRecorder
 * @brief	Recording modes concerning NIOs.
 * @details	The number of images recorded may depend on the occurrence of NIOs.
 */
enum NioModeType {
	eAllImages,					///< record all images
	eNioOnly,					///< record nio images only
	eNioSeamOnly,				///< record only images of seams, wherein a nio occurred
	eNioModeMin	= eAllImages,	///< delimiter
	eNioModeMax	= eNioSeamOnly	///< delimiter
}; // NioModeType

} // vdr
} // precitec

#endif /* NIOMODETYPE_H_20120322_INCLUDED */
