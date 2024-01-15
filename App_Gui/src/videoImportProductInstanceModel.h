#pragma once

#include "abstractVideoImportModel.h"

namespace precitec
{
namespace gui
{

/**
 * This model represents the next level after @link{VideoImportProductModel}. It lists
 * all sub directories (product instance) for a QFileInfo retrieved from @link{VideoImportProductModel}.
 *
 * The model provides the following roles:
 * @li name (QString, name of directory)
 * @li fileInfo (QFileInfo of the directory)
 **/
class VideoImportProductInstanceModel : public AbstractVideoImportModel
{
    Q_OBJECT
    /**
     * Path to the product for which video data should be provided.
     **/
    Q_PROPERTY(QFileInfo basePath READ basePath WRITE setBasePath NOTIFY basePathChanged)
public:
    VideoImportProductInstanceModel(QObject *parent = nullptr);
    ~VideoImportProductInstanceModel() override;

    QFileInfo basePath() const
    {
        return m_basePath;
    }
    void setBasePath(const QFileInfo &basePath);

Q_SIGNALS:
    void basePathChanged();

private:
    QFileInfo m_basePath;
};

}
}
