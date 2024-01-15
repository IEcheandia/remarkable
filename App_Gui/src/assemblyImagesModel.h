#pragma once

#include <QAbstractListModel>
#include <QFileInfo>

#include <vector>

class QDir;
class QFileSystemWatcher;

namespace precitec
{
namespace gui
{

/**
 * The AssemblyImagesModel provides the paths to all assembly images in a specified directory.
 *
 * Provides the roles:
 * @li display: base name of the image file
 * @li path: absolute path of the image file
 * @li fileName: the name with extension but without path
 **/
class AssemblyImagesModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * Whether the model is currently loading data asynchronously
     **/
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
public:
    explicit AssemblyImagesModel(QObject *parent = nullptr);
    ~AssemblyImagesModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    QHash<int, QByteArray> roleNames() const override;

    /**
     * Fills the model with the images in @p directory.
     * This is performed in an async way.
     **/
    void loadImages(const QDir &directory);

    /**
     * Overloaded method to support loading from QML.
     **/
    Q_INVOKABLE void loadImages(const QString &directory);

    bool isLoading() const
    {
        return m_loading;
    }

Q_SIGNALS:
    void loadingChanged();

private:
    void performLoad(const QDir &directory);
    std::vector<QFileInfo> m_assemblyImages;
    bool m_loading{false};
    QFileSystemWatcher *m_watcher;
};

}
}
