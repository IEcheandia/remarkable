#pragma once

#include <QSortFilterProxyModel>

namespace precitec
{

namespace storage
{
class Seam;
}

namespace gui
{

class SimulationController;

class SimulationImageFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(precitec::gui::SimulationController *simulationController READ simulationController WRITE setSimulationController NOTIFY simulationControllerChanged)
    Q_PROPERTY(quint32 seamSeries READ seamSeries NOTIFY seamChanged)
    Q_PROPERTY(quint32 seam READ seam NOTIFY seamChanged)
    Q_PROPERTY(int currentFrameIndex READ currentFrameIndex WRITE setCurrentFrameIndex NOTIFY currentFrameIndexChanged)
public:
    explicit SimulationImageFilterModel(QObject *parent = nullptr);
    ~SimulationImageFilterModel() override;

    SimulationController *simulationController() const
    {
        return m_controller;
    }
    void setSimulationController(SimulationController *controller);

    quint32 seam() const
    {
        return m_seam;
    }

    quint32 seamSeries() const
    {
        return m_seamSeries;
    }

    int currentFrameIndex() const
    {
        return m_currentFrameIndex;
    }
    void setCurrentFrameIndex(int index);

Q_SIGNALS:
    void simulationControllerChanged();
    void seamChanged();
    void currentFrameIndexChanged();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const override;

private:
    void updateImage();
    void updateFilter();
    SimulationController *m_controller = nullptr;
    quint32 m_seam = 0;
    quint32 m_seamSeries = 0;
    int m_currentFrameIndex = -1;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::SimulationImageFilterModel*)
