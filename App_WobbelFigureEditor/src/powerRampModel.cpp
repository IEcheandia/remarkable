#include <precitec/dataSet.h>

#include "powerRampModel.h"

#include "selectionHandler.h"
#include "LaserPoint.h"
#include "figureEditorSettings.h"

using precitec::scanmaster::components::wobbleFigureEditor::FigureEditorSettings;
using precitec::gui::components::plotter::DataSet;

namespace precitec
{
namespace scantracker
{
namespace components
{
namespace wobbleFigureEditor
{

PowerRampModel::PowerRampModel(QObject* parent)
    : QAbstractTableModel(parent)
    , m_laserPowers(new DataSet(this))
    , m_ringPowers(new DataSet(this))
{
    m_laserPowers->setColor(Qt::black);
    m_ringPowers->setColor(Qt::red);

    m_roles[NameRole] = "name";
    m_roles[IdRole] = "ID";
    m_roles[PowerRole] = "power";
    m_roles[RingPowerRole] = "ringPower";
    m_roles[ResetPowerRole] = "resetPower";
    m_roles[AddRole] = "add";
    m_roles[ViewRole] = "view";
    m_roles[DeleteRole] = "delete";

    m_rows.resize(2);

    updateValidity();

    connect(this, &PowerRampModel::modifyLaserPowerChanged, this, &PowerRampModel::updateValidity);
    connect(this, &PowerRampModel::modifyRingPowerChanged, this, &PowerRampModel::updateValidity);
    connect(this, &PowerRampModel::asOffsetChanged, this, &PowerRampModel::updateValidity);
    connect(this, &PowerRampModel::dualChannelChanged, this, &PowerRampModel::updateValidity);
}

PowerRampModel::~PowerRampModel() = default;

void PowerRampModel::reset(int laserPointCount)
{
    beginResetModel();

    m_laserPointCount = laserPointCount;

    m_rows.clear();

    int last = laserPointCount ? laserPointCount - 1 : 0;

    m_rows.push_back(Row{0, 0, 0});

    m_periodic = false;
    m_periodStart = 0;
    m_periodEnd = last;

    endResetModel();

    emit periodicChanged();
    emit periodStartChanged();
    emit periodEndChanged();
    emit rampChanged();
    emit pointCountChanged();
    emit periodStartChanged();
    emit periodEndChanged();

    updateValidity();
}

void PowerRampModel::updateLaserPointCount(int laserPointCount)
{
    m_laserPointCount = laserPointCount;

    beginResetModel();
    endResetModel();
    updateValidity();
}

void PowerRampModel::setPowerLimits(double powerBottom, double powerTop, double ringBottom, double ringTop)
{
    m_lpLimit = {powerBottom, powerTop};
    m_rpLimit = {ringBottom, ringTop};
    updateValidity();
}

void PowerRampModel::setPeriodStart(int id)
{
    m_periodStart = id;
    emit periodStartChanged();
    updateValidity();
}

void PowerRampModel::setPeriodEnd(int id)
{
    m_periodEnd = id;
    emit periodEndChanged();
    updateValidity();
}

QHash<int, QByteArray> PowerRampModel::roleNames() const
{
    return m_roles;
}

int PowerRampModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_rows.size();
}

int PowerRampModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_roles.size();
}

QVariant PowerRampModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    int const i = index.row();
    if (i < 0 || i > static_cast<int>(m_rows.size()))
    {
        Q_ASSERT(false);
        return {};
    }

    Row const& row = m_rows[i];

    switch (role)
    {
    case NameRole:
        if (m_rows.size() == 1)
        {
            return tr("Value");
        }

        if (i == 0)
        {
            return tr("Start");
        }

        if (i == static_cast<int>(m_rows.size()) - 1)
        {
            return tr("End");
        }

        return QString(tr("Point %1")).arg(i);
    case IdRole:
        return row.id;
    case PowerRole:
        return row.power;
    case RingPowerRole:
        Q_ASSERT(m_dualChannel);
        return row.ringPower;
    case ResetPowerRole:
    case AddRole:
    case ViewRole:
    case DeleteRole:
        return {};
    default:
        Q_ASSERT(false);
        return {};
    }
}

bool PowerRampModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_rows.size()))
    {
        return false;
    }

    Row& row = m_rows[index.row()];

    switch (role)
    {
    case IdRole:
        row.id = value.toUInt();
        break;
    case PowerRole:
        row.power = value.toDouble();
        emit rampChanged();
        break;
    case RingPowerRole:
        row.ringPower = value.toDouble();
        emit rampChanged();
        break;
    default:
        Q_ASSERT(false);
        return false;
    }

    //emit dataChanged(index, index, {role});
    emit dataChanged(index.siblingAtColumn(0), index.siblingAtColumn(LastCol), {role}); // NOTE: Workaround because otherwise the UI is not updated when using the reset button.

    updateValidity();
    return true;
}

void PowerRampModel::deletePoint(int rowIdx)
{
    if (rowIdx < 0 || rowIdx >= static_cast<int>(m_rows.size()))
    {
        Q_ASSERT(false);
        return;
    }

    if (m_rows.size() <= 1)
    {
        Q_ASSERT(!"should not be offered by the UI");
        return;
    }

    beginRemoveRows(QModelIndex(), rowIdx, rowIdx);
    m_rows.erase(m_rows.begin() + rowIdx);
    endRemoveRows();

    emit rampChanged();
    emit pointCountChanged();
    emit validChanged();
    emit pointIdValidityChanged();
}

void PowerRampModel::addPoint(int rowIdx)
{
    if (rowIdx < 0 || rowIdx >= static_cast<int>(m_rows.size()))
    {
        Q_ASSERT(false);
        return;
    }

    if (rowIdx + 1 != static_cast<int>(m_rows.size()))
    {
        ++rowIdx;
    }

    beginInsertRows(QModelIndex(), rowIdx, rowIdx);

    Row row;

    if (m_rows.size() == 1)
    {
        row = m_rows.front();
        row.id = m_laserPointCount - 1;
        m_rows.push_back(row);
    }
    else
    {
        Row const& prev = m_rows[rowIdx - 1];
        Row const& next = m_rows[rowIdx];

        row.id = (prev.id + next.id) / 2;
        row.power = (prev.power + next.power) / 2;
        row.ringPower = (prev.ringPower + next.ringPower) / 2;
        m_rows.insert(m_rows.begin() + rowIdx, row);
    }

    endInsertRows();
    emit rampChanged();
    emit pointCountChanged();
    updateValidity();
}

void PowerRampModel::setPeriodic(bool val)
{
    if (val == m_periodic)
    {
        return;
    }

    m_periodic = val;
    emit periodicChanged();
    emit rampChanged();
    updateValidity();
}

precitec::gui::components::plotter::DataSet* PowerRampModel::laserPowers()
{
    m_laserPowers->clear();

    if (m_rows.size() == 1)
    {
        double const val = m_rows.front().power;
        m_laserPowers->addSample(QVector2D(0, val));
        m_laserPowers->addSample(QVector2D(m_laserPointCount - 1, val));
    }
    else
    {
        for (Row const& row : m_rows)
        {
            m_laserPowers->addSample(QVector2D(row.id, row.power));
        }
    }

    return m_laserPowers;
}

precitec::gui::components::plotter::DataSet* PowerRampModel::ringPowers()
{
    m_ringPowers->clear();

    if (m_rows.size() == 1)
    {
        double const val = m_rows.front().ringPower;
        m_ringPowers->addSample(QVector2D(0, val));
        m_ringPowers->addSample(QVector2D(m_laserPointCount - 1, val));
    }
    else
    {
        for (Row const& row : m_rows)
        {
            m_ringPowers->addSample(QVector2D(row.id, row.ringPower));
        }
    }

    return m_ringPowers;
}

std::vector<int> PowerRampModel::pointIds() const
{
    std::vector<int> ret;
    for (Row const& row : m_rows)
    {
        ret.push_back(row.id);
    }

    return ret;
}

void PowerRampModel::updateValidity()
{
    {
        bool periodAvailable =
            m_rows.size() > 1 &&                                                    // just one value that is applied to all points
            !(m_rows.front().id == 0 && m_rows.back().id + 1 == m_laserPointCount); // the ramp covers all points, repetition makes no sense

        if (periodAvailable != m_periodAvailable)
        {
            m_periodAvailable = periodAvailable;
            emit periodAvailableChanged();
        }
    }

    bool allValid = true;

    auto check = [&](PowerLimit const& pl, double val)
    {
        if (!std::isfinite(val))
        {
            return false;
        }

        if (m_asOffset)
        {
            return val >= -pl.high && val <= pl.high;
        }
        else
        {
            return val == -1 || (val >= pl.low && val <= pl.high);
        }
    };

    std::vector<bool> idValidity, powerValidity, ringPowerValidity;

    if (m_rows.size() == 1)
    {
        idValidity.push_back(true);
    }
    else
    {
        for (size_t i = 0; i < m_rows.size(); ++i)
        {
            auto const & row = m_rows[i];
            bool valid = isValidId(row.id);

            if (i + 1 != m_rows.size() && row.id >= m_rows[i + 1].id)
            { // the ID of a ramp point must be smaller than the ID of its successor
                valid = false;
            }

            allValid &= valid;
            idValidity.push_back(valid);
        }
    }

    for (auto const & row : m_rows)
    {
        powerValidity.push_back(!m_modifyLaserPower || check(m_lpLimit, row.power));
        ringPowerValidity.push_back(!m_modifyRingPower || check(m_rpLimit, row.ringPower));
    }

    if (idValidity != m_idValidity)
    {
        m_idValidity = idValidity;
        emit pointIdValidityChanged();
    }

    if (powerValidity != m_powerValidity)
    {
        m_powerValidity = powerValidity;
        emit powerValidityChanged();
    }

    if (ringPowerValidity != m_ringPowerValidity)
    {
        m_ringPowerValidity = ringPowerValidity;
        emit ringPowerValidityChanged();
    }

    for (auto const & v : {idValidity, powerValidity, ringPowerValidity})
    {
        for (auto const & x : v)
        {
            allValid &= x;
        }
    }

    if (m_periodAvailable && m_periodic)
    {
        bool const valid = isValidId(m_periodStart) && isValidId(m_periodEnd) && m_periodStart < m_periodEnd;
        allValid &= valid;

        if (m_periodValid != valid)
        {
            m_periodValid = valid;
            emit periodValidChanged();
        }
    }

    if (allValid != m_valid)
    {
        m_valid = allValid;
        emit validChanged();
    }
}
}
}
}
}
