/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		HS
 * 	@date		2013
 * 	@brief		Filter which computes the principal components of a pore.
 */

#ifndef PRINCIPALCOMPONENTS_INCLUDED_20130218_H_
#define PRINCIPALCOMPONENTS_INCLUDED_20130218_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include "common/frame.h"
#include "geo/geo.h"
#include "geo/array.h"

#include "poreStatistics.h"


namespace precitec {
namespace filter {

/**
 * @brief Filter which computes the principal components of a pore.
 */
class FILTER_API PrincipalComponents : public fliplib::TransformFilter
{
public:

	/**
	 * @brief CTor.
	 */
	PrincipalComponents();

	/**
	 * @brief Set filter parameters.
	 */
	void setParameter();

	/**
	 * @brief Paint overlay output.
	 */
	void paint();


	// Declare constants

	static const std::string m_oFilterName;					///< Filter name.
	static const std::string m_oPipeOutLength1Name;			///< Pipe name: out-pipe.
	static const std::string m_oPipeOutLength2Name;			///< Pipe name: out-pipe.
	static const std::string m_oPipeOutAxesRatioName;		///< Pipe name: out-pipe.


protected:

	/**
	 * @brief In-pipe registration.
	 * @param p_rPipe Reference to pipe that is getting connected to the filter.
	 * @param p_oGroup Group number (0 - proceed is called whenever a pipe has a data element, 1 - proceed is only called when all pipes have a data element).
	 */
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);

	/**
	 * @brief Processing routine.
	 * @param p_pSender pointer to
	 * @param p_rEvent
	 */
	void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs&);

private:

	typedef fliplib::SynchronePipe<interface::ImageFrame>		image_pipe_t;
	typedef fliplib::SynchronePipe<interface::GeoBlobarray>		blob_pipe_t;
	typedef fliplib::SynchronePipe<interface::GeoDoublearray>	scalar_pipe_t;

	const image_pipe_t*			m_pPipeInImageFrame;	///< in pipe
	const blob_pipe_t*			m_pPipeInBlob;			///< in-pipe.
	scalar_pipe_t 				m_oPipeOutLength1;		///< out-pipe.
	scalar_pipe_t 				m_oPipeOutLength2;		///< out-pipe.
	scalar_pipe_t 				m_oPipeOutAxesRatio;    ///< out-pipe.

    interface::GeoBlobarray     m_oInputBlob;           ///< result received from in-pipe
	interface::SmpTrafo			m_oSpTrafo;				///< roi translation

	geo2d::Doublearray			m_oArrayLength1;		///< length of major axis 1
	geo2d::Doublearray			m_oArrayLength2;		///< length of major axis 2
	geo2d::Doublearray			m_oArrayAxesRatio;		///< ratio of axis 1 length and axis 2 length

	PoreStatistics				m_oPoreStatistics;
	std::vector<MajorAxes>		m_oMajorAxes;
}; // class PrincipalComponents

} // namespace filter
} // namespace precitec

#endif /* PRINCIPALCOMPONENTS_INCLUDED_20130218_H_ */
