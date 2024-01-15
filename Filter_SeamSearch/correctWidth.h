/*!
 *  @copyright	Precitec Vision GmbH & Co. KG
 
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2011
 *  @file
 *  @brief			Fliplib filter 'CorrectWidth' in component 'Filter_SeamSearch'. Eliminates seam width outliers in contour points.
 */

#ifndef CORRECTWIDTH_20110727_H_
#define CORRECTWIDTH_20110727_H_

#include "fliplib/Fliplib.h"			///< fliplib image processing pipes and filter framework
#include "fliplib/TransformFilter.h"	///< base class
#include "fliplib/PipeEventArgs.h"		///< event processing
#include "fliplib/SynchronePipe.h"		///< in- / output

#include "geo/geo.h"					///< GeoDoublearray
#include "geo/array.h"					///< TArray
#include "geo/size.h"					///< size

namespace precitec {
	using interface::GeoDoublearray;
	using geo2d::Doublearray;
namespace filter {
	using fliplib::SynchronePipe;


///  CorrectWidth filter.
/**
 * Fliplib filter 'CorrectWidth' in component 'Filter_SeamSearch'. Eliminates seam width outliers in contour points by comparison with default seam width.
 */
class FILTER_API CorrectWidth  : public fliplib::TransformFilter
{
public:
	CorrectWidth();

	static const std::string m_oFilterName;			///< Name Filter
	static const std::string m_oPipeOutName1;		///< Name Out-Pipe
	static const std::string m_oPipeOutName3;		///< Name Out-Pipe
	static const std::string m_oPipeOutName2;		///< Name Out-Pipe

	//! Calculates right and left seam position from contour points.
    /*!
      \param p_rContourLIn			left search gradient
      \param p_rContourRIn			right search gradient
      \param p_oDefaultSeamWidth	default seam width
      \param p_oMaxDiff				max difference to default seam width
	  \param p_rContourLOut			left seam position.
	  \param p_rContourROut			right seam position.
	  \return void
      \sa -
    */
	void calcCorrectWidth(
	const Doublearray		&p_rContourLIn,
	const Doublearray		&p_rContourRIn,
	int						p_oDefaultSeamWidth,
	int						p_oMaxDiff,
	Doublearray				&p_rContourLOut,
	Doublearray				&p_rContourROut
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
      \param	p_rContourLIn	left search gradient
      \param	p_rContourRIn	right search gradient
	  \return	void				
      \sa -
    */
	void reinitialize(
		const Doublearray			&p_rContourLIn,
		const Doublearray			&p_rContourRIn
	); 

	typedef fliplib::SynchronePipe< interface::GeoVecDoublearray >	line_pipe_t;
	typedef fliplib::SynchronePipe< interface::GeoDoublearray >		scalar_pipe_t;

	const scalar_pipe_t*	m_pPipeInContourL;	///< in pipe left contour points
	const scalar_pipe_t*	m_pPipeInContourR;	///< in pipe right contour points
	const scalar_pipe_t*	m_pPipeInImgSize;	///< in pipe img size
	scalar_pipe_t			m_oPipeOutContourL;	///< out pipe
	scalar_pipe_t			m_oPipeOutContourR;	///< out pipe

	Doublearray				m_oContourLOut;					///< output left contour points
	Doublearray				m_oContourROut;					///< output right contour points
	geo2d::Size				m_oImageSize;					///< image size

	int						m_oDefaultSeamWidth;			///< default seam width
	int						m_oMaxDiff;						///< max difference to default seam width

}; // CorrectWidth

} // namespace filter
} // namespace precitec

#endif /*CORRECTWIDTH_20110727_H_*/



