/*!
 *  @copyright	Precitec Vision GmbH & Co. KG
 
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2011
 *  @file
 *  @brief			Fliplib filter 'Gradient' in component 'Filter_SeamSearch'. Gradient calculation on grey level profile.
 */

#ifndef GRADIENT_H_
#define GRADIENT_H_

#include "fliplib/Fliplib.h"			///< fliplib image processing pipes and filter framework
#include "fliplib/TransformFilter.h"	///< base class
#include "fliplib/PipeEventArgs.h"		///< event processing
#include "fliplib/SynchronePipe.h"		///< in- / output

#include "geo/geo.h"					///< Size2d, Intarray
#include "geo/array.h"					///< TArray
#include "filter/parameterEnums.h"		///< enum GradientType

namespace precitec {
	using interface::GeoVecDoublearray;
	using geo2d::VecDoublearray;
namespace filter {
	using fliplib::SynchronePipe;


///  Gradient filter.
/**
 * Takes N mean intensity profiles as input. For each profile a seam position (left and right row index) is calculated.
 * Each profile line is low pass filtered (mean average) with two different filter lengths.
 * On the filtered profiles the gradient is calculated. The gradient is the difference from 
 * two different mean intensities left and right of current position.
 */
class FILTER_API Gradient  : public fliplib::TransformFilter
{
public:
	Gradient();

	static const std::string m_oFilterName;			///< Name Filter
	static const std::string m_oPipeOutName1;		///< Name Out-Pipe
	static const std::string m_oPipeOutName2;		///< Name Out-Pipe
	static const std::string m_oPipeOutName3;		///< Name Out-Pipe

	//! Calulates the gradient on a 1d intensity line.
    /*!
      \param p_rProfileLpOffSeamIn	Input mean intensity of image slices extracted from filter 'IntensityProfile'. May be low-pass filtered at filter length 1.
      \param p_rProfileLpOnSeamIn	Input mean intensity of image slices extracted from filter 'IntensityProfile'. May be low-pass filtered at filter length 2.
	  \param p_oFilterRadiusOffSeam	Filter radius out of seam, region left and right of seam
	  \param p_oFilterRadiusOnSeam	Filter radius on seam 
	  \param p_oGradientType		Type of gradient. Absolute / dark seam (bright dark bright) / bright seam  (dark bright dark). See 'parameterEnums.h'.
	  \param p_rGradientLeftOut		Gradient for left seam [outer mean][point left][innermean] 
	  \param p_rGradientRightOut	Gradient for right seam [outer mean][point right][innermean] 
	  \return void
      \sa Filter 'IntensityProfile', 'Maximum', filter/parameterEnums.h.
    */
	// TODO make static as soon as no painting 
	/*static*/ void calcGradient(
	const VecDoublearray &p_rProfileLpOffSeamIn,
	const VecDoublearray &p_rProfileLpOnSeamIn,
	unsigned int	p_oFilterRadiusOffSeam,
	unsigned int	p_oFilterRadiusOnSeam,
	GradientType	p_oGradientType,
	VecDoublearray		&p_rGradientLeftOut,
	VecDoublearray		&p_rGradientRightOut
	);


	/// set filter parameter defined in database / xml file
	void setParameter();
	/// paint overerlay primitives
	void paint();

protected:
	/// in pipe registrastion
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	/// in pipe group event processing
	void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg);


private:
	//! (Re)initializes output member structures based on input structure dimension.
    /*!
      \param	p_rProfileIn	N image profiles
	  \return	void				
      \sa -
    */
	void reinitialize(const VecDoublearray &p_rProfileIn);

	typedef fliplib::SynchronePipe< interface::GeoVecDoublearray >	line_pipe_t;
	typedef fliplib::SynchronePipe< interface::GeoDoublearray >		scalar_pipe_t;


	const line_pipe_t		*m_pPipeInProfileLpOffSeam;	///< in pipe grey value profiles
	const line_pipe_t		*m_pPipeInProfileLpOnSeam;	///< in pipe grey value profiles
	line_pipe_t 			m_oPipeOutGradLeft;			///< out pipe left gradient
	line_pipe_t 			m_oPipeOutGradRight;		///< out pipe right gradient
	scalar_pipe_t  			m_oPipeOutMaxFLength;		///< out pipe WORKAROUND

	VecDoublearray			m_oGradientLeftOut;			///< out buffer for left search gradient
	VecDoublearray			m_oGradientRightOut;		///< out buffer for right search gradient

	int					m_oFilterRadiusOffSeam;			///< filter radius of (weak) subsampling filter that provides input line off seam
	int					m_oFilterRadiusOnSeam;			///< filter radius of (strong) subsampling filter that provides input line on seam
	GradientType		m_oGradientType;				///< Gradient Type

}; // Gradient


} // namespace filter
} // namespace precitec

#endif /*GRADIENT_H_*/



