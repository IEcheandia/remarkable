/**
 *  @file       showCalibrationGrid.h
 *  @ingroup    Filter_Calibration
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		LB
 *  @date		2017
 *  @brief		Show calibration grid and settings
 */


#ifndef TESTCALIBRATIONGRID_H
#define TESTCALIBRATIONGRID_H


#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include <common/frame.h>				///< ImageFrame
#include <image/image.h>				///< BImage
#include <geo/geo.h>					///< Size2d, VecDoublearray
#include <geo/array.h>					///< ByteArray
#include <geo/rect.h>
#include "util/calibDataSingleton.h"


namespace precitec {
namespace filter {

	using geo2d::Point;
    using geo2d::Rect;
    using image::BImage;

	class FILTER_API TestCalibrationGrid  : public fliplib::TransformFilter
	{
	public:

		static const std::string m_oFilterName;
		static const std::string m_oPipeXName;

		/// CTor.
		TestCalibrationGrid();
		/// DTor.
		virtual ~TestCalibrationGrid();

		/// Set filter parameters as defined in database / xml file.
		void setParameter();
		void paint();

	protected:
		bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);

		/// In-pipe event processing.
		void proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rEventArg);


        void arm(const fliplib::ArmStateBase& p_rArmstate);

	private:
		enum PointsElement{
			eTopLeft, eTopRight, eBottomLeft, eBottomRight, eCenter, eNUMPOINTS
		};
		enum PointIndex{
			     eX, eY, eZ
		};
		typedef std::array<float, 3> tPhysicalCoord; //indexed by PointIndex

		const fliplib::SynchronePipe< interface::ImageFrame >*	m_pPipeInImageFrame;    ///< in pipe
		interface::SmpTrafo	m_oSpTrafo;			///< roi translation
		fliplib::SynchronePipe<interface::GeoDoublearray>		m_oPipeOutCoordX;

		const precitec::math::SensorId m_oSensorId;
		const filter::LaserLine m_oLaserLine;

		std::array<Point, eNUMPOINTS> m_oImagePoints;
		std::array<Point, eNUMPOINTS> m_oSensorPoints;
		std::array<tPhysicalCoord, eNUMPOINTS> m_oPointsTriangulation;
		std::array<tPhysicalCoord, eNUMPOINTS> m_oPointsInternalPlane;
		std::vector<geo2d::coord2D> m_oDiscontinuityPoints;
		bool mCalibrationChecked;

		BImage m_oCheckerboard;
		BImage m_oImageOut;
		geo2d::TPoint<int>	m_oHwRoi;


		double m_oCheckerboardSide;


	};

} // namespace filter
} // namespace precitec

#endif /* TESTCALIBRATIONGRID_H */

