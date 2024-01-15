#pragma once

#include "event/inspectionCmd.proxy.h"
#include "deviceProxyWrapper.h"

#include <QAbstractListModel>
#include <QRect>

#include <functional>

class QTimer;

namespace precitec
{

typedef std::shared_ptr<precitec::interface::TInspectionCmd<precitec::interface::EventProxy>> InspectionCmdProxy;

namespace gui
{

class CameraCalibrationModel : public QAbstractListModel
{
    Q_OBJECT

    /**
     * Inspection Command Proxy, needed to trigger the calibration
     **/
    Q_PROPERTY(precitec::InspectionCmdProxy inspectionCmdProxy READ inspectionCmdProxy WRITE setInspectionCmdProxy NOTIFY inspectionCmdProxyChanged)

    /**
     * Calibration Device Proxy, needed to acquire and update the calibration values
     **/
    Q_PROPERTY(precitec::gui::DeviceProxyWrapper* calibrationDeviceProxy READ calibrationDeviceProxy WRITE setCalibrationDeviceProxy NOTIFY calibrationDeviceProxyChanged)

    /**
     * Workflow Device Proxy, needed to acquire and update the Line Laser intensity
     **/
    Q_PROPERTY(precitec::gui::DeviceProxyWrapper* workflowDeviceProxy READ workflowDeviceProxy WRITE setWorkflowDeviceProxy NOTIFY workflowDeviceProxyChanged)

    /**
     * Grabber Device Proxy, needed to acquire the image roi size
     **/
    Q_PROPERTY(precitec::gui::DeviceProxyWrapper* grabberDeviceProxy READ grabberDeviceProxy WRITE setGrabberDeviceProxy NOTIFY grabberDeviceProxyChanged)

    /**
     * The model is marked as ready, when all of the device proxies are set and the init values successfully read
     **/
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)

    /**
     * The current image roi as reported by the camera/grabber
     **/
    Q_PROPERTY(QRect imageRoi READ imageRoi NOTIFY imageRoiChanged)

    /**
     * Roi coordinates for the drag functionality. Is set to the current Laser Line upon release.
     * Prevents multiple intermediate calls to the Calibration Device Proxy
     **/
    Q_PROPERTY(QRect preview READ preview WRITE setPreview NOTIFY previewChanged)

    /**
     * Whether it is possible to perform a Line Calibration (i.e. the Inspection Command Proxy is valid)
     **/
    Q_PROPERTY(bool canCalibrate READ canCalibrate NOTIFY inspectionCmdProxyChanged)

    /**
     * @c true while the calibration is running, otherwise @c false
     **/
    Q_PROPERTY(bool calibrating READ calibrating NOTIFY calibratingChanged)

    Q_PROPERTY(double targetWidth READ targetWidth WRITE setTargetWidth NOTIFY targetWidthChanged)
    Q_PROPERTY(double targetHeight READ targetHeight WRITE setTargetHeight NOTIFY targetHeightChanged)


public:
    explicit CameraCalibrationModel(QObject *parent = nullptr);
    ~CameraCalibrationModel() override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    InspectionCmdProxy inspectionCmdProxy() const
    {
        return m_inspectionCmdProxy;
    }
    void setInspectionCmdProxy(const InspectionCmdProxy& proxy);

    DeviceProxyWrapper* calibrationDeviceProxy() const
    {
        return m_calibrationDeviceProxy;
    }
    void setCalibrationDeviceProxy(DeviceProxyWrapper* device);

    DeviceProxyWrapper* workflowDeviceProxy() const
    {
        return m_workflowDeviceProxy;
    }
    void setWorkflowDeviceProxy(DeviceProxyWrapper* device);

    DeviceProxyWrapper* grabberDeviceProxy() const
    {
        return m_grabberDeviceProxy;
    }
    void setGrabberDeviceProxy(DeviceProxyWrapper* device);

    bool ready() const
    {
        return m_calibrationReady && m_workflowReady && m_grabberReady;
    }

    QRect imageRoi() const
    {
        return m_imageRoi;
    }

    QRect preview() const
    {
        return m_preview;
    }
    void setPreview(const QRect& preview);

    bool canCalibrate() const
    {
        return m_inspectionCmdProxy.get() != nullptr;
    }

    bool calibrating() const
    {
        return m_calibrating;
    }

    double targetWidth() const
    {
        return m_targetWidth;
    }
    void setTargetWidth(double value);

    double targetHeight() const
    {
        return m_targetHeight;
    }
    void setTargetHeight(double value);

    Q_INVOKABLE void startCalibration(int laserLine);
    Q_INVOKABLE void startChessboardCalibration();
    Q_INVOKABLE void startAngleCalibration();

    Q_INVOKABLE void endCalibration();

    Q_INVOKABLE void updateImageRoi();

    Q_INVOKABLE void setPreviewToRoi(int laserLine);

Q_SIGNALS:
    void inspectionCmdProxyChanged();
    void calibrationDeviceProxyChanged();
    void workflowDeviceProxyChanged();
    void grabberDeviceProxyChanged();
    void readyChanged();
    void imageRoiChanged();
    void previewChanged();
    void calibratingChanged();
    void targetWidthChanged();
    void targetHeightChanged();

private:
    void setCalibrationReady(bool ready);
    void setWorkflowReady(bool ready);
    void setGrabberReady(bool ready);
    void setCalibrating(bool calibrating);

    void updateCalibrationDevice(std::function<void()> updateFunction);
    void updateWorkflowDevice(std::function<void()> updateFunction);

    bool hasCalibrationPermission();
    bool hasWorkflowPermission();

    void updateRange();

    enum class LaserLine {
        One,
        Two,
        Three
    };

    bool setLeft(LaserLine line, int value);
    bool setTop(LaserLine line, int value);
    bool setRight(LaserLine line, int value);
    bool setBottom(LaserLine line, int value);
    bool setIntensity(LaserLine line, int value);
    bool setChecked(LaserLine line, bool value);

    struct LaserLineData {
        int left = 40;
        int right = 100;
        int top = 40;
        int bottom = 100;
        int intensity = 90;
        bool checked = true;

        QRect roi() const
        {
            return {left, top, right - left, bottom - top};
        }
    };

    bool m_calibrationReady = false;
    bool m_workflowReady = false;
    bool m_grabberReady = false;
    bool m_calibrating = false;
    bool m_calibrationUpdating = false;
    bool m_workflowUpdating = false;

    QRect m_imageRoi;
    QRect m_preview;
    std::map<LaserLine, LaserLineData> m_laserLineData;
    double m_targetWidth;
    double m_targetHeight;

    QTimer* m_calibrationTimeoutTimer;

    InspectionCmdProxy m_inspectionCmdProxy;

    DeviceProxyWrapper* m_calibrationDeviceProxy  = nullptr;
    QMetaObject::Connection m_calibrationDeviceDestroyed;

    DeviceProxyWrapper* m_workflowDeviceProxy = nullptr;
    QMetaObject::Connection  m_workflowDeviceDestroyConnection;

    DeviceProxyWrapper* m_grabberDeviceProxy = nullptr;
    QMetaObject::Connection m_grabberDeviceDestroyConnection;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::CameraCalibrationModel*)
