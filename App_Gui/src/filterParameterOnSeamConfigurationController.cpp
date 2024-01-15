#include "filterParameterOnSeamConfigurationController.h"

#include "parameter.h"
#include "parameterSet.h"
#include "product.h"
#include "seam.h"
#include "seamSeries.h"
#include "graphFunctions.h"
#include "subGraphModel.h"

namespace precitec
{

using storage::Seam;
using storage::SubGraphModel;
using storage::graphFunctions::getCurrentGraphId;

namespace gui
{

FilterParameterOnSeamConfigurationController::FilterParameterOnSeamConfigurationController(QObject* parent)
    : QObject(parent)
{
    connect(this, &FilterParameterOnSeamConfigurationController::currentSeamChanged, this, &FilterParameterOnSeamConfigurationController::graphIdChanged);
}

FilterParameterOnSeamConfigurationController::~FilterParameterOnSeamConfigurationController() = default;

void FilterParameterOnSeamConfigurationController::setCurrentSeam(Seam* seam)
{
    if (m_currentSeam == seam)
    {
        return;
    }
    if (m_currentSeam)
    {
        disconnect(m_currentSeam, &Seam::graphChanged, this, &FilterParameterOnSeamConfigurationController::graphIdChanged);
        disconnect(m_currentSeam, &Seam::subGraphChanged, this, &FilterParameterOnSeamConfigurationController::graphIdChanged);
    }
    m_currentSeam = seam;
    disconnect(m_destroyConnection);
    m_destroyConnection = QMetaObject::Connection{};
    if (m_currentSeam)
    {
        m_destroyConnection = connect(m_currentSeam, &QObject::destroyed, this, std::bind(&FilterParameterOnSeamConfigurationController::setCurrentSeam, this, nullptr));
        connect(m_currentSeam, &Seam::graphChanged, this, &FilterParameterOnSeamConfigurationController::graphIdChanged);
        connect(m_currentSeam, &Seam::subGraphChanged, this, &FilterParameterOnSeamConfigurationController::graphIdChanged);
    }
    emit currentSeamChanged();
}

void FilterParameterOnSeamConfigurationController::setSubGraphModel(SubGraphModel *subGraphModel)
{
    if (m_subGraphModel == subGraphModel)
    {
        return;
    }
    disconnect(m_subGraphModelDestroyedConnection);
    m_subGraphModel = subGraphModel;
    if (m_subGraphModel)
    {
        m_subGraphModelDestroyedConnection = connect(m_subGraphModel, &QObject::destroyed, this, std::bind(&FilterParameterOnSeamConfigurationController::setSubGraphModel, this, nullptr));
    } else
    {
        m_subGraphModelDestroyedConnection = {};
    }
    emit subGraphModelChanged();
}

QUuid FilterParameterOnSeamConfigurationController::currentGraphId() const
{
    return getCurrentGraphId(m_currentSeam, m_subGraphModel);
}

precitec::storage::Parameter *FilterParameterOnSeamConfigurationController::getFilterParameter(const QUuid &uuid, precitec::storage::Attribute *attribute, const QUuid &filterId, const QVariant &defaultValue) const
{
    auto paramSet = currentParameterSet();
    if (!paramSet)
    {
        return nullptr;
    }
    auto it = std::find_if(paramSet->parameters().begin(), paramSet->parameters().end(), [uuid] (auto param) { return param->uuid() == uuid; });
    if (it != paramSet->parameters().end())
    {
        return *it;
    }

    if (attribute)
    {
        // add to the ParameterSet
        return paramSet->createParameter(uuid, attribute, filterId, defaultValue);
    }

    return nullptr;
}

precitec::storage::ParameterSet *FilterParameterOnSeamConfigurationController::currentParameterSet() const
{
    if (!m_currentSeam)
    {
        return nullptr;
    }
    return m_currentSeam->seamSeries()->product()->filterParameterSet(m_currentSeam->graphParamSet());
}

void FilterParameterOnSeamConfigurationController::updateFilterParameter(const QUuid &uuid, const QVariant &value)
{
    if (!m_currentSeam)
    {
        return;
    }

    auto paramSet = m_currentSeam->seamSeries()->product()->filterParameterSet(m_currentSeam->graphParamSet());
    if (!paramSet)
    {
        return;
    }
    auto it = std::find_if(paramSet->parameters().begin(), paramSet->parameters().end(), [uuid] (auto param) { return param->uuid() == uuid; });
    if (it == paramSet->parameters().end())
    {
        return;
    }
    (*it)->setValue(value);
    emit markAsChanged();
}

}
}
