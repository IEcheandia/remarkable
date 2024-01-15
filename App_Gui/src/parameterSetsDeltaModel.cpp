#include "parameterSetsDeltaModel.h"
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

#include <guiConfiguration.h>

namespace precitec
{

using storage::ParameterSet;
using storage::graphFunctions::getGraphFromModel;

using namespace storage::compatibility;

namespace gui
{

ParameterSetsDeltaModel::ParameterSetsDeltaModel(QObject *parent)
    : AbstractParameterSetDeltaModel(parent)
{
}

ParameterSetsDeltaModel::~ParameterSetsDeltaModel() = default;

void ParameterSetsDeltaModel::init()
{
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

    // find all seams
    m_measureTaks.clear();
    m_measureTaks.push_back(seam);

    auto *product = seam->seamSeries()->product();
    std::vector<ParameterSet*> parameterSets;
    parameterSets.push_back(product->filterParameterSet(seam->graphParamSet()));
    for (auto *seamSeries : product->seamSeries())
    {
        for (auto *s : seamSeries->seams())
        {
            if (s == seam)
            {
                continue;
            }
            if (s->graph() != seam->graph())
            {
                continue;
            }
            if (s->usesSubGraph() && seam->usesSubGraph())
            {
                if (subGraphModel->generateGraphId(s->subGraphs()) != subGraphModel->generateGraphId(seam->subGraphs()))
                {
                    continue;
                }
            }
            m_measureTaks.push_back(s);
            parameterSets.push_back(product->filterParameterSet(s->graphParamSet()));
        }
    }

    std::vector<ParameterSet*> deltaSets;
    for (std::size_t i = 0u; i < m_measureTaks.size(); i++)
    {
        deltaSets.emplace_back(new ParameterSet{QUuid::createUuid(), this});
    }

    if (auto *parameterSet = parameterSets.front())
    {
        std::vector<storage::Parameter*> parameterInSeams;
        parameterInSeams.reserve(parameterSets.size());
        for (auto *parameter : parameterSet->parameters())
        {
            parameterInSeams.clear();
            parameterInSeams.push_back(parameter);
            for (std::size_t i = 1u; i < parameterSets.size(); i++)
            {
                auto *ps = parameterSets.at(i);
                if (!ps)
                {
                    parameterInSeams.push_back(nullptr);
                    continue;
                }
                parameterInSeams.push_back(ps->findById(parameter->uuid()));
            }
            bool difference = false;
            for (auto *p : parameterInSeams)
            {
                if (!p)
                {
                    difference = true;
                    break;
                }
                if (parameter->value() != p->value())
                {
                    difference = true;
                    break;
                }
            }
            if (!difference)
            {
                continue;
            }
            for (std::size_t i = 0u; i < deltaSets.size(); i++)
            {
                auto *ps = deltaSets.at(i);
                auto *p = parameterInSeams.at(i);
                if (p)
                {
                    ps->addParameter(p->duplicate(ps));
                }
                else
                {
                    auto *p = parameter->duplicate(ps);
                    p->setValue({});
                    ps->addParameter(p);
                }
            }
        }
    }

    // calculate width
    m_maxString.clear();
    m_maxString.reserve(deltaSets.size());
    for (auto *ps : deltaSets)
    {
        QString testString;
        for (auto *p : ps->parameters())
        {
            const auto &compare = p->value().toString();
            if (compare.length() > testString.length())
            {
                testString = compare;
            }
        }
        m_maxString.push_back(testString);
    }

    setParameterSets(std::move(deltaSets));

    endResetModel();
}

QVariant ParameterSetsDeltaModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical)
    {
        return AbstractParameterSetDeltaModel::headerData(section, Qt::Vertical, role);
    }

    if (section < 0 || section >= int(m_measureTaks.size()))
    {
        return {};
    }
    if (role == Qt::DisplayRole)
    {
        auto *seam = m_measureTaks.at(section);
        if (GuiConfiguration::instance()->seamSeriesOnProductStructure())
        {
            return QStringLiteral("%1\n%2").arg(seam->seamSeries()->visualNumber()).arg(seam->visualNumber());
        }
        else
        {
            return QStringLiteral("%2").arg(seam->visualNumber());
        }
    }
    if (role == Qt::UserRole)
    {
        return m_maxString.at(section);
    }
    if (role == Qt::UserRole + 1)
    {
        return QVariant::fromValue(m_measureTaks.at(section));
    }
    return {};
}

precitec::storage::Parameter *ParameterSetsDeltaModel::getFilterParameter(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return nullptr;
    }
    auto *seam = m_measureTaks.at(index.column());
    auto *parameter = index.data().value<storage::Parameter*>();
    if (!parameter || !seam)
    {
        return nullptr;
    }
    auto *parameterSet = seam->seamSeries()->product()->filterParameterSet(seam->graphParamSet());
    if (!parameterSet)
    {
        return nullptr;
    }
    return parameterSet->findById(parameter->uuid());
}

void ParameterSetsDeltaModel::updateFilterParameter(const QModelIndex &index, const QVariant &value)
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
