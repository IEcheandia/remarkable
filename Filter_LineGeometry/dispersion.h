/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		  Andreas Beschorner (BA)
 *  @date		    2012
 *  @brief	    Computes local Dispersions from gradient and averaged/ smoothed gradient
 */

#ifndef DISPERSION_H_
#define DISPERSION_H_

#include <vector>

#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include <image/image.h>				///< BImage
#include <common/frame.h>				///< ImageFrame
#include <geo/geo.h>					///< Size2d, VecDoublearray
#include <geo/array.h>					///< ByteArray


namespace precitec {
namespace filter {

/**
 * @ingroup Filter_LineGeometry
 * @brief Gets baseline.
 * Given 3 points, gradMin, gradMax and extremum, this filter returns a baseline from gradMin to gradMax,
 * if the x-coord of the extremum is between the two points. Returns slope and intercep.
 */
class FILTER_API Dispersion : public fliplib::TransformFilter
{
public:

	/// CTor and DTor.
	Dispersion();
	~Dispersion();

	static const std::string m_oFilterName;		       ///< Name Filter
	static const std::string PIPENAME_AboveAvgLeft;	 ///< Name Out Above Avg. left
	static const std::string PIPENAME_BelowAvgLeft;	 ///< Name Out Below Avg. left
	static const std::string PIPENAME_AboveAvgRight; ///< Name Out Above Avg. right
	static const std::string PIPENAME_BelowAvgRight; ///< Name Out Below Avg. left
	static const std::string PIPENAME_Trend;         ///< Name Out Below Avg. left

	/// Set filter parameters defined in database / xml file
	void setParameter();

	/// paints overlay
	void paint();

	/**
	 * @brief Calculates / find an extremum of a line.
	 *
	 * @param p_rLineIn    		(Laser)-Line input object.
	 * @param p_oExtremumType	Shall the function search for the maximum (p_oExtremumType = eMaximum) or minimum (p_oExtremumType = eMinimum).
	 *
	 * @param p_oPosition		Index of extremum found
	 * @return void				
	 */
	 
protected:
	/// in pipe registration
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	/// in pipe event processing
	void proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rEventArg);
	void computeGradient( const geo2d::VecDoublearray &p_rLine, std::vector<double> &p_rGrad, std::vector<double> &p_rGradRank );
	void computeDispersion( const geo2d::VecDoublearray &p_rLine );

	const fliplib::SynchronePipe< interface::GeoVecDoublearray > *m_pPipeInLine;	 ///< In pipe
	fliplib::SynchronePipe< interface::GeoVecDoublearray > m_oPipeOutUpperLeft;   ///< above avg. left half
	fliplib::SynchronePipe< interface::GeoVecDoublearray > m_oPipeOutLowerLeft;   ///< below avg. left half
	fliplib::SynchronePipe< interface::GeoVecDoublearray > m_oPipeOutUpperRight;  ///< above avg. right half
	fliplib::SynchronePipe< interface::GeoVecDoublearray > m_oPipeOutLowerRight;  ///< below avg. right half
	fliplib::SynchronePipe< interface::GeoVecDoublearray > m_oPipeOutTrend;       ///< trend = avg. gradient over width

	interface::SmpTrafo m_oSpTrafo; ///< roi translation

	// parameters
	unsigned int m_oWidth;
	unsigned int m_oShift;

	// internal variables
	geo2d::VecDoublearray m_oKLine;
	geo2d::VecDoublearray m_oUpperLeft, m_oLowerLeft, m_oUpperRight, m_oLowerRight, m_oTrend;               // deviations
};

} // namespace precitec
} // namespace filter

#endif /* GETBASELINE_H_ */
