#include "videoImportProductModel.h"

#include <QDir>

namespace precitec
{
namespace gui
{

VideoImportProductModel::VideoImportProductModel(QObject *parent)
    : AbstractVideoImportModel(parent)
{
    connect(this, &VideoImportProductModel::basePathChanged, this, &VideoImportProductModel::handleInit);
    connect(this, &VideoImportProductModel::directoryNameChanged, this, &VideoImportProductModel::handleInit);
}

VideoImportProductModel::~VideoImportProductModel() = default;

void VideoImportProductModel::setBasePath(const QString &basePath)
{
    if (m_basePath == basePath)
    {
        return;
    }
    m_basePath = basePath;
    emit basePathChanged();
}

void VideoImportProductModel::setDirectoryName(const QString &directoryName)
{
    if (m_directoryName == directoryName)
    {
        return;
    }
    m_directoryName = directoryName;
    emit directoryNameChanged();
}

void VideoImportProductModel::handleInit()
{
    if (m_basePath.isEmpty() || m_directoryName.isEmpty())
    {
        init({});
    } else
    {
        init({QDir{m_basePath}.absoluteFilePath(QStringLiteral("weldmaster/%1").arg(m_directoryName))});
    }
}

}
}
