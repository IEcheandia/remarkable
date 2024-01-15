#include "resultsModel.h"

#include <QMetaEnum>

namespace precitec
{
namespace gui
{

ResultsModel::ResultsModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

ResultsModel::~ResultsModel() = default;

QHash<int, QByteArray> ResultsModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {Qt::DecorationRole, QByteArrayLiteral("icon")},
        {Qt::UserRole, QByteArrayLiteral("enabled")},
        {Qt::UserRole + 1, QByteArrayLiteral("component")}
    };
}

int ResultsModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return QMetaEnum::fromType<ResultsComponent>().keyCount();
}

QVariant ResultsModel::data(const QModelIndex &index, int role) const
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

QString ResultsModel::name(ResultsComponent component) const
{
    switch (component)
    {
    case ResultsComponent::Product:
        return tr("Product");
    case ResultsComponent::Instance:
        return tr("Instance");
    case ResultsComponent::Series:
        return tr("Series");
    case ResultsComponent::Seam:
        return tr("Seam");
    case ResultsComponent::Results:
        return tr("Results");
    default:
        return {};
    }
}

QString ResultsModel::iconName(ResultsComponent component) const
{
    switch (component)
    {
    case ResultsComponent::Product:
        return QStringLiteral("select-product");
    case ResultsComponent::Instance:
        return QStringLiteral("select-assembly");
    case ResultsComponent::Series:
        return QStringLiteral("select-seam-series");
    case ResultsComponent::Seam:
        return QStringLiteral("select-seam");
    case ResultsComponent::Results:
        return QStringLiteral("view-plot");
    default:
        return QString();
    }
}

void ResultsModel::setActiveLevel(ResultsModel::ResultsComponent level)
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

