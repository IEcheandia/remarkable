/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2014
 * 	@brief		Fliplib filter 'LineFeature' in component 'Filter_LineGeometry'.
 * 	@detail		Compares the shape of the input line with a user-defined line template. 
 *				The form of the template consists of two connected straight line segments. Each segment is defined by length and angle. 
 *				The result is the mean square error between the input line and the template at each point of the line.
 */

#ifndef LINEFEATURE_H_20140411_INCLUDED
#define LINEFEATURE_H_20140411_INCLUDED


#include "fliplib/Fliplib.h"			///< fliplib image processing pipes and filter framework
#include "fliplib/TransformFilter.h"	///< base class
#include "fliplib/PipeEventArgs.h"		///< event processing
#include "fliplib/SynchronePipe.h"		///< in- / output

#include "geo/geo.h"					///< Size2d, VecDoublearray
#include "geo/array.h"					///< TArray

namespace precitec {
namespace filter {

/**
 *	@ingroup	Filter_LineGeometry
 * 	@detail		Compares the shape of the input line with a user-defined line template. 
 *				The form of the template consists of two connected straight line segments. Each segment is defined by length and angle. 
 *				The result is the mean square error between the input line and the template at each point of the line.
*/
class FILTER_API LineFeature  : public fliplib::TransformFilter {
public:

	/// CTOR
	LineFeature();

private:

	static const std::string m_oFilterName;			///< Name Filter
	static const std::string m_oPipeOutName;		///< Name Out-Pipe

	/// Set filter parameters as defined in database / xml file
	void setParameter();

	/// in pipe registrastion
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);

	/// in pipe event processing
	void proceed(const void* sender, fliplib::PipeEventArgs& e);

	/**
	 * @brief Compares the shape of the input line with a user-defined line template.
	 * @param p_rLineIn			Intput laser line.
	 * @param p_rFeature		Template which is compared with input line.
	 * @param p_oSegment1Length	Lenght of first feature segment.
	 * @param p_rLineOut		Template match (mean sqare error).
	 */
	static void calcTemplateMatch(const geo2d::Doublearray&	p_rLineIn, const std::vector<double>& p_rFeature, std::size_t p_oSegment1Length, geo2d::Doublearray& p_rLineOut);

	/// Paint overerlay primitives
	void paint();


	typedef fliplib::SynchronePipe<interface::GeoVecDoublearray>	line_pipe_t;

	const line_pipe_t*				m_pPipeInLine;				///< in pipe
	line_pipe_t						m_oPipeOutLine;				///< out pipe

	interface::SmpTrafo				m_oSpTrafo;					///< roi translation
	geo2d::VecDoublearray			m_oLinesOut;				///< output laser line(s)
	unsigned int					m_oSegment1Length;			///< parameter - length of first template segment
	unsigned int					m_oSegment2Length;			///< parameter - length of second template segment
	int								m_oSegment1Angle;			///< parameter - angle of first template segment
	int								m_oSegment2Angle;			///< parameter - angle of second template segment
	std::vector<double>				m_oFeature;					///< two connected straight line segments defined by length and angle
}; // LineFeature

} // namespace filter
} // namespace precitec 

#endif /*LINEFEATURE_H_20140411_INCLUDED*/



