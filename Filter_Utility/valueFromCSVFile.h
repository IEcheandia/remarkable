#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "geo/geo.h"

#include <filesystem>
#include <optional>

namespace precitec::filter
{

/**
 * @brief A two-dimensional grid of data which can be accessed via double-valued indices.
 */
class DoubleIndexedTable
{
public:
    void clear()
    {
        m_data.clear();
    }

    void emplace_row(std::vector<double>&& row)
    {
        m_data.emplace_back(std::move(row));
    }

    interface::GeoDoublearray valueOfCell(const interface::GeoDoublearray& rowIndexGeo, const interface::GeoDoublearray& colIndexGeo) const;
private:
    std::optional<std::pair<std::size_t, std::size_t>> toIndexPair(const interface::GeoDoublearray& rowIndexGeo, const interface::GeoDoublearray& colIndexGeo) const;

    std::vector<std::vector<double>> m_data; // The data stored by this table.
};

/**
 * @brief A filter providing values of a CSV file at a given row and column.
 */
class FILTER_API ValueFromCSVFile : public fliplib::TransformFilter
{
private:
    // Pipes
    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_rowIn;
    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_columnIn;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_valueOut;
    // Parameters
    std::string m_filename; // The user-configured csv file name, relative to m_baseDir
    char m_decimalSeparator = '.';  // The character which separates integers from fractional parts of floating point numbers.
    // Data
    std::filesystem::path m_baseDir; // The folder in which to look for CSV files.

    DoubleIndexedTable m_data; // The data loaded from the CSV file.

public:
    ValueFromCSVFile();
    ValueFromCSVFile(std::filesystem::path baseDir);

    void setParameter() override;

    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceedGroup(const void* sender,
                      fliplib::PipeGroupEventArgs& event) override;
};

} // namespace precitec::filter