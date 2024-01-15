#include "resultsStatisticsSeamModel.h"
#include "resultsStatisticsController.h"
#include "product.h"
#include "seamSeries.h"
#include "seam.h"
#include "linkedSeam.h"

namespace precitec
{
namespace storage
{

ResultsStatisticsSeamModel::ResultsStatisticsSeamModel(QObject* parent)
: QAbstractListModel(parent)
{
    connect(this, &ResultsStatisticsSeamModel::linkedSeamIdChanged, this, &ResultsStatisticsSeamModel::updateModel);
    connect(this, &ResultsStatisticsSeamModel::seamIdChanged, this, &ResultsStatisticsSeamModel::updateModel);
    connect(this, &ResultsStatisticsSeamModel::seamSeriesIdChanged, this, &ResultsStatisticsSeamModel::updateModel);
    connect(this, &ResultsStatisticsSeamModel::resultsStatisticsControllerChanged, this, &ResultsStatisticsSeamModel::updateModel);
    connect(this, &ResultsStatisticsSeamModel::modelReset, this, &ResultsStatisticsSeamModel::statisticsChanged);
}

ResultsStatisticsSeamModel::~ResultsStatisticsSeamModel() = default;

QVariant ResultsStatisticsSeamModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()|| index.row() >= int(m_seamStatistics.linkedSeams().size()))
    {
        return {};
    }

    const auto& linkedSeamStats = m_seamStatistics.linkedSeams().at(index.row());

    switch(role)
    {
        case Qt::DisplayRole:
            return linkedSeamStats.name();
        case Qt::UserRole:
            return linkedSeamStats.uuid();
        case Qt::UserRole + 1:
            return linkedSeamStats.ioInPercent();
        case Qt::UserRole + 2:
            return linkedSeamStats.nioInPercent();
        case Qt::UserRole + 3:
            return linkedSeamStats.ioCount();
        case Qt::UserRole + 4:
            return linkedSeamStats.nioCount();
        case Qt::UserRole + 5:
            return linkedSeamStats.visualNumber();
    }
    return {};
}

int ResultsStatisticsSeamModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_seamStatistics.linkedSeams().size();
}

QHash<int, QByteArray> ResultsStatisticsSeamModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("name")},
        {Qt::UserRole, QByteArrayLiteral("id")},
        {Qt::UserRole + 1, QByteArrayLiteral("ioInPercent")},
        {Qt::UserRole + 2, QByteArrayLiteral("nioInPercent")},
        {Qt::UserRole + 3, QByteArrayLiteral("io")},
        {Qt::UserRole + 4, QByteArrayLiteral("nio")},
        {Qt::UserRole + 5, QByteArrayLiteral("visualNumber")}
    };
}

void ResultsStatisticsSeamModel::setResultsStatisticsController(precitec::storage::ResultsStatisticsController* controller)
{
    if (m_resultsStatisticsController == controller)
    {
        return;
    }

    if (m_resultsStatisticsController)
    {
        disconnect(m_resultsStatisticsControllerDestroyed);
        disconnect(m_resultsStatisticsController, &ResultsStatisticsController::update, this, &ResultsStatisticsSeamModel::updateModel);
    }

    m_resultsStatisticsController = controller;

    if (m_resultsStatisticsController)
    {
        m_resultsStatisticsControllerDestroyed = connect(m_resultsStatisticsController, &ResultsStatisticsController::destroyed, this, std::bind(&ResultsStatisticsSeamModel::setResultsStatisticsController, this, nullptr));
        connect(m_resultsStatisticsController, &ResultsStatisticsController::update, this, &ResultsStatisticsSeamModel::updateModel);
    } else
    {
        m_resultsStatisticsControllerDestroyed = {};
    }

    emit resultsStatisticsControllerChanged();
}

void ResultsStatisticsSeamModel::setSeamSeriesId(const QUuid &id)
{
    if (m_seamSeriesId == id)
    {
        return;
    }
    m_seamSeriesId = id;
    emit seamSeriesIdChanged();
}

void ResultsStatisticsSeamModel::setSeamId(const QUuid &id)
{
    if (m_seamId == id)
    {
        return;
    }
    m_seamId = id;
    emit seamIdChanged();
}

void ResultsStatisticsSeamModel::setLinkedSeamId(const QUuid& id)
{
    if (m_linkedSeamId == id)
    {
        return;
    }
    m_linkedSeamId = id;
    emit linkedSeamIdChanged();
}

void ResultsStatisticsSeamModel::updateModel()
{
    beginResetModel();

    m_seamStatistics.clear();

    if (m_resultsStatisticsController && !m_seamSeriesId.isNull() && !m_seamId.isNull())
    {
        const auto& seriesStatistics = m_resultsStatisticsController->productStatistics().seriesStatistics();

        auto series_it = std::find_if(seriesStatistics.begin(), seriesStatistics.end(), [this] (const auto& seriesStats) { return m_seamSeriesId == seriesStats.uuid(); });

        if (series_it != seriesStatistics.end())
        {
            const auto& seamStatistics = series_it->seamStatistics();

            auto seam_it = std::find_if(seamStatistics.begin(), seamStatistics.end(), [this] (const auto& seamStats) { return m_seamId == seamStats.uuid(); });

            if (seam_it != seamStatistics.end())
            {
                if (m_linkedSeamId.isNull())
                {
                    m_seamStatistics = *seam_it;
                } else
                {
                    const auto& linkedSeamStatistics = seam_it->linkedSeams();

                    auto linkedSeam_it = std::find_if(linkedSeamStatistics.begin(), linkedSeamStatistics.end(), [this] (const auto& linkedSeamStats) { return m_linkedSeamId == linkedSeamStats.uuid(); });

                    if (linkedSeam_it != linkedSeamStatistics.end())
                    {
                        m_seamStatistics = *linkedSeam_it;
                    }
                }
            }
        }
    }

    endResetModel();
}

bool ResultsStatisticsSeamModel::seamIncludesLinkedSeams() const
{
    return m_seamStatistics.includesLinkedSeams();
}

unsigned int ResultsStatisticsSeamModel::seamIo() const
{
    return m_seamStatistics.ioCount();
}

unsigned int ResultsStatisticsSeamModel::seamNio() const
{
    return m_seamStatistics.nioCount();
}

double ResultsStatisticsSeamModel::seamIoInPercent() const
{
    return m_seamStatistics.ioInPercent();
}

double ResultsStatisticsSeamModel::seamNioInPercent() const
{
    return m_seamStatistics.nioInPercent();
}

unsigned int ResultsStatisticsSeamModel::seamIoIncludingLinkedSeams() const
{
    return m_seamStatistics.ioCountIncludingLinkedSeams();
}

unsigned int ResultsStatisticsSeamModel::seamNioIncludingLinkedSeams() const
{
    return m_seamStatistics.nioCountIncludingLinkedSeams();
}

double ResultsStatisticsSeamModel::seamIoInPercentIncludingLinkedSeams() const
{
    return m_seamStatistics.ioCountIncludingLinkedSeamsInPercent();
}

double ResultsStatisticsSeamModel::seamNioInPercentIncludingLinkedSeams() const
{
    return m_seamStatistics.nioCountIncludingLinkedSeamsInPercent();
}

}
}

