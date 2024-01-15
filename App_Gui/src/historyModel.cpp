#include "historyModel.h"

#include "resultsServer.h"

#include "seam.h"
#include "product.h"
#include "seamSeries.h"
#include "event/results.h"
#include "extendedProductInfoHelper.h"

#include <iterator>

using precitec::interface::ImageContext;
using precitec::interface::ResultArgs;
using precitec::interface::ResultDoubleArray;
using precitec::storage::Product;
using precitec::storage::ResultsServer;
using precitec::storage::Seam;
namespace precitec
{
namespace gui
{

HistoryModel::HistoryModel(QObject *parent) : QAbstractListModel(parent)
    , m_extendedProductInfoHelper{new storage::ExtendedProductInfoHelper{this}}
{
}

HistoryModel::~HistoryModel() = default;

QHash<int, QByteArray> HistoryModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("serialNumber")},
        {Qt::UserRole, QByteArrayLiteral("productName")},
        {Qt::UserRole + 1, QByteArrayLiteral("lastResultTime")},
        {Qt::UserRole + 2, QByteArrayLiteral("isNio")},
        {Qt::UserRole + 3, QByteArrayLiteral("partNumber")},
    };
}

int HistoryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_history.size();
}

QVariant HistoryModel::data(const QModelIndex &index, int role) const
{
    if (!isIndexValid(index))
    {
        return {};
    }
    const auto &serialInfo = m_history.at(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
        return serialInfo.serialNumber;
    case Qt::UserRole:
        return serialInfo.productName;
    case Qt::UserRole + 1:
        return serialInfo.lastResultTime.toString("hh:mm:ss");
    case Qt::UserRole + 2:
        return serialInfo.isNio;
    case Qt::UserRole + 3:
        return serialInfo.partNumber;
    }
    return {};
}

void HistoryModel::setResultsServer(ResultsServer *server)
{
    if (m_resultsServer == server)
    {
        return;
    }
    disconnect(m_resultsServerDestroyedConnection);
    if (m_resultsServer)
    {
        disconnect(m_resultsServer, &ResultsServer::productInspectionStarted, this,
                   &HistoryModel::startProductInspection);
        disconnect(m_resultsServer, &ResultsServer::seamInspectionStarted, this, &HistoryModel::startSeamInspection);
        disconnect(m_resultsServer, &ResultsServer::productInspectionStopped, this,
                   &HistoryModel::stopProductInspection);
        disconnect(m_resultsServer, &ResultsServer::combinedResultsReceived, this, &HistoryModel::combinedResults);
        disconnect(m_resultsServer, &ResultsServer::nioReceived, this, &HistoryModel::result);
        disconnect(m_resultsServer, &ResultsServer::resultsReceived, this, &HistoryModel::result);
    }

    m_resultsServer = server;
    if (m_resultsServer)
    {
        m_resultsServerDestroyedConnection = connect(m_resultsServer, &QObject::destroyed, this,
                                                     std::bind(&HistoryModel::setResultsServer, this, nullptr));
        connect(m_resultsServer, &ResultsServer::productInspectionStarted, this, &HistoryModel::startProductInspection);
        connect(m_resultsServer, &ResultsServer::productInspectionStopped, this, &HistoryModel::stopProductInspection);
        connect(m_resultsServer, &ResultsServer::seamInspectionStarted, this, &HistoryModel::startSeamInspection);
        connect(m_resultsServer, &ResultsServer::combinedResultsReceived, this, &HistoryModel::combinedResults);
        connect(m_resultsServer, &ResultsServer::nioReceived, this, &HistoryModel::result);
        connect(m_resultsServer, &ResultsServer::resultsReceived, this, &HistoryModel::result);
    }
    else
    {
        m_resultsServerDestroyedConnection = QMetaObject::Connection();
    }
    emit resultsServerChanged();
}

void HistoryModel::setMax(quint32 max)
{
    if (m_max != max)
    {
        m_max = max;
        emit maxChanged();
    }
}

void HistoryModel::startProductInspection(QPointer<precitec::storage::Product> product, const QUuid &productInstance, const QString &extendedProductInfo)
{
    setCurrentProduct(product);
    m_currentProductInstance = productInstance;
    m_currentExtendedProductInfo = extendedProductInfo;
}

void HistoryModel::stopProductInspection(QPointer<precitec::storage::Product> product)
{
    if (m_currentProduct == nullptr)
    {
        return;
    }
    if (!m_currentProductInstanceTime.isNull())
    {
        updateHistory();
    }
    else
    {
        if (product)
        {
            setCurrentProduct(product);
            m_currentProductInstanceNio = true;
            m_currentProductInstanceTime = QTime::currentTime();
            updateHistory();
        }
    }
}

void HistoryModel::startSeamInspection(QPointer<Seam> seam, const QUuid &productInstance, quint32 serialNumber)
{
    if (seam == nullptr || m_currentProduct == nullptr)
    {
        return;
    }
    m_nextSerialNumber = serialNumber;
    if (m_currentProductInstance != productInstance && !m_currentProductInstanceTime.isNull())
    {
        updateHistory();
    };
}
void HistoryModel::updateHistory()
{
    if (const auto maxDelta = m_history.size() - m_max; maxDelta >= 0)
    {
        beginRemoveRows(QModelIndex(), 0, maxDelta);
        while (m_history.size() >= m_max)
        {
            m_history.pop_front();
        }
        endRemoveRows();
    }

    const auto rowIndex = rowCount();
    m_currentSerialNumber = m_nextSerialNumber;
    beginInsertRows(QModelIndex(), rowIndex, rowIndex);

    const auto serialNumber{m_extendedProductInfoHelper->serialNumber(m_currentExtendedProductInfo)};
    const auto partNumber{m_extendedProductInfoHelper->partNumber(m_currentExtendedProductInfo)};

    m_history.emplace_back(
        Cycle{serialNumber.value_or(QString::number(m_currentSerialNumber)), m_currentProduct->name(), partNumber.value_or(QString{}),  m_currentProductInstanceTime, m_currentProductInstanceNio});
    endInsertRows();
    m_currentProductInstanceNio = false;
    m_currentProductInstanceTime = {};
}

void HistoryModel::result(const ResultArgs &result)
{
    m_currentProductInstanceTime = QTime::currentTime();
    if (result.isNio())
    {
        m_currentProductInstanceNio = true;
    }
}

void HistoryModel::combinedResults(const std::vector<ResultDoubleArray> &results)
{
    m_currentProductInstanceTime = QTime::currentTime();
    for (const auto &result : results)
    {
        if (result.value().empty())
        {
            continue;
        }
        if (result.isNio())
        {
            m_currentProductInstanceNio = true;
            break;
        }
    }
}

void HistoryModel::setCurrentProduct(QPointer<Product> product)
{
    if (m_currentProduct == product)
    {
        return;
    }
    m_currentProduct = product;
}

void HistoryModel::clear()
{
    beginResetModel();
    m_history.clear();
    endResetModel();
}

bool HistoryModel::isIndexValid(const QModelIndex &index) const
{
    return ((m_history.size() >= static_cast<std::size_t>(index.row())) && (index.row() >= 0));
}

} // namespace gui
} // namespace precitec
