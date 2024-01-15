#pragma once

#include "event/resultType.h"

#include <QUuid>
#include <map>
#include <vector>

namespace precitec
{
namespace storage
{

class SeamMetaData;
class Seam;

/**
 * Helper class to accumulate the instance MetaData of a product
 **/
class SeamStatistics
{

public:
    explicit SeamStatistics(const QUuid& uuid = {});
    explicit SeamStatistics(const QUuid& uuid, const QString& name, int visualNumber);

    const QUuid& uuid() const
    {
        return m_uuid;
    }

    const QString& name() const
    {
        return m_name;
    }

    int visualNumber() const
    {
        return m_visualNumber;
    }

    const QUuid& linkTo() const
    {
        return m_linkTo;
    }

    unsigned int ioCount() const
    {
        return m_ioCount;
    }

    unsigned int nioCount() const
    {
        return m_nioCount;
    }

    unsigned int instanceCount() const
    {
        return m_ioCount + m_nioCount;
    }

    bool empty() const;

    double ioInPercent() const;

    double nioInPercent() const;

    const std::map<precitec::interface::ResultType, unsigned int>& nios() const
    {
        return m_nios;
    }

    const std::map<precitec::interface::ResultType, unsigned int>& linkedNios() const
    {
        return m_linkedNios;
    }

    bool includesLinkedSeams() const
    {
        return !m_linkedSeams.empty();
    }

    const std::vector<SeamStatistics>& linkedSeams() const
    {
        return m_linkedSeams;
    }

    unsigned int ioCountIncludingLinkedSeams() const;

    unsigned int nioCountIncludingLinkedSeams() const;

    double ioCountIncludingLinkedSeamsInPercent() const;

    double nioCountIncludingLinkedSeamsInPercent() const;

    void importMetaData(const SeamMetaData& metaData);

    void importLinkedSeamData(const SeamMetaData& linkedSeamMetaData, Seam* linkedSeam);

    void clear();

private:
    QUuid m_uuid;
    QString m_name;
    int m_visualNumber = -1;
    QUuid m_linkTo;

    unsigned int m_ioCount = 0;
    unsigned int m_nioCount = 0;

    std::map<precitec::interface::ResultType, unsigned int> m_nios;
    std::map<precitec::interface::ResultType, unsigned int> m_linkedNios;
    std::vector<SeamStatistics> m_linkedSeams;
};

}
}


