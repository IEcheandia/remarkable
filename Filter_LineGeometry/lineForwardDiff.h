/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Stefan Birmanns (SB)
 *  @date		2011
 *  @brief		Computes a simple gradient / forward difference of a signal.
 */

#ifndef LINEFORWARDDIFF_H_
#define LINEFORWARDDIFF_H_

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
 * @brief This filter performs a simple forward difference / gradient calculation.
 */
class FILTER_API LineForwardDiff  : public fliplib::TransformFilter
{
public:

	/// CTor.
	LineForwardDiff();
	/// DTor.
	virtual ~LineForwardDiff();

	static const std::string m_oFilterName;		///< Name Filter
	static const std::string PIPENAME_OUT1;		///< Name Out-Pipe

	/// Set filter parameter defined in database / xml file.
	void setParameter();
	/// paints overerlay primitives
	void paint();

	/**
	 * @brief The function computes a simple gradient / forward difference of a signal.
	 *
	 * @param p_rLineIn    	(Laser)-Line input object.
	 * @param p_rLineOut   	Low-pass filtered output object.
	 */
	static void forwardDiff( const geo2d::VecDoublearray &p_rLineIn, geo2d::VecDoublearray &p_rLineOut);

protected:

	/// in pipe registration
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	/// in pipe event processing
	void proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rEventArg);

private:

	const fliplib::SynchronePipe< interface::GeoVecDoublearray >* 	m_pPipeLineIn;    	///< In pipe
	fliplib::SynchronePipe< interface::GeoVecDoublearray >*       	m_pPipeLineOut;		///< Out pipe

	interface::SmpTrafo											m_oSpTrafo;			///< roi translation
	geo2d::VecDoublearray										m_oLineOut;			///< Output profile
};

} // namespace precitec
} // namespace filter

#endif /* LINEFORWARDDIFF_H_ */
