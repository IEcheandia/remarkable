#include "abstractParameterSetDeltaModel.h"
#include "attribute.h"
#include "attributeModel.h"
#include "graphModel.h"
#include "parameterSet.h"
#include "subGraphModel.h"
#include "seam.h"
#include "seamSeries.h"
#include "product.h"
#include "filterAttributeSortFilterModel.h"

#include "../../App_Storage/src/compatibility.h"

namespace precitec
{

using storage::ParameterSet;
using storage::Seam;

using namespace storage::compatibility;

namespace gui
{

AbstractParameterSetDeltaModel::AbstractParameterSetDeltaModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    connect(this, &AbstractParameterSetDeltaModel::seamChanged, this,
        [this]
        {
            if (m_seam)
            {
                init();
                return;
            }
            beginResetModel();
            setParameterSets({});
            m_graph = {};
            endResetModel();
        }
    );
    connect(this, &AbstractParameterSetDeltaModel::graphModelChanged, this, &AbstractParameterSetDeltaModel::init);
    connect(this, &AbstractParameterSetDeltaModel::subGraphModelChanged, this, &AbstractParameterSetDeltaModel::init);
    connect(this, &AbstractParameterSetDeltaModel::attributeModelChanged, this, &AbstractParameterSetDeltaModel::init);
}

AbstractParameterSetDeltaModel::~AbstractParameterSetDeltaModel() = default;

QVariant AbstractParameterSetDeltaModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    if (role == Qt::DisplayRole)
    {
        ParameterSet *set = m_parameterSets.at(index.column());
        return QVariant::fromValue(set->parameters().at(index.row()));
    }
    return {};
}

int AbstractParameterSetDeltaModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_parameterSets.empty() ? 0 : m_parameterSets.front()->parameters().size();
}

int AbstractParameterSetDeltaModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_parameterSets.size();
}

void AbstractParameterSetDeltaModel::setSeam(Seam *seam)
{
    if (m_seam == seam)
    {
        return;
    }
    m_seam = seam;
    disconnect(m_seamDestroyed);
    if (m_seam)
    {
        m_seamDestroyed = connect(m_seam, &QObject::destroyed, this, std::bind(&AbstractParameterSetDeltaModel::setSeam, this, nullptr));
    }
    else
    {
        m_seamDestroyed = {};
    }
    emit seamChanged();
}

void AbstractParameterSetDeltaModel::setGraphModel(storage::GraphModel *graphModel)
{
    if (m_graphModel == graphModel)
    {
        return;
    }
    m_graphModel = graphModel;
    disconnect(m_graphModelDestroyed);
    if (m_graphModel)
    {
        m_graphModelDestroyed = connect(m_graphModel, &QObject::destroyed, this, std::bind(&AbstractParameterSetDeltaModel::setGraphModel, this, nullptr));
    }
    else
    {
        m_graphModelDestroyed = {};
    }
    emit graphModelChanged();
}

void AbstractParameterSetDeltaModel::setSubGraphModel(storage::SubGraphModel *subGraphModel)
{
    if (m_subGraphModel == subGraphModel)
    {
        return;
    }
    m_subGraphModel = subGraphModel;
    disconnect(m_subGraphModelDestroyed);
    if (m_subGraphModel)
    {
        m_subGraphModelDestroyed = connect(m_subGraphModel, &QObject::destroyed, this, std::bind(&AbstractParameterSetDeltaModel::setSubGraphModel, this, nullptr));
    }
    else
    {
        m_subGraphModelDestroyed = {};
    }
    emit subGraphModelChanged();
}

void AbstractParameterSetDeltaModel::setAttributeModel(storage::AttributeModel *attributeModel)
{
    if (m_attributeModel == attributeModel)
    {
        return;
    }
    m_attributeModel = attributeModel;
    disconnect(m_attributeModelDestroyed);
    if (m_attributeModel)
    {
        m_attributeModelDestroyed = connect(m_attributeModel, &QObject::destroyed, this, std::bind(&AbstractParameterSetDeltaModel::setAttributeModel, this, nullptr));
    }
    else
    {
        m_attributeModelDestroyed = {};
    }
    emit attributeModelChanged();
}

QVariant AbstractParameterSetDeltaModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal || m_parameterSets.empty())
    {
        return {};
    }
    auto *parameterSet = m_parameterSets.front();
    if (section < 0 || section >= int(parameterSet->parameters().size()))
    {
        return {};
    }
    auto *parameter = parameterSet->parameters().at(section);
    const Poco::UUID instanceFilterId{toPoco(parameter->filterId())};
    const auto filterIt = std::find_if(m_graph.instanceFilters.begin(), m_graph.instanceFilters.end(), [&instanceFilterId] (const auto &filter) { return filter.id == instanceFilterId; });
    if (filterIt == m_graph.instanceFilters.end())
    {
        return {};
    }
    switch (role)
    {
    case Qt::DisplayRole:
        return QString::fromStdString(filterIt->name);
    case Qt::UserRole:
    case Qt::UserRole + 5:
    {
        const auto groupIt = std::find_if(m_graph.filterGroups.begin(), m_graph.filterGroups.end(), [filterIt] (const auto &group) { return group.number == filterIt->group; });
        if (groupIt == m_graph.filterGroups.end())
        {
            return tr("Not grouped");
        }
        if (role == Qt::UserRole)
        {
            return QString::fromStdString(groupIt->name);
        }
        else if (role == Qt::UserRole + 5)
        {
            return m_subGraphModel->indexFor(storage::compatibility::toQt(groupIt->sourceGraph)).data();
        }
        Q_UNREACHABLE();
    }
    case Qt::UserRole + 1:
        return QString::fromStdString(filterIt->filterId.toString()).toLower();
    case Qt::UserRole + 2:
    case Qt::UserRole + 3:
    case Qt::UserRole + 4:
    {
        const Poco::UUID instanceAttributeId{toPoco(parameter->uuid())};
        const auto attributeIt = std::find_if(filterIt->attributes.begin(), filterIt->attributes.end(), [&instanceAttributeId] (const auto &attribute) { return attribute.instanceAttributeId == instanceAttributeId; });
        if (attributeIt == filterIt->attributes.end())
        {
            return {};
        }
        auto *attribute = m_attributeModel->findAttribute(toQt(attributeIt->attributeId));
        if (role == Qt::UserRole + 2)
        {
            // the Attribute, may be null
            return QVariant::fromValue(attribute);
        }
        else if (role == Qt::UserRole + 3)
        {
            // whether the current user can edit this attribute
            return FilterAttributeSortFilterModel::checkUserLevel(attributeIt->userLevel);
        }
        else if (role == Qt::UserRole + 4 && attribute)
        {
            // the default value of the attribute in the filter graph
            return attribute->convert(attributeIt->value);
        }
        break;
    }
    }
    return {};
}

void AbstractParameterSetDeltaModel::setParameterSets(std::vector<storage::ParameterSet*> &&sets)
{
    for (auto *ps : m_parameterSets)
    {
        ps->deleteLater();
    }
    m_parameterSets = std::move(sets);
}

}
}
