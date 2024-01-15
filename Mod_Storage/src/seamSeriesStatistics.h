#pragma once

#include "seamStatistics.h"
#include "event/resultType.h"

#include <QUuid>
#include <map>
#include <vector>

namespace precitec
{
namespace storage
{

class SeamSeriesMetaData;
class SeamMetaData;
class Seam;

/**
 * Helper class to accumulate the instance MetaData of a product
 **/
class SeamSeriesStatistics
{

public:
    explicit SeamSeriesStatistics(const QUuid& uuid);
    explicit SeamSeriesStatistics(const QUuid& uuid, const QString& name, int visualNumber);

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

    const std::vector<SeamStatistics>& seamStatistics() const
    {
        return m_seams;
    }

    void importMetaData(const SeamSeriesMetaData& metaData);

    void importSeamMetaData(const SeamMetaData& metaData, Seam* seam);

    void importLinkedSeamMetaData(const SeamMetaData& metaData, Seam* seam);

private:
    QUuid m_uuid;
    QString m_name;
    int m_visualNumber = -1;

    unsigned int m_ioCount = 0;
    unsigned int m_nioCount = 0;

    std::map<precitec::interface::ResultType, unsigned int> m_nios;

    std::vector<SeamStatistics> m_seams;
};

}
}

