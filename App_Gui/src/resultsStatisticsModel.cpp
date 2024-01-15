#include "resultsStatisticsModel.h"

#include <QMetaEnum>

namespace precitec
{
namespace gui
{

ResultsStatisticsModel::ResultsStatisticsModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

ResultsStatisticsModel::~ResultsStatisticsModel() = default;

QHash<int, QByteArray> ResultsStatisticsModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {Qt::DecorationRole, QByteArrayLiteral("icon")},
        {Qt::UserRole, QByteArrayLiteral("enabled")},
        {Qt::UserRole + 1, QByteArrayLiteral("component")}
    };
}

int ResultsStatisticsModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return QMetaEnum::fromType<ResultsComponent>().keyCount();
}

QVariant ResultsStatisticsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    const auto component = static_cast<ResultsComponent>(index.row());

    switch (role)
    {
    case Qt::DisplayRole:
        return name(component);
    case Qt::DecorationRole:
        return iconName(component);
    case Qt::UserRole:
        return component <= m_activeLevel;
    case Qt::UserRole + 1:
        return QVariant::fromValue(component);
    default:
        return {};
    }
}

QString ResultsStatisticsModel::name(ResultsComponent component) const
{
    switch (component)
    {
    case ResultsComponent::Product:
        return tr("Product");
    case ResultsComponent::Instance:
        return tr("Instance Statistics");
    case ResultsComponent::Series:
        return tr("Series Statistics");
    case ResultsComponent::Seams:
        return tr("Seams Statistics");
    case ResultsComponent::Seam:
        return tr("Seam Statistics");
    case ResultsComponent::LinkedSeam:
        return tr("Linked Seam Statistics");
    default:
        return {};
    }
}

QString ResultsStatisticsModel::iconName(ResultsComponent component) const
{
    switch (component)
    {
    case ResultsComponent::Product:
        return QStringLiteral("select-product");
    case ResultsComponent::Instance:
        return QStringLiteral("select-assembly");
    case ResultsComponent::Series:
        return QStringLiteral("select-seam-series");
    case ResultsComponent::Seams:
        return QStringLiteral("select-seam");
    case ResultsComponent::Seam:
    case ResultsComponent::LinkedSeam:
        return QStringLiteral("view-plot");
    default:
        return QString();
    }
}

void ResultsStatisticsModel::setActiveLevel(ResultsStatisticsModel::ResultsComponent level)
{
    if (m_activeLevel == level)
    {
        return;
    }
    m_activeLevel = level;
    emit dataChanged(index(0,0), index(rowCount() - 1, 0), {Qt::UserRole});
    emit levelChanged();
}

}
}

