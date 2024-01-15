#include "hardwareParameterSeriesModel.h"
#include "attribute.h"
#include "attributeModel.h"
#include "seamSeries.h"
#include "parameter.h"
#include "parameterSet.h"

#include <QMetaEnum>

using precitec::storage::AttributeModel;
using precitec::storage::SeamSeries;
using precitec::storage::ParameterSet;

namespace precitec
{
namespace gui
{

HardwareParameterSeriesModel::HardwareParameterSeriesModel(QObject *parent)
    : AbstractHardwareParameterModel(parent)
{
    connect(this, &HardwareParameterSeriesModel::seamSeriesChanged, this,
        [this]
        {
            dataChanged(index(0, 0), index(rowCount() - 1, 0), {});
        }
    );
}

HardwareParameterSeriesModel::~HardwareParameterSeriesModel() = default;

void HardwareParameterSeriesModel::setSeamSeries(SeamSeries* seamSeries)
{
    if (m_seamSeries == seamSeries)
    {
        return;
    }
    m_seamSeries = seamSeries;
    disconnect(m_seamSeriesDestroyed);
    if (m_seamSeries)
    {
        m_seamSeriesDestroyed = connect(m_seamSeries, &SeamSeries::destroyed, this, std::bind(&HardwareParameterSeriesModel::setSeamSeries, this, nullptr));
    } else
    {
        m_seamSeriesDestroyed = QMetaObject::Connection{};
    }
    emit seamSeriesChanged();
}

ParameterSet *HardwareParameterSeriesModel::getParameterSet() const
{
    if (!m_seamSeries)
    {
        return nullptr;
    }
    if (!m_seamSeries->hardwareParameters())
    {
        m_seamSeries->setHardwareParameters(new ParameterSet{QUuid::createUuid(), m_seamSeries});
    }
    return m_seamSeries->hardwareParameters();
}

ParameterSet *HardwareParameterSeriesModel::getParameterSetDirect() const
{
    if (!m_seamSeries)
    {
        return nullptr;
    }
    return m_seamSeries->hardwareParameters();
}

}
}


