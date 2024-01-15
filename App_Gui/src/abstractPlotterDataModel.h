#pragma once

#include <QAbstractListModel>
#include <QPointer>
#include <QColor>

#include <optional>

#include "resultData.h"

namespace precitec
{
namespace interface
{

class ResultArgs;

}
namespace storage
{

class Seam;
class Product;
class ResultSetting;
class ResultSettingModel;
class ErrorSettingModel;
class SensorSettingsModel;

}
namespace gui
{
namespace components
{
namespace plotter
{

class MulticolorSet;
class DataSet;
class ColorMap;
class RangedSet;

}
}

class AbstractPlotterDataModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(precitec::storage::ResultSettingModel* resultsConfigModel READ resultsConfigModel WRITE setResultsConfigModel NOTIFY resultsConfigModelChanged)

    Q_PROPERTY(precitec::storage::ErrorSettingModel* errorConfigModel READ errorConfigModel WRITE setErrorConfigModel NOTIFY errorConfigModelChanged)

    Q_PROPERTY(precitec::storage::SensorSettingsModel* sensorConfigModel READ sensorConfigModel WRITE setSensorConfigModel NOTIFY sensorConfigModelChanged)

    Q_PROPERTY(precitec::storage::Product* currentProduct READ currentProduct WRITE setCurrentProduct NOTIFY currentProductChanged)

    Q_PROPERTY(int numberOfSeamsInPlotter READ numberOfSeamsInPlotter WRITE setNumberOfSeamsInPlotter NOTIFY numberOfSeamsInPlotterChanged)

public:
    ~AbstractPlotterDataModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;
    bool setData(const QModelIndex & index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    precitec::storage::ResultSettingModel* resultsConfigModel() const
    {
        return m_resultsConfigModel;
    }
    void setResultsConfigModel(precitec::storage::ResultSettingModel* model);

    precitec::storage::ErrorSettingModel* errorConfigModel() const
    {
        return m_errorConfigModel;
    }
    void setErrorConfigModel(precitec::storage::ErrorSettingModel* model);

    precitec::storage::SensorSettingsModel* sensorConfigModel() const
    {
        return m_sensorConfigModel;
    }
    void setSensorConfigModel(precitec::storage::SensorSettingsModel* model);

    precitec::storage::Product* currentProduct() const
    {
        return m_currentProduct;
    }
    void setCurrentProduct(precitec::storage::Product* product);

    int currentIndex() const
    {
        return m_currentIndex;
    }

    int numberOfSeamsInPlotter() const
    {
        return m_numberOfSeamsInPlotter;
    }
    void setNumberOfSeamsInPlotter(int numberOfSeamsInPlotter);

    virtual int maxIndex() const = 0;

    virtual QPointer<precitec::storage::Seam> findSeam(uint index) const = 0;

Q_SIGNALS:
    void resultsConfigModelChanged();
    void errorConfigModelChanged();
    void sensorConfigModelChanged();
    void currentProductChanged();
    void currentIndexChanged();
    void maxIndexChanged();
    void numberOfSeamsInPlotterChanged();

protected:
    explicit AbstractPlotterDataModel(QObject* parent = nullptr);

    const std::vector<ResultData>& results() const
    {
        return m_results;
    }

    const ResultData& resultAt(uint index) const
    {
        return m_results.at(index);
    }
    int resultIndex(int type);

    void addData();
    void popData();
    void setCurrentIndex(int index);
    void setCurrentIndexDirekt(int index);

    /**
     * Disables the signal at @p resultIndex. Only to be called during loading.
     **/
    void internalDisable(uint resultIndex);

    /**
     * Should only be used when enclosed by begin/endResetModel
     **/
    virtual void clear();

    void insertNewResult(const precitec::interface::ResultArgs& result, precitec::storage::ResultSetting* resultConfig, int seamIndex, const std::list<std::pair<QVector2D, float>>& signalSamples, const std::list<QVector2D>& upperReferenceSamples, const std::list<QVector2D>& lowerReferenceSamples, uint numberOfSeams);
    void insertNewSensor(int sensorId, precitec::storage::ResultSetting* resultConfig, int seamIndex, const std::list<std::pair<QVector2D, float>>& signalSamples, uint numberOfSeams);
    void addResults(int seamIndex, int resultIndex, const precitec::interface::ResultArgs& result, precitec::storage::ResultSetting* resultConfig, const std::list<std::pair<QVector2D, float>>& signalSamples, const std::list<QVector2D>& upperReferenceSamples, const std::list<QVector2D>& lowerReferenceSamples, uint numberOfSeams);
    void addSignalSamples(int resultIndex, int seamIndex, precitec::storage::ResultSetting* resultConfig, int sensorId, const std::list<std::pair<QVector2D, float>>& signalSamples);
    bool collectResults(std::list<std::pair<QVector2D, float>>& signalSamples, std::list<QVector2D>& upperReferenceSamples, std::list<QVector2D>& lowerReferenceSamples, const precitec::interface::ResultArgs& result, bool skipNullValues);

    void setMaxElements(const std::optional<quint32> &elements)
    {
        m_maxElements = elements;
    }

private:
    void updateSettings();

    void addSignalSamples(int resultIndex, int seamIndex, precitec::storage::ResultSetting* resultConfig, const precitec::interface::ResultArgs& result, const std::list<std::pair<QVector2D, float>>& signalSamples);
    void addTopBoundarySamples(int resultIndex, int seamIndex, precitec::storage::ResultSetting* resultConfig, const precitec::interface::ResultArgs& result, const std::list<QVector2D>& upperReferenceSamples);
    void addBottomBoundarySamples(int resultIndex, int seamIndex, precitec::storage::ResultSetting* resultConfig, const precitec::interface::ResultArgs& result, const std::list<QVector2D>& lowerReferenceSamples);

    components::plotter::MulticolorSet* createMulticolorSet(precitec::storage::ResultSetting* resultConfig, const precitec::interface::ResultArgs& result);
    components::plotter::MulticolorSet* createMulticolorSet(precitec::storage::ResultSetting* resultConfig, int sensorId);
    components::plotter::DataSet* createBoundarySet(precitec::storage::ResultSetting* resultConfig, const precitec::interface::ResultArgs& result, bool isTop);

    int m_currentIndex = -1;
    int m_numberOfSeamsInPlotter = 1;

    std::vector<ResultData> m_results;

    precitec::storage::ResultSettingModel* m_resultsConfigModel = nullptr;
    QMetaObject::Connection m_resultsConfigModelDestroyedConnection;

    precitec::storage::ErrorSettingModel* m_errorConfigModel = nullptr;
    QMetaObject::Connection m_errorConfigModelDestroyedConnection;

    precitec::storage::SensorSettingsModel* m_sensorConfigModel = nullptr;
    QMetaObject::Connection m_sensorConfigModelDestroyedConnection;

    precitec::storage::Product* m_currentProduct = nullptr;
    QMetaObject::Connection m_currentProductDestroyedConnection;
    precitec::gui::components::plotter::ColorMap* m_nioColors = nullptr;
    precitec::gui::components::plotter::ColorMap* m_signalQualityColors = nullptr;

    std::optional<quint32> m_maxElements;
};

}
}
