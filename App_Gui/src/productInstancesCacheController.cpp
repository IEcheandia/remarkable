#include "productInstancesCacheController.h"
#include "productInstancesCacheEditor.h"

#include <limits>

#include <QDir>

namespace precitec
{
namespace gui
{

ProductInstancesCacheController::ProductInstancesCacheController(QObject *parent)
    : QObject(parent)
{
}

void ProductInstancesCacheController::addProductInstancePathToBuffer(const QString &pathToProductInstance)
{
    m_buffer.push_back(pathToProductInstance);
}

bool ProductInstancesCacheController::liveMode() const
{
    return m_liveMode;
}

void ProductInstancesCacheController::setLiveMode(bool liveMode)
{
    if (m_liveMode == liveMode)
    {
        return;
    }

    m_liveMode = liveMode;
    liveModeChanged();
}

void ProductInstancesCacheController::cacheBuffer()
{
    ProductInstancesCacheEditor editor(m_liveMode);
    editor.setProductInstanceMaxNumber(INT32_MAX);
    if (!m_absoluteCacheFileName.isEmpty())
    {
        editor.setAbsoluteCacheFileName(m_absoluteCacheFileName);
    }
    editor.add(m_buffer);
}

void ProductInstancesCacheController::clearCacheBuffer()
{
    m_buffer.clear();
}

bool ProductInstancesCacheController::exists(const QString &directory)
{
    return QDir(directory).exists();
}

void ProductInstancesCacheController::setAbsoluteCacheFileName(const QString &fileName)
{
    m_absoluteCacheFileName = fileName;
}

}
}