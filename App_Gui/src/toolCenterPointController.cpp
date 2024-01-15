#include "toolCenterPointController.h"
#include "deviceProxyWrapper.h"
#include "permissions.h"

#include <precitec/userManagement.h>

#include <QtConcurrentRun>
#include <QFutureWatcher>

using precitec::interface::SmpKeyValue;
using precitec::interface::Configuration;
using precitec::interface::TKeyValue;
using precitec::gui::components::user::UserManagement;


namespace precitec
{
namespace gui
{
    

ToolCenterPointController::ToolCenterPointController(QObject *parent)
    : LiveModeController(parent)
{
    connect(this, &ToolCenterPointController::grabberDeviceProxyChanged, this, &ToolCenterPointController::grabberDeviceChanged);

    connect(this, &ToolCenterPointController::calibrationDeviceChanged, this, &ToolCenterPointController::fetchTcp);
    connect(this, &ToolCenterPointController::grabberDeviceProxyChanged, this, &ToolCenterPointController::fetchRoiOffset);
    connect(this, &ToolCenterPointController::isOCTChanged, this, &ToolCenterPointController::fetchTcp);
    connect(this, &ToolCenterPointController::isOCTChanged, this, &ToolCenterPointController::fetchRoiOffset);

}

ToolCenterPointController::~ToolCenterPointController() = default;

void ToolCenterPointController::setTcp(const QPointF &tcp)
{
    if (m_tcp == tcp)
    {
        return;
    }
    m_tcp = tcp;
    markAsChanged();
    emit tcpChanged();
}

void ToolCenterPointController::markAsChanged()
{
    if (m_changes)
    {
        return;
    }
    m_changes = true;
    emit hasChangesChanged();
}

void ToolCenterPointController::saveChanges()
{
    if (!m_changes || !m_calibrationDevice)
    {
        return;
    }
    if (!UserManagement::instance()->hasPermission(int(m_calibrationDevice->writePermission())))
    {
        return;
    }
    if (!UserManagement::instance()->hasPermission(int(Permission::SetToolCenterPoint)))
    {
        return;
    }
    if (m_isOCT &&  !UserManagement::instance()->hasPermission(int(Permission::EditIDMDeviceConfig)))
    {
        return;
    }
    
    if (m_isOCT &&  m_coordinatesRequestProxy.get() == nullptr)
    {
        return;
    }
    
    auto watcher = new QFutureWatcher<void>{this};
    increaseUpdateCounter();
    connect(watcher, &QFutureWatcher<void>::finished, this,
        [this, watcher]
        {
            watcher->deleteLater();
            m_changes = false;
            emit hasChangesChanged();
            m_originalTcp = m_tcp;
            emit originalTcpChanged();
            decreaseUpdateCounter();
        }
    );
    const auto calibrationDevice = m_calibrationDevice;
    if (m_isOCT)
    {
        watcher->setFuture(QtConcurrent::run(
            [this, calibrationDevice]
            {
                // m_tcp is the tcp coordinate on image
                
                //compute corresponding position in Newson, the coax equivalent tcp will be computed by the calibration
                auto tcp_newson =  m_coordinatesRequestProxy->getNewsonPosition(m_tcp.x(), m_tcp.y(), interface::OCT_Mode::eScan2D );
                calibrationDevice->setKeyValues({
                     SmpKeyValue{new TKeyValue<double>{"X_TCP_Newson", tcp_newson.x }}, 
                     SmpKeyValue{new TKeyValue<double>{"Y_TCP_Newson", tcp_newson.y }}
                    });
            }
        ));
    }
    else
    {
        watcher->setFuture(QtConcurrent::run(
            [this, calibrationDevice]
            {
                calibrationDevice->setKeyValue(SmpKeyValue{new precitec::interface::TKeyValue<double>{"xtcp", m_tcp.x() + m_roiOffset.x()}});
                calibrationDevice->setKeyValue(SmpKeyValue{new precitec::interface::TKeyValue<double>{"ytcp", m_tcp.y() + m_roiOffset.y()}});
            }
        ));
    }

}

void ToolCenterPointController::setCalibrationDevice(DeviceProxyWrapper *device)
{
    if (m_calibrationDevice == device)
    {
        return;
    }
    disconnect(m_calibrationDeviceDestroyed);
    m_calibrationDevice = device;
    if (m_calibrationDevice)
    {
        m_calibrationDeviceDestroyed = connect(device, &DeviceProxyWrapper::destroyed, this, std::bind(&ToolCenterPointController::setCalibrationDevice, this, nullptr));
    } else
    {
        m_calibrationDeviceDestroyed = {};
    }
    emit calibrationDeviceChanged();
}

void ToolCenterPointController::fetchTcp()
{
    if (!m_calibrationDevice)
    {
        return;
    }
    auto calibrationDevice = m_calibrationDevice;
    if (!UserManagement::instance()->hasPermission(int(calibrationDevice->readPermission())))
    {
        return;
    }
    auto watcher = new QFutureWatcher<Configuration>{this};
    
    if (m_isOCT)
    {
        connect(watcher, &QFutureWatcher<Configuration>::finished, this,
            [this, watcher]
            {
                decreaseUpdateCounter();
                watcher->deleteLater();
                const auto configuration = watcher->result();
                auto xIt = std::find_if(configuration.begin(), configuration.end(), [] (const auto &kv) { return kv->key() == "X_TCP_Newson"; });
                auto yIt = std::find_if(configuration.begin(), configuration.end(), [] (const auto &kv) { return kv->key() == "Y_TCP_Newson"; });
                if (xIt == configuration.end() || yIt == configuration.end())
                {
                    return;
                }
                if (!m_coordinatesRequestProxy->availableOCTCoordinates(interface::OCT_Mode::eScan2D))
                {
                    return ;
                }
                
                auto tcp_scan2dImage = m_coordinatesRequestProxy->getScreenPositionFromNewson((*xIt)->value<double>(), (*yIt)->value<double>(), interface::OCT_Mode::eScan2D);
                updateOriginalTcp({tcp_scan2dImage.x, tcp_scan2dImage.y});
            }
        );
        
    }
    else
    {
        connect(watcher, &QFutureWatcher<Configuration>::finished, this,
            [this, watcher]
            {
                decreaseUpdateCounter();
                watcher->deleteLater();
                const auto configuration = watcher->result();
                auto xIt = std::find_if(configuration.begin(), configuration.end(), [] (const auto &kv) { return kv->key() == "xtcp"; });
                auto yIt = std::find_if(configuration.begin(), configuration.end(), [] (const auto &kv) { return kv->key() == "ytcp"; });
                if (xIt == configuration.end() || yIt == configuration.end())
                {
                    return;
                }
                const QPointF tcp{(*xIt)->value<double>() - m_roiOffset.x(), (*yIt)->value<double>() - m_roiOffset.y()};
                updateOriginalTcp(tcp);
            }
        );
    }
    increaseUpdateCounter();
    watcher->setFuture(QtConcurrent::run([calibrationDevice] { return calibrationDevice->deviceProxy()->get(); }));
}

void ToolCenterPointController::fetchRoiOffset()
{
    auto grabberDevice = grabberDeviceProxy();
    if (!grabberDevice)
    {
        return;
    }
    
    if (m_isOCT)
    {
        const QPoint offset{0,0};
        if (m_roiOffset == offset)
        {
            return;
        }
        const QPointF tcp = m_tcp + m_roiOffset - offset;
        m_roiOffset = offset;
        updateOriginalTcp(tcp);
        return;
    }
    
    auto watcher = new QFutureWatcher<Configuration>{this};
    connect(watcher, &QFutureWatcher<Configuration>::finished, this,
        [this, watcher]
        {
            decreaseUpdateCounter();
            watcher->deleteLater();
            const auto configuration = watcher->result();
            auto xIt = std::find_if(configuration.begin(), configuration.end(), [] (const auto &kv) { return kv->key() == "Window.X"; });
            auto yIt = std::find_if(configuration.begin(), configuration.end(), [] (const auto &kv) { return kv->key() == "Window.Y"; });
            if (xIt == configuration.end() || yIt == configuration.end())
            {
                return;
            }
            const QPoint offset{(*xIt)->value<int>(), (*yIt)->value<int>()};
            if (m_roiOffset == offset)
            {
                return;
            }
            const QPointF tcp = m_tcp + m_roiOffset - offset;
            m_roiOffset = offset;
            updateOriginalTcp(tcp);
        }
    );
    increaseUpdateCounter();
    watcher->setFuture(QtConcurrent::run([grabberDevice] { return grabberDevice->deviceProxy()->get(); }));
}


void ToolCenterPointController::setCalibrationCoordinatesRequestProxy ( CalibrationCoordinatesRequestProxy proxy )
{
    if (m_coordinatesRequestProxy == proxy)
    {
        return;
    }
    m_coordinatesRequestProxy = proxy;
    emit calibrationCoordinatesRequestProxyChanged();
}

void ToolCenterPointController::discardChanges()
{
    m_tcp = m_originalTcp;
    emit tcpChanged();
    if (m_changes)
    {
        m_changes = false;
        emit hasChangesChanged();
    }
}

void ToolCenterPointController::updateOriginalTcp(const QPointF &tcp)
{
    if (tcp != m_originalTcp)
    {
        m_originalTcp = tcp;
        emit originalTcpChanged();
    }
    if (tcp != m_tcp)
    {
        m_tcp = tcp;
        emit tcpChanged();
    }
    if (m_changes)
    {
        m_changes = false;
        emit hasChangesChanged();
    }
}

void ToolCenterPointController::increaseUpdateCounter()
{
    m_updateCounter++;
    if (m_updateCounter == 1)
    {
        emit updatingChanged();
    }
}

void ToolCenterPointController::decreaseUpdateCounter()
{
    m_updateCounter--;
    if (m_updateCounter == 0)
    {
        emit updatingChanged();
    }
}

}
}
