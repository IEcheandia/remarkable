#include "infoBoxModel.h"

#include <QRect>

using namespace precitec::image;
using precitec::gui::components::image::ImageData;

namespace precitec
{
namespace gui
{

InfoBoxModel::InfoBoxModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

InfoBoxModel::~InfoBoxModel() = default;

QHash<int, QByteArray> InfoBoxModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("id")},
        {Qt::UserRole, QByteArrayLiteral("type")},
        {Qt::UserRole + 1, QByteArrayLiteral("bounds")},
        {Qt::UserRole + 2, QByteArrayLiteral("texts")}
    };
}

QVariant InfoBoxModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    if (index.row() < 0 || index.row() >= int(m_info.size()))
    {
        return {};
    }
    if (role == Qt::DisplayRole)
    {
        return m_info.at(index.row()).getId() + 1;
    }
    if (role == Qt::UserRole)
    {
        return contentTypeToString(m_info.at(index.row()).getContentType());
    }
    if (role == Qt::UserRole + 1)
    {
        return QVariant::fromValue(toQt(m_info.at(index.row()).getBoundingBox()));
    }
    if (role == Qt::UserRole + 2)
    {
        return QVariant::fromValue(m_info.at(index.row()).getLines());
    }
    return {};
}

int InfoBoxModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_info.size();
}

void InfoBoxModel::setImageData(const ImageData &data)
{
    const auto infoBoxLayer = std::get<OverlayCanvas>(data).getLayer(LayerType::eLayerInfoBox);
    const auto infoShapes = infoBoxLayer.info();

    beginResetModel();
    m_info.clear();
    for (auto shape : infoShapes)
    {
        const auto infoBox = static_cast<OverlayInfoBox*>(shape.get());
        m_info.push_back(*infoBox);
    }
    endResetModel();
}

void InfoBoxModel::clear()
{
    beginResetModel();
    m_info.clear();
    endResetModel();
}

QString InfoBoxModel::contentTypeToString(ContentType type) const
{
    switch (type)
    {
        case ContentType::ePore:
            return QStringLiteral("Blob/Hole");

        case ContentType::ePoreSet1:
            return QStringLiteral("Blob/Hole (1)");

        case ContentType::ePoreSet2:
            return QStringLiteral("Blob/Hole (2)");

        case ContentType::ePoreSet3:
            return QStringLiteral("Blob/Hole (3)");

        case ContentType::ePoorPenetration:
            return QStringLiteral("Penetration Min/Multi");

        case ContentType::eSurface:
            return QStringLiteral("Surface");

        case ContentType::ePoorPenetrationHough:
            return QStringLiteral("Penetration Gradient");
        default:
            return QStringLiteral("Unknown");
    }
}

QRectF InfoBoxModel::toQt(const geo2d::Rect &rect) const
{
    return QRectF{qreal(rect.x().start()), qreal(rect.y().start()), qreal(rect.x().end() - rect.x().start()), qreal(rect.y().end() - rect.y().start())};
}

}
}

