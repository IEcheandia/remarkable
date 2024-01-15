#include "resultsExporter.h"

#include <QFileInfo>
#include <QDateTime>
#include <QFutureWatcher>
#include <QtConcurrentRun>
#include <QColor>
#include <QDir>

#include <numeric>

#include <xlsxwriter/workbook.h>

#include "errorSettingModel.h"
#include "resultSettingModel.h"
#include "product.h"
#include "seam.h"
#include "seamSeries.h"
#include "resultsLoader.h"
#include "event/results.h"

namespace precitec
{
namespace storage
{

static constexpr int s_maxPositionsForChart{10000};

static lxw_workbook_options s_options = {.constant_memory = LXW_TRUE,
                                .use_zip64 = LXW_FALSE};

class Workbook
{
public:
    Workbook(const QString &directory, const QString &fileName)
        : m_workbook(workbook_new_opt((directory + fileName).toUtf8().constData(), &s_options))
        , m_headerFormat(workbook_add_format(m_workbook))
        , m_headerNioFormat(workbook_add_format(m_workbook))
        , m_positionFormat(workbook_add_format(m_workbook))
        , m_nioPositionFormat(workbook_add_format(m_workbook))
        , m_nioFormat(workbook_add_format(m_workbook))
    {
        format_set_bold(m_headerFormat);
        format_set_bottom(m_headerFormat, LXW_BORDER_THIN);
        format_set_right(m_headerFormat, LXW_BORDER_THIN);
        format_set_top(m_headerFormat, LXW_BORDER_THIN);

        format_set_bold(m_headerNioFormat);
        format_set_bottom(m_headerNioFormat, LXW_BORDER_THIN);
        format_set_right(m_headerNioFormat, LXW_BORDER_THIN);
        format_set_top(m_headerNioFormat, LXW_BORDER_THIN);
        format_set_font_color(m_headerNioFormat, LXW_COLOR_RED);

        format_set_bold(m_positionFormat);
        format_set_right(m_positionFormat, LXW_BORDER_THIN);

        format_set_bold(m_nioPositionFormat);
        format_set_right(m_nioPositionFormat, LXW_BORDER_THIN);
        format_set_font_color(m_nioPositionFormat, LXW_COLOR_RED);

        format_set_font_color(m_nioFormat, LXW_COLOR_RED);
    }

    ~Workbook()
    {
        save();
    }

    void save()
    {
        if (!m_workbook)
        {
            return;
        }
        workbook_close(m_workbook);
        m_workbook = nullptr;
    }

    lxw_worksheet *addWorksheet(const QString &name)
    {
        auto excelName = name;
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
            return workbook_add_worksheet(m_workbook, excelName.toUtf8().constData());
        }
        return workbook_add_worksheet(m_workbook, nullptr);
    }

    lxw_format *headerFormat() const
    {
        return m_headerFormat;
    }

    lxw_format *headerNioFormat() const
    {
        return m_headerNioFormat;
    }

    lxw_format *positionFormat() const
    {
        return m_positionFormat;
    }

    lxw_format *nioPositionFormat() const
    {
        return m_nioPositionFormat;
    }

    lxw_format *nioFormat() const
    {
        return m_nioFormat;
    }

    lxw_chart *addPlot()
    {
        return workbook_add_chart(m_workbook, LXW_CHART_LINE);
    }

    void setTitle(const QByteArray &title)
    {
        m_title = title;
        m_metaData.title = m_title.data();

        workbook_set_properties(m_workbook, &m_metaData);
    }

private:
    lxw_workbook *m_workbook;
    lxw_format *m_headerFormat;
    lxw_format *m_headerNioFormat;
    lxw_format *m_positionFormat;
    lxw_format *m_nioPositionFormat;
    lxw_format *m_nioFormat;
    QByteArray m_company = QByteArrayLiteral("Precitec GmbH & Co. KG");
    QByteArray m_comments = QByteArrayLiteral("Exported by WeldMaster");
    lxw_doc_properties m_metaData = {
        .title = nullptr,
        .subject = nullptr,
        .author = nullptr,
        .manager = nullptr,
        .company = m_company.data(),
        .category = nullptr,
        .keywords = nullptr,
        .comments = m_comments.data(),
        .status = nullptr,
        .hyperlink_base = nullptr
    };
    QByteArray m_title;
};


ResultsExporter::ResultsExporter(QObject *parent)
    : QObject(parent)
{
}

ResultsExporter::~ResultsExporter() = default;

void ResultsExporter::performExport(const QFileInfo& info, const QDateTime &date, precitec::storage::Product* product, const QString& serialNumber)
{
    if (!scheduleExport(info, date, product, serialNumber) || !verifyExportDirectory())
    {
        return;
    }
    exportNext();
}

bool ResultsExporter::exportNext()
{
    if (m_scheduledExports.empty())
    {
        return false;
    }
    const auto scheduled = m_scheduledExports.front();
    m_scheduledExports.pop();
    if (!scheduled.product)
    {
        return false;
    }

    const QString fileName = QStringLiteral("%1_%2_%3.xlsx")
        .arg(scheduled.date.toString(QStringLiteral("yyyyMMdd-HHmmss")))
        .arg(scheduled.product->name())
        .arg(scheduled.serialNumber);
    m_workbook = new Workbook{m_exportDirectory, fileName};
    m_workbook->setTitle(QStringLiteral("Results of Product \"%1\" (Serial Number: %2)").arg(scheduled.product->name()).arg(scheduled.serialNumber).toUtf8());

    const auto &seamSeries = scheduled.product->seamSeries();
    for (auto ss : seamSeries)
    {
        const auto &seams = ss->seams();
        for (auto seam : seams)
        {
            exportSeam(seam, scheduled.info);
        }
    }

    if (checkStartExport())
    {
        emit exportStarted(fileName);
        return true;
    }
    return false;
}

void ResultsExporter::performExport(const QFileInfo& info, const QDateTime &date, precitec::storage::Product *product, const QString &serialNumber, int seamSeries)
{
    if (isExporting() || !verifyExportDirectory())
    {
        return;
    }
    auto ss = product->findSeamSeries(seamSeries);
    const QString fileName = QStringLiteral("%1_%2_%3_%4.xlsx")
        .arg(date.toString(QStringLiteral("yyyyMMdd-HHmmss")))
        .arg(product->name())
        .arg(serialNumber)
        .arg(ss ? ss->visualNumber() : seamSeries);
    m_workbook = new Workbook{m_exportDirectory, fileName};
    if (ss)
    {
        m_workbook->setTitle(QStringLiteral("Results of series \"%1\" (%2) of Product \"%3\" (Serial Number: %4)")
                .arg(ss->name())
                .arg(ss->visualNumber())
                .arg(product->name())
                .arg(serialNumber).toUtf8());

        const auto &seams = ss->seams();
        for (auto seam : seams)
        {
            exportSeam(seam, info);
        }
    }
    if (checkStartExport())
    {
        emit exportStarted(fileName);
    }
}

void ResultsExporter::performExport(const QFileInfo& info, const QDateTime &date, precitec::storage::Product *product, const QString &serialNumber, int seamSeries, int seam)
{
    if (isExporting() || !verifyExportDirectory())
    {
        return;
    }
    auto s = product->findSeam(seamSeries, seam);
    const QString fileName = QStringLiteral("%1_%2_%3_%4_%5.xlsx")
        .arg(date.toString(QStringLiteral("yyyyMMdd-HHmmss")))
        .arg(product->name())
        .arg(serialNumber)
        .arg(s ? s->seamSeries()->visualNumber() : seamSeries)
        .arg(s ? s->visualNumber() : seam);

    m_workbook = new Workbook{m_exportDirectory, fileName};
    if (s)
    {
        m_workbook->setTitle(QStringLiteral("Results of seam \"%1\" (%2) of Product \"%3\" (Serial Number: %4)")
                .arg(s->name())
                .arg(s->visualNumber())
                .arg(product->name())
                .arg(serialNumber).toUtf8());
    }
    exportSeam(s, info);
    if (checkStartExport())
    {
        emit exportStarted(fileName);
    }
}

bool ResultsExporter::checkStartExport()
{
    if (!m_sheets.empty())
    {
        setExporting(true);
        return true;
    } else
    {
        delete m_workbook;
        m_workbook = nullptr;
        return false;
    }
}

void ResultsExporter::exportSeam(Seam* seam, const QFileInfo& productInstance)
{
    if (!seam)
    {
        return;
    }
    ResultsLoader *loader = new ResultsLoader{this};
    auto it = m_sheets.emplace(loader, m_workbook->addWorksheet(seam->name())).first;
    connect(loader, &ResultsLoader::resultsLoaded, this, std::bind(&ResultsExporter::exportSheet, this, loader), Qt::QueuedConnection);
    loader->setProductInstance(productInstance);
    loader->setSeamSeries(seam->seamSeries()->number());
    loader->setSeam(seam->number());
    if (!loader->isLoading())
    {
        m_sheets.erase(it);
        loader->deleteLater();
    }
}

void ResultsExporter::setResultsConfigModel(storage::ResultSettingModel *model)
{
    if (m_resultsConfigModel == model)
    {
        return;
    }
    m_resultsConfigModel = model;
    disconnect(m_resultsConfigModelDestroyedConnection);
    if (m_resultsConfigModel)
    {
        m_resultsConfigModelDestroyedConnection = connect(m_resultsConfigModel, &QObject::destroyed, this, std::bind(&ResultsExporter::setResultsConfigModel, this, nullptr));
    } else
    {
        m_resultsConfigModelDestroyedConnection = {};
    }

    emit resultsConfigModelChanged();
}

void ResultsExporter::setErrorConfigModel(ErrorSettingModel *model)
{
    if (m_errorConfigModel == model)
    {
        return;
    }
    m_errorConfigModel = model;
    disconnect(m_errorConfigModelDestroyedConnection);
    if (m_errorConfigModel)
    {
        m_errorConfigModelDestroyedConnection = connect(m_errorConfigModel, &QObject::destroyed, this, std::bind(&ResultsExporter::setErrorConfigModel, this, nullptr));
    } else
    {
        m_errorConfigModelDestroyedConnection = {};
    }

    emit errorConfigModelChanged();
}

void ResultsExporter::exportSheet(ResultsLoader* loader)
{
    const auto results{loader->takeResults()};

    auto it = m_sheets.find(loader);
    if (it == m_sheets.end())
    {
        removeLoader(loader);
        return;
    }
    auto sheet = it->second;

    const int firstRowIndex(5);

    const QString positionTitle = tr("Position in mm");
    worksheet_set_column(sheet, 0, 0, positionTitle.length(), m_workbook->positionFormat());
    worksheet_write_string(sheet, firstRowIndex, 0, positionTitle.toUtf8().constData(), m_workbook->headerFormat());
    const QString delta = tr("Delta in μs");
    worksheet_set_column(sheet, firstRowIndex + 1, 1, delta.length(), m_workbook->positionFormat());
    worksheet_write_string(sheet, firstRowIndex, 1, delta.toUtf8().constData(), m_workbook->headerFormat());
    // write header
    int columnIndex = 2;

    // Each result set in results is either result or error. Errors are sometimes in resultConfigModel, so do not use this to decide if it is error or result!
    auto isError = [this] (const auto& resultSet)
    {
        return m_errorConfigModel && m_errorConfigModel->getItem(resultSet.front().resultType());
    };
    auto nameForResult = [this, isError] (const auto &resultSet)
    {
        if (auto resultConfig = m_resultsConfigModel ? m_resultsConfigModel->getItem(resultSet.front().resultType()) : nullptr)
        {
            if (!isError(resultSet))
            {
                return resultConfig->name();
            }
            else
            {
                return QString::number(resultSet.front().resultType());
            }
        } else
        {
            return QString::number(resultSet.front().resultType());
        }
    };
    auto nameForError = [this] (const auto &resultSet)
    {
        if (auto resultConfig = m_errorConfigModel ? m_errorConfigModel->getItem(resultSet.front().resultType()) : nullptr)
        {
            return QString::number(resultSet.front().resultType()) + QString(" - ") + resultConfig->name();
        } else
        {
            return QString::number(resultSet.front().resultType());
        }
    };
    auto addHeader = [sheet, &columnIndex, nameForResult, nameForError] (const auto &resultSet, lxw_format *headerFormat, lxw_format *columnFormat, bool nio)
    {
        // if error not nio, name is equal to resultType
        QString name = nio ? nameForError(resultSet) : nameForResult(resultSet);

        worksheet_set_column(sheet, columnIndex, columnIndex, name.length() + 1, columnFormat);
        worksheet_write_string(sheet, firstRowIndex, columnIndex++, name.toUtf8().constData(), headerFormat);
    };

    // result columns
    for (const auto &resultSet : results)
    {
        if (resultSet.empty())
        {
            continue;
        }

        if (!isError(resultSet))
        {
            addHeader(resultSet, m_workbook->headerFormat(), nullptr, false);
        }
    }
    // error columns
    for (const auto &resultSet : results)
    {
        if (resultSet.empty())
        {
            continue;
        }

        if (isError(resultSet))
        {
            bool isNio = false;
            for (auto& result : resultSet)
            {
                if (result.isNio())
                {
                    isNio = true;
                    break;
                }
            }
            const auto& format = isNio ? m_workbook->headerNioFormat() : m_workbook->headerFormat();
            addHeader(resultSet, format, nullptr, isNio);
        }
    }
    // prepare positions map
    struct PositionInfo
    {
        std::size_t sampleNumber = 0;
        long int triggerDelta = 0;
        long int relativeTime = 0;
    };
    std::map<long int, PositionInfo> positions;

    for (const auto &resultSet : results)
    {
        for (const auto &result : resultSet)
        {
            std::size_t sampleNumber = result.value<double>().size();
            if (sampleNumber >= 1)
            {
                const auto position = result.context().position();
                if (positions.find(position) != positions.end())
                {
                    positions[position].sampleNumber = std::lcm(positions[position].sampleNumber, sampleNumber);
                } else
                {
                    positions.emplace(position, PositionInfo{sampleNumber,
                                                     result.context().taskContext().measureTask().get()->triggerDelta(),
                                                                result.context().relativeTime()});
                }
            }
        }
    }

    int chartColumn = columnIndex + 1;
    // collect all positions
    std::map<long int, int> positionToRow;
    int positionRowIndex = firstRowIndex + 1;
    for (const auto &position : positions)
    {
        positionToRow.insert(std::make_pair(position.first, positionRowIndex));
        positionRowIndex += position.second.sampleNumber;
    }
    positionRowIndex--;
    std::array<lxw_chart*, 3> plots{ {nullptr, nullptr, nullptr} };
    std::vector<std::map<uint32_t, std::variant<std::pair<int, bool>, double>>> dataByColumns;

    columnIndex = 2;
    for (const auto &resultSet : results)
    {
        if (resultSet.empty())
        {
            continue;
        }
        if (isError(resultSet))
        {
            continue;
        }
        std::map<uint32_t, std::variant<std::pair<int, bool>, double>> columnData;
        for (const auto &result : resultSet)
        {
            const auto position = result.context().position();
            auto it = positionToRow.find(position);
            if (it == positionToRow.end())
            {
                continue;
            }
            if (result.isNio())
            {
                continue;
            }
            const auto &value = result.value<double>();
            if (!value.empty())
            {
                const std::size_t step = positions[position].sampleNumber / value.size();
                for (std::size_t i = 0; i < value.size(); i++)
                {
                    columnData.emplace(std::make_pair(it->second + i * step, value[i]));
                }
            }
        }
        dataByColumns.emplace_back(std::move(columnData));
        if (auto resultConfig = m_resultsConfigModel ? m_resultsConfigModel->getItem(resultSet.front().resultType()) : nullptr)
        {
            if (resultConfig->visibleItem() && !isError(resultSet) && resultSet.size() && positionRowIndex < s_maxPositionsForChart)
            {
                const std::size_t index = resultConfig->plotterNumber() -1;
                if (!plots.at(index))
                {
                    plots[index] = m_workbook->addPlot();
                }
                auto series = chart_add_series(plots.at(index), nullptr, nullptr);
                chart_series_set_name(series, resultConfig->name().toUtf8().constData());
                chart_series_set_smooth(series, 0);
                chart_series_set_categories(series, sheet->name, firstRowIndex + 1, 0, positionRowIndex, 0);
                chart_series_set_values(series, sheet->name, firstRowIndex + 1, columnIndex, positionRowIndex, columnIndex);
                lxw_chart_line line = {
                    .color = static_cast<lxw_color_t>(QColor{resultConfig->lineColor()}.rgb()),
                    .none = 0,
                    .width = 1.0f
                };
                chart_series_set_line(series, &line);
                lxw_chart_fill fill = {
                    .color = static_cast<lxw_color_t>(QColor{resultConfig->lineColor()}.rgb())
                };
                lxw_chart_line markerLine = {
                    .color = static_cast<lxw_color_t>(QColor{resultConfig->lineColor()}.rgb())
                };
                chart_series_set_marker_type(series, LXW_CHART_MARKER_CIRCLE);
                chart_series_set_marker_fill(series, &fill);
                chart_series_set_marker_line(series, &markerLine);
                chart_series_set_marker_size(series, 2);
            }
        }
        columnIndex++;
    }
    // nio
    long int firstNioPosition = INT_MAX;
    for (const auto &resultSet : results)
    {
        if (resultSet.empty())
        {
            continue;
        }
        if (!isError(resultSet))
        {
            continue;
        }

        bool isNio(false);
        std::map<uint32_t, std::variant<std::pair<int, bool>, double>> columnData;
        for (const auto &result : resultSet)
        {
            const auto position = result.context().position();
            auto it = positionToRow.find(position);
            if (it == positionToRow.end())
            {
                continue;
            }

            const auto &value = result.value<double>();
            if (!value.empty())
            {
                int errorValue(0);
                if (result.isNio())
                {
                    errorValue = 1;
                    isNio = true;
                    firstNioPosition = std::min(firstNioPosition, it->first);
                }
                const std::size_t step = positions[position].sampleNumber / value.size();
                for (std::size_t i = 0; i < value.size(); i++)
                {
                    columnData.emplace(std::make_pair(it->second + i * step, std::make_pair(errorValue, isNio)));
                }
            }
        }
        dataByColumns.emplace_back(std::move(columnData));
        columnIndex++;
    }

    // write positions
    positionRowIndex = firstRowIndex + 1;
    std::vector<double> positionInSpace;
    std::vector<long int> positionInTime;
    positionInSpace.reserve(positionRowIndex);
    positionInTime.reserve(positionRowIndex);
    int maxPosition = 0;
    for (const auto &position : positions)
    {
        if (position.second.sampleNumber > 0)
        {
            const auto sampleDistance = (double) position.second.triggerDelta / position.second.sampleNumber;
            for (auto i = 0u; i < position.second.sampleNumber; i++)
            {
                const auto currentPosition = 0.001 * (position.first + (i * sampleDistance));
                positionInSpace.emplace_back(currentPosition);
                positionInTime.emplace_back(position.first);
                positionRowIndex++;
                maxPosition = currentPosition;
            }
        }
    }

    auto createAxis = [sheet, positionRowIndex, maxPosition] (auto plot)
    {
        auto axis = chart_axis_get(plot, LXW_CHART_AXIS_TYPE_X);
        chart_axis_set_name_range(axis, sheet->name, 0, 0);
        chart_axis_set_label_position(axis, LXW_CHART_AXIS_LABEL_POSITION_LOW);
        chart_axis_set_major_tick_mark(axis, LXW_CHART_AXIS_TICK_MARK_NONE);
    };

    lxw_image_options plotOptions = {
        .x_offset = 0,
        .y_offset = 0,
        .x_scale = 2.0,
        .y_scale = 1.5
    };

    std::size_t plotIndex = 0;
    for (int row = firstRowIndex + 1; row < positionRowIndex; row++)
    {
        long int position = positionInTime.at(row - firstRowIndex - 1);
        worksheet_write_number(sheet, row, 0, positionInSpace.at(row - firstRowIndex - 1), position >= firstNioPosition ? m_workbook->nioPositionFormat() : nullptr);
        worksheet_write_number(sheet, row, 1, position, nullptr);
        for (std::size_t column = 0; column < dataByColumns.size(); column++)
        {
            const auto& columnData = dataByColumns.at(column);
            auto it = columnData.find(row);
            if (it == columnData.end())
            {
                continue;
            }
            if (std::holds_alternative<double>(it->second))
            {
                worksheet_write_number(sheet, it->first, column + 2, std::get<double>(it->second), nullptr);
            }
            else
            {
                const auto& value = std::get<std::pair<int, bool>>(it->second);
                const auto& format = value.second ? m_workbook->nioFormat() : nullptr;
                worksheet_write_boolean(sheet, it->first, column + 2, value.first, format);
            }
        }

        // test plot
        if (plotIndex < plots.size() && (row - firstRowIndex - 1) % 25 == 0)
        {
            if (auto plot = plots.at(plotIndex))
            {
                createAxis(plot);
                worksheet_insert_chart_opt(sheet, row, chartColumn, plot, &plotOptions);
            }
            plotIndex++;
        }
    }

    removeLoader(loader);
}

void ResultsExporter::removeLoader(storage::ResultsLoader* loader)
{
    m_sheets.erase(loader);
    loader->deleteLater();
    if (m_sheets.empty())
    {
        auto watcher = new QFutureWatcher<void>{this};
        connect(watcher, &QFutureWatcher<void>::finished, this,
            [this, watcher]
            {
                watcher->deleteLater();
                delete m_workbook;
                m_workbook = nullptr;
                while (!m_scheduledExports.empty())
                {
                    if (exportNext())
                    {
                        break;
                    }
                }
                if (m_scheduledExports.empty())
                {
                    // finished
                    setExporting(false);
                }
            });
        watcher->setFuture(QtConcurrent::run(m_workbook, &Workbook::save));
    }
}

bool ResultsExporter::isExporting() const
{
    return m_exporting;
}

void ResultsExporter::setExportDirectory(const QString &exportDirectory)
{
    if (m_exportDirectory == exportDirectory)
    {
        return;
    }
    m_exportDirectory = exportDirectory;
    emit exportDirectoryChanged();
}

bool ResultsExporter::verifyExportDirectory()
{
    if (m_exportDirectory.isEmpty())
    {
        return false;
    }
    return QDir{}.mkpath(m_exportDirectory);
}

bool ResultsExporter::scheduleExport(const QFileInfo &info, const QDateTime &date, precitec::storage::Product *product, const QString &serialNumber)
{
    if (isExporting())
    {
        return false;
    }
    m_scheduledExports.emplace(ScheduledExport{info, date, product, serialNumber});
    return true;
}

void ResultsExporter::exportScheduled()
{
    if (isExporting())
    {
        return;
    }
    if (!verifyExportDirectory())
    {
        return;
    }
    exportNext();
}

void ResultsExporter::setExporting(bool exporting)
{
    if (m_exporting == exporting)
    {
        return;
    }
    m_exporting = exporting;
    emit exportingChanged();
}

}
}
