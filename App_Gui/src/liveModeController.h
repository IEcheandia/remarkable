#pragma once

#include <QObject>
#include <QFuture>
#include <QMutex>

#include "event/inspectionCmd.proxy.h"

#include <functional>

class QTimer;
class QUuid;

namespace precitec
{

typedef std::shared_ptr<precitec::interface::TInspectionCmd<precitec::interface::EventProxy>> InspectionCmdProxy;

namespace storage
{
class ProductModel;
}

namespace gui
{
class SystemStatusServer;
class DeviceProxyWrapper;

/**
 * The LiveModeController is a basic controller which can enable/disable live mode.
 *
 * It can be used as a base class for other controllers needing live mode
 **/
class LiveModeController : public QObject
{
    Q_OBJECT
    /**
     * InspectionCmdProxy, needed for setting to live mode.
     **/
    Q_PROPERTY(precitec::InspectionCmdProxy inspectionCmdProxy READ inspectionCmdProxy WRITE setInspectionCmdProxy NOTIFY inspectionCmdProxyChanged)
    /**
     * Whether the system should go to liveMode.
     **/
    Q_PROPERTY(bool liveMode READ liveMode WRITE setLiveMode NOTIFY liveModeChanged)
    /**
     * ProductModel, needed for enabling live mode (setting to default product)
     **/
    Q_PROPERTY(precitec::storage::ProductModel *productModel READ productModel WRITE setProductModel NOTIFY productModelChanged)
    /**
     * Whether the system is currently updating the live product.
     **/
    Q_PROPERTY(bool updating READ isUpdating NOTIFY updatingChanged)
    /**
     * SystemStatusServer to notice when live product got updated
     **/
    Q_PROPERTY(precitec::gui::SystemStatusServer *systemStatus READ systemStatus WRITE setSystemStatus NOTIFY systemStatusChanged)
    /**
     * The device proxy to the camera/grabber, needed for updating.
     **/
    Q_PROPERTY(precitec::gui::DeviceProxyWrapper *grabberDeviceProxy READ grabberDeviceProxy WRITE setGrabberDeviceProxy NOTIFY grabberDeviceProxyChanged)
public:
    explicit LiveModeController(QObject *parent = nullptr);
    ~LiveModeController() override;

    InspectionCmdProxy inspectionCmdProxy() const
    {
        return m_inspectionCmdProxy;
    }
    void setInspectionCmdProxy(const InspectionCmdProxy &proxy);

    bool liveMode() const
    {
        return m_liveMode;
    }
    void setLiveMode(bool liveMode);

    void setProductModel(precitec::storage::ProductModel *pm);
    precitec::storage::ProductModel *productModel() const
    {
        return m_productModel;
    }

    bool isUpdating() const;

    SystemStatusServer *systemStatus() const
    {
        return m_systemStatus;
    }
    void setSystemStatus(SystemStatusServer *systemStatus);

    precitec::gui::DeviceProxyWrapper *grabberDeviceProxy() const
    {
        return m_grabberDevice;
    }
    void setGrabberDeviceProxy(precitec::gui::DeviceProxyWrapper *grabberDevice);

Q_SIGNALS:
    void inspectionCmdProxyChanged();
    void liveModeChanged();
    void productModelChanged();
    void updatingChanged();
    void systemStatusChanged();
    void systemStateNormal();
    void grabberDeviceProxyChanged();

protected:
    QFuture<void> startLiveMode();
    QFuture<void> stopLiveMode(bool returnOnReady = true);
    void startDelayedLiveMode();
    void handlesStartLiveMode()
    {
        m_handlesStartLiveMode = true;
    }

    /**
     * Live mode won't be activated till we return from not ready.
     **/
    void requireActivateAfterReturnFromNotReady();

    QMutex *grabberMutex()
    {
        return &m_grabberMutex;
    }

private:
    void productUpdated(const QUuid &uuid);
    InspectionCmdProxy m_inspectionCmdProxy;
    bool m_liveMode = false;
    precitec::storage::ProductModel *m_productModel = nullptr;
    QMetaObject::Connection m_destroyConnection;
    QMutex m_inspectionCmdMutex;
    bool m_liveModeActive = false;
    QTimer *m_startDelayedLiveMode;
    SystemStatusServer *m_systemStatus = nullptr;
    QMetaObject::Connection m_systemStatusDestroyed;
    QMetaObject::Connection m_productUpdatedConnection;
    QTimer *m_returnToLiveModeTimer = nullptr;
    bool m_activateAfterReturnFromNotReady = false;
    bool m_returnOnReady = true;
    bool m_handlesStartLiveMode = false;
    DeviceProxyWrapper *m_grabberDevice = nullptr;
    QMetaObject::Connection m_grabberDestroyConnection;
    QMutex m_grabberMutex;
    const bool m_gigECamera;
};

}
}

