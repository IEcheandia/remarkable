#pragma once

#include "liveModeController.h"
#include  "message/calibrationCoordinatesRequest.proxy.h"

#include <QPointF>

namespace precitec
{
//compare definition in imageMeasurementController.h
typedef std::shared_ptr<precitec::interface::TCalibrationCoordinatesRequest<precitec::interface::AbstractInterface>> CalibrationCoordinatesRequestProxy;

namespace gui
{

class DeviceProxyWrapper;


/**
 * Controller to modifiy the tool center point.
 * It fetches the current tcp from the calibration device, tracks changes and supports saving.
 **/
class ToolCenterPointController : public LiveModeController
{
    Q_OBJECT
    /**
     * Current tcp value. Initially set to the value fetched from the @link{calibrationDevice}.
     **/
    Q_PROPERTY(QPointF tcp READ tcp WRITE setTcp NOTIFY tcpChanged)
    /**
     * Original tcp value. Initially set to the value fetched from the @link{calibrationDevice}.
     **/
    Q_PROPERTY(QPointF originalTcp READ originalTcp NOTIFY originalTcpChanged)
    /**
     * Whether the tcp was changed
     **/
    Q_PROPERTY(bool changes READ hasChanges NOTIFY hasChangesChanged)
    /**
     * Controller is currently updating values from/to system asynchrounously
     **/
    Q_PROPERTY(bool updating READ isUpdating NOTIFY updatingChanged)
    /**
     * The device proxy for the Calibration device. Required to fetch the initial tcp value and update the tcp value.
     **/
    Q_PROPERTY(precitec::gui::DeviceProxyWrapper *calibrationDevice READ calibrationDevice WRITE setCalibrationDevice NOTIFY calibrationDeviceChanged)
    /**
     * The device proxy to the camera/grabber, needed for fetching the hardware roi offset.
     **/
    Q_PROPERTY(precitec::gui::DeviceProxyWrapper *grabberDevice READ grabberDeviceProxy WRITE setGrabberDeviceProxy NOTIFY grabberDeviceChanged)
    
    /**
     * Whether the tcp is for an oct system
     **/
    Q_PROPERTY(bool isOCT READ isOCT WRITE setIsOCT NOTIFY isOCTChanged)
    
    Q_PROPERTY(precitec::CalibrationCoordinatesRequestProxy  calibrationCoordinatesRequestProxy READ calibrationCoordinatesRequestProxy WRITE setCalibrationCoordinatesRequestProxy NOTIFY calibrationCoordinatesRequestProxyChanged)
    
public:
    ToolCenterPointController(QObject *parent = nullptr);
    ~ToolCenterPointController() override;

    QPointF tcp() const
    {
        return m_tcp;
    }
    void setTcp(const QPointF &tcp);

    QPointF originalTcp() const
    {
        return m_originalTcp;
    }

    bool hasChanges() const
    {
        return m_changes;
    }

    bool isUpdating() const
    {
        return m_updateCounter > 0;
    }

    DeviceProxyWrapper *calibrationDevice() const
    {
        return m_calibrationDevice;
    }
    void setCalibrationDevice(DeviceProxyWrapper *device);

    bool isOCT() const
    {
        return m_isOCT;
    }
    
    void setIsOCT(bool p_oIsOCT)
    {
        if (m_isOCT == p_oIsOCT)
        {
            return;
        }
        m_isOCT = p_oIsOCT;
        emit isOCTChanged();
    }
    
    CalibrationCoordinatesRequestProxy  calibrationCoordinatesRequestProxy() const
    {
        return m_coordinatesRequestProxy;
    }
    
    void setCalibrationCoordinatesRequestProxy(CalibrationCoordinatesRequestProxy  proxy);
    
    /**
     * Saves the changes to the tcp.
     **/
    Q_INVOKABLE void saveChanges();

    /**
     * Discards the changes to the tcp by re-reading from device
     **/
    Q_INVOKABLE void discardChanges();

Q_SIGNALS:
    void tcpChanged();
    void originalTcpChanged();
    void hasChangesChanged();
    void calibrationDeviceChanged();
    void grabberDeviceChanged();
    void isOCTChanged();
    void calibrationCoordinatesRequestProxyChanged();
    void updatingChanged();

private:
    void markAsChanged();
    void fetchTcp();
    void fetchRoiOffset();
    void updateOriginalTcp(const QPointF &tcp);
    void increaseUpdateCounter();
    void decreaseUpdateCounter();
    QPointF m_tcp = {512, 512};
    QPointF m_originalTcp = {512, 512};
    bool m_changes = false;
    DeviceProxyWrapper *m_calibrationDevice = nullptr;
    QMetaObject::Connection m_calibrationDeviceDestroyed;
    DeviceProxyWrapper *m_grabberDevice = nullptr;
    QMetaObject::Connection m_grabberDeviceDestroyed;
    QPoint m_roiOffset = {0, 0};
    bool m_isOCT = false; 
    CalibrationCoordinatesRequestProxy m_coordinatesRequestProxy = nullptr;
    int m_updateCounter = 0;
};

}
}
