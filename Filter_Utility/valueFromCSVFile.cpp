#include "valueFromCSVFile.h"

#include "filter/algoArray.h"
#include "fliplib/TypeToDataTypeImpl.h"
#include "memory/memoryFile.h"
#include "module/moduleLogger.h"
#include "system/tools.h"

#include <csv.hpp>

#include <fstream>

namespace fs = std::filesystem;

namespace precitec::filter
{

namespace
{

constexpr const char* FILTER_ID = "e3932e31-3676-4ab1-841f-7a60a6ed580d";
constexpr const char* VARIANT_ID = "8ab151ef-86e2-411b-a063-bc4b6df40d3a";

constexpr const char* PIPE_ID_ROWIN = "fcaa3df8-564e-493f-bacf-bb244006f2ea";
constexpr const char* PIPE_ID_COLUMNIN = "32c0aade-9b81-420e-b4fc-051535cd3a7c";
constexpr const char* PIPE_ID_VALUEOUT = "ccf4a0e9-a74a-4ace-9324-956bfacc29a1";

constexpr const char* ROWIN_NAME = "RowIn";
constexpr const char* COLUMNIN_NAME = "ColumnIn";
constexpr const char* VALUEOUT_NAME = "ValueOut";

constexpr const char* PARAMETER_FILENAME_NAME = "Filename";
constexpr const char* PARAMETER_DECIMAL_SEPARATOR_NAME = "DecimalSeparator";

char decimalSeparatorFromEnum(int enumValue)
{
    if (enumValue == 0)
    {
        return '.';
    }
    if (enumValue == 1)
    {
        return ',';
    }

    wmLog(eDebug, "Unknown decimal separator, using default decimal point.");
    return '.';
}

/**
 * @brief Creates a temporary copy of the input file where all commas are replaced by points.
 *
 * This is needed because the library we use cannot handle commas as decimal separator and can
 * detect the field separator automatically only when reading from a file, so we replace the separator
 * first and pretend to store the replaced content in a file (which resides in memory, not on disk).
 *
 * @returns a temporary in-memory copy of the input file with commas replaced by points.
 */
std::unique_ptr<system::MemoryFile> replaceCommaByPoint(const fs::path& inputFile)
{
    std::ifstream file{inputFile};
    file >> std::noskipws;

    auto tempOutFile = std::make_unique<system::MemoryFile>("with_decimal_points.csv");

    std::ofstream inMemoryFileStream{tempOutFile->filepath()};
    std::replace_copy(std::istream_iterator<char>{file},
                      std::istream_iterator<char>{},
                      std::ostream_iterator<char>{inMemoryFileStream},
                      ',',
                      '.');
    return tempOutFile;
}

/**
 * @brief Interprets the given file as a CSV file and parses its data.
 *
 * @returns a table of data from the CSV file. Empty in case of an error.
 */
DoubleIndexedTable readTableFromCSV(const fs::path& fileToRead)
{
    DoubleIndexedTable data;
    try
    {
        csv::CSVFormat format = csv::CSVFormat::guess_csv();
        format.variable_columns(csv::VariableColumnPolicy::THROW);

        for (const csv::CSVRow& row : csv::CSVReader{fileToRead.string(), std::move(format)})
        {
            std::vector<double> rowData(row.size());
            std::transform(row.begin(), row.end(), rowData.begin(), [](csv::CSVField& field)
                           { return field.get<double>(); });
            data.emplace_row(std::move(rowData));
        }
    }
    catch (const std::runtime_error& e)
    {
        wmLog(eDebug, "Unable to parse CSV file %s: %s\n", fileToRead, e.what());
        data.clear();
    }
    return data;
}

} // namespace

interface::GeoDoublearray DoubleIndexedTable::valueOfCell(const interface::GeoDoublearray& rowIndexGeo, const interface::GeoDoublearray& colIndexGeo) const
{
    auto indexPair = toIndexPair(rowIndexGeo, colIndexGeo);
    if (!indexPair.has_value())
    {
        return {
            rowIndexGeo.context(), geo2d::Doublearray{1, 0.0, eRankMin},
            rowIndexGeo.analysisResult(), interface::NotPresent};
    }

    const auto [rowIndex, colIndex] = indexPair.value();
    return {
        rowIndexGeo.context(), geo2d::Doublearray{1, m_data[rowIndex][colIndex], eRankMax},
        rowIndexGeo.analysisResult(), rowIndexGeo.rank()};
}

std::optional<std::pair<std::size_t, std::size_t>> DoubleIndexedTable::toIndexPair(const interface::GeoDoublearray& rowIndexGeo, const interface::GeoDoublearray& colIndexGeo) const
{
    if (inputIsInvalid(rowIndexGeo) || inputIsInvalid(colIndexGeo))
    {
        return std::nullopt;
    }
    const auto rowIndex = static_cast<std::size_t>(rowIndexGeo.ref().getData()[0]);
    const auto colIndex = static_cast<std::size_t>(colIndexGeo.ref().getData()[0]);

    if (rowIndex >= m_data.size() || colIndex >= m_data[rowIndex].size())
    {
        return std::nullopt;
    }
    return std::pair{rowIndex, colIndex};
}

//-------------------------------------------------------------------------------------------------
// The main filter implementation.
//-------------------------------------------------------------------------------------------------

ValueFromCSVFile::ValueFromCSVFile()
    : ValueFromCSVFile{fs::path{system::wmBaseDir()} / "config"}
{
}

ValueFromCSVFile::ValueFromCSVFile(fs::path baseDir)
    : TransformFilter("ValueFromCSVFile", Poco::UUID{FILTER_ID})
    , m_rowIn(nullptr)
    , m_columnIn(nullptr)
    , m_valueOut(this, VALUEOUT_NAME)
    , m_baseDir(std::move(baseDir))
{
    parameters_.add(PARAMETER_FILENAME_NAME, fliplib::Parameter::TYPE_string, "");
    parameters_.add(PARAMETER_DECIMAL_SEPARATOR_NAME, fliplib::Parameter::TYPE_int, 0);

    setInPipeConnectors({
        {Poco::UUID(PIPE_ID_ROWIN), m_rowIn, ROWIN_NAME, 1, ROWIN_NAME},
        {Poco::UUID(PIPE_ID_COLUMNIN), m_columnIn, COLUMNIN_NAME, 1,
         COLUMNIN_NAME},
    });
    setOutPipeConnectors({
        {Poco::UUID(PIPE_ID_VALUEOUT), &m_valueOut, VALUEOUT_NAME, 1,
         VALUEOUT_NAME},
    });
    setVariantID(Poco::UUID(VARIANT_ID));
}

void ValueFromCSVFile::setParameter()
{
    TransformFilter::setParameter();

    const auto decimalSeparator = decimalSeparatorFromEnum(parameters_.getParameter(PARAMETER_DECIMAL_SEPARATOR_NAME).convert<int>());
    if (decimalSeparator != m_decimalSeparator)
    {
        m_decimalSeparator = decimalSeparator;
    }

    const auto filename = parameters_.getParameter(PARAMETER_FILENAME_NAME).convert<std::string>();

    if (filename != m_filename)
    {
        m_filename = filename;
        const fs::path fileToRead = m_baseDir / m_filename;

        std::unique_ptr<system::MemoryFile> tempFile;
        if (m_decimalSeparator == ',')
        {
            tempFile = replaceCommaByPoint(fileToRead);
        }

        m_data = readTableFromCSV(tempFile ? tempFile->filepath() : fileToRead.string());
    }
}

bool ValueFromCSVFile::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == ROWIN_NAME)
    {
        m_rowIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&pipe);
    }
    else if (pipe.tag() == COLUMNIN_NAME)
    {
        m_columnIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&pipe);
    }
    else
    {
        return false;
    }
    return fliplib::BaseFilter::subscribe(pipe, group);
}

void ValueFromCSVFile::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    preSignalAction();
    const auto rowIndexGeo = m_rowIn->read(m_oCounter);
    const auto colIndexGeo = m_columnIn->read(m_oCounter);

    auto resultGeo = m_data.valueOfCell(rowIndexGeo, colIndexGeo);

    m_valueOut.signal(resultGeo);
}

} // namespace precitec::filter