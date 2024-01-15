#include "seamPropertyFilterModel.h"
#include "seamPropertyModel.h"
#include "guiConfiguration.h"

namespace precitec
{
namespace gui
{

SeamPropertyFilterModel::SeamPropertyFilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    connect(GuiConfiguration::instance(), &GuiConfiguration::configureMovingDirectionOnSeamChanged, this, &SeamPropertyFilterModel::invalidate);
    connect(GuiConfiguration::instance(), &GuiConfiguration::configureThicknessOnSeamChanged, this, &SeamPropertyFilterModel::invalidate);
    connect(GuiConfiguration::instance(), &GuiConfiguration::seamIntervalsOnProductStructureChanged, this, &SeamPropertyFilterModel::invalidate);
    connect(this, &SeamPropertyFilterModel::laserControlChanged, this, &SeamPropertyFilterModel::invalidate);
    connect(this, &SeamPropertyFilterModel::scanlabScannerChanged, this, &SeamPropertyFilterModel::invalidate);
}

SeamPropertyFilterModel::~SeamPropertyFilterModel() = default;

bool SeamPropertyFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    const auto sourceIndex = sourceModel()->index(source_row, 0, source_parent);
    if (!sourceIndex.isValid())
    {
        return false;
    }
    const auto property = sourceIndex.data(Qt::UserRole + 1).value<SeamPropertyModel::Property>();

    if (!GuiConfiguration::instance()->configureMovingDirectionOnSeam())
    {
        if (property == SeamPropertyModel::Property::MovingDirection)
        {
            return false;
        }
    }

    if (!GuiConfiguration::instance()->configureThicknessOnSeam())
    {
        if (property == SeamPropertyModel::Property::ThicknessLeft
            || property == SeamPropertyModel::Property::ThicknessRight
            || property == SeamPropertyModel::Property::TargetDistance)
        {
            return false;
        }
    }

    if (!m_scanlabScannerAvailable)
    {
        if (property == SeamPropertyModel::Property::SeamRoi)
        {
            return false;
        }
    }

    if (!GuiConfiguration::instance()->seamIntervalsOnProductStructure())
    {
        if (property == SeamPropertyModel::Property::Intervals
            || property == SeamPropertyModel::Property::IntervalErrors)
        {
            return false;
        }
    } else {
        if (property == SeamPropertyModel::Property::Length)
        {
            return false;
        }
    }

    return true;
}

void SeamPropertyFilterModel::setLaserControl(bool value)
{
    if (m_laserControlAvailable == value)
    {
        return;
    }
    m_laserControlAvailable = value;
    emit laserControlChanged();
}

void SeamPropertyFilterModel::setScanlabScanner(bool value)
{
    if (m_scanlabScannerAvailable == value)
    {
        return;
    }
    m_scanlabScannerAvailable = value;
    emit scanlabScannerChanged();
}

void SeamPropertyFilterModel::selectAll()
{
    for (auto i = 0; i < rowCount(); i++)
    {

        const auto sourceIndex = mapToSource(this->index(i, 0));
        if (!sourceIndex.isValid())
        {
            continue;
        }
        sourceModel()->setData(sourceIndex, true, Qt::UserRole);
    }
}

void SeamPropertyFilterModel::selectNone()
{
    for (auto i = 0; i < rowCount(); i++)
    {

        const auto sourceIndex = mapToSource(this->index(i, 0));
        if (!sourceIndex.isValid())
        {
            continue;
        }
        sourceModel()->setData(sourceIndex, false, Qt::UserRole);
    }
}

}
}
