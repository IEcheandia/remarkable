#pragma once

#include "abstractGraphModel.h"

#include <QDir>

class QFileSystemWatcher;

namespace precitec
{
namespace storage
{

/**
 * Model containing information about the graphs available in the configuration.
 *
 * The model provides the following roles:
 * @li name
 * @li uuid
 **/
class GraphModel : public AbstractGraphModel
{
    Q_OBJECT
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
public:
    explicit GraphModel(QObject *parent = nullptr);
    ~GraphModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

     /**
      * Loads the graphs in @p systemGraphStorage and @p userGraphStorage.
      **/
     Q_INVOKABLE void loadGraphs(const QString &systemGraphStorage, const QString &userGraphStorage = QString());

private:
    /**
     * Loads all graphs in the @p graphStorageDirectories.
     **/
     void loadGraphs(const std::vector<std::pair<QDir, StorageType>> &graphStorageDirectories);
    QFileSystemWatcher *m_watcher;
    QString m_systemDir;
    QString m_userDir;
};

}
}

Q_DECLARE_METATYPE(precitec::storage::GraphModel*)
