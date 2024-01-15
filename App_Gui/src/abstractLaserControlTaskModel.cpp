#include "abstractLaserControlTaskModel.h"
#include "laserControlPreset.h"
#include "attribute.h"
#include "attributeModel.h"
#include "parameter.h"
#include "parameterSet.h"

using precitec::storage::LaserControlPreset;
using precitec::storage::Attribute;
using precitec::storage::AttributeModel;
using precitec::storage::Parameter;
using precitec::storage::ParameterSet;

namespace precitec
{
namespace gui
{

std::map<AbstractLaserControlTaskModel::Key, std::tuple<std::string, QUuid> > lc_keys {
    {AbstractLaserControlTaskModel::Key::LC_Parameter_No2, {QT_TRANSLATE_NOOP("", "Laser Control Delay"), {QByteArrayLiteral("8AD9BE12-4026-45BF-B106-5CC487EB8B1D")}}},
    {AbstractLaserControlTaskModel::Key::LC_Send_Data, {QT_TRANSLATE_NOOP("", "Laser Control Send Data"), {QByteArrayLiteral("268968BE-2EA0-45C5-A1E4-2E3B31F47EF2")}}}
};

AbstractLaserControlTaskModel::AbstractLaserControlTaskModel(QObject *parent)
    : AbstractLaserControlModel(parent)
{
    connect(this, &AbstractLaserControlTaskModel::loadingFinished, this, &AbstractLaserControlTaskModel::init);
    connect(this, &AbstractLaserControlTaskModel::attributeModelChanged, this, &AbstractLaserControlTaskModel::init);
    connect(this, &AbstractLaserControlTaskModel::channel2EnabledChanged, this, &AbstractLaserControlTaskModel::updateHardwareParameters);
}

AbstractLaserControlTaskModel::~AbstractLaserControlTaskModel() = default;

void AbstractLaserControlTaskModel::setAttributeModel(AttributeModel *model)
{
    if (attributeModel() == model)
    {
        return;
    }

    disconnect(m_attributeModelReset);

    AbstractLaserControlModel::setAttributeModel(model);

    if (attributeModel())
    {
        m_attributeModelReset = connect(attributeModel(), &AttributeModel::modelReset, this, &AbstractLaserControlTaskModel::init);
    } else
    {
        m_attributeModelReset = QMetaObject::Connection{};
    }
}

void AbstractLaserControlTaskModel::setCurrentPreset(const int index)
{
    AbstractLaserControlModel::setCurrentPreset(index);

    updateHardwareParameters();
}

Attribute* AbstractLaserControlTaskModel::findAttribute(Key key) const
{
    if (!attributeModel())
    {
        return nullptr;
    }
    return attributeModel()->findAttribute(std::get<1>(lc_keys.at(key)));
}

Parameter* AbstractLaserControlTaskModel::findParameter(ParameterSet* ps, Key key) const
{
    if (!ps)
    {
        return nullptr;
    }

    auto attribute = findAttribute(key);

    if (!attribute)
    {
        return nullptr;
    }
    const auto &parameters = ps->parameters();

    auto it = std::find_if(parameters.begin(), parameters.end(), [attribute] (auto param) { return param->name() == attribute->name(); });
    if (it == parameters.end())
    {
        return nullptr;
    }
    return *it;
}

void AbstractLaserControlTaskModel::createCurrentPreset()
{
    const auto current = new LaserControlPreset{this};
    current->setName(QStringLiteral("Unsaved Configuration"));

    updatePresetValues(getParameterSet(), current);
    insertPresetInstance(0, current);
    AbstractLaserControlModel::setCurrentPreset(0);
}

void AbstractLaserControlTaskModel::removeCurrentPreset()
{
    if (rowCount() != 0 && index(0, 0).data().toString() == QStringLiteral("Unsaved Configuration"))
    {
        beginRemoveRows(QModelIndex(), 0, 0);
        removePresetInstance(0);
        endRemoveRows();
    }
}

void AbstractLaserControlTaskModel::load()
{
    AbstractLaserControlModel::load();

    emit loadingFinished();
}

void AbstractLaserControlTaskModel::setPresetEnabled(bool enabled)
{
    if (m_presetEnabled == enabled)
    {
        return;
    }
    m_presetEnabled = enabled;
    emit presetEnabledChanged();

    updateHardwareParameters();
}

void AbstractLaserControlTaskModel::setDelayEnabled(bool enabled)
{
    if (m_delayEnabled == enabled)
    {
        return;
    }
    m_delayEnabled = enabled;
    emit delayEnabledChanged();

    updateHardwareParameters();
}

void AbstractLaserControlTaskModel::setDelay(int delay)
{
    if (m_delay == delay)
    {
        return;
    }
    m_delay = delay;
    emit delayChanged();

    updateHardwareParameters();
}

void AbstractLaserControlTaskModel::setPresetEnabledDirect(bool enabled)
{
    m_presetEnabled = enabled;
    emit presetEnabledChanged();
}

void AbstractLaserControlTaskModel::setDelayEnabledDirect(bool enabled)
{
    m_delayEnabled = enabled;
    emit delayEnabledChanged();
}

void AbstractLaserControlTaskModel::setDelayDirect(int delay)
{
    m_delay = delay;
    emit delayChanged();
}

}
}

