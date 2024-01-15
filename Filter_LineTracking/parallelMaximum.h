/*!
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2010-2011
 *  @file
 */

#ifndef PARALLELMAXIMUM_H_
#define PARALLELMAXIMUM_H_

#include <memory>						///< unique_ptr
#include <cstdint>						///< uint32

#include "fliplib/Fliplib.h"			///< fliplib image processing pipes and filter framework
#include "fliplib/TransformFilter.h"	///< base class
#include "fliplib/PipeEventArgs.h"		///< event processing
#include "fliplib/SynchronePipe.h"		///< in- / output

#include "common/frame.h"				///< ImageFrame
#include "image/image.h"				///< BImage
#include "geo/geo.h"					///< Size2d, VecDoublearray
#include "geo/array.h"					///< TArray

namespace precitec {
namespace filter {


///  ParallelMaximum filter.
/**
 * Line tracking.
 */
class FILTER_API ParallelMaximum  : public fliplib::TransformFilter
{
public:
	ParallelMaximum();

	static const std::string m_oFilterName;			///< Name Filter
	static const std::string m_oPipeOutName1;		///< Name Out-Pipe

	//! Parallel tracking. Used by filter FineTracking as well.
    /*!
      \param p_rImageIn         Input image to be read.
	  \param p_oResX            Sampling rate in X direction
	  \param p_oResY            Sampling rate in Y direction
	  \param p_oThreshold       Tracking threshold for gray value.
      \param p_oMaxLineWidth    Maximal line width of laser line. Prevents big jumps of tracked line position in Y
	  \param p_rLineOut         Output laser line
	  \param p_oOffset          Offset for writing in out laserline.-
    */
	static void parMax(
		const image::BImage		&p_rImageIn, 
		std::uint32_t			p_oResX, 
		std::uint32_t			p_oResY, 
		std::uint32_t			p_oThreshold, 
        std::uint32_t			p_oMaxLineWidth,
		geo2d::VecDoublearray	&p_rLineOut,
		std::uint32_t			p_oOffset = 0
	); 

	/**
      \param p_rImageIn		Input image to be read.
	  \param p_rLaserline	Laser line vector (inplace processing). Contains y values at a certain x sampling rate.
	  \param p_oThreshold	Tracking threshold for gray value.
	  \param p_oDilationY	Subroi dilation in Y direction.
      \return void
    */
	void fineTrack(
		const image::BImage			&p_rImageIn,
		geo2d::VecDoublearray			&p_rLaserline,
		std::uint32_t				p_oThreshold,
		std::uint32_t				p_oDilationY
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
	typedef fliplib::SynchronePipe< interface::ImageFrame >		image_pipe_t;
	typedef fliplib::SynchronePipe< interface::GeoVecDoublearray >	line_pipe_t;
	typedef std::unique_ptr< image_pipe_t >						up_image_pipe_t;

	const image_pipe_t*				m_pPipeInImageFrame;					///< in pipe
	line_pipe_t 					m_LinePipeOut;							///< out pipe

	interface::SmpTrafo				m_oSpTrafo;								///< roi translation
	geo2d::VecDoublearray				m_oLineOut;								///< output laser line

	std::vector<geo2d::Rect>		m_oPatchRois;							///< contains all path rois from finetracking, just for painting

	std::uint32_t					m_oResX;								///< Sampling rate in X direction
	std::uint32_t					m_oResY;								///< Sampling rate in Y direction
	std::uint32_t					m_oThreshold;							///< threshold gray value
	std::uint32_t					m_oDilationY;							///< Subroi dilation in Y direction. 
	bool							m_oIsFineTracking;						///< If subsampled line is filled by fine tracking
    std::uint32_t					m_oMaxLineWidth;						///< Maximal line width of laser line. Prevents big line width changes. 
	bool                            m_oPaint;                               ///< Do NOT paint in case of error in preceeding filters

}; // ParallelMaximum

} // namespace filter
} // namespace precitec

#endif /*PARALLELMAXIMUM_H_*/



