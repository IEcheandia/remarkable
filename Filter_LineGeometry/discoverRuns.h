/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Andreas Beschorner (BA)
 *  @date		2012 - 2013
 *  @ingroup    Filter_LineGeometry
 */


#ifndef DiscoverRuns_H_
#define DiscoverRuns_H_


#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include <image/image.h>				///< BImage
#include "filter/parameterEnums.h"		///< enum ExtremumType

#include <common/frame.h>				///< ImageFrame
#include <geo/geo.h>					///< Size2d, VecDoublearray
#include <geo/array.h>					///< ByteArray

#include <filter/structures.h>

namespace precitec {
namespace filter {


/**
 *  @brief	 Searches for most prominent seam run in laserline.
 *
 *  Searches for for most prominent seam run in laserline given results of KCurvation and GradientTrend filters.
 *  Following a discussion (SB, JS, AB, GUR) on february 15th 2013, the filter can expect the results from GradientTrend and KCurvation
 *  on a continuous, connected good rank laserline with potential areas of bad ranks only at the margins, if at all. No singularities, however!
 *  Returns boundaries of seam run on success; returns orientation (convex or concave) of seam run on success, invalid orientation otherwise.
 *
 * @param m_pPipeLineInKCurve        Trend line (smoothed version) from KCurvation filter.
 * @param m_pPipeLineInGradTrend     Gradient trend output line from GradientTrend filter.
 * @param m_pPipeInSignatureChanges  Signature changes out line from GradientTrend filter.
 * @param m_oSearchWinSize           Extend of search for finding run limits. Influences length of seam run and to a certain degree whether
 *                                   a run can be found at all. See findEndpoints for details.
 * @param m_oTurningPointEps         Size of Neighbourhood of signature change/ turning point. See DiscoverRuns::findEndpoints for details.
 *
 * @param[out] m_oOutLeftX          X coord of left marker/ start position of seam run on success.
 * @param[out] m_oOutLeftY          Value (NOT y coord!) at left marker/ start position of seam run on success.
 * @param[out] m_oOutRightX         X coord of right marker/ end position of seam run on success.
 * @param[out] m_oOutRightY         Value (NOT y coord!) at left marker/ end position of seam run on success.
 * @param[out] m_oOrientationValue  eOrientationConcave or eOrientationConvex on success, eOrientationInvalid otherwise.
 */
class FILTER_API DiscoverRuns  : public fliplib::TransformFilter
{
public:

	/// CTor.
	DiscoverRuns();

	static const std::string m_oFilterName;           ///< Name of filter
	static const std::string PIPENAME_OUTLeftX;       ///< Name of out pipe for x coordinate of left boundary or marker
	static const std::string PIPENAME_OUTLeftY;       ///< Name of out pipe for y coordinate of left boundary or marker
	static const std::string PIPENAME_OUTRightX;      ///< Name of out pipe for x coordinate of right boundary or marker
	static const std::string PIPENAME_OUTRightY;      ///< Name of out pipe for y coordinate of right boundary or marker
	static const std::string PIPENAME_OUTOrientation; ///< Name of out pipe for orientation of run (concav, convex, invalid)

	/// Sets filter parameters defined in database / xml file
	void setParameter();

	/// Paints overlay
	void paint();

protected:
	/// Internal results of analysis. Needed to signal the corresponding output to the pipe.
	enum stateOfAnalysis
	{
		eRunOK = 0, eRunInvalidOrientation = 1, eRunBadLine = 2, eRunNoRunFound
	};

	/// Signal outpipes. p_oIO=1 -> bead found, p_oIO = 0 -> bead not found, p_oIO = -1 -> laserline problem
	void signalSend(const interface::ImageContext &p_rImgContext, interface::ResultType p_oAnalysisResult, const int p_oIO);
	/// Integrate filter into pipe and filters patter
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	/// Main processing routine invoked by <b>pipes and filter</b> pattern implementation
	void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg);

	/** @brief Determines orientation by inspection of the extremal points of the gradient trend/ averaged gradient. Returns eRunOK (0) on success.
	*
	* @param p_rGradTrendLine    Trend line of GradientTrend filter.
	* @param p_rKCurveTrendLine  Trend line of KCurvation filter.
	* @return DiscoverRuns::stateOfAnalysis DiscoverRuns::eRunOK on success. More details below. 
	*
	* @details The image below shows what the gradient trend and the kCurvation trend look like for the convex case.
	*         If the rank of any pixel deceeds m_oRankThreshold, the function returns false!.\n
	*         The extremal points clearly mark the most prominent humps.\n
	*
	*  \image html 01AllOKConvex.png "kCurvation (bottom) and gradient (top) trend lines for the case of a convex seam run (laserline, middle purple line)"
	*
	*  Given growing coordinate values for the xcoord from left to right and for the ycoord from top to bottom, for convex runs
	*  the gradient trend has a ycoord maximum to the left of a minimum. The direction of the gradient trend left from max and
	*  right from min is downwards. Next, the kCurvation line in the x coord neighbourhood of these
	*  extremal points has -- in good situations -- two minimae, one near each extremal position, and is negative!
	*  For concave seam runs, things are just upside down: a gradient trend minimum preceeds a maximum and the kCurvation extremal points
	*  near those are maximae. The next figure illustrates the data for a convex seam run.
	*
	*  \image html 01AllOKConvexData01.png "Characteristics of gradient trand and kCurvation for an optimal, here convex, seam run."
	*
	*  The kind and order the gradient trend's extremal points determine the orientation:
	*  <ul>
	*  <li>Convex: Max first, then min</li>
	*  <li>Concave: Min first, then max</li>
	*  <li>Invalid: anything else</li>
	*  </ul>
	* 
	*  Returns DiscoverRuns::eRunOK on success, DiscoverRuns::eRunInvalidOrientation if no two extremal points satisfying the conditions given above
	*  are found and DiscoverRuns::eRunBadLine if a pixel of bad rank (< eRankMax*0.95) occurs anywhere but at the margins.
  *
	*/
	stateOfAnalysis determineOrientation(const geo2d::VecDoublearray &p_rGradTrendLine, const geo2d::VecDoublearray &p_rKCurveTrendLine);

	/** @brief Tests gradient trend and kCurvation for met constraints that qualify potential candidates as seam runs.
	*
	* @param[out] p_rPosMin  X coord of minimum on success, -1 otherwise.
	* @param[out] p_rValMin  Value of minimum (maybe invalid if p_rPosMin=-1)
	* @param[out] p_rPosMax  X coord of maximum on success, -1 otherwise.
	* @param[out] p_rValMax  Value of maximum (maybe invalid if p_rPosMax=-1)
	* @param p_rKCurveLine         Trend line of KCurvation filter.
	* @param p_rGradTrend          Trend line of GradientTrend filter.
	*
	* @details Based on the results of determineOrientation, this procedure tests the data for certain necessary conditions that need
	* to be fulfilled for a seam run.\n\n
	*
	*  Two parameters dictate the results of the analysis in this step, the SearchWinSize (DiscoverRuns::m_oSearchWinSize) and the TurningPointEps (DiscoverRuns::m_oTurningPointEps).
	*  <ul>
	*  <li>The first represents the distance in pixel, how far from each extremal point of the gradient trend the search for a potential start/end of
	*  the seam run extends. In the following figure below, this search size is depicted for the left extremal point only.</li>
	*  <li>The TurningPointEps defines an area around the turning point of the gradient trend between the two extremal points, illustrated by the brackets
	*  in the next figure. The idea behind the turning point and an epsilon neighbourhood around it is the following:
	*  The ycoord of the turning point apprxomately represents the relative height of the baseline and is invariant to quite some degree to images with
	*  stronger throuhgout slopes. The algorithm will search until a first value in this ycoord epsilon neighbourhood is found.</li>
	* </ul>
	*  Optimal start and endpoints will have ycoords near to that turning point, and the epsilon neighbourhood around it allows for more or less freedom.
	*  \image html 02SeachrWinSizeTooSmallData.png "Another image and its analysis results depicting the meaning of the parameters."
	*
	*  In the preceeding figure, SearchWinSize and TurningPointEps are chosen such that the result of the analysis is rather suboptimal:
	*  The extend of the search window is too small to find a more adequate start of the run, and in combination with a large TurningPoinEps
	*  the resulting start is too far tor the right. We want to improve this result and first adjust the parameter SearchWindowSize by chosing
	*  a gretaer value (here 90 pixel instead of 60). The subsequent figure shows the result.
	*  \image html 02SeachWinBigWinSizeBigTP.png "The same image with larger SearchWindowsSize"
	*
	* The result seems fine, but now we are facing less good results for the other, first image:
	*  \image html 01AllOKConvexBigWinSize.png "Parameters improving the results for one image may worsen that for another!"
	*
	* We realize that the result would be better if we allowed for a ycoord farther away from the turning point this would help setting the startpoint
	* farther to the right. The next two images show the results for both enlarged SearchWindowSize (90) reduced the TurningPointEps (from 6 to 2 in this case).
	* \image html 01AllOKConvexBigWinSizeSmallTurningPoint.png "Image number one again, now with tuned parameters..."
	* \image html 02SearchWinBigWinSizeSmallTPData.png "...and number two: good results as well!"
	*/
	stateOfAnalysis findEndpoints(int &p_rPosMin, double &p_rValMin, int &p_rPosMax, double &p_rValMax,
		const geo2d::VecDoublearray &p_rKCurveLine, const geo2d::VecDoublearray &p_rGradTrend);

	/** @brief Returns position (xcoord) of signature change between extremal points of GradientTrend.
	* @param p_rSigChanges Vector of all signature changes of GradientTrend filter.
	* @return Integer index representing x coord of turning point on success, -1 otherwise.
	*
	* @details The signature change between the extremal points marks the turning point.
	*/
	int getSigChangeIndex(const geo2d::VecDoublearray& p_rSigChanges); ///< Determines positions of signature change

	/// Checks direciotn on the gradient trend in a small neighbourhood around the given x coord p_oPos
	bool isGradTrendDirection(const geo2d::VecDoublearray &p_rGradTrend, const unsigned int p_oPos, const bool p_oDirDown);
	/// Checks, whether the direction of the gradient trend  is downwards (from left to right)
	bool isGradTrendDownwards(const geo2d::VecDoublearray &p_rGradTrend, const unsigned int p_oPos);
	/// Checks, whether the direction of the gradient trend  is upwards (from left to right)
	bool isGradTrendUpwards(const geo2d::VecDoublearray &p_rGradTrend, const unsigned int p_oPos);

	/** @brief Determines whether there is a seam run or not.
	*
	* @param[out] p_rPosMin  see DiscoverRuns::findEndpoints.
	* @param[out] p_rValMin  see DiscoverRuns::findEndpoints.
	* @param[out] p_rPosMax  see DiscoverRuns::findEndpoints.
	* @param[out] p_rValMax  see DiscoverRuns::findEndpoints.
	* @param p_rGradTrendLine    Trend line of GradientTrend filter.
	* @param p_rKCurveTrendLine  Trend line of KCurvation filter.
	* @param p_rSigChanges       Signature changes from GradientTrend filter.
	*
	* @result DiscoverRuns::stateOfAnalysis Based on called functions, see below. Returns DiscoverRuns::eRunOK on success.
	*
	* @details Based on the results of DiscoverRuns::determineOrientation, DiscoverRuns::getSigChanges and DiscoverRuns::findEndpoints, this function
	* decides wether a potential seam run really is one or not.
	*
	*/
	stateOfAnalysis verifyRun( int &p_rPosMin, double &p_rValMin, int &p_rPosMax, double &p_rValMax,
		const geo2d::VecDoublearray &p_rGradTrendLine, const geo2d::VecDoublearray &p_rKCurveTrendLine, const geo2d::VecDoublearray &p_rSigChanges);

	/** @brief Tests whether the parameter value p_oVal is in the interval [m_oValAtSigChange - m_oValEpsOK, m_oValAtSigChange + m_oValEpsOK]
	*
	* @param p_oVal  Value to be checked.
	* @returns true if value is in the above mentioned interval, false otherwise
	*
	* @details See DiscoverRuns::findEndpoints for detailed information. This functions comes into play when testing values for being in the
	*         m_oTurningPointEps neighbourhood of the turning point.
	*/
	bool isNearSigChangeVal(const double p_oVal, bool&);
	bool isValidLine(const geo2d::VecDoublearray &p_rLine);

private:
	static const int m_oRankThreshold;  ///< eRankMax*0.95. No pixel of the incoming trends may be of quality below this threshold.

	const fliplib::SynchronePipe< interface::GeoVecDoublearray >* m_pPipeLineInKCurve; ///< In pipe for kCurvation trend from kCurvation filter
	const fliplib::SynchronePipe< interface::GeoVecDoublearray >* m_pPipeLineInGradTrend;   ///< In pipe for gradient trend from gradientTrend filter
	const fliplib::SynchronePipe< interface::GeoVecDoublearray >* m_pPipeInSignatureChanges; ///< In pipe for signature changes from gradientTrend filter

	fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipePositionOutLeftX;	///< Out pipe for left marker, xpos
	fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipePositionOutLeftY;	///< Out pipe for left marker, value
	fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipePositionOutRightX;	///< Out pipe for right marker, xpos
	fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipePositionOutRightY;	///< Out pipe for right marker, value
	fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutOrientation;    ///< Out pipe for orientation of run/bump (convex, concav, invalid)

	interface::SmpTrafo  m_oSpTrafo;		 ///< Roi translation

	// parameters
	unsigned int m_oSearchWinSize;      ///< Extend of search for finding run limits. Influences length of seam run and to a certain degree whether a run can be found at all. See findEndpoints for details.
	unsigned int m_oTurningPointEps;    ///< Size of Neighbourhood of signature change/ turning point. See findEndpoints for details.

	// internal variables
	geo2d::Doublearray m_oOutLeftX;         ///< Variable holding result value of left marker's x coord for coresponding out pipe
	geo2d::Doublearray m_oOutLeftY;         ///< Variable holding result value of left marker's value for coresponding out pipe
	geo2d::Doublearray m_oOutRightX;        ///< Variable holding result value of right marker's x coord for coresponding out pipe
	geo2d::Doublearray m_oOutRightY;        ///< Variable holding result value of right marker's value for coresponding out pipe
	geo2d::Doublearray m_oOrientationValue; ///< Variable holding result value of orientation of seam run for corresponding out pipe

	int m_oMinX, m_oMaxX;           ///< Coords of extremae
	double m_oMinY, m_oMaxY;        ///< Values of extremae
	RunOrientation m_oOrientation;  ///< Orientation of potential seam run (concav, convex, invalid)

	int m_oStartpos, m_oEndpos;     ///< Margin position, allowing for bad rank margins. Those are the real start and end of the line segment inspected.

	double m_oValAtSigChange;       ///< Value at signature change between gradient trend extremal points. Necessary for finding run limits.
	double m_oValEpsOK;             ///< Variable for parameter <i>TurningPointEps</i> for \f$\epsilon\f$-tube around position of signature changes. Influences seam run length.

	bool m_oPaint;

	static const double m_oMarginSize; ///< Percentage of allowed bad rank pixel at margins in [0.0, 1.0], following discussion of BA & JS on 2013/04/25
};

} // namespace precitec
} // namespace filter

#endif /* DiscoverRuns_H_ */
