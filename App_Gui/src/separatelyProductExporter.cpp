
#include "separatelyProductExporter.h"

#include <precitec/notificationSystem.h>
#include <precitec/copyService.h>

#include <QDir>
#include <QFutureWatcher>
#include <QtConcurrent>

#include "module/moduleLogger.h"
#include "../../Mod_Storage/src/product.h"
#include "../../Mod_Storage/src/seamSeries.h"
#include "../plugins/general/removableDevicePaths.h"
#include "../../App_Gui/plugins/general/weldmasterPaths.h"

using precitec::gui::components::notifications::NotificationSystem;
using precitec::gui::components::removableDevices::CopyService;
using precitec::gui::WeldmasterPaths;

namespace precitec
{
namespace gui
{

SeparatelyProductExporter::SeparatelyProductExporter(CopyService* copyService, QObject *parent):
    m_copyService(copyService)
{
    connect(m_copyService, &CopyService::backupInProgressChanged, this, [this]
    {
            m_copyServiceInProgress = this->m_copyService->isBackupInProgress();
            setCopyInProgress(m_copyServiceInProgress);
    }, Qt::QueuedConnection);
}

void SeparatelyProductExporter::start(const QString& path, const QString& scanfieldPath, Product* product)
{
    setCopyInProgress(true);
    const auto &productSubfolderName = RemovableDevicePaths::instance()->separatedProductJsonDir();
    const auto &referenceCurveSubfolderName = RemovableDevicePaths::instance()->separatedProductReferenceCurveDir();
    const auto &scanFieldImageFolderName = RemovableDevicePaths::instance()->separatedProductScanFieldImagesDir();
    std::shared_ptr<bool> toCopyScanfieldImages = std::make_shared<bool>(false);
    QFuture<void> future = QtConcurrent::run(
        [this, path, product, toCopyScanfieldImages, referenceCurveSubfolderName, productSubfolderName]()
        {
            if (!product)
            {
                NotificationSystem::instance()->warning(tr("Cannot find product!\nExport failed."));
                setCopyInProgress(false);
                *toCopyScanfieldImages = false;
                return;
            }

            QDir probableProductDir (path + product->name());
            if (probableProductDir.exists())
            {
                if (!probableProductDir.removeRecursively())
                {
                    NotificationSystem::instance()->warning(tr("Cannot delete a product folder with the same name in target directory.\nExport failed."));
                    setCopyInProgress(false);
                    *toCopyScanfieldImages = false;
                    return;
                }
            }

            const auto targetPathDirProduct(path + product->name() + "/" + productSubfolderName);
            if (!QDir{}.mkpath(targetPathDirProduct))
            {
                NotificationSystem::instance()->warning(tr("Cannot create a folder for the product json file in target directory.\nExport failed."));
                setCopyInProgress(false);
                *toCopyScanfieldImages = false;
                return;
            }

            // save json file with the product in the correspondent sub-folder
            const QFileInfo sourceFilePath(product->filePath());
            auto targetFilePath(targetPathDirProduct + "/" + sourceFilePath.fileName());

            if (!product->save(targetFilePath))
            {
                NotificationSystem::instance()->warning(tr("Cannot save product json file in target directory.\nExport failed."));
                setCopyInProgress(false);
                *toCopyScanfieldImages = false;
                return;
            }

            // save correspondent reference curve in case json file was saved successfully
            const auto targetReferenceCurvePath(path + product->name() + "/" + referenceCurveSubfolderName);
            if (!QDir{}.mkpath(targetReferenceCurvePath))
            {
                NotificationSystem::instance()->warning(tr("Cannot create path folder for the product .ref file in target directory.\nExport failed."));
                setCopyInProgress(false);
                *toCopyScanfieldImages = false;
                return;
            }
            if (!product->saveReferenceCurves(targetReferenceCurvePath))
            {
                NotificationSystem::instance()->warning(tr("Cannot save product .ref file in target directory.\nExport failed."));
                setCopyInProgress(false);
                *toCopyScanfieldImages = false;
                return;
            }
            *toCopyScanfieldImages = true;
            return;
        });

    auto watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished,
            [this, product, path, scanfieldPath, scanFieldImageFolderName, toCopyScanfieldImages]()
            {
                if (*toCopyScanfieldImages)
                {
                    exportAssemblyImage(path, product);
                    m_copyService->clear();
    
                    const auto &seamSeries = product->seamSeries();
                    for (const auto &series : seamSeries)
                    {
                        const auto uuidString = series->uuid().toString(QUuid::WithoutBraces);
                        const auto source = scanfieldPath + uuidString;
                        const auto target = path + product->name() + "/" + scanFieldImageFolderName + "/" + uuidString;
                        QDir sourceDir(source);
                        if (sourceDir.exists())
                        {
                            m_copyService->addElement(source, target);
                        }
                    }
                    if (m_copyService->size() != 0)
                    {
                        m_copyService->performCopy();
                    }
                    else
                    {
                        setCopyInProgress(false);
                        NotificationSystem::instance()->information(tr("Files are copied successfully."));
                    }
                }
                emit finished();
            });
    connect(watcher, &QFutureWatcher<void>::finished, watcher, &QFutureWatcher<void>::deleteLater);
    watcher->setFuture(future);
}

void SeparatelyProductExporter::setCopyInProgress(bool isCopying)
{
    if (m_isCopying != isCopying)
    {
        m_isCopying = isCopying;
        emit copyInProgressChanged(m_isCopying);
    }
}

void SeparatelyProductExporter::exportAssemblyImage(const QString& path, const Product* product) const
{
    if (!product->assemblyImage().isEmpty())
    {
        QFile source{WeldmasterPaths::instance()->assemblyImagesDir() + "/" + product->assemblyImage()};
        QDir targetProductDir{path + "/" + product->name()};
        const auto assemblyImageDir = QStringLiteral("assemblyImage");
        targetProductDir.mkdir(assemblyImageDir);
        const auto target = targetProductDir.path() + "/" + assemblyImageDir + "/" + product->assemblyImage();
        source.copy(target);
    }
}

}
}
