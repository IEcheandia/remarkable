#include "scanLabController.h"
#include "deviceProxyWrapper.h"
#include "attributeModel.h"
#include "attribute.h"
#include "hardwareParameters.h"

#include "precitec/userManagement.h"

#include <QFutureWatcher>
#include <QtConcurrentRun>

using precitec::interface::Configuration;
using precitec::interface::SmpKeyValue;
using precitec::interface::TKeyValue;
using precitec::gui::components::user::UserManagement;
using precitec::storage::AttributeModel;

namespace precitec
{
namespace gui
{

namespace
{

template <typename T>
T getValue(const Configuration &configuration, const std::string &key, T defaultValue)
{
    auto it = std::find_if(configuration.begin(), configuration.end(), [key] (auto kv) { return kv->key() == key; });
    if (it == configuration.end())
    {
        return defaultValue;
    }
    return (*it)->template value<T>();
}

}

ScanLabController::ScanLabController(QObject* parent)
    : LiveModeController(parent)
{
    connect(this, &ScanLabController::weldheadDeviceProxyChanged, this,
        [this]
        {
            if (!m_weldheadDeviceProxy)
            {
                setReady(false);
                return;
            }
            auto weldheadDeviceProxy = m_weldheadDeviceProxy;
            auto watcher = new QFutureWatcher<Configuration>{this};
            connect(watcher, &QFutureWatcher<Configuration>::finished, this,
                [this, watcher]
                {
                    watcher->deleteLater();
                    auto configuration = watcher->result();

                    m_scannerXPosition = getValue(configuration, std::string("Scanner_Actual_X_Position"), 0.0);
                    emit scannerXPositionChanged();
                    m_scannerYPosition = getValue(configuration, std::string("Scanner_Actual_Y_Position"), 0.0);
                    emit scannerYPositionChanged();

                    setReady(true);
                }
            );
            watcher->setFuture(QtConcurrent::run(
                [weldheadDeviceProxy]
                {
                    return weldheadDeviceProxy->deviceProxy()->get();
                }));
        }
    );

    connect(this, &ScanLabController::scannerXPositionChanged, this, &ScanLabController::canReachXLimitChanged);
    connect(this, &ScanLabController::scannerYPositionChanged, this, &ScanLabController::canReachYLimitChanged);
    connect(this, &ScanLabController::xLimitChanged, this, &ScanLabController::canReachXLimitChanged);
    connect(this, &ScanLabController::yLimitChanged, this, &ScanLabController::canReachYLimitChanged);
}

ScanLabController::~ScanLabController() = default;

void ScanLabController::setWeldheadDeviceProxy(DeviceProxyWrapper* device)
{
    if (m_weldheadDeviceProxy == device)
    {
        return;
    }
    disconnect(m_weldheadDeviceDestroyConnection);

    m_weldheadDeviceProxy = device;

    if (m_weldheadDeviceProxy)
    {
        m_weldheadDeviceDestroyConnection = connect(m_weldheadDeviceProxy, &QObject::destroyed, this, std::bind(&ScanLabController::setWeldheadDeviceProxy, this, nullptr));
    } else
    {
        m_weldheadDeviceDestroyConnection = {};
    }

    emit weldheadDeviceProxyChanged();
}

void ScanLabController::setAttributeModel(AttributeModel *model)
{
    if (m_attributeModel == model)
    {
        return;
    }

    m_attributeModel = model;
    disconnect(m_attributeModelDestroyedConnection);

    if (m_attributeModel)
    {
        m_attributeModelDestroyedConnection = connect(m_attributeModel, &AttributeModel::destroyed, this, std::bind(&ScanLabController::setAttributeModel, this, nullptr));

        if (auto attribute = m_attributeModel->findAttribute(HardwareParameters::instance()->properties(HardwareParameters::Key::ScannerNewXPosition).uuid))
        {
            m_xMin = attribute->minValue().toDouble();
            m_xMax = attribute->maxValue().toDouble();
            emit xLimitChanged();
        }

        if (auto attribute = m_attributeModel->findAttribute(HardwareParameters::instance()->properties(HardwareParameters::Key::ScannerNewYPosition).uuid))
        {
            m_yMin = attribute->minValue().toDouble();
            m_yMax = attribute->maxValue().toDouble();
            emit yLimitChanged();
        }

    } else
    {
        m_attributeModelDestroyedConnection = {};
    }

    emit attributeModelChanged();
}

void ScanLabController::setReady(bool set)
{
    if (m_ready == set)
    {
        return;
    }
    m_ready = set;
    emit readyChanged();
}

void ScanLabController::setUpdating(bool set)
{
    if (m_updating == set)
    {
        return;
    }
    m_updating = set;
    emit updatingChanged();
}

void ScanLabController::updatePosition()
{
    update([this]
    {
        if (m_weldheadDeviceProxy)
        {
            m_weldheadDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<double>(std::string("Scanner_New_X_Position"), m_scannerXPosition)));
            m_weldheadDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<double>(std::string("Scanner_New_Y_Position"), m_scannerYPosition)));
            m_weldheadDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<bool>(std::string("Scanner_DriveToPosition"), true)));
        }
    });
}

void ScanLabController::setScannerXPosition(double x)
{
    const auto newXValue = qBound(m_xMin, x, m_xMax);
    if (qFuzzyCompare(m_scannerXPosition, newXValue))
    {
        return;
    }
    m_scannerXPosition = newXValue;

    updatePosition();

    emit scannerXPositionChanged();
}

void ScanLabController::setScannerYPosition(double y)
{
    const auto newYValue = qBound(m_yMin, y, m_yMax);
    if (qFuzzyCompare(m_scannerYPosition, newYValue))
    {
        return;
    }
    m_scannerYPosition = newYValue;

    updatePosition();

    emit scannerYPositionChanged();
}

void ScanLabController::update(std::function<void()> updateFunction)
{
    if (!m_weldheadDeviceProxy || updating())
    {
        return;
    }

    setUpdating(true);

    if (liveMode())
    {
        stopLiveMode();
    }

    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, this,
        [this, watcher]
        {
            watcher->deleteLater();
            if (liveMode())
            {
                startLiveMode();
            }
            setUpdating(false);
        }
    );
    watcher->setFuture(QtConcurrent::run(updateFunction));
}

void ScanLabController::setNotificationServer(DeviceNotificationServer *server)
{
    if (m_notificationServer.data() == server)
    {
        return;
    }
    disconnect(m_notificationConnection);
    m_notificationConnection = QMetaObject::Connection{};
    m_notificationServer = server;
    if (server)
    {
        m_notificationConnection = connect(server, &DeviceNotificationServer::changed, this,
            [this] (const QUuid &id, SmpKeyValue kv)
            {
                if (!m_weldheadDeviceProxy)
                {
                    return;
                }
                if (kv->key() == "Scanner_Actual_X_Position")
                {
                    m_actualXPosition = kv->value<double>();
                    emit scannerActualXPostionChanged();
                } else if (kv->key() == "Scanner_Actual_Y_Position")
                {
                    m_actualYPosition = kv->value<double>();
                    emit scannerActualXPostionChanged();
                }
            }, Qt::QueuedConnection
        );
    }
    emit notificationServerChanged();
}

bool ScanLabController::hasPermission()
{
    return m_weldheadDeviceProxy && UserManagement::instance()->hasPermission(int(m_weldheadDeviceProxy->writePermission()));
}

bool ScanLabController::canIncrementX() const
{
    return m_scannerXPosition < m_xMax;
}

bool ScanLabController::canDecrementX() const
{
    return m_scannerXPosition > m_xMin;
}

bool ScanLabController::canIncrementY() const
{
    return m_scannerYPosition < m_yMax;
}

bool ScanLabController::canDecrementY() const
{
    return m_scannerYPosition > m_yMin;
}

void ScanLabController::incrementXPosition()
{
    setScannerXPosition(m_scannerXPosition + m_stepSize);
}

void ScanLabController::decrementXPosition()
{
    setScannerXPosition(m_scannerXPosition - m_stepSize);
}

void ScanLabController::incrementYPosition()
{
    setScannerYPosition(m_scannerYPosition + m_stepSize);
}

void ScanLabController::decrementYPosition()
{
    setScannerYPosition(m_scannerYPosition - m_stepSize);
}

void ScanLabController::resetToZero()
{
    update([this]
    {
        if (m_weldheadDeviceProxy)
        {
            m_weldheadDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<bool>(std::string("Scanner_DriveToZero"), true)));

            m_scannerXPosition = 0.0;
            emit scannerXPositionChanged();
            m_scannerYPosition = 0.0;
            emit scannerYPositionChanged();
        }
    });
}

void ScanLabController::setFiberSwitchPosition(int n)
{
    update([this, n]
    {
        wmLog(eInfo, "set fiber switch position to %d", n);

        m_weldheadDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("OCT_Reference_Arm"), n)));
        m_weldheadDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<bool>(std::string("Scanner_SetOCTReference"), true)));
    });
}

}
}

