#include "slaveInfoModel.h"
#include "../../src/serviceToGuiServer.h"

#include "event/ethercatDefines.h"

namespace precitec
{
namespace gui
{
namespace components
{
namespace ethercat
{

SlaveInfoModel::SlaveInfoModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &SlaveInfoModel::serviceChanged, this, &SlaveInfoModel::initSlaveInfo);
}

SlaveInfoModel::~SlaveInfoModel() = default;

int SlaveInfoModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_slaveInfo.size();
}

QHash<int, QByteArray> SlaveInfoModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {Qt::UserRole, QByteArrayLiteral("vendorId")},
        {Qt::UserRole + 1, QByteArrayLiteral("productCode")},
        {Qt::UserRole + 2, QByteArrayLiteral("instance")},
        {Qt::UserRole + 3, QByteArrayLiteral("latestInputData")},
        {Qt::UserRole + 4, QByteArrayLiteral("latestOutputData")},
        {Qt::UserRole + 5, QByteArrayLiteral("type")},
        {Qt::UserRole + 6, QByteArrayLiteral("inputSize")},
        {Qt::UserRole + 7, QByteArrayLiteral("outputSize")}
    };
}

QVariant SlaveInfoModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    const auto &slaveInfo = m_slaveInfo.at(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
        return QString::fromLocal8Bit(slaveInfo.abyDeviceName);
    case Qt::UserRole:
        return slaveInfo.dwVendorId;
    case Qt::UserRole + 1:
        return slaveInfo.dwProductCode;
    case Qt::UserRole + 2:
    {
        auto endIt = std::begin(m_slaveInfo);
        std::advance(endIt, index.row());
        return static_cast<int>(std::count_if(m_slaveInfo.begin(), endIt, [&slaveInfo] (const auto &s) { return slaveInfo.dwVendorId == s.dwVendorId && slaveInfo.dwProductCode == s.dwProductCode;})) + 1;
    }
    case Qt::UserRole + 3: {
        if (!m_service)
        {
            return {};
        }
        const auto data = m_service->inputData(slaveInfo);
        if (data.empty())
        {
            return {};
        }
        return data.back();
    }
    case Qt::UserRole + 4: {
        if (!m_service)
        {
            return {};
        }
        const auto data = m_service->outputData(slaveInfo);
        if (data.empty())
        {
            return {};
        }
        return data.back();
    }
    case Qt::UserRole + 5:
        if (slaveInfo.dwVendorId == VENDORID_HMS && slaveInfo.dwProductCode == PRODUCTCODE_ANYBUS_GW)
        {
            return QVariant::fromValue(Type::Gateway);
        }
        if (slaveInfo.dwVendorId == VENDORID_KUNBUS && slaveInfo.dwProductCode == PRODUCTCODE_KUNBUS_GW)
        {
            return QVariant::fromValue(Type::Gateway);
        }
        if (slaveInfo.dwVendorId == VENDORID_HILSCHER && slaveInfo.dwProductCode == PRODUCTCODE_FIELDBUS)
        {
            return QVariant::fromValue(Type::Gateway);
        }
        if (slaveInfo.dwVendorId == VENDORID_BECKHOFF && slaveInfo.dwProductCode == PRODUCTCODE_EL1018)
        {
            return QVariant::fromValue(Type::DigitalIn);
        }
        if (slaveInfo.dwVendorId == VENDORID_BECKHOFF && slaveInfo.dwProductCode == PRODUCTCODE_EL2008)
        {
            return QVariant::fromValue(Type::DigitalOut);
        }
        if (slaveInfo.dwVendorId == VENDORID_BECKHOFF && slaveInfo.dwProductCode == PRODUCTCODE_EL3102)
        {
            return QVariant::fromValue(Type::AnalogIn);
        }
        if (slaveInfo.dwVendorId == VENDORID_BECKHOFF && slaveInfo.dwProductCode == PRODUCTCODE_EL3702)
        {
            return QVariant::fromValue(Type::AnalogOversamplingIn);
        }
        if (slaveInfo.dwVendorId == VENDORID_BECKHOFF && slaveInfo.dwProductCode == PRODUCTCODE_EL4102)
        {
            return QVariant::fromValue(Type::AnalogOut0To10);
        }
        if (slaveInfo.dwVendorId == VENDORID_BECKHOFF && slaveInfo.dwProductCode == PRODUCTCODE_EL4132)
        {
            return QVariant::fromValue(Type::AnalogOutPlusMinus10);
        }
        return QVariant::fromValue(Type::Unknown);
    case Qt::UserRole + 6:
        return slaveInfo.dwPdSizeIn;
    case Qt::UserRole + 7:
        return slaveInfo.dwPdSizeOut;
    }
    return {};
}

void SlaveInfoModel::setService(ServiceToGuiServer *service)
{
    if (m_service == service)
    {
        return;
    }
    m_service = service;
    disconnect(m_serviceDestroyed);
    disconnect(m_serviceInfo);
    disconnect(m_serviceData);
    if (m_service)
    {
        m_serviceInfo = connect(m_service, &ServiceToGuiServer::slaveInfoChanged, this, &SlaveInfoModel::initSlaveInfo, Qt::QueuedConnection);
        m_serviceDestroyed = connect(m_service, &QObject::destroyed, this, std::bind(&SlaveInfoModel::setService, this, nullptr));
        m_serviceData = connect(m_service, &ServiceToGuiServer::processDataChanged, this,
            [this]
            {
                emit dataChanged(index(0, 0), index(rowCount() - 1, 0), {Qt::UserRole + 3, Qt::UserRole + 4});
            }, Qt::QueuedConnection
        );
    } else
    {
        m_serviceDestroyed = {};
        m_serviceInfo = {};
        m_serviceData = {};
    }
    emit serviceChanged();
}

void SlaveInfoModel::initSlaveInfo()
{
    if (!m_service)
    {
        if (!m_slaveInfo.empty())
        {
            beginResetModel();
            m_slaveInfo.clear();
            endResetModel();
        }
        return;
    }
    beginResetModel();
    m_slaveInfo = m_service->slaveInfo();
    endResetModel();
}

void SlaveInfoModel::setOutputBit(const QModelIndex &index, int byteIndex, int bitIndex, bool state)
{
    if (!m_service)
    {
        return;
    }
    if (std::size_t(index.row()) >= m_slaveInfo.size())
    {
        return;
    }
    m_service->setOutputBit(m_slaveInfo.at(index.row()), byteIndex, bitIndex, state);
}

EC_T_GET_SLAVE_INFO SlaveInfoModel::slaveInfo(const QModelIndex &index) const
{
    if (!index.isValid() || std::size_t(index.row()) >= m_slaveInfo.size())
    {
        return {};
    }
    return m_slaveInfo.at(index.row());
}

}
}
}
}
