#include "assemblyImageInspectionModel.h"

#include "product.h"
#include "seam.h"
#include "seamSeries.h"
#include "resultsServer.h"

#include <QDebug>

using precitec::storage::ResultsServer;
using precitec::storage::Seam;

namespace precitec
{
namespace gui
{

AssemblyImageInspectionModel::AssemblyImageInspectionModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

AssemblyImageInspectionModel::~AssemblyImageInspectionModel() = default;

QHash<int, QByteArray> AssemblyImageInspectionModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("seamName")},
        {Qt::UserRole, QByteArrayLiteral("position")},
        {Qt::UserRole + 1, QByteArrayLiteral("current")},
        {Qt::UserRole + 2, QByteArrayLiteral("state")},
        {Qt::UserRole + 3, QByteArrayLiteral("seam")},
    };
}

QVariant AssemblyImageInspectionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    const auto &data = m_seams.at(index.row());
    if (!data.seam)
    {
        return {};
    }
    switch (role)
    {
    case Qt::DisplayRole:
        return data.seam->name();
    case Qt::UserRole:
        return data.seam->positionInAssemblyImage();
    case Qt::UserRole + 1:
        return data.seam == m_currentInpsectedSeam;
    case Qt::UserRole + 2:
        return QVariant::fromValue(data.state);
    case Qt::UserRole + 3:
        return QVariant::fromValue(data.seam.data());
    }
    return {};
}

int AssemblyImageInspectionModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_seams.size();
}

void AssemblyImageInspectionModel::setResultsServer(ResultsServer *server)
{
    if (m_resultsServer == server)
    {
        return;
    }
    disconnect(m_resultsServerDestroyedConnection);
    disconnect(m_productInstanceStarted);
    disconnect(m_nio);
    disconnect(m_seamInspectionStarted);
    disconnect(m_seamInspectionEnded);

    m_resultsServer = server;
    if (m_resultsServer)
    {
        m_resultsServerDestroyedConnection = connect(m_resultsServer, &QObject::destroyed, this, std::bind(&AssemblyImageInspectionModel::setResultsServer, this, nullptr));
        m_productInstanceStarted = connect(m_resultsServer, &ResultsServer::productInspectionStarted, this, &AssemblyImageInspectionModel::init);
        m_nio = connect(m_resultsServer, &ResultsServer::nioReceived, this, &AssemblyImageInspectionModel::nio);
        m_seamInspectionStarted = connect(m_resultsServer, &ResultsServer::seamInspectionStarted, this, &AssemblyImageInspectionModel::seamInspectionStarted);
        m_seamInspectionEnded = connect(m_resultsServer, &ResultsServer::seamInspectionEnded, this, &AssemblyImageInspectionModel::seamInspectionEnded);
    } else
    {
        m_resultsServerDestroyedConnection = {};
        m_productInstanceStarted = {};
        m_nio = {};
        m_seamInspectionStarted = {};
        m_seamInspectionEnded = {};
    }
    emit resultsServerChanged();
}

void AssemblyImageInspectionModel::init(QPointer<precitec::storage::Product> product)
{
    if (m_product == product)
    {
        // just set back all seams to uninspected
        for (auto &data: m_seams)
        {
            data.state = SeamState::Uninspected;
        }
        emit dataChanged(index(0, 0), index(rowCount() - 1, 0), {Qt::UserRole + 2});
        return;
    }
    // different product
    beginResetModel();
    m_product = product;
    m_seams.clear();
    for (auto series : m_product->seamSeries())
    {
        const auto &seams = series->seams();
        std::transform(seams.begin(), seams.end(), std::back_inserter(m_seams), [] (auto seam) { return Data{QPointer<Seam>{seam}, SeamState::Uninspected}; });
    }

    endResetModel();
    emit productChanged();
}

void AssemblyImageInspectionModel::seamInspectionStarted(QPointer<Seam> seam)
{
    m_currentInpsectedSeam = seam;
    auto it = std::find_if(m_seams.begin(), m_seams.end(), [this] (const auto &data) { return data.seam == m_currentInpsectedSeam; });
    if (it == m_seams.end())
    {
        return;
    }
    const auto i = index(std::distance(m_seams.begin(), it), 0);
    emit dataChanged(i, i, {Qt::UserRole + 1});
}

void AssemblyImageInspectionModel::seamInspectionEnded()
{
    auto it = std::find_if(m_seams.begin(), m_seams.end(), [this] (const auto &data) { return data.seam == m_currentInpsectedSeam; });
    if (it == m_seams.end())
    {
        return;
    }
    m_currentInpsectedSeam = nullptr;
    if ((*it).state == SeamState::Uninspected)
    {
        (*it).state = SeamState::Success;
    }
    const auto i = index(std::distance(m_seams.begin(), it), 0);
    emit dataChanged(i, i, {Qt::UserRole + 1, Qt::UserRole + 2});
}

void AssemblyImageInspectionModel::nio()
{
    auto it = std::find_if(m_seams.begin(), m_seams.end(), [this] (const auto &data) { return data.seam == m_currentInpsectedSeam; });
    if (it == m_seams.end())
    {
        return;
    }
    (*it).state = SeamState::Failure;
    const auto i = index(std::distance(m_seams.begin(), it), 0);
    emit dataChanged(i, i, {Qt::UserRole + 2});
}

}
}
