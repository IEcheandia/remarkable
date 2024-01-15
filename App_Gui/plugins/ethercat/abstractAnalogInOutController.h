#pragma once

#include "common/ethercat.h"

#include <QObject>
#include <QModelIndex>
#include <QPointF>

#include <memory>

class QMutex;

namespace precitec
{
namespace gui
{

class ServiceToGuiServer;

namespace components
{
namespace ethercat
{

class SlaveInfoModel;

/**
 * Abstract base class for analog in/out EtherCAT slaves.
 **/
class AbstractAnalogInOutController : public QObject
{
    Q_OBJECT
    /**
     * Model containing all Ethercat slaves
     **/
    Q_PROPERTY(precitec::gui::components::ethercat::SlaveInfoModel *model READ model WRITE setModel NOTIFY modelChanged)
    /**
     * Index in SlaveInfoModel referencing the analog in slave.
     **/
    Q_PROPERTY(QModelIndex index READ index WRITE setIndex NOTIFY indexChanged)
    /**
     * Whether this controller is monitoring EtherCAT data. Default is @c false
     **/
    Q_PROPERTY(bool monitoring READ isMonitoring WRITE setMonitoring NOTIFY monitoringChanged)
public:
    ~AbstractAnalogInOutController() override;

    SlaveInfoModel *model() const
    {
        return m_model;
    }
    void setModel(SlaveInfoModel *model);

    QModelIndex index() const
    {
        return m_index;
    }
    void setIndex(const QModelIndex &index);

    bool isMonitoring() const
    {
        return m_monitoring;
    }
    void setMonitoring(bool set);

Q_SIGNALS:
    void modelChanged();
    void indexChanged();
    void monitoringChanged();
    /**
     * internal signal
     **/
    void serviceChanged();
    void slaveChanged();

protected:
    AbstractAnalogInOutController(QObject *parent = nullptr);
    virtual void fetchData() = 0;

    QMutex *mutex();
    ServiceToGuiServer *service() const
    {
        return m_service;
    }
    const EC_T_GET_SLAVE_INFO &slave() const
    {
        return m_slave;
    }

private:
    void initSlave();
    void initService();
    void initServiceConnection();
    SlaveInfoModel *m_model = nullptr;
    ServiceToGuiServer *m_service = nullptr;
    QModelIndex m_index;
    EC_T_GET_SLAVE_INFO m_slave;
    std::unique_ptr<QMutex> m_mutex;
    bool m_monitoring = false;
    QMetaObject::Connection m_serviceConnection;
};

}
}
}
}
