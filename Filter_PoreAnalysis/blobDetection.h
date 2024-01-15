/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIH, HS
 * 	@date		2012
 * 	@brief		Filter that extracts blobs of a binary image by means of a labeling algorithm.
 */

#ifndef BLOBDETECTION_H_
#define BLOBDETECTION_H_

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"

#include "segmentateImage.h"
#include "common/frame.h"				// ImageFrame
#include "geo/geo.h"

namespace precitec {
namespace filter {

/**
 * @brief Filter that extracts blobs of a binary image by means of a labeling algorithm.
 */
class FILTER_API BlobDetection  : public fliplib::TransformFilter
{
public:
	BlobDetection();
	~BlobDetection();

    void setParameter();

private:

	static const std::string FILTERNAME;
	static const std::string PIPENAME0;
	static const std::string PIPENAME1;
	static const std::string PIPENAME2;

	void paint();
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	void proceed(const void* sender, fliplib::PipeEventArgs& e);

	typedef fliplib::SynchronePipe<interface::ImageFrame>		pipe_image_t;
	typedef fliplib::SynchronePipe<interface::GeoBlobarray>		pipe_blob_t;
	typedef fliplib::SynchronePipe<interface::GeoDoublearray>	scalar_pipe_t;

	const pipe_image_t*			m_pPipeInImageFrame;	///< in pipe
	pipe_blob_t					m_oPipeOutBlob;			///< out pipe
	scalar_pipe_t 				m_oPipeOutPosX;			///< out-pipe.
	scalar_pipe_t 				m_oPipeOutPosY;			///< out-pipe.
	interface::SmpTrafo			m_oSpTrafo;				///< roi translation

	unsigned int				m_oMaxNbBlobs;			///< parameter
	unsigned int				m_oMinBlobSize;			///< parameter
	geo2d::DataBlobDetectionT	m_oDataBlobDetection;
	geo2d::Blobarray			m_oBlobArray;
	geo2d::Doublearray			m_oArrayPosX;			///< x position of pore
	geo2d::Doublearray			m_oArrayPosY;			///< y position of pore
};

} // namespace filter
} // namespace precitec

#endif /*BLOBDETECTION_H_*/
