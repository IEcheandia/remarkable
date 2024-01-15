#pragma once

#include "abstractPlotterDataModel.h"

#include <QPointer>

namespace precitec
{
namespace storage
{

class Seam;

}
namespace gui
{
namespace components
{
namespace plotter
{

class InfiniteSet;

}
}

class AbstractMultiSeamDataModel : public AbstractPlotterDataModel
{
    Q_OBJECT

    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentIndexChanged)

    Q_PROPERTY(int maxIndex READ maxIndex NOTIFY maxIndexChanged)

    Q_PROPERTY(quint32 maxSeamsLimit READ maxSeamsLimit WRITE setMaxSeamsLimit NOTIFY maxSeamLimitChaged)

public:
    ~AbstractMultiSeamDataModel() override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    int maxIndex() const override
    {
        return m_seams.size() - 1;
    }

    quint32 maxSeamsLimit() const
    {
        return m_maxSeamsLimit;
    }
    void setMaxSeamsLimit(quint32 value);

    QPointer<precitec::storage::Seam> findSeam(uint index) const override;

    Q_INVOKABLE precitec::storage::Seam* currentSeam() const;
    Q_INVOKABLE void previous();
    Q_INVOKABLE void next();
    Q_INVOKABLE void clear() override;

Q_SIGNALS:
    void maxSeamLimitChaged();

protected:
    explicit AbstractMultiSeamDataModel(QObject* parent = nullptr);

    void addSeam(QPointer<precitec::storage::Seam> seam);
    void updateSeamLength(uint idx);
    void updateSeamLength();
    int seamIndex(precitec::storage::Seam* seam);

    void insertNewSensor(int sensorId, precitec::storage::ResultSetting* resultConfig, int seamIndex, const std::list<std::pair<QVector2D, float>>& signalSamples);
    void addResults(int seamIndex, int resultIndex, const precitec::interface::ResultArgs& result, precitec::storage::ResultSetting* resultConfig, const std::list<std::pair<QVector2D, float>>& signalSamples, const std::list<QVector2D>& upperReferenceSamples, const std::list<QVector2D>& lowerReferenceSamples);

private:
    void insertNewResult(const precitec::interface::ResultArgs& result, precitec::storage::ResultSetting* resultConfig, int seamIndex, const std::list<std::pair<QVector2D, float>>& signalSamples, const std::list<QVector2D>& upperReferenceSamples, const std::list<QVector2D>& lowerReferenceSamples);

    double seamLength(uint index) const;

    quint32 m_maxSeamsLimit = 100;

    std::deque<std::pair<QPointer<precitec::storage::Seam>, double>> m_seams;
    std::deque<precitec::gui::components::plotter::InfiniteSet*> m_separators;
};

}
}
Q_DECLARE_METATYPE(precitec::gui::AbstractMultiSeamDataModel*)
