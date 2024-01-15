#include "productInstancesCacheEditor.h"

#include "videoRecorder/literal.h"
#include "videoRecorder/fileCommand.h"

#include <QDir>

namespace precitec
{
namespace gui
{

using namespace precitec::vdr;
ProductInstancesCacheEditor::ProductInstancesCacheEditor(bool liveMode, QObject *parent)
    : QObject(parent)
    , m_liveMode(liveMode)
{
    auto baseDirectory = QString::fromStdString(
        getenv(g_oWmBaseDirEnv.c_str()) ? (std::string(getenv(g_oWmBaseDirEnv.c_str())) + "/") : g_oWmBaseDirFallback);
    auto cacheFileDirectory = baseDirectory + QString::fromStdString(g_oWmDataDir);
    auto cacheFileName =
        (m_liveMode) ? QString::fromStdString(g_oLiveModeCacheFile) : QString::fromStdString(g_oProductCacheFile);
    m_absoluteCacheFileName = cacheFileDirectory + "/" + cacheFileName;
}

void ProductInstancesCacheEditor::setProductInstanceMaxNumber(std::size_t maxNumber)
{
    m_maxProductInstancesNumber = maxNumber;
}

void ProductInstancesCacheEditor::setAbsoluteCacheFileName(const QString &fileName)
{
    m_absoluteCacheFileName = fileName;
}

void ProductInstancesCacheEditor::add(const std::vector<QString> &productInstances)
{
    for (const auto &productInstance : productInstances)
    {
        auto productInstancePath = QDir::cleanPath(productInstance);

        if (!productInstancePath.endsWith(QDir::separator()))
        {
            productInstancePath.append(QDir::separator());
        }

        if (QDir(productInstance).exists())
        {
            CachePath functor{m_absoluteCacheFileName.toStdString(), productInstancePath.toStdString(),
                              m_maxProductInstancesNumber, g_oWmVideoDir};
            functor();
        }
    }
}

}
}