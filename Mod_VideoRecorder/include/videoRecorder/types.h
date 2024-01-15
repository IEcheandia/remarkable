
/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief		Video recorder related types and definitions.
 */

#ifndef TYPES_H_20121108_INCLUDE
#define TYPES_H_20121108_INCLUDE

// local includes
#include "common/seamData.h"
#include "image/image.h"
#include "image/ipSignal.h"
// stdlib includes
#include <tuple>
#include <queue>

namespace precitec {
namespace vdr {


/**
*	@brief		Video recorder image type. Contains image data, image number and seam data.
*/
namespace vdr_image_type { // prevent name conflict, enum class not usable with std::get
enum VdrImageType {
	eImage  		= 0,		///< image
	eImageNumber, 				///< trigger number (image number)
	eSeamData,					///< seam data
	eHwRoiX,					///< hw roi x coordinate
	eHwRoiY,					///< hw roi y coordinate
	eImageId					///< id of the image (global framegrabber counter)
}; // VdrImgageType
} // namespace vdrimagtype

namespace vdr_sample_type { // prevent name conflict, enum class not usable with std::get
	enum VdrSampleType {
		eSample  		= 0,		///< sample
		eTriggerNb, 				///< trigger number
		eSeamData,					///< seam data
		eSensorId					///< sensor id
	}; // VdrSampleType
} // namespace vdr_sample_type

typedef std::tuple<image::BImage, int, interface::SeamData, int, int, uint32_t>		vdrImage_t;
typedef std::queue<vdrImage_t>												vdrImgage_queue_t;
typedef std::tuple<image::Sample, int, interface::SeamData, int>					vdrSample_t;
typedef std::queue<vdrSample_t>												vdrSample_queue_t;


/**
*	@brief		Add data type. Values serve as indices for an byte array of additional data stored in bmp.
*/
namespace add_data_indices {
enum AddDataIndices{
	eVersion	= 0,	///< unsigned short -> 2 bytes
	eHwRoiX		= 2,	///< unsigned short -> 2 bytes
	eHwRoiY		= 4,	///< unsigned short -> 2 bytes
	eImgNb		= 6,	///< unsigned short -> 2 bytes
	eNbBytes	= 8		///< total size in bytes
}; // AddDataIndices
} // namespace add_data_indices

/**
*	@brief		Aggregates counters. Counters are reset after each seam / recording cycle.
*/
class Counters {
public:
	Counters () :
		m_oNbImagesInserted			( 0 ),
		m_oNbImagesRecorded			( 0 ),
		m_oNbImagesMissed			( 0 ),
		m_oNbImageWritesFailed		( 0 ),
		m_oNbSamplesInserted		( 0 ),
		m_oNbSamplesRecorded		( 0 ),
		m_oNbSamplesMissed			( 0 ),
		m_oNbSampleWritesFailed	( 0 )
	{} // Counters

	unsigned int			m_oNbImagesInserted;		///< Number of images inserted in write queue
	unsigned int			m_oNbImagesRecorded;		///< Number of images recorded
	unsigned int			m_oNbImagesMissed;			///< Number of images missed
	unsigned int			m_oNbImageWritesFailed;		///< Number of save-bmp-calls that failed
	unsigned int			m_oNbSamplesInserted;		///< Number of samples inserted in write queue
	unsigned int			m_oNbSamplesRecorded;		///< Number of samples recorded
	unsigned int			m_oNbSamplesMissed;			///< Number of samples missed
	unsigned int			m_oNbSampleWritesFailed;	///< Number of save-bmp-calls that failed
}; // Counters

/**
 * @brief 	 Return value and parameters of method requestProductSequences. Needed for thread adapter.
 */
struct RequestSequenceArgs {
	RequestSequenceArgs() : m_oNbFilesTransferred(0) {}
	Poco::UUID								m_ProductInstID;
	unsigned int							m_oNbFilesTransferred;
	unsigned int							m_oSeamseries;
	unsigned int							m_oSeam;

}; // RequestSequenceArgs

/**
 * @brief 	 Return value and parameters of method getNbRawDataFilesAvailableThread. Needed for thread adapter.
 */
struct GetNbRawDataFilesAvailableArgs {
	GetNbRawDataFilesAvailableArgs() : m_oNbRawDataFilesAvailable(0) {}
	Poco::UUID		m_oProductId;
	Poco::UUID		m_oProductInstId;
	unsigned int	m_oNbRawDataFilesAvailable;
	unsigned int	m_oSeamseries;
	unsigned int	m_oSeam;
}; // GetNbRawDataFilesAvailableArgs

} // namespace vdr
} // namespace precitec

#endif // TYPES_H_20121108_INCLUDE
