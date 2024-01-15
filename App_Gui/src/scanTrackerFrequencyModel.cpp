#include "scanTrackerFrequencyModel.h"

namespace precitec
{

namespace gui
{

ScanTrackerFrequencyModel::ScanTrackerFrequencyModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

ScanTrackerFrequencyModel::~ScanTrackerFrequencyModel() = default;

QVariant ScanTrackerFrequencyModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    auto it = m_frequencies.find(index.row());
    if (it == m_frequencies.end())
    {
        return {};
    }
    switch (role)
    {
    case Qt::DisplayRole:
        return tr("%1 Hz").arg(it->second);
    case Qt::UserRole:
        return it->first;
    case Qt::UserRole + 1:
        return it->second;
    }
    return {};
}

int ScanTrackerFrequencyModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_frequencies.size();
}

}
}
