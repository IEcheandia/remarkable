#include "seamPropertyModel.h"
#include "copyMode.h"
#include "product.h"
#include "seam.h"
#include "seamSeries.h"
#include "seamInterval.h"
#include "parameterSet.h"
#include "parameter.h"
#include "graphFunctions.h"
#include "graphModel.h"
#include "subGraphModel.h"
#include "filterAttributeModel.h"
#include "attributeModel.h"
#include "attribute.h"
#include "attributeGroup.h"
#include "attributeGroupItem.h"
#include "../../App_Storage/src/compatibility.h"

using precitec::storage::CopyMode;
using precitec::storage::Seam;
using precitec::storage::GraphModel;
using precitec::storage::SubGraphModel;
using precitec::storage::FilterAttributeModel;
using precitec::storage::graphFunctions::getGraphFromModel;

namespace precitec
{
namespace gui
{

static std::map<SeamPropertyModel::Property, std::tuple<QString, bool, bool>> s_propertyKeys = {
    {SeamPropertyModel::Property::Name, {QStringLiteral("Name"), false, false}},
    {SeamPropertyModel::Property::Length, {QStringLiteral("Length"), false, false}},
    {SeamPropertyModel::Property::RateOfFeed, {QStringLiteral("Rate Of Feed"), false, false}},
    {SeamPropertyModel::Property::TriggerDistance, {QStringLiteral("Trigger Distance"), false, false}},
    {SeamPropertyModel::Property::MovingDirection, {QStringLiteral("Moving Direction"), false, false}},
    {SeamPropertyModel::Property::ThicknessLeft, {QStringLiteral("Thickness Left"), false, false}},
    {SeamPropertyModel::Property::ThicknessRight, {QStringLiteral("Thickness Right"), false, false}},
    {SeamPropertyModel::Property::TargetDistance, {QStringLiteral("Target Distance"), false, false}},
    {SeamPropertyModel::Property::PositionInAssemblyImage, {QStringLiteral("Position In Assembly Image"), false, false}},
    {SeamPropertyModel::Property::HardwareParameters, {QStringLiteral("Hardware Parameters"), false, false}},
    {SeamPropertyModel::Property::Detection, {QStringLiteral("Detection Algorithm"), false, false}},
    {SeamPropertyModel::Property::FilterParameters, {QStringLiteral("Filter Parameters"), false, true}},
    {SeamPropertyModel::Property::ErrorsAndReferenceCurves, {QStringLiteral("Errors And Reference Curves"), false, false}},
    {SeamPropertyModel::Property::Intervals, {QStringLiteral("Intervals"), false, false}},
    {SeamPropertyModel::Property::IntervalErrors, {QStringLiteral("Interval Errors"), false, false}},
    {SeamPropertyModel::Property::SeamRoi, {QStringLiteral("Seam Roi"), false, false}}
};

SeamPropertyModel::SeamPropertyModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

SeamPropertyModel::~SeamPropertyModel() = default;

QHash<int, QByteArray> SeamPropertyModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("parameter")},
        {Qt::UserRole, QByteArrayLiteral("checked")},
        {Qt::UserRole + 1, QByteArrayLiteral("property")},
        {Qt::UserRole + 2, QByteArrayLiteral("value")},
        {Qt::UserRole + 3, QByteArrayLiteral("adjustable")},
    };
}

QVariant SeamPropertyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= int(s_propertyKeys.size()))
    {
        return {};
    }

    const auto& property = s_propertyKeys.at(Property(index.row()));
    if (role == Qt::DisplayRole)
    {
        return std::get<QString>(property);
    }
    if (role == Qt::UserRole)
    {
        return std::get<1>(property);
    }
    if (role == Qt::UserRole + 1)
    {
        auto it = s_propertyKeys.begin();
        std::advance(it, index.row());
        return QVariant::fromValue((*it).first);
    }
    if (role == Qt::UserRole + 2)
    {
        return currentValue(Property(index.row()));
    }
    if (role == Qt::UserRole + 3)
    {
        return std::get<2>(property);
    }
    return {};
}

int SeamPropertyModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return s_propertyKeys.size();
}

bool SeamPropertyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::UserRole || index.row() >= int(s_propertyKeys.size()))
    {
        return false;
    }

    std::get<1>(s_propertyKeys.at(Property(index.row()))) = value.toBool();
    emit dataChanged(index, index, {Qt::UserRole});

    return true;
}

Qt::ItemFlags SeamPropertyModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

void SeamPropertyModel::setCurrentSeam(Seam* seam)
{
    if (m_currentSeam == seam)
    {
        return;
    }

    m_currentSeam = seam;
    disconnect(m_seamDestroyed);
    if (m_currentSeam)
    {
        m_seamDestroyed = connect(m_currentSeam, &Seam::destroyed, this, std::bind(&SeamPropertyModel::setCurrentSeam, this, nullptr));
    } else
    {
        m_seamDestroyed = QMetaObject::Connection{};
    }

    emit currentSeamChanged();
}

void SeamPropertyModel::setGraphModel(GraphModel *graphModel)
{
    if (m_graphModel == graphModel)
    {
        return;
    }
    disconnect(m_graphModelDestroyedConnection);
    m_graphModel = graphModel;
    if (m_graphModel)
    {
        m_graphModelDestroyedConnection = connect(m_graphModel, &QObject::destroyed, this, std::bind(&SeamPropertyModel::setGraphModel, this, nullptr));
    } else
    {
        m_graphModelDestroyedConnection = {};
    }
    emit graphModelChanged();
}

void SeamPropertyModel::setSubGraphModel(SubGraphModel* subGraphModel)
{
    if (m_subGraphModel == subGraphModel)
    {
        return;
    }
    disconnect(m_subGraphModelDestroyedConnection);
    m_subGraphModel = subGraphModel;
    if (m_subGraphModel)
    {
        m_subGraphModelDestroyedConnection = connect(m_subGraphModel, &QObject::destroyed, this, std::bind(&SeamPropertyModel::setSubGraphModel, this, nullptr));
    } else
    {
        m_subGraphModelDestroyedConnection = {};
    }
    emit subGraphModelChanged();
}

void SeamPropertyModel::setFilterAttributeModel(FilterAttributeModel* filterAttributeModel)
{
    if (m_filterAttributeModel == filterAttributeModel)
    {
        return;
    }
    disconnect(m_filterAttributeModelDestroyedConnection);
    disconnect(m_filterAttributeDataChangedConnection);
    disconnect(m_filterAttributeModelResetConnection);
    m_filterAttributeModel = filterAttributeModel;
    if (m_filterAttributeModel)
    {
        m_filterAttributeModelDestroyedConnection = connect(m_filterAttributeModel, &QObject::destroyed, this, std::bind(&SeamPropertyModel::setFilterAttributeModel, this, nullptr));

        m_filterAttributeDataChangedConnection = connect(m_filterAttributeModel, &FilterAttributeModel::dataChanged, this,
            [this] (const QModelIndex &topLeft, const QModelIndex &, const QVector<int> &roles)
            {
                if (roles.empty() || std::any_of(roles.begin(), roles.end(), [] (int a) { return a == Qt::UserRole + 7;}))
                {
                    const auto selected = topLeft.data(Qt::UserRole + 7).toBool();
                    if (auto group = topLeft.data(Qt::UserRole + 11).value<storage::AttributeGroup*>())
                    {
                        const auto& items = group->items();
                        for (auto groupItem : items)
                        {
                            if (selected)
                            {
                                m_selectedAttributes.emplace(groupItem->instanceId());
                            }
                            else
                            {
                                m_selectedAttributes.erase(groupItem->instanceId());
                            }
                        }
                    }
                }
            }
        );
        m_filterAttributeModelResetConnection = connect(m_filterAttributeModel, &FilterAttributeModel::modelReset, this,
            [this]
            {
                for (const auto &instanceId : m_selectedAttributes)
                {
                    if (auto index = m_filterAttributeModel->indexForAttributeInstance(instanceId); index.isValid())
                    {
                        m_filterAttributeModel->setData(index, true, Qt::UserRole + 7);
                    }
                }
            }
        );
    } else
    {
        m_filterAttributeModelDestroyedConnection = {};
        m_filterAttributeDataChangedConnection = {};
        m_filterAttributeModelResetConnection = {};
    }
    emit filterAttributeModelChanged();
}

fliplib::GraphContainer SeamPropertyModel::getGraph(Seam* seam) const
{
    return getGraphFromModel(seam, m_graphModel, m_subGraphModel);
}

bool SeamPropertyModel::copy(Seam* source, Seam* destination)
{
    if (!source || !destination)
    {
        return false;
    }

    auto changed = false;
    const auto copyMode = CopyMode::WithDifferentIds;

    for (auto &property : s_propertyKeys)
    {
        if (!std::get<1>(property.second))
        {
            continue;
        }
        switch (property.first)
        {
            case Property::Name:
            {
                destination->setName(source->name());
                changed = true;
                break;
            }
            case Property::Length:
            {
                destination->firstSeamInterval()->setLength(source->length());
                changed = true;
                break;
            }
            case Property::RateOfFeed:
            {
                destination->setVelocity(source->velocity());
                changed = true;
                break;
            }
            case Property::TriggerDistance:
            {
                destination->setTriggerDelta(source->triggerDelta());
                changed = true;
                break;
            }
            case Property::MovingDirection:
            {
                destination->setMovingDirection(source->movingDirection());
                changed = true;
                break;
            }
            case Property::ThicknessLeft:
            {
                destination->setThicknessLeft(source->thicknessLeft());
                changed = true;
                break;
            }
            case Property::ThicknessRight:
            {
                destination->setThicknessRight(source->thicknessRight());
                changed = true;
                break;
            }
            case Property::TargetDistance:
            {
                destination->setTargetDifference(source->targetDifference());
                changed = true;
                break;
            }
            case Property::PositionInAssemblyImage:
            {
                destination->setPositionInAssemblyImage(source->positionInAssemblyImage());
                changed = true;
                break;
            }
            case Property::HardwareParameters:
            {
                if (source->hardwareParameters())
                {
                    destination->setHardwareParameters(source->hardwareParameters()->duplicate(copyMode, destination));
                    destination->setLaserControlPreset(source->laserControlPreset());
                    changed = true;
                }
                break;
            }
            case Property::Detection:
            {
                destination->setGraphReference(source->graphReference());
                changed = true;
                break;
            }
            case Property::FilterParameters:
            {
                const auto source_product = source->product();
                auto paramSet = source_product->filterParameterSet(source->graphParamSet());
                if (!paramSet)
                {
                    break;
                }

                const auto destination_product = destination->product();

                if (m_copyAllFilterParameters)
                {

                    auto duplicated = paramSet->duplicate(copyMode, destination_product);
                    destination_product->addFilterParameterSet(duplicated);
                    destination->setGraphParamSet(duplicated->uuid());
                } else
                {
                    if (!m_filterAttributeModel)
                    {
                        break;
                    }

                    auto destinationParameterSet = destination_product->filterParameterSet(destination->graphParamSet());

                    const auto& parameters = paramSet->parameters();
                    const auto& destinationParameters = destinationParameterSet->parameters();

                    for (const auto& instanceAttributeId : m_selectedAttributes)
                    {
                        auto source_it = std::find_if(parameters.begin(), parameters.end(), [instanceAttributeId] (auto param) { return param->uuid() == instanceAttributeId; });

                        if (source_it == parameters.end())
                        {
                            continue;
                        }

                        auto destination_it = std::find_if(destinationParameters.begin(), destinationParameters.end(), [instanceAttributeId] (auto param) { return param->uuid() == instanceAttributeId; });

                        if (destination_it == destinationParameters.end())
                        {
                            // not in destination filter parameters, check and add if in destination graph
                            addParameterIfInGraph(destination, instanceAttributeId, (*source_it)->value());
                            continue;
                        }

                        (*destination_it)->setValue((*source_it)->value());
                    }
                }
                changed = true;
                break;
            }
            case Property::ErrorsAndReferenceCurves:
            {
                destination->copyErrorsAndReferenceCurves(copyMode, source);
                changed = true;
                break;
            }
            case Property::Intervals:
            {
                destination->duplicateSeamIntervals(copyMode, source);
                changed = true;
                break;
            }
            case Property::IntervalErrors:
            {
                destination->duplicateIntervalErrors(copyMode, source);
                changed = true;
                break;
            }
            case Property::SeamRoi:
            {
                destination->setRoi(source->roi());
                changed = true;
                break;
            }
        }
    }
    return changed;
}

void SeamPropertyModel::addParameterIfInGraph(Seam* destination, const QUuid& instanceAttributeId, const QVariant& value)
{
    if (!destination || !m_filterAttributeModel || !m_filterAttributeModel->attributeModel())
    {
        return;
    }

    auto product = destination->product();
    if (!product)
    {
        return;
    }

    auto parameterSet = product->filterParameterSet(destination->graphParamSet());
    if (!parameterSet)
    {
        return;
    }

    const auto& graph = getGraph(destination);

    for (const auto& instanceFilter : graph.instanceFilters)
    {
        for (const auto& attribute : instanceFilter.attributes)
        {
            if (storage::compatibility::toQt(attribute.instanceAttributeId) == instanceAttributeId)
            {
                if (auto attr = m_filterAttributeModel->attributeModel()->findAttribute(storage::compatibility::toQt(attribute.attributeId)))
                {
                    auto parameter = parameterSet->createParameter(storage::compatibility::toQt(attribute.instanceAttributeId), attr, storage::compatibility::toQt(instanceFilter.id), attr->convert(attribute.value));
                    parameter->setValue(value);
                }
                return;
            }
        }
    }
}

QString SeamPropertyModel::currentValue(Property property) const
{
    if (!m_currentSeam)
    {
        return QStringLiteral("");
    }
    switch (property)
    {
        case Property::Name:
            return m_currentSeam->name();
        case Property::Length:
            return QString::number(double(m_currentSeam->length()) / 1000, 'f', 3);
        case Property::RateOfFeed:
            return QString::number(double(m_currentSeam->velocity()) / 1000, 'f', 3);
        case Property::TriggerDistance:
            return QString::number(double(m_currentSeam->triggerDelta()) / 1000, 'f', 3);
        case Property::MovingDirection:
            return m_currentSeam->movingDirection() == Seam::MovingDirection::FromLower ? QStringLiteral("From Lower") : QStringLiteral("From Upper");
        case Property::ThicknessLeft:
            return QString::number(double(m_currentSeam->thicknessLeft()) / 1000, 'f', 3);
        case Property::ThicknessRight:
            return QString::number(double(m_currentSeam->thicknessRight()) / 1000, 'f', 3);
        case Property::TargetDistance:
            return QString::number(double(m_currentSeam->targetDifference()) / 1000, 'f', 3);
        case Property::PositionInAssemblyImage:
            return QStringLiteral("X: %1, Y: %2").arg(m_currentSeam->positionInAssemblyImage().x()).arg(m_currentSeam->positionInAssemblyImage().y());
        case Property::SeamRoi:
            return QStringLiteral("X: %1, Y: %2, Width: %3, Height: %4").arg(m_currentSeam->roi().x()).arg(m_currentSeam->roi().y()).arg(m_currentSeam->roi().width()).arg(m_currentSeam->roi().height());
        case Property::Detection:
        {
            if (!m_currentSeam->usesSubGraph() && m_graphModel)
            {
                return m_graphModel->name(m_currentSeam->graph());
            }
        }
        case Property::FilterParameters:
        case Property::ErrorsAndReferenceCurves:
        case Property::Intervals:
        case Property::IntervalErrors:
        case Property::HardwareParameters:
        default:
            return QStringLiteral("");
    }
}

void SeamPropertyModel::setCopyAllFilterParameters(bool copyAll)
{
    if (m_copyAllFilterParameters == copyAll)
    {
        return;
    }
    m_copyAllFilterParameters = copyAll;
    emit copyAllFilterParametersChanged();
}

}
}
