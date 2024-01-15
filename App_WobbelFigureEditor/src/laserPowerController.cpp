#include "laserPowerController.h"

#include "WobbleFigureEditor.h"
#include "powerRampModel.h"
#include "LaserPoint.h"

namespace precitec
{
namespace scantracker
{
namespace components
{
namespace wobbleFigureEditor
{

LaserPowerController::LaserPowerController(QObject* parent) : QObject(parent)
{ }

LaserPowerController::~LaserPowerController() = default;

void LaserPowerController::setFigureEditor(WobbleFigureEditor* newEditor)
{
    if (m_figureEditor == newEditor)
    {
        return;
    }

    disconnect(m_figureEditorDestroyedConnection);
    m_figureEditor = newEditor;

    if (m_figureEditor)
    {
        m_figureEditorDestroyedConnection = connect(m_figureEditor, &QObject::destroyed, this, std::bind(&LaserPowerController::setFigureEditor, this, nullptr));
    }
    else
    {
        m_figureEditorDestroyedConnection = {};
    }

    emit figureEditorChanged();
}

void LaserPowerController::setPowerRampModel(PowerRampModel* newRampModel)
{
    if (m_powerRampModel == newRampModel)
    {
        return;
    }

    disconnect(m_powerRampModelDestroyedConnection);
    m_powerRampModel = newRampModel;

    if (m_powerRampModel)
    {
        m_powerRampModelDestroyedConnection = connect(m_powerRampModel, &QObject::destroyed, this, std::bind(&LaserPowerController::setPowerRampModel, this, nullptr));
    }
    else
    {
        m_powerRampModelDestroyedConnection = {};
    }

    emit powerRampModelChanged();
}

void LaserPowerController::applyPowerRampChanges(bool modifyDefault, bool asOffset)
{
    if (!m_powerRampModel || !m_figureEditor)
    {
        return;
    }

    bool const modifyLaserPower = m_powerRampModel->modifyLaserPower();
    bool const modifyRingPower = m_powerRampModel->modifyRingPower();
    Q_ASSERT(modifyLaserPower || modifyRingPower);

    if (!m_powerRampModel->valid())
    {
        Q_ASSERT(!"UI should not offer this function if the input is invalid");
        return;
    }

    auto& seamPoints = m_figureEditor->getSeamFigure()->figure;
    auto& nodes = m_figureEditor->figure()->get_nodes();

    std::vector<PowerRampModel::Row> rows = m_powerRampModel->getRows();
    bool modifyZeros = true;
    if (rows.size() == 1)
    { // if there is only one row then create a dummy-ramp that applies it as constant
        modifyZeros = false; // zero powers shall not be modified when there is no ramp
        rows.resize(2);
        rows[1] = rows[0];
        rows[0].id = 0;
        rows[1].id = m_figureEditor->numberOfPoints() - 1;
    }

    std::vector<double> rampLens(rows.size() - 1);

    auto v2 = [](std::pair<double, double> const& p)
    {
        return QVector2D(p.first, p.second);
    };

    for (size_t i = 0; i < rampLens.size(); ++i)
    {
        size_t startIdx = rows[i].id;
        size_t endIdx = rows[i + 1].id;
        Q_ASSERT(startIdx < endIdx);

        double len = 0;
        for (size_t j = startIdx; j < endIdx; ++j)
        {
            len += v2(seamPoints[j].endPosition).distanceToPoint(v2(seamPoints[j + 1].endPosition));
        }
        rampLens[i] = len;
    }

    size_t rowIdx = 0;  // index of the row where the current ramp starts
    double rampPos = 0; // distance that we have advanced since the current ramp start

    bool const periodic = m_powerRampModel->periodic() && m_powerRampModel->periodAvailable();
    size_t const startIdx = periodic ? m_powerRampModel->periodStart() : rows.front().id;
    size_t const endIdx = periodic ? m_powerRampModel->periodEnd() : rows.back().id;
    Q_ASSERT(startIdx < endIdx);

    if (periodic)
    {
        // Start where the first ramp is defined. Iterate ramps and points until
        // we reach the point where the periodic application should start.
        for (size_t i = rows.front().id; i != startIdx;)
        {
            if (i < startIdx)
            { // Seek forward
                rampPos += v2(seamPoints[i].endPosition).distanceToPoint(v2(seamPoints[i + 1].endPosition));

                while (rampPos > rampLens[rowIdx])
                {
                    rampPos -= rampLens[rowIdx];
                    rowIdx = (rowIdx + 1) % rampLens.size();
                }

                ++i;
            }
            else
            { // Seek backward
                rampPos -= v2(seamPoints[i].endPosition).distanceToPoint(v2(seamPoints[i - 1].endPosition));

                while (rampPos < 0)
                {
                    rowIdx = rowIdx ? rowIdx - 1 : rampLens.size() - 1;
                    rampPos += rampLens[rowIdx];
                }

                --i;
            }
        }
    }

    double prevPower = -1;
    double prevRingPower = -1;
    for (size_t i = startIdx;; ++i)
    {
        double const s = rampPos / rampLens[rowIdx];
        auto const& a = rows[rowIdx];
        auto const& b = rows[rowIdx + 1];

        double power = a.power + s * (b.power - a.power);
        double ringPower = a.ringPower + s * (b.ringPower - a.ringPower);

        RTC6::seamFigure::command::Order& seamPoint = seamPoints[i];
        Q_ASSERT(m_powerRampModel->laserPowerLimits().check(seamPoint.power));
        Q_ASSERT(m_powerRampModel->ringPowerLimits().check(seamPoint.ringPower));

        if (asOffset)
        {
            if (seamPoint.power != -1)
            {
                power += seamPoint.power * 100;
            }
            else
            {
                power += prevPower * 100;
            }

            if (seamPoint.ringPower != -1)
            {
                ringPower += seamPoint.ringPower * 100;
            }
            else
            {
                ringPower += prevRingPower * 100;
            }
        }

        m_powerRampModel->laserPowerLimits().clamp(power);
        m_powerRampModel->ringPowerLimits().clamp(ringPower);

        auto laserPoint = dynamic_cast<LaserPoint*>(nodes.at(i));

        if (modifyLaserPower && (modifyDefault || seamPoint.power != -1.0) && (modifyZeros || seamPoint.power))
        {
            prevPower = seamPoint.power = power / 100;
            laserPoint->setLaserPower(power);
        }

        if (modifyRingPower && (modifyDefault || seamPoint.ringPower != -1.0) && (modifyZeros || seamPoint.ringPower))
        {
            prevRingPower = seamPoint.ringPower = ringPower / 100;
            laserPoint->setRingPower(ringPower);
        }

        if (i == endIdx)
        {
            break;
        }

        rampPos += v2(seamPoints[i].endPosition).distanceToPoint(v2(seamPoints[i + 1].endPosition));

        while (rampPos > rampLens[rowIdx])
        {
            rampPos -= rampLens[rowIdx];
            rowIdx = (rowIdx + 1) % rampLens.size();
        }
    }
}

}
}
}
}
