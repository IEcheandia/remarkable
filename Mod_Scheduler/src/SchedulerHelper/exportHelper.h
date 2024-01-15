#pragma once

#include <memory>

#include <QObject>

#include "../../Mod_Storage/src/productModel.h"
#include "../../App_Gui/src/separatelyProductExporter.h"

using precitec::gui::SeparatelyProductExporter;

class QCommandLineParser;

namespace precitec
{

namespace gui
{
namespace components
{
namespace removableDevices
{
class CopyService;
}
}
}

namespace scheduler
{

class ExportHelper : public QObject
{
    Q_OBJECT
public:
    ExportHelper(QObject *parent = nullptr);
    ~ExportHelper() override;

    void initExportService(const std::string& uuid, const QString &productStorageDirectory, const QString &resultDirectory);

    void start();

    void setLogFd(int fd);

    void setResultDirectory(const QString &directory);

    QString getProductName() const;

Q_SIGNALS:
    void succeeded();
    void failed();

private:

    int m_fd{-1};

    precitec::storage::Product* m_product = nullptr;
    precitec::storage::ProductModel* m_productModel = nullptr;
    std::unique_ptr<SeparatelyProductExporter> m_exportProduct = nullptr;

    QString m_uuid;
    QString m_resultDirectory;
    QString m_productStorageDirectory;
};

}
}
