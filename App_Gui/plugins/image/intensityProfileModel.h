#pragma once

#include <QAbstractItemModel>
#include "image/image.h"
#include "imageItem.h"


namespace precitec
{
namespace gui
{
namespace components
{

namespace plotter
{
class DataSet;
}

namespace image
{
class IntensityProfileModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(precitec::gui::components::image::ImageData imageData READ imageData WRITE setImageData NOTIFY imageDataChanged)
    Q_PROPERTY(QVector2D startPoint READ startPoint WRITE setStartPoint NOTIFY startPointChanged)
    Q_PROPERTY(QVector2D endPoint READ endPoint WRITE setEndPoint NOTIFY endPointChanged)
    Q_PROPERTY(precitec::gui::components::plotter::DataSet *dataSet READ dataSet CONSTANT)

public:
    explicit IntensityProfileModel (QObject *parent = nullptr);
    ~IntensityProfileModel() override;
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    precitec::gui::components::image::ImageData  imageData();
    void setImageData(const precitec::gui::components::image::ImageData & rImageData);
    QVector2D startPoint();
    void setStartPoint(const QVector2D& point);
    QVector2D endPoint();
    void setEndPoint(const QVector2D& point);

    precitec::gui::components::plotter::DataSet *dataSet() const
    {
        return m_dataSet;
    }

    Q_SIGNALS:

        void imageDataChanged();
    void startPointChanged();
    void endPointChanged();

private:

    struct SampledPoint {
        double x;
        double y;
        double intensity;
    };
    struct ExtremePoints {
        int iStart;
        int jStart;
        int iEnd;
        int jEnd;
    };
    enum class SamplingDirection {Horizontal, Vertical};

    precitec::gui::components::image::ImageData  m_ImageData;
    QVector2D m_startPoint;
    QVector2D m_endPoint;

    std::vector<SampledPoint> m_Profile;
    precitec::gui::components::plotter::DataSet *m_dataSet;

    ExtremePoints computeExtremePoints();
    void computeProfileWithoutInterpolation();
    void computeProfileWithMainDirectionInterpolation();
    void update();
};



}
}
}
}
