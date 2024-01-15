#include "videoImportProductInstanceModel.h"

namespace precitec
{
namespace gui
{

VideoImportProductInstanceModel::VideoImportProductInstanceModel(QObject *parent)
    : AbstractVideoImportModel(parent)
{
    connect(this, &VideoImportProductInstanceModel::basePathChanged, this, [this] { init(m_basePath); } );
}

VideoImportProductInstanceModel::~VideoImportProductInstanceModel() = default;

void VideoImportProductInstanceModel::setBasePath(const QFileInfo &basePath)
{
    if (m_basePath == basePath)
    {
        return;
    }
    m_basePath = basePath;
    emit basePathChanged();
}

}
}
