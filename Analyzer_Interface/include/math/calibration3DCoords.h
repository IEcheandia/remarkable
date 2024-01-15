/**
* Class that represents the coordinates (in mm) on the laser plane. Since the transformation from the laser plane reference 
* system to the real world reference system is a rotation around x, the first coordinate on the laser plane corresponds
* to the x coordinate, and the second coordinate corresponds to ( z coordinate * cos (triang angle))
*/


#ifndef _CALIBRATION3DCOORDS_H
#define _CALIBRATION3DCOORDS_H


#include <functional>
#include <Analyzer_Interface.h>
#include <module/moduleLogger.h>

#include "math/3D/projectiveMathStructures.h" // Vec3d
#include "filter/parameterEnums.h" //filter::LaserLine
#include "geo/size.h"
#include "geo/coordinate.h"

#include "calibrationStructures.h"
#include "math/mathCommon.h"  //angleUnit
#include <image/image.h> //for coordsToCheckerBoardImage

#include "message/serializer.h"
#include "message/messageBuffer.h"
#include "common/geoContext.h"
#include "coordinates/linearMagnificationModel.h"


namespace precitec {

namespace math {

//forward declaration of friend class
class Calibration3DCoordsTransformer;

class ANALYZER_INTERFACE_API Calibration3DCoords : public system::message::Serializable
{
    
public:

    
	static bool isValidCoord(const float & x_plane, const float & y_plane);

	Calibration3DCoords();
    
    void serialize ( system::message::MessageBuffer &buffer ) const override;
    void deserialize( system::message::MessageBuffer const&buffer ) override;

    void completeInitialization (math::SensorModel pSensorModel);

	/*
	* @brief 2D to 3D method, internally used to to3D.
	* @param &p_rX3D   Returned X coord.
	* @param &p_rY3D	Returned Y coord.
	* @param &p_rZ3D	Returned Z coord.
	* @param p_oX		2D screen x coord.
	* @param p_oY		2D screen y coord.
	*
	* 2D to 3D. Currently NOT subpixel accurate. Returns false and (0,0,0) if no 3D coordinate is available.
	* Internally chooses Scheimpflug or Coax correctly using beta0 and betaZ of sensor p_oSensorID.
	*/

	bool convertScreenTo3D(float &p_rX3d, float &p_rY3d, float &p_rZ3d,
		const int p_oX, const int p_oY, filter::LaserLine p_oLaserLine) const;
	bool convertScreenToLaserPlane(float &p_rX2d, float &p_rY2d,
		const int p_oX, const int p_oY, filter::LaserLine p_oLaserLine) const;
	bool convertScreenToHorizontalPlane(float &p_rX2d, float &p_rY2d,
		const int p_oX, const int p_oY) const;  //no laser line
	
	bool to3D(float &p_rX3d, float &p_rY3d, float &p_rZ3d,
        const int p_oX, const int p_oY, filter::LaserLine p_oLaserLine) const;
    
    Vec3D to3D(const int p_oX, const int p_oY, filter::LaserLine p_oLaserLine) const;
    Vec3D to3D(const Vec3D &p_rCoord2D, filter::LaserLine p_oLaserLine) const;


	double dist(const double &p_oX3d1, const double &p_oY3d1, const double &p_oZ3d1,
			const double &p_oX3d2, const double &p_oY3d2, const double &p_oZ3d2) const;
	double distFrom2D(const int p_oX1, const int p_oY1, const int p_oX2, const int p_oY2) const;
    double distanceOnInternalPlane(const int & p_Xpix1, const int & p_Ypix1, const int & p_Xpix2, const int & p_Ypix2) const;



	/**
	 * @brief Set triangulation angle in radians 
	 *
	 * @param p_oAngle       Angle as double in [-180.0, 180.0]
	 * @param p_oIsRad       If p_oIsRad == eDegrees, p_oAngle will be scaled to [-pi, pi]
	 *
	 */
	 void setTriangulationAngle(const float p_oAngle, const angleUnit p_angleUnit,
		 const filter::LaserLine  oLaserLine);
     void setAllTriangulationAngles(const float p_oAngle, const angleUnit p_angleUnit);
	
	 float getTriangulationAngle(const angleUnit p_oAngleUnit,	const filter::LaserLine  oLaserLine) const;

	/*
	* @brief Rotate around Y-axis, performing a transformation from the calibrated coordinate system to the rotated one.
	* @param &p_rPointInCalibratedCoordinateSystem  3D point to be rotated.
	* @param &p_oBeta	Angle of rotation in rad.
	* @return			Result of rotation.
	*
	* Rotate around Y-axis, performing a transformation from the calibrated coordinate system to the rotated one.
	*/
	 static Vec3D FromCalibratedToRotated(const Vec3D& p_rPointInCalibratedCoordinateSystem, double p_oBeta);

	/*
	* @brief Rotate around Y-axis, performing a transformation from the rotated coordinate system to the calibrated one.
	* @param &p_rPointInCalibratedCoordinateSystem  3D point to be rotated.
	* @param &p_oBeta	Angle of rotation in rad.
	* @return			Result of rotation.
	*
	* Rotate around Y-axis, performing a transformation from the rotated coordinate system to the calibrated one.
	*/
	 static Vec3D FromRotatedToCalibrated(const Vec3D& p_rPointInRotatedCoordinateSystem, double p_oBeta);
	
	 enum class CoordinatePlaneMode {
		LineLaser1Plane, // corresponds to LaserLine::FrontLaserLine
		LineLaser2Plane, // corresponds LaserLine::BehindLaserLine
		LineLaser3Plane, // corresponds to LaserLine::CenterLaserLine (TCP)
		XYPlane, 
		InternalPlane
	 };

	 //debugging methods
	 /**
	 * Output 3D Data to an image of a checkerboard corresponding to the coordinates
	 */
    void coordsToCheckerBoardImage(image::BImage & pImage,
		 const geo2d::Point & pRoiOrigin, const geo2d::Size & pRoiSize,
		 const float & pSquareSide, const CoordinatePlaneMode  mode ) const;
         
         
    void coordsToMagnificationX(image::BImage & pImage,
		 const geo2d::Point & pRoiOrigin, const geo2d::Size & pRoiSize,
		 int p_oRadius) const;
         
    void coordsToMagnificationY(image::BImage & pImage,
		 const geo2d::Point & pRoiOrigin, const geo2d::Size & pRoiSize,
		 int p_oRadius) const;
         
         
	 /**
	 * Output 3D data to simple textfile that can be read opened with LibreCalc, Excel and similar apps. Simple ; separation.
	 */
	 void coordsToTable(const std::string & pFilenamePrefix, const geo2d::Point & pRoiOrigin, const geo2d::Size & pRoiSize) const;
	 //check monotonicity
	 std::vector<geo2d::coord2D> checkDiscontinuities(bool invalidCoordsAllowed = false,
	bool notContigousValidCoordinatesAllowed = false) const;

     ////approximate conversion from mm to pixel (in the coax case proportional to beta0) [pix/mm]
     double factorHorizontal(int pLength = 100, int pCenterX = 512, int pCenterY = 512) const;
     ////approximate conversion from mm (length 3D) to pixel in the vertical direction (in the coax case proportional to betaZ) [pix/mm]
     double factorVertical(int pLength, int pCenterX, int pCenterY, filter::LaserLine p_oLaserLine)  const;
     ////approximate conversion from mm (delta Z) to pixel in the vertical direction(in the coax case proportional to betaZ) [pix/mm]
     double factorVertical_Z(int pLength, int pCenterX, int pCenterY, filter::LaserLine p_oLaserLine)  const;
     ////distance between 2 points on the horizontal plane (in the coax case proportional to beta0)
     double distanceOnHorizontalPlane(int x0, int y0, int x1, int y1) const;
     ////approximate conversion from pixel to mm (in the coax case proportional to beta0) [mm/pix]
     double pixel_to_mm_OnHorizontalPlane(int pLength, int pCenterX, int pCenterY) const;

	 //getters
	 void getSensorSize(int &p_rWidth, int &p_rHeight) const;
	 geo2d::Size getSensorSize() const;
	 bool getCoordinates(float & x_plane, float & y_plane, const int x_pixel, const int y_pixel) const;

    bool usesOrientedLineCalibration() const;
    void resetOrientedLineCalibration();
    void setOrientedLineCalibration(filter::LaserLine line, coordinates::LinearMagnificationModel model);
    const coordinates::LinearMagnificationModel & getOrientedLineCalibration(filter::LaserLine line) const;
    void adjustZPointForOrientedLaserLine(filter::LaserLine line, geo2d::DPoint pix);

    void resetGridCellData(const int & pWidth, const int & pHeight, const math::SensorModel & pSensorModel);

    //get ref to x coordinate, no range checking
    float & X(unsigned int x_pixel, unsigned int y_pixel);
    //get ref to x coordinate, no range checking
    float & Y(unsigned int x_pixel, unsigned int y_pixel);
   	bool isScheimpflugCase() const;

    std::vector<geo2d::DPoint> distanceTCPmmToSensorCoordCoax(const std::vector<geo2d::DPoint>& contour_mm, double tcpSensorX, double tcpSensorY) const;
private:     

    typedef std::vector<float> coordsArray_t; 
    typedef std::map<filter::LaserLine, double> mapTriangAngles_t;
	/*
	* @brief Rotate around Y-axis
	* @param &p_rInput  3D point to be rotated.
	* @param &p_oBeta	Angle of rotation in rad.
	* @return			Result of rotation.
	*
	* Performs a rotation about the Y-axis.
	*/
	static Vec3D RotateY(const Vec3D& p_rInput, double p_oBeta);

    template<int t_coord>
    void coordsToMagnification(image::BImage & pImage,
		 const geo2d::Point & pRoiOrigin, const geo2d::Size & pRoiSize,
		 int p_oRadius) const;

	
    size_t getIndex(unsigned int x_pixel, unsigned int y_pixel) const;
	mapTriangAngles_t  m_oTriangAngles;  ///< Triangulation angle  in radian! (90 - (angle of rotation around x )
	coordsArray_t m_oCoordsArrayX; /// Coordinates in mm on a plane
	coordsArray_t m_oCoordsArrayY; /// Coordinates in mm on a plane

	size_t m_oSensorWidth;
	size_t m_oSensorHeight;
	math::SensorModel m_oSensorModel; //Scheimpflug or Coax give a different meaning to the internal plane

	std::map<filter::LaserLine, coordinates::LinearMagnificationModel> m_oLinearMagnificationModels;

	
	friend Calibration3DCoordsTransformer;
};

//utility class for quick conversion between image coordinates and 3d coordinates
class ANALYZER_INTERFACE_API ImageCoordsTo3DCoordsTransformer
{
public:
	   ImageCoordsTo3DCoordsTransformer(const math::Calibration3DCoords & r3DCoords, const interface::ImageContext & rContext, filter::LaserLine p_oLaserLine)
		: m_r3DCoords(r3DCoords),
		m_oTrafo_Offset_x(rContext.trafo()->dx()),
		m_oTrafo_Offset_y(rContext.trafo()->dy()),
		m_oHWROI_Offset_x(rContext.HW_ROI_x0),
		m_oHWROI_Offset_y(rContext.HW_ROI_y0),
		m_finalOffset_x(m_oTrafo_Offset_x + m_oHWROI_Offset_x),
		m_finalOffset_y(m_oTrafo_Offset_y + m_oHWROI_Offset_y),
		m_oLaserLine(p_oLaserLine),
		m_2DMeasurement(false),
		m_transposed(rContext.m_transposed)
	{};
    
    ImageCoordsTo3DCoordsTransformer(const math::Calibration3DCoords & r3DCoords, const interface::ImageContext & rContext)
		: m_r3DCoords(r3DCoords),
		m_oTrafo_Offset_x(rContext.trafo()->dx()),
		m_oTrafo_Offset_y(rContext.trafo()->dy()),
		m_oHWROI_Offset_x(rContext.HW_ROI_x0),
		m_oHWROI_Offset_y(rContext.HW_ROI_y0),
		m_finalOffset_x(m_oTrafo_Offset_x + m_oHWROI_Offset_x),
		m_finalOffset_y(m_oTrafo_Offset_y + m_oHWROI_Offset_y),
		m_oLaserLine(filter::LaserLine::NumberLaserLines), //invalid laser line
		m_2DMeasurement(true),
		m_transposed(rContext.m_transposed)
	{};


	const math::Calibration3DCoords & m_r3DCoords;
	const int m_oTrafo_Offset_x;
	const int m_oTrafo_Offset_y;
	const int m_oHWROI_Offset_x;
	const int m_oHWROI_Offset_y;
    const int m_finalOffset_x;
    const int m_finalOffset_y;
    const filter::LaserLine m_oLaserLine;
    const bool m_2DMeasurement; // do not use laser line
    const bool m_transposed;

    //TODO subpixel
    
	geo2d::Point getSensorPoint(int ImageX, int ImageY) const
	{
		return geo2d::Point(ImageX + m_finalOffset_x, ImageY + m_finalOffset_y);
	}
	
	geo2d::Point getCanvasPoint(int ImageX, int ImageY) const
	{
		return geo2d::Point(ImageX + m_oTrafo_Offset_x, ImageY + m_oTrafo_Offset_y);
	}
	
	math::Vec3D imageCoordTo3D(int ImageX, int ImageY) const
	{
        if (m_transposed)
        {
            wmLog(eError, "%s for transposed context not implemented\n", __FUNCTION__);
        }
		auto X = ImageX + m_finalOffset_x;
		auto Y = ImageY + m_finalOffset_y;
        if (m_2DMeasurement)
        {
            float p_rX2d, p_rY2d;
            m_r3DCoords.convertScreenToHorizontalPlane(p_rX2d, p_rY2d, X, Y);
            return {p_rX2d, p_rY2d, 0.0};
        }
		return m_r3DCoords.to3D(X, Y, m_oLaserLine);
	}
	
	std::vector<geo2d::DPoint> distanceTCPmmToImageCoordCoax(const std::vector<geo2d::DPoint>& contour_mm, double tcpSensorX, double tcpSensorY) const
	{
        if (m_r3DCoords.isScheimpflugCase())
        {
            wmLog(eWarning, "Not implemented \n");
            return {};
        }
        
        if (!m_2DMeasurement)
        {
            wmLog(eWarning, "Not implemented \n");
            return {};
        }
        if (m_transposed)
        {
            wmLog(eError, "%s for transposed context not implemented\n", __FUNCTION__);
        }
        auto oPointCoordinates = m_r3DCoords.distanceTCPmmToSensorCoordCoax(contour_mm, tcpSensorX, tcpSensorY);
        for (auto && rPoint : oPointCoordinates)
        {
            rPoint.x -= m_finalOffset_x;
            rPoint.y -= m_finalOffset_y;
        }
        return oPointCoordinates;
        
    }

};

}
}

#endif
