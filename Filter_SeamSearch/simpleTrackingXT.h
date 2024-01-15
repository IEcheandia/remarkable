/*!
 *  @file			
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Claudius Batzlen (CB)
 *  @date			2019
 *  @brief			Fliplib filter 'SimpleTracking' in component 'Filter_SeamSearch'. Simple tracking based on extremum search and simple or gradient threshold.
 */

#ifndef SIMPLETRACKINGXT_H_20110920
#define SIMPLETRACKINGXT_H_20110920


#include "geo/geo.h"					///< Size2d, Intarray
#include "filter/parameterEnums.h"		///< enum ComparisonType

#include "fliplib/Fliplib.h"			///< fliplib image processing pipes and filter framework
#include "fliplib/TransformFilter.h"	///< base class
#include "fliplib/PipeEventArgs.h"		///< event processing
#include "fliplib/SynchronePipe.h"		///< in- / output


namespace precitec {
namespace filter {


///  SimpleTracking filter.
/**
 *
 */
class FILTER_API SimpleTrackingXT  : public fliplib::TransformFilter
{
public:
	SimpleTrackingXT();

	static const std::string m_oFilterName;			///< Name Filter
	static const std::string m_oPipeOutName1;		///< Name Out-Pipe
	static const std::string m_oPipeOutName2;		///< Name Out-Pipe

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
	void reinitialize(const geo2d::VecDoublearray	&p_rProfileIn);

	typedef fliplib::SynchronePipe< interface::GeoVecDoublearray >	line_pipe_t;
	typedef fliplib::SynchronePipe< interface::GeoDoublearray >		scalar_pipe_t;

	const line_pipe_t*			m_pPipeInProfile;		///< in pipe grey value profiles
	const scalar_pipe_t*		m_pPipeInPosition;		///< in pipe seam / gap position(s)
	const scalar_pipe_t*		m_pPipeInImgSize;		///< in pipe image size
	const scalar_pipe_t*		m_pPipeInThresholdL;	///< in pipe threshold left
	const scalar_pipe_t*		m_pPipeInThresholdR;	///< in pipe threshold right
	scalar_pipe_t 				m_oPipeOutPosL;			///< out pipe left simpleTracking
	scalar_pipe_t 				m_oPipeOutPosR;			///< out pipe right simpleTracking

	geo2d::Doublearray			m_oSeamPosL;			///< Output left seam / gap contour
	geo2d::Doublearray			m_oSeamPosR;			///< Output right seam / gap contour
	geo2d::Size					m_oImageSize;			///< image size

	ComparisonType				m_oComparisonType;		///< Searching for the maximum or minimum
	int							m_oThresholdL;			///< starting from the extrum expand to left side until threshold reached
	int							m_oThresholdR;			///< starting from the extrum expand to rifht side until threshold reached
}; // SimpleTracking


} // namespace filter
} // namespace precitec

#endif /*SIMPLETRACKINGXT_H_20110920*/



