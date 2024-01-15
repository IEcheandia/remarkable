#pragma once

#include "event/results.interface.h"

#include <memory>
#include <queue>

#include <QTime>
#include <QMutex>
#include <QPointer>
#include <QUuid>

#include <QAbstractListModel>

class HistoryModelTest;

namespace precitec
{

namespace storage
{
class ExtendedProductInfoHelper;
class Product;
class Seam;
class ResultsServer;
} // namespace storage

namespace interface
{
class ResultArgs;
}

namespace gui
{

/**
 * The Model contains a history of results
 **/
class HistoryModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * The ResultsServer to get the results from
     **/
    Q_PROPERTY(precitec::storage::ResultsServer *resultsServer READ resultsServer WRITE setResultsServer NOTIFY resultsServerChanged)

    /**
     * Max number of elements in the list model
     **/
    Q_PROPERTY(quint32 max READ max WRITE setMax NOTIFY maxChanged)

    Q_PROPERTY(precitec::storage::ExtendedProductInfoHelper *extendedProductInfoHelper READ extendedProductInfoHelper CONSTANT)
public:
    explicit HistoryModel(QObject *parent = nullptr);
    ~HistoryModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE void clear();

    precitec::storage::ResultsServer *resultsServer() const
    {
        return m_resultsServer;
    }

    quint32 max() const
    {
        return m_max;
    }

    void setResultsServer(precitec::storage::ResultsServer *server);
    void setMax(quint32 max);

    storage::ExtendedProductInfoHelper *extendedProductInfoHelper() const
    {
        return m_extendedProductInfoHelper;
    }

Q_SIGNALS:
    void resultsServerChanged();
    void maxChanged();

private:
    void startProductInspection(QPointer<precitec::storage::Product> product, const QUuid &productInstance, const QString &extendedProductInfo);
    void stopProductInspection(QPointer<precitec::storage::Product> product);

    void startSeamInspection(QPointer<precitec::storage::Seam> seam,
                             const QUuid &productInstance,
                             quint32 serialNumber);

    void result(const precitec::interface::ResultArgs &result);
    void combinedResults(const std::vector<precitec::interface::ResultDoubleArray> &results);

    void setCurrentProduct(QPointer<precitec::storage::Product> product);

    void updateHistory();

    bool isIndexValid(const QModelIndex &index) const;

    QPointer<precitec::storage::Product> m_currentProduct;
    QString m_currentExtendedProductInfo;
    quint32 m_currentSerialNumber = 0;
    quint32 m_nextSerialNumber = 0;
    QUuid m_currentProductInstance;
    bool m_currentProductInstanceNio = false;
    QTime m_currentProductInstanceTime;

    quint32 m_max = 100;
    precitec::storage::ResultsServer *m_resultsServer = nullptr;
    QMetaObject::Connection m_resultsServerDestroyedConnection;
    struct Cycle
    {
        QString serialNumber;
        QString productName;
        QString partNumber;
        QTime lastResultTime;
        bool isNio = false;
    };
    std::deque<Cycle> m_history;
    storage::ExtendedProductInfoHelper *m_extendedProductInfoHelper;

    friend HistoryModelTest;
};

} // namespace gui
} // namespace precitec
