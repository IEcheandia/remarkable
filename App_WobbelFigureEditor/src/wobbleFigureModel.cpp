#include "wobbleFigureModel.h"

namespace precitec
{
namespace scanmaster
{
namespace components
{
namespace wobbleFigureEditor
{
WobbleFigureModel::WobbleFigureModel(QObject* parent) : QAbstractListModel(parent)
{
    connect(this, &WobbleFigureModel::isDualChannelModulationChanged, this, &WobbleFigureModel::updateMicroVectorFactorByIsDualModulation);
    connect(this, &WobbleFigureModel::lowestFrequencyChanged, this, &WobbleFigureModel::updateMicroVectorFactorByLowestFrequency);
    connect(this, &WobbleFigureModel::highestFrequencyChanged, this, &WobbleFigureModel::updateMicroVectorFactorByHighestFrequency);
    connect(this, &WobbleFigureModel::microVectorFactorChanged, this, &WobbleFigureModel::resetModel);
    connect(this, &WobbleFigureModel::reset, this, &WobbleFigureModel::resetModel);
}

WobbleFigureModel::~WobbleFigureModel() = default;

void WobbleFigureModel::setPointCount(unsigned int newNumberOfPoints)
{
    if (m_pointCount == newNumberOfPoints)
    {
        return;
    }

    m_pointCount = newNumberOfPoints;
    emit pointCountChanged();
}

void WobbleFigureModel::setIsDualChannelModulation(bool isDualChannel)
{
    if (m_isDualChannelModulation == isDualChannel)
    {
        return;
    }
    m_isDualChannelModulation = isDualChannel;
    emit isDualChannelModulationChanged();
}

void WobbleFigureModel::setMicroVectorFactor(unsigned int newMicroVectorFactor)
{
    if (m_microVectorFactor == newMicroVectorFactor)
    {
        return;
    }
    m_microVectorFactor = newMicroVectorFactor;
    emit microVectorFactorChanged();
}

void WobbleFigureModel::setFrequencyIndex(const QModelIndex& newModelIndex)
{
    if (m_frequencyIndex == newModelIndex)
    {
        return;
    }
    m_frequencyIndex = newModelIndex;
    emit frequencyIndexChanged();
    emit frequencyChanged();
}

void WobbleFigureModel::setLowestFrequency(int newFrequency)
{
    if (m_lowestFrequency == newFrequency)
    {
        return;
    }
    m_lowestFrequency = newFrequency;
    emit lowestFrequencyChanged();
}

void WobbleFigureModel::setHighestFrequency(int newFrequency)
{
    if (m_highestFrequency == newFrequency)
    {
        return;
    }
    m_highestFrequency = newFrequency;
    emit highestFrequencyChanged();
}

QHash<int, QByteArray> WobbleFigureModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("frequency")},
        {Qt::UserRole, QByteArrayLiteral("microVectorFactor")}                      //Hard coded in WobbleProperties.qml
    };
}

QVariant WobbleFigureModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    const auto &wobbleFrequencyInformation = m_possibleFrequencies.at(index.row());

    switch (role)
    {
        case Qt::DisplayRole:
            return wobbleFrequencyInformation.frequency;
        case Qt::UserRole:
            return wobbleFrequencyInformation.microVectorFactor;
    }

    return {};
}

int WobbleFigureModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_possibleFrequencies.size();
}

bool WobbleFigureModel::checkIfFrequencyIsNotAlreadyPresent(int frequency)
{
    return std::none_of(m_possibleFrequencies.begin(), m_possibleFrequencies.end(), [frequency] (const auto &possibleFrequency)
    {
        return possibleFrequency.frequency == frequency;
    });
}

void WobbleFigureModel::calculatePossibleFrequencies()
{
    beginResetModel();
    m_possibleFrequencies.clear();
    m_possibleFrequencies.reserve(m_counter);

    for (unsigned int microVectorFactor = 1; microVectorFactor <= m_counter; microVectorFactor++)
    {
        //If core and ring is power modulated together then the micro vector factor needs to be even, otherwise the modulation isn't symmetric.
        if (isDualChannelModulation() && !checkIfMicroVectorFactorIsEven(microVectorFactor))
        {
            continue;
        }
        auto frequency = calculateFrequency(microVectorFactor);
        if(frequency <= m_highestFrequency && frequency >= m_lowestFrequency && checkIfFrequencyIsNotAlreadyPresent(frequency))
        {
            m_possibleFrequencies.emplace_back(WobbleFrequencyInformation{frequency, microVectorFactor});
        }
    }
    endResetModel();
}

void WobbleFigureModel::indexByMicroVectorFactor()
{
    auto it = std::find_if(m_possibleFrequencies.begin(), m_possibleFrequencies.end(), [this](const auto &currentMicroVectorFactor){return currentMicroVectorFactor.microVectorFactor == m_microVectorFactor;});
    if (it == m_possibleFrequencies.end())
    {
        setFrequencyIndex({});
    }
    setFrequencyIndex(index(std::distance(m_possibleFrequencies.begin(), it), 0));
}

void WobbleFigureModel::updateMicroVectorFactorByIsDualModulation()
{
    if (m_isDualChannelModulation && !checkIfMicroVectorFactorIsEven(m_microVectorFactor))
    {
        m_microVectorFactor++;
        emit reset();
        emit microVectorFactorWillBeChanged();
        return;
    }

    emit reset();
}

void WobbleFigureModel::updateMicroVectorFactorByLowestFrequency()
{
    calculatePossibleFrequencies();

    if (calculateFrequency(m_microVectorFactor) < m_lowestFrequency)
    {
        m_microVectorFactor = m_possibleFrequencies.back().microVectorFactor;
        indexByMicroVectorFactor();
        emit microVectorFactorWillBeChanged();
        return;
    }

    indexByMicroVectorFactor();
}

void WobbleFigureModel::updateMicroVectorFactorByHighestFrequency()
{
    calculatePossibleFrequencies();

    if (calculateFrequency(m_microVectorFactor) > m_highestFrequency)
    {
        m_microVectorFactor = m_possibleFrequencies.front().microVectorFactor;
        indexByMicroVectorFactor();
        emit microVectorFactorWillBeChanged();
        return;
    }

    indexByMicroVectorFactor();
}

bool WobbleFigureModel::checkIfMicroVectorFactorIsEven(int microVectorFactor)
{
    return microVectorFactor % 2 == 0;
}

int WobbleFigureModel::calculateFrequency(unsigned int microVectorFactor)
{
    if (m_pointCount == 0 || microVectorFactor == 0)
    {
        return 0;
    }
    return qRound(1.0 / ((m_pointCount - 1) * microVectorFactor * 0.00001));
}

void WobbleFigureModel::resetModel()
{
    calculatePossibleFrequencies();
    indexByMicroVectorFactor();
}

}
}
}
}
