#include "imageHistogramModel.h"
#include <algorithm> //max_element

#include <precitec/dataSet.h>

namespace precitec
{
namespace gui
{
namespace components
{
namespace image
{

ImageHistogramModel::ImageHistogramModel(QObject *parent)
    : QAbstractListModel(parent),
    m_Histogram(levels)
    , m_dataSet(new plotter::DataSet{this})
    {
        m_dataSet->setDrawingMode(plotter::DataSet::DrawingMode::LineWithPoints);
    }

ImageHistogramModel::~ImageHistogramModel() = default;

QHash<int, QByteArray> ImageHistogramModel::roleNames() const
{
    return {
        {Qt::UserRole, QByteArrayLiteral("bin")},
        {Qt::UserRole + 1, QByteArrayLiteral("count")},
        {Qt::UserRole + 2, QByteArrayLiteral("countScaled")}
    };
}

QVariant ImageHistogramModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() > int(m_Histogram.size()))
    {
        return QVariant{};
    }
    switch (role)
    {
        case (Qt::UserRole):
            return index.row();
        case (Qt::UserRole + 1):
            return m_Histogram[index.row()];
        case (Qt::UserRole + 2):
            return m_Histogram[index.row()] / m_Peak; 
        default:
            return QVariant{};
    }
}

int ImageHistogramModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_Histogram.size();
}

precitec::gui::components::image::ImageData  ImageHistogramModel::imageData()
{
    return m_ImageData;
}

void ImageHistogramModel::setImageData(const precitec::gui::components::image::ImageData & rImageData)
{
    m_ImageData = rImageData;

    emit imageDataChanged();
    update();
}

QRect ImageHistogramModel::roi()
{
    return m_ROI;
}

void ImageHistogramModel::setROI(QRect p_ROI)
{
    if (m_ROI != p_ROI)
    {            
        m_ROI = p_ROI;
        emit roiChanged();
        update();
    }
}

void ImageHistogramModel::update()
{
    beginResetModel();
    m_Histogram.resize(levels, 0);
    std::fill(m_Histogram.begin(), m_Histogram.end(), 0);
    m_Peak = 0;
    m_dataSet->clear();
    const precitec::image::BImage & rImage = std::get<precitec::image::BImage>(m_ImageData);
    if (!rImage.isValid())
    {
        endResetModel();
        return;
    }
    //clip roi
    
    const unsigned int xMin = std::min<unsigned int>(m_ROI.x(), rImage.width() -1);
    const unsigned int yMin = std::min<unsigned int>(m_ROI.y(), rImage.height() -1);
    const unsigned int xMax = std::min<unsigned int>(xMin + m_ROI.width(), rImage.width());
    const unsigned int yMax = std::min<unsigned int>(yMin + m_ROI.height(), rImage.height());
    assert(xMin >= 0);
    assert(yMin >= 0);

    for (unsigned int y = yMin; y < yMax; ++y) 
    {
        const auto lastPixel = rImage.rowBegin(y) + xMax;
        for (auto  curPixel = rImage.rowBegin(y) + xMin; curPixel < lastPixel; ++curPixel)
        {
            //if (levels!=256) we need ++m_Histogram[(*curPixel)*levels >>8] 
            ++m_Histogram[(*curPixel)];
        }
    }
    std::vector<QVector2D> samples;
    samples.reserve(levels);
    auto counter = 0;
    std::transform(m_Histogram.begin(), m_Histogram.end(), std::back_inserter(samples), [&counter] (const auto& value) { return QVector2D{float(counter++), float(value)}; });
    m_dataSet->addSamples(samples);
    m_Peak = *(std::max_element( m_Histogram.cbegin(), m_Histogram.cend()));
    endResetModel();
}


}
}
}
}
