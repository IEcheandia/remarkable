#include "abstractFilterAttributeModel.h"
#include "attributeGroup.h"
#include "attributeGroupItem.h"
#include "attributeModel.h"
#include "attribute.h"
#include "parameterSet.h"
#include "parameter.h"
#include "../../App_Storage/src/compatibility.h"

namespace precitec
{
namespace storage
{

AbstractFilterAttributeModel::AbstractFilterAttributeModel(QObject* parent)
    : QAbstractListModel(parent)
{
    connect(this, &AbstractFilterAttributeModel::filterInstanceChanged, this, &AbstractFilterAttributeModel::updateModel);
    connect(this, &AbstractFilterAttributeModel::graphChanged, this, &AbstractFilterAttributeModel::updateModel);
    connect(this, &AbstractFilterAttributeModel::attributeModelChanged, this, &AbstractFilterAttributeModel::updateModel);
}

AbstractFilterAttributeModel::~AbstractFilterAttributeModel() = default;

int AbstractFilterAttributeModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_attributeGroups.size();
}

QHash<int, QByteArray> AbstractFilterAttributeModel::roleNames() const
{
    // editListOrder, userLevel and visible roles used in FilterAttributeSortFilterModel
    return {
        {Qt::DisplayRole, QByteArrayLiteral("contentName")},
        {Qt::UserRole, QByteArrayLiteral("unit")},
        {Qt::UserRole + 1, QByteArrayLiteral("description")},
        {Qt::UserRole + 2, QByteArrayLiteral("minValue")},
        {Qt::UserRole + 3, QByteArrayLiteral("maxValue")},
        {Qt::UserRole + 4, QByteArrayLiteral("defaultValue")},
        {Qt::UserRole + 5, QByteArrayLiteral("userLevel")},
        {Qt::UserRole + 6, QByteArrayLiteral("helpFile")},
        {Qt::UserRole + 7, QByteArrayLiteral("selected")},
        {Qt::UserRole + 8, QByteArrayLiteral("editListOrder")},
        {Qt::UserRole + 9, QByteArrayLiteral("visible")},
        {Qt::UserRole + 10, QByteArrayLiteral("publicity")},
        {Qt::UserRole + 11, QByteArrayLiteral("group")}
    };
}

QVariant AbstractFilterAttributeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    const auto& group = m_attributeGroups.at(index.row());

    switch (role)
    {
    case Qt::DisplayRole:
        return group->contentName();
    case Qt::UserRole:
        return group->unit();
    case Qt::UserRole + 1:
        return group->description();
    case Qt::UserRole + 2:
        return group->minValue();
    case Qt::UserRole + 3:
        return group->maxValue();
    case Qt::UserRole + 4:
        return group->defaultValue();
    case Qt::UserRole + 5:
        return group->userLevel();
    case Qt::UserRole + 6:
        return group->helpFile();
    case Qt::UserRole + 7:
        return group->selected();
    case Qt::UserRole + 8:
        return group->editListOrder();
    case Qt::UserRole + 9:
        return group->visible();
    case Qt::UserRole + 10:
        return group->publicity();
    case Qt::UserRole + 11:
        return QVariant::fromValue(group);
    }

    return {};
}

bool AbstractFilterAttributeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || (role != Qt::UserRole + 5 && role != Qt::UserRole + 7 && role != Qt::UserRole + 10))
    {
        return false;
    }

    auto& group = m_attributeGroups.at(index.row());

    switch (role)
    {
    case Qt::UserRole + 5:
        group->setUserLevel(value.toInt());
        break;
    case Qt::UserRole + 7:
        group->setSelected(value.toBool());
        break;
    case Qt::UserRole + 10:
        group->setPublicity(value.toBool());
        break;
    }

    emit dataChanged(index, index, {role});

    return true;
}

Qt::ItemFlags AbstractFilterAttributeModel::flags(const QModelIndex& index) const
{
    Q_UNUSED(index)
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

void AbstractFilterAttributeModel::setFilterInstance(const QUuid& id)
{
    if (m_filterInstance == id)
    {
        return;
    }
    m_filterInstance = id;
    emit filterInstanceChanged();
}

void AbstractFilterAttributeModel::setAttributeModel(AttributeModel* attributeModel)
{
    if (m_attributeModel == attributeModel)
    {
        return;
    }

    if (m_attributeModel)
    {
        disconnect(m_attributeModelDestroyedConnection);
        disconnect(m_attributeModel, &AttributeModel::modelReset, this, &AbstractFilterAttributeModel::updateModel);
    }

    m_attributeModel = attributeModel;

    if (m_attributeModel)
    {
        m_attributeModelDestroyedConnection = connect(m_attributeModel, &QObject::destroyed, this, std::bind(&AbstractFilterAttributeModel::setAttributeModel, this, nullptr));
        connect(m_attributeModel, &AttributeModel::modelReset, this, &AbstractFilterAttributeModel::updateModel);
    } else
    {
        m_attributeModelDestroyedConnection = {};
    }

    emit attributeModelChanged();
}

void AbstractFilterAttributeModel::constructAttributeGroups()
{
    qDeleteAll(m_attributeGroups);
    m_attributeGroups.clear();

    if (m_attributeModel && !m_filterInstance.isNull())
    {
        const auto& pocoId = precitec::storage::compatibility::toPoco(m_filterInstance);
        auto& instanceFilters = graph().instanceFilters;

        /**
         * Create pairs of
         *  - filter attribute instances, taken from the filter instance of the current graph
         *  - attribute templates, taken from the AttributeModel
         * and group them according to their groupId & groupIndex.
         * Attributes, which have no group (i.e. groupId is empty), are assigned to an empty group each
         **/
        if (auto it = std::find_if(instanceFilters.begin(), instanceFilters.end(), [&pocoId] (const auto& filter) { return filter.id == pocoId; }); it != instanceFilters.end())
        {
            auto& filter = *it;

            std::vector<std::pair<precitec::storage::Attribute*, fliplib::InstanceFilter::Attribute*>> attributes;
            for (auto& attribute : filter.attributes)
            {
                attributes.emplace_back(m_attributeModel->findAttribute(precitec::storage::compatibility::toQt(attribute.attributeId)), &attribute);
            }

            for (const auto& attributePair : attributes)
            {
                if (!attributePair.first || attributePair.first->groupId().isNull())
                {
                    m_attributeGroups.emplace_back(new AttributeGroup{attributePair.first, attributePair.second, this});
                } else
                {
                    auto groupIt = std::find_if(m_attributeGroups.begin(), m_attributeGroups.end(), [&attributePair] (auto group) {
                        return group->groupId() == attributePair.first->groupId();
                    });

                    if (groupIt == m_attributeGroups.end())
                    {
                        m_attributeGroups.emplace_back(new AttributeGroup{attributePair.first, attributePair.second, this});
                    } else
                    {
                        (*groupIt)->addItem(attributePair.first, attributePair.second);
                    }
                }
            }
        }
    }
}

QModelIndex AbstractFilterAttributeModel::indexForAttributeInstance(const QUuid &instanceId) const
{
    for (std::size_t i = 0; i < m_attributeGroups.size(); i++)
    {
        auto group = m_attributeGroups.at(i);
        const auto &items = group->items();
        if (std::any_of(items.begin(), items.end(), [instanceId] (auto item) { return item->instanceId() == instanceId; }))
        {
            return index(i, 0);
        }
    }
    return {};
}

}
}

