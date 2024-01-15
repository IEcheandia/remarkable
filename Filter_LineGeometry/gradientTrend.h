/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Andreas Beschorner (BA)
 *  @date		2012 - 2013
 *  @ingroup    Filter_LineGeometry
 */

#ifndef GradientTrend_H_
#define GradientTrend_H_

#include <vector>

#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include <image/image.h>				///< BImage
#include <common/frame.h>				///< ImageFrame
#include <geo/geo.h>					///< Size2d, VecDoublearray
#include <geo/array.h>					///< ByteArray

#include <filter/structures.h>

namespace precitec {
namespace filter {

/**
 * @ingroup Filter_LineGeometry
 * @brief Computes windowed average gradient
 */
class FILTER_API GradientTrend : public fliplib::TransformFilter
{
public:
	/// CTor and DTor.
	GradientTrend();
	virtual ~GradientTrend();

	static const std::string m_oFilterName;		 ///< Name Filter
	static const std::string PIPENAME_Trend;     ///< Name Out Below Avg. left
	static const std::string PIPENAME_Sigs;      ///< Name Out signature changes

	/// Set filter parameters defined in database / xml file
	void setParameter();

	/// paints overlay
	void paint();
	 
protected:
	/// Integrate filter into pipe and filters pattern
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	
	bool isValidLine(const geo2d::VecDoublearray p_rLine, int &p_rStartPos, int &p_rEndPos);

	/// Tests if two given doubles have different signatures and, if so, adds the signature at given index into a vector of signature changes.
	void processSignatureChange(const int, const double, const double);
	/**
	Computes discrete gradient of laserline (delta 1 pixel to left and right from current point) and sets rank to the lower of the two ranks. 
	As this method is primariliy developed for the seam run detection, it differs from our common averaging method as it throws bad rank results\n
	<ul>
	  <li>Whenever the point in question (for which the average is computed) is of rank < (eRankMax/10)</li>
	  <li>Whenever >33% of pixels in the smoothing window are of rank < (eRankMax/10)</li>
	</ul>
	In addition, connected bad rank intervals may exist at left and right margins, the size of which is arbitrary.
	*/
	void computeGradient( const geo2d::VecDoublearray &p_rLine, std::vector<double> &p_rGrad, std::vector<int> &p_rGradRank );
	/// Averages gradient over window of size m_oWidth. Margins are averaged only over the length from to the point in question to the respective border.
	bool computeGradientTrend( const geo2d::VecDoublearray &p_rLine, const unsigned int p_oStartPos, const unsigned int p_oEndPos );

	void rescale(std::vector<double> &p_rGrad, const double p_oMin, const double p_oMax);

	/// Main processing routine invoked by <b>pipes and filter</b> pattern implementation
	void proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rEventArg);

	const fliplib::SynchronePipe< interface::GeoVecDoublearray > *m_pPipeInLine;	  ///< In pipe
	fliplib::SynchronePipe< interface::GeoVecDoublearray > m_oPipeOutTrend;       ///< trend = avg. gradient over Width
	fliplib::SynchronePipe< interface::GeoVecDoublearray > m_oPipeOutSigs;        ///< vector of signature changes of gradient trend

	interface::SmpTrafo m_oSpTrafo; ///< roi translation

	// parameters
	unsigned int m_oWidth;  ///< Length of trend/ averaging window
	unsigned int m_oShift;  ///< Shift between two successive trend points. Currently fix at 1, that means for each pixel of the original line the tren is computed. Potentially redundant information...

	// internal variables
	geo2d::VecDoublearray m_oTrend;               ///< Array of deviations of neighboured line points
	geo2d::VecDoublearray m_oSigChangeVector;     ///< Array of signature changes for output pipe
	double m_oMinG;                               ///< Min. value of gradient trend
	double m_oMaxG;                               ///< Max. value of gradient trend

	std::vector<SigChange> m_oSignatureChange;    ///< Container where signature changes are collected in
};

} // namespace precitec
} // namespace filter

#endif /* GETBASELINE_H_ */
