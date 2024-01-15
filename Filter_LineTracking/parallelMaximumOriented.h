/*!
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			LB
 *  @date			2018
 *  @file
 */

#ifndef PARALLELMAXIMUMOriented_H_
#define PARALLELMAXIMUMOriented_H_

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


/*  ParallelMaximumOriented filter. Provides the same functionality as the filters 
parallelmaximum and linefit, with the option to handle quasi vertical lines
*/


/**
 * Line tracking.
 */
class FILTER_API ParallelMaximumOriented  : public fliplib::TransformFilter
{
public:
    ParallelMaximumOriented();

    static const std::string m_oFilterName;			///< Name Filter
    static const std::string m_oPipeOutNameLine;		///< Name Out-Pipe
    static const std::string m_oPipeOutNameLineModel;	///< Name Out-Pipe

    //! Parallel tracking. Used by filter FineTracking as well.
    /*!
      \param p_rImageIn         Input image to be read.
      \param p_oResX            Sampling rate in X direction
      \param p_oResY            Sampling rate in Y direction
      \param p_oThreshold       Tracking threshold for gray value.
      \param p_oMaxLineWidth    Maximal line width of laser line. Prevents big jumps of tracked line position in Y
      \param p_oMainDirectionHorizontal    True if the main direction of the laser line is horizontal
      \param p_rLineOut         Output laser line
      \param p_oOffset          Offset for processing out laserline.
      \param p_oOffset          Size for processing out laserline
    */
    static void parMax(
        const image::BImage		&p_rImageIn, 
        std::uint32_t			p_oResX, 
        std::uint32_t			p_oResY, 
        std::uint32_t			p_oThreshold, 
        std::uint32_t			p_oMaxLineWidth,
        bool p_oMainDirectionHorizontal,
        geo2d::VecDoublearray	&p_rLineOut,
        geo2d::Point			p_oOffset,
        geo2d::Size				p_oSize
    ); 

    /**
      \param p_rImageIn		Input image to be read.
      \param p_rLaserline	Laser line vector (inplace processing). Contains y values at a certain x sampling rate.
      \param p_oThreshold	Tracking threshold for gray value.
      \param p_oDilation	Subroi dilation in the direction orthogonal to the laser line
      \return std::vector<geo2d::Rect> list of subRois
    */
    static std::vector<geo2d::Rect> fineTrack(
        const image::BImage			&p_rImageIn,
        geo2d::VecDoublearray			&p_rLaserline,
        std::uint32_t p_oDilation,
        std::uint32_t				p_oThreshold,
        std::uint32_t p_oMaxLineWidth,
        std::uint32_t p_oResolution,
        bool p_oHorizontal,
        bool p_returnPaintInfo);

    //fit a line to the detected laser line points
    static geo2d::LineModel fitLine(geo2d::Doublearray	&p_rLaserline, bool p_oMainDirectionHorizontal);

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
    typedef fliplib::SynchronePipe<interface::GeoLineModelarray>	lineEquation_pipe_t;
    typedef std::unique_ptr< image_pipe_t >						up_image_pipe_t;

    const image_pipe_t*				m_pPipeInImageFrame;					///< in pipe
    line_pipe_t 					m_LinePipeOut;							///< out pipe
    lineEquation_pipe_t				m_LineEquationPipeOut;	///< outpipe 

    interface::SmpTrafo				m_oSpTrafo;								///< roi translation
    geo2d::VecDoublearray				m_oLineOut;								///< output laser line
    geo2d::VecDoublearray			m_oLineOutTransposed;					///< container for vertcal laser line (used internally)
    geo2d::LineModelarray			m_oLineModelOut;						///< output fitted line

    std::vector<geo2d::Rect>		m_oPatchRois;							///< contains all path rois from finetracking, just for painting

    std::uint32_t					m_oResX;								///< Sampling rate in X direction
    std::uint32_t					m_oResY;								///< Sampling rate in Y direction
    std::uint32_t					m_oThreshold;							///< threshold gray value
    std::uint32_t					m_oFineTrackingDilation;				///< Subroi dilation in Y direction. 
    bool							m_oIsFineTracking;						///< If subsampled line is filled by fine tracking
    std::uint32_t					m_oMaxLineWidth;						///< Maximal line width of laser line. Prevents big line width changes. 
    bool							m_oMainDirectionHorizontal;						///< Main direction (horizontal or vertical)
    bool							m_oHandleTransposeInContext;
    bool							m_oPaint;                               ///< Do NOT paint in case of error in preceeding filters
    image::BImage m_oPaintTransposedImage;

}; // ParallelMaximumOriented

} // namespace filter
} // namespace precitec

#endif /*PARALLELMAXIMUMOriented_H_*/



