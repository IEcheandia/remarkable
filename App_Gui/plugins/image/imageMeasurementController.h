#pragma once

#include  "message/calibrationCoordinatesRequest.h"
#include  "filter/parameterEnums.h"
#include  "message/calibrationCoordinatesRequest.proxy.h"

#include <QObject>
#include <QPoint>
#include <QVector3D>

namespace precitec
{

typedef std::shared_ptr<precitec::interface::TCalibrationCoordinatesRequest<precitec::interface::AbstractInterface>> CalibrationCoordinatesRequestProxy;

namespace gui
{

class ImageMeasurementController: public QObject
{
    Q_OBJECT

    Q_PROPERTY(precitec::CalibrationCoordinatesRequestProxy calibrationCoordinatesRequestProxy READ calibrationCoordinatesRequestProxy WRITE setCalibrationCoordinatesRequestProxy NOTIFY calibrationCoordinatesRequestProxyChanged)

    Q_PROPERTY(bool calibrationInitialized READ isCalibrationInitialized NOTIFY calibrationCoordinatesRequestProxyChanged)

    Q_PROPERTY(int laserLine READ laserLine WRITE setLaserLine NOTIFY laserLineChanged)

    Q_PROPERTY(bool grayscaleImage READ grayscaleImage WRITE setGrayscaleImage NOTIFY grayscaleImageChanged)

    Q_PROPERTY(QPoint pointA READ pointA WRITE setPointA NOTIFY pointAChanged)

    Q_PROPERTY(QPoint pointB READ pointB WRITE setPointB NOTIFY pointBChanged)

    Q_PROPERTY(QPoint hwRoi READ hwRoi WRITE setHwRoi NOTIFY hwRoiChanged)

    Q_PROPERTY(QPoint tcp READ tcp NOTIFY tcpChanged)

    Q_PROPERTY(bool tcpValid READ tcpValid NOTIFY tcpValidChanged)

    Q_PROPERTY(QVector3D coordinatesA READ coordinatesA NOTIFY coordinatesAChanged)

    Q_PROPERTY(QVector3D coordinatesB READ coordinatesB NOTIFY coordinatesBChanged)    
    
    Q_PROPERTY(QVector3D coordinatesTCP READ coordinatesTCP NOTIFY coordinatesTCPChanged)    
    
    
    Q_PROPERTY(bool hasScannerPosition READ hasScannerPosition WRITE setHasScannerPosition  NOTIFY hasScannerPositionChanged)
    
    Q_PROPERTY(QPointF scannerPosition READ scannerPosition WRITE setScannerPosition NOTIFY scannerPositionChanged)

public:
    explicit ImageMeasurementController(QObject *parent = nullptr);
    ~ImageMeasurementController() override;

    CalibrationCoordinatesRequestProxy calibrationCoordinatesRequestProxy() const
    {
        return m_calibrationCoordinatesRequestProxy;
    }
    void setCalibrationCoordinatesRequestProxy(const CalibrationCoordinatesRequestProxy &proxy);

    bool isCalibrationInitialized() const
    {
        return calibrationCoordinatesRequestProxy().get();
    }

    int laserLine() const
    {
        return m_laserLine;
    }
    void setLaserLine(int line);

    QPoint pointA() const
    {
        return m_pointA;
    }
    void setPointA(const QPoint &p);

    QPoint pointB() const
    {
        return m_pointB;
    }
    void setPointB(const QPoint &p);

    QPoint hwRoi() const
    {
        return m_hwRoi;
    }
    void setHwRoi(const QPoint &p);

    bool grayscaleImage() const
    {
        return m_grayscaleImage;
    }
    void setGrayscaleImage(bool set);

    QPoint tcp() const
    {
        return m_tcp;
    }

    bool tcpValid() const
    {
        return (m_tcp.x() >= 0) && (m_tcp.y() >= 0);
    }

    QVector3D coordinatesA() const
    {
        return m_coordinatesA;
    }

    QVector3D coordinatesB() const
    {
        return m_coordinatesB;
    }
    QVector3D coordinatesTCP() const
    {
        return m_coordinatesTCP;
    }
    
    bool hasScannerPosition() const
    {
        return m_ScannerInfo.m_hasPosition;
    }
    QPointF scannerPosition() const
    {
        return {m_ScannerInfo.m_x, m_ScannerInfo.m_y};
    }
    
    void setHasScannerPosition(bool value);
    void setScannerPosition(QPointF value);
    
    Q_INVOKABLE void reset();

    Q_INVOKABLE void updateTCP();

Q_SIGNALS:
    void calibrationCoordinatesRequestProxyChanged();
    void laserLineChanged();
    void pointAChanged();
    void pointBChanged();
    void hwRoiChanged();
    void grayscaleImageChanged();
    void tcpChanged();
    void tcpValidChanged();
    void coordinatesAChanged();
    void coordinatesBChanged();
    void coordinatesTCPChanged();
    void hasScannerPositionChanged();
    void scannerPositionChanged();

private:
    void update();
    QVector3D compute3DCoordinate(const QPoint &screen) const;
    void setCoordinatesA(const QVector3D &a);
    void setCoordinatesB(const QVector3D &b);
    void setCoordinatesTCP(const QVector3D &coords);

    QPoint m_pointA = QPoint{0, 0};
    QPoint m_pointB = QPoint{0, 0};
    QPoint m_hwRoi = QPoint{0, 0};
    QPoint m_tcp = QPoint{0, 0};

    QVector3D m_coordinatesA = QVector3D{0, 0, 0};
    QVector3D m_coordinatesB = QVector3D{0, 0, 0};
    QVector3D m_coordinatesTCP = QVector3D{0, 0, 0};

    bool m_grayscaleImage = true;
    int m_laserLine = (int)(filter::LaserLine::FrontLaserLine);

    const interface::SensorId m_sensorId = interface::SensorId::eSensorId0;
    CalibrationCoordinatesRequestProxy m_calibrationCoordinatesRequestProxy = nullptr;
    interface::ScannerContextInfo m_ScannerInfo;
};

}
}

Q_DECLARE_METATYPE(precitec::CalibrationCoordinatesRequestProxy)
