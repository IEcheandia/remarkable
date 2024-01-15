#include "simulationImageModel.h"

namespace precitec
{
namespace gui
{

SimulationImageModel::SimulationImageModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

SimulationImageModel::~SimulationImageModel() = default;

QHash<int, QByteArray> SimulationImageModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("path")},
        {Qt::UserRole, QByteArrayLiteral("seamSeries")},
        {Qt::UserRole + 1, QByteArrayLiteral("seam")},
        {Qt::UserRole + 2, QByteArrayLiteral("image")}
    };
}

QVariant SimulationImageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    const auto &data = m_imageData.at(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
        return QStringLiteral("%1/seam_series%2/seam%3/%4.bmp").arg(m_imageBasePath)
                                                               .arg(data.seamSeries, 4, 10, QLatin1Char('0'))
                                                               .arg(data.seam, 4, 10, QLatin1Char('0'))
                                                               .arg(data.image, 5, 10, QLatin1Char('0'));
    case Qt::UserRole:
        return data.seamSeries;
    case Qt::UserRole + 1:
        return data.seam;
    case Qt::UserRole + 2:
        return data.image;
    }
    return {};
}

int SimulationImageModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_imageData.size();
}

void SimulationImageModel::init(const QString &basePath, const std::vector<interface::SimulationInitStatus::ImageData> &imageData)
{
    beginResetModel();
    m_imageBasePath = basePath;
    m_imageData = imageData;
    endResetModel();
}

bool SimulationImageModel::containsImagesForSeam(quint32 seamSeries, quint32 seam) const
{
    return std::any_of(m_imageData.begin(), m_imageData.end(), [seamSeries, seam] (const auto &data) { return data.seamSeries == seamSeries && data.seam == seam; });
}

int SimulationImageModel::firstImageOfSeam(uint seamSeries, uint seam)
{
    auto it = std::find_if(m_imageData.begin(), m_imageData.end(), [seamSeries, seam] (auto data) { return data.seamSeries == seamSeries && data.seam == seam; });
    if (it == m_imageData.end())
    {
        return -1;
    }
    return std::distance(m_imageData.begin(), it);
}

QModelIndex SimulationImageModel::indexOfFrame(uint seamSeries, uint seam, uint frameNumber)
{
    auto it = std::find_if(m_imageData.begin(), m_imageData.end(), [seamSeries, seam, frameNumber] (auto data) { return data.seamSeries == seamSeries && data.seam == seam && data.image == frameNumber; });
    if (it == m_imageData.end())
    {
        return {};
    }
    return index(std::distance(m_imageData.begin(), it), 0);
}

}
}
