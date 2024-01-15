#include "abstractMultiSeamDataModel.h"
#include "seam.h"
#include "precitec/multicolorSet.h"
#include "precitec/dataSet.h"
#include "precitec/infiniteSet.h"

using precitec::storage::Seam;
using precitec::storage::ResultSetting;
using precitec::interface::ResultArgs;
using precitec::gui::components::plotter::MulticolorSet;
using precitec::gui::components::plotter::DataSet;
using precitec::gui::components::plotter::InfiniteSet;

namespace precitec
{
namespace gui
{

AbstractMultiSeamDataModel::AbstractMultiSeamDataModel(QObject* parent)
    : AbstractPlotterDataModel(parent)
{
    const auto sampleDataChanged = [this]
    {
        emit dataChanged(index(0), index(rowCount() - 1), {Qt::UserRole + 1, Qt::UserRole + 2, Qt::UserRole + 9, Qt::UserRole + 11});
    };

    connect(this, &AbstractMultiSeamDataModel::currentIndexChanged, sampleDataChanged);
    connect(this, &AbstractMultiSeamDataModel::numberOfSeamsInPlotterChanged, sampleDataChanged);
}

AbstractMultiSeamDataModel::~AbstractMultiSeamDataModel() = default;

QVariant AbstractMultiSeamDataModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || currentIndex() == -1)
    {
        return {};
    }

    const auto& resultsInfo = resultAt(index.row());
    const auto& data = resultsInfo.m_data;

    const auto startIt = data.begin() + currentIndex();
    const auto endIndex = uint(currentIndex() + numberOfSeamsInPlotter());
    const auto endIt = endIndex < data.size() ? data.begin() + endIndex : data.end();

    const auto vectorSize = std::distance(startIt, endIt);

    switch (role)
    {
        case Qt::DisplayRole:
            return resultsInfo.m_resultName;
        case Qt::UserRole:
        {
            const auto lastDataOfType = std::find_if(std::make_reverse_iterator(endIt), std::make_reverse_iterator(startIt), [] (const auto& dataSets) { return dataSets.m_signal != nullptr; });
            if (lastDataOfType != data.rend())
            {
                return (*lastDataOfType).m_signal->maxX();
            }
            return {};
        }
        case Qt::UserRole + 1:
        {
            const auto lastDataOfType = std::find_if(std::make_reverse_iterator(endIt), std::make_reverse_iterator(startIt), [] (const auto& dataSets) { return dataSets.m_signal != nullptr; });
            if (lastDataOfType != data.rend() && !(*lastDataOfType).m_signal->isEmpty())
            {
                return (*lastDataOfType).m_signal->lastSampleValue();
            }
            return {};
        }
        case Qt::UserRole + 2:
        {
            if (currentIndex() < int(data.size()))
            {
                std::vector<MulticolorSet*> multicolorSets;
                multicolorSets.reserve(vectorSize);
                auto offset = 0.0f;
                for (auto i = 0u; i < vectorSize; i++)
                {
                    if (auto signal = (*(startIt + i)).m_signal)
                    {
                        signal->setOffset({offset, 0.0f});
                        multicolorSets.push_back(signal);
                    }
                    offset += (qFuzzyIsNull(seamLength(currentIndex() + i)) ? 1.0 : seamLength(currentIndex() + i));
                }
                return QVariant::fromValue(multicolorSets);
            }
            return {};
        }
        case Qt::UserRole + 3:
            return resultsInfo.m_plotterNumber;
        case Qt::UserRole + 4:
            return resultsInfo.m_isBinary;
        case Qt::UserRole + 5:
            return resultsInfo.m_resultType;
        case Qt::UserRole + 6:
            return resultsInfo.m_isVisible;
        case Qt::UserRole + 7:
            return resultsInfo.m_color;
        case Qt::UserRole + 8:
            return resultsInfo.m_isEnabled;
        case Qt::UserRole + 9:
        {
            if (currentIndex() < int(m_separators.size()))
            {
                const auto startSeparatorIt = m_separators.begin() + currentIndex();
                const auto endSeparatorIt = endIndex < m_separators.size() ? m_separators.begin() + endIndex : m_separators.end();
                const std::vector<InfiniteSet*> separatorsInRange(startSeparatorIt, endSeparatorIt - 1);
                auto offset = 0.0;
                for (auto i = 0u; i < separatorsInRange.size(); i++)
                {
                    offset += (qFuzzyIsNull(seamLength(currentIndex() + i)) ? 1.0 : seamLength(currentIndex() + i));
                    if (auto ms = separatorsInRange.at(i))
                    {
                        ms->setValue(offset);
                    }
                }
                return QVariant::fromValue(separatorsInRange);
            }
            return {};
        }
        case Qt::UserRole + 10:
            return resultsInfo.m_isNioPercentageSet;
        case Qt::UserRole + 11:
        {
            if (currentIndex() < int(data.size()))
            {
                std::vector<DataSet*> boundarySets;
                boundarySets.reserve(2 * vectorSize);
                auto offset = 0.0f;
                for (auto i = 0u; i < vectorSize; i++)
                {
                    if (auto top = (*(startIt + i)).m_top)
                    {
                        top->setOffset({offset, 0.0f});
                        boundarySets.push_back(top);
                    }
                    if (auto bottom = (*(startIt + i)).m_bottom)
                    {
                        bottom->setOffset({offset, 0.0f});
                        boundarySets.push_back(bottom);
                    }
                    offset += seamLength(currentIndex() + i);
                }
                return QVariant::fromValue(boundarySets);
            }
            return {};
        }
    }

    return {};
}

void AbstractMultiSeamDataModel::setMaxSeamsLimit(quint32 value)
{
    if (m_maxSeamsLimit == value)
    {
        return;
    }
    m_maxSeamsLimit = value;
    const auto index = currentIndex();

    while (m_seams.size() >= m_maxSeamsLimit)
    {
        m_seams.pop_front();
        popData();
    }

    if (index != currentIndex())
    {
        emit currentIndexChanged();
    }
}

double AbstractMultiSeamDataModel::seamLength(uint index) const
{
    if (index >= m_seams.size())
    {
        return 0.0;
    }
    return m_seams.at(index).second;
}

void AbstractMultiSeamDataModel::updateSeamLength(uint idx)
{
    auto seamLength = 0.0f;
    for (const auto& result : results())
    {
        if (result.m_data.size() > idx)
        {
            if (auto signal = (result.m_data.at(idx)).m_signal)
            {
                seamLength = std::max(seamLength, signal->maxX());
            }
        }
    }

    if (m_seams.size() > idx)
    {
        m_seams.at(idx).second = seamLength;
    }

    emit dataChanged(index(0), index(rowCount() - 1), {Qt::UserRole + 2, Qt::UserRole + 9, Qt::UserRole + 11});
}

void AbstractMultiSeamDataModel::updateSeamLength()
{
    for (auto i = 0u; i < m_seams.size(); i++)
    {
        updateSeamLength(i);
    }
}

int AbstractMultiSeamDataModel::seamIndex(Seam* seam)
{
    const auto it = std::find_if(m_seams.begin(), m_seams.end(), [seam] (auto& s) { return seam == s.first; });
    if (it == m_seams.end())
    {
        return -1;
    }
    return std::distance(m_seams.begin(), it);
}

void AbstractMultiSeamDataModel::previous()
{
    setCurrentIndex(currentIndex() - 1);
}

void AbstractMultiSeamDataModel::next()
{
    setCurrentIndex(currentIndex() + 1);
}

void AbstractMultiSeamDataModel::clear()
{
    beginResetModel();

    AbstractPlotterDataModel::clear();

    m_seams.clear();
    qDeleteAll(m_separators);
    m_separators.clear();

    endResetModel();
}

void AbstractMultiSeamDataModel::addSeam(QPointer<Seam> seam)
{
    m_seams.emplace_back(seam, 0.0);
    m_separators.emplace_back(new InfiniteSet{this, Qt::Vertical});

    addData();

    auto limitReached = false;

    if (m_seams.size() == m_maxSeamsLimit && !m_seams.empty())
    {
        m_seams.pop_front();

        if (!m_separators.empty())
        {
            m_separators.pop_front();
        }

        popData();

        limitReached = true;
    }

    if (!limitReached)
    {
        emit maxIndexChanged();
    }

    if (currentIndex() < 0)
    {
        setCurrentIndex(0);
    }
}

QPointer<Seam> AbstractMultiSeamDataModel::findSeam(uint index) const
{
    if (index >= m_seams.size())
    {
        return nullptr;
    }
    return m_seams.at(index).first;
}

Seam* AbstractMultiSeamDataModel::currentSeam() const
{
    if (currentIndex() == -1)
    {
        return nullptr;
    }
    return findSeam(currentIndex());
}

void AbstractMultiSeamDataModel::insertNewResult(const ResultArgs& result, ResultSetting* resultConfig, int seamIndex, const std::list<std::pair<QVector2D, float>>& signalSamples, const std::list<QVector2D>& upperReferenceSamples, const std::list<QVector2D>& lowerReferenceSamples)
{
    AbstractPlotterDataModel::insertNewResult(result, resultConfig, seamIndex, signalSamples, upperReferenceSamples, lowerReferenceSamples, m_seams.size());
}

void AbstractMultiSeamDataModel::insertNewSensor(int sensorId, ResultSetting* resultConfig, int seamIndex, const std::list<std::pair<QVector2D, float>>& signalSamples)
{
    AbstractPlotterDataModel::insertNewSensor(sensorId, resultConfig, seamIndex, signalSamples, m_seams.size());
}

void AbstractMultiSeamDataModel::addResults(int seamIndex, int resultIndex, const ResultArgs& result, ResultSetting* resultConfig, const std::list<std::pair<QVector2D, float>>& signalSamples, const std::list<QVector2D>& upperReferenceSamples, const std::list<QVector2D>& lowerReferenceSamples)
{
    AbstractPlotterDataModel::addResults(seamIndex, resultIndex, result, resultConfig, signalSamples, upperReferenceSamples, lowerReferenceSamples, m_seams.size());
}

}
}


