#pragma once

#include <QAbstractListModel>

#include "overlay/layerType.h"

#include <vector>

namespace precitec
{
namespace gui
{
namespace components
{
namespace image
{

/**
 * @brief Model describing the overlay item groups.
 *
 * This class encapsulates the groups of overlay layers as used by the UI.
 * The model provides the following roles:
 * @li display (@link{Qt::DisplayRole}) (The name of the overlay group)
 * @li available (@link{Qt::UserRole}) (Whether the overlay group is available for the OverlayCanvas).
 * @li enabled (@link{Qt::UserRole} + @c 1) (Whether the overlay group is enabled)
 **/
class OverlayGroupModel : public QAbstractListModel
{
    Q_OBJECT
public:
    /**
     * Enum describing the groups of Overlay Layers.
     **/
    enum class OverlayGroup
    {
        Line,
        Contour,
        Position,
        Text,
        Grid,
        Image,
        LiveImage
    };
    /**
     * Maps the OverlayGroup to all @link{precitec::image::LayerType}s it belongs to.
     **/
    static std::vector<precitec::image::LayerType> groupToLayers(OverlayGroup group);
    explicit OverlayGroupModel(QObject *parent = nullptr);
    ~OverlayGroupModel() override;

    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex & index, const QVariant &value, int role) override;
    int rowCount(const QModelIndex &parent) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex index(OverlayGroup group) const;
    QHash<int, QByteArray> roleNames() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /**
     * Sets the availability of the OverlayGroup identified by @p index to @p available.
     **/
    void setAvailable(const QModelIndex &index, bool available);

    /**
     * @returns The @link{precitec::image::LayerType}s for the OverlayGroup identified by @p index.
     **/
    std::vector<precitec::image::LayerType> layers(const QModelIndex &index) const;

    /**
     * @p enable all overlays except of LiveImage.
     **/
    Q_INVOKABLE void enableAllOverlays(bool enable);

    /**
     * @p enable all overlays except of LiveImage, Line and Text.
     **/
    Q_INVOKABLE void prepareInfoboxOverlays();
private:
    struct Data
    {
        QString name;
        bool enabled;
        bool available;
    };
    std::map<OverlayGroup, Data> m_data;
};

}
}
}
}
