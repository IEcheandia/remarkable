#include "cameraCalibrationModel.h"
#include "message/calibration.interface.h"
#include "calibrationChangeEntry.h"

#include <precitec/userManagement.h>
#include <precitec/userLog.h>

#include <QFutureWatcher>
#include <QtConcurrentRun>
#include <QTimer>

using precitec::interface::Configuration;
using precitec::interface::SmpKeyValue;
using precitec::interface::TKeyValue;
using precitec::gui::components::userLog::UserLog;
using precitec::gui::components::user::UserManagement;

namespace precitec
{
namespace gui
{

namespace
{

template <typename T>
T getValue(const Configuration& configuration, const std::string& key, T defaultValue)
{
    auto it = std::find_if(configuration.begin(), configuration.end(), [key] (auto kv) { return kv->key() == key; });
    if (it == configuration.end())
    {
        return defaultValue;
    }
    return (*it)->template value<T>();
}

}

CameraCalibrationModel::CameraCalibrationModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_calibrationTimeoutTimer(new QTimer(this))
{
    m_laserLineData.emplace(LaserLine::One, LaserLineData{});
    m_laserLineData.emplace(LaserLine::Two, LaserLineData{});
    m_laserLineData.emplace(LaserLine::Three, LaserLineData{});

    m_calibrationTimeoutTimer->setSingleShot(true);
    m_calibrationTimeoutTimer->setInterval(std::chrono::seconds{30});
    connect(m_calibrationTimeoutTimer, &QTimer::timeout, this, &CameraCalibrationModel::endCalibration);

    connect(this, &CameraCalibrationModel::calibrationDeviceProxyChanged, this,
        [this]
        {
            if (!m_calibrationDeviceProxy)
            {
                setCalibrationReady(false);
                return;
            }
            auto calibrationDeviceProxy = m_calibrationDeviceProxy;
            if (!UserManagement::instance()->hasPermission(int(calibrationDeviceProxy->readPermission())))
            {
                return;
            }
            auto watcher = new QFutureWatcher<Configuration>{this};
            connect(watcher, &QFutureWatcher<Configuration>::finished, this,
                    [this, watcher]
                    {
                        watcher->deleteLater();
                        auto configuration = watcher->result();

                        m_laserLineData.at(LaserLine::One).left = getValue(configuration, std::string("XLeftEdge0"), 40);
                        m_laserLineData.at(LaserLine::One).right = getValue(configuration, std::string("XRightEdge0"), 100);
                        m_laserLineData.at(LaserLine::One).top = getValue(configuration, std::string("YTopEdge0"), 40);
                        m_laserLineData.at(LaserLine::One).bottom = getValue(configuration, std::string("YBottomEdge0"), 100);

                        m_laserLineData.at(LaserLine::Two).left = getValue(configuration, std::string("XLeftEdge2"), 40);
                        m_laserLineData.at(LaserLine::Two).right = getValue(configuration, std::string("XRightEdge2"), 100);
                        m_laserLineData.at(LaserLine::Two).top = getValue(configuration, std::string("YTopEdge2"), 40);
                        m_laserLineData.at(LaserLine::Two).bottom = getValue(configuration, std::string("YBottomEdge2"), 100);

                        m_laserLineData.at(LaserLine::Three).left = getValue(configuration, std::string("XLeftEdgeTCP"), 40);
                        m_laserLineData.at(LaserLine::Three).right = getValue(configuration, std::string("XRightEdgeTCP"), 100);
                        m_laserLineData.at(LaserLine::Three).top = getValue(configuration, std::string("YTopEdgeTCP"), 40);
                        m_laserLineData.at(LaserLine::Three).bottom = getValue(configuration, std::string("YBottomEdgeTCP"), 100);

                        emit dataChanged(index(0), index(m_laserLineData.size() - 1), {Qt::UserRole, Qt::UserRole + 1, Qt::UserRole + 2, Qt::UserRole + 3});
                        m_targetWidth = getValue(configuration, std::string("GrooveWidth"), 3.0);
                        m_targetHeight = getValue(configuration, std::string("GrooveDepth"), 2.0);
                        emit targetWidthChanged();
                        emit targetHeightChanged();

                        setCalibrationReady(true);

                        updateRange();
                    }
            );
            watcher->setFuture(QtConcurrent::run(
                [calibrationDeviceProxy]
                {
                    return calibrationDeviceProxy->deviceProxy()->get();
                }
            ));
        }
    );

    connect(this, &CameraCalibrationModel::workflowDeviceProxyChanged, this,
        [this]
        {
            if (!m_workflowDeviceProxy)
            {
                setWorkflowReady(false);
                return;
            }
            auto workflowDeviceProxy = m_workflowDeviceProxy;
            if (!UserManagement::instance()->hasPermission(int(workflowDeviceProxy->readPermission())))
            {
                return;
            }
            auto watcher = new QFutureWatcher<Configuration>{this};
            connect(watcher, &QFutureWatcher<Configuration>::finished, this,
                    [this, watcher]
                    {
                        watcher->deleteLater();
                        auto configuration = watcher->result();

                        m_laserLineData.at(LaserLine::One).intensity = getValue(configuration, std::string("LineLaser1DefaultIntensity"), 90);
                        m_laserLineData.at(LaserLine::Two).intensity = getValue(configuration, std::string("LineLaser2DefaultIntensity"), 40);
                        m_laserLineData.at(LaserLine::Three).intensity = getValue(configuration, std::string("FieldLight1DefaultIntensity"), 90);

                        emit dataChanged(index(0), index(m_laserLineData.size() - 1), {Qt::UserRole + 4});

                        setWorkflowReady(true);
                    }
            );
            watcher->setFuture(QtConcurrent::run(
                [workflowDeviceProxy]
                {
                    return workflowDeviceProxy->deviceProxy()->get();
                }
            ));
        }
    );

    connect(this, &CameraCalibrationModel::grabberDeviceProxyChanged, this, &CameraCalibrationModel::updateImageRoi);
    connect(this, &CameraCalibrationModel::imageRoiChanged, this, &CameraCalibrationModel::updateRange);
}

void CameraCalibrationModel::updateRange()
{
    if (m_imageRoi.isValid())
    {
        setRight(LaserLine::One, std::min(m_laserLineData.at(LaserLine::One).right, m_imageRoi.width()));
        setRight(LaserLine::Two, std::min(m_laserLineData.at(LaserLine::Two).right, m_imageRoi.width()));
        setRight(LaserLine::Three, std::min(m_laserLineData.at(LaserLine::Three).right, m_imageRoi.width()));

        setBottom(LaserLine::One, std::min(m_laserLineData.at(LaserLine::One).bottom, m_imageRoi.height()));
        setBottom(LaserLine::Two, std::min(m_laserLineData.at(LaserLine::Two).bottom, m_imageRoi.height()));
        setBottom(LaserLine::Three, std::min(m_laserLineData.at(LaserLine::Three).bottom, m_imageRoi.height()));

        emit dataChanged(index(0), index(m_laserLineData.size() - 1), {Qt::UserRole + 2, Qt::UserRole + 3});
    }
}

CameraCalibrationModel::~CameraCalibrationModel() = default;

int CameraCalibrationModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_laserLineData.size();
}

QHash<int, QByteArray> CameraCalibrationModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("name")},
        {Qt::UserRole, QByteArrayLiteral("x")},
        {Qt::UserRole + 1, QByteArrayLiteral("y")},
        {Qt::UserRole + 2, QByteArrayLiteral("width")},
        {Qt::UserRole + 3, QByteArrayLiteral("height")},
        {Qt::UserRole + 4, QByteArrayLiteral("intensity")},
        {Qt::UserRole + 5, QByteArrayLiteral("checked")}
    };
}

QVariant CameraCalibrationModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= int(m_laserLineData.size()))
    {
        return {};
    }

    const auto laserLine = static_cast<LaserLine>(index.row());
    const auto roi = m_laserLineData.at(laserLine).roi();

    switch (role)
    {
        case Qt::DisplayRole:
            return QStringLiteral("Line Laser %1").arg(index.row() + 1);
        case Qt::UserRole:
            return roi.x();
        case Qt::UserRole + 1:
            return roi.y();
        case Qt::UserRole + 2:
            return roi.width();
        case Qt::UserRole + 3:
            return roi.height();
        case Qt::UserRole + 4:
            return m_laserLineData.at(laserLine).intensity;
        case Qt::UserRole + 5:
            return m_laserLineData.at(laserLine).checked;
    }

    return {};
}

bool CameraCalibrationModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.row() >= int(m_laserLineData.size()))
    {
        return false;
    }

    const auto laserLine = static_cast<LaserLine>(index.row());
    bool updated = false;

    switch (role)
    {
        case Qt::UserRole:
            updated = setLeft(laserLine, value.toInt());
            break;
        case Qt::UserRole + 1:
            updated = setTop(laserLine, value.toInt());
            break;
        case Qt::UserRole + 2:
            updated = setRight(laserLine, m_laserLineData.at(laserLine).left + value.toInt());
            break;
        case Qt::UserRole + 3:
            updated = setBottom(laserLine, m_laserLineData.at(laserLine).top + value.toInt());
            break;
        case Qt::UserRole + 4:
            updated = setIntensity(laserLine, value.toInt());
            break;
        case Qt::UserRole + 5:
            updated = setChecked(laserLine, value.toBool());
            break;
    }

    if (updated)
    {
        emit dataChanged(index, index, {role});
    }

    return updated;
}

Qt::ItemFlags CameraCalibrationModel::flags(const QModelIndex& index) const
{
    Q_UNUSED(index)
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

void CameraCalibrationModel::setInspectionCmdProxy(const InspectionCmdProxy& proxy)
{
    if (m_inspectionCmdProxy == proxy)
    {
        return;
    }
    m_inspectionCmdProxy = proxy;
    emit inspectionCmdProxyChanged();
}

void CameraCalibrationModel::setCalibrationDeviceProxy(DeviceProxyWrapper* device)
{
    if (m_calibrationDeviceProxy == device)
    {
        return;
    }

    disconnect(m_calibrationDeviceDestroyed);

    m_calibrationDeviceProxy = device;

    if (m_calibrationDeviceProxy)
    {
        m_calibrationDeviceDestroyed = connect(device, &DeviceProxyWrapper::destroyed, this, std::bind(&CameraCalibrationModel::setCalibrationDeviceProxy, this, nullptr));
    } else
    {
        m_calibrationDeviceDestroyed = {};
    }

    emit calibrationDeviceProxyChanged();
}

void CameraCalibrationModel::setWorkflowDeviceProxy(DeviceProxyWrapper* device)
{
    if (m_workflowDeviceProxy == device)
    {
        return;
    }

    disconnect(m_workflowDeviceDestroyConnection);

    m_workflowDeviceProxy = device;

    if (m_workflowDeviceProxy)
    {
        m_workflowDeviceDestroyConnection = connect( m_workflowDeviceProxy, &QObject::destroyed, this, std::bind(&CameraCalibrationModel::setWorkflowDeviceProxy, this, nullptr));
    } else
    {
        m_workflowDeviceDestroyConnection = {};
    }

    emit workflowDeviceProxyChanged();
}

void CameraCalibrationModel::setGrabberDeviceProxy(DeviceProxyWrapper* device)
{
    if (m_grabberDeviceProxy == device)
    {
        return;
    }

    disconnect(m_grabberDeviceDestroyConnection);

    m_grabberDeviceProxy = device;

    if (m_grabberDeviceProxy)
    {
        m_grabberDeviceDestroyConnection = connect(m_grabberDeviceProxy, &QObject::destroyed, this, std::bind(&CameraCalibrationModel::setGrabberDeviceProxy, this, nullptr));
    } else
    {
        m_grabberDeviceDestroyConnection = {};
    }

    emit grabberDeviceProxyChanged();
}

void CameraCalibrationModel::setCalibrationReady(bool set)
{
    if (m_calibrationReady == set)
    {
        return;
    }
    m_calibrationReady = set;
    emit readyChanged();
}

void CameraCalibrationModel::setWorkflowReady(bool set)
{
    if (m_workflowReady == set)
    {
        return;
    }
    m_workflowReady = set;
    emit readyChanged();
}

void CameraCalibrationModel::setGrabberReady(bool set)
{
    if (m_grabberReady == set)
    {
        return;
    }
    m_grabberReady = set;
    emit readyChanged();
}

void CameraCalibrationModel::setCalibrating(bool calibrating)
{
    if (m_calibrating == calibrating)
    {
        return;
    }
    m_calibrating = calibrating;
    emit calibratingChanged();
}

void CameraCalibrationModel::startCalibration(int laserLine)
{
    if (!inspectionCmdProxy() || calibrating() || laserLine < 0)
    {
        return;
    }
    setCalibrating(true);

    const auto line = static_cast<LaserLine>(laserLine);

    switch (line)
    {
        case LaserLine::One:
            QtConcurrent::run(inspectionCmdProxy().get(), &precitec::interface::TInspectionCmd<precitec::interface::EventProxy>::requestCalibration, int(precitec::interface::eCalibrateOsIbLine0));
            break;
        case LaserLine::Two:
            QtConcurrent::run(inspectionCmdProxy().get(), &precitec::interface::TInspectionCmd<precitec::interface::EventProxy>::requestCalibration, int(precitec::interface::eCalibrateOsIbLine2));
            break;
        case LaserLine::Three:
            QtConcurrent::run(inspectionCmdProxy().get(), &precitec::interface::TInspectionCmd<precitec::interface::EventProxy>::requestCalibration, int(precitec::interface::eCalibrateOsIbLineTCP));
            break;
    }

    m_calibrationTimeoutTimer->start();
    UserLog::instance()->addChange(new CalibrationChangeEntry(CalibrationType::Camera));
}

void CameraCalibrationModel::startChessboardCalibration()
{
    if (!inspectionCmdProxy() || calibrating())
    {
        return;
    }
    setCalibrating(true);

    QtConcurrent::run(inspectionCmdProxy().get(), &precitec::interface::TInspectionCmd<precitec::interface::EventProxy>::requestCalibration, int(precitec::interface::eCalibGridChessboard));

    m_calibrationTimeoutTimer->start();
    UserLog::instance()->addChange(new CalibrationChangeEntry(CalibrationType::Chessboard));
}

void CameraCalibrationModel::startAngleCalibration()
{
    if (!inspectionCmdProxy() || calibrating())
    {
        return;
    }
    setCalibrating(true);

    QtConcurrent::run(inspectionCmdProxy().get(), &precitec::interface::TInspectionCmd<precitec::interface::EventProxy>::requestCalibration, int(precitec::interface::eCalibGridAngle));

    m_calibrationTimeoutTimer->start();
    UserLog::instance()->addChange(new CalibrationChangeEntry(CalibrationType::LaserAngle));
}

void CameraCalibrationModel::endCalibration()
{
    if (!calibrating())
    {
        return;
    }
    m_calibrationTimeoutTimer->stop();
    setCalibrating(false);
}

bool CameraCalibrationModel::setLeft(LaserLine line, int value)
{
    const auto& lineData = m_laserLineData.at(line);
    if (lineData.left == value || value < 0 || !m_calibrationReady || !hasCalibrationPermission())
    {
        return false;
    }
    m_laserLineData.at(line).left = value;

    const auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    updateCalibrationDevice([calibrationDeviceProxy, line, value]
    {
        switch (line)
        {
            case LaserLine::One:
                calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("XLeftEdge0"), value)));
                break;
            case LaserLine::Two:
                calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("XLeftEdge2"), value)));
                break;
            case LaserLine::Three:
                calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("XLeftEdgeTCP"), value)));
                break;
        }
    });

    return true;
}

bool CameraCalibrationModel::setTop(LaserLine line, int value)
{
    const auto& lineData = m_laserLineData.at(line);
    if (lineData.top == value || value < 0 || !m_calibrationReady || !hasCalibrationPermission())
    {
        return false;
    }
    m_laserLineData.at(line).top = value;

    const auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    updateCalibrationDevice([calibrationDeviceProxy, line, value]
    {
        switch (line)
        {
            case LaserLine::One:
                calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("YTopEdge0"), value)));
                break;
            case LaserLine::Two:
                calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("YTopEdge2"), value)));
                break;
            case LaserLine::Three:
                calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("YTopEdgeTCP"), value)));
                break;
        }
    });

    return true;
}

bool CameraCalibrationModel::setRight(LaserLine line, int value)
{
    const auto& lineData = m_laserLineData.at(line);
    if (lineData.right == value || value < 1 || value > m_imageRoi.width() || !m_calibrationReady || !hasCalibrationPermission())
    {
        return false;
    }
    m_laserLineData.at(line).right = value;

    const auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    updateCalibrationDevice([calibrationDeviceProxy, line, value]
    {
        switch (line)
        {
            case LaserLine::One:
                calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("XRightEdge0"), value)));
                break;
            case LaserLine::Two:
                calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("XRightEdge2"), value)));
                break;
            case LaserLine::Three:
                calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("XRightEdgeTCP"), value)));
                break;
        }
    });

    return true;
}

bool CameraCalibrationModel::setBottom(LaserLine line, int value)
{
    const auto& lineData = m_laserLineData.at(line);
    if (lineData.bottom == value || value < 1 || value > m_imageRoi.height() || !m_calibrationReady || !hasCalibrationPermission())
    {
        return false;
    }
    m_laserLineData.at(line).bottom = value;

    const auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    updateCalibrationDevice([calibrationDeviceProxy, line, value]
    {
        switch (line)
        {
            case LaserLine::One:
                calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("YBottomEdge0"), value)));
                break;
            case LaserLine::Two:
                calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("YBottomEdge2"), value)));
                break;
            case LaserLine::Three:
                calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("YBottomEdgeTCP"), value)));
                break;
        }
    });

    return true;
}

void CameraCalibrationModel::updateCalibrationDevice(std::function<void()> updateFunction)
{
    if (!m_calibrationDeviceProxy || m_calibrationUpdating)
    {
        return;
    }
    m_calibrationUpdating  = true;

    QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, this,
            [this, watcher]
            {
                watcher->deleteLater();
                m_calibrationUpdating = false;
            }
    );
    watcher->setFuture(QtConcurrent::run(updateFunction));
}

bool CameraCalibrationModel::setIntensity(LaserLine line, int value)
{
    if (m_laserLineData.at(line).intensity == value || !m_workflowReady || !hasWorkflowPermission())
    {
        return false;
    }
    m_laserLineData.at(line).intensity = value;

    const auto workflowDeviceProxy = m_workflowDeviceProxy;
    updateWorkflowDevice([workflowDeviceProxy, line, value]
    {
        switch (line)
        {
            case LaserLine::One:
                workflowDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("LineLaser1DefaultIntensity"), value)));
                break;
            case LaserLine::Two:
                workflowDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("LineLaser2DefaultIntensity"), value)));
                break;
            case LaserLine::Three:
                workflowDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("FieldLight1DefaultIntensity"), value)));
                break;
        }
    });

    return true;
}

bool CameraCalibrationModel::setChecked(LaserLine line, bool value)
{
    if (m_laserLineData.at(line).checked == value)
    {
        return false;
    }
    m_laserLineData.at(line).checked = value;
    return true;
}

void CameraCalibrationModel::updateWorkflowDevice(std::function<void()> updateFunction)
{
    if (!m_workflowDeviceProxy || m_workflowUpdating)
    {
        return;
    }
    m_workflowUpdating  = true;

    QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, this,
            [this, watcher]
            {
                watcher->deleteLater();
                m_workflowUpdating = false;
            }
    );
    watcher->setFuture(QtConcurrent::run(updateFunction));
}

void CameraCalibrationModel::updateImageRoi()
{
    if (!m_grabberDeviceProxy)
    {
        setGrabberReady(false);
        return;
    }
    auto grabberDeviceProxy = m_grabberDeviceProxy;
    QFutureWatcher<Configuration>* watcher = new QFutureWatcher<Configuration>(this);
    connect(watcher, &QFutureWatcher<Configuration>::finished, this,
        [this, watcher]
        {
            watcher->deleteLater();
            auto configuration = watcher->result();
            m_imageRoi = QRect{getValue(configuration, std::string("Window.X"), 0),
                            getValue(configuration, std::string("Window.Y"), 0),
                            getValue(configuration, std::string("Window.W"), 512),
                            getValue(configuration, std::string("Window.H"), 512)};

            emit imageRoiChanged();

            setGrabberReady(true);
        }
    );
    watcher->setFuture(QtConcurrent::run(
        [grabberDeviceProxy]
        {
            return grabberDeviceProxy->deviceProxy()->get();
        })
    );
}

void CameraCalibrationModel::setPreview(const QRect& preview)
{
    const auto& normalized = preview.normalized();
    const auto& bound = QRect{
        std::max(normalized.x(), 0),
        std::max(normalized.y(), 0),
        qBound(1, normalized.width(), m_imageRoi.width() - normalized.x()),
        qBound(1, normalized.height(), m_imageRoi.height() - normalized.y())
    };
    if (m_preview == bound)
    {
        return;
    }
    m_preview = bound;
    emit previewChanged();
}

void CameraCalibrationModel::setPreviewToRoi(int laserLine)
{
    if (laserLine < 0)
    {
        return;
    }
    const auto line = static_cast<LaserLine>(laserLine);

    if (!m_calibrationReady || !hasCalibrationPermission())
    {
        return;
    }

    m_laserLineData.at(line).left = m_preview.x();
    m_laserLineData.at(line).top = m_preview.y();
    m_laserLineData.at(line).right = m_preview.x() + m_preview.width();
    m_laserLineData.at(line).bottom = m_preview.y() + m_preview.height();

    const auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    updateCalibrationDevice([calibrationDeviceProxy, line, this]
    {
        switch (line)
        {
            case LaserLine::One:
                calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("XLeftEdge0"), m_preview.x())));
                calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("YTopEdge0"), m_preview.y())));
                calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("XRightEdge0"), m_preview.x() + m_preview.width())));
                calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("YBottomEdge0"), m_preview.y() + m_preview.height())));
                break;
            case LaserLine::Two:
                calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("XLeftEdge2"), m_preview.x())));
                calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("YTopEdge2"), m_preview.y())));
                calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("XRightEdge2"), m_preview.x() + m_preview.width())));
                calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("YBottomEdge2"), m_preview.y() + m_preview.height())));
                break;
            case LaserLine::Three:
                calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("XLeftEdgeTCP"), m_preview.x())));
                calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("YTopEdgeTCP"), m_preview.y())));
                calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("XRightEdgeTCP"), m_preview.x() + m_preview.width())));
                calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("YBottomEdgeTCP"), m_preview.y() + m_preview.height())));
                break;
        }
    });

    emit dataChanged(index(laserLine), index(laserLine), {Qt::UserRole, Qt::UserRole + 1, Qt::UserRole + 2, Qt::UserRole + 3});
}

bool CameraCalibrationModel::hasCalibrationPermission()
{
    if (!m_calibrationDeviceProxy)
    {
        return false;
    }
    if (!UserManagement::instance()->hasPermission(int(m_calibrationDeviceProxy->writePermission())))
    {
        return false;
    }
    if (!UserManagement::instance()->hasPermission(int(Permission::EditCalibrationDeviceConfig)))
    {
        return false;
    }
    return true;
}

bool CameraCalibrationModel::hasWorkflowPermission()
{
    if (!m_workflowDeviceProxy)
    {
        return false;
    }
    if (!UserManagement::instance()->hasPermission(int(m_workflowDeviceProxy->writePermission())))
    {
        return false;
    }
    if (!UserManagement::instance()->hasPermission(int(Permission::EditWorkflowDeviceConfig)))
    {
        return false;
    }
    return true;
}

void CameraCalibrationModel::setTargetWidth(double value)
{
    if (value == m_targetWidth)
    {
        return;
    }
    if (!m_calibrationReady || !hasCalibrationPermission())
    {
        return;
    }
    m_targetWidth = value;
    auto updateValue = [this]()
    {
        m_calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<double>(std::string("GrooveWidth"), m_targetWidth)));
    };
    updateCalibrationDevice(updateValue);

    emit targetWidthChanged();
}

void CameraCalibrationModel::setTargetHeight(double value)
{
    if (value == m_targetHeight)
    {
        return;
    }
    if (!m_calibrationReady || !hasCalibrationPermission())
    {
        return;
    }
    m_targetHeight = value;
    auto updateValue = [this]()
    {
        m_calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<double>(std::string("GrooveDepth"), m_targetHeight)));
    };
    updateCalibrationDevice(updateValue);
    emit targetHeightChanged();
}

}
}
