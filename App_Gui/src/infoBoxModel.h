#pragma once

#include <QAbstractListModel>
#include <QPointF>

#include "image/image.h"
#include "common/geoContext.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

class QRectF;

namespace precitec
{
namespace gui
{
namespace components
{
namespace image
{

using ImageData = std::tuple<precitec::interface::ImageContext, precitec::image::BImage, precitec::image::OverlayCanvas>;

}
}

using namespace precitec::image;

class InfoBoxModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit InfoBoxModel(QObject *parent = nullptr);
    ~InfoBoxModel() override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    Q_INVOKABLE void setImageData(const precitec::gui::components::image::ImageData &data);

    Q_INVOKABLE void clear();

    std::vector< OverlayText > info(int id);

private:
    QString contentTypeToString(ContentType type) const;
    QRectF toQt(const geo2d::Rect &rect) const;

    std::vector<OverlayInfoBox> m_info;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::InfoBoxModel*)
Q_DECLARE_METATYPE(std::vector< precitec::image::OverlayText >)

