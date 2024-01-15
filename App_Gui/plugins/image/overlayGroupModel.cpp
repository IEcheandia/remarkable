#include "overlayGroupModel.h"

namespace precitec
{
namespace gui
{
namespace components
{
namespace image
{

std::vector<precitec::image::LayerType> OverlayGroupModel::groupToLayers(OverlayGroup group)
{
    static const std::map<OverlayGroup, std::vector<precitec::image::LayerType>> map{
        {OverlayGroup::Line, {precitec::image::eLayerLine, precitec::image::eLayerLineTransp}},
        {OverlayGroup::Contour, {precitec::image::eLayerContour, precitec::image::eLayerContourTransp}},
        {OverlayGroup::Position, {precitec::image::eLayerPosition, precitec::image::eLayerPositionTransp}},
        {OverlayGroup::Text, {precitec::image::eLayerText, precitec::image::eLayerTextTransp}},
        {OverlayGroup::Grid, {precitec::image::eLayerGridTransp}},
        {OverlayGroup::Image, {precitec::image::eLayerImage}}
    };
    // LiveImage is not mapped as there is no layer for it
    const auto it = map.find(group);
    if (it == map.end())
    {
        return std::vector<precitec::image::LayerType>();
    }
    return it->second;
}

OverlayGroupModel::OverlayGroupModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_data({
        {OverlayGroup::Line, {tr("Line", "Precitec.Controls.Layer00"), true, false}},
        {OverlayGroup::Contour, {tr("Contour", "Precitec.Controls.Layer01"), true, false}},
        {OverlayGroup::Position, {tr("Position", "Precitec.Controls.Layer02"), true, false}},
        {OverlayGroup::Text, {tr("Text", "Precitec.Controls.Layer03"), true, false}},
        {OverlayGroup::Grid, {tr("Grid", "Precitec.Controls.Layer04"), true, false}},
        {OverlayGroup::Image, {tr("Image", "Precitec.Controls.Layer06"), true, false}},
        {OverlayGroup::LiveImage, {tr("Live image", "Precitec.Controls.Layer015"), true, false}}
    })
{
}

OverlayGroupModel::~OverlayGroupModel() = default;

QHash<int, QByteArray> OverlayGroupModel::roleNames() const
{
    return QHash<int, QByteArray>{
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {Qt::UserRole, QByteArrayLiteral("available")},
        {Qt::UserRole + 1, QByteArrayLiteral("enabled")}
    };
}

QVariant OverlayGroupModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }
    const auto it = m_data.find(OverlayGroup(index.internalId()));
    if (it == m_data.end())
    {
        return QVariant();
    }
    switch (role)
    {
    case Qt::DisplayRole:
        return it->second.name;
    case Qt::UserRole:
        return it->second.available;
    case Qt::UserRole + 1:
        return it->second.enabled;
    default:
        return QVariant();
    }
}

bool OverlayGroupModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || (role != Qt::UserRole && role != Qt::UserRole + 1))
    {
        return false;
    }

    const auto it = m_data.find(OverlayGroup(index.internalId()));
    if (it == m_data.end())
    {
        return false;
    }
    const bool newValue = value.toBool();
    bool *data = nullptr;
    switch (role)
    {
    case Qt::UserRole:
        data = &it->second.available;
        break;
    case Qt::UserRole +1:
        data = &it->second.enabled;
        break;
    default:
        return false;
    }
    if (*data != newValue)
    {
        *data = newValue;
        emit dataChanged(index, index, {role});
        return true;
    } else
    {
        return false;
    }
}

QModelIndex OverlayGroupModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid() || column != 0 || std::size_t(row) > m_data.size())
    {
        return QModelIndex();
    }
    return createIndex(row, column, quintptr(OverlayGroup(row)));
}

QModelIndex OverlayGroupModel::index(OverlayGroup group) const
{
    return index(int(group), 0, QModelIndex());
}

int OverlayGroupModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_data.size();
}

void OverlayGroupModel::setAvailable(const QModelIndex &index, bool available)
{
    setData(index, available, Qt::UserRole);
}

std::vector<precitec::image::LayerType> OverlayGroupModel::layers(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return std::vector<precitec::image::LayerType>{};
    }
    return groupToLayers(OverlayGroup(index.internalId()));
}

Qt::ItemFlags OverlayGroupModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

void OverlayGroupModel::enableAllOverlays(bool enable)
{
    const auto liveImageIndex = index(OverlayGroup::LiveImage);
    for (int i = 0; i < rowCount({}); i++)
    {
        auto current = index(i, 0, {});
        if (current == liveImageIndex)
        {
            continue;
        }
        setData(current, enable, Qt::UserRole + 1);
    }
}

void OverlayGroupModel::prepareInfoboxOverlays()
{
    const auto positionIndex = index(OverlayGroup::Position);
    const auto gridIndex = index(OverlayGroup::Grid);
    const auto imageIndex = index(OverlayGroup::Image);
    for (int i = 0; i < rowCount({}); i++)
    {
        auto current = index(i, 0, {});
        if (current == positionIndex || current == gridIndex || current == imageIndex)
        {
            setData(current, false, Qt::UserRole + 1);
        }
    }
}

}
}
}
}
