#pragma once

#include "common/ethercat.h"

#include <QAbstractListModel>

#include <vector>

namespace precitec
{
namespace gui
{

class ServiceToGuiServer;

namespace components
{
namespace ethercat
{

/**
 * Model providing information about ethercat slaves in the system.
 * Requires the service property to be set.
 *
 * The model provides the following roles:
 * @li display
 * @li vendorId
 * @li productCode
 * @li instance
 * @li latestInputData (latest ethercat input data)
 * @li latestOutputData (latest ethercat output data)
 * @li type
 **/
class SlaveInfoModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(precitec::gui::ServiceToGuiServer *service READ service WRITE setService NOTIFY serviceChanged)
public:
    SlaveInfoModel(QObject *parent = nullptr);
    ~SlaveInfoModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    QHash<int, QByteArray> roleNames() const override;

    ServiceToGuiServer *service() const
    {
        return m_service;
    }
    void setService(ServiceToGuiServer *service);

    /**
     * The type of Slaves
     **/
    enum class Type {
        Unknown,
        Gateway,
        DigitalIn,
        DigitalOut,
        /**
         * Range -10 to 10 V
         **/
        AnalogIn,
        /**
         * Range -10 to 10 V, n times
         **/
        AnalogOversamplingIn,
        /**
         * Range 0 to 10 V
         **/
        AnalogOut0To10,
        /**
         * Range -10 to 10 V
         **/
        AnalogOutPlusMinus10
    };
    Q_ENUM(Type)

    /**
     * Sets the output bit @p byteIndex / @p bitIndex of the slave identified by @p index to the new value @p state.
     **/
    Q_INVOKABLE void setOutputBit(const QModelIndex &index, int byteIndex, int bitIndex, bool state);

    /**
     * Retrieves the Slave Info at given @p index.
     **/
    EC_T_GET_SLAVE_INFO slaveInfo(const QModelIndex &index) const;

Q_SIGNALS:
    void serviceChanged();

private:
    void initSlaveInfo();
    ServiceToGuiServer *m_service = nullptr;
    QMetaObject::Connection m_serviceDestroyed;
    QMetaObject::Connection m_serviceInfo;
    QMetaObject::Connection m_serviceData;
    std::vector<EC_T_GET_SLAVE_INFO> m_slaveInfo;
};

}
}
}
}

Q_DECLARE_METATYPE(precitec::gui::components::ethercat::SlaveInfoModel*)
