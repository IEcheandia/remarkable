#include "xlsxWorkbook.h"

#include <xlsxwriter/workbook.h>

#include "xlsxAbstractWorksheet.h"

#include <QDir>

namespace precitec::gui
{

Workbook::Workbook(QObject *parent)
    : QObject(parent)
{
}

Workbook::~Workbook()
{
    save();
}

void Workbook::save()
{
    if (!m_workbook)
    {
        return;
    }
    workbook_close(m_workbook); // save and clean up memory
    m_workbook = nullptr;
}

void Workbook::addWorksheet(std::unique_ptr<AbstractWorksheet> worksheet, const QString &worksheetName)
{
    worksheet->addToWorkbook(m_workbook, worksheetName);
    m_worksheets.emplace_back(std::move(worksheet));
}

const std::vector<std::unique_ptr<AbstractWorksheet>> &Workbook::worksheets()
{
    return m_worksheets;
}

void Workbook::prepareWorkbook(const QString &directory, const QString &fileName, const QString &title)
{
    if (QDir(directory).exists())
    {
        m_workbook = workbook_new((directory + fileName).toUtf8().constData());
        m_metaData = new lxw_doc_properties{.title = nullptr,
                                            .subject = nullptr,
                                            .author = nullptr,
                                            .manager = nullptr,
                                            .company = nullptr,
                                            .category = nullptr,
                                            .keywords = nullptr,
                                            .comments = nullptr,
                                            .status = nullptr,
                                            .hyperlink_base = nullptr};
        m_title = title.toUtf8();
        m_metaData->title = m_title.data();
        workbook_set_properties(m_workbook, m_metaData);
    }
}

} // namespace precitec::gui