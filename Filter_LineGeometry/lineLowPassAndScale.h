/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Andreas Beschorner (AB)
 *  @date		2013
 *  @brief		Low-pass filter for the laser-lines.
 */

#ifndef LineLowPassAndScale_H_
#define LineLowPassAndScale_H_

#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include <common/frame.h>				///< ImageFrame
#include <image/image.h>				///< BImage
#include <geo/geo.h>					///< Size2d, VecDoublearray
#include <geo/array.h>					///< ByteArray

namespace precitec {
namespace filter {

/**
 * @ingroup Filter_LineGeometry
 * @brief Low pass filters and scales a signal and removes, depending on the size of FirWeight, high-frequency parts.
 */
class FILTER_API LineLowPassAndScale  : public fliplib::TransformFilter
{
public:

	/// CTor.
	LineLowPassAndScale();
	/// DTor.
	virtual ~LineLowPassAndScale();

	static const std::string m_oFilterName;		///< Name Filter
	static const std::string PIPENAME_OUT1;		///< Name Out-Pipe

	/// Set filter parameters as defined in database / xml file.
	void setParameter();
	/// paints overerlay primitives
	void paint();

	/**
	 * @brief  Low pass filter a signal and removes, depending on the size of FirWeight, high-frequency parts. The signal will not get shifted.
	 * @details The reason for this filter to include a scaling option is, that -- in contrast to ordinary independent scaling -- it automatically interpolates
	 *         between scaled outputs (during a second backloop) and thus both scales and smoothes a signal. Just scaling a low passed signal would result in
	 *         potentially large gaps.
	 * @param p_rLineIn      (Laser)-Line input object.
	 * @param p_oFirWeight   Coefficient \f$ c \f$ of the recursive term in percent: Let \f$ x_i, y_i \f$ be input- and output signal, respectively,
	 *                       at position \f$ i > 1 \f$. Then \f$ y_{i+1} = \alpha y_i + (1-alpha)x_i,\; 0 \le \alpha \le 1 \f$ and \f$y_0=x_0\f$.
	 *                       Here, \f$ \alpha = \frac{c}{100}. \f$
	 * @param p_oScale       Scale factor for post filtering scaling.
	 * @param p_oMulOrDiv    Multiply or divide by p_oScale. IOW: Amplify or attenuate  input signal.
	 * 						0: Signal unchanged
	 * 						1: Computes average of two elements
	 * 						2 and higher: Low-pass.
	 * @param p_rLineOut    Low-pass filtered output object.
	 */
	void lowPass( const geo2d::VecDoublearray &p_rLineIn, double p_oScale, int p_oFirWeight, geo2d::VecDoublearray &p_rLineOut);

protected:
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	/// In-pipe event processing.
	void proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rEventArg);

private:
	const fliplib::SynchronePipe< interface::GeoVecDoublearray >* 	m_pPipeLineIn;    	///< In pipe
	fliplib::SynchronePipe< interface::GeoVecDoublearray >*       	m_pPipeLineOut;		///< Out pipe

	// parameters
	int					m_oFirWeight;         ///< FirWeight of the low-pass, iow impact of output signal. Range [0, 100].  See documentation details.
	int					m_oScale;             ///< Signal scale factor (multiplied to the signal before the low-pass). Range [1, 100]. See documentation details.
	int                 m_oMulOrDiv;          ///< Divide or multiply by scale factor. Range [0, 1]
	bool                m_oInterpolateScaled; ///< Interpolate Scaled version (true) or leave value jumps in signal (false)

	// internal variables
	interface::SmpTrafo		m_oSpTrafo;			///< roi translation
	geo2d::VecDoublearray		m_oLineOut;			///< Output profile
	bool m_oPaint; ///< Do NOT paint in case of error in preceeding filter
};

} // namespace precitec
} // namespace filter

#endif /* LineLowPassAndScale_H_ */
