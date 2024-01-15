#include "FilterPortPartnerModel.h"
#include <QDebug>

namespace precitec
{

namespace gui
{

namespace components
{

namespace grapheditor
{

FilterPortPartnerModel::FilterPortPartnerModel(QObject* parent)
    : QAbstractListModel(parent)
{   }

FilterPortPartnerModel::~FilterPortPartnerModel() = default;

FilterPort* FilterPortPartnerModel::actualPort() const
{
    return m_actualPort;
}

void FilterPortPartnerModel::setActualPort(FilterPort* port)
{
    if (m_actualPort != port)
    {
        m_actualPort = port;
        if (m_actualPort)
        {
            m_destroyConnection = connect(m_actualPort, &FilterPort::destroyed, this, std::bind(&FilterPortPartnerModel::setActualPort, this, nullptr));
            connect(this, &FilterPortPartnerModel::actualPortChanged, this, &FilterPortPartnerModel::init);
            connect(m_actualPort, &FilterPort::partnerSizeChanged, this, &FilterPortPartnerModel::init);
        }
        emit actualPortChanged();
    }
}

QHash<int, QByteArray> FilterPortPartnerModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("id")},
        {Qt::UserRole, QByteArrayLiteral("name")},
        {Qt::UserRole + 1, QByteArrayLiteral("group")},
        {Qt::UserRole + 2, QByteArrayLiteral("portType")},
        {Qt::UserRole + 3, QByteArrayLiteral("position")},
        {Qt::UserRole + 4, QByteArrayLiteral("dataType")}
    };
}

int FilterPortPartnerModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_partners.size();
}

QVariant FilterPortPartnerModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || m_partners.size() == 0 || !m_actualPort)
    {
        return {};
    }

    const auto &portPartnerInfo = m_partners.at(index.row());

    switch (role)
    {
        case Qt::DisplayRole:
            return portPartnerInfo->ID();
        case Qt::UserRole:
            return portPartnerInfo->getLabel();
        case Qt::UserRole + 1:
            return portPartnerInfo->groupID();
        case Qt::UserRole + 2:
            return portPartnerInfo->type();
        case Qt::UserRole + 3:
            return portPartnerInfo->getItemGeometry();
        case Qt::UserRole + 4:
            return portPartnerInfo->dataType();
    }

    return {};
}

QObject* FilterPortPartnerModel::getPartner(int index)
{
    if (m_partners.size() == 0)
    {
        return {};
    }
    return m_partners.at(index);
}

void FilterPortPartnerModel::init()
{
    emit beginResetModel();
    if (!m_actualPort)
    {
        m_partners.clear();
        emit endResetModel();
        return;
    }
    m_partners = m_actualPort->getFilterPortPartner();
    emit endResetModel();
}

}
}
}
}

