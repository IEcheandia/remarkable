#include "productStatistics.h"
#include "productMetaData.h"
#include "product.h"
#include "seamSeries.h"
#include "seam.h"

using precitec::interface::ResultType;

namespace precitec
{
namespace storage
{

bool ProductStatistics::empty() const
{
    return instanceCount() == 0;
}

double ProductStatistics::ioInPercent() const
{
    if (empty())
    {
        return 0.0;
    }

    return m_ioCount / double(instanceCount());
}

double ProductStatistics::nioInPercent() const
{
    if (empty())
    {
        return 0.0;
    }

    return m_nioCount / double(instanceCount());
}

void ProductStatistics::importMetaData(const ProductMetaData& metaData, Product* currentProduct)
{
    if (!metaData.isNioValid())
    {
        // nio information not stored correctly
        return;
    }

    if (metaData.nio())
    {
        ++m_nioCount;
    } else
    {
        ++m_ioCount;
    }

    const auto& instanceNios = metaData.nios();
    for (const auto& nio : instanceNios)
    {
        auto it = m_nios.find(ResultType(nio.type));

        if (it == m_nios.end())
        {
            m_nios.emplace(ResultType(nio.type), nio.count);
        } else
        {
            it->second += nio.count;
        }
    }

    if (!metaData.isSeamSeriesValid())
    {
        // processed SeamSeries information not stored correctly
        return;
    }

    const auto& processedSeries = metaData.seamSeries();
    for (const auto& seriesMetaData : processedSeries)
    {
        importSeriesMetaData(seriesMetaData, currentProduct ? currentProduct->findSeamSeries(seriesMetaData.uuid()) : nullptr);
    }

    if (!metaData.isSeamsValid())
    {
        // processed Seams information not stored correctly
        return;
    }

    const auto& processedSeams = metaData.seams();
    for (const auto& seamMetaData : processedSeams)
    {
        // skip linked seams, ensure all normals seams are processed first
        if (seamMetaData.isLinkToValid())
        {
            continue;
        }

        if (!seamMetaData.isSeamSeriesUuidValid())
        {
            continue;
        }

        auto it = std::find_if(m_series.begin(), m_series.end(), [&seamMetaData] (const auto& seriesStats) { return seriesStats.uuid() == seamMetaData.seamSeriesUuid(); });

        if (it == m_series.end())
        {
            // should never happen
            // each processed seam belongs to a processed series
            continue;
        }

        it->importSeamMetaData(seamMetaData, currentProduct ? currentProduct->findSeam(seamMetaData.uuid()) : nullptr);
    }

    for (const auto& seamMetaData : processedSeams)
    {
        // only linked seams
        if (!seamMetaData.isLinkToValid())
        {
            continue;
        }

        if (!seamMetaData.isSeamSeriesUuidValid())
        {
            continue;
        }

        auto it = std::find_if(m_series.begin(), m_series.end(), [&seamMetaData] (const auto& seriesStats) { return seriesStats.uuid() == seamMetaData.seamSeriesUuid(); });

        if (it == m_series.end())
        {
            // should never happen
            // each processed seam belongs to a processed series
            continue;
        }

        it->importLinkedSeamMetaData(seamMetaData, currentProduct ? currentProduct->findSeam(seamMetaData.uuid()) : nullptr);
    }
}

void ProductStatistics::importSeriesMetaData(const SeamSeriesMetaData& metaData, SeamSeries* seamSeries)
{
    if (!metaData.isUuidValid())
    {
        return;
    }

    auto it = std::find_if(m_series.begin(), m_series.end(), [&metaData] (const auto& seriesStats) { return seriesStats.uuid() == metaData.uuid(); });

    if (it == m_series.end())
    {
        if (seamSeries)
        {
            m_series.emplace_back(metaData.uuid(), seamSeries->name(), seamSeries->visualNumber());
        } else
        {
            m_series.emplace_back(metaData.uuid());
        }

        m_series.back().importMetaData(metaData);
    } else
    {
        it->importMetaData(metaData);
    }
}

void ProductStatistics::clear()
{
    m_ioCount = 0;
    m_nioCount = 0;
    m_nios.clear();
    m_series.clear();
}

}
}
