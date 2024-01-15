/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Stefan Birmanns (SB)
 *  @date		2011
 *  @brief		Performs a normalization of a signal between a given maximum and minimum.
 */

#ifndef LINENORMALIZE_H_
#define LINENORMALIZE_H_

#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include <common/frame.h>				///< ImageFrame
#include <image/image.h>				///< BImage
#include <geo/geo.h>					///< Size2d, VecDoublearray
#include <geo/array.h>					///< Array

namespace precitec {
namespace filter {

/**
 * @ingroup Filter_LineGeometry
 * @brief This filter performs a normalization of a signal between a given maximum and minimum.
 */
class FILTER_API LineNormalize  : public fliplib::TransformFilter
{
public:

	/// CTor.
	LineNormalize();
	/// DTor.
	virtual ~LineNormalize();

	static const std::string m_oFilterName;		///< Name Filter
	static const std::string PIPENAME_OUT1;		///< Name Out-Pipe

	/// Set filter parameter as defined in database / xml file.
	void setParameter();
	/// paints overerlay primitives
	void paint();

	/**
     * @brief Normalize a signal to a new value range.
     *
     * @param p_rLineIn    (Laser)-Line input object.
     * @param p_rLineOut   Low-pass filtered output object.
     * @param p_oMin       New minimum.
     * @param p_oMax       New maximum.
     */
	static void normalize( const geo2d::VecDoublearray &p_rLineIn, geo2d::VecDoublearray &p_rLineOut, int p_oMin, int p_oMax);

protected:

	/// In-pipe registration.
	bool subscribe(fliplib::BasePipe& p_pPipe, int p_oGroup);
	/// In-pipe event processing
	void proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rEventArg);

private:

	const fliplib::SynchronePipe< interface::GeoVecDoublearray >* 	m_pPipeLineIn;		///< In pipe
	fliplib::SynchronePipe< interface::GeoVecDoublearray >*       	m_pPipeLineOut;		///< Out pipe

	interface::SmpTrafo											m_oSpTrafo;			///< roi translation
	int														  	m_oMinimum;			///< New minimum
	int														  	m_oMaximum;			///< New maximum

	geo2d::VecDoublearray											m_oLineOut;			///< Output profile
};

} // namespace precitec
} // namespace filter

#endif /* LINENORMALIZE_H_ */
