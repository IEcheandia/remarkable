#include "seamStatistics.h"
#include "seamSeriesMetaData.h"
#include "seam.h"

using precitec::interface::ResultType;

namespace precitec
{
namespace storage
{

SeamStatistics::SeamStatistics(const QUuid& uuid)
    : m_uuid(uuid)
{
}

SeamStatistics::SeamStatistics(const QUuid& uuid, const QString& name, int visualNumber)
    : m_uuid(uuid)
    , m_name(name)
    , m_visualNumber(visualNumber)
{
}

bool SeamStatistics::empty() const
{
    return instanceCount() == 0;
}

double SeamStatistics::ioInPercent() const
{
    if (empty())
    {
        return 0.0;
    }

    return m_ioCount / double(instanceCount());
}

double SeamStatistics::nioInPercent() const
{
    if (empty())
    {
        return 0.0;
    }

    return m_nioCount / double(instanceCount());
}

unsigned int SeamStatistics::ioCountIncludingLinkedSeams() const
{
    auto ioCount = m_ioCount;

    for (const auto& linkedSeam : m_linkedSeams)
    {
        ioCount += linkedSeam.ioCount();
    }

    return ioCount;
}

unsigned int SeamStatistics::nioCountIncludingLinkedSeams() const
{
    auto nioCount = m_nioCount;

    for (const auto& linkedSeam : m_linkedSeams)
    {
        nioCount += linkedSeam.nioCount();
    }

    return nioCount;
}

double SeamStatistics::ioCountIncludingLinkedSeamsInPercent() const
{
    const auto ioCount = ioCountIncludingLinkedSeams();
    const auto nioCount = nioCountIncludingLinkedSeams();

    const auto instanceCount = ioCount + nioCount;

    if (instanceCount == 0u)
    {
        return 0.0;
    }

    return ioCount / double(instanceCount);
}

double SeamStatistics::nioCountIncludingLinkedSeamsInPercent() const
{
    const auto ioCount = ioCountIncludingLinkedSeams();
    const auto nioCount = nioCountIncludingLinkedSeams();

    const auto instanceCount = ioCount + nioCount;

    if (instanceCount == 0u)
    {
        return 0.0;
    }

    return nioCount / double(instanceCount);
}

void SeamStatistics::importMetaData(const SeamMetaData& metaData)
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

        if (!m_linkedNios.empty())
        {
            auto linked_it = m_linkedNios.find(ResultType(nio.type));

            if (linked_it == m_linkedNios.end())
            {
                m_linkedNios.emplace(ResultType(nio.type), nio.count);
            } else
            {
                linked_it->second += nio.count;
            }
        }
    }

    if (metaData.isLinkToValid())
    {
        m_linkTo = metaData.linkTo();
    }
}

void SeamStatistics::importLinkedSeamData(const SeamMetaData& linkedSeamMetaData, Seam* linkedSeam)
{
    if (!linkedSeamMetaData.isUuidValid())
    {
        return;
    }

    auto it = std::find_if(m_linkedSeams.begin(), m_linkedSeams.end(), [&linkedSeamMetaData] (const auto& linkedSeamStats) { return linkedSeamStats.uuid() == linkedSeamMetaData.uuid(); });

    if (it == m_linkedSeams.end())
    {
        if (linkedSeam)
        {
            m_linkedSeams.emplace_back(linkedSeamMetaData.uuid(), linkedSeam->name(), linkedSeam->visualNumber());
        } else
        {
            m_linkedSeams.emplace_back(linkedSeamMetaData.uuid());
        }

        m_linkedSeams.back().importMetaData(linkedSeamMetaData);
    } else
    {
        it->importMetaData(linkedSeamMetaData);
    }

    if (m_linkedNios.empty())
    {
        // copy existing nios the first time a link seam is imported
        m_linkedNios = m_nios;
    }

    const auto& instanceNios = linkedSeamMetaData.nios();
    for (const auto& nio : instanceNios)
    {
        auto it = m_linkedNios.find(ResultType(nio.type));

        if (it == m_linkedNios.end())
        {
            m_linkedNios.emplace(ResultType(nio.type), nio.count);
        } else
        {
            it->second += nio.count;
        }
    }
}

void SeamStatistics::clear()
{
    m_uuid = {};
    m_name = QLatin1String("");
    m_visualNumber = -1;
    m_linkTo = {};

    m_ioCount = 0;
    m_nioCount = 0;

    m_nios.clear();
    m_linkedNios.clear();
    m_linkedSeams.clear();
}

}
}


