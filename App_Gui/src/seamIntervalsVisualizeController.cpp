#include "seamIntervalsVisualizeController.h"
#include "seam.h"
#include "seamInterval.h"
#include "guiConfiguration.h"

using precitec::storage::Seam;
using precitec::storage::SeamInterval;

namespace precitec
{
namespace gui
{

SeamIntervalsVisualizeController::SeamIntervalsVisualizeController(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &SeamIntervalsVisualizeController::currentSeamChanged, this, &SeamIntervalsVisualizeController::updateModel);
    connect(GuiConfiguration::instance(), &GuiConfiguration::seamIntervalsOnProductStructureChanged, this, &SeamIntervalsVisualizeController::updateModel);
}

SeamIntervalsVisualizeController::~SeamIntervalsVisualizeController() = default;

int SeamIntervalsVisualizeController::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !m_currentSeam)
    {
        return 0;
    }
    return m_intervals.size();
}

QHash<int, QByteArray> SeamIntervalsVisualizeController::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("interval")},
        {Qt::UserRole, QByteArrayLiteral("uuid")},
        {Qt::UserRole + 1, QByteArrayLiteral("relativeLength")},
        {Qt::UserRole + 2, QByteArrayLiteral("accumulatedLength")},
        {Qt::UserRole + 3, QByteArrayLiteral("relativeAccumulatedLength")},
        {Qt::UserRole + 4, QByteArrayLiteral("minAccumulatedLength")}
    };
}

QVariant SeamIntervalsVisualizeController::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !m_currentSeam)
    {
        return {};
    }

    auto interval = m_intervals.at(index.row());

    const auto accumulated = accumulatedLength(index.row());

    switch (role)
    {
        case Qt::DisplayRole:
            return QVariant::fromValue(interval);
        case Qt::UserRole:
            return interval->uuid();
        case Qt::UserRole + 1:
            return double(interval->length()) / double(m_currentSeam->length());
        case Qt::UserRole + 2:
            return 0.001 * accumulated;
        case Qt::UserRole + 3:
            return accumulated / double(m_currentSeam->length());
        case Qt::UserRole + 4:
            return accumulatedLength(index.row() - 1);
    }

    return {};
}

void SeamIntervalsVisualizeController::setCurrentSeam(Seam *seam)
{
    if (m_currentSeam == seam)
    {
        return;
    }
    if (m_currentSeam)
    {
        disconnect(m_seamDestroyConnection);
        disconnect(m_currentSeam, &Seam::allSeamIntervalsChanged, this,  &SeamIntervalsVisualizeController::updateModel);
    }
    m_currentSeam = seam;

    if (m_currentSeam)
    {
        m_seamDestroyConnection = connect(m_currentSeam, &QObject::destroyed, this, std::bind(&SeamIntervalsVisualizeController::setCurrentSeam, this, nullptr));
        connect(m_currentSeam, &Seam::lengthChanged, this,  &SeamIntervalsVisualizeController::updateModel);
    } else
    {
        m_seamDestroyConnection = {};
    }
    emit currentSeamChanged();
}

Qt::ItemFlags SeamIntervalsVisualizeController::flags(const QModelIndex& index) const
{
    Q_UNUSED(index)
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

bool SeamIntervalsVisualizeController::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid())
    {
        return false;
    }

    if (role == Qt::UserRole + 2)
    {
        const auto accumulatedPrevious = accumulatedLength(index.row() - 1);
        m_intervals.at(index.row())->setLength(1000 * value.toDouble() - accumulatedPrevious);
        return true;
    }

    return false;
}

void SeamIntervalsVisualizeController::updateModel()
{
    beginResetModel();

    m_intervals.clear();

    if (m_currentSeam && GuiConfiguration::instance()->seamIntervalsOnProductStructure())
    {
        m_intervals = m_currentSeam->seamIntervals();
    }

    endResetModel();
}

double SeamIntervalsVisualizeController::accumulatedLength(int index) const
{
    return std::accumulate(m_intervals.begin(), std::next(m_intervals.begin(), index + 1), 0.0, [] (auto sum, auto interval) { return sum + interval->length(); });
}

QColor SeamIntervalsVisualizeController::colorForLevel(int level) const
{
    return Seam::levelColor(level);
}

}
}
