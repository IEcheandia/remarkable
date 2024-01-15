#pragma once

#include "event/inspectionCmd.proxy.h"

#include <QAbstractListModel>
#include "deviceProxyWrapper.h"

class QTimer;

namespace precitec
{
typedef std::shared_ptr<precitec::interface::TInspectionCmd<precitec::interface::EventProxy>> InspectionCmdProxy;

namespace gui
{

class LEDChannel;


    
class LEDCalibrationController : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(precitec::InspectionCmdProxy inspectionCmdProxy READ inspectionCmdProxy WRITE setInspectionCmdProxy NOTIFY inspectionCmdProxyChanged)

    Q_PROPERTY(precitec::gui::DeviceProxyWrapper *calibrationDevice READ calibrationDevice WRITE setCalibrationDevice NOTIFY calibrationDeviceChanged)

    Q_PROPERTY(precitec::gui::DeviceProxyWrapper *serviceDevice READ serviceDevice WRITE setServiceDevice NOTIFY serviceDeviceChanged)

    Q_PROPERTY(precitec::gui::DeviceProxyWrapper *weldHeadDevice READ weldHeadDevice WRITE setWeldHeadDevice NOTIFY weldHeadDeviceChanged)

    Q_PROPERTY(precitec::gui::DeviceProxyWrapper *grabberDevice READ grabberDevice WRITE setGrabberDevice NOTIFY grabberDeviceChanged)
    /**
     * Whether it is possible to perform a LED Calibration
     **/
    Q_PROPERTY(bool canCalibrate READ canCalibrate NOTIFY inspectionCmdProxyChanged)
    /**
     * @c true while performing the OCT-Line Calibration, otherwise @c false
     **/
    Q_PROPERTY(bool calibrating READ isCalibrating NOTIFY calibratingChanged)

    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)

    Q_PROPERTY(int ledType READ ledType NOTIFY ledTypeChanged)

public:
    explicit LEDCalibrationController(QObject *parent = nullptr);
    ~LEDCalibrationController() override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    InspectionCmdProxy inspectionCmdProxy() const
    {
        return m_inspectionCmdProxy;
    }
    void setInspectionCmdProxy(const InspectionCmdProxy &proxy);

    DeviceProxyWrapper *calibrationDevice() const
    {
        return m_calibrationDevice;
    }
    void setCalibrationDevice(DeviceProxyWrapper *device);

    DeviceProxyWrapper *serviceDevice() const
    {
        return m_serviceDevice;
    }
    void setServiceDevice(DeviceProxyWrapper *device);

    DeviceProxyWrapper *weldHeadDevice() const
    {
        return m_weldHeadDevice;
    }
    void setWeldHeadDevice(DeviceProxyWrapper *device);

    DeviceProxyWrapper *grabberDevice() const
    {
        return m_grabberDevice;
    }
    void setGrabberDevice(DeviceProxyWrapper *device);

    bool canCalibrate() const
    {
        return inspectionCmdProxy().get();
    }

    bool isCalibrating() const
    {
        return m_calibrating;
    }

    bool isCalibrationReady() const
    {
        return m_calibrationReady;
    }

    bool isServiceReady() const
    {
        return m_serviceReady;
    }

    bool isWeldHeadReady() const
    {
        return m_weldHeadReady;
    }

    bool isGrabberReady() const
    {
        return m_grabberReady;
    }

    bool isCalibrationUpdating() const
    {
        return m_calibrationUpdating;
    }

    bool isWeldHeadUpdating() const
    {
        return m_weldHeadUpdating;
    }

    bool isGrabberUpdating() const
    {
        return m_grabberUpdating;
    }

    bool isActive() const
    {
        return m_active;
    }
    void setActive(bool set);

    int ledType() const
    {
        return m_ledType;
    }

    /**
     * Starts the LED calibration process
     * @see calibrating
     **/
    Q_INVOKABLE void startLEDCalibration();

    /**
     * Tells this controller that the LED calibration has ended
     **/
    Q_INVOKABLE void endLEDCalibration();

    /**
     * Turns Channel @p index on and off
     **/
    Q_INVOKABLE void updateEnabled(int index, bool enabled);

    /**
     * Updates the current value of Channel @p index
     **/
    Q_INVOKABLE void updateCurrentValue(int index, int currentValue);

    /**
     * Updates the reference brightness of Channel @p index
     **/
    Q_INVOKABLE void updateReferenceBrightness(int index, int currentValue);

Q_SIGNALS:
    void inspectionCmdProxyChanged();
    void calibrationDeviceChanged();
    void serviceDeviceChanged();
    void weldHeadDeviceChanged();
    void grabberDeviceChanged();
    void calibratingChanged();
    void calibrationReadyChanged();
    void serviceReadyChanged();
    void weldHeadReadyChanged();
    void grabberReadyChanged();
    void updatingChanged();
    void activeChanged();
    void ledTypeChanged();

private:
    bool hasCalibrationPermission();
    bool hasWeldHeadPermission();
    bool hasGrabberPermission();
    void setCalibrationReady(bool set);
    void setServiceReady(bool set);
    void setWeldHeadReady(bool set);
    void setGrabberReady(bool set);
    void setCalibrationUpdating(bool set);
    void setWeldHeadUpdating(bool set);
    void setGrabberUpdating(bool set);
    void setCalibrating(bool set);
    void updateCalibrationDevice(std::function<void()> updateFunction);
    void updateWeldHeadDevice(std::function<void()> updateFunction);
    void updateGrabberDevice(std::function<void()> updateFunction);
    void initCurrentValues();
    void setCameraBrightness();
    void loadCalibrationResults();
    void turnLEDChannelsOff();
    bool hasLEDEnabled();
    void updateVisible();

    std::vector<LEDChannel*> m_channels;

    int m_zeroAdjustment = 3300;
    int m_blackLevelShift = -100;

    InspectionCmdProxy m_inspectionCmdProxy;
    bool m_calibrationReady = false;
    bool m_serviceReady = false;
    bool m_weldHeadReady = false;
    bool m_grabberReady = false;
    bool m_calibrating = false;
    bool m_calibrationUpdating = false;
    bool m_weldHeadUpdating = false;
    bool m_grabberUpdating = false;
    bool m_active = false;
    int m_ledType = 0;
    int m_cameraInterfaceType = 0;
    QTimer *m_ledCalibrationTimeout;
    DeviceProxyWrapper *m_calibrationDevice = nullptr;
    QMetaObject::Connection m_calibrationDeviceDestroyed;
    DeviceProxyWrapper *m_serviceDevice = nullptr;
    QMetaObject::Connection m_serviceDeviceDestroyed;
    DeviceProxyWrapper *m_weldHeadDevice = nullptr;
    QMetaObject::Connection m_weldHeadDeviceDestroyed;
    DeviceProxyWrapper *m_grabberDevice = nullptr;
    QMetaObject::Connection m_grabberDeviceDestroyed;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::LEDCalibrationController*)
