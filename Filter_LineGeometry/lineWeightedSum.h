/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Stefan Birmanns (SB)
 * 	@date		2011
 * 	@brief 		This filter computes the weighted sum of two signals / laser lines.
 */

#ifndef LINEWEIGHTEDSUM_H_
#define LINEWEIGHTEDSUM_H_

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
 * @brief This filter computes the linear weighted sum of two input signals and produces a new output signal.
 */
class FILTER_API LineWeightedSum  : public fliplib::TransformFilter
{
public:

	/// CTor.
	LineWeightedSum();
	/// DTor.
	virtual ~LineWeightedSum();

	static const std::string m_oFilterName;		///< Name Filter
	static const std::string PIPENAME_OUT1;		///< Name Out-Pipe

	/// Set filter parameters as defined in database / xml file.
	void setParameter();
	/// paints overerlay primitives
	void paint();

	/**
	 * @brief Computes the weighted sum of two signals / laser lines.
	 * @param p_oWeightA       Weight factor for line A.
	 * @param p_rLineInA       First laser line input object.
	 * @param p_oWeightB       Weight factor for line B.
	 * @param p_rLineInB       Second laser line input object.
	 * @param p_rLineOut       Profile output object.
	 */
	static void weightedSum(const float p_oWeightA, const geo2d::VecDoublearray &p_rLineInA, const float p_oWeightB, const geo2d::VecDoublearray &p_rLineInB, geo2d::VecDoublearray &p_rLineOut);

protected:

	/// In-pipe registration.
	bool subscribe(fliplib::BasePipe& pipe, int group);
	/// In-pipe event processing.
	void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

private:
	const fliplib::SynchronePipe< interface::GeoVecDoublearray >* 	m_pPipeLineInA;		///< In pipe
	const fliplib::SynchronePipe< interface::GeoVecDoublearray >* 	m_pPipeLineInB;		///< In pipe
	fliplib::SynchronePipe< interface::GeoVecDoublearray >*       	m_pPipeLineOut;		///< Out pipe

	interface::SmpTrafo											m_oSpTrafo;			///< roi translation
	float														m_oWeightA;			///< Weight parameter for line A
	float														m_oWeightB;			///< Weight parameter for line B

	geo2d::VecDoublearray										m_oLineOut;			///< Output profile
};

} // namespace precitec
} // namespace filter

#endif /* LINEWEIGHTEDSUM_H_ */
