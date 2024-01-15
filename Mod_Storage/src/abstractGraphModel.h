#pragma once

#include <QAbstractListModel>
#include <QDir>
#include <QUuid>
#include <QUrl>

#include <vector>

#include "fliplib/graphContainer.h"

class QFileSystemWatcher;

namespace precitec
{
namespace storage
{

/**
 * Base model for providing information about graphs.
 **/
class AbstractGraphModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
    /**
     * macroGraphModel to extend correspondent graph with macros
     **/
    Q_PROPERTY(precitec::storage::AbstractGraphModel* macroGraphModel READ macroGraphModel WRITE setMacroGraphModel NOTIFY macroGraphModelChanged)

    Q_PROPERTY(QString pdfFilesDir READ pdfFilesDir WRITE setPdfFilesDir NOTIFY pdfFilesDirChanged)

public:
    explicit AbstractGraphModel(QObject *parent = nullptr);
    ~AbstractGraphModel() override;

    // The following two functions are used for qmlRegisterType compatability
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @returns the name of the graph with the @p id.
     **/
    QString name(const QUuid &id) const;

    /**
     * @returns the index for the given @p id.
     **/
    Q_INVOKABLE QModelIndex indexFor(const QUuid &id) const;

    /**
     * @returns the graph for the given @p index
     **/
    fliplib::GraphContainer graph(const QModelIndex &index) const;

    void setMacroGraphModel(precitec::storage::AbstractGraphModel *macroGraphModel);
    precitec::storage::AbstractGraphModel* macroGraphModel() const
    {
        return m_macroGraphModel;
    }

    bool isLoading() const
    {
        return m_loadCounter > 0;
    }

    bool reloadingGraph(const QString &fileName)
    {
        auto changedGraph = std::find_if(m_graphs.begin(), m_graphs.end(), [fileName](const auto &actualPair){return std::get<1>(actualPair) == fileName;});
        if (changedGraph != m_graphs.end())
        {
            beginResetModel();
            m_graphs.erase(changedGraph);
            endResetModel();
            return true;
        }
        return false;
    }

    QString pdfFilesDir() const
    {
        return m_pdfFilesDir;
    }
    void setPdfFilesDir(const QString& pdfFilesDir);

    enum class StorageType
    {
        System,
        User
    };

Q_SIGNALS:
    void loadingChanged();
    void macroGraphModelChanged();
    void pdfFilesDirChanged();

protected:
    /**
     * @returns @c true if any file in @p files is not present in m_graphs or any path in m_graphs is not present in @p files.
     * The idea is to detect whether there are graphs which got added or removed.
     **/
    bool checkSymmetricDifference(const std::vector<std::pair<QFileInfo, StorageType>> &files);
    /**
     * Loads all graphs from @p files asynchronously.
     **/
    void loadGraphsFromFiles(const std::vector<std::pair<QFileInfo, StorageType>> &files);
    const std::vector<std::tuple<fliplib::GraphContainer, QString, StorageType>> &graphStorage() const
    {
        return m_graphs;
    }

    QUrl graphImage(const QModelIndex &index) const;

    const QStringList &pdfFiles() const
    {
        return m_pdfFiles;
    }

private:
    bool handleGraphLoaded(fliplib::GraphContainer &graphContainer, const QString &path);
    void reloadGraphFile(const QString &path);
    void createAvailablePdfFiles();
    static void extendGraphContainerWithAllMacros(fliplib::GraphContainer &targetGraph,
                                                  const AbstractGraphModel *macroGraphModel);
    std::vector<std::tuple<fliplib::GraphContainer, QString, StorageType>> m_graphs;
    int m_loadCounter = 0;
    QFileSystemWatcher *m_fileWatcher;
    std::vector<std::pair<QFileInfo, StorageType>> m_filesToLoad;
    AbstractGraphModel *m_macroGraphModel = nullptr;
    QMetaObject::Connection m_macroGraphModelDestroyed;
    QMetaObject::Connection m_macroGraphModelDataChanged;

    QString m_pdfFilesDir;
    QStringList m_pdfFiles;
};

}
}
Q_DECLARE_METATYPE(precitec::storage::AbstractGraphModel*)
