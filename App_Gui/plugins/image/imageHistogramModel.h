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
class ImageHistogramModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(precitec::gui::components::image::ImageData imageData READ imageData WRITE setImageData NOTIFY imageDataChanged)
    Q_PROPERTY(QRect roi READ roi WRITE setROI NOTIFY roiChanged)  
    Q_PROPERTY(precitec::gui::components::plotter::DataSet *dataSet READ dataSet CONSTANT)

public:
    explicit ImageHistogramModel(QObject *parent = nullptr);
    ~ImageHistogramModel() override;
    
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;    
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    
    precitec::gui::components::image::ImageData  imageData();
    void setImageData(const precitec::gui::components::image::ImageData & rImageData);
    
    QRect roi();
    void setROI(QRect p_ROI);

    precitec::gui::components::plotter::DataSet *dataSet() const
    {
        return m_dataSet;
    }
    
    Q_SIGNALS:
        
    void imageDataChanged();
    void roiChanged();

private:
    static const unsigned int levels = 256; //8 bit gray image
    
    precitec::gui::components::image::ImageData  m_ImageData;
    QRect m_ROI;
    std::vector<unsigned int> m_Histogram;
    double m_Peak; //double to avoid integer division when normalizing

    precitec::gui::components::plotter::DataSet *m_dataSet;
    
    void update();
};



}
}
}
}
