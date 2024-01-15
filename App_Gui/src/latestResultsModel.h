#pragma once

#include "abstractSingleSeamDataModel.h"

#include "event/results.interface.h"
#include "image/ipSignal.h"

#include <QMutex>

class QTimer;

namespace precitec
{
namespace storage
{

class ResultsServer;

}
namespace gui
{

class LatestResultsModel : public AbstractSingleSeamDataModel
{
    Q_OBJECT

    Q_PROPERTY(precitec::storage::ResultsServer *resultsServer READ resultsServer WRITE setResultsServer NOTIFY resultsServerChanged)

    Q_PROPERTY(bool liveUpdate READ isLiveUpdate WRITE setLiveUpdate NOTIFY liveUpdateChanged)

    Q_PROPERTY(QString seamLabel READ seamLabel NOTIFY lastPositionChanged)

    Q_PROPERTY(QString seriesLabel READ seriesLabel CONSTANT)

public:
    explicit LatestResultsModel(QObject *parent = nullptr);
    ~LatestResultsModel();

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

    QString seamLabel() const;
    QString seriesLabel() const
    {
        return QStringLiteral("");
    }

    Q_INVOKABLE void clear() override;

public Q_SLOTS:
    void addSample(int sensor, const precitec::image::Sample &sample, const precitec::interface::ImageContext &context);

Q_SIGNALS:
    void resultsServerChanged();
    void liveUpdateChanged();

private:
    void startSeamInspection(QPointer<precitec::storage::Seam> seam, const QUuid& productInstance, quint32 serialNumber);
    void endSeamInspection();
    void result(const precitec::interface::ResultArgs &result);
    void combinedResults(const std::vector<precitec::interface::ResultDoubleArray> &results);
    void bulkUpdate();

    bool m_liveUpdate = false;

    precitec::storage::ResultsServer *m_resultsServer = nullptr;
    QMetaObject::Connection m_resultsServerDestroyedConnection;

    std::list<precitec::interface::ResultArgs> m_queue;
    QMutex m_queueMutex;
    QTimer *m_queueTimer;
};

}
}
