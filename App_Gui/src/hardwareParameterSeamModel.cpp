#include "hardwareParameterSeamModel.h"
#include "attribute.h"
#include "attributeModel.h"
#include "seam.h"
#include "seamSeries.h"
#include "product.h"
#include "parameter.h"
#include "parameterSet.h"

#include <QMetaEnum>

using precitec::storage::AttributeModel;
using precitec::storage::Seam;
using precitec::storage::ParameterSet;

namespace precitec
{
namespace gui
{

HardwareParameterSeamModel::HardwareParameterSeamModel(QObject *parent)
    : AbstractHardwareParameterModel(parent)
{
    connect(this, &HardwareParameterSeamModel::seamChanged, this,
        [this]
        {
            dataChanged(index(0, 0), index(rowCount() - 1, 0), {});
        }
    );
}

HardwareParameterSeamModel::~HardwareParameterSeamModel() = default;

void HardwareParameterSeamModel::setSeam(Seam *seam)
{
    if (m_seam == seam)
    {
        return;
    }
    m_seam = seam;
    disconnect(m_seamDestroyed);
    if (m_seam)
    {
        m_seamDestroyed = connect(m_seam, &Seam::destroyed, this, std::bind(&HardwareParameterSeamModel::setSeam, this, nullptr));
    } else
    {
        m_seamDestroyed = QMetaObject::Connection{};
    }
    emit seamChanged();
}

ParameterSet *HardwareParameterSeamModel::getParameterSet() const
{
    if (!m_seam)
    {
        return nullptr;
    }
    if (!m_seam->hardwareParameters())
    {
        m_seam->createHardwareParameters();
    }
    return m_seam->hardwareParameters();
}

ParameterSet *HardwareParameterSeamModel::getParameterSetDirect() const
{
    if (!m_seam)
    {
        return nullptr;
    }
    return m_seam->hardwareParameters();
}

}
}
