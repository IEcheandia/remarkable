#pragma once

#include <QVariant>
#include <QString>
#include <QObject>

struct lxw_worksheet;
struct lxw_workbook;
struct lxw_format;

namespace precitec::gui
{

/**
 * Abstract augment class play a decorator role for writing, forming, testing xlsx data.
 **/
class AbstractWorksheet : public QObject
{
    Q_OBJECT
public:
    AbstractWorksheet(QObject *parent);
    virtual ~AbstractWorksheet() = default;

    // functions for testing
    QVariant readFromCellValue(std::size_t row, std::size_t column);

    QString name() const
    {
        return m_name;
    }

protected:
    // utility functions
    void writeToTable(std::size_t row, std::size_t column, const QVariant &cellValue);
    void fitColumnsWidth();
    void addBoxBorders(std::size_t topRow, std::size_t topColumn, std::size_t bottomRow, std::size_t bottomColumn);
    void centerAllCellsInBox(std::size_t topRow,
                             std::size_t topColumn,
                             std::size_t bottomRow,
                             std::size_t bottomColumn);
    void colorAllCellInBox(std::size_t topRow, std::size_t topColumn, std::size_t bottomRow, std::size_t bottomColumn);

    virtual void fromTableToXlsx();
    lxw_worksheet *worksheet();

private:
    friend class Workbook;
    virtual void formWorksheet() = 0;
    void addToWorkbook(lxw_workbook *lxwWorkbook, const QString &worksheetName);

    // There are two key structures for xlsx files:
    lxw_workbook *m_workbook = nullptr;
    lxw_worksheet *m_worksheet = nullptr; // It is cleaned via workbook_close(m_workbook) by xlsxWorkbook class

    struct Cell
    {
        QVariant value;
        lxw_format *format = nullptr;
    };
    // the following structure is used as a buffer for convenient formatting and testing
    std::map<std::pair<size_t, size_t>, Cell> m_table;

    QString m_name;
};

} // namespace precitec::gui