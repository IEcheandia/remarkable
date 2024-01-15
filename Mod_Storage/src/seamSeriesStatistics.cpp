#include "seamSeriesStatistics.h"
#include "seamSeriesMetaData.h"
#include "seamMetaData.h"
#include "seam.h"

using precitec::interface::ResultType;

namespace precitec
{
namespace storage
{

SeamSeriesStatistics::SeamSeriesStatistics(const QUuid& uuid)
    : m_uuid(uuid)
{
}

SeamSeriesStatistics::SeamSeriesStatistics(const QUuid& uuid, const QString& name, int visualNumber)
    : m_uuid(uuid)
    , m_name(name)
    , m_visualNumber(visualNumber)
{
}

bool SeamSeriesStatistics::empty() const
{
    return instanceCount() == 0;
}

double SeamSeriesStatistics::ioInPercent() const
{
    if (empty())
    {
        return 0.0;
    }

    return m_ioCount / double(instanceCount());
}

double SeamSeriesStatistics::nioInPercent() const
{
    if (empty())
    {
        return 0.0;
    }

    return m_nioCount / double(instanceCount());
}

void SeamSeriesStatistics::importMetaData(const SeamSeriesMetaData& metaData)
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
}

void SeamSeriesStatistics::importSeamMetaData(const SeamMetaData& metaData, Seam* seam)
{
    if (!metaData.isUuidValid())
    {
        return;
    }

    auto it = std::find_if(m_seams.begin(), m_seams.end(), [&metaData] (const auto& seamStats) { return seamStats.uuid() == metaData.uuid(); });

    if (it == m_seams.end())
    {
        if (seam)
        {
            m_seams.emplace_back(metaData.uuid(), seam->name(), seam->visualNumber());
        } else
        {
            m_seams.emplace_back(metaData.uuid());
        }

        m_seams.back().importMetaData(metaData);
    } else
    {
        it->importMetaData(metaData);
    }
}

void SeamSeriesStatistics::importLinkedSeamMetaData(const SeamMetaData& metaData, Seam* seam)
{
    auto it = std::find_if(m_seams.begin(), m_seams.end(), [&metaData] (const auto& seamStats) { return seamStats.uuid() == metaData.linkTo(); });

    if (it == m_seams.end())
    {
        // should never happen
        // linked seams are processed only after the original has been processed
        return;
    }

    it->importLinkedSeamData(metaData, seam);
}

}
}

