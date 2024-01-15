#pragma once

#include <QAbstractListModel>
#include <QMutex>

#include "event/results.interface.h"

#include <list>
#include <vector>

class QTimer;
class QUuid;
class QColor;
class LatestProductErrorsModelTest;

namespace precitec
{
namespace interface
{

class ResultArgs;

}
namespace storage
{

class ResultsServer;
class Seam;
class ErrorSettingModel;

}
namespace gui
{


/**
 * Model containing the errors of the current product instance.
 * To use one need to set the resultsServer property.
 **/
class LatestProductErrorsModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * The ResultsServer to get the results from.
     **/
    Q_PROPERTY(precitec::storage::ResultsServer *resultsServer READ resultsServer WRITE setResultsServer NOTIFY resultsServerChanged)

    /**
     * The Error Configuration
     **/
    Q_PROPERTY(precitec::storage::ErrorSettingModel *errorConfigModel READ errorConfigModel WRITE setErrorConfigModel NOTIFY errorConfigModelChanged)

    /**
     * Whether the model is updating whenever new values get in
     **/
    Q_PROPERTY(bool liveUpdate READ liveUpdate WRITE setLiveUpdate NOTIFY liveUpdateChanged)

public:
    explicit LatestProductErrorsModel(QObject *parent = nullptr);
    ~LatestProductErrorsModel();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    precitec::storage::ResultsServer *resultsServer() const
    {
        return m_resultsServer;
    }
    void setResultsServer(precitec::storage::ResultsServer *server);

    precitec::storage::ErrorSettingModel *errorConfigModel() const
    {
        return m_errorConfigModel;
    }
    void setErrorConfigModel(precitec::storage::ErrorSettingModel *model);

    bool liveUpdate() const
    {
        return m_liveUpdate;
    }
    void setLiveUpdate(bool set);

    Q_INVOKABLE void clear();

Q_SIGNALS:
    void resultsServerChanged();
    void liveUpdateChanged();
    void errorConfigModelChanged();
    void seamNumberChanged();

private:
    void updateSeamNumber(QPointer<precitec::storage::Seam> seam, const QUuid &productInstance, quint32 serialNumber);
    void addToQueue(const precitec::interface::ResultArgs &result);
    void update();
    void updateSettings();

    int m_currentSeamNumber = -1;
    bool m_liveUpdate = false;
    QTimer *m_queueTimer;
    std::list< std::pair<int, precitec::interface::ResultArgs> > m_queue;
    QMutex m_queueMutex;
    std::vector< std::tuple<int, int, QString, QColor, qreal> > m_errors;
    precitec::storage::ResultsServer *m_resultsServer = nullptr;
    QMetaObject::Connection m_resultsServerDestroyedConnection;
    precitec::storage::ErrorSettingModel *m_errorConfigModel = nullptr;
    QMetaObject::Connection m_errorConfigModelDestroyedConnection;
    QMetaObject::Connection m_errorConfigResetConnection;
    QMetaObject::Connection m_errorConfigChangedConnection;

    friend LatestProductErrorsModelTest;
};

}
}

