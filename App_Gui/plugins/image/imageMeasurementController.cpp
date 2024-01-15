#include "imageMeasurementController.h"
#include  <QDebug>

namespace precitec
{
namespace gui
{

using interface::SensorId;
using filter::LaserLine;

ImageMeasurementController::ImageMeasurementController(QObject *parent)
    : QObject(parent)
{
    connect(this, &ImageMeasurementController::laserLineChanged, this, &ImageMeasurementController::update);
    connect(this, &ImageMeasurementController::pointAChanged, this, &ImageMeasurementController::update);
    connect(this, &ImageMeasurementController::pointBChanged, this, &ImageMeasurementController::update);
    connect(this, &ImageMeasurementController::hwRoiChanged, this, &ImageMeasurementController::update);
    connect(this, &ImageMeasurementController::hwRoiChanged, this, &ImageMeasurementController::updateTCP);
    connect(this, &ImageMeasurementController::hwRoiChanged, this, &ImageMeasurementController::tcpValidChanged);
    connect(this, &ImageMeasurementController::tcpChanged, this, &ImageMeasurementController::tcpValidChanged);
    connect(this, &ImageMeasurementController::grayscaleImageChanged, this, &ImageMeasurementController::update);
    connect(this, &ImageMeasurementController::calibrationCoordinatesRequestProxyChanged, this, &ImageMeasurementController::updateTCP);
    connect(this, &ImageMeasurementController::tcpChanged, this, &ImageMeasurementController::update);

}

ImageMeasurementController::~ImageMeasurementController() = default;

void ImageMeasurementController::update()
{
    if (!isCalibrationInitialized())
    {
        return;
    }

    setCoordinatesA(compute3DCoordinate(m_pointA + m_hwRoi));
    setCoordinatesB(compute3DCoordinate(m_pointB + m_hwRoi));
    setCoordinatesTCP(compute3DCoordinate(m_tcp + m_hwRoi));
}

void ImageMeasurementController::setCoordinatesA(const QVector3D &a)
{
    if (m_coordinatesA == a)
    {
        return;
    }
    m_coordinatesA = a;
    emit coordinatesAChanged();
}

void ImageMeasurementController::setCoordinatesB(const QVector3D &b)
{
    if (m_coordinatesB == b)
    {
        return;
    }
    m_coordinatesB = b;
    emit coordinatesBChanged();
}

void ImageMeasurementController::setCoordinatesTCP(const QVector3D &coords)
{
    if (m_coordinatesTCP == coords)
    {
        return;
    }
    m_coordinatesTCP = coords;
    emit coordinatesTCPChanged();
}


QVector3D ImageMeasurementController::compute3DCoordinate(const QPoint &screen) const
{
    if (!isCalibrationInitialized())
    {
        return {};
    }

    if (m_grayscaleImage)
    {
        const auto coordinates = m_calibrationCoordinatesRequestProxy->getCoordinatesFromGrayScaleImage(screen.x(), screen.y(), m_ScannerInfo, m_sensorId);
        if (coordinates.x == -1000)
        {
            wmLog(eDebug,"Invalid coordinates in grayscale image %d %d \n", screen.x(), screen.y());
        }
        return QVector3D(coordinates.x, coordinates.y, 0.0);
    }

    if (m_laserLine >= 0 && (m_laserLine < (int)filter::LaserLine::NumberLaserLines))
    {
        const auto coordinates = m_calibrationCoordinatesRequestProxy->get3DCoordinates(screen.x(), screen.y(), m_sensorId, static_cast<filter::LaserLine> (m_laserLine));
        //Q_ASSERT(coordinates[0] != -1000);
        return QVector3D(coordinates[0], coordinates[1], coordinates[2]);
    }

    return {};
}

void ImageMeasurementController::setCalibrationCoordinatesRequestProxy(const CalibrationCoordinatesRequestProxy& proxy)
{
    if (m_calibrationCoordinatesRequestProxy == proxy)
    {
        return;
    }
    m_calibrationCoordinatesRequestProxy = proxy;

    emit calibrationCoordinatesRequestProxyChanged();
}

void ImageMeasurementController::setLaserLine(int line)
{
    if (m_laserLine == line)
    {
        return;
    }
    m_laserLine = line;
    emit laserLineChanged();
}

void ImageMeasurementController::setPointA(const QPoint &p)
{
    if (m_pointA == p)
    {
        return;
    }
    m_pointA = p;
    emit pointAChanged();
}

void ImageMeasurementController::setPointB(const QPoint &p)
{
    if (m_pointB == p)
    {
        return;
    }
    m_pointB = p;
    emit pointBChanged();
}

void ImageMeasurementController::setHwRoi(const QPoint &p)
{
    if (m_hwRoi == p)
    {
        return;
    }
    m_hwRoi = p;
    emit hwRoiChanged();
}

void ImageMeasurementController::setGrayscaleImage(bool set)
{
    if (m_grayscaleImage == set)
    {
        return;
    }
    m_grayscaleImage = set;
    emit grayscaleImageChanged();
}

void ImageMeasurementController::setHasScannerPosition(bool value)
{
    if (m_ScannerInfo.m_hasPosition == value)
    {
        return;
    }
    m_ScannerInfo.m_hasPosition = value;
    emit hasScannerPositionChanged();
}

void ImageMeasurementController::setScannerPosition(QPointF value)
{
    if (m_ScannerInfo.m_x == value.x() && m_ScannerInfo.m_y == value.y())
    {
        return;
    }
    m_ScannerInfo.m_x = value.x();
    m_ScannerInfo.m_y = value.y();
    emit scannerPositionChanged();
}


void ImageMeasurementController::reset()
{
    setPointA(QPoint{0, 0});
    setPointB(QPoint{0, 0});
    setGrayscaleImage(true);
}

void ImageMeasurementController::updateTCP()
{
    if (!m_calibrationCoordinatesRequestProxy)
    {
        return;
    }
    const auto tcp =  m_calibrationCoordinatesRequestProxy->getTCPPosition(m_sensorId, m_ScannerInfo); 
    const auto relativeTcp = QPoint(tcp.x, tcp.y) - m_hwRoi;

    if (relativeTcp.x() == m_tcp.x() && relativeTcp.y() == m_tcp.y())
    {
        return;
    }
    m_tcp = relativeTcp;
    emit tcpChanged();
}

}
}

