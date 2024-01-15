#pragma once

#include "message/simulationCmd.interface.h"

#include <QAbstractListModel>

namespace precitec
{
namespace gui
{

class SimulationImageModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit SimulationImageModel(QObject *parent = nullptr);
    ~SimulationImageModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Q_INVOKABLE int rowCount(const QModelIndex &parent = {}) const override;

    void init(const QString &basePath, const std::vector<interface::SimulationInitStatus::ImageData> &imageData);

    Q_INVOKABLE bool containsImagesForSeam(quint32 seamSeries, quint32 seam) const;

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE int firstImageOfSeam(uint seamSeries, uint seam);

    Q_INVOKABLE QModelIndex indexOfFrame(uint seamSeries, uint seam, uint frameNumber);

private:
    QString m_imageBasePath;
    std::vector<interface::SimulationInitStatus::ImageData> m_imageData;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::SimulationImageModel*)
