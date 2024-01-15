#pragma once
#include "geo/point.h"
#include "math/2D/LineEquation.h"
#include "message/serializer.h"
#include "message/messageBuffer.h"
#include <utility>

namespace precitec
{

namespace coordinates
{

struct Coords3D
{
    float x,y,z;
};

struct Coords2D
{
    float x,y;
};

//TODO container for imageCoordinatesToGrayscaleImagePlane independent from laser line

class LinearMagnificationModel : public system::message::Serializable
{
public:
    LinearMagnificationModel();
    LinearMagnificationModel(double beta0, double betaZ, bool invertX, bool invertY, bool isHighPlaneOnImageTop,
                             double dPix, geo2d::Point pointOrigin, math::LineEquation laserLineOnXYPlane);
    void serialize ( system::message::MessageBuffer &buffer ) const override;
    void deserialize( system::message::MessageBuffer const&buffer ) override;

    std::pair<bool, Coords3D> laserScreenCoordinatesTo3D(int x_pix, int y_pix) const;
    std::pair<bool, Coords2D> laserScreenCoordinatesToLaserPlane(int x_pix, int y_pix) const;
    std::pair<bool, Coords2D> imageCoordinatesToGrayscaleImagePlane(int x_pix, int y_pix) const;
    //subpixel precision
    std::pair<bool, Coords3D> laserScreenCoordinatesTo3D(geo2d::DPoint pix) const;
    std::pair<bool, Coords2D> laserScreenCoordinatesToLaserPlane(geo2d::DPoint pix) const;
    std::pair<bool, Coords2D> imageCoordinatesToGrayscaleImagePlane(geo2d::DPoint pix) const;
    std::pair<bool, geo2d::DPoint> distanceTCPmmToSensorCoord(Coords2D distance_mm, geo2d::DPoint tcp_sensorPix) const;

    math::LineEquation getLaserLineAtZCollimatorHeight(double z_mm) const;
    void adjustLineToReferenceZ(geo2d::DPoint screenCoords_pix);

    static void transformToAxis(math::LineEquation laserLineOnXYPlane, const std::vector<geo2d::DPoint *> & points, const std::vector<math::LineEquation*> & lines);
    void transformToLaserLineAxis(const std::vector<geo2d::DPoint *> & points, const std::vector<math::LineEquation*> & lines) const;
    void transformToXYPlane(const std::vector<geo2d::DPoint *> & points, const std::vector<math::LineEquation*> & lines) const;
    geo2d::DPoint transformToLaserLineAxis(geo2d::DPoint point) const;
    geo2d::DPoint transformToXYPlane(geo2d::DPoint point) const;
    math::LineEquation transformToLaserLineAxis(math::LineEquation line) const;
    math::LineEquation transformToXYPlane(math::LineEquation line) const;
private:
    Coords2D m_pointOrigin = {0.0, 0.0};
    math::LineEquation m_laserLineOnXYPlane; //to transform x,y screen coordinates to u,v laser line coordinates
    float m_x_toXmm  = 0.0;
    float m_y_toYmm = 0.0;
    float m_laserY_toYmm = 0.0;
};


}
}
