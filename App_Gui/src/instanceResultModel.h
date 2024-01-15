#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <QColor>
#include <QPointer>

#include <deque>

#include "productInstanceModel.h"

class InstanceResultModelTest;
class ReferenceCurveConstructorTest;
class QTimer;

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

class DataSet;

}
}

/**
 * Model which provides some instance meta data, loaded through a ProductInstanceModel,
 * as well as DataSets for the results of a single product instance and a specific result type and/or trigger type
 **/

class InstanceResultModel : public QAbstractListModel
{
    Q_OBJECT

    /**
     * Product instance model, to load meta data
     **/
    Q_PROPERTY(precitec::storage::ProductInstanceModel* productInstanceModel READ productInstanceModel WRITE setProductInstanceModel NOTIFY productInstanceModelChanged)

    /**
     * Seam, whose results are to be displayed
     **/
    Q_PROPERTY(precitec::storage::Seam* seam READ seam WRITE setSeam NOTIFY seamChanged)

    /**
     * Result type to be displayed
     **/
    Q_PROPERTY(int resultType READ resultType WRITE setResultType NOTIFY resultTypeChanged)

    /**
     * Trigger result type to be displayed
     **/
    Q_PROPERTY(int triggerType READ triggerType WRITE setTriggerType NOTIFY triggerTypeChanged)

    /**
     * Trigger threshold
     **/
    Q_PROPERTY(double threshold READ threshold WRITE setThreshold NOTIFY thresholdChanged)

    /**
     * The index for which the result should be loaded
     **/
    Q_PROPERTY(QModelIndex currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)

    /**
     * Loaded result data as DataSet
     **/
    Q_PROPERTY(precitec::gui::components::plotter::DataSet* result READ result CONSTANT)

    /**
     * Loaded trigger data as DataSet
     * Needed only for lwm result types
     **/
    Q_PROPERTY(precitec::gui::components::plotter::DataSet* trigger READ trigger CONSTANT)

    /**
     * Whether the model is currently loading results
     **/
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)

public:
    explicit InstanceResultModel(QObject* parent = nullptr);
    ~InstanceResultModel() override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    precitec::storage::Seam* seam() const
    {
        return m_seam;
    }
    void setSeam(precitec::storage::Seam* seam);

    precitec::storage::ProductInstanceModel* productInstanceModel() const
    {
        return m_productInstanceModel;
    }
    void setProductInstanceModel(precitec::storage::ProductInstanceModel* model);

    int resultType() const
    {
        return m_resultType;
    }
    void setResultType(int type);

    int triggerType() const
    {
        return m_triggerType;
    }
    void setTriggerType(int type);

    double threshold() const
    {
        return m_threshold;
    }
    void setThreshold(double threshold);

    QModelIndex currentIndex() const
    {
        return m_currentIndex;
    }
    void setCurrentIndex(const QModelIndex& index);

    precitec::gui::components::plotter::DataSet* result() const
    {
        return m_result;
    }

    precitec::gui::components::plotter::DataSet* trigger() const
    {
        return m_trigger;
    }

    bool loading() const
    {
        return m_loadingCounter != 0;
    }

    Q_INVOKABLE void resetCurrentIndex()
    {
        setCurrentIndex({});
    }

    Q_INVOKABLE void selectAll();

    Q_INVOKABLE void selectNone();

Q_SIGNALS:
    void seamChanged();
    void productInstanceModelChanged();
    void resultTypeChanged();
    void triggerTypeChanged();
    void thresholdChanged();
    void currentIndexChanged();
    void loadingChanged();

protected:
    struct InstanceInfo {
        QString serialNumber;
        QUuid uuid;
        QDateTime timestamp;
        precitec::storage::ProductInstanceModel::State state = precitec::storage::ProductInstanceModel::State::Unknown;
        bool linkedSeam = false;
        quint32 seamNumber = -1;
        quint32 visualSeamNumber = -1;
        QString path;
        bool selected = false;
        precitec::gui::components::plotter::DataSet* result = nullptr;
    };

    std::vector<InstanceInfo>& data()
    {
        return m_data;
    }
    std::vector<QVector2D> loadResult(QDir directory, int resultType);
    std::vector<QVector2D> loadResultRange(QDir directory, int resultType, int triggerType, double threshold);
    void loadLastMaxResults();

private:
    void updateModel();
    void updateResult();
    QColor stateColor(precitec::storage::ProductInstanceModel::State state) const;
    void incrementLoading();
    void decrementLoading();
    void loadInstanceData(const QModelIndex& index);
    void clearResults();

    void loadResultOnly(QPointer<precitec::gui::components::plotter::DataSet> result, const QString& path, quint32 seamNumber);
    void loadResultAndTrigger(QPointer<precitec::gui::components::plotter::DataSet> result, precitec::gui::components::plotter::DataSet* trigger, const QString& path, quint32 seamNumber);
    void removeInstanceData(InstanceInfo& info, const QModelIndex& index);

    const std::size_t m_maxLoadedInstanceCount = 20;
    uint m_loadingCounter = 0;
    std::mutex m_loadingMutex;
    int m_resultType = -1;
    int m_triggerType = -1;
    double m_threshold = 0.0;
    QModelIndex m_currentIndex;
    std::vector<InstanceInfo> m_data;
    std::deque<QUuid> m_loadedInstances;

    precitec::storage::Seam* m_seam = nullptr;
    QMetaObject::Connection m_seamDestroyedConnection;

    precitec::storage::ProductInstanceModel* m_productInstanceModel = nullptr;
    QMetaObject::Connection m_productInstanceModelDestroyedConnection;

    precitec::gui::components::plotter::DataSet* m_result;
    precitec::gui::components::plotter::DataSet* m_trigger;
    QTimer* m_updateMetaDataTimer;

    friend InstanceResultModelTest;
    friend ReferenceCurveConstructorTest;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::InstanceResultModel*)


