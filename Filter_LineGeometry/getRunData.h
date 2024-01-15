/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Andreas Beschorner (BA)
 *  @date		2012 - 2013
 *  @ingroup    Filter_LineGeometry
 */

#ifndef getRunData_H_
#define getRunData_H_

#include <vector>

#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include <image/image.h>				///< BImage
#include "filter/parameterEnums.h"		///< enum ExtremumType

#include <common/frame.h>				///< ImageFrame
#include <geo/geo.h>					///< Size2d, VecDoublearray
#include <geo/array.h>					///< ByteArray

namespace precitec {
namespace filter {

/**
 *  @brief	 Computes seam run data height, length and surface.
 *
 *  @details  Finds (first) extremal point w.r.t. line segment bounded by markers and
 *  computes certain data of run such as height, length and surface.
 */
class FILTER_API GetRunData  : public fliplib::TransformFilter
{
public:

	/// CTor.
	GetRunData();

	static const std::string m_oFilterName;   ///< Name of filter
	static const std::string PIPENAME_HEIGHT; ///< Name of out pipe for height of extremum wrt run baseline
	static const std::string PIPENAME_LENGTH; ///< Name of out pipe for length of run
	static const std::string PIPENAME_SURF;   ///< Name of out pipe for surface of run

	/// Sets filter parameters defined in database / xml file
	void setParameter();

	/// Paints overlay
	void paint();

protected:
	/// Integrate filter into pipe and filters pattern
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);

	/** @brief Find (first) extremal point w.r.t. line segment bounded by markers.

	   Given the line segment between the left and right marker, this functions searches for the (first) extremal point
	   of the run w.r.t the projection onto this line segment.\n
	   Given the point \f$ q \f$ tp project, the segment's intercept \f$o\f$ and
	   the left and right boundign points \f$p_l, p_r\f$ we first compute the directional vector \f$ u=p_r-p_l\f$ of the line segment.\n
	   Letting \f$ \langle \cdot,\cdot \rangle \f$ denote the standard euclidean inner product, the projection \f$ P(q) \f$ is then computed via\n
	   \f[ P(q)=o+\underbrace{\frac{ \langle (q-o), u\rangle }{ \langle u, u\rangle }}_a\cdot u. \f]
	   Addition with \f$o\f$ and scalar multiplication with \f$a\f$ are vector-vector and scalar-vector operations.
	   The point \f$q\f$ with maximal length is returned.
	   Currently, the routine does not consider the orientation (concavitiy, convexity) here.\n
	   While searching for the extremal point, the area between the markers is computed simultaneously.
	   */
	bool findExtremum(const geo2d::VecDoublearray &p_rLine, const RunOrientation p_oOrientation);

	/// Main processing routine invoked by <b>pipes and filter</b> pattern implementation
	void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg);

	// pipes
	const fliplib::SynchronePipe< interface::GeoVecDoublearray >  *m_pPipeInLaserline;   ///< In pipe laserline
	const fliplib::SynchronePipe< interface::GeoDoublearray >  *m_pPipeInOrientation; ///< In pipe orientation (concave, convex, invalid)
	const fliplib::SynchronePipe< interface::GeoDoublearray >  *m_pPipeInXLeft;       ///< In pipe xcoord left point (left marker)
	const fliplib::SynchronePipe< interface::GeoDoublearray >  *m_pPipeInXRight;      ///< In pipe xcoord right point (right amrker)
	const fliplib::SynchronePipe< interface::GeoDoublearray >  *m_pPipeInSlope;       ///< In pipe slope
	const fliplib::SynchronePipe< interface::GeoDoublearray >  *m_pPipeInIntercept;   ///< In pipe intercept

	fliplib::SynchronePipe< interface::GeoDoublearray >         m_oPipeOutHeight;     ///< Out pipe proj. height (in pixel!)
	fliplib::SynchronePipe< interface::GeoDoublearray >         m_oPipeOutLength;     ///< Out pipe proj. length (in pixel!)
	fliplib::SynchronePipe< interface::GeoDoublearray >         m_oPipeOutSurface;    ///< Out pipe ycoord extremum
	
	interface::SmpTrafo m_oSpTrafo;                  ///< Roi translation

	RunOrientation m_oOrientation;                   ///< Orientation (convex, concave, invalid)
	double m_oXLeft, m_oXRight, m_oYLeft, m_oYRight; ///< Marker positions, left and right
	double m_oSlope; double m_oIntercept;            ///< Slope and intercept of line segment determined by markers

	double m_oXPos, m_oYPos;                         ///< Position of the (first) extremal point of the run bounded by markers
	double m_oXProj, m_oYProj;                       ///< Position of the projection of the extremal point onto the line segment
	double m_oHeight, m_oLength, m_oSurface;         ///< Seam run data the filter computes: Max. eight, length and surface
};

} // namespace precitec
} // namespace filter

#endif /* getRunData_H_ */
