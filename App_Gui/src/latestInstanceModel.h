#pragma once

#include "abstractMultiSeamDataModel.h"

#include "event/results.interface.h"
#include "image/ipSignal.h"

#include <QMutex>

class QTimer;

namespace precitec
{
namespace storage
{

class ExtendedProductInfoHelper;
class ResultsServer;

}
namespace gui
{

class LatestInstanceModel : public AbstractMultiSeamDataModel
{
    Q_OBJECT

    Q_PROPERTY(precitec::storage::ResultsServer* resultsServer READ resultsServer WRITE setResultsServer NOTIFY resultsServerChanged)

    Q_PROPERTY(QString seamLabel READ seamLabel NOTIFY currentIndexChanged)

    Q_PROPERTY(QString seriesLabel READ seriesLabel NOTIFY currentIndexChanged)

    Q_PROPERTY(bool liveUpdate READ isLiveUpdate WRITE setLiveUpdate NOTIFY liveUpdateChanged)

    Q_PROPERTY(bool defaultProduct READ isDefaultProduct NOTIFY currentIndexChanged)

    Q_PROPERTY(precitec::storage::ExtendedProductInfoHelper *extendedProductInfoHelper READ extendedProductInfoHelper CONSTANT)

public:
    explicit LatestInstanceModel(QObject* parent = nullptr);
    ~LatestInstanceModel();

    precitec::storage::ResultsServer* resultsServer() const
    {
        return m_resultsServer;
    }
    void setResultsServer(precitec::storage::ResultsServer* server);

    bool isLiveUpdate() const
    {
        return m_liveUpdate;
    }
    void setLiveUpdate(bool set);

    bool isDefaultProduct() const;

    QString seamLabel() const;
    QString seriesLabel() const;

    storage::ExtendedProductInfoHelper *extendedProductInfoHelper() const
    {
        return m_extendedProductInfoHelper;
    }

    Q_INVOKABLE void startProductInspection(QPointer<precitec::storage::Product> product, const QUuid& productInstance, const QString &extendedProductInfo);

    Q_INVOKABLE void clear() override;

public Q_SLOTS:
    void addSample(int sensor, const precitec::image::Sample& sample, const precitec::interface::ImageContext& context);

Q_SIGNALS:
    void resultsServerChanged();
    void liveUpdateChanged();

private:
    using ResultQueue = std::list<std::pair<QPointer<precitec::storage::Seam>, precitec::interface::ResultArgs>>;
    using SampleQueue = std::list<std::tuple<QPointer<precitec::storage::Seam>, int, precitec::image::Sample, precitec::interface::ImageContext>>;
    void startSeamInspection(QPointer<precitec::storage::Seam> seam, const QUuid& productInstance, quint32 serialNumber);
    void endSeamInspection();
    void result(const precitec::interface::ResultArgs& result);
    void combinedResults(const std::vector<precitec::interface::ResultDoubleArray>& results);
    void bulkUpdate();
    void bulkUpdateResults(ResultQueue &&queue);
    void bulkUpdateSamples(SampleQueue &&queue);
    void discardResultsDuringQueuing();
    void discardSamplesDuringQueuing();

    QString m_serialNumber;
    bool m_liveUpdate = false;
    QPointer<precitec::storage::Seam> m_currentSeam = nullptr;

    precitec::storage::ResultsServer* m_resultsServer = nullptr;
    QMetaObject::Connection m_resultsServerDestroyedConnection;

    ResultQueue m_resultQueue;
    SampleQueue m_sampleQueue;
    QMutex m_queueMutex;
    QMutex m_currentSeamMutex;
    QTimer* m_queueTimer;
    QString m_extendedProductInfo;
    storage::ExtendedProductInfoHelper *m_extendedProductInfoHelper;
};

}
}
