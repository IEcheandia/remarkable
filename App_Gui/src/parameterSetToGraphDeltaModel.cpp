#include "parameterSetToGraphDeltaModel.h"
#include "attribute.h"
#include "attributeModel.h"
#include "graphFunctions.h"
#include "graphModel.h"
#include "parameterSet.h"
#include "subGraphModel.h"
#include "seam.h"
#include "seamSeries.h"
#include "product.h"

#include "../../App_Storage/src/compatibility.h"

namespace precitec
{

using storage::ParameterSet;
using storage::graphFunctions::getGraphFromModel;

using namespace storage::compatibility;

namespace gui
{

ParameterSetToGraphDeltaModel::ParameterSetToGraphDeltaModel(QObject *parent)
    : AbstractParameterSetDeltaModel(parent)
{
    connect(this, &ParameterSetToGraphDeltaModel::seamChanged, this,
        [this]
        {
            if (!seam())
            {
                m_longestValueGraph = QString{};
                m_longestValueSeam = QString{};
                emit longestValueGraphChanged();
                emit longestValueSeamChanged();
            }
        }
    );
    connect(this, &ParameterSetToGraphDeltaModel::modelReset, this, &ParameterSetToGraphDeltaModel::longestValueGraphChanged);
    connect(this, &ParameterSetToGraphDeltaModel::modelReset, this, &ParameterSetToGraphDeltaModel::longestValueSeamChanged);
}

ParameterSetToGraphDeltaModel::~ParameterSetToGraphDeltaModel() = default;

void ParameterSetToGraphDeltaModel::init()
{
    QTextStream out{stdout};
    auto *seam = this->seam();
    auto *graphModel = this->graphModel();
    auto *subGraphModel = this->subGraphModel();
    auto *attributeModel = this->attributeModel();
    if (!seam || !graphModel || !subGraphModel || !attributeModel)
    {
        return;
    }
    beginResetModel();

    setGraph(getGraphFromModel(seam, graphModel, subGraphModel));

    auto *seamDeltaSet = new ParameterSet{QUuid::createUuid(), this};
    auto *graphDeltaSet = new ParameterSet{QUuid::createUuid(), this};

    m_longestValueSeam = QString{};
    m_longestValueGraph = QString{};

    if (auto *parameterSet = seam->seamSeries()->product()->filterParameterSet(seam->graphParamSet()))
    {
        for (const auto &instanceFilter : graph().instanceFilters)
        {
            for (const auto &attribute : instanceFilter.attributes)
            {
                if (auto *parameter = parameterSet->findById(toQt(attribute.instanceAttributeId)))
                {
                    QVariant value;
                    if (auto *a = attributeModel->findAttribute(toQt(attribute.attributeId)))
                    {
                        value = a->convert(attribute.value);
                    }
                    else if (attribute.value.isString())
                    {
                        value = QString::fromStdString(attribute.value.convert<std::string>());
                    }

                    if (value != parameter->value())
                    {
                        seamDeltaSet->addParameter(parameter->duplicate(seamDeltaSet));
                        auto *graphParameter = parameter->duplicate(graphDeltaSet);
                        graphParameter->setValue(value);
                        graphDeltaSet->addParameter(graphParameter);

                        const auto seamValue = parameter->value().toString();
                        if (seamValue.length() > m_longestValueSeam.length())
                        {
                            m_longestValueSeam = seamValue;
                        }
                        if (value.isValid())
                        {
                            const auto graphValue = value.toString();
                            if (graphValue.length() > m_longestValueGraph.length())
                            {
                                m_longestValueGraph = graphValue;
                            }
                        }
                    }
                }
            }
        }
    }

    std::vector<ParameterSet*> sets{seamDeltaSet, graphDeltaSet};
    setParameterSets(std::move(sets));

    endResetModel();
}

precitec::storage::Parameter *ParameterSetToGraphDeltaModel::getFilterParameter(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return nullptr;
    }

    auto *parameterSet = this->seam()->seamSeries()->product()->filterParameterSet(this->seam()->graphParamSet());;
    auto *parameter = index.data().value<storage::Parameter*>();
    if (!parameterSet || !parameter)
    {
        return nullptr;
    }
    return parameterSet->findById(parameter->uuid());
}

void ParameterSetToGraphDeltaModel::updateFilterParameter(const QModelIndex &index, const QVariant &value)
{
    auto *parameter = getFilterParameter(index);
    if (!parameter)
    {
        return;
    }
    parameter->setValue(value);
    index.data().value<storage::Parameter*>()->setValue(parameter->value());
    emit dataChanged(index, index, {Qt::DisplayRole});
}


}
}
