#pragma once

#include <QObject>
#include <QFileInfo>
#include <QDateTime>
#include <QPointer>

#include <queue>

class QFileInfo;

struct lxw_worksheet;

namespace precitec
{

namespace storage
{
class ErrorSettingModel;
class ResultSettingModel;
class Product;
class ResultsLoader;
class Seam;

class Workbook;

/**
 * The ResultsExporter can export result data to an xlsx file.
 **/
class ResultsExporter : public QObject
{
    Q_OBJECT
    /**
     * The result configuration
     **/
    Q_PROPERTY(precitec::storage::ResultSettingModel *resultsConfigModel READ resultsConfigModel WRITE setResultsConfigModel NOTIFY resultsConfigModelChanged)
    /**
     * The error configuration
     **/
    Q_PROPERTY(precitec::storage::ErrorSettingModel *errorConfigModel READ errorConfigModel WRITE setErrorConfigModel NOTIFY errorConfigModelChanged)
    /**
     * Whether the ResultsExporter is currently exporting results
     **/
    Q_PROPERTY(bool exporting READ isExporting NOTIFY exportingChanged)
    /**
     * The directory where to create the file.
     * If empty export will not be performed
     **/
    Q_PROPERTY(QString exportDirectory READ exportDirectory WRITE setExportDirectory NOTIFY exportDirectoryChanged)
public:
    ResultsExporter(QObject *parent = nullptr);
    ~ResultsExporter() override;

    void setResultsConfigModel(precitec::storage::ResultSettingModel *model);
    precitec::storage::ResultSettingModel *resultsConfigModel() const
    {
        return m_resultsConfigModel;
    }

    void setErrorConfigModel(precitec::storage::ErrorSettingModel *model);
    precitec::storage::ErrorSettingModel *errorConfigModel() const
    {
        return m_errorConfigModel;
    }

    /**
     * Perform export of the given @p seam
     * @param info path to the product instance result data
     * @param date the date of the product instance
     * @param product The Product of the result data
     * @param serialNumber The serial number of the product instance
     * @param seamSeries The seam series number
     * @param seam The seam number
     **/
    Q_INVOKABLE void performExport(const QFileInfo &info, const QDateTime &date, precitec::storage::Product *product, const QString &serialNumber, int seamSeries, int seam);
    /**
     * Perform export of the given @p seamSeries
     * @param info path to the product instance result data
     * @param date the date of the product instance
     * @param product The Product of the result data
     * @param serialNumber The serial number of the product instance
     * @param seamSeries The seam series number
     **/
    Q_INVOKABLE void performExport(const QFileInfo &info, const QDateTime &date, precitec::storage::Product *product, const QString &serialNumber, int seamSeries);
    /**
     * Perform export of the product instance
     * @param info path to the product instance result data
     * @param date the date of the product instance
     * @param product The Product of the result data
     * @param serialNumber The serial number of the product instance
     **/
    Q_INVOKABLE void performExport(const QFileInfo &info, const QDateTime &date, precitec::storage::Product *product, const QString &serialNumber);

    /**
     * Schedule the export or the product instance. To trigger the export use @link{exportScheduled}.
     * @param info path to the product instance result data
     * @param date the date of the product instance
     * @param product The Product of the result data
     * @param serialNumber The serial number of the product instance
     **/
    Q_INVOKABLE bool scheduleExport(const QFileInfo &info, const QDateTime &date, precitec::storage::Product *product, const QString &serialNumber);

    /**
     * Export all scheduled product instances
     **/
    Q_INVOKABLE void exportScheduled();

    bool isExporting() const;

    QString exportDirectory() const
    {
        return m_exportDirectory;
    }
    void setExportDirectory(const QString &exportDirectory);

Q_SIGNALS:
    void exportingChanged();
    void resultsConfigModelChanged();
    void errorConfigModelChanged();
    void exportStarted(const QString &fileName);
    void exportDirectoryChanged();

private:
    bool exportNext();
    void exportSeam(Seam *seam, const QFileInfo &productInstance);
    void exportSheet(ResultsLoader *loader);
    void removeLoader(ResultsLoader *loader);
    bool checkStartExport();
    bool verifyExportDirectory();
    void setExporting(bool exporting);
    ResultSettingModel *m_resultsConfigModel = nullptr;
    QMetaObject::Connection m_resultsConfigModelDestroyedConnection;
    ErrorSettingModel *m_errorConfigModel = nullptr;
    QMetaObject::Connection m_errorConfigModelDestroyedConnection;
    Workbook *m_workbook = nullptr;
    std::map<ResultsLoader*, lxw_worksheet*> m_sheets;
    QString m_exportDirectory;
    struct ScheduledExport
    {
        QFileInfo info;
        QDateTime date;
        QPointer<Product> product;
        QString serialNumber;
    };
    std::queue<ScheduledExport> m_scheduledExports;
    bool m_exporting = false;
};

}
}
