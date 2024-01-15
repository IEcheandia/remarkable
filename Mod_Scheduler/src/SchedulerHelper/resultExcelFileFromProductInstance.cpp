#include "resultExcelFileFromProductInstance.h"

#include "../../Mod_Storage/src/product.h"
#include "../../Mod_Storage/src/seamSeries.h"
#include "../../Mod_Storage/src/seam.h"
#include "../../Mod_Storage/src/resultSettingModel.h"
#include "../../Mod_Storage/src/errorSettingModel.h"
#include "../../Mod_Storage/src/extendedProductInfoHelper.h"

namespace precitec
{

using namespace storage;

namespace scheduler
{

ResultExcelFileFromProductInstance::ResultExcelFileFromProductInstance(QObject *parent)
        : QObject(parent),
          m_productModel(new precitec::storage::ProductModel(parent)),
          m_resultsExporter(new precitec::storage::ResultsExporter(parent)),
          m_extendedProductHelper{new ExtendedProductInfoHelper{this}}
{
}

void ResultExcelFileFromProductInstance::setResultDirectory(const QString &directory)
{
    m_resultDirectory = directory;
}

void ResultExcelFileFromProductInstance::setResultProductInstanceDirectory(const QString &productInstanceDirectory)
{
    m_productInstanceDirectory = productInstanceDirectory;
}

void ResultExcelFileFromProductInstance::setProductStorageDirectory(const QString &productStorageDirectory)
{
    m_productStorageDirectory = productStorageDirectory;
}

void ResultExcelFileFromProductInstance::createResultExcelFile()
{
    if (m_productInstanceDirectory.isEmpty() || m_resultDirectory.isEmpty() || m_productStorageDirectory.isEmpty())
    {
        emit failed();
        return;
    }

    m_productModel->loadProducts(m_productStorageDirectory);
    auto metaData = m_productMetaData.parse(QDir(m_productInstanceDirectory));
    precitec::storage::Product *product = nullptr;
    if (metaData.productUuid().has_value())
    {
        product = m_productModel->findProduct(metaData.productUuid().value());
    }
    else
    {
        emit failed();
        return;
    }
    connect(this->m_resultsExporter, &precitec::storage::ResultsExporter::exportStarted, this, &ResultExcelFileFromProductInstance::setResultFileName, Qt::QueuedConnection);
    connect(this->m_resultsExporter, &precitec::storage::ResultsExporter::exportingChanged, this, &ResultExcelFileFromProductInstance::checkResultFile, Qt::QueuedConnection);
    if (product && !product->isDefaultProduct())
    {
        ResultSettingModel *resultSetting{new ResultSettingModel{this}};
        m_resultsExporter->setResultsConfigModel(resultSetting);

        ErrorSettingModel *errorSettings{new ErrorSettingModel{this}};
        m_resultsExporter->setErrorConfigModel(errorSettings);

        QString serialNumber{QString::number(metaData.number())};
        if (auto serial = m_extendedProductHelper->serialNumber(metaData.extendedProductInfo()))
        {
            serialNumber = serial.value();
        }

        m_resultsExporter->setExportDirectory(m_resultDirectory);
        m_resultsExporter->performExport(QFileInfo(m_productInstanceDirectory),
                                      metaData.date(),
                                      product,
                                      serialNumber);
    }
    else
    {
        emit failed();
    }
}

void ResultExcelFileFromProductInstance::setResultFileName(const QString &resultFileName)
{
    if (m_resultFileName != resultFileName)
    {
        m_resultFileName = resultFileName;
    }
}

void ResultExcelFileFromProductInstance::checkResultFile()
{
    if (m_resultsExporter->isExporting())
    {
        return;
    }
    else
    {
        if (!resultFileName().isEmpty())
        {
            emit resultFileIsCreated();
        }
        else
        {
            emit failed();
        }
    }
}

ResultExcelFileFromProductInstance::~ResultExcelFileFromProductInstance() = default;

}
}
