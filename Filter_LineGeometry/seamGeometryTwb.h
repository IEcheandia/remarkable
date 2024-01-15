/**
*  @file
*  @copyright	Precitec Vision GmbH & Co. KG
*  @author		Andreas Beschorner, Joachim Schwarz, Oliver Schulz, Christian Duchow
*  @date		2015
*  @ingroup    Filter_LineGeometry
*/

#ifndef SEAM_GEOMETRY_TWB_H_
#define SEAM_GEOMETRY_TWB_H_

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

namespace precitec {
	using namespace precitec::interface;
	namespace filter {

		using namespace precitec::math;

		class Line2D_
		{
		public:
			Line2D_(double slope, double yIntercept);
			Line2D_(double x, double y, double slope);
			Line2D_(double x1, double y1, double x2, double y2);

			double getSlope();
			double getYIntercept();
			double getY(double x);
			double getIsValid();
			double getIsVertical();

			double getOrthoSlope();
			double calcDistance(double x, double y);

			double getIntersectionX(Line2D_ otherLine);

			double m_oInterceptX;
			double m_oInterceptY;

		private:
			double m_slope;
			double m_yIntercept;
			bool m_isVertical;
			bool m_isValid;
		};

		/**
		*  @brief	 Computes seam run data height, length and surface.
		*
		*  @details  Finds (first) extremal point w.r.t. line segment bounded by markers and
		*  computes certain data of run such as height, length and surface.
		*
		* @param m_pPipeInLaserline   Laser line
		* @param m_pPipeInXLeft   X coordinate of left point
		* @param m_pPipeInXRight  X coordinate of right point
		* @param m_pPipeInAngle  Angle
		*/
		class FILTER_API SeamGeometryTwb : public fliplib::TransformFilter
		{
		public:

			/// CTor.
			SeamGeometryTwb();

			static const std::string m_oFilterName;   ///< Name of filter
			static const std::string PIPENAME_WIDTH; ///< Name of out pipe for width of seam
			//static const std::string PIPENAME_HEIGHT; ///< Name of out pipe for height of extremum wrt run baseline => raus, wird durch zwei Pipes fuer pos./neg. Roundness ersetzt
			static const std::string PIPENAME_ROUNDNESS_POS; ///< Name of out pipe for length of seam
			static const std::string PIPENAME_ROUNDNESS_NEG; ///< Name of out pipe for length of seam
			static const std::string PIPENAME_AREA_POS;   ///< Name of out pipe for surface of seam
			static const std::string PIPENAME_AREA_NEG; ///< Name of out pipe for orientation of seam
			static const std::string m_oParamTypeOfLaserLine;		///< Parameter: Taype of LaserLine (e.g. FrontLaserLine, BehindLaserLine)

			/// Sets filter parameters defined in database / xml file
			void setParameter();

			/// Paints overlay
			void paint();
			void arm(const fliplib::ArmStateBase& state);	///< arm filter

		protected:
			/// Integrate filter into pipe and filters pattern
			bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);

			/** @brief Find (first) extremal point w.r.t. line segment bounded by markers.
			*
			Given start- end end point of a bead/gap, this functions searches for the (first) extremal point
			of the run w.r.t the projection onto the line segment joining the two points and determines length, surface, signed height
			and orientation of the bead/gap.
			*/
			bool findExtremum(const geo2d::Doublearray &p_rLine);

			void signalSendInvalidResult(const ImageContext &p_rImgContext, ResultType p_oAnalysisResult);
			void resizeOutArrays(unsigned int size);
			void updateOutArrays(unsigned int lineN, bool p_oResultValid);

			/// Main processing routine invoked by <b>pipes and filter</b> pattern implementation
			void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg);

			bool get3DValues(const geo2d::Doublearray &p_rLine, const Vec3D &p_oStart, const Vec3D &p_oEnd);

			// pipes
			const fliplib::SynchronePipe< interface::GeoVecDoublearray > *m_pPipeInLaserline;  ///< In pipe laserline
			const fliplib::SynchronePipe< interface::GeoDoublearray >    *m_pPipeInXLeft;	   ///< In pipe xcoord left point
			const fliplib::SynchronePipe< interface::GeoDoublearray >    *m_pPipeInXRight;	   ///< In pipe xcoord right point
			const fliplib::SynchronePipe< interface::GeoDoublearray >    *m_pPipeInAngle;	   ///< In pipe angle

			fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutWidth;      ///< Out pipe proj. length (in pixel!)
			//fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutHeight;      ///< Out pipe proj. height (in pixel!)
			fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutRoundnessPos;      ///< Out pipe proj. height (in pixel!)
			fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutRoundnessNeg;      ///< Out pipe proj. height (in pixel!)
			fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutAreaPos;     ///< Out pipe ycoord extremum
			fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutAreaNeg;  ///< In pipe orientation (concave, convex, invalid)

			interface::SmpTrafo m_oSpTrafo;                  ///< Roi translation

			// for in pipes
			int m_oXLeft;
			int m_oXRight;                         ///< Start and end positions of bead/gap, xcoords left and right

			// for out pipes
			double m_oHeight;         ///< Seam bead/gap data the filter computes: Max. height, length and surface
			double m_oWidth;         ///< Seam bead/gap data the filter computes: Max. height, length and surface
			
			double m_oAreaNeg;
			double m_oAreaPos;

			double m_oRoundnessPos;
			double m_oRoundnessNeg;

			// for painting
			int m_oYLeft, m_oYRight;                         ///< For painting start and end of seam bead/gap
			std::vector<Vec2DHomogeneous> m_oYcoords;        ///< For painting the line segment between start and end of seam bead/gap

		private:
			void clipDouble(double & d, double lowerLimit, double upperLimit);
			bool m_oPaint;
			filter::LaserLine m_oTypeOfLaserLine;							///< which laser line and therefor which calibration data should be used?
			int m_oSwitchRoundness;

			int m_oXPosMax, m_oYPosMax, m_oXPosMaxLine, m_oYPosMaxLine;

			int m_oXPosRoundnessMax, m_oYPosRoundnessMax, m_oXPosRoundnessMaxLine, m_oYPosRoundnessMaxLine;
			int m_oXNegRoundnessMax, m_oYNegRoundnessMax, m_oXNegRoundnessMaxLine, m_oYNegRoundnessMaxLine;

			geo2d::Doublearray m_oWidthArray;
			geo2d::Doublearray m_oRoundnessPosArray;
			geo2d::Doublearray m_oRoundnessNegArray;
			geo2d::Doublearray m_oAreaPosArray;
			geo2d::Doublearray m_oAreaNegArray;
			double m_oGeoOutRank;
			int m_lastUpdatedArrayIndex; //only for debug

		};


	} // namespace precitec
} // namespace filter

#endif /* SEAM_GEOMETRY_TWB_H_ */
