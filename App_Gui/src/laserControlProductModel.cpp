#include "laserControlProductModel.h"
#include "laserControlPreset.h"
#include "attribute.h"
#include "attributeModel.h"
#include "parameter.h"
#include "parameterSet.h"
#include "product.h"
#include <QDebug>

using precitec::storage::LaserControlPreset;
using precitec::storage::Attribute;
using precitec::storage::AttributeModel;
using precitec::storage::Parameter;
using precitec::storage::ParameterSet;
using precitec::storage::Product;

namespace precitec
{
namespace gui
{

LaserControlProductModel::LaserControlProductModel(QObject *parent)
    : AbstractLaserControlTaskModel(parent)
{
    connect(this, &LaserControlProductModel::productChanged, this, &LaserControlProductModel::init);
}

LaserControlProductModel::~LaserControlProductModel() = default;

void LaserControlProductModel::setProduct(Product* product)
{
    if (m_product == product)
    {
        return;
    }
    m_product = product;
    disconnect(m_productDestroyed);
    if (m_product)
    {
        m_productDestroyed = connect(m_product, &Product::destroyed, this, std::bind(&LaserControlProductModel::setProduct, this, nullptr));
    } else
    {
        m_productDestroyed = QMetaObject::Connection{};
    }
    emit productChanged();
}

ParameterSet *LaserControlProductModel::getParameterSet() const
{
    if (!m_product->hardwareParameters())
    {
        m_product->createHardwareParameters();
    }
    return m_product->hardwareParameters();
}

void LaserControlProductModel::updateHardwareParameters()
{
    if (!m_product)
    {
        return;
    }

    auto ps = getParameterSet();

    auto changed = false;

    if (presetEnabled())
    {
        if (currentPreset() >= 0 && currentPreset() < rowCount())
        {
            const auto& row = index(currentPreset(), 0);

            const auto& presetId = row.data(Qt::UserRole + 1).toUuid();

            if (m_product->laserControlPreset() != presetId)
            {
                m_product->setLaserControlPreset(presetId);

                changed = true;
            }

            changed |= updateParameterSet(ps, row);
        }
    } else
    {
        changed = clearPresetValues(ps);
    }

    auto delay_parameter = findParameter(ps, LaserControlProductModel::Key::LC_Parameter_No2);
    const auto delay_attribute = findAttribute(LaserControlProductModel::Key::LC_Parameter_No2);

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

    auto sd_parameter = findParameter(ps, LaserControlProductModel::Key::LC_Send_Data);
    const auto sd_attribute = findAttribute(LaserControlProductModel::Key::LC_Send_Data);

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

void LaserControlProductModel::init()
{
    if (!m_product)
    {
        return;
    }
    if (!m_product->hardwareParameters())
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
            if (compareParameterSet(ps, index(i, 0)) && m_product->laserControlPreset() == index(i, 0).data(Qt::UserRole + 1))
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

    if (auto delayParameter = findParameter(ps, LaserControlProductModel::Key::LC_Parameter_No2))
    {
        setDelayEnabledDirect(true);
        setDelayDirect(delayParameter->value().toInt());
    }
}

}
}
