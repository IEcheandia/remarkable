/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		  Andreas Beschorner (BA)
 *  @date		    2012
 *  @brief	    Gets baseline.
 */

#ifndef GetRunBaseline_H_
#define GetRunBaseline_H_

#include <vector>

#include <fliplib/Fliplib.h>			    ///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include <image/image.h>				      ///< BImage
#include "filter/parameterEnums.h"		///< enum ExtremumType

#include <common/frame.h>				      ///< ImageFrame
#include <geo/geo.h>					        ///< Size2d, VecDoublearray
#include <geo/array.h>					      ///< ByteArray


namespace precitec {
namespace filter {

/**
 * @ingroup Filter_LineGeometry
 * @brief Gets baseline.
 * 
 * @param m_pPipeInXLeft   X coordinate of left point
 * @param m_pPipeInYLeft   Value of left point
 * @param m_pPipeInXRight  X coordinate of right point
 * @param m_pPipeInYRight  Value of right point
 *
 * @param[out] m_oPipeOutSlope        Slope of resulting line (segment)
 * @param[out] m_oPipeOutIntercept    Intercept of resulting line (segment)
 * @param[out] m_oPipeOutXStart       X coord of left point/ start of line segment
 * @param[out] m_oPipeOutLinesegment  Array of y coord of the linesegment. In combination with m_oPipeOutXStart one has all it needs for the segment.
 *
 * Given two points /f$p_1, p_2/f$ in pixel coordinates this filter returns the straight linesegment joining /f$p_1, p_2/f$ as well as its
 * slope and intercept in pixel coordinates.
 */
class FILTER_API GetRunBaseline  : public fliplib::TransformFilter
{
public:

	/// CTor.
	GetRunBaseline();

	static const std::string m_oFilterName;		     ///< Name Filter
	static const std::string PIPENAME_SLOPE;		   ///< Name Out Slope
	static const std::string PIPENAME_INTERCEPT;	 ///< Name Out Intercept
	static const std::string PIPENAME_LINESEGMENT; ///< Name Out Linesegment
	static const std::string PIPENAME_XSTART;      ///< Name Out Intercept
    static const std::string PIPENAME_LINE_OUT; ///< Name Out-Pipe

	/// Set filter parameters defined in database / xml file
	void setParameter();

	/// paints overlay
	void paint();

	/**
	 * @brief Given two points, computes linesegment and line parameters (slope and intercept) w.r.t. image pixel representation.
	 *
	 * .
	 * @return void				
	 */

protected:
	/// in pipe registration
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	/// in pipe event processing
	void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg);
	bool computeLinesegment(double p_oXLeft, double p_oYLeft, double p_oXRight, double p_oYRight);
	void signalSend(const interface::ImageContext &p_rImgContext, const bool p_oIO);

	// pipes
	const fliplib::SynchronePipe< interface::GeoDoublearray >  *m_pPipeInXLeft;	///< In pipe xcoord left point
	const fliplib::SynchronePipe< interface::GeoDoublearray >  *m_pPipeInYLeft;	///< In pipe ycoord left point
	const fliplib::SynchronePipe< interface::GeoDoublearray >  *m_pPipeInXRight;	///< In pipe xcoord right point
	const fliplib::SynchronePipe< interface::GeoDoublearray >  *m_pPipeInYRight;	///< In pipe ycoord right point

	fliplib::SynchronePipe< interface::GeoDoublearray >       	m_oPipeOutSlope;	///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >       	m_oPipeOutIntercept;	///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >         m_oPipeOutXStart;	    ///< Out pipe
	fliplib::SynchronePipe< interface::GeoVecDoublearray > 	      m_oPipeOutLinesegment;
    fliplib::SynchronePipe<interface::GeoLineModelarray> m_oPipeOutLineEquation;	///< Data out-pipe.

	
	interface::SmpTrafo m_oSpTrafo; ///< roi translation

	double m_oSlope;
	double m_oIntercept;
	double m_oXStart;
	geo2d::VecDoublearray m_oLinesegment;

	bool m_oPaint;
};

} // namespace precitec
} // namespace filter

#endif /* GetRunBaseline_H_ */
