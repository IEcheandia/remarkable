#include "rampModel.h"

namespace precitec
{
namespace scanmaster
{
namespace components
{
namespace wobbleFigureEditor
{

RampModel::RampModel(QObject* parent) : QObject(parent)
{
    connect(this, &RampModel::startPointIDChanged, this, &RampModel::updateRampModel);
}

RampModel::~RampModel() = default;

void RampModel::setFigureEditor(WobbleFigureEditor* newFigureEditor)
{
    if (m_figureEditor == newFigureEditor)
    {
        return;
    }
    disconnect(m_figureEditorDestroyedConnection);

    if (m_figureEditor)
    {
        disconnect(m_figureEditor, &WobbleFigureEditor::rampsChanged, this, &RampModel::updateRampModel);
    }
    m_figureEditor = newFigureEditor;

    if (m_figureEditor)
    {
        m_figureEditorDestroyedConnection = connect(m_figureEditor, &QObject::destroyed, this, std::bind(&RampModel::setFigureEditor, this, nullptr));
        connect(m_figureEditor, &WobbleFigureEditor::rampsChanged, this, &RampModel::updateRampModel);
    }
    else
    {
        m_figureEditorDestroyedConnection = {};
    }
    emit figureEditorChanged();
}

void RampModel::setStartPointID(int newStartPointID)
{
    if (m_startPointID == newStartPointID)
    {
        return;
    }

    m_startPointID = newStartPointID;
    emit startPointIDChanged();
}

void RampModel::setRampLength(double newRampLength)
{
    if (qFuzzyCompare(m_rampLength, newRampLength))
    {
        return;
    }

    m_rampLength = newRampLength;
    emit rampLengthChanged();
}

void RampModel::setStartPower(double newStartPower)
{
    if (qFuzzyCompare(m_startPower, newStartPower))
    {
        return;
    }

    m_startPower = newStartPower;
    emit startPowerChanged();
}

void RampModel::setEndPower(double newEndPower)
{
    if (qFuzzyCompare(m_endPower, newEndPower))
    {
        return;
    }

    m_endPower = newEndPower;
    emit endPowerChanged();
}

void RampModel::setStartPowerRing(double newStartRingPower)
{
    if (qFuzzyCompare(m_startPowerRing, newStartRingPower))
    {
        return;
    }

    m_startPowerRing = newStartRingPower;
    emit startPowerRingChanged();
}

void RampModel::setEndPowerRing(double newEndRingPower)
{
    if (qFuzzyCompare(m_endPowerRing, newEndRingPower))
    {
        return;
    }

    m_endPowerRing = newEndRingPower;
    emit endPowerRingChanged();
}

void RampModel::setRamps(const std::vector<Ramp>& currentRamps)
{
    if (m_ramps == currentRamps)
    {
        return;
    }

    m_ramps = currentRamps;
    emit rampsChanged();
}

void RampModel::updateRamps()
{
    const auto& rampIterator = searchRamp();

    if (qFuzzyIsNull(rampLength()))
    {
        if (!foundRamp(rampIterator))
        {
            return;
        }
        else
        {
            eraseRamp();
            return;
        }
    }

    if (!foundRamp(rampIterator))
    {
        appendRamp();
        sortRamps();
        giveBackRamps();
        return;
    }

    updateFoundRamp(rampIterator);
    giveBackRamps();
}

void RampModel::eraseRamp()
{
    const auto& rampIterator = searchRamp();
    if (!foundRamp(rampIterator))
    {
        return;
    }

    m_ramps.erase(rampIterator);
    giveBackRamps();
}

void RampModel::takeRamps()
{
    if (!m_figureEditor)
    {
        return;
    }

    setRamps(m_figureEditor->ramps());
}

void RampModel::giveBackRamps()
{
    if (!m_figureEditor)
    {
        return;
    }

    m_figureEditor->setRamps(m_ramps);
}

void RampModel::reset()
{
    setRampLength(0.0);
    setStartPower(0.0);
    setEndPower(0.0);
    setStartPowerRing(0.0);
    setEndPowerRing(0.0);
}

void RampModel::updateRampModel()
{
    reset();

    if (!isStartPointIDValid())
    {
        return;
    }

    takeRamps();

    const auto& rampIterator = searchRamp();
    if (foundRamp(rampIterator))
    {
        updateRampModelProperties(*rampIterator);
    }
}

std::vector<Ramp>::iterator RampModel::searchRamp()
{
    auto foundRamp = std::find_if(m_ramps.begin(), m_ramps.end(), [this](const auto& currentRamp)
    {
        return static_cast<int> (currentRamp.startPointID) == m_startPointID;
    });
    return foundRamp;
}

bool RampModel::foundRamp (std::vector<Ramp>::iterator rampIt)
{
    return rampIt != m_ramps.end();
}

void RampModel::updateRampModelProperties(const Ramp& InfoToBeUpdated)
{
    setRampLength(InfoToBeUpdated.length);
    setStartPower(InfoToBeUpdated.startPower);
    setEndPower(InfoToBeUpdated.endPower);
    setStartPowerRing(InfoToBeUpdated.startPowerRing);
    setEndPowerRing(InfoToBeUpdated.endPowerRing);
}

void RampModel::appendRamp()
{
    m_ramps.emplace_back(Ramp {static_cast<size_t>(m_startPointID), m_rampLength, m_startPower, m_endPower, m_startPowerRing, m_endPowerRing});
}

void RampModel::sortRamps()
{
    std::sort(m_ramps.begin(), m_ramps.end(), [](const auto& firstRamp, const auto& secondRamp)
    {
        return firstRamp.startPointID < secondRamp.startPointID;
    });
}

void RampModel::updateFoundRamp(std::vector<Ramp>::iterator foundRampIt)
{
    foundRampIt->length = m_rampLength;
    foundRampIt->startPower = m_startPower;
    foundRampIt->endPower = m_endPower;
    foundRampIt->startPowerRing = m_startPowerRing;
    foundRampIt->endPowerRing = m_endPowerRing;
}

bool RampModel::isStartPointIDValid()
{
    return m_startPointID > -1;
}

}
}
}
}
