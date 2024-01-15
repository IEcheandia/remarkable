/*!
 *  @copyright	Precitec Vision GmbH & Co. KG
 
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2011
 *  @file
 *  @brief			Fliplib filter 'IntensityProfile' in component 'Filter_SeamSearch'. Calculates grey level profile on image.
 */

#ifndef INTENSITYPROFILE_H_
#define INTENSITYPROFILE_H_

#include "fliplib/Fliplib.h"			///< fliplib image processing pipes and filter framework
#include "fliplib/TransformFilter.h"	///< base class
#include "fliplib/PipeEventArgs.h"		///< event processing
#include "fliplib/SynchronePipe.h"		///< in- / output

#include "common/frame.h"				///< ImageFrame
#include "image/image.h"				///< BImage
#include "geo/geo.h"					///< Size2d, Intarray
#include "geo/array.h"					///< TArray

namespace precitec {
	using image::BImage;
	using interface::ImageFrame;
	using interface::Size2D;
	using interface::GeoVecDoublearray;
	using geo2d::Doublearray;
	using geo2d::VecDoublearray;
namespace filter {
	using fliplib::SynchronePipe;


///  IntensityProfile filter.
/**
 * The image is divided into N horicontal slices. For each slice its mean intensity profile is calculated column-wise. 
 * The resulting row contains the mean intensity profile. 
 * Finally, the N profile rows are storded in one big row vector of the length N * image width.
 * For connectivity, the first intensity slice is additionally signaled on an outpipe of type 'Line' (TArray).
 */
class FILTER_API IntensityProfile  : public fliplib::TransformFilter
{
public:
	IntensityProfile();

	static const std::string m_oFilterName;			///< Name Filter
	static const std::string m_oPipeOut1Name;		///< Name Out-Pipe
	static const std::string m_oPipeOut2Name;		///< Name Out-Pipe

	//! The image is divided into N horicontal slices. For each slice its mean intensity profile is calculated column-wise.
    /*!
      \param p_rImageIn		Input image to be read.
	  \param p_oResX		Sampling rate in X direction - CURRENTLY NOT IMPLEMENTED
	  \param p_oResY		Sampling rate in Y direction
	  \param p_oNSlices		number of horizotal slices the image is to be divided
	  \param p_rProfileOut	Output intensity profile(s)
	  \return void
      \sa -
    */
	static int calcIntensityProfile(
		const BImage &p_rImageIn,
		unsigned int p_oResX,
		unsigned int p_oResY,
		unsigned int p_oNSlices,
		std::vector< Doublearray > &p_rProfileOut
	);


	/// set filter parameter defined in database / xml file
	void setParameter();
	/// paint overerlay primitives
	void paint();

protected:
	/// in pipe registrastion
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	/// in pipe event processing
	void proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rEventArg);


private:
	bool inputIsInvalid(const BImage &p_rImageIn);	///< check input integrity conditions
	void reinitialize(const BImage &p_rImageIn);	///< (re)initialization of output structure

	typedef fliplib::SynchronePipe< interface::ImageFrame >			image_pipe_t;
	typedef fliplib::SynchronePipe< interface::GeoVecDoublearray >	line_pipe_t;
	typedef fliplib::SynchronePipe< interface::GeoDoublearray >		scalar_pipe_t;

	const image_pipe_t*	m_pPipeInImageFrame;	///< in pipe
	line_pipe_t			m_oPipeOutProfile;		///< out pipe
	scalar_pipe_t 		m_oPipeOutImgSize;		///< out pipe WORKAROUND

	VecDoublearray		m_oProfileOut;			///< output array, holds m_oNSlices GeoIntarrays to contain profiles

	int					m_oResX;				///< Sampling rate in X direction
	int					m_oResY;				///< Sampling rate in Y direction
	int					m_oNSlices;				///< number of horizotal slices the image is to be divided

}; // IntensityProfile

} // namespace filter
} // namespace precitec

#endif /*INTENSITYPROFILE_H_*/



