/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Stefan Birmanns (SB)
 * 	@date		2011
 * 	@brief 		Extracts an element of a laser-line and returns it as a position.
 */

#ifndef LINEPOS_H_
#define LINEPOS_H_

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
 * @brief Extracts an element of a laser-line and returns it as a position.
 *
 * @details This filter takes two input pipes, a laser line and a position and interprets the x coordinate of the position as index into the laser line. The output is a position. This
 * can be useful if a filter has extracted a feature not directly from a laser line, but from a manipulated version (low-pass filtered, etc). In such a case one often wants to
 * re-transform the point onto the laser line itself, which can be accomplished using this filter.
 */
class FILTER_API LinePos  : public fliplib::TransformFilter
{

public:

	/// CTor.
	LinePos();
	/// DTor.
	virtual ~LinePos();

	static const std::string m_oFilterName;		///< Name Filter
	static const std::string PIPENAME_OUT1;		///< Name Out-Pipe 1
	static const std::string PIPENAME_OUT2;		///< Name Out-Pipe 2

	/// Set filter parameters as defined in database / xml file.
	void setParameter();
	/// Draw the filter results.
	void paint();

protected:
	/// In-pipe registration.
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	/// In-pipe event processing.
	void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

private:

	const fliplib::SynchronePipe< interface::GeoVecDoublearray >* 	m_pPipeLineIn;		///< In pipe
	const fliplib::SynchronePipe< interface::GeoDoublearray >* 	m_pPipePosXIn;		///< In pipe
	const fliplib::SynchronePipe< interface::GeoDoublearray >* 	m_pPipePosYIn;		///< In pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >*       	m_pPipePosXOut;		///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >*       	m_pPipePosYOut;		///< Out pipe

	interface::SmpTrafo											m_oSpTrafo;			///< roi translation
    int                                                         m_oStartPos;        ///< Expected 'Start position' of the gap/seam
    int                                                         m_oStartDelta;      ///< Max diff 'actual' to 'Start position' in the first image
    bool m_useBadRankPoint;
    geo2d::DPointarray								  			m_oOut;				///< Output position
};

} // namespace precitec
} // namespace filter

#endif /* LINEPOS_H_ */
