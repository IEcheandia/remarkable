/**
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2011
 *  @file
 *  @brief			Fliplib filter 'SelectSeamPos' in component 'Filter_SeamSearch'. Checks seam positions and selects final seam position.
 */

#ifndef SELECTSEAMPOS_20110727_H_
#define SELECTSEAMPOS_20110727_H_

#include "fliplib/Fliplib.h"			///< fliplib image processing pipes and filter framework
#include "fliplib/TransformFilter.h"	///< base class
#include "fliplib/PipeEventArgs.h"		///< event processing
#include "fliplib/SynchronePipe.h"		///< in- / output

#include "geo/geo.h"					///< GeoDoublearray
#include "geo/array.h"					///< TArray

namespace precitec {
namespace filter {

///  SelectSeamPos filter.
/**
 * Fliplib filter 'SelectSeamPos' in component 'Filter_SeamSearch'. Checks seam positions and selects final seam position.
 */
class FILTER_API SelectSeamPos  : public fliplib::TransformFilter
{
public:
	SelectSeamPos();

	static const std::string m_oFilterName;			///< Name Filter
	static const std::string m_oPipeOutName1;		///< Name Out-Pipe
	static const std::string m_oPipeOutName2;		///< Name Out-Pipe

    /**
      *  @brief	Calculates right and left seam position from contour points.
	  *  @param p_rContourLIn		left search gradient
      *  @param p_rContourRIn		right search gradient
      *  @param p_oSeamPosL			output left seam position
      *  @param p_oSeamPosR			output right seam position
	  *  @return void
      *  @sa -
    */
	static void calcSelectSeamPos(
		const geo2d::Doublearray						&p_rContourLIn,
		const geo2d::Doublearray						&p_rContourRIn,
		geo2d::Doublearray							&p_oSeamPosL,
		geo2d::Doublearray							&p_oSeamPosR
	);


	/// set filter parameter defined in database / xml file
	void setParameter();
	/// paint overerlay primitives
	void paint();


protected:
	/// arm filter
	/*virtual*/ 
	void arm (const fliplib::ArmStateBase& state);
	/// in pipe registrastion
	/*virtual*/ 
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	/// in pipe event processing
	/*virtual*/ 
	void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg);


private:

    /**
      *  @brief	(Re)initializes output member structures based on input structure dimension.
	  *  @return	void				
      *  @sa -
    */
	void reinitialize(); 

	typedef fliplib::SynchronePipe< interface::GeoDoublearray >		scalar_pipe_t;

	const scalar_pipe_t*	m_pPipeInContourL;		///< in pipe left contour points
	const scalar_pipe_t*	m_pPipeInContourR;		///< in pipe right contour points
	scalar_pipe_t			m_oPipeOutSeamPosL;		///< out pipe
	scalar_pipe_t			m_oPipeOutSeamPosR;		///< out pipe

	geo2d::Doublearray		m_oSeamPosL;			///< output seam position left
	geo2d::Doublearray		m_oSeamPosR;			///< output seam position right

}; // SelectSeamPos

} // namespace filter
} // namespace precitec

#endif /*CORRECTWIDTH_20110727_H_*/



