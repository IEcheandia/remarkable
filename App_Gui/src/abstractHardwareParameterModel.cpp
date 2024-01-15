#include "abstractHardwareParameterModel.h"
#include "attribute.h"
#include "attributeModel.h"
#include "seam.h"
#include "seamSeries.h"
#include "product.h"
#include "parameter.h"
#include "parameterSet.h"

using precitec::storage::AttributeModel;
using precitec::storage::Attribute;
using precitec::storage::ParameterSet;
using precitec::storage::Parameter;
using precitec::storage::Seam;

namespace precitec
{
namespace gui
{

AbstractHardwareParameterModel::AbstractHardwareParameterModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &AbstractHardwareParameterModel::attributeModelChanged, this,
        [this]
        {
            dataChanged(index(0, 0), index(rowCount() - 1, 0), {Qt::UserRole + 2});
        }
    );
}

AbstractHardwareParameterModel::~AbstractHardwareParameterModel() = default;

QVariant AbstractHardwareParameterModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    const auto key = HardwareParameters::Key(index.row());
    const auto& properties = HardwareParameters::instance()->properties(key);
    auto parameter = findParameter(key);

    switch (role)
    {
    case Qt::DisplayRole:
        return tr(properties.name.c_str());
    case Qt::UserRole:
        return QVariant::fromValue(key);
    case Qt::UserRole + 1:
        return parameter != nullptr;
    case Qt::UserRole + 2:
        return QVariant::fromValue(findAttribute(key));
    case Qt::UserRole + 3:
        return QVariant::fromValue(parameter);
    case Qt::UserRole + 4:
        return properties.conversion == HardwareParameters::UnitConversion::MilliFromMicro;
    }

    return {};
}

int AbstractHardwareParameterModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return HardwareParameters::instance()->keyCount();
}

QHash<int, QByteArray> AbstractHardwareParameterModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {Qt::UserRole, QByteArrayLiteral("key")},
        {Qt::UserRole + 1, QByteArrayLiteral("enabled")},
        {Qt::UserRole + 2, QByteArrayLiteral("attribute")},
        {Qt::UserRole + 3, QByteArrayLiteral("parameter")},
        {Qt::UserRole + 4, QByteArrayLiteral("milliFromMicro")}
    };
}

Parameter *AbstractHardwareParameterModel::findParameter(HardwareParameters::Key key) const
{
    if (!isValid())
    {
        return nullptr;
    }
    auto ps = getParameterSetDirect();
    return findParameter(ps, key);
}

Parameter *AbstractHardwareParameterModel::findParameter(ParameterSet *parameterSet, HardwareParameters::Key key) const
{
    if (!parameterSet)
    {
        return nullptr;
    }
    auto attribute = findAttribute(key);
    if (!attribute)
    {
        return nullptr;
    }
    const auto &parameters = parameterSet->parameters();

    auto it = std::find_if(parameters.begin(), parameters.end(), [attribute] (auto param) { return param->name() == attribute->name(); });
    if (it == parameters.end())
    {
        return nullptr;
    }
    return *it;
}

void AbstractHardwareParameterModel::setEnable(HardwareParameters::Key key, bool set)
{
    if (!isValid())
    {
        return;
    }
    auto ps = getParameterSet();
    std::vector<HardwareParameters::Key> changedKeys = {key};
    if (set)
    {
        if (auto attribute = findAttribute(key))
        {
            ps->createParameter(QUuid::createUuid(), attribute, QUuid{});
        }

        const auto& mutuallyExclusiveKeys = HardwareParameters::instance()->mutuallyExclusiveKeys();
        for (const auto& exclusiveKeys : mutuallyExclusiveKeys)
        {
            if (std::any_of(exclusiveKeys.begin(), exclusiveKeys.end(), [key] (auto exclusiveKey) { return key == exclusiveKey; }))
            {
                for (auto exclusiveKey : exclusiveKeys)
                {
                    if (exclusiveKey == key)
                    {
                        continue;
                    }
                    if (auto parameter = findParameter(exclusiveKey))
                    {
                        ps->removeParameter(parameter);
                        changedKeys.push_back(exclusiveKey);
                    }
                }
            }
        }
    } else
    {
        if (auto parameter = findParameter(key))
        {
            ps->removeParameter(parameter);
        }
    }
    if (HardwareParameters::instance()->isLedKey(key))
    {
        updateLedSendData();
    }
    if (HardwareParameters::instance()->isScanTracker2DKey(key))
    {
        updateGenerateScanTracker2DFigure();
    }
    for (auto changedKey : changedKeys)
    {
        const auto i = index(int(changedKey));
        emit dataChanged(i, i, {});
    }
    emit markAsChanged();
    emit parameterChanged();
}

void AbstractHardwareParameterModel::updateHardwareParameter(HardwareParameters::Key key, const QVariant &value)
{
    if (auto parameter = findParameter(key))
    {
        parameter->setValue(value);
        if (HardwareParameters::instance()->isLedKey(key))
        {
            updateLedSendData();
        }
        if (HardwareParameters::instance()->isScanTracker2DKey(key))
        {
            updateGenerateScanTracker2DFigure();
        }
        emit markAsChanged();
        emit parameterChanged();
    }
}

void AbstractHardwareParameterModel::setAttributeModel(AttributeModel *model)
{
    if (m_attributeModel == model)
    {
        return;
    }
    m_attributeModel = model;
    disconnect(m_attributeModelReset);
    disconnect(m_attributeModelDestroyed);
    if (m_attributeModel)
    {
        m_attributeModelReset = connect(m_attributeModel, &AttributeModel::modelReset, this,
            [this]
            {
                dataChanged(index(0, 0), index(rowCount() - 1, 0), {Qt::UserRole + 2});
            }
        );
        m_attributeModelDestroyed = connect(m_attributeModel, &AttributeModel::destroyed, this, std::bind(&AbstractHardwareParameterModel::setAttributeModel, this, nullptr));
    } else
    {
        m_attributeModelReset = QMetaObject::Connection{};
        m_attributeModelDestroyed = QMetaObject::Connection{};
    }
    emit attributeModelChanged();
}

Attribute *AbstractHardwareParameterModel::findAttribute(HardwareParameters::Key key) const
{
    if (!m_attributeModel)
    {
        return nullptr;
    }
    return m_attributeModel->findAttribute(HardwareParameters::instance()->properties(key).uuid);
}

void AbstractHardwareParameterModel::updateSendParameter(ParameterSet* parameterSet, const QLatin1String& keyName, const QUuid& attributeUuid, std::function<bool(HardwareParameters*, const QUuid&)> isKey)
{
    if (!m_attributeModel || !parameterSet)
    {
        return;
    }

    const auto &parameters = parameterSet->parameters();

    const auto sendKeyIt = std::find_if(parameters.begin(), parameters.end(), [keyName] (const auto *parameter) { return parameter->name() == keyName; });

    const bool hasSendKey = std::any_of(parameters.begin(), parameters.end(),
        [this, isKey] (const auto *parameter)
        {
            auto attribute = m_attributeModel->findAttributeByName(parameter->name());
            if (!attribute)
            {
                return false;
            }
            return isKey(HardwareParameters::instance(), attribute->uuid());
        }
    );
    if (hasSendKey && (sendKeyIt == parameters.end()))
    {
        // add keyvalue
        if (auto attribute = m_attributeModel->findAttribute(attributeUuid))
        {
            auto parameter = parameterSet->createParameter(QUuid::createUuid(), attribute, QUuid{});
            parameter->setValue(true);
        }
    }
    else if (!hasSendKey && (sendKeyIt != parameters.end()))
    {
        // remove parameter
        parameterSet->removeParameter(*sendKeyIt);
    }
}

void AbstractHardwareParameterModel::updateLedSendData(ParameterSet* parameterSet)
{
    updateSendParameter(parameterSet, QLatin1String("LEDSendData"), {QByteArrayLiteral("F6D4C84B-9F71-4054-8245-7DB93DBBF81E")}, qOverload<const QUuid&>(&HardwareParameters::isLedKey));
}

void AbstractHardwareParameterModel::updateLedSendData()
{
    updateLedSendData(getParameterSet());
}

void AbstractHardwareParameterModel::updateGenerateScanTracker2DFigure(precitec::storage::ParameterSet* parameterSet)
{
    updateSendParameter(parameterSet, QLatin1String("Generate_ScanTracker2D_Figure"), {QByteArrayLiteral("e8185a71-0e95-4e87-a730-ac91bf256222")},  qOverload<const QUuid&>(&HardwareParameters::isScanTracker2DKey));
}

void AbstractHardwareParameterModel::updateGenerateScanTracker2DFigure()
{
    updateGenerateScanTracker2DFigure(getParameterSet());
}

QString AbstractHardwareParameterModel::nameForAttribute(Attribute *attribute)
{
    const auto key = HardwareParameters::instance()->key(attribute->uuid());
    if (key != HardwareParameters::Key::InvalidKey)
    {
        return QObject::tr(HardwareParameters::instance()->keys().at(key).name.c_str());
    }
    return attribute->name();
}

QModelIndex AbstractHardwareParameterModel::indexForKey(precitec::gui::HardwareParameters::Key key) const
{
    return index(int(key), 0);
}

}
}
