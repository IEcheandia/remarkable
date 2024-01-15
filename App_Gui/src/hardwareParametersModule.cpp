#include "hardwareParametersModule.h"
#include "attributeModel.h"
#include "attribute.h"
#include "parameterSet.h"
#include "parameter.h"

using precitec::storage::AttributeModel;
using precitec::storage::Attribute;
using precitec::storage::ParameterSet;
using precitec::storage::Parameter;

namespace precitec
{
namespace gui
{

HardwareParametersModule::HardwareParametersModule(QObject* parent)
    : QObject(parent)
{
}

HardwareParametersModule::~HardwareParametersModule() = default;

void HardwareParametersModule::setAttributeModel(AttributeModel* model)
{
    if (m_attributeModel == model)
    {
        return;
    }

    disconnect(m_attributeModelDestroyed);

    m_attributeModel = model;

    if (m_attributeModel)
    {
        m_attributeModelDestroyed = connect(m_attributeModel, &AttributeModel::destroyed, this, std::bind(&HardwareParametersModule::setAttributeModel, this, nullptr));
    } else
    {
        m_attributeModelDestroyed = {};
    }

    emit attributeModelChanged();
}

Attribute* HardwareParametersModule::findAttribute(const QUuid& id) const
{
    if (!m_attributeModel)
    {
        return nullptr;
    }
    return m_attributeModel->findAttribute(id);
}

Parameter* HardwareParametersModule::findParameter(ParameterSet* parameterSet, const QUuid& id) const
{
    if (!parameterSet)
    {
        return nullptr;
    }

    auto attribute = findAttribute(id);
    if (!attribute)
    {
        return nullptr;
    }

    const auto& parameters = parameterSet->parameters();

    auto it = std::find_if(parameters.begin(), parameters.end(), [attribute] (auto param) { return param->name() == attribute->name(); });
    if (it == parameters.end())
    {
        return nullptr;
    }

    return *it;
}

void HardwareParametersModule::updateParameter(ParameterSet* parameterSet, const QUuid& id, const QVariant& value)
{
    if (!parameterSet)
    {
        return;
    }
    if (auto parameter = findParameter(parameterSet, id))
    {
        parameter->setValue(value);
    } else
    {
        if (auto attribute = findAttribute(id))
        {
            auto parameter = parameterSet->createParameter(QUuid::createUuid(), attribute, QUuid{});
            parameter->setValue(value);
        }
    }
}

void HardwareParametersModule::removeParameter(precitec::storage::ParameterSet* parameterSet, const QUuid& id)
{
    if (!parameterSet)
    {
        return;
    }
    if (auto parameter = findParameter(parameterSet, id))
    {
        parameterSet->removeParameter(parameter);
    }
}

}
}

