#include "hardwareParameterOverriddenModel.h"
#include "abstractHardwareParameterModel.h"

#include "parameter.h"
#include "parameterSet.h"
#include "abstractMeasureTask.h"
#include "product.h"
#include "seamSeries.h"
#include "seam.h"

namespace precitec
{
namespace gui
{

HardwareParameterOverriddenModel::HardwareParameterOverriddenModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &HardwareParameterOverriddenModel::hardwareParameterModelChanged, this, &HardwareParameterOverriddenModel::init);
    connect(this, &HardwareParameterOverriddenModel::hardwareParameterIndexChanged, this, &HardwareParameterOverriddenModel::init);
}

HardwareParameterOverriddenModel::~HardwareParameterOverriddenModel() = default;

QHash<int, QByteArray> HardwareParameterOverriddenModel::roleNames() const
{
    return {
        {Qt::UserRole, QByteArrayLiteral("parameter")},
        {Qt::UserRole + 1, QByteArrayLiteral("measureTask")},
    };
}

int HardwareParameterOverriddenModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_data.size();
}

QVariant HardwareParameterOverriddenModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    const auto &element = m_data.at(index.row());
    switch (role)
    {
    case Qt::UserRole:
        return QVariant::fromValue(element.parameter);
    case Qt::UserRole + 1:
        return QVariant::fromValue(element.measureTask);
    }
    return {};
}

void HardwareParameterOverriddenModel::setHardwareParameterModel(AbstractHardwareParameterModel *model)
{
    if (m_hardwareParameterModel == model)
    {
        return;
    }
    m_hardwareParameterModel = model;
    disconnect(m_hardwareParameterModelDestroyedConnection);
    disconnect(m_hardwareParameterModelDataChanged);
    if (m_hardwareParameterModel)
    {
        m_hardwareParameterModelDestroyedConnection = connect(m_hardwareParameterModel, &AbstractHardwareParameterModel::destroyed, this, std::bind(&HardwareParameterOverriddenModel::setHardwareParameterModel, this, nullptr));
        m_hardwareParameterModelDataChanged = connect(m_hardwareParameterModel, &AbstractHardwareParameterModel::dataChanged, this,
            [this] (const QModelIndex &index)
            {
                if (index != m_parameterModelIndex)
                {
                    return;
                }
                init();
            }
        );
    }
    emit hardwareParameterModelChanged();
}

void HardwareParameterOverriddenModel::setHardwareParameterIndex(const QModelIndex &index)
{
    if (m_parameterModelIndex == index)
    {
        return;
    }
    m_parameterModelIndex = index;
    emit hardwareParameterIndexChanged();
}

void HardwareParameterOverriddenModel::init()
{
    beginResetModel();
    m_data.clear();
    if (m_hardwareParameterModel && m_parameterModelIndex.isValid() && m_parameterModelIndex.model() == m_hardwareParameterModel)
    {
        auto *parameter = m_parameterModelIndex.data(Qt::UserRole + 3).value<storage::Parameter*>();
        auto *parameterSetParent = m_hardwareParameterModel->getParameterSet()->parent();

        if (auto *product = qobject_cast<storage::Product*>(parameterSetParent))
        {
            findForProduct(product, parameter);
        }
        if (auto *seamSeries = qobject_cast<storage::SeamSeries*>(parameterSetParent))
        {
            findForSeamSeries(seamSeries, parameter);
        }
    }

    endResetModel();
}

void HardwareParameterOverriddenModel::findForProduct(storage::Product *product, storage::Parameter *parameter)
{
    if (!parameter)
    {
        return;
    }
    for (auto *seamSeries : product->seamSeries())
    {
        if (auto *ps = seamSeries->hardwareParameters())
        {
            if (auto *p = ps->findByNameAndTypeId(parameter->name(), parameter->typeId()))
            {
                m_data.emplace_back(p, seamSeries);
            }
        }
        findForSeamSeries(seamSeries, parameter);
    }
}

void HardwareParameterOverriddenModel::findForSeamSeries(storage::SeamSeries *seamSeries, storage::Parameter *parameter)
{
    if (!parameter)
    {
        return;
    }
    for (auto *seam : seamSeries->seams())
    {
        if (auto *ps = seam->hardwareParameters())
        {
            if (auto *p = ps->findByNameAndTypeId(parameter->name(), parameter->typeId()))
            {
                m_data.emplace_back(p, seam);
            }
        }
    }
}

}
}
