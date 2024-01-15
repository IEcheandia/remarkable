#include "xlsxAbstractWorksheet.h"

#include <xlsxwriter/workbook.h>

namespace precitec::gui
{

void AbstractWorksheet::addToWorkbook(lxw_workbook *lxwWorkbook, const QString &worksheetName)
{
    if (lxwWorkbook == nullptr)
    {
        return;
    }

    m_workbook = lxwWorkbook;
    m_name = worksheetName;
    auto excelName = m_name;

    excelName.replace(QLatin1Char('['), QLatin1Char('_'));
    excelName.replace(QLatin1Char(']'), QLatin1Char('_'));
    excelName.replace(QLatin1Char(':'), QLatin1Char('_'));
    excelName.replace(QLatin1Char('*'), QLatin1Char('_'));
    excelName.replace(QLatin1Char('?'), QLatin1Char('_'));
    excelName.replace(QLatin1Char('/'), QLatin1Char('_'));
    excelName.replace(QLatin1Char('\\'), QLatin1Char('_'));

    while (excelName.startsWith(QLatin1Char('`')))
    {
        excelName = excelName.remove(0, 1);
    }

    while (excelName.endsWith(QLatin1Char('`')))
    {
        excelName.chop(1);
    }

    excelName.truncate(31);

    if (workbook_validate_sheet_name(m_workbook, excelName.toUtf8().constData()) == LXW_NO_ERROR)
    {
        m_worksheet = workbook_add_worksheet(m_workbook, excelName.toUtf8().constData());
    }
    else
    {
        m_worksheet = workbook_add_worksheet(m_workbook, nullptr);
    }

    formWorksheet();
}

QVariant AbstractWorksheet::readFromCellValue(std::size_t row, std::size_t column)
{
    return (m_table.find(std::make_pair(row, column)) != m_table.end()) ? m_table[std::make_pair(row, column)].value : QVariant();
}

void AbstractWorksheet::fitColumnsWidth()
{
    std::size_t columnNumber = 0;
    std::for_each(m_table.begin(), m_table.end(), [&columnNumber](const auto &cell) {
        if (cell.first.second > columnNumber)
        {
            columnNumber = cell.first.second;
        }
    });

    columnNumber++;
    std::vector<std::size_t> columnSizes(columnNumber, 0);
    std::for_each(m_table.begin(), m_table.end(), [&columnSizes](const auto &cell) {
        if ((size_t)cell.second.value.toString().size() > (size_t)columnSizes.at(cell.first.second))
        {
            columnSizes[cell.first.second] = cell.second.value.toString().size();
        }
    });

    for (std::size_t columnSize = 0; columnSize < columnNumber; columnSize++)
    {
        worksheet_set_column(m_worksheet, columnSize, columnSize, ++columnSizes.at(columnSize), nullptr);
    }
}

void AbstractWorksheet::addBoxBorders(std::size_t topRow,
                                      std::size_t topColumn,
                                      std::size_t bottomRow,
                                      std::size_t bottomColumn)
{
    // check values
    if (!(topRow <= bottomRow && topColumn <= bottomColumn))
    {
        return;
    }

    // top row
    for (std::size_t column = topColumn; column <= bottomColumn; ++column)
    {
        const auto index = std::make_pair(topRow, column);
        const auto cellFormatIsNotInitialized =
            (m_table.find(index) == m_table.end() || m_table.at(index).format == nullptr);
        auto format = (cellFormatIsNotInitialized) ? workbook_add_format(m_workbook) : m_table[index].format;
        format_set_top(format, LXW_BORDER_MEDIUM);
        if (cellFormatIsNotInitialized)
        {
            m_table[index].format = format;
        }
    }

    // left column
    for (std::size_t row = topRow; row <= bottomRow; ++row)
    {
        const auto index = std::make_pair(row, topColumn);
        const auto cellFormatIsNotInitialized =
            (m_table.find(index) == m_table.end() || m_table.at(index).format == nullptr);
        auto format = (cellFormatIsNotInitialized) ? workbook_add_format(m_workbook) : m_table[index].format;
        format_set_left(format, LXW_BORDER_MEDIUM);
        if (cellFormatIsNotInitialized)
        {
            m_table[index].format = format;
        }
    }

    //  bottom row
    for (std::size_t column = topColumn; column <= bottomColumn; ++column)
    {
        const auto index = std::make_pair(bottomRow, column);
        const auto cellFormatIsNotInitialized =
            (m_table.find(index) == m_table.end() || m_table.at(index).format == nullptr);
        auto format = (cellFormatIsNotInitialized) ? workbook_add_format(m_workbook) : m_table[index].format;
        format_set_bottom(format, LXW_BORDER_MEDIUM);
        if (cellFormatIsNotInitialized)
        {
            m_table[index].format = format;
        }
    }

    // right column
    for (std::size_t row = topRow; row <= bottomRow; ++row)
    {
        const auto index = std::make_pair(row, bottomColumn);
        const auto cellFormatIsNotInitialized =
            (m_table.find(index) == m_table.end() || m_table.at(index).format == nullptr);
        auto format = (cellFormatIsNotInitialized) ? workbook_add_format(m_workbook) : m_table[index].format;
        format_set_right(format, LXW_BORDER_MEDIUM);
        if (cellFormatIsNotInitialized)
        {
            m_table[index].format = format;
        }
    }
}

void AbstractWorksheet::writeToTable(std::size_t row, std::size_t column, const QVariant &cellValue)
{
    m_table[std::make_pair(row, column)].value = cellValue;
}

void AbstractWorksheet::fromTableToXlsx()
{
    for (const auto &cell : m_table)
    {
        auto format = cell.second.format;
        switch (cell.second.value.userType())
        {
        case QVariant::UInt:
        case QVariant::Int:
            worksheet_write_number(m_worksheet, cell.first.first, cell.first.second, cell.second.value.toInt(), format);
            break;
        case QVariant::LongLong:
            worksheet_write_number(m_worksheet, cell.first.first, cell.first.second, cell.second.value.toFloat(),
                                   format);
            break;
        case QVariant::Double:
            worksheet_write_number(m_worksheet, cell.first.first, cell.first.second, cell.second.value.toDouble(),
                                   format);
            break;
        case QVariant::Bool:
            worksheet_write_boolean(m_worksheet, cell.first.first, cell.first.second, cell.second.value.toBool(),
                                    format);
            break;
        default:
        {
            if (cell.second.value.canConvert<QString>())
            {
                worksheet_write_string(m_worksheet, cell.first.first, cell.first.second,
                                       cell.second.value.toString().toUtf8(), format);
            }
            else
            {
                worksheet_write_string(m_worksheet, cell.first.first, cell.first.second, "UNKNOWN_TYPE", format);
            }
        }
        }
    }
}

void AbstractWorksheet::centerAllCellsInBox(std::size_t topRow,
                                            std::size_t topColumn,
                                            std::size_t bottomRow,
                                            std::size_t bottomColumn)
{
    // check values
    if (!(topRow <= bottomRow && topColumn <= bottomColumn))
    {
        return;
    }

    for (std::size_t row = topRow; row <= bottomRow; ++row)
    {
        for (std::size_t column = topColumn; column <= bottomColumn; ++column)
        {
            const auto index = std::make_pair(row, column);
            if (!(m_table.find(index) == m_table.end()))
            {
                const auto cellFormatIsNotInitialized = (m_table.at(index).format == nullptr);
                auto format = (cellFormatIsNotInitialized) ? workbook_add_format(m_workbook) : m_table[index].format;
                format_set_align(format, LXW_ALIGN_CENTER);
                if (cellFormatIsNotInitialized)
                {
                    m_table[index].format = format;
                }
            }
        }
    }
}

void AbstractWorksheet::colorAllCellInBox(std::size_t topRow,
                                          std::size_t topColumn,
                                          std::size_t bottomRow,
                                          std::size_t bottomColumn)
{
    // check values
    if (!(topRow <= bottomRow && topColumn <= bottomColumn))
    {
        return;
    }

    for (std::size_t row = topRow; row <= bottomRow; ++row)
    {
        for (std::size_t column = topColumn; column <= bottomColumn; ++column)
        {
            const auto index = std::make_pair(row, column);
            const auto cellFormatIsNotInitialized =
                (m_table.find(index) == m_table.end() || m_table.at(index).format == nullptr);
            auto format = (cellFormatIsNotInitialized) ? workbook_add_format(m_workbook) : m_table[index].format;
            format_set_fg_color(format, LXW_COLOR_GRAY);
            if (cellFormatIsNotInitialized)
            {
                m_table[index].format = format;
            }
        }
    }
}

lxw_worksheet *AbstractWorksheet::worksheet()
{
    return m_worksheet;
}

AbstractWorksheet::AbstractWorksheet(QObject *parent)
    : QObject(parent)
{
}

} // namespace precitec::gui