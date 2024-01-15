#pragma once

#include "abstractVideoImportModel.h"

namespace precitec
{
namespace gui
{

/**
 * A model providing all products (directories) for which videos are available for import.
 * It uses the @p basePath property combined with "weldmaster/@p directoryName " to look for sub directories.
 * All found sub directories are provided by the model. This matches how video and result data is downloaded
 * to a usb device and thus the video and result data on the usb device can be listed again.
 *
 * The model provides the following roles:
 * @li name (QString, name of directory)
 * @li fileInfo (QFileInfo of the directory)
 **/
class VideoImportProductModel : public AbstractVideoImportModel
{
    Q_OBJECT
    /**
     * Path from where to search for products, e.g. path to mounted usb device
     **/
    Q_PROPERTY(QString basePath READ basePath WRITE setBasePath NOTIFY basePathChanged)

    /**
     * Last element of the path to search for products. Combined with basePath/weldmaster
     **/
    Q_PROPERTY(QString directoryName READ directoryName WRITE setDirectoryName NOTIFY directoryNameChanged)
public:
    VideoImportProductModel(QObject *parent = nullptr);
    ~VideoImportProductModel() override;

    QString basePath() const
    {
        return m_basePath;
    }
    void setBasePath(const QString &basePath);

    QString directoryName() const
    {
        return m_directoryName;
    }
    void setDirectoryName(const QString &directoryName);

Q_SIGNALS:
    void basePathChanged();
    void directoryNameChanged();

private:
    void handleInit();
    QString m_basePath;
    QString m_directoryName;
};

}
}
