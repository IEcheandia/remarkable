/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Andreas Beschorner (BA)
 *  @date		2014
 *  @ingroup    Filter_LineGeometry
 */

#ifndef SeamData3D_H_
#define SeamData3D_H_

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
class FILTER_API SeamData3D  : public fliplib::TransformFilter
{
public:

	/// CTor.
	SeamData3D();

	static const std::string m_oFilterName;   ///< Name of filter
	static const std::string PIPENAME_HEIGHT; ///< Name of out pipe for height of extremum wrt run baseline
	static const std::string PIPENAME_LENGTH; ///< Name of out pipe for length of seam
	static const std::string PIPENAME_SURF;   ///< Name of out pipe for surface of seam
	static const std::string PIPENAME_ORIENT; ///< Name of out pipe for orientation of seam
	static const std::string m_oParamTypeOfLaserLine;		///< Parameter: Taype of LaserLine (e.g. FrontLaserLine, BehindLaserLine)

	/// Sets filter parameters defined in database / xml file
	void setParameter();

	/// Paints overlay
	void paint();

protected:
	/// Integrate filter into pipe and filters pattern
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);

	/** @brief Find (first) extremal point w.r.t. line segment bounded by markers.
	 *
	   Given start- end end point of a bead/gap, this functions searches for the (first) extremal point
	   of the run w.r.t the projection onto the line segment joining the two points and determines length, surface, signed height
	   and orientation of the bead/gap.
	   */
	bool findExtremum(const geo2d::VecDoublearray &p_rLine, const math::Calibration3DCoords &p_rCalib);

	void signalSend(const ImageContext &p_rImgContext, const int p_oIO);
	void signalSendInvalidResult(const ImageContext &p_rImgContext, ResultType p_oAnalysisResult);
	

	/// Main processing routine invoked by <b>pipes and filter</b> pattern implementation
	void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg);

    bool get3DValues(const geo2d::VecDoublearray &p_rLine, const Vec3D &p_oStart, const Vec3D &p_oEnd, const math::Calibration3DCoords &p_rCalib);

	// pipes
	const fliplib::SynchronePipe< interface::GeoDoublearray >    *m_pPipeInXLeft;	   ///< In pipe xcoord left point
	const fliplib::SynchronePipe< interface::GeoDoublearray >    *m_pPipeInXRight;	   ///< In pipe xcoord right point
	const fliplib::SynchronePipe< interface::GeoVecDoublearray > *m_pPipeInLaserline;  ///< In pipe laserline

	fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutHeight;      ///< Out pipe proj. height (in pixel!)
	fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutLength;      ///< Out pipe proj. length (in pixel!)
	fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutSurface;     ///< Out pipe ycoord extremum
	fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutOrient;  ///< In pipe orientation (concave, convex, invalid)
	
	interface::SmpTrafo m_oSpTrafo;                  ///< Roi translation

	// for in pipes
	int m_oXLeft, m_oXRight;                         ///< Start and end positions of bead/gap, xcoords left and right

	// for out pipes
	Vec3D m_oMid3D;
	RunOrientation m_oOrientation;                   ///< Orientation (convex, concave, invalid)
	double m_oHeight, m_oLength, m_oSurface;         ///< Seam bead/gap data the filter computes: Max. height, length and surface

	// internal variables
	Vec2DHomogeneous m_oExtremum, m_oExtremumProj;   ///< Position of the extremal point and of its projection onto the line segment

	// for painting
	int m_oYLeft, m_oYRight;                         ///< For painting start and end of seam bead/gap
	std::vector<Vec2DHomogeneous> m_oYcoords;        ///< For painting the line segment between start and end of seam bead/gap


private:
	void clipDouble(double & d, double lowerLimit, double upperLimit);
	bool m_oPaint;
	filter::LaserLine m_oTypeOfLaserLine;							///< which laser line and therefor which calibration data should be used?
	int m_oXPosMax, m_oYPosMax;
	int m_oXPosMaxLine, m_oYPosMaxLine;
};


} // namespace precitec
} // namespace filter

#endif /* GetRunData3D_H_ */
