#include "hardwareParameterOverridesModel.h"
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

HardwareParameterOverridesModel::HardwareParameterOverridesModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &HardwareParameterOverridesModel::hardwareParameterModelChanged, this, &HardwareParameterOverridesModel::init);
    connect(this, &HardwareParameterOverridesModel::hardwareParameterIndexChanged, this, &HardwareParameterOverridesModel::init);
}

HardwareParameterOverridesModel::~HardwareParameterOverridesModel() = default;

QHash<int, QByteArray> HardwareParameterOverridesModel::roleNames() const
{
    return {
        {Qt::UserRole, QByteArrayLiteral("parameter")},
        {Qt::UserRole + 1, QByteArrayLiteral("seamSeries")},
        {Qt::UserRole + 2, QByteArrayLiteral("product")}
    };
}

int HardwareParameterOverridesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_data.size();
}

QVariant HardwareParameterOverridesModel::data(const QModelIndex &index, int role) const
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
        return QVariant::fromValue(element.seamSeries);
    case Qt::UserRole + 2:
        return QVariant::fromValue(element.product);
    }
    return {};
}

void HardwareParameterOverridesModel::setHardwareParameterModel(AbstractHardwareParameterModel *model)
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
        m_hardwareParameterModelDestroyedConnection = connect(m_hardwareParameterModel, &AbstractHardwareParameterModel::destroyed, this, std::bind(&HardwareParameterOverridesModel::setHardwareParameterModel, this, nullptr));
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

void HardwareParameterOverridesModel::setHardwareParameterIndex(const QModelIndex &index)
{
    if (m_parameterModelIndex == index)
    {
        return;
    }
    m_parameterModelIndex = index;
    emit hardwareParameterIndexChanged();
}

void HardwareParameterOverridesModel::init()
{
    beginResetModel();
    m_data.clear();
    if (m_hardwareParameterModel && m_parameterModelIndex.isValid() && m_parameterModelIndex.model() == m_hardwareParameterModel)
    {
        auto *parameter = m_parameterModelIndex.data(Qt::UserRole + 3).value<storage::Parameter*>();
        auto *parameterSetParent = m_hardwareParameterModel->getParameterSet()->parent();

        if (auto *seam = qobject_cast<storage::Seam*>(parameterSetParent))
        {
            findForSeam(seam, parameter);
        }
        if (auto *seamSeries = qobject_cast<storage::SeamSeries*>(parameterSetParent))
        {
            findForSeamSeries(seamSeries, parameter);
        }
    }

    endResetModel();
}

void HardwareParameterOverridesModel::findForSeam(storage::Seam *seam, storage::Parameter *parameter)
{
    if (!parameter)
    {
        return;
    }
    auto *seamSeries = seam->seamSeries();
    if (auto *ps = seamSeries->hardwareParameters())
    {
        if (auto *p = ps->findByNameAndTypeId(parameter->name(), parameter->typeId()))
        {
            m_data.emplace_back(p, seamSeries);
        }
    }
    findForSeamSeries(seamSeries, parameter);
}

void HardwareParameterOverridesModel::findForSeamSeries(storage::SeamSeries *seamSeries, storage::Parameter *parameter)
{
    if (!parameter)
    {
        return;
    }
    auto *product = seamSeries->product();
    if (auto *ps = product->hardwareParameters())
    {
        if (auto *p = ps->findByNameAndTypeId(parameter->name(), parameter->typeId()))
        {
            m_data.emplace_back(p, product);
        }
    }
}

}
}
