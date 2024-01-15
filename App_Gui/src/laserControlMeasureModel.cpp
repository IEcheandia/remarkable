#include "laserControlMeasureModel.h"
#include "laserControlPreset.h"
#include "attribute.h"
#include "attributeModel.h"
#include "parameter.h"
#include "parameterSet.h"
#include "abstractMeasureTask.h"

using precitec::storage::LaserControlPreset;
using precitec::storage::Attribute;
using precitec::storage::AttributeModel;
using precitec::storage::Parameter;
using precitec::storage::ParameterSet;
using precitec::storage::AbstractMeasureTask;

namespace precitec
{
namespace gui
{

LaserControlMeasureModel::LaserControlMeasureModel(QObject *parent)
    : AbstractLaserControlTaskModel(parent)
{
    connect(this, &LaserControlMeasureModel::measureTaskChanged, this, &LaserControlMeasureModel::init);
}

LaserControlMeasureModel::~LaserControlMeasureModel() = default;

void LaserControlMeasureModel::setMeasureTask(AbstractMeasureTask *measureTask)
{
    if (m_measureTask == measureTask)
    {
        return;
    }
    m_measureTask = measureTask;
    disconnect(m_measureTaskDestroyed);
    if (m_measureTask)
    {
        m_measureTaskDestroyed = connect(m_measureTask, &AbstractMeasureTask::destroyed, this, std::bind(&LaserControlMeasureModel::setMeasureTask, this, nullptr));
    } else
    {
        m_measureTaskDestroyed = QMetaObject::Connection{};
    }
    emit measureTaskChanged();
}

ParameterSet *LaserControlMeasureModel::getParameterSet() const
{
    if (!m_measureTask->hardwareParameters())
    {
        m_measureTask->createHardwareParameters();
    }
    return m_measureTask->hardwareParameters();
}

void LaserControlMeasureModel::updateHardwareParameters()
{
    if (!m_measureTask)
    {
        return;
    }

    auto ps = getParameterSet();

    auto changed = false;

    if (presetEnabled())
    {
        if(currentPreset() >= 0 && currentPreset() < rowCount())
        {
            const auto& row = index(currentPreset(), 0);

            const auto& presetId = row.data(Qt::UserRole + 1).toUuid();

            if (m_measureTask->laserControlPreset() != presetId)
            {
                m_measureTask->setLaserControlPreset(presetId);

                changed = true;
            }

            changed |= updateParameterSet(ps, row);
        }
    } else
    {
        changed = clearPresetValues(ps);
    }

    auto delay_parameter = findParameter(ps, LaserControlMeasureModel::Key::LC_Parameter_No2);
    const auto delay_attribute = findAttribute(LaserControlMeasureModel::Key::LC_Parameter_No2);

    if (delayEnabled() && delay_attribute)
    {
        if (!delay_parameter)
        {
            delay_parameter = ps->createParameter(QUuid::createUuid(), delay_attribute, QUuid{});

            changed = true;
        }

        if (delay_parameter->value().toInt() != delay())
        {
            delay_parameter->setValue(delay());

            changed = true;
        }
    } else
    {
        if (delay_parameter)
        {
            setDelayDirect(0);
            ps->removeParameter(delay_parameter);

            changed = true;
        }
    }

    auto sd_parameter = findParameter(ps, LaserControlMeasureModel::Key::LC_Send_Data);
    const auto sd_attribute = findAttribute(LaserControlMeasureModel::Key::LC_Send_Data);

    if ((presetEnabled() || delayEnabled()) && sd_attribute)
    {
        if (!sd_parameter)
        {
            sd_parameter = ps->createParameter(QUuid::createUuid(), sd_attribute, QUuid{});

            changed = true;
        }

        if (!sd_parameter->value().toBool())
        {
            sd_parameter->setValue(true);

            changed = true;
        }
    } else
    {
        if (sd_parameter)
        {
            ps->removeParameter(sd_parameter);

            changed = true;
        }
    }

    if (changed)
    {
        emit markAsChanged();
    }
}

void LaserControlMeasureModel::init()
{
    if (!m_measureTask)
    {
        return;
    }
    if (!m_measureTask->hardwareParameters())
    {
        if (presetEnabled())
        {
            setPresetEnabledDirect(false);
        }

        if (delayEnabled())
        {
            setDelayEnabledDirect(false);
        }

        return;
    }

    AbstractLaserControlModel::setCurrentPreset(-1);

    removeCurrentPreset();

    auto ps = getParameterSet();

    if (hasPresetParameters(ps))
    {
        setPresetEnabledDirect(true);

        for (auto i = 0; i < rowCount(); i++)
        {
            if (compareParameterSet(ps, index(i, 0)) && m_measureTask->laserControlPreset() == index(i, 0).data(Qt::UserRole + 1))
            {
                AbstractLaserControlModel::setCurrentPreset(i);
                break;
            }
        }

        if (currentPreset() == -1)
        {
            createCurrentPreset();
        }
    } else
    {
        setPresetEnabledDirect(false);
    }

    if (auto delayParameter = findParameter(ps, LaserControlMeasureModel::Key::LC_Parameter_No2))
    {
        setDelayEnabledDirect(true);
        setDelayDirect(delayParameter->value().toInt());
    }
}

}
}
