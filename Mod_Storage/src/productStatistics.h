#pragma once

#include "seamSeriesStatistics.h"
#include "event/resultType.h"

#include <map>
#include <vector>

namespace precitec
{
namespace storage
{

class ProductMetaData;
class SeamSeriesMetaData;
class Product;
class SeamSeries;

/**
 * Helper class to accumulate the instance MetaData of a product
 **/
class ProductStatistics
{

public:
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

    const std::vector<SeamSeriesStatistics>& seriesStatistics() const
    {
        return m_series;
    }

    void importMetaData(const ProductMetaData& metaData, Product* currentProduct);

    void importSeriesMetaData(const SeamSeriesMetaData& metaData, SeamSeries* seamSeries);

    void clear();

private:
    unsigned int m_ioCount = 0;
    unsigned int m_nioCount = 0;

    std::map<precitec::interface::ResultType, unsigned int> m_nios;

    std::vector<SeamSeriesStatistics> m_series;
};

}
}
