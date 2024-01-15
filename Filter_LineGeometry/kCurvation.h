/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		  Andreas Beschorner (BA)
 *  @date		    2012
 *  @brief	    Computes K-Curvation (see: Mustererkennung Proceedings 1991, 13. DAGM-Symposium, p. 168).
 */

#ifndef KCURVATION_H_
#define KCURVATION_H_

#include <vector>

#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include <image/image.h>				///< BImage
#include <common/frame.h>				///< ImageFrame
#include <geo/geo.h>					///< Size2d, VecDoublearray
#include <geo/array.h>					///< ByteArray

/// @brief: namespace precitec
namespace precitec {
/// @brief: namespace filter
namespace filter {

/**
 * @ingroup Filter_LineGeometry
 * @brief Computes \f$k\f$-curvation of a line.

 * Given a line input, this filter computes the so called k-curvation. Given a point \f$(x_i, y(x_i))\f$ on a line and a positive integer \f$k\f$
 * and a valid line position index \f$i\f$, the \f$k\f$-curvation computes the difference of the slopes of the two lines runnning through points
 * \f$(x_{i-k}, y(x_{i-k})), (x_i, y(x_i))\f$ and \f$(x_i, y(x_i)), (x_{i+k}, y(x_{i+k}))\f$, respectively.
 * In contrast to the reference given in the documentation of kCurvation.h, we do not compute the absolutes value due to the additional information
 * (direction of curvation) the signature delivers.
 */
class FILTER_API KCurvation : public fliplib::TransformFilter
{
public:

	/// CTor and DTor.
	KCurvation();
	virtual ~KCurvation();

	static const std::string m_oFilterName;		    ///< Name of filter
	static const std::string PIPENAME_KCURVATION;	///< Name Out kCurvation
	static const std::string PIPENAME_Trend;		  ///< Name Out Trend (= Avg. over windows length 2*shiftK+1)
	static const std::string m_oParameterComputeAnglesRotationInvariantName;///< Name of parameter m_oComputeAnglesRotationInvariant

	/// Set filter parameters defined in database / xml file
	void setParameter();

	/// paints overlay
	void paint();
	 
	void arm(const fliplib::ArmStateBase& state);	///< arm filter

protected:
	/// Internal results fo output correct values to pipes.
	enum resultOfKCurvation {eKCurvAllOK=0, eKCurvNoTrend=1, eKCurvNothing=2};
	struct KCurvationData
	{
		int m_oRankLeft = 0;       ///< rank of left line segment for both regression and two point case
		int m_oRankRight = 0;      ///< rank of left line segment for both regression and two point case
		double m_oSlopeSum = 0.0;
		double m_oKCurvation = 0.0;        
	};

	/// In pipe registration.
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	/// In pipe event processing.
	void proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rEventArg);
	/// Tests whether we have enough pixel of good rank for performing kCurvation
	bool isValidLine(const geo2d::Doublearray & p_rLine, int &p_rStartPos, int &p_rEndPos) const;

	void rescale(std::vector<double> &p_rGrad, const double p_oMin, const double p_oMax, const double p_oMinG, const double p_oMaxG) const;

	/// Computes trend/ averaged /f$k/f$-curvation
	resultOfKCurvation computeTrend(geo2d::Doublearray & rOutTrend, const geo2d::Doublearray &p_rKLine, const int p_oStartPos, const int p_oEndPos, const int p_oLength ) const;
	/// Main procedure. Comnputes signed /f$k/f$-curvation at x-coordinate p_oIdx given input line p_rLine.
	KCurvationData computeKCurvation( const geo2d::Doublearray &p_rLine, 
		const unsigned int p_oIdx, const unsigned int p_oMarginShift);

	const fliplib::SynchronePipe< interface::GeoVecDoublearray > *m_pPipeInLine;	///< In pipe
	fliplib::SynchronePipe< interface::GeoVecDoublearray > m_oPipeOutKLine;   ///< K Curvation out line
	fliplib::SynchronePipe< interface::GeoVecDoublearray > m_oPipeOutTrend;   ///< K Curvation out trend, smoothed/ averaged version. Reduces discontinuities.
	
	interface::SmpTrafo m_oSpTrafo; ///< roi translation

	// parameters
	unsigned int m_oK;  ///< Size of \f$ k \f$, determines characteristics of line segments. For \f$ k = 1 \f$, /f$k/f$-curvation equals the ordinary discrete gradient.
	unsigned int m_oTrendWindowLength; ///< Size of trend/ averaging window.
	/** Use regression for computing the lines. If true, all points on the lines between \f$ x, x\pm k \f$ are used to compute the segments via least mean square linear regression.
	Otherwise, the computation uses the limiting points only. 
	*/
	bool m_oUseRegression;
	bool m_oComputeAnglesRotationInvariant;
	
	// internal variables
	double m_oKInverse;             ///< ... To substitute multiplications by divisions ...
	geo2d::VecDoublearray m_oKLine; ///< line out of ordinary k-curvation line
	geo2d::VecDoublearray m_oTrend; ///< line out of averaged (hence smoothed) k-curvation line

	double *m_oLeftX;      ///< for regressions: x coords of left linesegment
	double *m_oRightX;     ///< for regressions: y coords of left linesegment
	double *m_oLeftY;      ///< for regressions: x coords of right linesegment
	double *m_oRightY;     ///< for regressions: y coords of right linesegment

};

} // namespace precitec
} // namespace filter

#endif /* GETBASELINE_H_ */
