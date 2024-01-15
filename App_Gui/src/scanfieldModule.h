#pragma once

#include "message/calibrationCoordinatesRequest.h"
#include "message/calibrationCoordinatesRequest.proxy.h"
#include "message/device.h"

#include <QObject>
#include <QSize>
#include <QUuid>

class ScanfieldModuleTest;
class ScanfieldSeamModelTest;
class ScanfieldSeamControllerTest;

namespace precitec
{

typedef std::shared_ptr<precitec::interface::TCalibrationCoordinatesRequest<precitec::interface::AbstractInterface>> CalibrationCoordinatesRequestProxy;

namespace gui
{

class DeviceProxyWrapper;

class ScanfieldModule : public QObject
{
    Q_OBJECT

    /**
     * Proxy to the camera calibration
     * Required to convert pixel information into actual distance
     **/
    Q_PROPERTY(precitec::CalibrationCoordinatesRequestProxy calibrationCoordinatesRequestProxy READ calibrationCoordinatesRequestProxy WRITE setCalibrationCoordinatesRequestProxy NOTIFY calibrationCoordinatesRequestProxyChanged)

    /**
     * Directory where the Scanfield Image of the current SeamSeries @link{seamSeries} is stored
     **/
    Q_PROPERTY(QString sourceImageDir READ sourceImageDir NOTIFY sourceImageDirChanged)

    /**
     * If the calibration configuration, read from @link{sourceImageDir} is complete
     **/
    Q_PROPERTY(bool configurationValid READ configurationValid NOTIFY configurationValidChanged)

    /**
     * Proxy to the grabber
     * Needed to acquire the actual camera size
     **/
    Q_PROPERTY(precitec::gui::DeviceProxyWrapper* grabberDeviceProxy READ grabberDeviceProxy WRITE setGrabberDeviceProxy NOTIFY grabberDeviceProxyChanged)

    /**
     * The current camera size, provided by the @link{grabberDeviceProxy}
     **/
    Q_PROPERTY(QSize cameraSize READ cameraSize NOTIFY cameraSizeChanged)

    /**
     * The size of the scanfield image file (in pixel), read from @link{sourceImageDir}
     * Provided by the image instance itself
     **/
    Q_PROPERTY(QSize imageSize READ imageSize WRITE setImageSize NOTIFY imageSizeChanged)

    /**
     * Whether the @link{cameraSize} is currently being loaded from the @link{grabberDeviceProxy}
     **/
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)

public:
    explicit ScanfieldModule(QObject* parent = nullptr);
    ~ScanfieldModule() override;

    CalibrationCoordinatesRequestProxy calibrationCoordinatesRequestProxy() const
    {
        return m_calibrationCoordinatesRequestProxy;
    }
    void setCalibrationCoordinatesRequestProxy(const CalibrationCoordinatesRequestProxy& proxy);

    QString sourceImageDir() const;

    bool configurationValid() const
    {
        return !m_calibrationConfig.empty();
    }

    DeviceProxyWrapper* grabberDeviceProxy() const
    {
        return m_grabberDeviceProxy;
    }
    void setGrabberDeviceProxy(DeviceProxyWrapper* device);

    QSize cameraSize() const
    {
        return m_cameraSize;
    }

    QSize imageSize() const
    {
        return m_imageSize;
    }
    void setImageSize(const QSize& size);

    bool centerValid(const QPointF& point) const;

    QRectF cameraRect(const QPointF& point) const;

    QUuid series() const
    {
        return m_series;
    }
    void setSeries(const QUuid& id);

    bool isLoading() const
    {
        return m_loading;
    }

    QRectF mirrorRect(const QRectF& rect) const;

    QPointF scannerToImageCoordinates(double x, double y) const;

    QPointF imageToScannerCoordinates(double x, double y) const;

    QPointF toValidCameraCenter(const QPointF& point) const;

    Q_INVOKABLE void loadCalibration();
    /**
     * Copies the scanfield image from the seam series identified by @p otherSeries
     * to the current @link{series}.
     *
     * Once copied @link{loadCalibration} is triggered.
     **/
    Q_INVOKABLE void copyFromOtherSeries(const QUuid& otherSeries);

Q_SIGNALS:
    void calibrationCoordinatesRequestProxyChanged();
    void sourceImageDirChanged();
    void configurationValidChanged();
    void grabberDeviceProxyChanged();
    void cameraSizeChanged();
    void imageSizeChanged();
    void seriesChanged();
    void loadingChanged();

private:
    void setCameraSize(const QSize& size);
    // for testing purposes
    void loadCalibrationFile(const QString& pathToSource);
    void clearConfiguration();
    void setLoading(bool loading);

    QUuid m_series;

    QSize m_cameraSize = {1280, 1024};
    QSize m_imageSize = {7168, 4096};

    CalibrationCoordinatesRequestProxy m_calibrationCoordinatesRequestProxy = nullptr;
    precitec::interface::Configuration m_calibrationConfig;

    DeviceProxyWrapper* m_grabberDeviceProxy = nullptr;
    QMetaObject::Connection m_grabberDeviceDestroyConnection;

    bool m_loading{false};

    friend ScanfieldModuleTest;
    friend ScanfieldSeamModelTest;
    friend ScanfieldSeamControllerTest;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::ScanfieldModule*)



