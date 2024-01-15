#pragma once

#include <QObject>
#include <QFileInfo>

#include <map>
#include <memory>
#include <mutex>

namespace precitec
{
namespace storage
{

class Product;

/**
 * Helper class to locate stored video data from a seam directory.
 **/
class VideoDataLoader : public QObject
{
    Q_OBJECT
    /**
     * The base directory containing video data.
     **/
    Q_PROPERTY(QString dataDirectory READ dataDirectory WRITE setDataDirectory NOTIFY dataDirectoryChanged)

    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
public:
    VideoDataLoader(QObject *parent = nullptr);
    ~VideoDataLoader() override;

    QString dataDirectory() const
    {
        return m_dataDirectory;
    }
    void setDataDirectory(const QString &dataDirectory);

    bool isLoading() const
    {
        return m_loading;
    }

    /**
     * @returns the file to the image in the directory set by @link{update} for @p imageNumber.
     * If there is no image available an invalid QFileInfo is returned.
     **/
    Q_INVOKABLE QFileInfo getImageFile(quint32 imageNumber) const;

    /**
     * @returns the path to the image in the directory set by @link{update} for @p imageNumber.
     * If there is no image available an invalid QString is returned.
     **/
    Q_INVOKABLE QString getImagePath(quint32 imageNumber) const;

    /**
     * Updates the internal storage to contain information about the video data in the directory identified by
     * @param product The product
     * @param productInstance The directory name of the product instance
     * @param seamSeries
     * @param seam
     **/
    Q_INVOKABLE void update(precitec::storage::Product *product, const QString &productInstance, int seamSeries, int seam);

Q_SIGNALS:
    void dataDirectoryChanged();
    void loadingChanged();

private:
    void setLoading(bool set);
    struct Frame
    {
        QFileInfo imageFile;
        QFileInfo sampleFile;
    };
    std::map<uint32_t, Frame> m_frames;
    QString m_dataDirectory;
    bool m_loading{false};
    std::unique_ptr<std::mutex> m_mutex;
};

}
}
