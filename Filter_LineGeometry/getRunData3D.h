/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Andreas Beschorner (BA)
 *  @date		2013
 *  @ingroup    Filter_LineGeometry
 */

#ifndef GetRunData3D_H_
#define GetRunData3D_H_

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

#include "3D/projectiveMathStructures.h"
#include "math/calibration3DCoords.h"

namespace precitec {
	using namespace precitec::interface;
namespace filter {

	using namespace precitec::math;
/**
 *  @brief	 Computes seam run data height, length and surface.
 *
 *  @details  Finds (first) extremal point w.r.t. line segment bounded by markers and
 *  computes certain data of run such as height, length and surface.
 *
 * @param m_pPipeInXLeft   X coordinate of left point
 * @param m_pPipeInYLeft   Value of left point
 * @param m_pPipeInXRight  X coordinate of right point
 * @param m_pPipeInYRight  Value of right point
 */
class FILTER_API GetRunData3D  : public fliplib::TransformFilter
{
public:

	/// CTor.
	GetRunData3D();

	static const std::string m_oFilterName;   ///< Name of filter
	static const std::string PIPENAME_HEIGHT; ///< Name of out pipe for height of extremum wrt run baseline
	static const std::string PIPENAME_LENGTH; ///< Name of out pipe for length of run
	static const std::string PIPENAME_SURF;   ///< Name of out pipe for surface of run
	static const std::string PIPENAME_MID;  ///< Name of out pipe for start point of run

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
	bool findExtremum(const geo2d::VecDoublearray &p_rLine);

	void signalSend(const ImageContext &p_rImgContext, const int p_oIO);

	/// Main processing routine invoked by <b>pipes and filter</b> pattern implementation
	void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg);

    bool get3DValues(const geo2d::VecDoublearray &p_rLine, const Vec3D &p_oStart, const Vec3D &p_oEnd);

	// pipes
	const fliplib::SynchronePipe< interface::GeoDoublearray >  *m_pPipeInXLeft;	///< In pipe xcoord left point
	const fliplib::SynchronePipe< interface::GeoDoublearray >  *m_pPipeInXRight;	///< In pipe xcoord right point
	const fliplib::SynchronePipe< interface::GeoVecDoublearray >  *m_pPipeInLaserline;   ///< In pipe laserline
	const fliplib::SynchronePipe< interface::GeoDoublearray >  *m_pPipeInOrientation; ///< In pipe orientation (concave, convex, invalid)

	fliplib::SynchronePipe< interface::GeoDoublearray >         m_oPipeOutHeight;     ///< Out pipe proj. height (in pixel!)
	fliplib::SynchronePipe< interface::GeoDoublearray >         m_oPipeOutLength;     ///< Out pipe proj. length (in pixel!)
	fliplib::SynchronePipe< interface::GeoDoublearray >         m_oPipeOutSurface;    ///< Out pipe ycoord extremum
	fliplib::SynchronePipe< interface::GeoVecDoublearray >      m_oPipeOutRunMid; 	  ///< Out pipe (3D coord)
	
	interface::SmpTrafo m_oSpTrafo;                  ///< Roi translation
	std::unique_ptr<math::ImageCoordsTo3DCoordsTransformer> m_pCoordTransformer;///< converter between image coordinates and 3d coordinates 
	// for in pipes
	int m_oXLeft, m_oXRight;                         ///< Marker positions, xcoords left and right (start and end)
	RunOrientation m_oOrientation;                   ///< Orientation (convex, concave, invalid)

	// for out pipes
	Vec3D m_oMid3D;
	geo2d::VecDoublearray m_oMidArray;
	double m_oHeight, m_oLength, m_oSurface;  ///< Seam run data the filter computes: Max. height, length and surface

	// internal variables
	Vec2DHomogeneous m_oExtremum, m_oExtremumProj;   ///< Position of the extremal point and of its projection onto the line segment
	Vec2DHomogeneous m_oMid, m_oMidProj;

	// for painting
	int m_oYLeft, m_oYRight;        // for start and end of seam run
	std::vector<Vec2DHomogeneous> m_oYcoords;     // the line segment between start and end of run

private:
	bool m_oPaint;
	//parameter
	filter::LaserLine m_oTypeOfLaserLine;
};

} // namespace precitec
} // namespace filter

#endif /* GetRunData3D_H_ */
