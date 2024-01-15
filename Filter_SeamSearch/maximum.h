/*!
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2011
 *  @file
 *  @brief			Fliplib filter 'Maximum' in component 'Filter_SeamSearch'. Calculates right and left seam position.
 */

#ifndef MAXIMUM_20110621_H_
#define MAXIMUM_20110621_H_

#include "fliplib/Fliplib.h"			///< fliplib image processing pipes and filter framework
#include "fliplib/TransformFilter.h"	///< base class
#include "fliplib/PipeEventArgs.h"		///< event processing
#include "fliplib/SynchronePipe.h"		///< in- / output

#include "common/frame.h"				///< ImageFrame
#include "geo/geo.h"					///< GeoVecDoublearray
#include "geo/array.h"					///< TArray
#include "geo/size.h"					///< size

namespace precitec {
namespace filter {


///  Maximum filter.
/**
 * Fliplib filter 'Maximum' in component 'Filter_SeamSearch'. Calculates right and left seam position from gradients.
 */
class FILTER_API Maximum  : public fliplib::TransformFilter
{
public:
	Maximum();

	static const std::string m_oFilterName;			///< Name Filter
	static const std::string m_oPipeOutName1;		///< Name Out-Pipe
	static const std::string m_oPipeOutName2;		///< Name Out-Pipe
	static const std::string m_oPipeOutName3;		///< Name Out-Pipe

	//! Calculates right and left seam position from contour points.
    /*!
      \param p_rGradientLeftIn		left search gradient
      \param p_rGradientRightIn		right search gradient
      \param p_oMaxFilterLenght		max filter lenght for boundary lenght
      \param p_oImageHeight			image height
	  \param Output					left seam position.
	  \param Output					right seam position.
	  \return void
      \sa -
    */
	static void calcMaximum(
	const geo2d::VecDoublearray	&p_rGradientLeftIn,
	const geo2d::VecDoublearray	&p_rGradientRightIn,
	int							p_oMaxFilterLenght,
	int							p_oImageHeight,
	geo2d::Doublearray			&p_rContourLeftOut,
	geo2d::Doublearray			&p_rContourRightOut
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
	//! (Re)initializes output member structures based on input structure dimension.
    /*!
      \param	p_rGradientLeftIn	left search gradient
      \param	p_rGradientRightIn	right search gradient
	  \return	void				
      \sa -
    */
	void reinitialize(
		const geo2d::VecDoublearray		&p_rGradientLeftIn,
		const geo2d::VecDoublearray		&p_rGradientRightIn
	); 

	typedef fliplib::SynchronePipe< interface::GeoVecDoublearray >	line_pipe_t;
	typedef fliplib::SynchronePipe< interface::GeoDoublearray >		scalar_pipe_t;
	typedef fliplib::SynchronePipe< interface::ImageFrame >			imageFramePipe_t;

	const line_pipe_t*				m_pPipeInGradientLeft;		///< in pipe left search gradient
	const line_pipe_t*				m_pPipeInGradientRight;		///< in pipe right search gradient
	const scalar_pipe_t*			m_pPipeInMaxFLenght;		///< in pipe max filter length WORKAROUND
	const scalar_pipe_t*			m_pPipeInImgSize;			///< in pipe image siye
	scalar_pipe_t					m_oPipeOutContourLeft;		///< out pipe
	scalar_pipe_t					m_oPipeOutContourRight;		///< out pipe

	geo2d::Doublearray				m_oContourLeftOut;			///< output left seamposition
	geo2d::Doublearray				m_oContourRightOut;			///< output right seamposition

	int								m_oMaxFilterLenght;			///< max filter lenght for boundary lenght
	geo2d::Size						m_oImageSize;				///< image size

}; // Maximum

} // namespace filter
} // namespace precitec

#endif /*MAXIMUM_20110621_H_*/



