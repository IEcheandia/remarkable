#include "coordinates/linearMagnificationModel.h"
#include <tuple>
#include "module/moduleLogger.h"

namespace precitec
{

namespace coordinates
{
LinearMagnificationModel::LinearMagnificationModel()
{
    assert(!m_laserLineOnXYPlane.isValid());
}
LinearMagnificationModel::LinearMagnificationModel(double beta0, double betaZ,
                                                   bool invertX, bool invertY, bool isHighPlaneOnImageTop,
                                                   double dPix, geo2d::Point pointOrigin,
                                                    math::LineEquation laserLineOnXYPlane)
: m_pointOrigin{static_cast<float> ( pointOrigin.x ), static_cast<float> ( pointOrigin.y ) },
m_laserLineOnXYPlane{laserLineOnXYPlane},
m_x_toXmm{static_cast<float> ( (invertX? -1.0 : 1.0) * dPix / beta0 ) },
m_y_toYmm{static_cast<float> ( (invertY? 1.0 : -1.0) * dPix / beta0 ) },
m_laserY_toYmm{static_cast<float> ((isHighPlaneOnImageTop ? -1.0 : 1.0) * dPix / betaZ )}
{

}

void LinearMagnificationModel::serialize ( system::message::MessageBuffer &buffer ) const
{
    marshal(buffer, m_pointOrigin);
    marshal(buffer, m_laserLineOnXYPlane);
    marshal(buffer, m_x_toXmm);
    marshal(buffer, m_y_toYmm);
    marshal(buffer, m_laserY_toYmm);
}
void LinearMagnificationModel::deserialize( system::message::MessageBuffer const&buffer )
{
    deMarshal(buffer, m_pointOrigin);
    deMarshal(buffer, m_laserLineOnXYPlane);
    deMarshal(buffer, m_x_toXmm);
    deMarshal(buffer, m_y_toYmm);
    deMarshal(buffer, m_laserY_toYmm);
}

std::pair<bool, Coords3D> LinearMagnificationModel::laserScreenCoordinatesTo3D(int x_pix, int y_pix) const
{
    return laserScreenCoordinatesTo3D(geo2d::DPoint{static_cast<double>(x_pix), static_cast<double>(y_pix)});
}

std::pair<bool, Coords3D> LinearMagnificationModel::laserScreenCoordinatesTo3D(geo2d::DPoint pix) const
{
    if (!m_laserLineOnXYPlane.isValid())
    {
        return {false, {}};
    }
    bool valid=false;
    Coords2D coordsXYPlane;
    std::tie(valid, coordsXYPlane) = imageCoordinatesToGrayscaleImagePlane(pix);
    if (!valid)
    {
        return {false, {}};
    }

    auto referenceLine = m_laserLineOnXYPlane;
    geo2d::DPoint laserCoordinates = pix;
    transformToLaserLineAxis({&laserCoordinates}, {&referenceLine});

    assert(math::isClose(referenceLine.getInclinationDegrees(),0.0));
    assert(math::isClose(m_laserLineOnXYPlane.distance(pix.x, pix.y), referenceLine.distance(laserCoordinates.x, laserCoordinates.y), 1e-9));

    auto z = static_cast<float>(laserCoordinates.y) * m_laserY_toYmm;

    return {true, {coordsXYPlane.x, coordsXYPlane.y, z}};
}

std::pair<bool, Coords2D> LinearMagnificationModel::laserScreenCoordinatesToLaserPlane(int x_pix, int y_pix) const
{
    return laserScreenCoordinatesToLaserPlane(geo2d::DPoint{static_cast<double>(x_pix), static_cast<double>(y_pix)});
}

std::pair<bool, Coords2D> LinearMagnificationModel::laserScreenCoordinatesToLaserPlane(geo2d::DPoint pix) const
{
    if (!m_laserLineOnXYPlane.isValid())
    {
        return {false, {}};
    }
    geo2d::DPoint laserCoordinates = transformToLaserLineAxis(pix);
    auto z = static_cast<float>(laserCoordinates.y)* m_laserY_toYmm;
    auto laserY = geo2d::distance(geo2d::DPoint{laserCoordinates.x, z}, geo2d::DPoint{.0, .0});
    return {true, {static_cast<float> ( laserCoordinates.x ), static_cast<float> ( laserY ) }};
}

std::pair<bool, Coords2D> LinearMagnificationModel::imageCoordinatesToGrayscaleImagePlane(int x_pix, int y_pix) const
{
    return imageCoordinatesToGrayscaleImagePlane(geo2d::DPoint{static_cast<double>(x_pix), static_cast<double>(y_pix)});
}

std::pair<bool, Coords2D> LinearMagnificationModel::imageCoordinatesToGrayscaleImagePlane(geo2d::DPoint pix) const
{
    if (!m_laserLineOnXYPlane.isValid())
    {
        return {false, {}};
    }
    return {true, {static_cast<float>((pix.x - m_pointOrigin.x) * m_x_toXmm), static_cast<float>((pix.y - m_pointOrigin.y) * m_y_toYmm) }};
}

std::pair<bool, geo2d::DPoint> LinearMagnificationModel::distanceTCPmmToSensorCoord(Coords2D distance_mm, geo2d::DPoint tcp_sensorPix) const
{
    if (!m_laserLineOnXYPlane.isValid())
    {
        return {false, {}};
    }
    return {true, {distance_mm.x / m_x_toXmm + tcp_sensorPix.x, distance_mm.y / m_y_toYmm + tcp_sensorPix.y }};
}



math::LineEquation LinearMagnificationModel::getLaserLineAtZCollimatorHeight(double z_mm) const
{
    // corrdinate in the laser line reference system
    double laserY = z_mm / m_laserY_toYmm;
    math::LineEquation line{0.0, laserY}; //y = 0x + z_mm/laserYtomm

    transformToXYPlane({}, {&line});

    #ifndef NDEBUG
    {
        assert(math::isClose(line.getInclinationDegrees(), m_laserLineOnXYPlane.getInclinationDegrees()));
        geo2d::DPoint test0, test1;
        line.get2Points(test0.x, test0.y, test1.x, test1.y,500);
        for (auto testPoint : {test0, test1})
        {
            auto result = laserScreenCoordinatesTo3D(testPoint);
            assert(result.first && math::isClose<double>(result.second.z,z_mm, 1e-6));
        }
    }
#endif
    return line;
}

void LinearMagnificationModel::adjustLineToReferenceZ(geo2d::DPoint screenCoords_pix)
{
    m_laserLineOnXYPlane = math::LineEquation::getParallelLinePassingThroughPoint(m_laserLineOnXYPlane, screenCoords_pix.x, screenCoords_pix.y);
}

void LinearMagnificationModel::transformToAxis(const math::LineEquation laserLineOnXYPlane,
                                                      const std::vector<geo2d::DPoint *> & points,
                                                      const std::vector<math::LineEquation *> & lines)
{

    double u,v;
    laserLineOnXYPlane.getVector(u,v);
    const double cos_phi = u;
    const double sin_phi = -v;
    assert(math::isClose(cos_phi*cos_phi + sin_phi*sin_phi, 1.0));

    double x0,y0,x1,y1;
    laserLineOnXYPlane.get2Points(x0,y0,x1,y1, 512);
    const geo2d::DPoint offset{x0,y0};


    auto transformPoint = [&]( const geo2d::DPoint & point )
    {
        return geo2d::DPoint ( ( cos_phi * ( point.x - offset.x ) - sin_phi * ( point.y - offset.y )),
                               ( sin_phi * ( point.x - offset.x ) + cos_phi * ( point.y - offset.y)));

    };
    auto transformLine = [&transformPoint] ( const math::LineEquation & line ) {
        if (!line.isValid())
        {
            return line;
        }
        geo2d::DPoint pointA, pointB, pointC;
        line.get2Points ( pointA.x, pointA.y, pointB.x, pointB.y, 1000.0 );
        line.get2Points ( pointA.x, pointA.y, pointC.x, pointC.y, -1000.0 );
        auto pointANew = transformPoint(pointA);
        auto pointBNew = transformPoint(pointB);
        auto pointCNew = transformPoint(pointC);
        auto result =  math::LineEquation{std::vector<double>{pointANew.x, pointBNew.x, pointCNew.x},
                                  std::vector<double>{pointANew.y, pointBNew.y, pointCNew.y}, true};
        return result;
    };

    for (auto && line : lines)
    {
        *line = transformLine(*line);
    }

    for (auto && point : points)
    {
        *point = transformPoint(*point);
    }
}
void LinearMagnificationModel::transformToLaserLineAxis(const std::vector< precitec::geo2d::DPoint* >& points, const std::vector< precitec::math::LineEquation* >& lines) const
{
    transformToAxis(m_laserLineOnXYPlane, points, lines);
}

void LinearMagnificationModel::transformToXYPlane(const std::vector< precitec::geo2d::DPoint* >& points, const std::vector< precitec::math::LineEquation* >& lines) const
{

    double u,v;
    m_laserLineOnXYPlane.getVector(u,v);
    //inverse rotation respect to transformToLaserLineAxis theta = -phi
    const double cos_theta = u;
    const double sin_theta = v;
    assert(math::isClose( cos_theta*cos_theta + sin_theta*sin_theta, 1.0));

    double x0,y0,x1,y1;
    m_laserLineOnXYPlane.get2Points(x0,y0,x1,y1, 512);
    const geo2d::DPoint offset{x0,y0};


    auto transformPoint = [&]( const geo2d::DPoint & point )
    {
        return geo2d::DPoint ( ( cos_theta * point.x - sin_theta * point.y + offset.x),
                               ( sin_theta * point.x + cos_theta * point.y + offset.y));

    };
    auto transformLine = [&transformPoint] ( const math::LineEquation & line ) {
        if (!line.isValid())
        {
            return line;
        }
        geo2d::DPoint pointA, pointB, pointC;
        line.get2Points ( pointA.x, pointA.y, pointB.x, pointB.y, 1000.0 );
        line.get2Points ( pointA.x, pointA.y, pointC.x, pointC.y, -1000.0 );
        auto pointANew = transformPoint(pointA);
        auto pointBNew = transformPoint(pointB);
        auto pointCNew = transformPoint(pointC);
        auto result =  math::LineEquation{std::vector<double>{pointANew.x, pointBNew.x, pointCNew.x},
                                  std::vector<double>{pointANew.y, pointBNew.y, pointCNew.y}, true};
        return result;
    };

    for (auto && line : lines)
    {
        *line = transformLine(*line);
    }

    for (auto && point : points)
    {
        *point = transformPoint(*point);

    }
}

geo2d::DPoint LinearMagnificationModel::transformToLaserLineAxis(geo2d::DPoint point) const
{
    //important: point is passed by value
    transformToLaserLineAxis({&point}, {});
    return point;
}
geo2d::DPoint LinearMagnificationModel::transformToXYPlane(geo2d::DPoint point) const
{
    //important: point is passed by value
    transformToXYPlane({&point}, {});
    return point;
}
math::LineEquation LinearMagnificationModel::transformToLaserLineAxis(math::LineEquation line) const
{
    //important: point is passed by value
    transformToLaserLineAxis({}, {&line});
    return line;
}
math::LineEquation LinearMagnificationModel::transformToXYPlane(math::LineEquation line) const
{
    //important: point is passed by value
    transformToXYPlane({}, {&line});
    return line;
}


}
}
