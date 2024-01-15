#include "exportHelper.h"

#include <QFutureWatcher>
#include <QtConcurrent>
#include <QObject>

#include <precitec/notificationSystem.h>
#include <precitec/copyService.h>
#include <precitec/service.h>

#include "../../Mod_Scheduler/src/SchedulerHelper/pipeLogger.h"
#include "../../App_Gui/plugins/general/guiConfiguration.h"
#include "../../App_Gui/plugins/general/weldmasterPaths.h"
#include "../../Mod_Storage/src/product.h"
#include "../../App_Gui/plugins/general/removableDevicePaths.h"

using namespace precitec::gui::components::removableDevices;
using precitec::gui::WeldmasterPaths;
using precitec::storage::Product;
using precitec::gui::components::notifications::NotificationSystem;

namespace precitec
{
namespace scheduler
{

ExportHelper::ExportHelper(QObject* parent)
    : QObject(parent)
    , m_productModel(new precitec::storage::ProductModel(parent))
    , m_exportProduct(new SeparatelyProductExporter(new CopyService{this}))
{
    QObject::connect(m_exportProduct.get(), &SeparatelyProductExporter::finished, this, &ExportHelper::succeeded);
}

ExportHelper::~ExportHelper() = default;


void ExportHelper::initExportService(const std::string& uuid, const QString &productStorageDirectory, const QString &resultDirectory)
{
    m_uuid =  QString::fromStdString(uuid);
    m_productStorageDirectory = productStorageDirectory;
    m_productModel->loadProducts(m_productStorageDirectory);
    m_product = m_productModel->findProduct(m_uuid);
    m_resultDirectory = resultDirectory;
}

void ExportHelper::start()
{
    if (m_product)
    {
        const auto scanfieldImageDirectory = m_productModel->scanfieldImageStorageDirectory();
        m_exportProduct->start(m_resultDirectory + "/", scanfieldImageDirectory, m_product);
    }
    else
    {
        precitec::pipeLogger::SendLoggerMessage(m_fd, precitec::eError, "Scheduled product export failed!\n");
        emit failed();
        return;
    }
}

void ExportHelper::setLogFd(int fd)
{
    m_fd = fd;
}

void ExportHelper::setResultDirectory(const QString &directory)
{
    m_resultDirectory = directory;
}


QString ExportHelper::getProductName() const 
{
    return m_product->name();
}
}
}


