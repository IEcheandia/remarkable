/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Andreas Beschorner (BA)
 *  @date		2014
 *  @ingroup    Filter_LineGeometry
 */


#ifndef FindBeadEnds_H_
#define FindBeadEnds_H_


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
 *  @brief	 Determines reasonable boundaries for beads and gaps given limiting ROIs.
 *
 *  Given parametrizable left and right ROIs, reasonable boundaries for beads and gaps are determined.
 *  In addition, the orientation (convex, concave) is fathomes w.r.t the given the boundaries.
 *
 * @param m_pPipeLineInGradTrend    Normalized Gradient trend output line from GradientTrend filter. Values in [-1, 1].
 * @param m_oRoiLeft                ROI into which the left end of the bead/gap falls.
 * @param m_oEndLeft                Parameter in [0, 1] that influences the extend/length of the left end. 0 = shorter ... 1 = longer.
 * @param m_oRoiRight               ROI into which the right end of the bead/gap falls.
 * @param m_oEndRight               Parameter in [0, 1] that influences the extend/length of the right end. 0 = shorter ... 1 = longer.
 *
 * @param[out] m_oOutLeftX          X coord of left marker/ start position of seam run on success.
 * @param[out] m_oOutLeftY          Value (NOT y coord!) at left marker/ start position of seam run on success.
 * @param[out] m_oOutRightX         X coord of right marker/ end position of seam run on success.
 * @param[out] m_oOutRightY         Value (NOT y coord!) at left marker/ end position of seam run on success.
 * @param[out] m_oOrientationValue  eOrientationConcave or eOrientationConvex on success, eOrientationInvalid otherwise.
 */
class FILTER_API FindBeadEnds  : public fliplib::TransformFilter
{
public:

	/// CTor.
	FindBeadEnds();

	static const std::string m_oFilterName;           ///< Name of filter
	static const std::string PIPENAME_OUTLeftX;       ///< Name of out pipe for x coordinate of left boundary or marker
	static const std::string PIPENAME_OUTLeftY;       ///< Name of out pipe for y coordinate of left boundary or marker
	static const std::string PIPENAME_OUTRightX;      ///< Name of out pipe for x coordinate of right boundary or marker
	static const std::string PIPENAME_OUTRightY;      ///< Name of out pipe for y coordinate of right boundary or marker
	static const std::string PIPENAME_OUTOrientation; ///< Name of out pipe for orientation of run (concave, convex, invalid)

	/// Sets filter parameters defined in database / xml file
	void setParameter();

	/// Paints overlay
	void paint();

protected:
	/// Internal results of analysis. Needed to signal the corresponding output to the pipe.
	enum stateOfAnalysis
	{
		eBeadOK = 0, eBeadInvalidOrientation = 1, eBeadBadLine = 2, eBeadAnalysisError = 3, eBeadNoBeadFound
	};
	/// Internal state for function findBeadsEnds::getBaseLinePos.
	enum stateRoiArea
	{
		eRoiOK = 0, eBadRoi = 1, eOutlier = 2, eNotch = 3, eUndefinedError
	};

	/// Signal outpipes. p_oIO=1 -> bead found, p_oIO = 0 -> bead not found, p_oIO = -1 -> laserline problem
	void signalSend(const interface::ImageContext &p_rImgContext, interface::ResultType p_oAnalysisResult, const int p_oIO);
	/// Integrate filter into pipe and filters patter
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	/// Main processing routine invoked by <b>pipes and filter</b> pattern implementation
	void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg);

	bool isOutlier(const int p_oTestDirection, const unsigned int p_oNumOutliersAbove, const unsigned int p_oNumOutliersBelow,
		const double p_oArea, const double p_oPercent) const;

	/** @brief Determines orientation by inspection of the extremal points of the gradient trend/ averaged gradient. Returns eBeadOK (0) on success.
	*
	* @param p_rGradTrendLine    Trend line of GradientTrend filter.
	* @return FindBeadEnds::stateOfAnalysis eBeadOK on success. More details below.
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
	*  \image html 01AllOKConvexData01.png "Characteristics of gradient trend and kCurvation for an optimal, here convex, seam run."
	*
	*  The kind and order the gradient trend's extremal points determine the orientation:
	*  <ul>
	*  <li>Convex: Max first, then min</li>
	*  <li>Concave: Min first, then max</li>
	*  <li>Invalid: anything else</li>
	*  </ul>
	* 
	*  Returns eBeadOK on success, FindBeadEnds::eBeadInvalidOrientation if no two extremal points satisfying the conditions given above
	*  are found and FindBeadEnds::eBeadBadLine if a pixel of bad rank (< eRankMax*0.95) occurs anywhere but at the margins.
    *
	*/
	stateRoiArea getBaseLinePos(int &p_rX, double &p_rY, int &p_rMin, int &p_rMax,  const bool p_oLeftRoi,
			const geo2d::VecDoublearray &p_rLine);

	RunOrientation determineOrientation(const geo2d::VecDoublearray &p_rLine,
			const int p_oXLeft, const double p_oYLeft, const int p_oXRight, const double p_oYRight);


/*
	*  Two parameters dictate the results of the analysis in this step, the SearchWinSize (FindBeadEnds::m_oSearchWinSize) and the TurningPointEps (FindBeadEnds::m_oTurningPointEps).
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

	/** @brief Determines reasonable left or right end of a potential bead/gap, given a limiting ROI
	*
	* @param[out] p_rX           X coord of determined endpoint
	* @param[out] p_rY           Y coord of determined endpoint
	* @param      p_oMin         X coord of left end of ROI
	* @param      p_oMax         Y coord of right end of ROI
	* @param      p_oRightToLeft Search from right ro left (left ROI) or from left to right (right ROI)
	* @param      p_rLine        Reference to laserline
	* @param      p_rGradTredn   Reference to noprmalized gradient trend
	*
	* @result FindBeadEnds::stateOfAnalysis   Returns result in form of analysis state: eBeadNoBeadFound on error, eBeadOK on success.
	*
	* @details Based on the results of FindBeadEnds::determineOrientation and FindBeadEnds::findEndpoints, this function
	* decides wether a potential seam run really is one or not.
	*
	*/
	stateOfAnalysis findEndpoint( int &p_rX, double &p_rY,
			const int p_oMin, const int p_oMax, const bool p_oRightToLeft,
			const geo2d::VecDoublearray &p_rLine,  const geo2d::VecDoublearray &p_rGradTrend);

	/** @brief Tests gradient trend and kCurvation for met constraints that qualify potential candidates as seam runs.
	*
	* @param[out] p_rPosMin  X coord of minimum on success, -1 otherwise.
	* @param[out] p_rValMin  Value of minimum (maybe invalid if p_rPosMin=-1)
	* @param[out] p_rPosMax  X coord of maximum on success, -1 otherwise.
	* @param[out] p_rValMax  Value of maximum (maybe invalid if p_rPosMax=-1)
	* @param p_rGradTrend          Trend line of GradientTrend filter.
	*
	* @details Based on the results of determineOrientation, this procedure tests the data for certain necessary conditions that need
	* to be fulfilled for a seam run.\n\n
	*/
	stateOfAnalysis verifyRun( int &p_rPosMin, double &p_rValMin, int &p_rPosMax, double &p_rValMax,
		const geo2d::VecDoublearray &p_rLine, const geo2d::VecDoublearray &p_rGradTrendLine);

	bool isValidLine(const geo2d::VecDoublearray &p_rLine);

private:
	bool inROIX(const int p_oX, const geo2d::Rect &p_rRoi);
	bool inROIY(const int p_oY, const geo2d::Rect &p_rRoi);
	static const int m_oRankThreshold;  ///< eRankMax*0.95. No pixel of the incoming trends may be of quality below this threshold.

	const fliplib::SynchronePipe< interface::GeoVecDoublearray >* m_pPipeLineIn;             ///< In pipe for laserline
	const fliplib::SynchronePipe< interface::GeoVecDoublearray >* m_pPipeLineInGradTrend;    ///< In pipe for gradient trend from gradientTrend filter
	const fliplib::SynchronePipe< interface::ImageFrame >* m_pPipeInRoiLeft;                 ///< In pipe for left ROI
	const fliplib::SynchronePipe< interface::ImageFrame >* m_pPipeInRoiRight;                ///< In pipe for right ROI

	fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipePositionOutLeftX;	///< Out pipe for left marker, xpos
	fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipePositionOutLeftY;	///< Out pipe for left marker, value
	fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipePositionOutRightX;	///< Out pipe for right marker, xpos
	fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipePositionOutRightY;	///< Out pipe for right marker, value
	fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutOrientation;      ///< Out pipe for orientation of run/bump (convex, concave, invalid)

	interface::SmpTrafo  m_oSpTrafo;    ///< Roi translation

	// internal variables
	geo2d::Doublearray m_oOutLeftX;         ///< Variable holding result value of left marker's x coord for corresponding out pipe
	geo2d::Doublearray m_oOutLeftY;         ///< Variable holding result value of left marker's value for corresponding out pipe
	geo2d::Doublearray m_oOutRightX;        ///< Variable holding result value of right marker's x coord for corresponding out pipe
	geo2d::Doublearray m_oOutRightY;        ///< Variable holding result value of right marker's value for corresponding out pipe
	geo2d::Doublearray m_oOrientationValue; ///< Variable holding result value of orientation of seam run for corresponding out pipe

	RunOrientation m_oOrientation;  ///< Orientation of potential seam run (concav, convex, invalid)

	// parameters
	geo2d::Rect m_oRoiLeft;		    ///< left roi
	geo2d::Rect m_oRoiRight;		///< right roi
	double m_oEndLeft;              ///< Factor influencing left end of bead
	double m_oEndRight;             ///< Factor influencing right end of bead

	// outlier
	double m_oOutlierLeft;         ///< Outlier limit of laserline in left roi
	double m_oOutlierRight;        ///< Outlier limit of laserline in right roi
	int m_oDirOutlierLeft;         ///< Direction flag. Determines, which pixel account to outliers (None, Above, Below, Both)
	int m_oDirOutlierRight;        ///< Direction flag. Determines, which pixel account to outliers (None, Above, Below, Both)

	// notches (can account to outliers, too!)
	double m_oNotchesLeft;         ///< Notch limit of laserline in left roi
	double m_oNotchesRight;        ///< Notch limit of laserline in right roi
	int m_oDirNotchesLeft;         ///< Direction flag. Determines, which pixel account to outliers (None, Above, Below, Both)
	int m_oDirNotchesRight;        ///< Direction flag. Determines, which pixel account to outliers (None, Above, Below, Both)
	int m_oRoiOffsetX;              ///< Offset left limitnig ROI to main ROI, x coord
	int m_oRoiOffsetY;              ///< Offset left limitnig ROI to main ROI, y coord

	int m_oStartpos, m_oEndpos;     ///< Margin position, allowing for bad rank margins. Those are the real start and end of the line segment inspected.
	double m_oSlope;
	double m_oIntercept;

	bool m_oPaint;

	interface::ResultType m_oAnalysisResult;

	static const double m_oMarginSize;

	// for paint
	std::vector<geo2d::Point> m_oLine;
};

} // namespace precitec
} // namespace filter

#endif /* FindBeadEnds_H_ */
