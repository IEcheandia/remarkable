/*!
 *  @copyright	Precitec Vision GmbH & Co. KG

 *  @author			Simon Hilsenbeck (HS), Stefan Birmanns (SB)
 *  @date			2011, 2016
 *  @file
 *  @brief			Fliplib filter 'IntensityProfileLine' in component 'Filter_SeamSearch'. Calculates a grey level profile around a laser line.
 */

#ifndef INTENSITYPROFILELINE_H_
#define INTENSITYPROFILELINE_H_

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
 * The mean intensity profile is calculated column-wise for an image. The starting y-position of each column is taken from a laser-line.
 */
class FILTER_API IntensityProfileLine  : public fliplib::TransformFilter
{
public:
	IntensityProfileLine();

	static const std::string m_oFilterName;			///< Name Filter
	static const std::string m_oPipeOut1Name;		///< Name Out-Pipe

    /*!
      \param p_rImageIn		Input image to be read.
      \param p_rLaserLineIn	The laser-line around which the profile is to be calculated.
	  \param p_oResX		Sampling rate in X direction - CURRENTLY NOT IMPLEMENTED
	  \param p_oResY		Sampling rate in Y direction
	  \param p_rProfileOut	Output intensity profile(s)
	  \return void
      \sa -
    */
	static void calcIntensityProfile(
		const BImage 			&p_rImageIn,
		const VecDoublearray 	&p_rLaserLineIn,
		unsigned int			p_oDistance,
		unsigned int			p_oThickness,
		unsigned int 			p_oResX,
		unsigned int 			p_oResY,
		VecDoublearray 			&p_rProfileOut
	);

	/// set filter parameter defined in database / xml file
	void setParameter();
	/// paint overerlay primitives
	void paint();

protected:
	/// in pipe registrastion
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	/// in pipe event processing
	void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg);

private:
	bool inputIsInvalid(const BImage &p_rImageIn);	///< check input integrity conditions
	void reinitialize(const BImage &p_rImageIn);	///< (re)initialization of output structure

	typedef fliplib::SynchronePipe< interface::ImageFrame >			image_pipe_t;
	typedef fliplib::SynchronePipe< interface::GeoVecDoublearray >	line_pipe_t;
	typedef fliplib::SynchronePipe< interface::GeoDoublearray >		scalar_pipe_t;

	const image_pipe_t*	m_pPipeInImageFrame;	///< in pipe
	const line_pipe_t*	m_pPipeInLaserLine;		///< in pipe
	line_pipe_t			m_oPipeOutProfile;		///< out pipe
	interface::SmpTrafo	m_oSpTrafo;				///< roi translation

	const ImageFrame	*m_pFrameIn;			///< input frame

	VecDoublearray		m_oLineIn;				///< input laser line, necessary only for the paint routine
	VecDoublearray		m_oProfileOut;			///< output array, holds the intensity profile

	int					m_oDistance;			///< Distance from the laser-line, after which the profile is supposed to be calculated.
	int					m_oThickness;			///< Thickness of the area (pixel), from which the profile is supposed to be calculated.
	int					m_oResX;				///< Sampling rate in X direction
	int					m_oResY;				///< Sampling rate in Y direction

}; // IntensityProfileLine

} // namespace filter
} // namespace precitec

#endif /*INTENSITYPROFILELINE_H_*/



