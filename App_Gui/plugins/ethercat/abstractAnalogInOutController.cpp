#include "abstractAnalogInOutController.h"
#include "slaveInfoModel.h"
#include "../../src/serviceToGuiServer.h"

#include <precitec/dataSet.h>

#include <QMutex>
#include <QMutexLocker>
#include <QPointF>

#include <cmath>

using precitec::gui::components::plotter::DataSet;

namespace precitec
{
namespace gui
{
namespace components
{
namespace ethercat
{

AbstractAnalogInOutController::AbstractAnalogInOutController(QObject *parent)
    : QObject(parent)
    , m_mutex(std::make_unique<QMutex>())
{
    connect(this, &AbstractAnalogInOutController::modelChanged, this, &AbstractAnalogInOutController::initSlave);
    connect(this, &AbstractAnalogInOutController::modelChanged, this, &AbstractAnalogInOutController::initService, Qt::QueuedConnection);
    connect(this, &AbstractAnalogInOutController::indexChanged, this, &AbstractAnalogInOutController::initSlave);
    connect(this, &AbstractAnalogInOutController::monitoringChanged, this, &AbstractAnalogInOutController::initServiceConnection);
    connect(this, &AbstractAnalogInOutController::serviceChanged, this, &AbstractAnalogInOutController::initServiceConnection);
}

AbstractAnalogInOutController::~AbstractAnalogInOutController() = default;

void AbstractAnalogInOutController::setIndex(const QModelIndex &index)
{
    if (m_index == index)
    {
        return;
    }
    m_index = index;
    emit indexChanged();
}

void AbstractAnalogInOutController::setModel(SlaveInfoModel *model)
{
    if (m_model == model)
    {
        return;
    }
    m_model = model;
    emit modelChanged();
}

void AbstractAnalogInOutController::initSlave()
{
    if (!m_model || !m_index.isValid())
    {
        m_slave = {};
        return;
    }
    m_slave = m_model->slaveInfo(m_index);
    emit slaveChanged();
}

void AbstractAnalogInOutController::initService()
{
    QMutexLocker lock{m_mutex.get()};
    if (!m_model)
    {
        m_service = nullptr;
    } else
    {
        m_service = m_model->service();
    }
    emit serviceChanged();
}

void AbstractAnalogInOutController::initServiceConnection()
{
    disconnect(m_serviceConnection);
    if (m_service && m_monitoring)
    {
        m_serviceConnection = connect(m_service, &ServiceToGuiServer::processDataChanged, this, &AbstractAnalogInOutController::fetchData, Qt::QueuedConnection);
    } else
    {
        m_serviceConnection = {};
    }
}

void AbstractAnalogInOutController::setMonitoring(bool set)
{
    if (m_monitoring == set)
    {
        return;
    }
    m_monitoring = set;
    emit monitoringChanged();
}

QMutex *AbstractAnalogInOutController::mutex()
{
    return m_mutex.get();
}

}
}
}
}
