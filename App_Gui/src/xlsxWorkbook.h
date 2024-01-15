#pragma once

#include <QObject>
#include <QString>
#include <memory>

struct lxw_worksheet;
struct lxw_workbook;
struct lxw_format;
struct lxw_doc_properties;
struct lxw_chart;

namespace precitec::gui
{

class AbstractWorksheet;

/**
 * Helper class play a decorator role for setup and storing xlsx data.
 *
 * Ex.
 * auto workbook = new Workbook{this};
 * workbook->prepareWorkbook(path, fileName, title);
 *
 * initialisation of hardware parameters worksheet
 * auto hardwareParametersWorksheet = std::make_unique<HardwareParametersWorksheet>();
 * hardwareParametersWorksheet->setProduct(m_product);
 * m_workbook->addWorksheet(std::move(hardwareParametersWorksheet), QString("Hardware parameters overview"));
 * workbook->save()
 **/
class Workbook : public QObject
{
    Q_OBJECT
public:
    Workbook(QObject *parent = nullptr);
    ~Workbook();

    void prepareWorkbook(const QString &directory, const QString &fileName, const QString &title);
    void addWorksheet(std::unique_ptr<AbstractWorksheet> worksheet, const QString &worksheetName);
    void save();

    const std::vector<std::unique_ptr<AbstractWorksheet>> &worksheets(); // for testing
private:
    QByteArray m_title;

    lxw_doc_properties *m_metaData = nullptr;

    lxw_workbook *m_workbook = nullptr;
    std::vector<std::unique_ptr<AbstractWorksheet>> m_worksheets;
};

} // namespace precitec::gui
